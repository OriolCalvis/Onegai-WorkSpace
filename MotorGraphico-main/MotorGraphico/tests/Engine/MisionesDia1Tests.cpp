// E-MISIONES-01 B — Una escena por mision del dia 1 (tools/matriz_misiones.py) y una prueba por cada salida que el explorador no ha visto.
//
// Cada escena juega el prologo y el dia 1 de verdad con el arnes de las demos (examples/demo_boundington_partida.h, que respeta el
// `ocultoSi` de cinematicas.json: no se habla con quien la escena esconde): entra en los niveles, habla con los PNJ, contesta lo que
// pide cada dialogo, y comprueba que la mision se ABRE y se CIERRA por su salida principal (los beats que tienen que saltar, las flags
// del cierre, las que no). Las escenas «salida» llegan a lo que el explorador (20 partidas) no vio nunca: la pifia, la derrota, la huida,
// la opcion que nadie eligio. Las tiradas se fuerzan con la suerte del arnes (Jugador::tiradas), como en MatrizDia1Tests.
// Mutacion por mision: una COPIA temporal del JSON del dia 1 a la que se le quita el efecto que enciende el cierre (o la salida) tiene
// que dejar la escena en rojo; el original no se toca.
// Sin GL ni ventana.
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <regex>
#include <string>

#include "TestHarness.h"
#include "fabricar_guardados.h"

namespace {

using namespace fabricar;
using demo_bnd::Suerte;

// Lo que se neutraliza en la copia: en el beat `beat` (y, si `opcion` no esta vacia, desde la opcion con ese texto), la primera flag
// `flag` que un EFECTO enciende (`"arg": "flag"` o una salida de tirada/combate `"flagXxx": "flag"`), no la que lee un `requires`.
struct Quita {
    const char* beat = "";
    const char* opcion = "";
    const char* flag = "";
    const char* hacia = "zz_mutada";
    int vez = 1;  // la n-esima opcion con ese texto
};

struct Esc {
    const char* id;
    const char* mision;
    bool principal;
    std::vector<std::string> pasos;  // E:nivel  H:pnj  N (anochece)  D (duerme)  Z:cinematica con zona  C (cinematica sin zona)  S:flag  V:var=n  R:flag (exige)
                                     // T:flagBotch=mala|regular|buena|critica  L:flagVictory (se pierde)  U:flagVictory (se huye)  G:mala|... (suerte de aqui en adelante)
    std::vector<std::string> dice;
    Suerte suerte;
    std::vector<std::string> beats, ve, noVe;
    Quita mut;
    std::vector<std::string> noBeats;  // beats que NO tienen que saltar (hallazgos: un beat tapado por otro)
};

std::vector<std::string> junta(std::vector<std::string> a, const std::vector<std::string>& b) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

Suerte suerteDe(const std::string& s) {
    return s == "critica" ? Suerte::Critica : s == "buena" ? Suerte::Buena : s == "regular" ? Suerte::Regular : Suerte::Mala;
}

bool tiene(const std::vector<std::string>& v, const std::string& x) { return std::find(v.begin(), v.end(), x) != v.end(); }

std::string leeJson(const char* dia) {
    std::string t;
    FILE* f = std::fopen((std::string("assets/adventures/boundington_") + dia + ".json").c_str(), "rb");
    require(f != nullptr);
    char buf[65536];
    std::size_t n;
    while ((n = std::fread(buf, 1, sizeof buf, f)) > 0) t.append(buf, n);
    std::fclose(f);
    return t;
}

demo_bnd::Guiones conQuita(const demo_bnd::Guiones& g, const Quita& q, const std::string& temporal) {
    std::string t = leeJson("primer_dia");
    const std::size_t b = t.find(std::string("\"id\": \"") + q.beat + "\"");
    require(b != std::string::npos);
    const std::size_t fin = t.find("\n   \"id\": \"", b + 1);  // el siguiente beat (los enemigos de un combate llevan «id» mas hondo)
    std::size_t o = b;
    if (!std::string(q.opcion).empty()) {
        o = t.find(std::string("\"texto\": \"") + q.opcion + "\"", b);
        for (int v = 1; v < q.vez && o != std::string::npos; ++v) o = t.find(std::string("\"texto\": \"") + q.opcion + "\"", o + 1);
        require(o != std::string::npos && o < fin);
    }
    const std::string quiero = std::string("\"") + q.flag + "\"";
    const std::regex efecto("(\"arg\"|\"flag[A-Za-z]+\")\\s*:\\s*$");
    std::size_t x = t.find(quiero, o);
    while (x != std::string::npos && x < fin) {
        const std::string antes = t.substr(x > 60 ? x - 60 : 0, x > 60 ? 60 : x);
        if (std::regex_search(antes, efecto)) break;
        x = t.find(quiero, x + 1);
    }
    if (x == std::string::npos || x >= fin) {
        std::fprintf(stderr, "  conQuita: no hay un efecto que encienda «%s» en %s / %s\n", q.flag, q.beat, q.opcion);
        require(false);
    }
    t.replace(x, quiero.size(), std::string("\"") + q.hacia + "\"");
    FILE* f = std::fopen(temporal.c_str(), "wb");
    require(f != nullptr);
    std::fwrite(t.data(), 1, t.size(), f);
    std::fclose(f);
    auto mutada = RPG::AdventureScript::loadFromFile(temporal);
    std::remove(temporal.c_str());
    require(mutada.isOk());
    demo_bnd::Guiones r = g;
    r.dia1 = mutada.value();
    return r;
}

struct Resultado {
    std::vector<std::string> fallos;
    std::vector<std::string> vistos;
};

// Juega la escena sobre los guiones `g` y devuelve lo que falla (vacio = la mision se abre y se cierra como dice la escena).
Resultado juega(const demo_bnd::Guiones& g, const Heroe& h, const Esc& e, bool mutada = false) {
    Resultado r;
    Partida p(g, h, e.suerte);
    demo_bnd::Jugador& j = p.j;
    j.vigilaSiempre = mutada && std::getenv("MISIONES_SIN_VIGILA") == nullptr;  // MISIONES_SIN_VIGILA=1 reproduce el arnes de antes (log de contraste)  // un cierre mutado (auto que no enciende su flag) no se repite 150 veces: la escena queda en rojo
    j.prefiere = e.dice;
    j.prefiere.insert(j.prefiere.end(), {"Ponerse en marcha", "Pagar la cama", "Subir a dormir"});
    j.prologo();
    fase(j, "dia1");
    int n = 0;
    for (const std::string& paso : e.pasos) {
        ++n;
        const char op = paso[0];
        const std::string arg = paso.size() > 2 ? paso.substr(2) : "";
        auto falla = [&](const std::string& m) { r.fallos.push_back("paso " + std::to_string(n) + " «" + paso + "»: " + m); };
        switch (op) {
            case 'E': j.entra(arg); break;
            case 'H': j.habla(arg); break;
            case 'N': j.anochece(); break;
            case 'D': j.duerme(); break;
            case 'Z': if (!j.pisaLaZona(arg)) falla("la cinematica no empieza"); break;
            case 'C': j.cinematica(GridCoord{-1, -1}); break;
            case 'K': j.drena(); break;
            case 'S': j.s.setFlag(arg); break;
            case 'V': j.s.setVariable(arg.substr(0, arg.find('=')), std::atoi(arg.substr(arg.find('=') + 1).c_str())); break;
            case 'R': if (!j.s.hasFlag(arg)) falla("no esta " + arg); break;
            case 'T': j.tiradas[arg.substr(0, arg.find('='))] = suerteDe(arg.substr(arg.find('=') + 1)); break;
            case 'L': j.pierde.insert(arg); break;
            case 'U': j.huyen.insert(arg); break;
            case 'G': j.suerte = suerteDe(arg); break;
            default: falla("paso desconocido");
        }
    }
    r.vistos = j.vistos;
    for (const std::string& b : e.beats)
        if (!j.visto(b)) r.fallos.push_back("no salto el beat " + b);
    for (const std::string& b : e.noBeats)
        if (j.visto(b)) r.fallos.push_back("salto el beat " + b);
    for (const std::string& f : e.ve)
        if (!j.s.hasFlag(f)) r.fallos.push_back("falta la flag " + f);
    for (const std::string& f : e.noVe)
        if (j.s.hasFlag(f)) r.fallos.push_back("sobra la flag " + f);
    return r;
}

// ----------------------------------------------------------------------------------------------------------------------------
// Los pasos que se repiten
// ----------------------------------------------------------------------------------------------------------------------------
const std::vector<std::string> kCama = {"E:interior_taberna", "H:skilla"};  // llega, paga la cama y sube a dormir; al amanecer baja: el desayuno
const std::vector<std::string> kManana = junta(kCama, {"E:ciudad_centro"});  // ... y sale a la plaza (beat_llegada: es de manana)
const std::vector<std::string> kTutorial = junta(kManana, {"E:interior_mercado", "H:tendero_mercado", "E:interior_joyeria", "H:joyero",
                                                            "E:interior_sastreria", "H:sastre", "E:ciudad_centro"});  // las tres presentaciones y la emboscada

// La guarida: la iglesia vieja (invitado o no), las dependencias de la secta y el santuario.
const std::vector<std::string> kDep = junta(kCama, {"E:interior_iglesia_s1", "E:dependencias_secta"});
const std::vector<std::string> kSantuario = junta(kCama, {"E:ciudad_centro", "H:sectario_perdido", "E:interior_iglesia_s1", "E:santuario_subterraneo"});
const std::vector<std::string> kDiceSantuario = {"Sembrar dudas en Verina", "Contárselo todo", "Mostrar interés [Persuasión]", "Cuenta conmigo.", "Dejar las armas",
                                                 "Mirar la estatua [Conocimiento arcano]", "Escuchar el canto", "Hablar con alguien [Persuasión]", "¿Qué mezclas?",
                                                 "¿Conoces a un caballero llamado Venides?", "Callar"};

const std::vector<Esc>& escenas() {
    // Orden de campos: id, mision, principal, pasos, dice, suerte, beats, ve, noVe, mutacion (beat, opcion, flag).
    static const std::vector<Esc> c = {
        // 1 --- La primera noche: cama y rumores
        {"llegada_cama", "llegada_cama", true, kCama, {}, Suerte::Buena,
         {"b0_llegada", "b0_rosita", "b0_subir"}, {"bnd0_noche", "bnd0_cama", "bnd0_a_dormir", "bnd0_dormido"}, {}, {"b0_subir", "", "bnd0_dormido"}},
        {"calle_sin_pagar", "llegada_cama", false, {"E:interior_taberna", "E:ciudad_centro"}, {}, Suerte::Buena,
         {"b0_llegada", "b0_calle_sin_cama"}, {"bnd0_cama", "bnd0_a_dormir", "bnd_skilla"}, {"bnd0_dormido"}, {}},
        {"rosita_perdidos", "llegada_cama", false, {"E:interior_taberna", "H:skilla"},
         {"Pagar la cama", "Preguntar por los Perdidos", "¿Qué se dice de los Perdidos?", "Preguntar por la Furia", "¿Siete trolls?", "Hablar de otra cosa", "Subir a dormir"}, Suerte::Buena,
         {"b0_rosita_n_perdidos", "b0_rosita_n_furia", "b0_subiendo_oido", "b0_subir_oido"}, {"bnd0_oido_por_devoto", "bnd_rumor", "bnd0_trolls_preguntado", "bnd0_dormido"}, {}, {}},
        // 2 --- Los vecinos de la ventana
        {"ventana", "ventana", true, {"E:interior_taberna", "H:bordadora"}, {"¿Qué es eso de las cortinas de luto?", "Volver a tu mesa"}, Suerte::Buena,
         {"b0_bordadora"}, {"bnd0_ventana_visto", "bnd0_ventana_t_cortinas"}, {"bnd0_ventana_abierta", "bnd0_ventana_en_charla"}, {"b0_ventana", "", "bnd0_ventana_visto"}},
        // 3 --- El rincon de los jornaleros
        {"jornaleros", "jornaleros", true, {"E:interior_taberna", "H:tomas_jornalero"}, {"Darle la razón: si dan pan, algo harán bien"}, Suerte::Buena,
         {"b0_tomas", "b0_rincon"}, {"bnd0_rincon_hecho", "bnd0_tomas_amigo", "bnd0_pista_iglesia", "bnd_rumor"}, {"bnd0_tomas_ofendido", "bnd0_rincon_abierto"},
         {"b0_rincon", "Darle la razón: si dan pan, algo harán bien", "bnd0_rincon_hecho"}},
        {"jornaleros_reirse", "jornaleros", false, {"E:interior_taberna", "H:tomas_jornalero", "H:tomas_jornalero"}, {"Reírte de él"}, Suerte::Buena,
         {"b0_rincon", "b0_tomas_ofendido"}, {"bnd0_rincon_hecho", "bnd0_tomas_ofendido"}, {"bnd0_tomas_amigo", "bnd0_pista_iglesia"}, {}},
        {"jornaleros_amigo_y_berto", "jornaleros", false, {"E:interior_taberna", "H:tomas_jornalero", "H:tomas_jornalero", "H:berto_jornalero", "H:berto_jornalero"},
         {"Darle la razón: si dan pan, algo harán bien"}, Suerte::Buena,
         {"b0_tomas_amigo", "b0_berto_despues", "b0_berto_ya"}, {"bnd0_berto_cunado", "bnd_rumor"}, {}, {}},
        {"jornaleros_otra_cosa", "jornaleros", false, {"E:interior_taberna", "H:tomas_jornalero", "H:parroquiano_humilde"}, {"Hablar de otra cosa"}, Suerte::Buena,
         {"b0_rincon", "beat_parroquiano"}, {"bnd0_rincon_amigos", "bnd_rumor_parroquiano", "bnd_rumor"}, {}, {}},
        // 4 --- El hombre de la capucha
        {"encapuchado", "encapuchado", true, {"E:interior_taberna", "H:encapuchado"}, {}, Suerte::Buena,
         {"b0_capucha"}, {"bnd0_capucha_vista"}, {}, {"b0_capucha", "", "bnd0_capucha_vista"}},
        {"encapuchado_te_mira", "encapuchado", false, {"E:interior_taberna", "H:skilla", "H:encapuchado", "H:encapuchado"},
         {"Hablar de otra cosa", "Preguntar por los Perdidos", "¿Qué se dice de los Perdidos?", "Nada más"}, Suerte::Buena,
         {"b0_capucha", "b0_capucha_te_mira"}, {"bnd0_capucha_te_mira", "bnd0_oido_por_devoto"}, {}, {}},
        // 5 --- El caballero que escribe
        {"venides_noche", "venides_noche", true, {"E:interior_taberna", "H:venides"}, {"¿Qué escribes?", "Dejarle con su cuaderno"}, Suerte::Buena,
         {}, {"bnd0_venides_visto", "bnd0_venides_t_escribes"}, {"bnd0_venides_en_charla"}, {"b0_venides", "", "bnd0_venides_visto"}},
        // 6 --- El desayuno con Venides
        {"desayuno", "desayuno", true, kCama, {"¿Qué trabajo?", "¿Qué sabes de los Perdidos?", "Ponerse en marcha"}, Suerte::Buena,
         {"b1_venides_llama", "b1_desayuno_v0"}, {"bnd1_desayuno_hecho", "bnd1_encargo_venides", "bnd1_desayuno_t_trabajo"}, {"bnd1_desayuno_abierta", "bnd1_desayuno_en_charla"},
         {"b1_desayuno_s1_a", "Ponerse en marcha", "bnd1_desayuno_hecho"}},
        // 7 --- Presentarte a los tenderos y la emboscada
        {"presentaciones", "presentaciones", true, kTutorial, {}, Suerte::Buena,
         {"beat_llegada", "tut_encargo", "tut_tendero", "tut_joyero", "tut_sastre", "tut_emboscada", "tut_victoria"},
         {"bnd_tut_tendero", "bnd_tut_joyero", "bnd_tut_sastre", "bnd_tut_saqueador_vencido", "bnd_tut_emboscada_resuelta", "bnd_tut_cerrado"}, {"bnd_tut_saqueador_gana"},
         {"tut_sastre", "", "bnd_tut_sastre"}},
        {"emboscada_derrota", "presentaciones", false, junta({"L:bnd_tut_saqueador_vencido"}, kTutorial), {}, Suerte::Buena,
         {"tut_emboscada", "tut_derrota"}, {"bnd_tut_saqueador_gana", "bnd_tut_cerrado"}, {"bnd_tut_saqueador_vencido"}, {}},
        {"emboscada_huida", "presentaciones", false, junta({"U:bnd_tut_saqueador_vencido"}, kTutorial), {}, Suerte::Buena,
         {"tut_emboscada", "tut_huida"}, {"bnd_tut_saqueador_huido", "bnd_tut_cerrado"}, {"bnd_tut_saqueador_vencido"}, {}},
        // 8 --- Recorrer la plaza
        {"plaza", "plaza", true, junta(kTutorial, {"E:interior_mercado", "E:ciudad_centro", "H:anciana", "H:vecino_de_plaza"}), {"¿Por qué hay tanto bordado negro?", "Dejarla en su portal"}, Suerte::Buena,
         {"tut_plaza_ya", "b1_vieja", "b1_vecina_plaza_pista"}, {"bnd1_plaza_explorada", "bnd1_vieja_visto", "bnd1_xila_pista_plaza"}, {},
         {"tut_plaza_ya", "", "bnd1_plaza_explorada"}},
        // 9 --- El carterista
        {"carterista", "carterista", true, junta(kTutorial, {"E:interior_mercado", "E:ciudad_centro"}), {"Dejarle ir"}, Suerte::Buena,
         {"b1_carterista", "b1_carterista_pillado"}, {"bnd1_carterista_resuelto"}, {"bnd1_carterista_pifia"}, {"b1_carterista_pillado", "", "bnd1_carterista_resuelto"}},
        {"carterista_robo", "carterista", false, junta({"T:bnd1_carterista_pifia=mala"}, junta(kTutorial, {"E:interior_mercado", "E:ciudad_centro"})), {}, Suerte::Buena,
         {"b1_carterista", "b1_carterista_robo"}, {"bnd1_carterista_resuelto", "bnd1_carterista_pifia"}, {"bnd1_pulga_abierta"}, {}},
        {"carterista_rapido", "carterista", false, junta({"V:heroe_des=3"}, junta(kTutorial, {"E:interior_mercado", "E:ciudad_centro"})), {}, Suerte::Buena,
         {"b1_carterista_rapido"}, {"bnd1_carterista_exito"}, {}, {}},
        // 10 --- Aigren
        {"aigren", "aigren", true, junta(kCama, {"E:interior_herreria", "H:aigren"}), {"Preguntarle por los Perdidos", "Convencerle de que son el mal [Persuasión]"}, Suerte::Buena,
         {"b1_aigren_perdidos", "b1_aigren_redimido"}, {"bnd1_aigren_c_exito", "bnd1_aigren_redimido", "bnd1_aigren_resuelto"}, {"bnd1_aigren_c_pifia"},
         {"b1_aigren_redimido", "", "bnd1_aigren_resuelto"}},
        {"aigren_discurso", "aigren", false, junta(kCama, {"E:interior_herreria", "H:aigren"}), {"Preguntarle por los Perdidos", "Convencerle de que son el mal [Persuasión]"}, Suerte::Regular,
         {"b1_aigren_perdidos", "b1_aigren_papel"}, {"bnd1_aigren_c_parcial", "bnd1_aigren_orden_pedida"}, {"bnd1_aigren_redimido", "bnd1_aigren_resuelto", "bnd1_aigren_c_pifia"}, {"b1_aigren_papel", "", "bnd1_aigren_orden_pedida"}, {"b1_aigren_discurso"}},
        // E-DISPARADORES-2 A (Oriol, 04/11): la pifia ya no pide el papel; recibe el discurso y se cierra. Papel solo con c_parcial, discurso solo con c_pifia.
        {"aigren_pifia_discurso", "aigren", false, junta(kCama, {"E:interior_herreria", "T:bnd1_aigren_c_pifia=mala", "H:aigren"}), {"Preguntarle por los Perdidos", "Convencerle de que son el mal [Persuasión]"}, Suerte::Buena,
         {"b1_aigren_perdidos", "b1_aigren_discurso"}, {"bnd1_aigren_c_pifia", "bnd1_aigren_resuelto"}, {"bnd1_aigren_c_parcial", "bnd1_aigren_orden_pedida", "bnd1_aigren_redimido"}, {"b1_aigren_discurso", "", "bnd1_aigren_resuelto"}, {"b1_aigren_papel"}},
        // 11 --- Luisarda
        {"luisarda", "luisarda", true, junta(kCama, {"E:interior_herreria", "H:aigren", "H:aigren", "E:interior_sastreria", "H:luisarda", "E:interior_taberna", "H:skilla"}),
         junta({"Ven a cenar a la taberna. Rosita te espera."}, {"¿Trabajas solo?", "Pues suena muy sano.", "Presionarle con su hijo [Intimidación]", "Busco antorchas.",
                                                                  "Volver a sacar las antorchas", "Hacerle creer que ya lo sabes todo [Engaño]", "Escuchar lo que te pide"}), Suerte::Buena,
         {"b1_luisarda_pide"}, {"bnd1_luisarda_con_rosita"}, {}, {"b1_luisarda_pide", "Ven a cenar a la taberna. Rosita te espera.", "bnd1_luisarda_con_rosita"}},
        // 12 --- Los sectarios de la plaza
        {"sectarios", "sectarios", true, junta(kCama, {"E:ciudad_centro", "H:sectario_perdido"}), {"Mostrar interés [Persuasión]", "Cuenta conmigo."}, Suerte::Buena,
         {"b1_secta"}, {"bnd1_invitado_guarida", "bnd1_sectarios_resueltos"}, {}, {"b1_sectarios_masacre", "", "bnd1_sectarios_resueltos"}},
        // 13 --- Xila
        {"xila", "xila", true, junta(kCama, {"E:ciudad_oeste", "H:xila", "E:interior_taberna", "H:skilla", "E:ciudad_oeste", "H:xila"}),
         {"Sentarte con él, sin decir nada", "Comer algo mientras pasa la mañana (5)", "Invitarle a un trago (1)", "Irte"}, Suerte::Buena,
         {"b1_xila_desconocido_tank", "b1_xila_tarde", "b1_xila_habla"}, {"bnd1_xila_conocido", "bnd1_xila_convidado", "bnd1_xila_resuelto"}, {},
         {"b1_xila_tarde", "Invitarle a un trago (1)", "bnd1_xila_convidado"}},
        // 14 --- La familia de la puerta azul
        {"familia", "familia", true, junta(kCama, {"E:interior_taberna", "H:skilla", "E:ciudad_oeste", "H:nina_del_gato"}), {"Comer algo mientras pasa la mañana (5)", "Calmarlos [Persuasión]"}, Suerte::Buena,
         {"b1_verdaderos"}, {"bnd1_verdaderos_intentado", "bnd1_verdaderos_exito"}, {}, {"b1_verdaderos", "Calmarlos [Persuasión]", "bnd1_verdaderos_exito"}},
        // 15 --- Las pruebas de la iglesia
        {"iglesia", "iglesia", true, {"E:interior_taberna", "H:tomas_jornalero", "E:interior_iglesia", "H:sacerdote"},
         {"Darle la razón: si dan pan, algo harán bien", "¿Qué pasa en esta iglesia?", "Irte"}, Suerte::Buena,
         {"beat_iglesia", "b1_sacerdote"}, {"bnd_iglesia_vista", "bnd_pruebas_suficientes", "bnd1_sabe_osario"}, {}, {"beat_iglesia", "", "bnd_pruebas_suficientes"}},
        // 16 --- La ronda de noche
        {"ronda_noche", "ronda_noche", true, junta(kManana, {"N", "E:interior_cuartel", "E:ciudad_este"}), {"Pasar sin que te vea [Sigilo]"}, Suerte::Buena,
         {"b1_cuartel_noche", "b1_vigilante"}, {"bnd1_cuartel_visto", "bnd1_vigilante_visto"}, {}, {"b1_cuartel_noche", "", "bnd1_cuartel_visto"}},
        // 17 --- Entrar en la guarida
        {"entrada_guarida", "entrada_guarida", true, junta(kCama, {"E:interior_iglesia_s1"}), {"Dejar las armas"}, Suerte::Buena,
         {"b1_guarida_puerta"}, {"bnd1_guarida_dentro"}, {}, {"b1_guarida_puerta", "Dejar las armas", "bnd1_guarida_dentro"}},
        {"guarida_herrero_exito", "entrada_guarida", false, junta(kCama, {"E:interior_iglesia_s1"}), {"«Vengo de parte del herrero.» [Engaño]"}, Suerte::Buena,
         {"b1_guarida_puerta", "b1_herrero_exito"}, {"bnd1_guarida_dentro", "bnd1_herrero_mentido"}, {}, {}},
        {"guarida_herrero_parcial", "entrada_guarida", false, junta(kCama, {"E:interior_iglesia_s1"}), {"«Vengo de parte del herrero.» [Engaño]"}, Suerte::Regular,
         {"b1_guarida_puerta", "b1_herrero_parcial"}, {"bnd1_guarida_dentro"}, {}, {}},
        {"guarida_herrero_pifia_bajar", "entrada_guarida", false, junta(kCama, {"T:bnd1_herrero_pifia=mala", "E:interior_iglesia_s1"}), {"«Vengo de parte del herrero.» [Engaño]", "Dejar las armas y bajar"}, Suerte::Buena,
         {"b1_guarida_puerta", "b1_herrero_pifia"}, {"bnd1_guarida_dentro"}, {}, {}},
        {"guarida_herrero_pifia_bravas", "entrada_guarida", false, junta(kCama, {"T:bnd1_herrero_pifia=mala", "E:interior_iglesia_s1"}), {"«Vengo de parte del herrero.» [Engaño]", "Apartarlos a las bravas"}, Suerte::Buena,
         {"b1_guarida_puerta", "b1_herrero_pifia", "b1_puerta_ganada"}, {"bnd1_puerta_ganada"}, {"bnd1_guarida_dentro"}, {}},
        // 18 --- Descubierto en la cripta (la sospecha llega a 3: se siembra, la cripta se juega de verdad en 17 y 22)
        {"descubierto_calma", "descubierto", true, junta(kCama, {"E:interior_iglesia_s1", "V:cripta_sospecha=3", "K"}), {"Dejar las armas", "«Sandes me ha mandado a mí.» [Engaño]"}, Suerte::Buena,
         {"b1_cripta_descubierto", "b1_calma_exito"}, {"bnd1_cripta_calmada", "bnd1_calma_cerrado"}, {"bnd1_cripta_capturado", "bnd1_cripta_descubierta"},
         {"b1_cripta_descubierto", "«Sandes me ha mandado a mí.» [Engaño]", "bnd1_calma_exito"}},
        {"descubierto_pifia", "descubierto", false, junta(kCama, {"E:interior_iglesia_s1", "V:cripta_sospecha=3", "K"}), {"Dejar las armas", "«Sandes me ha mandado a mí.» [Engaño]"}, Suerte::Mala,
         {"b1_cripta_descubierto", "b1_calma_pifia"}, {"bnd1_cripta_capturado", "bnd1_sandes_te_conoce"}, {"bnd1_cripta_descubierta"}, {}},
        {"descubierto_rendirse", "descubierto", false, junta(kCama, {"E:interior_iglesia_s1", "V:cripta_sospecha=3", "K"}), {"Dejar las armas", "Rendirte"}, Suerte::Buena,
         {"b1_cripta_descubierto"}, {"bnd1_cripta_capturado", "bnd1_sandes_te_conoce"}, {"bnd1_cripta_descubierta"}, {}},
        {"descubierto_correr_exito", "descubierto", false, junta(kCama, {"E:interior_iglesia_s1", "V:cripta_sospecha=3", "K"}), {"Dejar las armas", "Correr hacia la salida [Atletismo]"}, Suerte::Buena,
         {"b1_cripta_descubierto", "b1_carrera_exito"}, {"bnd1_cripta_huida", "bnd1_guarida_resuelta", "bnd1_cripta_hostil"}, {"bnd1_cripta_capturado"}, {}},
        {"descubierto_correr_parcial", "descubierto", false, junta(kCama, {"E:interior_iglesia_s1", "V:cripta_sospecha=3", "K"}), {"Dejar las armas", "Correr hacia la salida [Atletismo]"}, Suerte::Regular,
         {"b1_cripta_descubierto", "b1_carrera_parcial"}, {"bnd1_cripta_huida", "bnd1_guarida_resuelta"}, {"bnd1_cripta_capturado"}, {}},
        {"descubierto_correr_pifia", "descubierto", false, junta(kCama, {"E:interior_iglesia_s1", "V:cripta_sospecha=3", "K"}), {"Dejar las armas", "Correr hacia la salida [Atletismo]"}, Suerte::Mala,
         {"b1_cripta_descubierto", "b1_carrera_pifia"}, {"bnd1_cripta_capturado"}, {"bnd1_cripta_huida"}, {}},
        {"descubierto_pelear", "descubierto", false, junta(kCama, {"E:interior_iglesia_s1", "V:cripta_sospecha=3", "K"}), {"Dejar las armas", "Pelear"}, Suerte::Buena,
         {"b1_cripta_descubierto", "b1_cripta_ganada"}, {"bnd1_cripta_hostil", "bnd1_cripta_ganada"}, {"bnd1_cripta_capturado"}, {}},
        // 19, 22 --- La ceremonia y Verina/Edul: la cripta por la invitacion de los sectarios
        {"ceremonia", "ceremonia", true, junta(kCama, {"E:ciudad_centro", "H:sectario_perdido", "E:interior_iglesia_s1", "E:santuario_subterraneo"}),
         {"Sembrar dudas en Verina", "Contárselo todo", "Mostrar interés [Persuasión]", "Cuenta conmigo.", "Dejar las armas", "Mirar la estatua [Conocimiento arcano]",
          "Escuchar el canto", "Hablar con alguien [Persuasión]", "¿Qué mezclas?", "¿Conoces a un caballero llamado Venides?", "Callar"}, Suerte::Buena,
         {"b1_guarida_ceremonia", "b1_ceremonia", "b1_idolo_gurkazaal"}, {"bnd1_guarida_vista", "bnd1_ceremonia_visto"}, {}, {"b1_ceremonia", "Mirar la estatua [Conocimiento arcano]", "bnd1_idolo_exito"}},
        {"verina_edul", "verina_edul", true, junta(kCama, {"E:ciudad_centro", "H:sectario_perdido", "E:interior_iglesia_s1", "E:santuario_subterraneo"}),
         {"Sembrar dudas en Verina", "Contárselo todo", "Mostrar interés [Persuasión]", "Cuenta conmigo.", "Dejar las armas", "Mirar la estatua [Conocimiento arcano]",
          "Escuchar el canto", "Hablar con alguien [Persuasión]", "¿Qué mezclas?", "¿Conoces a un caballero llamado Venides?", "Callar"}, Suerte::Buena,
         {"b1_edul"}, {"bnd1_dudas_verina", "bnd1_verina_vista", "bnd1_guarida_resuelta"}, {"bnd1_unido_al_culto"}, {"b1_edul", "Sembrar dudas en Verina", "bnd1_dudas_verina"}},
        {"aigren_verina", "verina_edul", false, junta(kCama, {"E:interior_herreria", "H:aigren", "E:interior_iglesia_s1", "E:santuario_subterraneo"}),
         {"Sacar a Verina de aquí", "Contárselo todo", "Preguntarle por los Perdidos", "Decirle que te interesan", "Vengo de parte del herrero.", "Ir a por lo tuyo"}, Suerte::Buena,
         {"b1_guarida_por_aigren"}, {"bnd1_verina_salvada_guarida", "bnd1_guarida_resuelta", "bnd1_invitado_por_aigren"}, {"bnd1_unido_al_culto"}, {}},
        // 20 --- El archivo y las dependencias
        {"archivo", "archivo", true, junta(kCama, {"E:interior_iglesia_s1", "E:dependencias_secta", "H:almacen_perdidos"}), {"Dejar las armas"}, Suerte::Buena,
         {"b1_guarida_puerta", "b1_almacen"}, {"bnd1_almacen_visto", "bnd1_pista_aceite", "bnd1_sabe_barricadas"}, {}, {"b1_almacen", "", "bnd1_pista_aceite"}},
        {"archivo_estudio", "archivo", false, junta(kCama, {"E:interior_iglesia_s1", "E:dependencias_secta", "Z:cripta_estudio"}), {"Dejar las armas"}, Suerte::Buena,
         {"b1_guarida_puerta", "b1_estudio"}, {"bnd1_cripta_estudio", "bnd1_estudio_visto", "bnd1_verina_vista"}, {}, {"b1_estudio", "", "bnd1_estudio_visto"}},
        // 21 --- La camara honda
        {"camara_sereno", "camara_sereno", true, junta(kCama, {"E:ciudad_centro", "H:sectario_perdido", "E:interior_iglesia_s1", "E:santuario_subterraneo", "H:copista_cripta", "E:camara_profunda"}),
         {"¿Quién es Sandes?", "Me he perdido.", "Contárselo todo", "Mostrar interés [Persuasión]", "Cuenta conmigo.", "Dejar las armas"}, Suerte::Buena,
         {"b1_camara_sandes"}, {"bnd1_camara_vista", "bnd1_sandes_se_va"}, {}, {"b1_camara_sandes", "Me he perdido.", "bnd1_sandes_se_va"}},
        // 23 --- El informe a Venides
        {"informe_venides", "informe_venides", true, junta(kCama, {"E:ciudad_centro", "H:sectario_perdido", "E:interior_iglesia_s1", "E:santuario_subterraneo", "E:interior_taberna", "N", "H:venides"}),
         {"Sembrar dudas en Verina", "Contárselo todo", "Mostrar interés [Persuasión]", "Cuenta conmigo.", "Dejar las armas", "Hablar de lo que has averiguado", "Contarle lo que tienes"}, Suerte::Buena,
         {"b1_borracho"}, {"bnd1_venides_informe", "bnd1_venides_informe_hecho", "bnd_aviso_dado"}, {}, {"b1_borracho", "Hablar de lo que has averiguado", "bnd1_venides_informe"}},
        // 24 --- La pelea de la taberna
        {"pelea_taberna", "pelea_taberna", true, junta(kCama, {"E:interior_taberna", "D"}), {"Separarlos [pelea sin matar]", "Hacerle hablar", "¿Quién te manda?"}, Suerte::Buena,
         {"b1_pelea", "b1_pelea_apaleado"}, {"bnd1_pelea_lanzada", "bnd1_pelea_ganada", "bnd1_camorrista_visto"}, {"bnd1_pelea_perdida"}, {"b1_pelea", "Separarlos [pelea sin matar]", "bnd1_pelea_ganada"}},
        // 25 --- Cerrar el dia
        {"cierre_dia", "cierre_dia", true, junta(kCama, {"E:interior_taberna", "N", "H:venides", "D"}), {"Contárselo todo"}, Suerte::Buena,
         {"b1_sube_al_cuarto"}, {"bnd1_en_cuarto", "bnd_dia_cerrado"}, {}, {"cierre_silencio", "", "bnd_dia_cerrado"}},

        // ---- E-MISIONES-01b: las salidas que ni el explorador ni la primera pasada vieron (una escena por salida) ----
        // 8 --- tut_plaza (enter): el tutorial se cierra FUERA de la plaza y se pisa despues. En el juego la emboscada salta al entrar en la plaza y el
        // enter que se adelanta es b1_carterista (171, el mismo trigger que tut_plaza, 172): solo un enter por entrada, asi que tut_plaza_ya (auto) lo
        // sustituye siempre. Aqui el carterista ya esta visto (S) para dejar libre el enter y probar la salida de respaldo.
        {"plaza_entrar", "plaza", false, junta(kManana, {"E:interior_mercado", "S:bnd_tut_cerrado", "S:bnd_tut_emboscada_lanzada", "S:bnd1_carterista_visto", "E:ciudad_centro"}), {}, Suerte::Buena,
         {"tut_plaza"}, {"bnd1_plaza_explorada"}, {}, {"tut_plaza", "", "bnd1_plaza_explorada"}, {"tut_plaza_ya"}},
        // 11 --- Luisarda: el recado de las antorchas y el silencio pagado con pociones
        {"luisarda_recado", "luisarda", false, junta(kCama, {"S:bnd0_favor_luisarda", "E:interior_sastreria", "H:luisarda"}),
         {"Rosita dice que no vas a cenar.", "Que allí tienes un plato caliente.", "Busco antorchas.", "Volver a sacar las antorchas", "Insistir, de malos modos [Intimidación]"}, Suerte::Buena,
         {"b1_luisarda_tienda", "b1_luisarda_recado"}, {"bnd1_luisarda_vista", "bnd1_luisarda_asustada"}, {}, {}},
        {"luisarda_pociones_miedo", "luisarda", false,
         junta(kCama, {"S:bnd0_favor_luisarda", "E:interior_sastreria", "H:luisarda", "S:bnd1_luisarda_pide", "K", "H:luisarda"}),
         {"Rosita dice que no vas a cenar.", "Que allí tienes un plato caliente.", "Busco antorchas.", "Volver a sacar las antorchas", "Insistir, de malos modos [Intimidación]", "No diré nada. Tranquila."}, Suerte::Buena,
         {"b1_luisarda_recado", "b1_luisarda_pide", "b1_luisarda_pociones_miedo"}, {"bnd1_luisarda_silencio", "bnd1_luisarda_pociones"}, {"bnd1_luisarda_con_rosita"}, {}},
        {"luisarda_pociones", "luisarda", false, junta(kCama, {"S:bnd0_favor_luisarda", "E:interior_sastreria", "H:luisarda", "S:bnd1_luisarda_pide", "K", "H:luisarda"}),
         {"Rosita dice que no vas a cenar.", "Que allí tienes un plato caliente.", "Busco antorchas.", "Volver a sacar las antorchas",
          "Hacerle creer que ya lo sabes todo [Engaño]", "No diré nada. Tranquila."}, Suerte::Buena,
         {"b1_luisarda_pide", "b1_luisarda_pociones"}, {"bnd1_luisarda_silencio", "bnd1_luisarda_pociones"}, {"bnd1_luisarda_con_rosita", "bnd1_luisarda_asustada"},
         {"b1_luisarda_pociones", "", "bnd1_luisarda_pociones"}},
        // 12 --- Sectarios: la pelea perdida y el sigilo
        {"secta_ganan", "sectarios", false, junta(kCama, {"L:bnd1_secta_vencidos", "E:ciudad_centro", "H:sectario_perdido"}), {"Reírte de ellos", "Insistir en la burla"}, Suerte::Buena,
         {"b1_secta_burla", "b1_secta_ganan"}, {"bnd1_secta_ganan", "bnd1_secta_fin", "bnd1_sectarios_resueltos"}, {"bnd1_secta_vencidos"}, {"b1_secta_ganan", "", "bnd1_secta_fin"}},
        {"sectarios_seguidos", "sectarios", false, junta(kCama, {"E:ciudad_centro", "H:sectario_perdido", "H:sectario_perdido"}),
         {"Reírte de ellos", "Dejarlo ahí", "Seguirles de lejos [Sigilo]"}, Suerte::Buena,
         {"b1_secta_burla", "b1_sectarios_seguir", "b1_sectarios_seguidos"}, {"bnd_pista_sectarios", "bnd1_sectarios_resueltos"}, {"bnd1_invitado_guarida"}, {}},
        {"sectarios_te_ven", "sectarios", false, junta(kCama, {"E:ciudad_centro", "H:sectario_perdido", "H:sectario_perdido"}),
         {"Reírte de ellos", "Dejarlo ahí", "Seguirles de lejos [Sigilo]"}, Suerte::Mala,
         {"b1_secta_burla", "b1_sectarios_seguir", "b1_sectarios_te_ven"}, {"bnd1_sectarios_resueltos"}, {"bnd_pista_sectarios"}, {}},
        // 20 --- El archivo, el archivero y las literas (dependencias de la secta)
        {"archivo_cuentas", "archivo", false, junta(kDep, {"H:archivo_perdidos"}), {"Dejar las armas", "Leer de reojo [Investigación]", "Arrancar la página de las cuentas"}, Suerte::Buena,
         {"b1_archivo_vigilado", "b1_archivo_cuentas"}, {"bnd1_archivo_leido", "bnd1_sabe_barricadas", "bnd1_sabe_sereno"}, {"bnd1_archivo_cerrado"}, {"b1_archivo_cuentas", "", "bnd1_archivo_leido"}},
        {"archivo_parcial", "archivo", false, junta(kDep, {"H:archivo_perdidos"}), {"Dejar las armas", "Leer de reojo [Investigación]"}, Suerte::Regular,
         {"b1_archivo_vigilado", "b1_archivo_parcial"}, {"bnd1_archivo_cerrado", "bnd1_cripta_sospecha"}, {"bnd1_archivo_leido"}, {}},
        {"archivo_pifia", "archivo", false, junta(kDep, {"H:archivo_perdidos"}), {"Dejar las armas", "Leer de reojo [Investigación]"}, Suerte::Mala,
         {"b1_archivo_vigilado", "b1_archivo_pifia"}, {"bnd1_archivo_cerrado", "bnd1_cripta_sospecha"}, {"bnd1_archivo_leido"}, {}},
        {"archivo_libre", "archivo", false, junta(kDep, {"S:bnd1_archivero_fuera", "H:archivo_perdidos"}), {"Dejar las armas", "Registrar los cajones"}, Suerte::Buena,
         {"b1_archivo_libre", "b1_archivo_cuentas"}, {"bnd1_archivo_ok", "bnd1_archivo_leido"}, {"bnd1_cripta_sospecha"}, {}},
        {"archivero_engano_exito", "archivo", false, junta(kDep, {"H:archivero_cripta", "C", "C"}), {"Dejar las armas", "«Sandes te llama al santuario.» [Engaño]"}, Suerte::Buena,
         {"b1_archivero", "b1_archivero_eng_exito"}, {"bnd1_archivero_se_va", "bnd1_archivero_fuera"}, {"bnd1_cripta_sospecha"}, {"b1_archivero_eng_exito", "", "bnd1_archivero_se_va"}},
        {"archivero_engano_parcial", "archivo", false, junta(kDep, {"H:archivero_cripta", "C"}), {"Dejar las armas", "«Sandes te llama al santuario.» [Engaño]"}, Suerte::Regular,
         {"b1_archivero", "b1_archivero_eng_parcial"}, {"bnd1_archivero_se_va", "bnd1_cripta_sospecha"}, {}, {}},
        {"archivero_engano_pifia", "archivo", false, junta(kDep, {"H:archivero_cripta"}), {"Dejar las armas", "«Sandes te llama al santuario.» [Engaño]"}, Suerte::Mala,
         {"b1_archivero", "b1_archivero_eng_pifia"}, {"bnd1_cripta_sospecha"}, {"bnd1_archivero_se_va"}, {}},
        {"archivero_persuadir_exito", "archivo", false, junta(kDep, {"H:archivero_cripta"}), {"Dejar las armas", "«Quiero aprender a leerlas.» [Persuasión]"}, Suerte::Buena,
         {"b1_archivero", "b1_archivero_per_exito"}, {"bnd1_archivo_escrituras"}, {"bnd1_cripta_sospecha"}, {}},
        {"archivero_persuadir_parcial", "archivo", false, junta(kDep, {"H:archivero_cripta"}), {"Dejar las armas", "«Quiero aprender a leerlas.» [Persuasión]"}, Suerte::Regular,
         {"b1_archivero", "b1_archivero_per_parcial"}, {"bnd1_archivo_escrituras", "bnd1_cripta_sospecha"}, {}, {}},
        {"archivero_persuadir_pifia", "archivo", false, junta(kDep, {"H:archivero_cripta"}), {"Dejar las armas", "«Quiero aprender a leerlas.» [Persuasión]"}, Suerte::Mala,
         {"b1_archivero", "b1_archivero_per_pifia"}, {"bnd1_cripta_sospecha"}, {"bnd1_archivo_escrituras"}, {}},
        {"archivero_intimidar_exito", "archivo", false, junta(kDep, {"H:archivero_cripta"}), {"Dejar las armas", "«Lárgate.» [Intimidación]"}, Suerte::Buena,
         {"b1_archivero", "b1_archivero_int_exito"}, {"bnd1_archivero_se_va", "bnd1_cripta_sospecha"}, {}, {}},
        {"archivero_intimidar_parcial", "archivo", false, junta(kDep, {"H:archivero_cripta"}), {"Dejar las armas", "«Lárgate.» [Intimidación]"}, Suerte::Regular,
         {"b1_archivero", "b1_archivero_int_parcial"}, {"bnd1_archivero_se_va", "bnd1_cripta_sospecha"}, {}, {}},
        {"archivero_intimidar_pifia", "archivo", false, junta(kDep, {"H:archivero_cripta"}), {"Dejar las armas", "«Lárgate.» [Intimidación]"}, Suerte::Mala,
         {"b1_archivero", "b1_archivero_int_pifia"}, {"bnd1_cripta_sospecha"}, {"bnd1_archivero_se_va"}, {}},
        {"literas_tunica_exito", "archivo", false, junta(kDep, {"H:literas_novicios"}), {"Dejar las armas", "Coger una túnica [Sigilo]"}, Suerte::Buena,
         {"b1_literas", "b1_tunica_exito"}, {"bnd1_cripta_tunica", "bnd1_cripta_colado", "bnd1_tunica_cerrado"}, {"bnd1_cripta_sospecha"}, {"b1_tunica_exito", "", "bnd1_cripta_tunica"}},
        {"literas_tunica_parcial", "archivo", false, junta(kDep, {"H:literas_novicios"}), {"Dejar las armas", "Coger una túnica [Sigilo]"}, Suerte::Regular,
         {"b1_literas", "b1_tunica_parcial"}, {"bnd1_cripta_tunica", "bnd1_cripta_sospecha"}, {}, {}},
        {"literas_tunica_pifia", "archivo", false, junta(kDep, {"H:literas_novicios"}), {"Dejar las armas", "Coger una túnica [Sigilo]"}, Suerte::Mala,
         {"b1_literas", "b1_tunica_pifia"}, {"bnd1_literas_cerradas", "bnd1_cripta_sospecha"}, {"bnd1_cripta_tunica"}, {}},
        {"literas_libro", "archivo", false, junta(kDep, {"H:literas_novicios"}), {"Dejar las armas", "Mirar el libro de la almohada"}, Suerte::Buena,
         {"b1_literas"}, {"bnd1_cuaderno_verina"}, {"bnd1_cripta_tunica"}, {}},
        {"literas_vacias", "archivo", false, junta(kDep, {"S:bnd1_novicios_fuera", "H:literas_novicios"}), {"Dejar las armas", "Coger una túnica"}, Suerte::Buena,
         {"b1_literas_vacias"}, {"bnd1_cripta_tunica", "bnd1_cripta_colado"}, {}, {}},
        // 19 --- El idolo (pifia), el banco de Verina y los frascos de Edul, y las tres salidas de la pelea de la guarida
        {"idolo_misterio", "ceremonia", false, junta({"T:bnd1_idolo_pifia=mala"}, kSantuario), kDiceSantuario, Suerte::Buena,
         {"b1_ceremonia", "b1_idolo_misterio"}, {"bnd1_idolo_resuelto"}, {"bnd1_idolo_exito"}, {}},
        {"banco_verina", "verina_edul", false, junta({"T:bnd1_charla_pifia=regular"}, kSantuario), junta({"¿Qué lees?", "¿De dónde eres?", "Fingir que crees", "Sembrar dudas en Verina"}, kDiceSantuario), Suerte::Buena,
         {"b1_charla_verina", "b1_banco", "b1_edul"}, {"bnd1_banco_visto", "bnd1_verina_perishton", "bnd1_con_edul", "bnd1_guarida_resuelta"}, {"bnd1_frascos_abierta"}, {}},
        {"frascos_edul", "verina_edul", false, kSantuario, junta({"¿Qué mezclas?", "¿Conoces a un caballero llamado Venides?", "Callar", "Sembrar dudas en Verina"}, kDiceSantuario), Suerte::Buena,
         {"b1_charla_edul", "b1_frascos", "b1_edul"}, {"bnd1_frascos_visto", "bnd1_edul_mezcla", "bnd1_edul_venides", "bnd1_con_edul"}, {"bnd1_banco_abierta"}, {}},
        {"guarida_ganada", "verina_edul", false, junta({"T:bnd1_charla_pifia=regular"}, kSantuario), junta({"Fingir que crees", "Plantarles cara"}, kDiceSantuario), Suerte::Buena,
         {"b1_banco", "b1_edul", "b1_guarida_ganada"}, {"bnd1_guarida_ganada", "bnd1_guarida_resuelta", "bnd1_guarida_pelea"}, {"bnd1_guarida_perdida"}, {"b1_guarida_ganada", "", "bnd1_guarida_resuelta"}},
        {"guarida_perdida", "verina_edul", false, junta({"T:bnd1_charla_pifia=regular", "L:bnd1_guarida_ganada"}, kSantuario), junta({"Fingir que crees", "Plantarles cara"}, kDiceSantuario), Suerte::Buena,
         {"b1_banco", "b1_edul", "b1_guarida_perdida"}, {"bnd1_guarida_perdida", "bnd1_guarida_resuelta"}, {"bnd1_guarida_ganada"}, {"b1_guarida_perdida", "", "bnd1_guarida_resuelta"}},
        {"guarida_exterminada", "verina_edul", false, junta(kCama, {"E:interior_herreria", "H:aigren", "E:interior_iglesia_s1", "E:santuario_subterraneo"}),
         {"Intentar acabar con todos", "Contárselo todo", "Preguntarle por los Perdidos", "Decirle que te interesan", "Vengo de parte del herrero.", "Ir a por lo tuyo"}, Suerte::Buena,
         {"b1_guarida_por_aigren", "b1_guarida_exterminada"}, {"bnd1_guarida_exterminada", "bnd1_guarida_resuelta", "bnd1_sabe_sandes"}, {"bnd1_guarida_perdida"}, {"b1_guarida_exterminada", "", "bnd1_guarida_resuelta"}},
        // 22 --- Edul baja a por ti si te encadenan, y la llave de las cadenas del sereno
        {"edul_rescate", "verina_edul", false, junta(kCama, {"E:interior_iglesia_s1", "V:cripta_sospecha=3", "S:bnd1_edul_hablado", "K", "C", "C"}), {"Dejar las armas", "Rendirte", "Esperar a Edul", "Subir con él"}, Suerte::Buena,
         {"b1_cripta_descubierto", "b1_capturado", "b1_edul_rescate"}, {"bnd1_cine_edul_rescate", "bnd1_capturado_resuelto", "bnd1_edul_escolta_si", "bnd1_llave_cadenas"}, {}, {"b1_edul_rescate", "", "bnd1_capturado_resuelto"}},
        {"llave_exito", "verina_edul", false, junta(kSantuario, {"S:bnd1_sabe_sereno", "E:dependencias_secta", "H:edul_cripta"}),
         junta({"Fingir que crees", "Sembrar dudas en Verina", "Hay un hombre encadenado abajo.", "Dame la llave. [Persuasión]"}, kDiceSantuario), Suerte::Buena,
         {"b1_edul", "b1_edul_cripta", "b1_llave_exito"}, {"bnd1_llave_cerrado", "bnd1_llave_cadenas"}, {"bnd1_edul_retenido"}, {"b1_llave_exito", "", "bnd1_llave_cadenas"}},
        {"llave_parcial", "verina_edul", false, junta(kSantuario, {"S:bnd1_sabe_sereno", "E:dependencias_secta", "H:edul_cripta"}),
         junta({"Fingir que crees", "Sembrar dudas en Verina", "Hay un hombre encadenado abajo.", "Dame la llave. [Persuasión]"}, kDiceSantuario), Suerte::Regular,
         {"b1_llave_parcial"}, {"bnd1_llave_cerrado", "bnd1_llave_cadenas", "bnd1_cripta_sospecha"}, {"bnd1_edul_retenido"}, {}},
        {"llave_pifia", "verina_edul", false, junta(kSantuario, {"S:bnd1_sabe_sereno", "E:dependencias_secta", "H:edul_cripta"}),
         junta({"Fingir que crees", "Sembrar dudas en Verina", "Hay un hombre encadenado abajo.", "Dame la llave. [Persuasión]"}, kDiceSantuario), Suerte::Mala,
         {"b1_llave_pifia"}, {"bnd1_llave_cerrado", "bnd1_edul_retenido"}, {"bnd1_llave_cadenas"}, {}},
    };
    return c;
}

}  // namespace

MOTOR_TEST_CASE(cada_mision_del_dia1_se_abre_y_se_cierra) {
    const demo_bnd::Guiones g = demo_bnd::cargaTodo();
    const Heroe h = creaHeroe();
    const char* solo = std::getenv("MISIONES_SOLO");
    int jugadas = 0;
    for (const Esc& e : escenas()) {
        if (solo != nullptr && std::string(solo) != e.id) continue;
        ++jugadas;
        const Resultado r = juega(g, h, e);
        if (std::getenv("MISIONES_VERBOSO") != nullptr) {
            std::fprintf(stderr, "  [%s] vistos:", e.id);
            for (const std::string& b : r.vistos) std::fprintf(stderr, " %s", b.c_str());
            std::fprintf(stderr, "\n");
        }
        for (const std::string& f : r.fallos) std::fprintf(stderr, "  [%s] %s\n", e.id, f.c_str());
        MOTOR_CHECK(r.fallos.empty());
    }
    MOTOR_CHECK(jugadas > 0);
}

MOTOR_TEST_CASE(sin_el_efecto_de_cierre_la_escena_de_la_mision_falla) {
    const demo_bnd::Guiones g = demo_bnd::cargaTodo();
    const Heroe h = creaHeroe();
    const char* solo = std::getenv("MISIONES_SOLO");
    for (const Esc& e : escenas()) {
        if (std::string(e.mut.beat).empty() || (solo != nullptr && std::string(solo) != e.id)) continue;
        const std::string temporal = (std::filesystem::temp_directory_path() / (std::string("misiones_mut_") + e.id + ".json")).string();
        const demo_bnd::Guiones mutada = conQuita(g, e.mut, temporal);
        const Resultado r = juega(mutada, h, e, true);
        if (r.fallos.empty()) std::fprintf(stderr, "  [%s] la mutacion (%s / %s / %s) no hizo fallar nada\n", e.id, e.mut.beat, e.mut.opcion, e.mut.flag);
        else if (std::getenv("MISIONES_VERBOSO") != nullptr) std::fprintf(stderr, "  [%s] mutacion en rojo: %s%s\n", e.id, r.fallos.front().c_str(), r.fallos.size() > 1 ? " (+mas)" : "");
        MOTOR_CHECK(!r.fallos.empty());
    }
}

int main() { return motor_test::runAllTests(); }
