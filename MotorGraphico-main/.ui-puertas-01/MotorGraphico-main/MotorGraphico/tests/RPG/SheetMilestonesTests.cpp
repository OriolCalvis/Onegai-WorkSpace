// H2-04 (Gantt Fase H, rodaja GL-free) — la progresión por hitos que el
// GDD manda ("no hay que inventar XP: el reglamento va de hitos") y que
// ya estaba escrita (CharacterSheet::tierUp) pero NADIE llamaba, junto
// con el arreglo del caché que la hacía peligrosa: subir de tier NO
// refrescaba cachedHealthCap, así quien leyera healthCap() tras avanzar
// veía la vida tope del tier VIEJO (Player::maxHealth() incluido — el
// mismo patrón de bug que H1-04 cerró para la creación de personaje).
//
// Nuevo en la ficha: `milestones` (total histórico) y grantMilestones(),
// el «único camino: mediante grantMilestone» que el comentario de tierUp
// prometía desde el corte B y nadie había construido. El conteo por
// aventura (grantMilestoneOnEnd) y su enganche en GameSession son de la
// sesión dueña de esa zona (EST-23b en vuelo): aquí se prueba la ficha.
//
// Unit y sin GL. El último caso carga una clase REAL de
// assets/catalogs/classes.json (camino del catálogo: baseHealth del
// ClassDefinition → cachedBaseHealth → refresco del cap sin rebuscar la
// clase); los demás construyen definiciones a mano con sumas exactas
// contra la tabla por defecto de TierRules (t1: bonus 0/cap 35, t2:
// 4/50, t3: 9/70).

#include "TestHarness.h"

#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/CharacterSheet.h"
#include "RPG/Definitions/RpgCoreDefinitions.h"
#include "RPG/TierRules.h"

namespace {

RPG::ClassDefinition claseDePrueba(int baseHealth) {
    RPG::ClassDefinition cls;
    cls.id = "guerrera_de_prova";
    cls.baseHealth = baseHealth;
    return cls;
}

// Ficha de tier 1 ya recomputada con su clase. con/car fijos para que
// las sumas sean exactas (CON*3 + CAR*1).
RPG::CharacterSheet ficha(const RPG::ClassDefinition& cls,
                          const RPG::TierRules& rules, int con, int car) {
    RPG::CharacterSheet s;
    s.tier = 1;
    s.classId = cls.id;
    s.baseStats = {con, 0, 0, car};
    s.recompute_derived(rules, &cls);
    return s;
}

}  // namespace

// El bug que cierra esta rodaja: conceder hitos y subir de tier deja el
// healthCap() al día, equivalente a una ficha gemela puesta a mano en el
// tier nuevo y recomputada con la misma clase.
MOTOR_TEST_CASE(h204_subir_de_tier_refresca_el_healthcap) {
    const RPG::TierRules rules;
    const auto cls = claseDePrueba(10);
    RPG::CharacterSheet s = ficha(cls, rules, 3, 3);
    const int capTier1 = s.healthCap();   // 10 + 9 + 3 + 0 = 22

    s.nivel = 20;  // NIVELES-01: T2 pide el nivel 20 y dos hitos
    const auto cambio = s.grantMilestones(2, rules);
    MOTOR_CHECK(cambio.advanced);
    MOTOR_CHECK(cambio.from == 1);
    MOTOR_CHECK(cambio.to == 2);
    MOTOR_CHECK(s.tier == 2);

    RPG::CharacterSheet gemela = ficha(cls, rules, 3, 3);
    gemela.tier = 2;
    gemela.recompute_derived(rules, &cls);
    MOTOR_CHECK(s.healthCap() == gemela.healthCap());  // 10+9+3+4 = 26
    MOTOR_CHECK(s.healthCap() != capTier1);
    MOTOR_CHECK(s.healthCap() == 26);
}

// Con el clamp del tier por medio también sigue al tier nuevo: en tier 1
// el total pisa el tope 35; en tier 2 el tope es 50 y entra entero.
MOTOR_TEST_CASE(h204_clamp_de_tier_sigue_al_tier_nuevo) {
    const RPG::TierRules rules;
    const auto cls = claseDePrueba(12);
    RPG::CharacterSheet s = ficha(cls, rules, 6, 6);
    MOTOR_CHECK(s.healthCap() == 35);     // 12+18+6 = 36 → clamped a 35

    s.nivel = 20;
    MOTOR_CHECK(s.grantMilestones(2, rules).advanced);
    MOTOR_CHECK(s.healthCap() == 40);     // 40 < tope 50: sin clamp

    RPG::CharacterSheet gemela = ficha(cls, rules, 6, 6);
    gemela.tier = 2;
    gemela.recompute_derived(rules, &cls);
    MOTOR_CHECK(s.healthCap() == gemela.healthCap());
}

// NIVELES-01 — Los dos hitos de T1 sin el nivel 20 no suben (se guardan); al
// llegar al 20 sube de una vez. Y sin ellos no se pasa del 20.
MOTOR_TEST_CASE(h204_hitos_insuficientes_acumulan_sin_subir) {
    const RPG::TierRules rules;
    const auto cls = claseDePrueba(10);
    RPG::CharacterSheet s = ficha(cls, rules, 3, 3);
    const int capInicial = s.healthCap();

    auto cambio = s.grantMilestones(2, rules);
    MOTOR_CHECK(!cambio.advanced);
    MOTOR_CHECK(s.tier == 1);
    MOTOR_CHECK(s.milestones == 2);
    MOTOR_CHECK(s.healthCap() == capInicial);

    cambio = s.grantLevels(19, rules);   // nivel 20 con los hitos: ahora sí
    MOTOR_CHECK(cambio.advanced);
    MOTOR_CHECK(s.tier == 2 && s.nivel == 20);

    RPG::CharacterSheet sinHito = ficha(cls, rules, 3, 3);
    MOTOR_CHECK(!sinHito.grantLevels(30, rules).advanced);
    MOTOR_CHECK(sinHito.nivel == 20 && sinHito.tier == 1);  // ni el 21 ni Tier 2
}

// Por cada tier se empieza de cero: los 2 hitos de T1 y el nivel 40 dejan en
// T2 (los de T2 aun no estan); con los 4 de T2, T3.
MOTOR_TEST_CASE(h204_varios_tiers_de_una_vez) {
    const RPG::TierRules rules;
    const auto cls = claseDePrueba(10);
    RPG::CharacterSheet s = ficha(cls, rules, 3, 3);

    s.grantMilestones(6, rules);  // los que sobran de T1 no pasan a T2
    MOTOR_CHECK(s.hitosTier == 2);
    s.grantLevels(39, rules);
    MOTOR_CHECK(s.tier == 2 && s.nivel == 40 && s.hitosTier == 0 && s.hitosQueFaltan() == 4);
    const auto cambio = s.grantMilestones(4, rules);
    MOTOR_CHECK(cambio.advanced);
    MOTOR_CHECK(cambio.from == 2);
    MOTOR_CHECK(cambio.to == 3);

    RPG::CharacterSheet gemela = ficha(cls, rules, 3, 3);
    gemela.tier = 3;
    gemela.recompute_derived(rules, &cls);
    MOTOR_CHECK(s.healthCap() == gemela.healthCap());  // 10+9+3+9 = 31
    MOTOR_CHECK(s.healthCap() == 31);
}

// En TIER_MAX no se sube por muchos hitos que entren; la ficha no cambia.
MOTOR_TEST_CASE(h204_techo_en_tier_maximo) {
    const RPG::TierRules rules;
    const auto cls = claseDePrueba(10);
    RPG::CharacterSheet s = ficha(cls, rules, 3, 3);
    s.tier = 5;
    s.recompute_derived(rules, &cls);
    const int capMitico = s.healthCap();

    const auto cambio = s.grantMilestones(50, rules);
    MOTOR_CHECK(!cambio.advanced);
    MOTOR_CHECK(cambio.from == 5 && cambio.to == 5);
    MOTOR_CHECK(s.tier == 5);
    MOTOR_CHECK(s.healthCap() == capMitico);
    MOTOR_CHECK(s.milestones > 0);   // se cuentan igual: son historia
}

// count 0 o negativo no desordena el contador ni el tier.
MOTOR_TEST_CASE(h204_count_cero_o_negativo_no_desordena) {
    const RPG::TierRules rules;
    const auto cls = claseDePrueba(10);
    RPG::CharacterSheet s = ficha(cls, rules, 3, 3);

    MOTOR_CHECK(!s.grantMilestones(0, rules).advanced);
    MOTOR_CHECK(!s.grantMilestones(-3, rules).advanced);
    MOTOR_CHECK(s.milestones == 0);
    MOTOR_CHECK(s.tier == 1);
}

// Ficha que nunca pasó por recompute_derived (cache a cero, baseHealth
// por defecto 8): conceder hitos la deja coherente igualmente.
MOTOR_TEST_CASE(h204_sin_recompute_previo_queda_coherente) {
    const RPG::TierRules rules;
    RPG::CharacterSheet s;                 // sin clase, sin recompute
    s.tier = 1;                            // (TIER_MIN es 0: no es lo que prueba esto)
    s.baseStats = {3, 0, 0, 3};
    s.nivel = 20;

    const auto cambio = s.grantMilestones(2, rules);
    MOTOR_CHECK(cambio.advanced);
    MOTOR_CHECK(s.tier == 2);
    MOTOR_CHECK(s.healthCap() == 24);      // 8 + 9 + 3 + 4 (bonus t2)
}

// El camino real: baseHealth leído del ClassDefinition de classes.json
// (63 clases, todas con baseHealth), no una definición a mano.
MOTOR_TEST_CASE(h204_clase_real_del_catalogo) {
    const RPG::TierRules rules;
    RPG::Catalogs::Catalog<RPG::ClassDefinition> clases;
    MOTOR_CHECK(clases.loadFromFile("assets/catalogs/classes.json").isOk());

    const RPG::ClassDefinition* primera = nullptr;
    clases.forEach([&](const RPG::ClassDefinition& d) {
        if (primera == nullptr && d.baseHealth > 0) primera = &d;
    });
    MOTOR_CHECK(primera != nullptr);

    RPG::CharacterSheet s = ficha(*primera, rules, 3, 2);
    MOTOR_CHECK(s.milestones == 0);

    s.nivel = 20;
    const auto cambio = s.grantMilestones(2, rules);
    MOTOR_CHECK(cambio.advanced);
    MOTOR_CHECK(s.tier == 2);

    RPG::CharacterSheet gemela = ficha(*primera, rules, 3, 2);
    gemela.tier = 2;
    gemela.recompute_derived(rules, primera);
    MOTOR_CHECK(s.healthCap() == gemela.healthCap());
}

int main() { return motor_test::runAllTests(); }

// NIVELES-01 — Veinte niveles por tier y los hitos del tier para pasar
// (Oriol, 06/10): 2 en T1, 4 en T2, 8 en T3 y 11 en T4, empezando de cero en
// cada tier; T5 es el techo. Un hito solo (Boundington) no basta para Tier 2.
MOTOR_TEST_CASE(niveles_veinte_por_tier_y_los_hitos_del_tier_para_pasar) {
    const RPG::TierRules rules;
    const auto cls = claseDePrueba(10);
    RPG::CharacterSheet s = ficha(cls, rules, 3, 3);
    MOTOR_CHECK(s.nivel == 1 && s.nivelMaximo() == 20 && s.hitosQueFaltan() == 2);
    for (int n = 0; n < 25; ++n) s.grantLevels(1, rules);
    MOTOR_CHECK(s.nivel == 20 && s.tier == 1);
    MOTOR_CHECK(!s.grantMilestones(1, rules).advanced && s.tier == 1);  // Boundington
    MOTOR_CHECK(s.grantMilestones(1, rules).advanced && s.tier == 2);   // el capitulo 2
    MOTOR_CHECK(s.hitosTier == 0 && s.hitosQueFaltan() == 4);
    for (int n = 0; n < 25; ++n) s.grantLevels(1, rules);
    MOTOR_CHECK(s.nivel == 40 && s.tier == 2 && s.nivelMaximo() == 40);
    s.grantMilestones(3, rules);
    MOTOR_CHECK(s.tier == 2 && s.nivelMaximo() == 40 && s.hitosQueFaltan() == 1);
    s.grantMilestones(1, rules);
    MOTOR_CHECK(s.tier == 3 && s.hitosQueFaltan() == 8);
    s.grantLevels(1, rules);
    MOTOR_CHECK(s.nivel == 41);
    // Fracciones: cuatro pasos de 25 puntos son un nivel.
    for (int n = 0; n < 3; ++n) s.grantLevelPoints(25, rules);
    MOTOR_CHECK(s.nivel == 41 && s.progresoNivel == 75);
    s.grantLevelPoints(25, rules);
    MOTOR_CHECK(s.nivel == 42 && s.progresoNivel == 0);
    // T4 con el 60 y 8 hitos de T3; T5 con el 80 y 11 de T4; de ahi, al 100.
    s.grantMilestones(8, rules);
    s.grantLevels(18, rules);
    MOTOR_CHECK(s.tier == 4 && s.nivel == 60 && s.hitosQueFaltan() == 11);
    s.grantMilestones(10, rules);
    s.grantLevels(30, rules);
    MOTOR_CHECK(s.tier == 4 && s.nivel == 80);  // falta uno
    s.grantMilestones(1, rules);
    MOTOR_CHECK(s.tier == 5 && s.hitosQueFaltan() == 0);
    s.grantLevels(50, rules);
    MOTOR_CHECK(s.nivel == 100 && s.tier == 5);
    MOTOR_CHECK(s.milestones == 2 + 4 + 8 + 11);

    // El guardado: la misma ficha rehecha desde tier, nivel e hitos.
    RPG::CharacterSheet otra = ficha(cls, rules, 3, 3);
    otra.restauraProgresion(3, 47, 38, 5, 11, rules);
    MOTOR_CHECK(otra.tier == 3 && otra.nivel == 47 && otra.progresoNivel == 38 && otra.hitosTier == 5 &&
                otra.milestones == 11);
    RPG::CharacterSheet gemela = ficha(cls, rules, 3, 3);
    gemela.tier = 3;
    gemela.recompute_derived(rules, &cls);
    MOTOR_CHECK(otra.healthCap() == gemela.healthCap());
}
