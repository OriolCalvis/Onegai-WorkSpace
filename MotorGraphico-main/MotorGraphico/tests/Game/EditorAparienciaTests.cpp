// APARIENCIA-01 — El paso «Apariencia» de Nueva partida y el equipo inicial
// puesto, sin ventana (assets reales, copiados a build/ por copiar_assets).
//
//   1. Las filas dependen del pueblo: un humano elige piel y talla; un
//      aarakocra no tiene pelo ni pies; una naga no tiene piernas.
//   2. Izquierda/derecha recorren los valores con la vuelta; quitar el pelo
//      quita la fila de su color y el cursor se queda en PEINADO.
//   3. La barba sigue al pelo si iban a juego.
//   4. AL AZAR es determinista y SIEMPRE se puede montar, en las 43 razas.
//   5. COMO TU PUEBLO deshace; volver a la misma raza conserva lo elegido y
//      otra raza empieza de cero.
//   6. El equipo inicial sale puesto con la regla de Vestuario: lo primero
//      de cada ranura, si la clase puede llevarlo; el resto, a la mochila. Y
//      los requisitos cuentan: la armadura de placas pide CON 3.
//   7. El flujo tiene 14 pasos: Raza -> Apariencia -> Clase, ESC vuelve por
//      el mismo camino, la vista previa sigue a lo marcado y finalizar()
//      entrega la ficha con el equipo puesto y la vida al dia.
//   8. (PNJ-01) Las prendas de oficio de los PNJ se montan, pero la creacion
//      no las ofrece.
#include "TestHarness.h"

#include <algorithm>
#include <string>
#include <utility>

#include "Game/NewGameFlow.h"
#include "Game/Personajes/EditorApariencia.h"
#include "Game/Personajes/MontajePersonajes.h"
#include "Game/Personajes/Vestuario.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/CharacterCreation.h"
#include "RPG/TierRules.h"

namespace {

using Fila = Game::EditorApariencia::Fila;

const Game::ManifiestoPersonajes& manifiesto() {
    static const Game::ManifiestoPersonajes m = [] {
        auto r = Game::ManifiestoPersonajes::cargar("assets/textures");
        if (!r.isOk())
            throw std::runtime_error(r.errorMessage());
        return r.value();
    }();
    return m;
}

const RPG::CharacterCreation& reglas() {
    static const RPG::CharacterCreation c = [] {
        auto r = RPG::CharacterCreation::load("assets/catalogs");
        if (!r.isOk())
            throw std::runtime_error(r.errorMessage());
        return std::move(r.value());
    }();
    return c;
}

const RPG::Catalogs::EquipmentCatalog& equipo() {
    static const RPG::Catalogs::EquipmentCatalog cat = [] {
        RPG::Catalogs::EquipmentCatalog c;
        auto r = c.loadFromFile("assets/catalogs/equipment.json");
        if (!r.isOk())
            throw std::runtime_error(r.errorMessage());
        return c;
    }();
    return cat;
}

const RPG::TierRules& tiers() {
    static const RPG::TierRules t = [] {
        RPG::TierRules r;
        (void)r.loadFromFile("assets/rules/tier_rules.json");
        return r;
    }();
    return t;
}

bool tiene(const Game::EditorApariencia& e, Fila f) {
    for (Fila x : e.filas()) {
        if (x == f)
            return true;
    }
    return false;
}

void marcar(Game::EditorApariencia& e, Fila f) {
    for (std::size_t i = 0; i < e.filas().size() && e.filaMarcada() != f; ++i)
        e.abajo();
    if (e.filaMarcada() != f)
        throw std::runtime_error("fila no disponible");
}

}  // namespace

MOTOR_TEST_CASE(apariencia_filas_segun_el_pueblo) {
    Game::EditorApariencia e(manifiesto());
    e.empezar("race_human_marches");
    MOTOR_CHECK(tiene(e, Fila::Piel) && tiene(e, Fila::Talla));
    MOTOR_CHECK(tiene(e, Fila::Peinado) && tiene(e, Fila::ColorPelo));
    MOTOR_CHECK(tiene(e, Fila::Barba) && tiene(e, Fila::ColorBarba));  // barba corta de serie
    MOTOR_CHECK(tiene(e, Fila::Pies) && tiene(e, Fila::AlAzar) && tiene(e, Fila::ComoSuRaza));

    // EDITOR-PERSONALIZACION-01: el aarakocra tambien elige piel (las de su talla), peinado y barba.
    e.empezar("aarakocra_de_las_montanas");
    MOTOR_CHECK(tiene(e, Fila::Piel) && tiene(e, Fila::Peinado) && tiene(e, Fila::Barba) && !tiene(e, Fila::Talla));
    MOTOR_CHECK(!tiene(e, Fila::Pies) && tiene(e, Fila::Torso) && tiene(e, Fila::Ojos));
    // Las alas propias ahora tambien se pueden conservar, quitar o sustituir.
    MOTOR_CHECK(tiene(e, Fila::Alas) && tiene(e, Fila::Cola) && tiene(e, Fila::Cuernos) && tiene(e, Fila::Marca1));

    e.empezar("nagas_arcanos");
    MOTOR_CHECK(!tiene(e, Fila::Piernas) && !tiene(e, Fila::Pies) && tiene(e, Fila::Capa));
    // Una armadura no se le pone a una naga sin su talla sin piernas: la
    // apariencia de la raza se monta (las piezas existen en esa talla).
    MOTOR_CHECK(e.apariencia().raza == "nagas_arcanos");
}

MOTOR_TEST_CASE(apariencia_valores_con_la_vuelta) {
    Game::EditorApariencia e(manifiesto());
    e.empezar("race_human_marches");
    marcar(e, Fila::Peinado);
    const std::string antes = e.apariencia().peloEstilo;
    MOTOR_CHECK(!antes.empty());
    // Hasta «sin pelo»: la fila de color desaparece y el cursor sigue en PEINADO.
    for (int i = 0; i < 40 && !e.apariencia().peloEstilo.empty(); ++i)  // PL-ROPA-01: 27 peinados
        e.izquierda();
    MOTOR_CHECK(e.apariencia().peloEstilo.empty());
    MOTOR_CHECK(!tiene(e, Fila::ColorPelo));
    MOTOR_CHECK(e.filaMarcada() == Fila::Peinado);
    MOTOR_CHECK(e.valor(Fila::Peinado) == "Sin pelo");
    // Desde «sin pelo», izquierda da la vuelta al ultimo estilo.
    e.izquierda();
    MOTOR_CHECK(e.apariencia().peloEstilo == "coleta");
    MOTOR_CHECK(tiene(e, Fila::ColorPelo));
    e.derecha();
    e.derecha();
    MOTOR_CHECK(e.apariencia().peloEstilo == "corto");
    MOTOR_CHECK(e.valor(Fila::Peinado) == "Corto");

    // La ropa: «Nada» y las piezas con su nombre.
    marcar(e, Fila::Cabeza);
    e.derecha();
    // DROWS-01: en la cabeza, solo lo que va por debajo de un gorro o un casco.
    MOTOR_CHECK(e.apariencia().ropa.at("cabeza") == "panuelo_rojo");
    MOTOR_CHECK(e.valor(Fila::Cabeza) == "Panuelo rojo");
    e.izquierda();
    MOTOR_CHECK(e.apariencia().ropa.count("cabeza") == 0);
    MOTOR_CHECK(e.valor(Fila::Cabeza) == "Nada");
}

MOTOR_TEST_CASE(apariencia_la_barba_sigue_al_pelo) {
    Game::EditorApariencia e(manifiesto());
    e.empezar("race_human_marches");  // pelo y barba oscuros
    MOTOR_CHECK(e.apariencia().barbaColor == e.apariencia().peloColor);
    marcar(e, Fila::ColorPelo);
    e.derecha();
    MOTOR_CHECK(e.apariencia().peloColor != "pelo_oscuro");
    MOTOR_CHECK(e.apariencia().barbaColor == e.apariencia().peloColor);
    // Con colores distintos, cada uno va por su lado.
    marcar(e, Fila::ColorBarba);
    e.derecha();
    const std::string barba = e.apariencia().barbaColor;
    marcar(e, Fila::ColorPelo);
    e.derecha();
    MOTOR_CHECK(e.apariencia().barbaColor == barba);
}

MOTOR_TEST_CASE(apariencia_al_azar_siempre_se_monta) {
    Game::MontajePersonajes montaje(manifiesto(), "assets/textures");
    int montadas = 0;
    for (const Game::RazaVisual& raza : manifiesto().razas) {
        Game::EditorApariencia a(manifiesto());
        Game::EditorApariencia b(manifiesto());
        a.empezar(raza.id);
        b.empezar(raza.id);
        marcar(a, Fila::AlAzar);
        marcar(b, Fila::AlAzar);
        for (int i = 0; i < 12; ++i) {
            a.derecha();
            b.izquierda();
            MOTOR_CHECK(a.apariencia().clave() == b.apariencia().clave());  // determinista
            MOTOR_CHECK(a.filaMarcada() == Fila::AlAzar);
            auto capas = montaje.capas(a.apariencia());
            if (!capas.isOk())
                MOTOR_CHECK_FAIL(raza.id + ": " + capas.errorMessage());
            ++montadas;
        }
    }
    MOTOR_CHECK(montadas == 72 * 12);  // DIALOGOS-01: + el drow de las sombras y la saga de los vientos; + 27 del 2000 b.f.
    // Y montar de verdad una tanda (lee los atlas).
    Game::EditorApariencia e(manifiesto());
    e.empezar("race_human_marches");
    marcar(e, Fila::AlAzar);
    for (int i = 0; i < 5; ++i) {
        e.derecha();
        const auto montada = montaje.montar(e.apariencia());
        if (!montada.isOk())
            MOTOR_CHECK_FAIL(e.apariencia().clave() + ": " + montada.errorMessage());
    }
}

MOTOR_TEST_CASE(apariencia_sin_prendas_de_oficio) {
    // PNJ-01: las prendas que solo llevan los PNJ (la librea de la guardia, el
    // mandil, la casulla, la toga, el birrete...) se montan como las demas,
    // pero la creacion no las ofrece: ni recorriendo las filas ni AL AZAR.
    const auto deOficio = [](const std::string& ranura, const std::string& id) {
        const auto grupo = manifiesto().equipo.find(ranura);
        const Game::FilaDeCapa* f = grupo == manifiesto().equipo.end() ? nullptr : grupo->second.buscar(id);
        return f != nullptr && f->soloPnj;
    };
    MOTOR_CHECK(deOficio("torso", "librea_guardia") && deOficio("cabeza", "birrete"));
    // AV16-03: la ropa que no encaja en el cuerpo (tools/auditar_ropa.py) se monta, pero no se ofrece.
    auto noEncaja = [&](const char* ranura, const std::string& id) {
        const auto grupo = manifiesto().equipo.find(ranura);
        const Game::FilaDeCapa* f = grupo == manifiesto().equipo.end() ? nullptr : grupo->second.buscar(id);
        return f != nullptr && f->noEncaja;
    };
    // PL-JUEGO-01: la de la captura del 15/10 (tunica_mago_pl, que se salia del cuerpo) era una prenda de
    // PIXELLAB-L7, de cuatro vistas, y con los personajes de PixelLab no esta en los atlas (cada prenda se
    // dibuja sobre la familia de su talla). La regla sigue: lo que marque la lista no se ofrece.
    MOTOR_CHECK(manifiesto().equipo.at("torso").buscar("tunica_mago_pl") == nullptr);
    Game::EditorApariencia e(manifiesto());
    e.empezar("race_human_marches");
    const std::pair<Fila, const char*> ropa[] = {
        {Fila::Cabeza, "cabeza"}, {Fila::Torso, "torso"}, {Fila::Piernas, "piernas"},
        {Fila::Pies, "pies"}, {Fila::Capa, "capa"}};
    int vistas = 0;
    for (const auto& [fila, ranura] : ropa) {
        marcar(e, fila);
        for (int i = 0; i < 150; ++i) {  // mas vueltas que piezas: las ve todas
            e.derecha();
            const auto it = e.apariencia().ropa.find(ranura);
            if (it == e.apariencia().ropa.end())
                continue;
            MOTOR_CHECK(!deOficio(ranura, it->second));
            MOTOR_CHECK(!noEncaja(ranura, it->second));
            // DROWS-01: ni sombreros, ni capuchas, ni cascos (esos, con el equipo).
            if (std::string(ranura) == "cabeza")
                MOTOR_CHECK(it->second.rfind("panuelo", 0) == 0 || it->second.rfind("corona_", 0) == 0 ||
                            it->second == "cofia");
            ++vistas;
        }
    }
    MOTOR_CHECK(vistas > 100);
    marcar(e, Fila::AlAzar);
    for (int i = 0; i < 300; ++i) {
        e.derecha();
        for (const auto& [ranura, pieza] : e.apariencia().ropa) {
            MOTOR_CHECK(!deOficio(ranura, pieza));
            MOTOR_CHECK(!noEncaja(ranura.c_str(), pieza));
        }
    }
}

MOTOR_TEST_CASE(apariencia_como_tu_pueblo_y_cambio_de_raza) {
    Game::EditorApariencia e(manifiesto());
    e.empezar("enanos_forjadores");
    const std::string original = e.apariencia().clave();
    marcar(e, Fila::Piel);
    e.derecha();
    marcar(e, Fila::Torso);
    e.derecha();
    MOTOR_CHECK(e.apariencia().clave() != original);
    const std::string editada = e.apariencia().clave();
    e.empezar("enanos_forjadores");  // misma raza: se conserva
    MOTOR_CHECK(e.apariencia().clave() == editada);
    marcar(e, Fila::ComoSuRaza);
    e.derecha();
    MOTOR_CHECK(e.apariencia().clave() == original);
    e.empezar("elfos_del_bosque");  // otra raza: de cero
    MOTOR_CHECK(e.apariencia().raza == "elfos_del_bosque");
    MOTOR_CHECK(e.apariencia().clave() ==
                Game::Apariencia::deRaza(*manifiesto().raza("elfos_del_bosque")).clave());
    // Las armas de la raza no estan: se ven cuando se equipan.
    MOTOR_CHECK(e.apariencia().ropa.count("arma") == 0);
}

MOTOR_TEST_CASE(apariencia_equipo_inicial_puesto) {
    const RPG::ClassDefinition* caballero = reglas().classes().find("caballero_templario");
    MOTOR_CHECK(caballero != nullptr);
    RPG::CharacterSheet ficha;
    ficha.tier = 1;
    ficha.classId = caballero->id;
    ficha.baseStats = {2, 1, 1, 1};
    for (const std::string& id : reglas().equipoInicial(caballero->id, "")) {
        ficha.inventoryEquipment.emplace_back(id, 1);
    }
    const auto puestos = Game::Vestuario::equiparInicial(ficha, *caballero, tiers(), equipo());
    // Espada y cota puestas; el escudo es MainHand en el canon y se queda.
    MOTOR_CHECK(ficha.equippedIds[static_cast<std::size_t>(RPG::EquipSlot::MainHand)] ==
                "item_short_sword");
    MOTOR_CHECK(ficha.equippedIds[static_cast<std::size_t>(RPG::EquipSlot::Torso)] ==
                "cota_de_malla");
    MOTOR_CHECK(puestos.size() == 2);
    MOTOR_CHECK(ficha.inventoryEquipment.size() == 1);
    MOTOR_CHECK(ficha.inventoryEquipment[0].first == "item_reinforced_shield");
    const auto visibles = Game::Vestuario::piezasVisibles(ficha.equippedIds, manifiesto());
    // ESTETICA-01: la espada corta tiene su forma (f_espada_corta); es comun, sin tinte.
    MOTOR_CHECK(visibles.at("arma") == "f_espada_corta");
    MOTOR_CHECK(visibles.at("torso") == "cota_malla");

    // La armadura de placas pide CON 3: con CON 1 se queda en la mochila.
    RPG::CreationChoice eleccion;
    eleccion.raceId = "race_human_marches";
    eleccion.classId = "coloso_de_ceniza";
    eleccion.tier = 1;
    eleccion.baseStats = {1, 1, 1, 1};
    auto sinFuerza = Game::Vestuario::probar(reglas(), eleccion, tiers(), equipo());
    MOTOR_CHECK(sinFuerza[static_cast<std::size_t>(RPG::EquipSlot::Torso)].empty());
    MOTOR_CHECK(!sinFuerza[static_cast<std::size_t>(RPG::EquipSlot::MainHand)].empty());
    eleccion.baseStats = {3, 1, 1, 1};
    auto conFuerza = Game::Vestuario::probar(reglas(), eleccion, tiers(), equipo());
    MOTOR_CHECK(conFuerza[static_cast<std::size_t>(RPG::EquipSlot::Torso)] == "armadura_de_placas");
    // Sin clase no hay nada que probar.
    eleccion.classId.clear();
    auto nada = Game::Vestuario::probar(reglas(), eleccion, tiers(), equipo());
    for (const auto& id : nada)
        MOTOR_CHECK(id.empty());
}

MOTOR_TEST_CASE(apariencia_flujo_de_14_pasos) {
    using Paso = RPG::CharacterCreationScreen::Paso;
    Game::NewGameFlow f(reglas(), &manifiesto());
    MOTOR_CHECK(f.totalDePasos() == 14);
    MOTOR_CHECK(f.numeroDePaso() == 1);

    // En Raza la vista previa sigue a la raza marcada.
    f.moveDown();
    f.moveDown();
    const auto marcada = f.pantalla().opciones()[f.pantalla().marcado()];
    MOTOR_CHECK(f.vistaPrevia().apariencia.raza == marcada);
    MOTOR_CHECK(!f.vistaPrevia().conEquipo);

    f.confirm();  // Raza -> Apariencia
    MOTOR_CHECK(f.enApariencia());
    MOTOR_CHECK(f.numeroDePaso() == 2);
    MOTOR_CHECK(std::string(f.tituloPaso()) == "QUÉ ASPECTO TIENES");
    MOTOR_CHECK(f.editorApariencia()->apariencia().raza == marcada);
    const std::string deSerie = f.editorApariencia()->apariencia().clave();
    f.moveDown();
    f.adjustRight();
    MOTOR_CHECK(f.editorApariencia()->apariencia().clave() != deSerie);
    const std::string editada = f.editorApariencia()->apariencia().clave();
    f.textChar('x');  // en Apariencia no se escribe en ningun filtro
    MOTOR_CHECK(f.pantalla().filtro().empty());

    f.cancel();  // Apariencia -> Raza
    MOTOR_CHECK(!f.enApariencia() && f.pantalla().paso() == Paso::Raza && f.numeroDePaso() == 1);
    for (std::size_t i = 0; i < f.pantalla().opciones().size() &&
                            f.pantalla().opciones()[f.pantalla().marcado()] != marcada;
         ++i) {
        f.moveDown();
    }
    f.confirm();  // la misma raza: lo editado sigue
    MOTOR_CHECK(f.enApariencia() && f.editorApariencia()->apariencia().clave() == editada);
    f.confirm();  // Apariencia -> Clase
    MOTOR_CHECK(!f.enApariencia() && f.pantalla().paso() == Paso::Clase && f.numeroDePaso() == 3);
    MOTOR_CHECK(f.vistaPrevia().conEquipo);
    MOTOR_CHECK(f.vistaPrevia().eleccion.classId ==
                f.pantalla().opciones()[f.pantalla().marcado()]);
    MOTOR_CHECK(f.vistaPrevia().apariencia.clave() == editada);
    f.cancel();  // Clase -> Apariencia
    MOTOR_CHECK(f.enApariencia() && f.numeroDePaso() == 2);
    f.confirm();

    // Una clase con equipo y el resto de la creacion.
    const auto& clases = f.pantalla().opciones();
    std::size_t objetivo = 0;
    for (std::size_t i = 0; i < clases.size(); ++i) {
        if (clases[i] == "bardo_de_las_cuatro_estaciones")  // DIALOGOS-01: una de las 70 que se juegan
            objetivo = i;
    }
    for (std::size_t i = 0; i < objetivo; ++i)
        f.moveDown();
    f.confirm();  // Clase
    f.confirm();  // Trasfondo (el primero)
    // H2-06: sin reparto, tras el trasfondo viene el vinculo.
    MOTOR_CHECK(f.pantalla().paso() == Paso::Vinculo && f.numeroDePaso() == 5);
    for (int i = 0; i < 8; ++i)
        f.confirm();  // narrativas + deidad
    for (char c : std::string("Nerea"))
        f.textChar(c);
    f.confirm();
    MOTOR_CHECK(f.pantalla().paso() == Paso::Resumen && f.numeroDePaso() == 14);
    f.confirm();
    MOTOR_CHECK(f.estado() == Game::NewGameFlow::Estado::Terminado);

    auto ficha = f.finalizar(tiers(), &equipo());
    MOTOR_CHECK(ficha.isOk());
    MOTOR_CHECK(ficha.value().equippedIds[static_cast<std::size_t>(RPG::EquipSlot::MainHand)] ==
                "sable_ligero_de_viaje");
    MOTOR_CHECK(ficha.value().equippedIds[static_cast<std::size_t>(RPG::EquipSlot::Torso)] ==
                "armadura_de_cuero");
    MOTOR_CHECK(ficha.value().healthCap() > 0);
    // Sin catalogo, todo en la mochila, como antes.
    auto enLaMochila = f.finalizar(tiers());
    MOTOR_CHECK(enLaMochila.isOk());
    for (const auto& id : enLaMochila.value().equippedIds)
        MOTOR_CHECK(id.empty());
}

MOTOR_TEST_CASE(apariencia_sin_manifiesto_el_flujo_de_siempre) {
    Game::NewGameFlow f(reglas());
    MOTOR_CHECK(f.totalDePasos() == 13);
    MOTOR_CHECK(f.editorApariencia() == nullptr);
    f.confirm();
    MOTOR_CHECK(!f.enApariencia());
    MOTOR_CHECK(f.pantalla().paso() == RPG::CharacterCreationScreen::Paso::Clase);
    MOTOR_CHECK(f.numeroDePaso() == 2);
    MOTOR_CHECK(f.vistaPrevia().apariencia.raza.empty());
}

int main() { return motor_test::runAllTests(); }

// EDITOR-PERSONALIZACION-01 (Oriol, 26/10): piel de cualquier raza, cualquier color de pelo, marcas y rasgos.
MOTOR_TEST_CASE(apariencia_personalizacion_piel_marcas_y_rasgos) {
    Game::EditorApariencia e(manifiesto());
    e.empezar("draconidos_de_fuego");
    MOTOR_CHECK(tiene(e, Fila::Piel) && !tiene(e, Fila::Talla));
    MOTOR_CHECK(e.valor(Fila::Piel) == "La de tu pueblo");
    marcar(e, Fila::Piel);
    e.derecha();
    MOTOR_CHECK(!e.apariencia().piel.empty() && e.apariencia().piel != "draconidos_de_fuego");
    MOTOR_CHECK(e.valor(Fila::Piel) != "La de tu pueblo");
    e.izquierda();
    MOTOR_CHECK(e.valor(Fila::Piel) == "La de tu pueblo");

    // Cualquier color en cualquier peinado: la lista de colores trae todos los materiales de pelo.
    marcar(e, Fila::Peinado);
    while (e.apariencia().peloEstilo != "afro")
        e.derecha();
    marcar(e, Fila::ColorPelo);
    std::vector<std::string> colores;
    const std::string primero = e.apariencia().peloColor;
    do {
        colores.push_back(e.apariencia().peloColor);
        e.derecha();
    } while (e.apariencia().peloColor != primero && colores.size() < 50);
    MOTOR_CHECK(colores.size() >= 11);
    MOTOR_CHECK(std::find(colores.begin(), colores.end(), "pelo_llama") != colores.end());

    // Marcas: la segunda fila aparece al poner la primera; quitar la primera corre la segunda.
    marcar(e, Fila::Marca1);
    MOTOR_CHECK(!tiene(e, Fila::Marca2) && e.valor(Fila::Marca1) == "Ninguna");
    e.derecha();
    MOTOR_CHECK(e.apariencia().marcas.size() == 1 && tiene(e, Fila::Marca2));
    marcar(e, Fila::Marca2);
    e.derecha();
    e.derecha();
    MOTOR_CHECK(e.apariencia().marcas.size() == 2 && tiene(e, Fila::Marca3));
    const std::string segunda = e.apariencia().marcas[1];
    marcar(e, Fila::Marca1);
    e.izquierda();
    MOTOR_CHECK(e.apariencia().marcas.size() == 1 && e.apariencia().marcas[0] == segunda);

    // Un dracónido tiene cola propia: no se le ofrece otra. A un humano, cola y cuernos van en la ropa
    // (ranuras propias) y tienen nombre.
    MOTOR_CHECK(tiene(e, Fila::Cola) && tiene(e, Fila::Cuernos));
    e.empezar("race_human_marches");
    marcar(e, Fila::Cola);
    e.derecha();
    MOTOR_CHECK(e.apariencia().ropa.count("cola") == 1 && e.valor(Fila::Cola) != "Ninguna");
    marcar(e, Fila::Cuernos);
    e.derecha();
    MOTOR_CHECK(e.apariencia().ropa.count("cuernos") == 1);
    // Todo eso se monta.
    auto m = Game::MontajePersonajes::cargar("assets/textures");
    MOTOR_CHECK(m.isOk());
    if (m.isOk())
        MOTOR_CHECK(m.value()->capas(e.apariencia()).isOk());
}
