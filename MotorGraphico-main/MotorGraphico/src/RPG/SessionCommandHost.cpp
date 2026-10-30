#include "RPG/SessionCommandHost.h"

#include <algorithm>
#include <cstdlib>
#include <string>

// E-04 (segunda rebanada). Ver la nota larga del header.

namespace RPG {

namespace {

std::string Param(const MapEventCommand& command, const std::string& key,
                  const std::string& porDefecto = {}) {
    const auto encontrado = command.params.find(key);
    return encontrado == command.params.end() ? porDefecto : encontrado->second;
}

int Cantidad(const MapEventCommand& command) {
    const int n = std::atoi(Param(command, "cantidad", "1").c_str());
    // Una cantidad de cero o negativa en «dar objeto» es un comando mal
    // rellenado, no una forma retorcida de quitarlo: para eso está
    // inventario.quitar. Se trata como uno.
    return n > 0 ? n : 1;
}

CommandResult Fallo(std::string diagnostico) {
    CommandResult r;
    r.outcome = CommandOutcome::Failed;
    r.diagnostic = std::move(diagnostico);
    return r;
}

std::size_t CuantosHay(const std::vector<std::string>& inventario, const std::string& objeto) {
    return static_cast<std::size_t>(
        std::count(inventario.begin(), inventario.end(), objeto));
}

}  // namespace

Result<bool> EvaluateConditionWithInventory(const MapEventCondition& condition,
                                            const NarrativeState& state,
                                            const std::vector<std::string>& inventory) {
    if (condition.kind != EventConditionKind::Item) {
        return EvaluateCondition(condition, state);
    }
    return EvaluateItemCondition(condition, inventory);
}

Result<bool> EvaluateItemCondition(const MapEventCondition& condition,
                                   const std::vector<std::string>& inventory) {
    if (condition.kind != EventConditionKind::Item) {
        return Result<bool>::Error("EvaluateItemCondition solo sabe de condiciones de objeto");
    }
    if (condition.reference.empty()) {
        return Result<bool>::Error("una condición que no nombra nada no se puede evaluar");
    }
    const std::size_t hay = CuantosHay(inventory, condition.reference);
    const std::string op = condition.comparison.empty() ? ">=" : condition.comparison;
    // Sin cantidad, «tiene al menos uno» es lo que el autor quiere decir
    // casi siempre; obligarle a escribir ">= 1" sería ruido.
    const long long esperado = condition.value.empty() ? 1 : std::atoll(condition.value.c_str());
    const long long actual = static_cast<long long>(hay);

    bool resultado = false;
    if (op == ">=") resultado = actual >= esperado;
    else if (op == ">") resultado = actual > esperado;
    else if (op == "<=") resultado = actual <= esperado;
    else if (op == "<") resultado = actual < esperado;
    else if (op == "==") resultado = actual == esperado;
    else if (op == "!=") resultado = actual != esperado;
    else return Result<bool>::Error("comparación desconocida: '" + op + "'");

    return Result<bool>::Ok(condition.negated ? !resultado : resultado);
}

CommandResult SessionCommandHost::execute(const MapEventCommand& command) {
    const Result<bool> valido = ValidateCommand(command);
    if (!valido.isOk()) {
        return Fallo(valido.errorMessage());
    }

    if (command.type == "inventario.dar") {
        const std::string objeto = Param(command, "objeto");
        for (int i = 0; i < Cantidad(command); ++i) {
            m_inventory.push_back(objeto);
        }
        return CommandResult{};
    }

    if (command.type == "inventario.quitar") {
        const std::string objeto = Param(command, "objeto");
        const int cuantos = Cantidad(command);
        // Todo o nada: quitar tres de una llave cuando solo hay dos
        // dejaría al jugador sin las dos y sin la puerta abierta. Es
        // preferible fallar y que el autor lo vea.
        if (static_cast<int>(CuantosHay(m_inventory, objeto)) < cuantos) {
            return Fallo("el jugador no tiene " + std::to_string(cuantos) + " de '" + objeto + "'");
        }
        for (int i = 0; i < cuantos; ++i) {
            const auto encontrado = std::find(m_inventory.begin(), m_inventory.end(), objeto);
            m_inventory.erase(encontrado);
        }
        return CommandResult{};
    }

    if (command.type == "economia.oro") {
        const int cantidad = std::atoi(Param(command, "valor", "0").c_str());
        // addGold() nunca deja el oro negativo: un cobro mayor que el
        // saldo se recorta en silencio. Para un peaje de evento eso es
        // exactamente lo que NO queremos —la puerta se abriría sin
        // haberla pagado— así que aquí se comprueba antes.
        if (cantidad < 0 && m_session.gold() < -cantidad) {
            return Fallo("al jugador le faltan " +
                         std::to_string(-cantidad - m_session.gold()) + " de oro");
        }
        m_session.addGold(cantidad);
        return CommandResult{};
    }

    if (command.type == "mapa.ir") {
        // DEC-30 — Terminal. Se anota la transición y se termina el
        // evento: lo que haya debajo no se ejecuta, y E-07 ya lo marca
        // como inalcanzable en el editor.
        m_transition = LevelTransition{};
        m_transition.pending = true;
        m_transition.levelPath = Param(command, "mapa");
        const std::string entrada = Param(command, "entrada");
        if (!entrada.empty()) {
            // La entrada nombrada la resuelve quien carga el nivel: aquí
            // no se conocen las rutas ni las celdas del destino.
            m_transition.hasEntryPosition = false;
        }
        CommandResult r;
        r.outcome = CommandOutcome::EndEvent;
        return r;
    }

    if (command.type == "combate.iniciar") {
        RPG::BattleRequest peticion;
        peticion.monsterId = Param(command, "encuentro");
        peticion.flagVictory = Param(command, "flag_victoria");
        peticion.flagDefeat = Param(command, "flag_derrota");
        peticion.flagHuida = Param(command, "flag_huida");                 // CAER-01
        peticion.ineludible = Param(command, "ineludible", "false") == "true";
        m_lastOutcome = BattleOutcome::InProgress;
        m_session.startNarrativeBattle(peticion);
        // Si la sesion no ha entrado en combate, el enemigo no existe en
        // el catalogo o no hay mundo donde ponerlo. Fallar aqui es mejor
        // que esperar para siempre a un combate que nunca empezo.
        if (m_session.mode() != GameMode::Battle) {
            return Fallo("no se pudo iniciar el encuentro '" + peticion.monsterId + "'");
        }
        CommandResult r;
        r.outcome = CommandOutcome::Wait;
        r.waitMs = 0;   // sin duracion conocida: la resuelve waitResolved()
        return r;
    }

    // E-09 — El `si` que pregunta por un objeto. El anfitrión narrativo
    // no sabe mirar el inventario y delega el comando entero aquí; los
    // demás casos (bandera, variable) los resuelve él y no llegan.
    if (command.type == "control.si") {
        const Result<MapEventCondition> condicion = ConditionFromIf(command);
        if (!condicion.isOk()) {
            return Fallo(condicion.errorMessage());
        }
        if (condicion.value().kind != EventConditionKind::Item) {
            // Aquí solo llegan las que el anfitrión narrativo no supo
            // mirar. Si llega una bandera es que algo se ha delegado mal,
            // y resolverla contra un estado vacío daría siempre "no".
            return Fallo("este 'si' no es de inventario y no debería haber llegado hasta "
                         "la sesión: lo resuelve el anfitrión narrativo");
        }
        const Result<bool> valor = EvaluateItemCondition(condicion.value(), m_inventory);
        if (!valor.isOk()) {
            return Fallo(valor.errorMessage());
        }
        CommandResult r;
        r.outcome = CommandOutcome::Branch;
        r.branch = valor.value() ? 0 : 1;
        return r;
    }

    if (command.type == "control.segun_desenlace") {
        CommandResult r;
        r.outcome = CommandOutcome::Branch;
        switch (m_lastOutcome) {
            case BattleOutcome::Victory: r.branch = 0; break;
            case BattleOutcome::Fled:    r.branch = 1; break;
            case BattleOutcome::Defeat:  r.branch = 2; break;
            case BattleOutcome::InProgress:
                // Ramificar por un combate que no ha habido es un error
                // del autor, y silenciarlo daria una rama al azar.
                return Fallo("«Según desenlace» sin un combate delante");
        }
        return r;
    }

    if (command.type == "interruptor.fijar") {
        const std::string eventId = Param(command, "evento");
        const std::string letraStr = Param(command, "letra");
        if (eventId.empty() || letraStr.size() != 1) {
            return Fallo("interruptor.fijar requiere 'evento' (id) y 'letra' (A-D)");
        }
        const char letra = letraStr[0];
        const std::string valorStr = Param(command, "valor", "true");
        const bool valor = (valorStr == "true" || valorStr == "1" || valorStr == "on");
        const Result<bool> resultado = m_session.setLocalSwitch(eventId, letra, valor);
        if (!resultado.isOk()) {
            return Fallo(resultado.errorMessage());
        }
        return CommandResult{};
    }

    // E-04 (cierre) — Abre la tienda y espera a que el jugador la cierre.
    // Si no se abre, falla con el motivo de la sesión: esperar a una
    // tienda que nunca se abrió colgaría el evento.
    if (command.type == "comercio.abrir") {
        const std::string tienda = Param(command, "tienda");
        const Result<bool> abierta = m_session.openShop(tienda);
        if (!abierta.isOk()) {
            return Fallo(abierta.errorMessage());
        }
        CommandResult r;
        r.outcome = CommandOutcome::Wait;
        r.waitMs = 0;   // sin duracion conocida: la resuelve waitResolved()
        return r;
    }

    const CommandSpec* spec = FindCommand(command.type);
    return Fallo("el comando '" + (spec != nullptr ? spec->label : command.type) +
                 "' no lo sabe ejecutar la sesión de partida");
}

bool SessionCommandHost::waitResolved(const MapEventCommand& command) {
    if (command.type == "comercio.abrir") {
        // Sale de Shop al cerrar (closeInteraction). Cualquier otro modo
        // cuenta como cerrada: la tienda ya no esta en pantalla.
        return m_session.mode() != GameMode::Shop;
    }
    if (command.type != "combate.iniciar") {
        return true;
    }
    // Mientras dura el combate se va anotando el desenlace: cuando la
    // sesion lo cierra, battle() pasa a nullptr y ya no habria de donde
    // leerlo. Es el mismo motivo por el que m_lastOutcome sobrevive al
    // combate.
    const BattleState* combate = m_session.battle();
    if (combate != nullptr && combate->outcome() != BattleOutcome::InProgress) {
        m_lastOutcome = combate->outcome();
    }
    // La espera acaba cuando la sesion sale de Battle. Derrota incluida:
    // CAER-01 — si el combate no era ineludible, el heroe se levanta y el
    // evento sigue por la rama de derrota de «Según desenlace»; si lo era,
    // mode() pasa a GameOver, que es terminal, y quien lleve la pantalla de
    // derrota (G-11) es quien decide si cancela el interprete.
    return m_session.mode() != GameMode::Battle;
}

ActivePageResult ActivePageWithInventory(const MapEventDocument& event,
                                         const NarrativeState& state,
                                         const std::vector<std::string>& inventory) {
    return ActivePageWith(event, [&state, &inventory](const MapEventCondition& c) {
        return EvaluateConditionWithInventory(c, state, inventory);
    });
}

}  // namespace RPG
