// H2-06 — Las cartas de las cuatro clases hacen en combate lo que dice su
// texto (Game::CartasEnCombate sobre BattleState).
//
// Los catalogos son los reales (assets/catalogs): la carta publicada con sus
// ganchos. El dado es fijo, todas las caras iguales, y las cuentas se pueden
// hacer a mano (Ed. 3: 6 = 1 exito, 5 = medio; con ventaja el 4 tambien vale
// medio; con desventaja solo cuenta el 6):
//   heroe  CON 2, DES 5 -> ataca con 6 dados, defiende con 5;
//   lobo   CON 1, DES 5 -> ataca con 6 dados, defiende con 5; 60 PV.
// DANO-02: el dano son tantos d6 como la diferencia de exitos (medio exito
// cuenta como un dado). Con el dado en 5 los dos aciertan por medio exito
// (3 contra 2,5): 1d6 = 5. Las cartas de magia se salvan con su atributo
// (el lobo tiene CON 1 y CAR 3: 1 y 3 dados contra los 6 del heroe).
#include "CartasPrueba.h"
#include "RPG/CharacterCreation.h"
#include "RPG/DanoEnfrentado.h"
#include "Game/BarraCombate.h"
#include "TestHarness.h"

#include <set>

#include <string>

MOTOR_TEST_CASE(los_ganchos_publicados_se_entienden_todos) {
    // Ningun gancho de cartas, pasivas o condiciones queda «desconocido»:
    // si alguien escribe uno nuevo sin ensenarselo al motor, falla aqui.
    RPG::Catalogs::SkillCatalog skills;
    RPG::Catalogs::PassiveCatalog pasivas;
    RPG::Catalogs::ConditionCatalog condiciones;
    MOTOR_CHECK(skills.loadFromFile("assets/catalogs/skills.json").isOk());
    MOTOR_CHECK(pasivas.loadFromFile("assets/catalogs/passives.json").isOk());
    MOTOR_CHECK(condiciones.loadFromFile("assets/catalogs/conditions.json").isOk());
    int ganchos = 0;
    const auto revisa = [&](const std::vector<std::string>& crudos) {
        for (const Game::Gancho& g : Game::leerGanchos(crudos)) {
            ++ganchos;
            MOTOR_CHECK(g.tipo != T::Desconocido);
        }
    };
    skills.forEach([&](const RPG::SkillDefinition& d) { revisa(d.mechanicHooks); });
    pasivas.forEach([&](const RPG::PassiveDefinition& d) { revisa(d.mechanicHooks); });
    condiciones.forEach([&](const RPG::ConditionDefinition& d) { revisa(d.mechanicHooks); });
    MOTOR_CHECK(ganchos > 60);
}

MOTOR_TEST_CASE(las_cuarenta_cartas_tienen_ganchos_y_solo_lo_pendiente_se_bloquea) {
    const char* const clasesConCombate[] = {"bardo_de_las_cuatro_estaciones", "el_apostata",
                                            "senor_de_las_armas", "furia_de_los_creadores"};
    int bloqueadas = 0;
    for (const char* id : clasesConCombate) {
        for (const std::string& s : clases().find(id)->learnableSkillIds) {
            const Game::CatalogoCartas::Carta* c = catalogo().carta(s);
            MOTOR_CHECK(c != nullptr && !c->ganchos.empty());
            if (!c->bloqueo.empty()) ++bloqueadas;
        }
    }
    // Las dos que faltan del Senor de las Armas, Susurro Demoniaco y Ataque
    // Maldito ([pendiente]) y Paso Ilusorio (necesita posiciones).
    MOTOR_CHECK(bloqueadas == 5);
    MOTOR_CHECK(catalogo().carta("paso_ilusorio")->bloqueo == "necesita posiciones en el combate");
    // La pasiva del Bardo da a elegir entre cuatro estaciones.
    int opciones = 0;
    for (const Game::Gancho& g : catalogo().ganchosDePasiva("las_cuatro_estaciones")) {
        if (g.tipo == T::EligeUna) opciones = static_cast<int>(g.opciones.size());
    }
    MOTOR_CHECK(opciones == 4);
}

MOTOR_TEST_CASE(la_ficha_creada_conoce_todas_las_cartas_de_su_clase) {
    auto creacion = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(creacion.isOk());
    RPG::CreationChoice e;
    e.displayName = "Ilse";
    e.raceId = "elfos_del_bosque";
    e.classId = "bardo_de_las_cuatro_estaciones";
    // Primer trasfondo con opciones, como haria la pantalla.
    creacion.value().backgrounds().forEach([&](const RPG::BackgroundDefinition& b) {
        if (e.backgroundId.empty() && creacion.value().opcionesDe(b.id) != nullptr) e.backgroundId = b.id;
    });
    const auto* op = creacion.value().opcionesDe(e.backgroundId);
    MOTOR_CHECK(op != nullptr);
    e.bondId = op->bonds.front().id;
    e.fearId = op->fears.front().id;
    e.flawId = op->flaws.front().id;
    e.goalId = op->goals.front().id;
    e.idealId = op->ideals.front().id;
    e.personalityId = op->personalities.front().id;
    e.virtueId = op->virtues.front().id;
    e.baseStats = {4, 4, 4, 4};
    auto ficha = creacion.value().construir(e);
    MOTOR_CHECK(ficha.isOk());
    for (const std::string& id : clases().find(e.classId)->learnableSkillIds) {
        bool la = false;
        for (const std::string& k : ficha.value().knownSkillIds) la |= k == id;
        MOTOR_CHECK(la);
    }
}

// AV13-35 — Ya no hay combate sin mesa: sin ficha (la prueba de nivel del editor), la mesa solo
// tiene las basicas.
MOTOR_TEST_CASE(sin_ficha_la_mesa_solo_tiene_las_basicas) {
    Luchador sinFicha{30, 2, 5};
    const auto mesa = Game::CartasEnCombate::paraHeroe(catalogo(), sinFicha);
    MOTOR_CHECK(mesa != nullptr);
    if (mesa == nullptr) return;
    const Game::VistaBarra v = Game::vistaBarra(*mesa, 0);
    MOTOR_CHECK(v.mano.empty() && !v.basicas.empty());
}

MOTOR_TEST_CASE(bardo_cancion_del_otono_da_desventaja_dos_turnos) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    MOTOR_CHECK(m.jugar("Canción del Otoño"));
    MOTOR_CHECK(m.carta("cancion_del_otono").gastada);
    m.ronda();  // el lobo tira con desventaja: sus 5 no cuentan, 0 contra 2,5
    MOTOR_CHECK(m.heroe.health() == 30);
    m.atacar();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 30);
    m.atacar();
    m.ronda();  // la cancion se acabo: empate, parcial
    MOTOR_CHECK(m.heroe.health() == 25);
}

// PART-02d — La carta jugada queda apuntada para verla (sus particulas sobre a quien va): quien la juega,
// sobre quien y con que etiquetas. Elegir una estacion no es jugar una carta.
MOTOR_TEST_CASE(jugar_una_carta_la_apunta_para_verla) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    MOTOR_CHECK(m.cartas->cartasJugadas().empty());
    MOTOR_CHECK(m.jugar("Elegir Verano"));
    MOTOR_CHECK(m.cartas->cartasJugadas().empty());
    MOTOR_CHECK(m.jugar("Canción del Otoño"));
    MOTOR_CHECK(m.cartas->cartasJugadas().size() == 1);
    const Game::CartasEnCombate::CartaJugada& j = m.cartas->cartasJugadas().back();
    MOTOR_CHECK(j.numero == 1 && j.carta == "cancion_del_otono");
    MOTOR_CHECK(j.quien.aliado && j.quien.i == 0);
    MOTOR_CHECK(j.sobre.size() == 1 && !j.sobre[0].aliado && j.sobre[0].i == 0);  // el lobo
    m.ronda();
    MOTOR_CHECK(m.jugar("Canción del Invierno"));
    MOTOR_CHECK(m.cartas->cartasJugadas().size() == 2 && m.cartas->cartasJugadas().back().numero == 2);
}

MOTOR_TEST_CASE(bardo_cancion_del_invierno_deja_fuera_de_combate_dos_turnos) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    MOTOR_CHECK(m.jugar("Canción del Invierno"));
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 30);
    m.atacar();  // defensa del lobo con desventaja: 3 contra 0, 3d6
    MOTOR_CHECK(m.lobo.health() == 45);
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 30);
    m.atacar();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 25);
}

MOTOR_TEST_CASE(bardo_verano_da_ventaja_en_ataque) {
    Mesa sin("bardo_de_las_cuatro_estaciones");
    sin.dado.cara = 4;
    sin.atacar();  // 0 contra 0: empate, roza (D0014): 1
    MOTOR_CHECK(sin.lobo.health() == 59);

    Mesa m("bardo_de_las_cuatro_estaciones");
    m.dado.cara = 4;
    MOTOR_CHECK(m.fila("Elegir Primavera") == 0);
    MOTOR_CHECK(m.jugar("Elegir Verano"));
    MOTOR_CHECK(m.cartas->turnoAbierto());  // elegir no gasta la accion
    MOTOR_CHECK(m.fila("Elegir") < 0);
    m.atacar();  // con ventaja el 4 vale medio: 3 contra 0, 3d6 (de 4)
    MOTOR_CHECK(m.lobo.health() == 48);
}

MOTOR_TEST_CASE(bardo_invierno_refleja_el_primer_turno_y_da_mas_defensa) {
    Mesa m("bardo_de_las_cuatro_estaciones", 1, 8);  // lobo con DES 8: 9 dados
    MOTOR_CHECK(m.jugar("Elegir Invierno"));
    m.atacar();
    m.ronda();  // 4,5 contra 3,5 (5 dados + 2 de CA): 1d6, pero se refleja
    MOTOR_CHECK(m.heroe.health() == 30);
    MOTOR_CHECK(m.lobo.health() == 55);  // el heroe (3 contra 4,5) no entra
    m.atacar();
    m.ronda();  // segundo turno: ya no refleja
    MOTOR_CHECK(m.heroe.health() == 25);
}

MOTOR_TEST_CASE(bardo_invierno_suma_dos_dados_a_la_defensa_fisica) {
    // Lobo normal: 3 contra 5 dados (2,5) entraria; con +2 de CA el heroe
    // defiende con 7 (3,5) y el lobo falla los dos turnos.
    Mesa m("bardo_de_las_cuatro_estaciones");
    MOTOR_CHECK(m.jugar("Elegir Invierno"));
    m.atacar();
    m.ronda();
    m.atacar();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 30);
    MOTOR_CHECK(m.lobo.health() == 50);  // solo los dos golpes del heroe
}

MOTOR_TEST_CASE(un_enemigo_que_gana_la_tirada_resiste_la_cancion) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    m.dado.cara = 4;  // 0 contra 0: empate, parcial; el estado pide exito
    MOTOR_CHECK(m.jugar("Canción del Otoño"));
    MOTOR_CHECK(m.situLobo().estados.empty());
    MOTOR_CHECK(m.carta("cancion_del_otono").gastada);  // la carta se juega igual
}

MOTOR_TEST_CASE(bardo_bendicion_de_las_estrellas_bloquea_un_ataque) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    MOTOR_CHECK(m.jugar("Bendición de las Estrellas (reaccion)"));
    MOTOR_CHECK(m.cartas->turnoAbierto());  // preparar no gasta la accion
    MOTOR_CHECK(m.fila("Bendición de las Estrellas (preparada)") >= 0);
    m.atacar();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 30);
    MOTOR_CHECK(m.carta("bendicion_de_las_estrellas").gastada);
    m.atacar();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 25);
}

MOTOR_TEST_CASE(bardo_ola_de_elementos_devuelve_una_cancion_gastada) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    MOTOR_CHECK(m.fila("Ola de Elementos (no hay canciones gastadas)") >= 0);
    MOTOR_CHECK(m.jugar("Canción del Otoño"));
    m.ronda();
    MOTOR_CHECK(m.jugar("Ola de Elementos: Canción del Otoño"));
    MOTOR_CHECK(!m.carta("cancion_del_otono").gastada);
    MOTOR_CHECK(m.carta("ola_de_elementos").gastada);
    MOTOR_CHECK(m.cartas->turnoAbierto());  // accion libre
}

MOTOR_TEST_CASE(bardo_cancion_de_la_primavera_cura_y_primavera_suma_dos) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    m.heroe.ponerVida(10);
    MOTOR_CHECK(m.jugar("Elegir Primavera"));
    MOTOR_CHECK(m.jugar("Canción de la Primavera"));  // 1d6 = 5, exito, +2
    MOTOR_CHECK(m.heroe.health() == 17);
}

// H2-01 — La mesa guarda la ultima tirada (la del heroe o la del rival) con
// los dados de los dos lados, para que el combate la ensene.
MOTOR_TEST_CASE(la_mesa_guarda_la_ultima_tirada_de_cada_lado) {
    Mesa m("bardo_de_las_cuatro_estaciones", 1, 4);  // lobo con DES 4
    MOTOR_CHECK(m.cartas->ultimaTirada().numero == 0);
    m.atacar();  // 6 dados contra 4, todos 5: 3 contra 2, exito
    const RPG::TiradaVisible golpe = m.cartas->ultimaTirada();
    MOTOR_CHECK(golpe.numero == 1);
    MOTOR_CHECK(golpe.que == "Ataque" && golpe.quien == "Heroe" && golpe.contra == "Lobo");
    MOTOR_CHECK(golpe.delJugador && golpe.enfrentada && !golpe.pifia());
    MOTOR_CHECK(golpe.tirada.rolls.size() == 6 && golpe.rival.rolls.size() == 4);
    MOTOR_CHECK(golpe.exitos() == 3.0f && golpe.rival.successes == 2.0f);
    MOTOR_CHECK(golpe.grado == RPG::Degree::SUCCESS);
    m.ronda();  // el lobo: 2,5 contra 2,5, parcial
    const RPG::TiradaVisible& mordisco = m.cartas->ultimaTirada();
    MOTOR_CHECK(mordisco.numero == 2);
    MOTOR_CHECK(!mordisco.delJugador && mordisco.quien == "Lobo" && mordisco.contra == "Heroe");
    MOTOR_CHECK(mordisco.grado == RPG::Degree::PARTIAL);
    // Una carta lleva su nombre.
    MOTOR_CHECK(m.jugar("Canción del Otoño"));
    MOTOR_CHECK(m.cartas->ultimaTirada().numero == 3);
    MOTOR_CHECK(m.cartas->ultimaTirada().que == m.carta("cancion_del_otono").carta->def->name);
}

MOTOR_TEST_CASE(la_pifia_de_la_mesa_se_marca_y_perder_no_es_pifiar) {
    Mesa pifia("bardo_de_las_cuatro_estaciones");
    pifia.dado.cara = 1;  // ni un 5 ni un 6 y seis 1
    pifia.atacar();
    MOTOR_CHECK(pifia.cartas->ultimaTirada().pifia());
    Mesa pierde("bardo_de_las_cuatro_estaciones", 1, 8);  // el lobo defiende con 8: 3 contra 4
    pierde.atacar();
    MOTOR_CHECK(pierde.cartas->ultimaTirada().grado == RPG::Degree::BOTCH);
    MOTOR_CHECK(!pierde.cartas->ultimaTirada().pifia());
}

// Oriol (05/10): una accion que tira varias veces deja todas sus tiradas con
// la misma accion; cada enemigo que actua despues, la suya.
MOTOR_TEST_CASE(un_area_deja_una_tirada_por_blanco_con_la_misma_accion) {
    Mesa m("bardo_de_las_cuatro_estaciones", 1, 4, 2);
    MOTOR_CHECK(m.jugar("Canción del Otoño"));
    const std::vector<RPG::TiradaVisible> t = m.cartas->tiradas();
    MOTOR_CHECK(t.size() == 2);
    if (t.size() != 2) return;
    MOTOR_CHECK(t[0].numero == 1 && t[1].numero == 2);
    MOTOR_CHECK(t[0].accion == t[1].accion && t[0].accion > 0);
    MOTOR_CHECK(t[0].contra != t[1].contra);  // un lobo y el otro
    MOTOR_CHECK(m.cartas->ultimaTirada().numero == 2);
    m.ronda();  // los dos lobos muerden, cada uno en su accion
    const std::vector<RPG::TiradaVisible> d = m.cartas->tiradas();
    MOTOR_CHECK(d.size() == 4);
    if (d.size() != 4) return;
    MOTOR_CHECK(d[2].accion != d[1].accion && d[3].accion != d[2].accion);
    MOTOR_CHECK(!d[2].delJugador && !d[3].delJugador);
}

int main() { return motor_test::runAllTests(); }

// DANO-02 — La formula de dano de Oriol (06/10/2026), sin mesa.
MOTOR_TEST_CASE(dano_fisico_son_tantos_d6_como_la_diferencia) {
    using RPG::dano::Enfrentamiento;
    using RPG::dano::dadosFisicos;
    MOTOR_CHECK(dadosFisicos(Enfrentamiento{2.0f, 1.0f}) == 1);   // 5 y 6 contra un 6
    MOTOR_CHECK(dadosFisicos(Enfrentamiento{1.5f, 1.0f}) == 1);   // medio exito, un dado
    MOTOR_CHECK(dadosFisicos(Enfrentamiento{3.5f, 1.0f}) == 3);
    MOTOR_CHECK(dadosFisicos(Enfrentamiento{2.0f, 2.0f}) == 0);   // empate: sin dados...
    // ...pero roza (D0014, 14/10): 1 de dano, tambien el 0 contra 0; con pifia, no.
    MOTOR_CHECK(RPG::dano::roza(Enfrentamiento{2.0f, 2.0f}) && RPG::dano::kRoce == 1);
    MOTOR_CHECK(RPG::dano::roza(Enfrentamiento{0.0f, 0.0f}));
    MOTOR_CHECK(!RPG::dano::roza(Enfrentamiento{2.0f, 1.0f}) && !RPG::dano::roza(Enfrentamiento{1.0f, 2.0f}));
    MOTOR_CHECK(!RPG::dano::roza(Enfrentamiento{1.0f, 1.0f, true, false}));
    MOTOR_CHECK(!RPG::dano::roza(Enfrentamiento{1.0f, 1.0f, false, true}));
    MOTOR_CHECK(dadosFisicos(Enfrentamiento{1.0f, 2.0f}) == 0);
    MOTOR_CHECK(dadosFisicos(Enfrentamiento{2.5f, 0.0f, false, true}) == 3);  // pifia el defensor: todos
    MOTOR_CHECK(dadosFisicos(Enfrentamiento{0.0f, 1.0f, false, true}) == 1);  // como poco uno
    MOTOR_CHECK(dadosFisicos(Enfrentamiento{3.0f, 1.0f, true, false}) == 0);  // pifia el atacante
    MOTOR_CHECK(dadosFisicos(Enfrentamiento{2.0f, 1.0f, false, false, true}) == 2);  // todos 6: +1d6
}

MOTOR_TEST_CASE(dano_magico_entero_si_falla_la_salvacion_y_la_mitad_si_la_supera) {
    using namespace RPG::dano;
    MOTOR_CHECK(salvacion(Enfrentamiento{2.0f, 1.0f}) == Salvacion::Falla);
    MOTOR_CHECK(salvacion(Enfrentamiento{1.0f, 1.0f}) == Salvacion::Supera);  // empatar la supera
    MOTOR_CHECK(salvacion(Enfrentamiento{0.5f, 2.0f}) == Salvacion::Supera);
    MOTOR_CHECK(salvacion(Enfrentamiento{0.5f, 2.0f, false, true}) == Salvacion::Pifia);
    MOTOR_CHECK(salvacion(Enfrentamiento{3.0f, 0.0f, true, false}) == Salvacion::Nada);
    MOTOR_CHECK(danoMagico(15, Salvacion::Falla) == 15 && danoMagico(15, Salvacion::Supera) == 7);
    MOTOR_CHECK(danoMagico(15, Salvacion::Nada) == 0);
    // D0015: la pifia del objetivo ya no da un dado mas; le duplica el efecto.
    MOTOR_CHECK(dadosExtraMagia(Enfrentamiento{0.5f, 2.0f, false, true}) == 0);
    MOTOR_CHECK(efectoDoble(Enfrentamiento{0.5f, 2.0f, false, true}));
    MOTOR_CHECK(!efectoDoble(Enfrentamiento{2.0f, 1.0f}));
    MOTOR_CHECK(dadosExtraMagia(Enfrentamiento{2.0f, 1.0f, false, false, true}) == 1);
    MOTOR_CHECK(dadosExtraMagia(Enfrentamiento{2.0f, 1.0f}) == 0);
}

MOTOR_TEST_CASE(una_carta_fisica_suma_sus_dados_solo_si_gana) {
    Mesa m("senor_de_las_armas");
    MOTOR_CHECK(m.jugar("Corte Real"));  // 3 contra 2,5: 1d6 de la diferencia + 2d6 de la carta
    MOTOR_CHECK(m.lobo.health() == 45);
    Mesa empate("senor_de_las_armas");
    empate.dado.cara = 4;  // 0 contra 0: roza (D0014), 1 y sin los dados de la carta
    MOTOR_CHECK(empate.jugar("Corte Real"));
    MOTOR_CHECK(empate.lobo.health() == 59);
}

MOTOR_TEST_CASE(una_carta_de_magia_hace_la_mitad_a_quien_se_salva_y_nunca_sus_estados) {
    MOTOR_CHECK(catalogo().carta("cancion_del_verano")->def->save_attribute == RPG::SaveAttribute::CON);
    MOTOR_CHECK(catalogo().carta("corte_real")->def->save_attribute == RPG::SaveAttribute::NONE);
    Mesa falla("bardo_de_las_cuatro_estaciones");
    MOTOR_CHECK(falla.jugar("Canción del Verano"));  // CON 1 del lobo contra 6 dados: 3d6 enteros
    MOTOR_CHECK(falla.lobo.health() == 45);
    Mesa salva("bardo_de_las_cuatro_estaciones");
    salva.dado.cara = 4;  // 0 contra 0: la supera, la mitad de 3d6 (12)
    MOTOR_CHECK(salva.jugar("Canción del Verano"));
    MOTOR_CHECK(salva.lobo.health() == 54);
    Mesa invierno("bardo_de_las_cuatro_estaciones");
    invierno.dado.cara = 4;  // se salva: sin congelar
    MOTOR_CHECK(invierno.jugar("Canción del Invierno"));
    MOTOR_CHECK(invierno.situLobo().estados.empty());
}

// EH-13 (evaluacion heuristica, 14/10) — Cada carta lleva el icono de lo que hace, no la misma
// estrella para todas.
MOTOR_TEST_CASE(las_cartas_llevan_el_icono_de_su_familia) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    const Game::VistaBarra v = Game::vistaBarra(*m.cartas, 0);
    std::set<std::string> iconos;
    for (const Game::CartaBarra& c : v.mano) {
        MOTOR_CHECK(!c.icono.empty());
        iconos.insert(c.icono);
        if (c.id == "cancion_del_otono") MOTOR_CHECK(c.icono == "int");               // magia contra la mente
        if (c.id == "cancion_de_la_primavera") MOTOR_CHECK(c.icono == "mejora");      // a los tuyos
    }
    MOTOR_CHECK(iconos.size() >= 2);
}

// EH-12 — En el turno del rival nada parece jugable: todo bloqueado, «Espera: juega Lobo».
MOTOR_TEST_CASE(en_el_turno_del_rival_las_cartas_esperan) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    m.batalla->tiraIniciativa();
    for (int i = 0; i < 3 && m.batalla->enemigoEnTurno() < 0; ++i) m.batalla->pasaAlSiguiente();
    MOTOR_CHECK(m.batalla->enemigoEnTurno() == 0);
    const Game::VistaBarra v = Game::vistaDeBatalla(*m.batalla);
    for (const auto* lado : {&v.basicas, &v.mano})
        for (const Game::CartaBarra& c : *lado) {
            MOTOR_CHECK(c.estado != Game::CartaBarra::Estado::Lista);
            if (c.estado == Game::CartaBarra::Estado::Bloqueada) MOTOR_CHECK(c.motivo == "Espera: juega Lobo");
        }
}

// EH-16 — El golpe se cuenta en palabras: «… : el ataque (n) supera la defensa (m). 7 de daño (2d6).»
MOTOR_TEST_CASE(el_registro_cuenta_la_tirada_en_palabras) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    m.batalla->resolveAllyAction(0, BattleAction{BattleActionType::Attack, "", 0, ""});
    bool dice = false;
    for (const std::string& l : m.batalla->log())
        dice |= l.find("Heroe ataca a Lobo: ") == 0 && l.find("el ataque (") != std::string::npos &&
                (l.find("de daño") != std::string::npos || l.find("Sin daño") != std::string::npos);
    MOTOR_CHECK(dice);
}
