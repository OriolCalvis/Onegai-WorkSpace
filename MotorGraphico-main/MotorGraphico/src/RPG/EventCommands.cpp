// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-04  (fase E, 7 j) - El joc de comandes
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "RPG/EventCommand.h"
#include "RPG/EventosInterno.h"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

// E-04 — Implementación del juego de comandas. Ver la nota larga del
// header: aquí solo el cómo.

namespace RPG {

// EST-32 — las dos comprobaciones de texto que comparten el catalogo, la
// validacion y las condiciones viven en RPG/EventosInterno.h desde que
// este fichero se partio en cuatro.
using namespace RPG::interno;

namespace {


CommandParamSpec P(const char* key, CommandParamType type, bool required, const char* help) {
    CommandParamSpec spec;
    spec.key = key;
    spec.type = type;
    spec.required = required;
    spec.help = help;
    return spec;
}

// El catálogo. Cinco familias, que son las de la especificación §12:
// texto y elecciones, estado narrativo, inventario y economía, control de
// flujo, y mundo (mapa, combate, comercio).
//
// El orden es estable a propósito: es el orden en el que el editor los
// va a listar, y cambiarlo mueve las cosas de sitio bajo el ratón del
// autor sin motivo.
std::vector<CommandSpec> ConstruirCatalogo() {
    std::vector<CommandSpec> c;

    // --- Texto ---
    c.push_back({"texto.mostrar", "Mostrar texto",
                 "Enseña un texto y espera a que el jugador lo confirme.",
                 {P("texto", CommandParamType::Text, true, "Lo que se lee"),
                  P("hablante", CommandParamType::Text, false, "Quién lo dice")},
                 0, true});

    // --- Estado narrativo: lo que este anfitrión sabe hacer entero ---
    c.push_back({"bandera.encender", "Activar bandera",
                 "Enciende una bandera de la historia.",
                 {P("bandera", CommandParamType::FlagRef, true, "Qué bandera")},
                 0, true});
    c.push_back({"bandera.apagar", "Desactivar bandera",
                 "Apaga una bandera de la historia.",
                 {P("bandera", CommandParamType::FlagRef, true, "Qué bandera")},
                 0, true});
    c.push_back({"variable.fijar", "Fijar variable",
                 "Le da un valor a una variable numérica.",
                 {P("variable", CommandParamType::VariableRef, true, "Qué variable"),
                  P("valor", CommandParamType::Int, true, "Valor")},
                 0, true});
    c.push_back({"variable.sumar", "Sumar a variable",
                 "Suma (o resta, con un valor negativo) a una variable.",
                 {P("variable", CommandParamType::VariableRef, true, "Qué variable"),
                  P("valor", CommandParamType::Int, true, "Cuánto")},
                 0, true});
    c.push_back({"interruptor.fijar", "Fijar interruptor local",
                 "Enciende o apaga uno de los interruptores A-D de un evento.",
                 {P("evento", CommandParamType::EventRef, true, "Qué evento"),
                  P("letra", CommandParamType::SwitchLetter, true, "A, B, C o D"),
                  P("valor", CommandParamType::Bool, true, "Encendido o apagado")},
                 0, true});

    // --- Control de flujo ---
    c.push_back({"control.esperar", "Esperar",
                 "Cede el turno durante un rato antes de seguir.",
                 {P("ms", CommandParamType::DurationMs, true, "Milisegundos")},
                 0, true});
    c.push_back({"control.si", "Si / si no",
                 "Ejecuta la primera rama si la condición se cumple, y la "
                 "segunda si no. La segunda es opcional.",
                 {P("clase", CommandParamType::Text, true, "flag, variable o item"),
                  P("referencia", CommandParamType::Text, true, "Nombre de lo que se mira"),
                  P("comparacion", CommandParamType::Comparison, false, "== por defecto"),
                  P("valor", CommandParamType::Text, false, "Con qué se compara"),
                  P("negada", CommandParamType::Bool, false, "Invierte el resultado")},
                 2, true});

    // --- Familias de partida (segunda rebanada, RPG/SessionCommandHost).
    // Marcadas como implementadas: funcionan cuando el juego corre. En
    // una vista previa sin sesion, NarrativeCommandHost las delega en su
    // fallback y sin fallback dice que le falta la sesion.
    c.push_back({"inventario.dar", "Dar objeto",
                 "Añade un objeto al inventario del jugador.",
                 {P("objeto", CommandParamType::ObjectRef, true, "Qué objeto"),
                  P("cantidad", CommandParamType::Int, false, "1 por defecto")},
                 0, true});
    c.push_back({"inventario.quitar", "Quitar objeto",
                 "Quita un objeto del inventario del jugador.",
                 {P("objeto", CommandParamType::ObjectRef, true, "Qué objeto"),
                  P("cantidad", CommandParamType::Int, false, "1 por defecto")},
                 0, true});
    c.push_back({"economia.oro", "Dar o quitar oro",
                 "Suma oro a la partida; en negativo, lo cobra.",
                 {P("valor", CommandParamType::Int, true, "Cuánto")},
                 0, true});
    c.push_back({"mapa.ir", "Ir a otro mapa",
                 "Lleva al jugador a otro mapa. TERMINA el evento: lo que "
                 "pongas debajo no se ejecuta (DEC-30).",
                 {P("mapa", CommandParamType::MapRef, true, "Mapa de destino"),
                  P("entrada", CommandParamType::Text, false, "Por dónde llega")},
                 0, true});
    // --- Combate. Cableadas en la tercera rebanada (SessionCommandHost):
    // las dos cambian el mode() de la sesion y esperan un desenlace, asi
    // que solo funcionan con partida; NarrativeCommandHost sin fallback
    // dice que le falta la sesion en vez de fingir que las hace.
    //
    // `comercio.abrir`, al final del bloque, se cableo en el cierre de E-04
    // (GameSession::openShop). Desde entonces ningun comando del catalogo
    // tiene implemented=false; el mecanismo se conserva para el proximo
    // que se declare antes de cablearse.
    c.push_back({"combate.iniciar", "Iniciar combate",
                 "Lanza un encuentro y ESPERA a que termine. El desenlace no se "
                 "ramifica aquí: para eso está «Según desenlace», justo debajo.",
                 {P("encuentro", CommandParamType::Text, true, "Qué enemigo"),
                  P("flag_victoria", CommandParamType::FlagRef, false, "Bandera al ganar"),
                  P("flag_derrota", CommandParamType::FlagRef, false, "Bandera al perder"),
                  // CAER-01 — Huir y caer, segun la mision (como en startBattle).
                  P("flag_huida", CommandParamType::FlagRef, false, "Bandera al huir (vacía: la de perder)"),
                  P("ineludible", CommandParamType::Bool, false, "Sin huida, y caer acaba la partida")},
                 0, true});
    // Dos comandos y no uno, como el diseno de bloques (§6, B09 y B14).
    // Separarlos es lo que permite que el autor meta cosas ENTRE el
    // combate y la ramificacion, y que un combate sin ramas siga siendo
    // legal.
    c.push_back({"control.segun_desenlace", "Según desenlace del combate",
                 "Ramifica segun como acabo el ultimo combate: victoria, huida "
                 "o derrota, en ese orden.",
                 {},
                 3, true});
    c.push_back({"comercio.abrir", "Abrir tienda",
                 "Abre la tienda de un personaje y espera a que el jugador la cierre.",
                 {P("tienda", CommandParamType::ObjectRef, true, "El personaje que vende")},
                 0, true});
    return c;
}

}  // namespace

const std::vector<CommandSpec>& CommandCatalog() {
    static const std::vector<CommandSpec> catalogo = ConstruirCatalogo();
    return catalogo;
}

const CommandSpec* FindCommand(const std::string& type) {
    for (const CommandSpec& spec : CommandCatalog()) {
        if (spec.type == type) {
            return &spec;
        }
    }
    return nullptr;
}


// ---------------------------------------------------------------------
// NarrativeCommandHost
// ---------------------------------------------------------------------


}  // namespace RPG
