// FICHA-01 — La ficha del jugador como datos y el contrato de iconos.
//
// Se ejecuta desde build/ (WORKING_DIRECTORY): lee los catalogos reales de
// assets/catalogs y el manifiesto assets/textures/iconos.json.
#include "Game/FichaVista.h"
#include "HUD/Iconos.h"
#include "RPG/CharacterSheet.h"
#include "TestHarness.h"

#include <algorithm>
#include <string>

using Game::FichaDatos;
using Game::FichaVista;

namespace {

RPG::CharacterSheet fichaDePrueba() {
    RPG::CharacterSheet f;
    f.id = "pj_prueba";
    f.displayName = "Luisarda";
    f.classId = "acechador_de_tejados";
    f.raceId = "humano";
    f.tier = 2;
    f.gold = 137;
    f.inspirationPoints = 1;
    f.baseStats = {1, 2, 0, 1};
    f.racialBonuses = {0, 1, 0, 0};
    f.equipBonuses = {1, 0, 0, 0};
    f.conditionModifiers = {0, -1, 0, 0};
    f.equippedIds[static_cast<std::size_t>(RPG::EquipSlot::MainHand)] = "arco_cazador_de_milicia";
    f.passiveIds = {"anclaje"};
    f.featIds = {"canalizacion_estable"};
    f.conditions.push_back({"envenenado", "slime", 3, 2});
    f.conditions.push_back({"inspirado_por_un_bardo", "bardo", 1, 1});  // no esta en el catalogo
    return f;
}

const FichaVista& vistaConCatalogos() {
    static FichaVista vista;
    static bool cargada = false;
    if (!cargada) {
        const auto r = vista.cargarCatalogos("assets/catalogs");
        MOTOR_CHECK(r.isOk());
        cargada = true;
    }
    return vista;
}

}  // namespace

MOTOR_TEST_CASE(ficha_desglosa_cada_stat_y_el_total_es_el_del_motor) {
    const RPG::CharacterSheet f = fichaDePrueba();
    const FichaDatos d = vistaConCatalogos().construir(f, 12, 20, 3, 5);
    MOTOR_CHECK(!d.vacia);
    MOTOR_CHECK(d.stats[0].clave == "CON" && d.stats[0].icono == "con");
    MOTOR_CHECK(d.stats[0].base == 1 && d.stats[0].equipo == 1 && d.stats[0].total == 2);
    MOTOR_CHECK(d.stats[1].base == 2 && d.stats[1].raza == 1 && d.stats[1].condiciones == -1);
    MOTOR_CHECK(d.stats[1].total == f.des());
    MOTOR_CHECK(d.pv == 12 && d.pvMax == 20 && d.pm == 3 && d.pmMax == 5);
    MOTOR_CHECK(d.oro == 137 && d.tier == 2 && d.inspiracion == 1);
}

MOTOR_TEST_CASE(ficha_nombra_con_los_catalogos_y_legibiliza_lo_que_no_encuentra) {
    const FichaDatos d = vistaConCatalogos().construir(fichaDePrueba(), 1, 1, 0, 0);
    MOTOR_CHECK(d.nombre == "Luisarda");
    MOTOR_CHECK(d.clase == "Acechador de Tejados");
    MOTOR_CHECK(d.ranuras.size() == RPG::kRanurasDeEquipo);  // PART-02: once ranuras
    MOTOR_CHECK(d.ranuras[4].ranura == "MainHand" && d.ranuras[4].etiqueta == "Mano principal");
    MOTOR_CHECK(d.ranuras[4].icono == "ranura_mano_principal");
    MOTOR_CHECK(d.ranuras[4].nombre == "Arco Cazador de Milicia");
    MOTOR_CHECK(d.ranuras[0].nombre == "(vacio)" && d.ranuras[0].objetoId.empty());
    MOTOR_CHECK(d.rasgos.size() == 2);
    MOTOR_CHECK(d.rasgos[0].icono == "pasiva" && !d.rasgos[0].nombre.empty());
    MOTOR_CHECK(d.rasgos[1].icono == "dote" && !d.rasgos[1].descripcion.empty());
    MOTOR_CHECK(d.condiciones.size() == 2);
    MOTOR_CHECK(d.condiciones[0].nombre == "Envenenado" && d.condiciones[0].rondas == 3);
    MOTOR_CHECK(d.condiciones[0].acumulaciones == 2 && d.condiciones[0].negativa);
    // Una condicion desconocida no rompe nada: id legible y negativa por defecto.
    MOTOR_CHECK(d.condiciones[1].nombre == "Inspirado por un bardo");
    MOTOR_CHECK(FichaVista::nombreLegible("guerrera_de_la_marca") == "Guerrera de la marca");
}

// PART-02: en el Tier 1 los anillos, el collar y el cinturon salen cerrados ("desde el Tier 2"); en el 2,
// abiertos y vacios. Los brazos estan abiertos desde el 1.
MOTOR_TEST_CASE(ficha_las_ranuras_del_tier_2_salen_cerradas_en_el_1) {
    RPG::CharacterSheet f = fichaDePrueba();
    f.tier = 1;
    const FichaDatos d1 = vistaConCatalogos().construir(f, 1, 1, 0, 0);
    for (const RPG::EquipSlot s : {RPG::EquipSlot::Accessory, RPG::EquipSlot::Neck, RPG::EquipSlot::Belt,
                                   RPG::EquipSlot::Ring2}) {
        const auto& r = d1.ranuras[static_cast<std::size_t>(s)];
        MOTOR_CHECK(r.abreEnTier == 2 && r.nombre == "(desde el Tier 2)");
    }
    const auto& brazos = d1.ranuras[static_cast<std::size_t>(RPG::EquipSlot::Arms)];
    MOTOR_CHECK(brazos.etiqueta == "Brazos" && brazos.abreEnTier == 0 && brazos.nombre == "(vacio)");
    MOTOR_CHECK(d1.ranuras[static_cast<std::size_t>(RPG::EquipSlot::Neck)].etiqueta == "Collar");
    f.tier = 2;
    const FichaDatos d2 = vistaConCatalogos().construir(f, 1, 1, 0, 0);
    for (const auto& r : d2.ranuras) MOTOR_CHECK(r.abreEnTier == 0);
    MOTOR_CHECK(d2.ranuras[static_cast<std::size_t>(RPG::EquipSlot::Belt)].nombre == "(vacio)");
}

MOTOR_TEST_CASE(ficha_sin_catalogos_funciona_con_ids_legibles) {
    FichaVista sinCatalogos;
    const FichaDatos d = sinCatalogos.construir(fichaDePrueba(), 1, 1, 0, 0);
    MOTOR_CHECK(d.clase == "Acechador de tejados");
    MOTOR_CHECK(d.ranuras[4].nombre == "Arco cazador de milicia");
    MOTOR_CHECK(d.condiciones[0].nombre == "Envenenado");
}

MOTOR_TEST_CASE(ficha_en_texto_lleva_lo_esencial_en_orden) {
    const auto lineas = FichaVista::lineas(vistaConCatalogos().construir(fichaDePrueba(), 12, 20, 3, 5));
    MOTOR_CHECK(lineas.size() >= 3 + 4 + 7 + 2 + 2);
    MOTOR_CHECK(lineas[0] == "Luisarda  Nivel 1  T2");
    MOTOR_CHECK(lineas[2] == "PV 12/20  PM 3/5  Oro 137  Insp 1");
    MOTOR_CHECK(lineas[3] == "CON 2  (base 1 +1)");
    MOTOR_CHECK(lineas[4] == "DES 2  (base 2)");  // +1 raza -1 condicion se anulan
    MOTOR_CHECK(std::find(lineas.begin(), lineas.end(), "Mano principal: Arco Cazador de Milicia") != lineas.end());
    MOTOR_CHECK(lineas.back() == "- Inspirado por un bardo (1 rondas)");
    MOTOR_CHECK(lineas[lineas.size() - 2] == "- Envenenado (3 rondas) x2");
    MOTOR_CHECK(FichaVista::lineas(FichaDatos{}).front() == "(sin ficha)");
}

MOTOR_TEST_CASE(iconos_resuelven_stats_ranuras_y_las_16_condiciones) {
    Hud::Iconos iconos;
    const auto r = iconos.cargar("assets/textures/iconos.json");
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(iconos.cargado() && iconos.columnas() == 8 && iconos.lado() == 16);
    MOTOR_CHECK(iconos.imagen() == "iconos.png");
    MOTOR_CHECK(iconos.frame("con") == 1);
    MOTOR_CHECK(iconos.frame("no_existe") == 0);
    for (const char* stat : {"CON", "DES", "INT", "CAR"}) MOTOR_CHECK(iconos.frameStat(stat) > 0);
    for (const char* ranura : {"Head", "Torso", "Legs", "Feet", "MainHand", "OffHand", "Accessory"}) {
        MOTOR_CHECK(iconos.frameRanura(ranura) > 0);
    }
    RPG::Catalogs::ConditionCatalog condiciones;
    MOTOR_CHECK(condiciones.loadFromFile("assets/catalogs/conditions.json").isOk());
    int propias = 0;
    for (const char* id : {"agarrado", "anclado", "asustado", "caido", "cegado", "encantado",
                           "endeudado", "enfurecido", "envenenado", "fatiga", "guardia_rota",
                           "inconsciente", "marcado", "paralizado", "ralentizado", "sangrado"}) {
        MOTOR_CHECK(condiciones.find(id) != nullptr);
        const int f = iconos.frameCondicion(id, true);
        MOTOR_CHECK(f > 0 && f == iconos.frame(id));
        ++propias;
    }
    MOTOR_CHECK(propias == 16);
    MOTOR_CHECK(iconos.frameCondicion("desconocida", true) == iconos.frame("perjuicio"));
    MOTOR_CHECK(iconos.frameCondicion("desconocida", false) == iconos.frame("mejora"));
    MOTOR_CHECK(iconos.frameMision("activa") > 0 && iconos.frameMision("objetivoHecho") > 0);
    MOTOR_CHECK(iconos.frameCodice("curiosidad") > 0 && iconos.frameCodice("mapa") > 0);
    // Todos los frames caben en el atlas declarado.
    MOTOR_CHECK(static_cast<int>(iconos.total()) <= iconos.columnas() * iconos.filas());
}

int main() { return motor_test::runAllTests(); }
