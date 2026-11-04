// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 2 paquetes del corte B:
//   D-02  (fase D, 4 j) - Habilitats i encanteris
//   D-10  (fase D, 2 j) - CI valida els assets reals
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// D-02 — El contenido del catálogo de habilidades y hechizos, leído de
// verdad desde las cartas publicadas.
//
// Antes (medido el 07/09, DEC-02): las 492 skills y los 210 hechizos
// cargaban TODOS con la misma magnitud {0, 2, 4, 8} porque
// magnitudeByDegree es null en el asset y el fallback repartía base 4
// fija; defenseStat (465/492), cost (460/492), range (483/492) y
// effect.scaling (483/492) no los leía nadie; y las 9 utilitarias
// narrativas declaran magnitudeByDegree como OBJETO
// {botch, partial, success, critical} a cero, forma que el cargador no
// aceptaba y tragaba convertida en el {0, 2, 4, 8} de daño.
//
// Aquí: tres cartas concretas con sus valores escritos en el test (una
// por tier bajo, una de tier 5 y una utilitaria de check), un hechizo, y
// la propiedad que pide la ficha del paquete: dos cartas diferentes dan
// números diferentes. Los assets se leen del directorio de build (los
// copia copiar_assets), igual que test_catalog_contract.

#include "RPG/Catalogs/RpgCatalogs.h"

#include "TestHarness.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <string>
#include <utility>

namespace {

using RPG::Catalogs::CatalogStrictness;
using RPG::Catalogs::ConditionCatalog;
using RPG::Catalogs::ConsumableCatalog;
using RPG::Catalogs::DeityCatalog;
using RPG::Catalogs::EquipmentCatalog;
using RPG::Catalogs::LootTableCatalog;
using RPG::Catalogs::MonsterCatalog;
using RPG::Catalogs::MountCatalog;
using RPG::Catalogs::SkillCatalog;
using RPG::Catalogs::SpellCatalog;
using RPG::Catalogs::SummonCatalog;
using RPG::Catalogs::TrapCatalog;
using RPG::Degree;
using RPG::SaveAttribute;
using RPG::SkillRange;
using RPG::SkillTarget;
using RPG::RecoveryPile;
using RPG::Stat;

SkillCatalog carga_skills() {
    SkillCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/skills.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    return cat;
}

SpellCatalog carga_spells() {
    SpellCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/spells.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    return cat;
}

void check_mag(const RPG::SkillDefinition& def, float botch, float partial, float success,
               float critical) {
    MOTOR_CHECK(std::fabs(def.magnitude_f(Degree::BOTCH) - botch) < 0.01f);
    MOTOR_CHECK(std::fabs(def.magnitude_f(Degree::PARTIAL) - partial) < 0.01f);
    MOTOR_CHECK(std::fabs(def.magnitude_f(Degree::SUCCESS) - success) < 0.01f);
    MOTOR_CHECK(std::fabs(def.magnitude_f(Degree::CRITICAL) - critical) < 0.01f);
}

// Carta 1: tier 1 con todos los campos nuevos poblados. Valores copiados
// del JSON publicado — cualquier regresión del cargador los rompe.
MOTOR_TEST_CASE(abrazo_de_brasas_dice_lo_que_dice_su_carta) {
    const SkillCatalog cat = carga_skills();
    const RPG::SkillDefinition* def = cat.find("abrazo_de_brasas");
    MOTOR_CHECK(def != nullptr);
    if (def == nullptr) return;

    MOTOR_CHECK(def->tier_min == 1);
    MOTOR_CHECK(def->casting_stat == Stat::CON);
    MOTOR_CHECK(def->defense_stat == "resistencia_fisica");
    MOTOR_CHECK(def->save_attribute == SaveAttribute::CON);  // GDD §5: resistencia física = CON
    MOTOR_CHECK(def->range == SkillRange::MELEE);
    MOTOR_CHECK(def->cost_resource == "none");
    MOTOR_CHECK(def->cost_amount == 0);
    MOTOR_CHECK(def->recovery == RecoveryPile::SHORT_REST);
    MOTOR_CHECK(def->target == SkillTarget::SINGLE_ENEMY);
    MOTOR_CHECK(def->effect_scaling == "CON");
    // Sin magnitudeByDegree declarado y tier 1: base derivada 2+2·1 = 4,
    // EXACTAMENTE el {0, 2, 4, 8} de antes (los números plantados de la
    // campaña no cambian).
    check_mag(*def, 0.0f, 2.0f, 4.0f, 8.0f);
}

// Carta 2: tier 5. La mismísima derivación tiene que dar números
// DISTINTOS para una carta distinta — es la propiedad del paquete.
MOTOR_TEST_CASE(un_carta_de_tier_5_pesa_como_una_de_tier_5) {
    const SkillCatalog cat = carga_skills();
    const RPG::SkillDefinition* def = cat.find("itx_tejedora_de_mareas_ofensiva_t5");
    MOTOR_CHECK(def != nullptr);
    if (def == nullptr) return;

    MOTOR_CHECK(def->tier_min == 5);
    // castingStat null en la carta: físico sin stat firmante -> DES (el
    // stat del ataque básico), no INT como antes.
    MOTOR_CHECK(def->casting_stat == Stat::DES);
    // "CA" no es una salvación prefijada: física -> NONE, y el defensor
    // tira el MAYOR de DES/CON en la enfrentada (Ed. 3, bitácora 2.17.4).
    MOTOR_CHECK(def->defense_stat == "CA");
    MOTOR_CHECK(def->save_attribute == SaveAttribute::NONE);
    MOTOR_CHECK(def->range == SkillRange::MEDIUM);
    MOTOR_CHECK(def->recovery == RecoveryPile::LONG_REST);
    MOTOR_CHECK(def->effect_scaling == "INT");
    // base derivada 2+2·5 = 12 = el preset "Llegendari (1d12)".
    check_mag(*def, 0.0f, 6.0f, 12.0f, 24.0f);
}

// Carta 3: una utilitaria narrativa. Declara magnitudeByDegree como
// OBJETO {botch, partial, success, critical} a cero: un check no hace
// daño. El cargador antiguo no aceptaba la forma objeto y les ponía a
// todas {0, 2, 4, 8} de daño.
MOTOR_TEST_CASE(las_utilitarias_de_check_no_hacen_dano) {
    const SkillCatalog cat = carga_skills();
    const RPG::SkillDefinition* def = cat.find("percepcion");
    MOTOR_CHECK(def != nullptr);
    if (def == nullptr) return;

    MOTOR_CHECK(def->casting_stat == Stat::DES);
    MOTOR_CHECK(def->range == SkillRange::UNKNOWN);  // la carta no declara alcance
    MOTOR_CHECK(def->defense_stat.empty());
    MOTOR_CHECK(def->save_attribute == SaveAttribute::NONE);
    MOTOR_CHECK(def->cost_resource.empty());  // sin clave cost: sin default inventado
    check_mag(*def, 0.0f, 0.0f, 0.0f, 0.0f);

    // Las siete que usan los skillCheck de las aventuras publicadas
    // (prologo: percepcion/conocimiento_arcano; segundo dia: sigilo,
    // intimidacion, investigacion, persuasion) son todas así.
    for (const char* id : {"conocimiento_arcano", "sigilo", "intimidacion", "investigacion",
                           "persuasion", "atletismo"}) {
        const RPG::SkillDefinition* u = cat.find(id);
        MOTOR_CHECK(u != nullptr);
        if (u != nullptr) check_mag(*u, 0.0f, 0.0f, 0.0f, 0.0f);
    }
}

// El hechizo: mismo cargador (SpellCatalog = Catalog<SkillDefinition>),
// claves de spell. Todos los hechizos declaran castingStat (CAR 204,
// INT 6); ninguno trae cost.
MOTOR_TEST_CASE(alba_anunciar_el_dia_dice_lo_que_dice_su_carta) {
    const SpellCatalog cat = carga_spells();
    const RPG::SkillDefinition* def = cat.find("alba_anunciar_el_dia");
    MOTOR_CHECK(def != nullptr);
    if (def == nullptr) return;

    MOTOR_CHECK(def->casting_stat == Stat::CAR);
    MOTOR_CHECK(def->range == SkillRange::SELF);
    MOTOR_CHECK(def->recovery == RecoveryPile::ACTIVE);
    MOTOR_CHECK(def->defense_stat.empty());
    MOTOR_CHECK(def->save_attribute == SaveAttribute::NONE);
    MOTOR_CHECK(def->cost_resource.empty());
    MOTOR_CHECK(def->effect_scaling == "none");
    check_mag(*def, 0.0f, 2.0f, 4.0f, 8.0f);  // tier 1 sin magnitudes declaradas
}

// La propiedad del paquete, sobre el catálogo entero: con el fallback
// uniforme, las 492 skills compartían SUCCESS = 4. Ahora la magnitud
// depende del tier de cada carta (0 las utilitarias; 4/6/8/10/12 los
// cinco escalones). Desde CP-01 el catálogo trae 1191 cartas.
MOTOR_TEST_CASE(dos_cartas_diferentes_dan_numeros_diferentes) {
    const SkillCatalog cat = carga_skills();
    MOTOR_CHECK(cat.size() >= 1191);  // AV18-03: las 12 cartas de equipo amplían el catálogo.

    std::set<float> successMag;
    cat.forEach([&successMag](const RPG::SkillDefinition& def) {
        successMag.insert(def.magnitude_f(Degree::SUCCESS));
    });
    MOTOR_CHECK(successMag.size() >= 6);
    MOTOR_CHECK(successMag.count(0.0f) == 1);   // las utilitarias
    MOTOR_CHECK(successMag.count(4.0f) == 1);   // tier 1
    MOTOR_CHECK(successMag.count(12.0f) == 1);  // tier 5
}

// ===================================================================
// D-04 — Condiciones: el cargador lee el asset real (DEC-02).
//
// Antes: las 16 cartas hablan stackable/duration/effects[] y el cargador
// esperaba stacking/defaultRounds/damagePerRound — las 16 cargaban
// IDÉNTICAS (DURATION, 3 rondas, 1 stack, 0 daño, todos los flags
// apagados). Ahora cada campo se lee de la carta y los hooks del DSL
// (dano_por_turno:1, desventaja:ataque...) se traducen solo cuando la
// traducción es unívoca; el resto queda crudo en mechanicHooks.
// ===================================================================

ConditionCatalog carga_conditions() {
    ConditionCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/conditions.json",
                                    CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    return cat;
}

const RPG::ConditionDefinition* cond(const ConditionCatalog& cat, const char* id) {
    const RPG::ConditionDefinition* def = cat.find(id);
    MOTOR_CHECK(def != nullptr);
    return def;
}

MOTOR_TEST_CASE(sangrado_dice_lo_que_dice_su_carta) {
    const ConditionCatalog cat = carga_conditions();
    MOTOR_CHECK(cat.size() == 20);  // PO-37: + quemado, congelado, aturdido, silenciado
    const RPG::ConditionDefinition* d = cond(cat, "sangrado");
    if (d == nullptr) return;

    // "Al empezar tu turno recibes 1 de daño" (hook dano_por_turno:1).
    MOTOR_CHECK(d->damagePerRound == 1);
    // stackable: false -> reemplazo, no apilado.
    MOTOR_CHECK(d->stacking == RPG::ConditionDefinition::Stacking::NONE);
    // "hasta curarse": no expira por rondas.
    MOTOR_CHECK(d->durationMode == RPG::ConditionDefinition::Duration::UNTIL_CURED);
    MOTOR_CHECK(d->isNegative);
    // El hook queda también crudo: quien lo consuma no repite el parseo.
    bool tieneHook = false;
    for (const std::string& h : d->mechanicHooks) tieneHook |= (h == "dano_por_turno:1");
    MOTOR_CHECK(tieneHook);
}

MOTOR_TEST_CASE(guardia_rota_es_la_unica_que_expira_por_rondas) {
    const ConditionCatalog cat = carga_conditions();
    const RPG::ConditionDefinition* d = cond(cat, "guardia_rota");
    if (d == nullptr) return;

    // duration "1_turn": expira, y en UNA ronda — no en las 3 por defecto.
    MOTOR_CHECK(d->durationMode == RPG::ConditionDefinition::Duration::ROUNDS);
    MOTOR_CHECK(d->defaultRounds == 1);
    // penal_ca:2 no se pierde: queda crudo (Ed. 3 deroga la CA como
    // dificultad; su ejecutor tipado es D-11).
    bool tienePenalCa = false;
    for (const std::string& h : d->mechanicHooks) tienePenalCa |= (h == "penal_ca:2");
    MOTOR_CHECK(tienePenalCa);
}

MOTOR_TEST_CASE(fatiga_es_la_unica_apilable_y_anclado_no_es_negativa) {
    const ConditionCatalog cat = carga_conditions();
    const RPG::ConditionDefinition* fatiga = cond(cat, "fatiga");
    if (fatiga != nullptr) {
        // stackable: true -> suma intensidad (maxStacks sigue a 1: ninguna
        // carta declara el máximo y no se inventa).
        MOTOR_CHECK(fatiga->stacking == RPG::ConditionDefinition::Stacking::STACK_INTENSITY);
        MOTOR_CHECK(fatiga->maxStacks == 1);
        MOTOR_CHECK(fatiga->durationMode == RPG::ConditionDefinition::Duration::UNTIL_CURED);
    }
    // category "neutro" (anclado, enfurecido): isNegative a false.
    const RPG::ConditionDefinition* anclado = cond(cat, "anclado");
    if (anclado != nullptr) MOTOR_CHECK(!anclado->isNegative);
}

MOTOR_TEST_CASE(los_flags_traducen_los_hooks_univocos) {
    const ConditionCatalog cat = carga_conditions();
    // ralentizado: velocidad_mitad -> halfSpeed. velocidad_0 (agarrado,
    // paralizado...) NO es halfSpeed y queda crudo.
    const RPG::ConditionDefinition* ral = cond(cat, "ralentizado");
    if (ral != nullptr) MOTOR_CHECK(ral->halfSpeed);
    const RPG::ConditionDefinition* aga = cond(cat, "agarrado");
    if (aga != nullptr) MOTOR_CHECK(!aga->halfSpeed);
    // caido: desventaja:ataque -> el que la sufre tira con desventaja.
    const RPG::ConditionDefinition* caido = cond(cat, "caido");
    if (caido != nullptr) MOTOR_CHECK(caido->grantDisadvantageToCaster);
    // cegado: bloquea:vision -> ventaja a quien le ataca (GDD).
    const RPG::ConditionDefinition* ceg = cond(cat, "cegado");
    if (ceg != nullptr) {
        MOTOR_CHECK(ceg->grantAdvantageToEnemies);
        MOTOR_CHECK(ceg->grantDisadvantageToCaster);  // y también desventaja:ataque
    }
    // inconsciente: incapacitado -> preventAction.
    const RPG::ConditionDefinition* inc = cond(cat, "inconsciente");
    if (inc != nullptr) MOTOR_CHECK(inc->preventAction);
    // envenenado: desventaja en ataque Y habilidad, pero 0 daño por ronda
    // ("puede EVOLUCIONAR a daño continuo" no es daño hoy).
    const RPG::ConditionDefinition* env = cond(cat, "envenenado");
    if (env != nullptr) {
        MOTOR_CHECK(env->grantDisadvantageToCaster);
        MOTOR_CHECK(env->damagePerRound == 0);
    }
}

// La prueba que pide la ficha del paquete: al menos tres combinaciones
// distintas de apilamiento y durada entre las 16 cartas. Con el cargador
// antiguo todas daban (DURATION, 3 rondas): el set tenía UN elemento.
MOTOR_TEST_CASE(las_condiciones_no_cargan_todas_iguales) {
    const ConditionCatalog cat = carga_conditions();
    std::set<std::pair<int, int>> combos;
    cat.forEach([&combos](const RPG::ConditionDefinition& def) {
        combos.insert({static_cast<int>(def.stacking), static_cast<int>(def.durationMode)});
    });
    // (NONE, UNTIL_CURED) 14 cartas · (STACK_INTENSITY, UNTIL_CURED) fatiga ·
    // (NONE, ROUNDS) guardia_rota.
    MOTOR_CHECK(combos.size() >= 3);
    MOTOR_CHECK(combos.count({static_cast<int>(RPG::ConditionDefinition::Stacking::NONE),
                              static_cast<int>(RPG::ConditionDefinition::Duration::UNTIL_CURED)}) == 1);
    MOTOR_CHECK(combos.count({static_cast<int>(RPG::ConditionDefinition::Stacking::STACK_INTENSITY),
                              static_cast<int>(RPG::ConditionDefinition::Duration::UNTIL_CURED)}) == 1);
    MOTOR_CHECK(combos.count({static_cast<int>(RPG::ConditionDefinition::Stacking::NONE),
                              static_cast<int>(RPG::ConditionDefinition::Duration::ROUNDS)}) == 1);
}

// ===================================================================
// D-03 — Monstruos: vida, defensas y stats leídos de donde están.
// ===================================================================

MonsterCatalog carga_monsters() {
    MonsterCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/monsters.json",
                                    CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    return cat;
}

// Carta concreta con valores copiados del JSON publicado: la vida NO está
// en la raíz (está en defenses{}/derived{}) y los stats vienen en
// MAYÚSCULAS. Con el cargador antiguo: maxHealth 0, ca 10, stats a cero.
MOTOR_TEST_CASE(acolito_de_penumbra_dice_lo_que_dice_su_carta) {
    const MonsterCatalog cat = carga_monsters();
    MOTOR_CHECK(cat.size() == 432);
    const RPG::MonsterDefinition* def = cat.find("acolito_de_penumbra");
    MOTOR_CHECK(def != nullptr);
    if (def == nullptr) return;

    MOTOR_CHECK(def->tier == 1);
    MOTOR_CHECK(def->maxHealth == 12);   // defenses.maxHealth (derived.vida lo confirma)
    MOTOR_CHECK(def->ca == 11);
    MOTOR_CHECK(def->defMental == 12);
    MOTOR_CHECK(def->resFis == 11);
    MOTOR_CHECK(def->movement == 5);
    MOTOR_CHECK(def->stats[static_cast<int>(Stat::CON)] == 1);
    MOTOR_CHECK(def->stats[static_cast<int>(Stat::DES)] == 1);
    MOTOR_CHECK(def->stats[static_cast<int>(Stat::INT)] == 4);
    MOTOR_CHECK(def->stats[static_cast<int>(Stat::CAR)] == 2);
    MOTOR_CHECK(def->role == "conjurador");
    MOTOR_CHECK(def->rank == "criatura");
    // El botín va en `loot`, no en `lootIds`.
    MOTOR_CHECK(def->lootIds.size() == 2);
    MOTOR_CHECK(def->lootIds.front() == "estilete_de_milicia");
}

// La prueba que pide la ficha del paquete: ningún monstruo del catálogo
// carga con vida cero ni con las cuatro estadísticas a cero.
MOTOR_TEST_CASE(cap_monstre_del_cataleg_te_vida_i_stats) {
    const MonsterCatalog cat = carga_monsters();
    MOTOR_CHECK(cat.size() == 432);

    int conVida = 0;
    int conStats = 0;
    cat.forEach([&conVida, &conStats](const RPG::MonsterDefinition& def) {
        if (def.maxHealth > 0) ++conVida;
        if (def.stats[0] != 0 || def.stats[1] != 0 || def.stats[2] != 0 || def.stats[3] != 0) {
            ++conStats;
        }
    });
    MOTOR_CHECK(conVida == 432);
    MOTOR_CHECK(conStats == 432);
}

// ===================================================================
// D-05 (loaders) — Equipment y consumibles leen sus campos.
// ===================================================================

EquipmentCatalog carga_equipment() {
    EquipmentCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/equipment.json",
                                    CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    return cat;
}

ConsumableCatalog carga_consumables() {
    ConsumableCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/consumables.json",
                                    CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    return cat;
}

// Tres campos que el cargador ignoraba: el arco pide DES 2, dispara la
// skill `tiro_certero` y la placa pesada penaliza DES 2.
MOTOR_TEST_CASE(arco_corto_dice_lo_que_dice_su_carta) {
    const EquipmentCatalog cat = carga_equipment();
    MOTOR_CHECK(cat.size() >= 413);  // SP-02: + los trajes de los especialistas

    const RPG::EquipmentDefinition* arco = cat.find("arco_corto");
    MOTOR_CHECK(arco != nullptr);
    if (arco == nullptr) return;
    MOTOR_CHECK(arco->linkedSkillId == "tiro_certero");
    MOTOR_CHECK(arco->requiredStats[static_cast<int>(Stat::DES)] == 2);
    MOTOR_CHECK(arco->requiredStats[static_cast<int>(Stat::CON)] == 0);

    const RPG::EquipmentDefinition* placa = cat.find("armadura_de_placas");
    MOTOR_CHECK(placa != nullptr);
    if (placa == nullptr) return;
    MOTOR_CHECK(placa->statPenalties[static_cast<int>(Stat::DES)] == 2);
}

// `compatibilitySymbols` (13 cartas) es el mismo campo que
// `compatibleSymbols`: se fusiona sin duplicar. En el asset publicado las
// 13 lo traen redundante (también en minúscula), así que la fusión se
// prueba con una entrada sintética que SOLO trae la variante en mayúscula
// — con el cargador antiguo queda vacía.
MOTOR_TEST_CASE(els_simbols_amb_majuscula_no_es_perden) {
    const EquipmentCatalog cat = carga_equipment();
    const RPG::EquipmentDefinition* focus = cat.find("item_arcane_focus");
    MOTOR_CHECK(focus != nullptr);
    if (focus == nullptr) return;
    bool triangle = false, estrella = false;
    for (const std::string& s : focus->compatibleSymbols) {
        if (s == "△") triangle = true;
        if (s == "✦") estrella = true;
    }
    MOTOR_CHECK(triangle && estrella);

    RPG::Catalogs::EquipmentCatalog sintetic;
    const auto r = sintetic.loadFromString(R"({
        "entries": [{"id": "probeta", "name": "Probeta",
                     "compatibilitySymbols": ["△"]}]
    })");
    MOTOR_CHECK(r.isOk());
    const RPG::EquipmentDefinition* prov = sintetic.find("probeta");
    MOTOR_CHECK(prov != nullptr);
    if (prov == nullptr) return;
    MOTOR_CHECK(prov->compatibleSymbols.size() == 1);
    MOTOR_CHECK(prov->compatibleSymbols.front() == "△");
}

// Barrido: 108 cartas con requisitos, 33 linkedSkill que resuelven en
// skills∪spells y 6 con penalizadores. Con el cargador antiguo: 0, 0 y 0.
MOTOR_TEST_CASE(l_equipment_mecanic_es_llegeix) {
    const EquipmentCatalog eq = carga_equipment();
    const SkillCatalog sk = carga_skills();
    const SpellCatalog sp = carga_spells();

    int ambRequisits = 0, ambLinked = 0, ambPenal = 0;
    bool totResol = true;
    eq.forEach([&](const RPG::EquipmentDefinition& def) {
        bool req = false, pen = false;
        for (int v : def.requiredStats) req |= (v > 0);
        for (int v : def.statPenalties) pen |= (v > 0);
        if (req) ++ambRequisits;
        if (pen) ++ambPenal;
        if (!def.linkedSkillId.empty()) {
            ++ambLinked;
            if (sk.find(def.linkedSkillId.c_str()) == nullptr &&
                sp.find(def.linkedSkillId.c_str()) == nullptr) {
                totResol = false;
            }
        }
    });
    MOTOR_CHECK(ambRequisits == 108);
    MOTOR_CHECK(ambLinked == 45);  // 33 anteriores + 12 únicas de Boundington.
    MOTOR_CHECK(totResol);
    MOTOR_CHECK(ambPenal == 6);
}

// Consumible: lo mecánico es `uses` y `actionType` (31/31); `effect` es
// solo {description}, prosa no ejecutable.
MOTOR_TEST_CASE(aceite_de_filo_dice_lo_que_dice_su_carta) {
    const ConsumableCatalog cat = carga_consumables();
    MOTOR_CHECK(cat.size() == 31);
    const RPG::ConsumableDefinition* def = cat.find("aceite_de_filo");
    MOTOR_CHECK(def != nullptr);
    if (def == nullptr) return;
    MOTOR_CHECK(def->uses == 1);
    MOTOR_CHECK(def->actionType == "accion_menor");

    int menor = 0, accio = 0, reaccio = 0;
    cat.forEach([&](const RPG::ConsumableDefinition& c) {
        if (c.actionType == "accion_menor") ++menor;
        else if (c.actionType == "accion") ++accio;
        else if (c.actionType == "reaccion") ++reaccio;
    });
    MOTOR_CHECK(menor == 19);
    MOTOR_CHECK(accio == 9);
    MOTOR_CHECK(reaccio == 3);
}

// ===================================================================
// D-07 (deidades) — `domain`+`grantedSpells`+favor/obligaciones leídos.
// ===================================================================

DeityCatalog carga_deities() {
    DeityCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/deities.json",
                                    CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    return cat;
}

// El Padre de la Forja: `domain` es UN string con comas (se parte por
// comas, sin inventar fronteras en el " y "), las plegarias vienen en
// `grantedSpells` y el favor trae scaling. Con el cargador antiguo:
// 0 dominios, 0 plegarias, favor vacío.
MOTOR_TEST_CASE(el_padre_de_la_forja_dice_lo_que_dice_su_carta) {
    const DeityCatalog cat = carga_deities();
    MOTOR_CHECK(cat.size() == 23);  // DIOSES-01: los 13 Creadores y las 10 locales
    const RPG::DeityDefinition* def = cat.find("deity_forge_father");
    MOTOR_CHECK(def != nullptr);
    if (def == nullptr) return;

    MOTOR_CHECK(def->domains.size() == 2);
    MOTOR_CHECK(def->domains.front() == "forja");
    MOTOR_CHECK(def->domains.back() == "artesanía y juramentos");
    MOTOR_CHECK(def->prayerSkillIds.size() == 10);
    MOTOR_CHECK(def->prayerSkillIds.front() == "forja_soldadura_de_carne");
    MOTOR_CHECK(def->favorScaling == "CAR");
    MOTOR_CHECK(!def->favorDescription.empty());
    MOTOR_CHECK(def->obligations.size() == 1);
    MOTOR_CHECK(def->obligations.front() ==
        "Nunca destruyas una obra bien hecha sin necesidad, ni dejes una deuda de trabajo sin pagar.");
    MOTOR_CHECK(!def->compatibleWithIds.empty());
    MOTOR_CHECK(def->rank == "deidad_local" && !def->esCreador());
}

// DIOSES-01 — Oriol (08/10): «Los dioses y lore es mío». Los Creadores salen
// de su vault: el título y el mensaje, de Astrologia.docx; el patrón de cada
// pueblo, de la diapositiva «Creadores + razas»; las subrazas, de con quién
// dicen alinearse sus documentos de razas.
MOTOR_TEST_CASE(los_creadores_son_los_del_vault_de_oriol) {
    const DeityCatalog cat = carga_deities();
    int creadores = 0;
    cat.forEach([&](const RPG::DeityDefinition& def) {
        if (def.esCreador()) ++creadores;
    });
    MOTOR_CHECK(creadores == 13);
    const auto tiene = [](const std::vector<std::string>& v, const char* x) {
        return std::find(v.begin(), v.end(), std::string(x)) != v.end();
    };
    const RPG::DeityDefinition* chronos = cat.find("chronos");
    MOTOR_CHECK(chronos != nullptr);
    if (chronos == nullptr) return;
    MOTOR_CHECK(chronos->name == "Chronos" && chronos->title == "Dios del tiempo");
    MOTOR_CHECK(chronos->domains == std::vector<std::string>{"el tiempo"});
    MOTOR_CHECK(chronos->flavorText == "Todo llega cuando tiene que llegar.");
    // Patrón de los dracónidos (la diapositiva), aunque los de fuego recen a Egos y Vida.
    MOTOR_CHECK(tiene(chronos->compatibleWithIds, "draconidos_de_fuego"));
    MOTOR_CHECK(tiene(chronos->compatibleWithIds, "draconidos_de_hielo"));
    MOTOR_CHECK(chronos->prayerSkillIds.empty() && chronos->obligations.empty());
    const RPG::DeityDefinition* forja = cat.find("forja");
    MOTOR_CHECK(forja != nullptr && tiene(forja->compatibleWithIds, "enanos_forjadores"));
    const RPG::DeityDefinition* sofia = cat.find("sofia");
    MOTOR_CHECK(sofia != nullptr && sofia->name == "Sofía" && tiene(sofia->compatibleWithIds, "minotauros_de_la_llanura"));
}

// Barrido: las 23 traen dominio (47 en total partiendo por comas; 19 de las
// locales) y las 100 plegarias de las locales resuelven en los hechizos.
MOTOR_TEST_CASE(les_deitats_porten_dominis_i_plegaries_reals) {
    const DeityCatalog deit = carga_deities();
    const SpellCatalog spells = carga_spells();

    int dominis = 0, pregaries = 0;
    bool totResolve = true, totesAmbDomini = true;
    deit.forEach([&](const RPG::DeityDefinition& def) {
        if (def.domains.empty()) totesAmbDomini = false;
        dominis += static_cast<int>(def.domains.size());
        for (const std::string& id : def.prayerSkillIds) {
            ++pregaries;
            if (spells.find(id.c_str()) == nullptr) totResolve = false;
        }
    });
    MOTOR_CHECK(totesAmbDomini);
    MOTOR_CHECK(dominis == 47);
    MOTOR_CHECK(pregaries == 100);
    MOTOR_CHECK(totResolve);
}

// ===================================================================
// D-07 (summons) — `health`+`duration`/`control`/`summonedBy` leídos.
// ===================================================================

SummonCatalog carga_summons() {
    SummonCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/summons.json",
                                    CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    return cat;
}

// Arma Flotante: la vida va en `health` (no `maxHealth`), la duración es
// el string "concentration" (activa el flag que el struct ya preveía) y
// la carta dice quién la manda y quién la invocó. Con el cargador
// antiguo: vida 0, sin flag, control y origen vacíos.
MOTOR_TEST_CASE(arma_flotante_dice_lo_que_dice_su_carta) {
    const SummonCatalog cat = carga_summons();
    // + el Lobo solar del Cazasoles y los tres del Mago (SP-02) y las diez de las clases propias (CP-01)
    MOTOR_CHECK(cat.size() == 217);
    const RPG::SummonDefinition* def = cat.find("arma_flotante");
    MOTOR_CHECK(def != nullptr);
    if (def == nullptr) return;

    MOTOR_CHECK(def->maxHealth == 2);  // 4 en la carta; a la mitad desde DANO-02
    MOTOR_CHECK(def->durationRaw == "concentration");
    MOTOR_CHECK(def->concentrationRequired);
    MOTOR_CHECK(def->control == "jugador");
    MOTOR_CHECK(def->summonedBySkillId == "arsenal_del_rey");
    MOTOR_CHECK(def->movement == 6);
    MOTOR_CHECK(def->tier == 1);
}

// Barrido: las 217 traen vida, 117 concentration contra 84 "1_min" (+2
// "10 turnos" del crecimiento del catálogo; el Lobo solar del Cazasoles,
// que dura todo el combate, y el Tótem, las Arañas y los Clones del Mago,
// 1 turno; CP-01: la Bestia domada del Domador, todo el combate, y las
// torretas, la escolta, el roble, el tótem, las ánimas, el lobo espiritual y
// el marinero ahogado, 2-3 turnos) y el control se reparte 146/57/14 (en
// crudo: `car_invocador` es opaco y no se interpreta).
MOTOR_TEST_CASE(els_summons_porten_vida_durada_i_control) {
    const SummonCatalog cat = carga_summons();

    int ambVida = 0, concentracio = 0, unMinut = 0, deuTorns = 0;
    int jugador = 0, invocador = 0, automatica = 0;
    cat.forEach([&](const RPG::SummonDefinition& def) {
        if (def.maxHealth > 0) ++ambVida;
        if (def.durationRaw == "concentration") ++concentracio;
        else if (def.durationRaw == "1_min") ++unMinut;
        else if (def.durationRaw == "10 turnos") ++deuTorns;
        if (def.control == "jugador") ++jugador;
        else if (def.control == "car_invocador") ++invocador;
        else if (def.control == "automatica") ++automatica;
    });
    MOTOR_CHECK(ambVida == 217);
    MOTOR_CHECK(concentracio == 117);
    MOTOR_CHECK(unMinut == 84);
    MOTOR_CHECK(deuTorns == 2);
    MOTOR_CHECK(jugador == 146);
    MOTOR_CHECK(invocador == 57);
    MOTOR_CHECK(automatica == 14);
}

// ===================================================================
// D-07 (monturas) — `armor`/`health`/`capacity`/tags/terreno/dismount.
// ===================================================================

MountCatalog carga_mounts() {
    MountCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/mounts.json",
                                    CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    return cat;
}

// Ave de Silla: armor→CA, health→vida, 1 plaza, terrenos, requisito DES 1
// y regla de desmonte en crudo. Con el cargador antiguo: CA 12, vida 20,
// capacity sin leer (1 por defecto) y todo lo demás vacío.
MOTOR_TEST_CASE(ave_de_silla_dice_lo_que_dice_su_carta) {
    const MountCatalog cat = carga_mounts();
    MOTOR_CHECK(cat.size() == 50);
    const RPG::MountDefinition* def = cat.find("ave_de_silla_aarakocra");
    MOTOR_CHECK(def != nullptr);
    if (def == nullptr) return;

    MOTOR_CHECK(def->ca == 11);
    MOTOR_CHECK(def->maxHealth == 12);
    MOTOR_CHECK(def->movement == 9);
    MOTOR_CHECK(def->capacity == 1);
    MOTOR_CHECK(def->terrain.size() == 2);
    MOTOR_CHECK(def->terrain.front() == "aire");
    MOTOR_CHECK(def->mountedTags.size() == 1);
    MOTOR_CHECK(def->mountedTags.front() == "Montado");
    MOTOR_CHECK(def->requiredRideStat == "DES");
    MOTOR_CHECK(def->requiredRideMin == 1);
    MOTOR_CHECK(!def->dismountTrigger.empty());
    MOTOR_CHECK(!def->upkeep.empty());
    MOTOR_CHECK(def->stats[static_cast<int>(Stat::CON)] == 3);
    MOTOR_CHECK(def->stats[static_cast<int>(Stat::DES)] == 3);
}

// Barrido: las 50 traen vida y terreno, y las plazas se reparten 20/30.
MOTOR_TEST_CASE(les_muntures_porten_vida_terreny_i_places) {
    const MountCatalog cat = carga_mounts();

    int ambVida = 0, ambTerreny = 0, unaPlaca = 0, duesPlaces = 0;
    cat.forEach([&](const RPG::MountDefinition& def) {
        if (def.maxHealth > 0) ++ambVida;
        if (!def.terrain.empty()) ++ambTerreny;
        if (def.capacity == 1) ++unaPlaca;
        else if (def.capacity == 2) ++duesPlaces;
    });
    MOTOR_CHECK(ambVida == 50);
    MOTOR_CHECK(ambTerreny == 50);
    MOTOR_CHECK(unaPlaca == 20);
    MOTOR_CHECK(duesPlaces == 30);
}

// ===================================================================
// D-07 (npcs) — occupation, isMerchant, factionId i gold llegits.
// ===================================================================

RPG::Catalogs::Catalog<RPG::NpcDefinition> carga_npcs() {
    RPG::Catalogs::Catalog<RPG::NpcDefinition> cat;
    auto r = cat.loadFromFile("assets/catalogs/npcs.json");
    MOTOR_CHECK(r.isOk());
    return cat;
}

MOTOR_TEST_CASE(un_npc_concret_dice_lo_que_dice_la_seva_carta) {
    const auto cat = carga_npcs();
    MOTOR_CHECK(cat.size() == 140);  // 134 el 07/09; 140 en el árbol del 08/10

    const auto* def = cat.find("npc_bastia_de_la_esquina");
    MOTOR_CHECK(def != nullptr);
    if (def == nullptr) return;
    MOTOR_CHECK(def->tier == 0);
}

// Barrido: tots tenen tier >= 0 i name (los que haya: el recuento lo
// vigila el test de arriba).
MOTOR_TEST_CASE(tots_els_npc_tenen_tier_i_nom) {
    const auto cat = carga_npcs();

    std::size_t ambNom = 0, ambTier = 0;
    cat.forEach([&](const RPG::NpcDefinition& def) {
        if (!def.name.empty()) ++ambNom;
        if (def.tier >= 0) ++ambTier;
    });
    MOTOR_CHECK(cat.size() > 0);
    MOTOR_CHECK(ambNom == cat.size());
    MOTOR_CHECK(ambTier == cat.size());
}

// ===================================================================
// D-07 (locations) — locationType, culture, alignment llegits.
// ===================================================================

RPG::Catalogs::Catalog<RPG::LocationDefinition> carga_locations() {
    RPG::Catalogs::Catalog<RPG::LocationDefinition> cat;
    auto r = cat.loadFromFile("assets/catalogs/locations.json");
    MOTOR_CHECK(r.isOk());
    return cat;
}

MOTOR_TEST_CASE(una_location_concreta_dice_lo_que_dice_la_seva_carta) {
    const auto cat = carga_locations();
    MOTOR_CHECK(cat.size() == 140);

    // Buscar una nation (té polygon)
    bool foundNation = false;
    bool foundCity = false;
    cat.forEach([&](const RPG::LocationDefinition& def) {
        if (def.kind == RPG::LocationDefinition::Kind::Nation && !foundNation) {
            foundNation = true;
            MOTOR_CHECK(!def.polygon.empty());
        }
        if (def.kind == RPG::LocationDefinition::Kind::City && !foundCity) {
            foundCity = true;
            MOTOR_CHECK(!def.controlledBy.empty());
        }
    });
    MOTOR_CHECK(foundNation);
    MOTOR_CHECK(foundCity);
}

// Barrido: les 140 locations tenen tier >= 0.
MOTOR_TEST_CASE(totes_les_locations_tenen_tier) {
    const auto cat = carga_locations();

    int ambTier = 0, ambControl = 0;
    cat.forEach([&](const RPG::LocationDefinition& def) {
        if (def.tier >= 0) ++ambTier;
        if (!def.controlledBy.empty()) ++ambControl;
    });
    MOTOR_CHECK(ambTier == 140);
    MOTOR_CHECK(ambControl > 100);  // La majoria tenen controlador
}

// ===================================================================
// D-07 (adventures) — questIds, requiredNpcIds, mainFactionId llegits.
// ===================================================================

RPG::Catalogs::Catalog<RPG::AdventureDefinition> carga_adventures() {
    RPG::Catalogs::Catalog<RPG::AdventureDefinition> cat;
    auto r = cat.loadFromFile("assets/catalogs/adventures.json");
    MOTOR_CHECK(r.isOk());
    return cat;
}

MOTOR_TEST_CASE(una_aventura_concreta_dice_lo_que_dice_la_seva_carta) {
    const auto cat = carga_adventures();
    MOTOR_CHECK(cat.size() == 78);

    const auto* def = cat.find("1");
    MOTOR_CHECK(def != nullptr);
    if (def == nullptr) return;
    MOTOR_CHECK(def->tier >= 0);
}

// Barrido: les 78 aventures tenen tier >= 0 i name.
MOTOR_TEST_CASE(totes_les_aventures_tenen_tier_i_nom) {
    const auto cat = carga_adventures();

    int ambNom = 0, ambTier = 0;
    cat.forEach([&](const RPG::AdventureDefinition& def) {
        if (!def.name.empty()) ++ambNom;
        if (def.tier >= 0) ++ambTier;
    });
    MOTOR_CHECK(ambNom == 78);
    MOTOR_CHECK(ambTier == 78);
}

// ===================================================================
// D-07 (loot) — `tierRange`/`gold`/`drops`/`clue` leídos.
// ===================================================================

LootTableCatalog carga_loot() {
    LootTableCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/loot_tables.json",
                                    CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    return cat;
}

// Adalid: tier real en `tierRange` (`tier` plano es 0), oro en
// `gold{min,max}` y pista. Con el cargador antiguo: rango 0/0, oro 0/0,
// sin pista ni drops.
MOTOR_TEST_CASE(adalid_del_rey_de_armas_dice_lo_que_dice_su_carta) {
    const LootTableCatalog cat = carga_loot();
    MOTOR_CHECK(cat.size() == 201);
    const RPG::LootTableDefinition* def = cat.find("loot_adalid_del_rey_de_armas");
    MOTOR_CHECK(def != nullptr);
    if (def == nullptr) return;

    MOTOR_CHECK(def->tierMin == 5);
    MOTOR_CHECK(def->tierMax == 5);
    MOTOR_CHECK(def->minGold == 300);
    MOTOR_CHECK(def->maxGold == 750);
    MOTOR_CHECK(def->clue == "clue_forjados_sin_amo");
    MOTOR_CHECK(!def->entries.empty());
    MOTOR_CHECK(def->drops.size() == def->entries.size());
    MOTOR_CHECK(def->drops.front().itemId == "pocion_de_vigor");
    MOTOR_CHECK(def->drops.front().chance == 100);
}

// Barrido: oro coherente (min<=max) en las 201, drops en paralelo a
// entries en las 201 y cada drop con item no vacío.
MOTOR_TEST_CASE(totes_les_taules_porten_or_i_drops_coherents) {
    const LootTableCatalog cat = carga_loot();

    int orOk = 0, paralelas = 0, itemsOk = 0;
    cat.forEach([&](const RPG::LootTableDefinition& def) {
        if (def.minGold <= def.maxGold) ++orOk;
        if (def.drops.size() == def.entries.size()) ++paralelas;
        bool tot = true;
        for (const auto& dp : def.drops) tot &= !dp.itemId.empty();
        if (tot) ++itemsOk;
    });
    MOTOR_CHECK(orOk == 201);
    MOTOR_CHECK(paralelas == 201);
    MOTOR_CHECK(itemsOk == 201);
}

// ===================================================================
// D-07 (trampas) — `detection`/`disarm`/`effect`/`reset` leídos.
// ===================================================================

TrapCatalog carga_traps() {
    TrapCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/traps.json",
                                    CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    return cat;
}

// Aguja del Cerrojo Mentiroso: detectar INT 0.5, desarmar DES 0.5 con
// herramientas de ladrón, y si falla la resistencia física, Envenenado.
// Con el cargador antiguo: stats vacíos, condición vacía, oneShot siempre.
MOTOR_TEST_CASE(aguja_del_cerrojo_dice_lo_que_dice_su_carta) {
    const TrapCatalog cat = carga_traps();
    MOTOR_CHECK(cat.size() == 51);
    const RPG::TrapDefinition* def = cat.find("aguja_del_cerrojo_mentiroso");
    MOTOR_CHECK(def != nullptr);
    if (def == nullptr) return;

    MOTOR_CHECK(def->detectStat == "INT");
    MOTOR_CHECK(std::fabs(def->detectCd - 0.5f) < 0.001f);
    MOTOR_CHECK(def->disarmStat == "DES");
    MOTOR_CHECK(std::fabs(def->disarmCd - 0.5f) < 0.001f);
    MOTOR_CHECK(!def->disarmTool.empty());
    MOTOR_CHECK(def->applyConditionId == "envenenado");
    MOTOR_CHECK(def->saveAttribute == "resistencia_fisica");
    MOTOR_CHECK(def->category == "mecánica");
}

// Barrido: las 51 detectan y desarman con stat, 26 aplican condición y
// solo las 20 "única" son de un uso (antes las 51).
MOTOR_TEST_CASE(totes_les_trampes_detecten_i_pocs_son_d_un_sol_us) {
    const TrapCatalog cat = carga_traps();

    int detecten = 0, desarmen = 0, ambCondicio = 0, unSolUs = 0;
    cat.forEach([&](const RPG::TrapDefinition& def) {
        if (!def.detectStat.empty()) ++detecten;
        if (!def.disarmStat.empty()) ++desarmen;
        if (!def.applyConditionId.empty()) ++ambCondicio;
        if (def.oneShot) ++unSolUs;
    });
    MOTOR_CHECK(detecten == 51);
    MOTOR_CHECK(desarmen == 51);
    MOTOR_CHECK(ambCondicio == 26);
    MOTOR_CHECK(unSolUs == 20);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
