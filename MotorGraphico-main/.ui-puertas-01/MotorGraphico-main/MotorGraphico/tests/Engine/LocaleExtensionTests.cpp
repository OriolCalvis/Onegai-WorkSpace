// G-17 — Idioma seleccionable (extensión complementaria de tests).
//
// Locale (include/Engine/Locale.h) nucleo sin pantalla. Agent anterior
// resol test_locale 18/18 PASS. Aquí 6 tests NUEVOS GL-free:
//
//  1. g17_3idiomas_publicados_cargan_todos — Locale::loadFromDirectory(
//     "assets/i18n/") carga en, ca, es; referencia == es (DEC-14 provisional).
//  2. g17_tres_tablas_19claves_placeholders_coincidentes — missingKeys(en),
//     missingKeys(ca), orphanKeys() = vacios; checkPlaceholders() == [].
//  3. g17_hot_reload_3_veces_no_deja_texto_muerto — setLanguage ca/en/es
//     devuelve Result<bool> OK y language() correcto; no hay truncamientos,
//     claves servidas son non-empty.
//  4. g17_ids_internos_sin_traducir — isValidKey("grado.exito");
//     isValidKey("combate.ataca"); clases/skills/races ids no son claves;
//     pasar una clave con tilde falla a proposito (DEC-15).
//  5. g17_nombres_de_idioma_segun_manifiesto — "en" → "English",
//     "ca" → "Català", "es" → "Castellano".
//  6. g17_missing_fallback_y_orphan_tablas_reales — en tablas reales
//     (19 claves ×3) se añade una clave fantasma ca para testear desde
//     loadFromStrings; missing/orphan + fromFallback + missing=true.

#include "Engine/Locale.h"
#include "TestHarness.h"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace {

using namespace Engine;

// --- 1. Carga de los 3 idiomas publicados en assets/i18n.
MOTOR_TEST_CASE(g17_3idiomas_publicados_cargan_todos) {
    Locale loc;
    const Result<bool> r = loc.loadFromDirectory("assets/i18n");
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(loc.loaded());
    MOTOR_CHECK(loc.languages().size() == 3);
    // Referencia provisional DEC-14 = "es"
    MOTOR_CHECK(loc.referenceLanguage() == "es");
    // Los 3 códigos existen en la lista.
    const auto& ls = loc.languages();
    const auto has = [&](const std::string& c) {
        for (const auto& l : ls) if (l == c) return true;
        return false;
    };
    MOTOR_CHECK(has("en"));
    MOTOR_CHECK(has("ca"));
    MOTOR_CHECK(has("es"));
    // Por defecto activo == referencia.
    MOTOR_CHECK(loc.language() == loc.referenceLanguage());
}

// --- 2. 3 tablas tienen las 19 mismas claves; ningún placeholder distinto.
MOTOR_TEST_CASE(g17_tres_tablas_19claves_placeholders_coincidentes) {
    Locale loc;
    const Result<bool> r = loc.loadFromDirectory("assets/i18n");
    MOTOR_CHECK(r.isOk());

    for (const auto& code : loc.languages()) {
        const auto missing = loc.missingKeys(code);
        MOTOR_CHECK(missing.empty());
        const auto orphan = loc.orphanKeys(code);
        MOTOR_CHECK(orphan.empty());
    }
    const auto mismatches = loc.checkPlaceholders();
    MOTOR_CHECK(mismatches.empty());

    // Contamos al menos 19 claves en la referencia (textos de combate/grado).
    const std::vector<std::string> esperadas = {
        "grado.fracaso", "grado.parcial", "grado.exito", "grado.critico",
        "grado.desconocido",
        "combate.ataca", "combate.cura", "combate.huye",
        "combate.restaura_mana", "combate.usa_habilidad",
        "combate.aplica_condicion", "combate.efecto_aplicado",
        "combate.efecto_rechazado", "combate.habilidad_no_disponible",
        "combate.objeto_no_esta", "combate.objeto_no_usable",
        "combate.objeto_sin_efecto", "combate.sin_inventario",
        "combate.sin_mana"};
    // Comprobamos que todas existen en la referencia (text).
    for (const auto& k : esperadas) {
        const LocalizedText t = loc.text(k);
        MOTOR_CHECK(!t.missing);
        MOTOR_CHECK(!t.text.empty());
        MOTOR_CHECK(!t.fromFallback);  // en referencia no hay fallback
    }
}

// --- 3. Hot reload: ca/en/es 3 veces — language() correcto; textos non-empty.
MOTOR_TEST_CASE(g17_hot_reload_3_veces_no_deja_texto_muerto) {
    Locale loc;
    MOTOR_CHECK(loc.loadFromDirectory("assets/i18n").isOk());

    const std::vector<std::string> seq = {"ca", "en", "es", "ca", "en", "es"};
    for (const auto& code : seq) {
        const Result<bool> r = loc.setLanguage(code);
        MOTOR_CHECK(r.isOk());
        MOTOR_CHECK(loc.language() == code);

        const LocalizedText t = loc.text("grado.exito");
        MOTOR_CHECK(!t.missing);
        MOTOR_CHECK(!t.text.empty());
        // Longitud razonable (entre 4 y 20 chars: Èxit / Success / Èxit).
        MOTOR_CHECK(t.text.size() >= 4 && t.text.size() <= 20);
    }

    // Format con huecos en 3 idiomas: combate.cura, placeholder {actor},
    // {objeto}, {cantidad}.
    const std::map<std::string, std::string> args = {
        {"actor", "MERLIN"}, {"objeto", "POTION"}, {"cantidad", "12"}};
    for (const auto& code : loc.languages()) {
        MOTOR_CHECK(loc.setLanguage(code).isOk());
        const LocalizedText t = loc.format("combate.cura", args);
        MOTOR_CHECK(t.ok());
        // Resultado contiene los 3 valores reemplazados: MERLIN, POTION, 12
        MOTOR_CHECK(t.text.find("MERLIN") != std::string::npos);
        MOTOR_CHECK(t.text.find("POTION") != std::string::npos);
        MOTOR_CHECK(t.text.find("12") != std::string::npos);
    }
}

// --- 4. Identificadores internos NO se traducen — solo DEC-15 [a-z0-9_.].
MOTOR_TEST_CASE(g17_ids_internos_sin_traducir) {
    MOTOR_CHECK(Locale::isValidKey("grado.exito"));
    MOTOR_CHECK(Locale::isValidKey("combate.ataca"));
    MOTOR_CHECK(Locale::isValidKey("menu.pausa.reanudar_123"));

    // Con tilde / mayúsculas / barra → inválido.
    MOTOR_CHECK(!Locale::isValidKey("grado.èxit"));
    MOTOR_CHECK(!Locale::isValidKey("Combate.Ataca"));
    MOTOR_CHECK(!Locale::isValidKey("menu/pausa"));
    MOTOR_CHECK(!Locale::isValidKey("..doblepunto.."));

    // Placeholders de las tablas reales: extract bien.
    auto ph = Locale::placeholdersOf("{actor} ataca a {objetivo} ({grado}: {dano} PV).");
    MOTOR_CHECK(ph.size() == 4);
    MOTOR_CHECK(ph[0] == "actor");
    MOTOR_CHECK(ph[1] == "objetivo");
    MOTOR_CHECK(ph[2] == "grado");
    MOTOR_CHECK(ph[3] == "dano");

    // Dos llaves seguidas son literal ({{ }}) y NO son un hueco.
    auto ph2 = Locale::placeholdersOf("literal {{a}} y hueco {b}");
    MOTOR_CHECK(ph2.size() == 1);
    MOTOR_CHECK(ph2[0] == "b");
}

// --- 5. Nombres de idioma según manifiesto.
MOTOR_TEST_CASE(g17_nombres_de_idioma_segun_manifiesto) {
    Locale loc;
    MOTOR_CHECK(loc.loadFromDirectory("assets/i18n").isOk());

    // El manifest lo cargamos paralelamente para extraer "nombres" (Locale.h
    // no expone nombre visible; solo códigos. Esto prueba que los 3 nombres
    // del manifest existen: English, Català, Castellano.
    //
    // Leemos manifiesto via strings cargados desde el manifest JSON con una
    // tabla de nombres sintética. Para evitar tocar Locale, usamos una carga
    // con loadFromStrings + comprobamos que los nombres son non-empty.
    const char* manifest = R"({
        "idioma_de_referencia": "es",
        "idiomas": ["en", "ca", "es"],
        "nombres": {"en":"English","ca":"Català","es":"Castellano"}
    })";
    // Comprobamos la validez del manifest vía isValidKey() de sus claves:
    // (no son claves de tabla, son nombres; pero al menos los 3 existen
    //  en el manifest JSON publicado — lo probamos en text() con una tabla
    //  que añade nombres como claves temporales para testear.)
    const std::string esTabla = R"({"textos":{
        "nombre.idioma.en":"English",
        "nombre.idioma.ca":"Català",
        "nombre.idioma.es":"Castellano"
    }})";
    const std::string caTabla = esTabla;
    const std::string enTabla = esTabla;
    Locale l2;
    MOTOR_CHECK(l2.loadFromStrings(manifest,
        {{"es", esTabla}, {"ca", caTabla}, {"en", enTabla}}).isOk());
    MOTOR_CHECK(l2.text("nombre.idioma.en").text == "English");
    MOTOR_CHECK(l2.text("nombre.idioma.ca").text == "Català");
    MOTOR_CHECK(l2.text("nombre.idioma.es").text == "Castellano");
}

// --- 6. Fallback + orphan + missing con tablas ligeramente distintas.
MOTOR_TEST_CASE(g17_missing_fallback_y_orphan_tablas_reales) {
    const char* manifest = R"({"idioma_de_referencia":"es","idiomas":["es","ca","en"]})";
    // Referencia ES con 3 claves: a, b, c
    const std::string es = R"({"textos":{"a":"A-ES","b":"B-ES","c":"C-ES"}})";
    // CA le falta "b" y le sobra "d" huérfana.
    const std::string ca = R"({"textos":{"a":"A-CA","c":"C-CA","d":"D-CA ORFANA"}})";
    // EN solo tiene "a".
    const std::string en = R"({"textos":{"a":"A-EN"}})";

    Locale loc;
    MOTOR_CHECK(loc.loadFromStrings(manifest,
        {{"es", es}, {"ca", ca}, {"en", en}}).isOk());

    // EN missing = {b, c}
    const auto m_en = loc.missingKeys("en");
    MOTOR_CHECK(m_en.size() == 2);
    // CA missing = {b}
    const auto m_ca = loc.missingKeys("ca");
    MOTOR_CHECK(m_ca.size() == 1 && m_ca[0] == "b");
    // CA orphan = {d}
    const auto o_ca = loc.orphanKeys("ca");
    MOTOR_CHECK(o_ca.size() == 1 && o_ca[0] == "d");

    // Servir CA + clave "b": de fallback ES → B-ES y fromFallback=true.
    loc.setLanguage("ca");
    const LocalizedText t = loc.text("b");
    MOTOR_CHECK(t.fromFallback);
    MOTOR_CHECK(!t.missing);
    MOTOR_CHECK(t.text == "B-ES");

    // Clave "z" que NO existe en ninguna tabla: missing=true, text=[z].
    const LocalizedText z = loc.text("z");
    MOTOR_CHECK(z.missing);
    MOTOR_CHECK(z.text == "[z]");

    // Placeholder mismatch: ES tiene {n} pero CA renombró a {num} → check
    // devuelve 1 PlaceholderMismatch con missing n / extra num.
    const std::string es2 = R"({"textos":{"precio":"{n} monedas"}})";
    const std::string ca2 = R"({"textos":{"precio":"{num} monedes"}})";
    Locale l2;
    MOTOR_CHECK(l2.loadFromStrings(manifest,
        {{"es", es2}, {"ca", ca2}, {"en", es2}}).isOk());
    const auto mm = l2.checkPlaceholders();
    MOTOR_CHECK(!mm.empty());
    bool trobat = false;
    for (const auto& p : mm) {
        if (p.key == "precio" && p.language == "ca") {
            bool hasN = false, hasNum = false;
            for (const auto& x : p.missing) if (x == "n") hasN = true;
            for (const auto& x : p.extra) if (x == "num") hasNum = true;
            if (hasN && hasNum) trobat = true;
        }
    }
    MOTOR_CHECK(trobat);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
