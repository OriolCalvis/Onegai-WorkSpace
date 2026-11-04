// PERGAMINO-01 — test_pergamino (1/2): las piezas del tema sin ventana.
//
// Materiales deterministas, lienzos, las dos voces de texto (medir, partir,
// recortar), la vuelta de hoja (angulos, encadenado, recorte contra las
// tapas) y la regla del aura de la moral. Las pantallas enteras (papiro,
// cuaderno, dialogo, retales) estan en PergaminoPantallasTests.cpp.
//
// Lee assets/textures/letras_pergamino.json desde build/ (copiar_assets).
#include <algorithm>
#include <cmath>
#include <string>

#include "Core/Imagen/ImagenRGBA.h"
#include "Game/AuraMoral.h"
#include "Game/Morality.h"
#include "Game/PasarHoja.h"
#include "HUD/Pergamino/FuenteAtlas.h"
#include "HUD/Pergamino/Lienzo.h"
#include "HUD/Pergamino/Materiales.h"
#include "HUD/Pergamino/Tema.h"
#include "TestHarness.h"

using namespace Pergamino;

namespace {

const FuenteAtlas& letras() {
    static FuenteAtlas atlas;
    static bool cargada = false;
    if (!cargada) {
        const auto r = atlas.carga("assets/textures/letras_pergamino.json");
        MOTOR_CHECK(r.isOk());
        cargada = true;
    }
    return atlas;
}

int alfa(const Imagen& img, int x, int y) { return img.pixel(x, y)[3]; }

}  // namespace

// --- Materiales -------------------------------------------------------------

MOTOR_TEST_CASE(material_misma_semilla_misma_imagen) {
    const Imagen a = fabrica(hoja(200, 80, 7));
    const Imagen b = fabrica(hoja(200, 80, 7));
    const Imagen c = fabrica(hoja(200, 80, 8));
    MOTOR_CHECK(a.ancho == 200 && a.alto == 80);
    MOTOR_CHECK(a.rgba == b.rgba);
    MOTOR_CHECK(a.rgba != c.rgba);
}

MOTOR_TEST_CASE(material_hoja_rasgada_por_el_borde_y_entera_por_dentro) {
    const Imagen img = fabrica(hoja(240, 120, 3, /*rasgado=*/2));
    int transparentesBorde = 0;
    for (int x = 0; x < img.ancho; ++x) transparentesBorde += alfa(img, x, 0) == 0 ? 1 : 0;
    MOTOR_CHECK(transparentesBorde > 0);  // el rasgado muerde el borde
    for (int y = 20; y < img.alto - 20; y += 7)
        for (int x = 20; x < img.ancho - 20; x += 11) MOTOR_CHECK(alfa(img, x, y) == 255);
}

MOTOR_TEST_CASE(material_piezas_con_su_medida) {
    for (Material m : {Material::Sello, Material::Chincheta, Material::Cinta, Material::Tachuela,
                       Material::Flecha}) {
        int w = 0, h = 0;
        medidaPieza(m, w, h);
        const Imagen img = fabrica(pieza(m));
        MOTOR_CHECK(w > 0 && h > 0);
        MOTOR_CHECK(img.ancho == w && img.alto == h);
    }
    int w = 0, h = 0;
    medidaPieza(Material::Cinta, w, h);
    MOTOR_CHECK(w == 14 && h == 28);  // el marcapaginas de las maquetas
}

MOTOR_TEST_CASE(material_papiro_de_cualquier_ancho) {
    for (int ancho : {480, 560}) {
        const Imagen img = fabrica(papiro(ancho, 640));
        MOTOR_CHECK(img.ancho == ancho && img.alto == 640);
        MOTOR_CHECK(alfa(img, ancho / 2, 320) == 255);
    }
}

// --- Lienzo -----------------------------------------------------------------

MOTOR_TEST_CASE(lienzo_un_material_repetido_se_fabrica_una_vez) {
    Lienzo l;
    const int a = l.pide(hoja(100, 50, 1));
    const int b = l.pide(hoja(100, 50, 1));
    const int c = l.pide(hoja(100, 50, 2));
    MOTOR_CHECK(a == b);
    MOTOR_CHECK(c != a);
    MOTOR_CHECK(l.materiales().size() == 2);
}

MOTOR_TEST_CASE(lienzo_sombra_debajo_y_atenuado) {
    Lienzo l;
    l.materialConSombra(hoja(100, 50, 1), Vector2{10.0f, 20.0f}, Color::kSombra);
    MOTOR_CHECK(l.cuantos() == 2);
    MOTOR_CHECK(l.trazos()[0].p[0].y > l.trazos()[1].p[0].y);  // la sombra, antes y 4 px abajo
    const std::size_t desde = l.cuantos();
    l.color(Rect{0, 0, 10, 10}, Vector4{1, 1, 1, 0.8f});
    l.atenua(desde, 0.5f);
    MOTOR_CHECK(std::fabs(l.trazos().back().color.w - 0.4f) < 1e-5f);
    MOTOR_CHECK(std::fabs(l.trazos()[1].color.w - 1.0f) < 1e-5f);  // lo de antes no se toca
}

// --- Las dos voces ----------------------------------------------------------

MOTOR_TEST_CASE(utf8_se_decodifica_y_lo_roto_sale_como_interrogacion) {
    const auto cps = decodificaUtf8("ñá€");
    MOTOR_CHECK(cps.size() == 3);
    MOTOR_CHECK(cps[0] == 0xF1 && cps[1] == 0xE1 && cps[2] == 0x20AC);
    const auto rotos = decodificaUtf8(std::string("a\xC3", 2));
    MOTOR_CHECK(rotos.size() == 2 && rotos[1] == '?');
    MOTOR_CHECK(mayuscula(0xF1) == 0xD1);  // ñ -> Ñ para la voz pixel
}

MOTOR_TEST_CASE(voz_pixel_avanza_seis_columnas_y_va_en_mayusculas) {
    const FuenteAtlas& f = letras();
    // 5 columnas de letra + 1 de aire; la ultima no lleva aire detras.
    MOTOR_CHECK(f.ancho(Letra::Pixel, "AB", 2) == (6 * 2) * 2 - 2);
    MOTOR_CHECK(f.ancho(Letra::Pixel, "ab", 2) == f.ancho(Letra::Pixel, "AB", 2));
    MOTOR_CHECK(f.ascenso(Letra::Pixel, 2) == 18);  // 9 filas: 7 de letra y 2 de tilde
}

MOTOR_TEST_CASE(lora_mide_crece_y_aplica_kerning) {
    const FuenteAtlas& f = letras();
    MOTOR_CHECK(f.ancho(Letra::Lora18, "") == 0);
    MOTOR_CHECK(f.ancho(Letra::Lora18, "Hablar") > f.ancho(Letra::Lora18, "Hab"));
    MOTOR_CHECK(f.ancho(Letra::Lora24, "Hablar") > f.ancho(Letra::Lora18, "Hablar"));
    MOTOR_CHECK(f.ancho(Letra::Lora18b, "Hablar") >= f.ancho(Letra::Lora18, "Hablar"));
    // «AV» se junta: el par pesa menos que las dos letras sueltas.
    MOTOR_CHECK(f.ancho(Letra::Lora22, "AV") < f.ancho(Letra::Lora22, "A") + f.ancho(Letra::Lora22, "V"));
    // Las letras del castellano y el catalan estan todas.
    std::vector<GlifoColocado> s;
    f.coloca(Letra::Lora18, "ñÑáéíóúàèòçü·«»", 0, 20, 1, s);
    MOTOR_CHECK(s.size() == 15);
}

MOTOR_TEST_CASE(lora_envuelve_sin_pasarse_y_sin_perder_palabras) {
    const FuenteAtlas& f = letras();
    const std::string texto = "Ceaseton, ciudad costera de Nefaria, año 756 del Despertar Elemental. "
                              "Una taberna cualquiera, una mesa con cuatro jarras.";
    const auto lineas = f.envuelve(Letra::Lora22, texto, 420.0f);
    MOTOR_CHECK(lineas.size() >= 2);
    std::string junto;
    for (const std::string& l : lineas) {
        MOTOR_CHECK(f.ancho(Letra::Lora22, l) <= 420);
        junto += (junto.empty() ? "" : " ") + l;
    }
    MOTOR_CHECK(junto == texto);
    // Una palabra mas larga que la caja se corta por letras.
    const auto cortada = f.envuelve(Letra::Lora22, "Anticonstitucionalmente", 80.0f);
    MOTOR_CHECK(cortada.size() > 1);
    for (const std::string& l : cortada) MOTOR_CHECK(f.ancho(Letra::Lora22, l) <= 80);
    // Los saltos de linea del texto se respetan.
    MOTOR_CHECK(f.envuelve(Letra::Lora18, "uno\ndos", 500.0f).size() == 2);
}

MOTOR_TEST_CASE(lora_recorta_con_puntos_suspensivos) {
    const FuenteAtlas& f = letras();
    const std::string r = f.recorta(Letra::Lora18b, "Los Perdidos de Boundington", 150.0f);
    MOTOR_CHECK(f.ancho(Letra::Lora18b, r) <= 150);
    MOTOR_CHECK(r.size() >= 3 && r.substr(r.size() - 3) == "\xE2\x80\xA6");
    MOTOR_CHECK(f.recorta(Letra::Lora18b, "Corto", 150.0f) == "Corto");
}

// --- La vuelta de hoja ------------------------------------------------------

MOTOR_TEST_CASE(pasar_hoja_angulos_de_la_maqueta) {
    using Game::PasarHoja;
    MOTOR_CHECK(PasarHoja::anguloEn(true, 0.0f) == 0.0f);
    MOTOR_CHECK(std::fabs(PasarHoja::anguloEn(true, 1.0f) - 180.0f) < 1e-3f);
    // Al caer acelera (0,25 t + 0,75 t^2): a mitad de tiempo va por menos de la mitad.
    MOTOR_CHECK(PasarHoja::anguloEn(true, 0.5f) < 90.0f);
    float anterior = -1.0f;
    for (int i = 0; i <= 20; ++i) {
        const float a = PasarHoja::anguloEn(true, static_cast<float>(i) / 20.0f);
        MOTOR_CHECK(a >= anterior);
        anterior = a;
    }
    // Al subir frena y rebota unos grados por debajo de 0 antes de asentarse.
    MOTOR_CHECK(std::fabs(PasarHoja::anguloEn(false, 0.0f) - 180.0f) < 1e-3f);
    MOTOR_CHECK(std::fabs(PasarHoja::anguloEn(false, 1.0f)) < 1e-3f);
    float minimo = 180.0f;
    for (int i = 0; i <= 100; ++i) minimo = std::min(minimo, PasarHoja::anguloEn(false, i / 100.0f));
    MOTOR_CHECK(minimo < 0.0f && minimo > -8.0f);
}

MOTOR_TEST_CASE(pasar_hoja_dura_360_ms_y_las_pulsaciones_encadenan) {
    Game::PasarHoja v;
    v.empieza(0, 1);
    MOTOR_CHECK(v.enCurso() && v.haciaDelante());
    v.avanza(0.20f);
    MOTOR_CHECK(v.enCurso());
    // Otra pulsacion a media vuelta: la nueva sale de la hoja de destino.
    v.empieza(0, 2);
    MOTOR_CHECK(v.desde() == 1 && v.hacia() == 2 && v.progreso() == 0.0f);
    v.avanza(0.35f);
    MOTOR_CHECK(v.enCurso());
    v.avanza(0.02f);
    MOTOR_CHECK(!v.enCurso() && v.angulo() == 0.0f);
    // Misma hoja: no hay vuelta.
    v.empieza(2, 2);
    MOTOR_CHECK(!v.enCurso());
}

MOTOR_TEST_CASE(pasar_hoja_animaciones_reducidas_es_un_fundido_corto) {
    Game::PasarHoja v;
    v.empieza(3, 2, /*reducido=*/true);
    MOTOR_CHECK(v.angulo() == 0.0f);  // no gira
    MOTOR_CHECK(v.opacidadReducida() == 0.0f);
    v.avanza(0.08f);
    MOTOR_CHECK(std::fabs(v.opacidadReducida() - 0.5f) < 0.01f);
    v.avanza(0.09f);
    MOTOR_CHECK(!v.enCurso());
}

MOTOR_TEST_CASE(pasar_hoja_tiras_cubren_la_hoja_y_no_pasan_del_recorte) {
    using Game::PasarHoja;
    const float x = 100.0f, y = 200.0f, w = 400.0f, h = 300.0f;
    // Plana (0 grados): las tiras son la hoja, de arriba (v=1) a la bisagra (v=0).
    const auto plana = PasarHoja::tiras(x, y, w, h, 0.0f, 10, 10000.0f);
    MOTOR_CHECK(plana.size() == 10);
    float arriba = 1e9f, abajo = -1e9f;
    for (const auto& t : plana)
        for (const Vector2& p : t.p) {
            arriba = std::min(arriba, p.y);
            abajo = std::max(abajo, p.y);
            MOTOR_CHECK(p.x >= x - 0.01f && p.x <= x + w + 0.01f);
        }
    MOTOR_CHECK(std::fabs(arriba - y) < 0.01f && std::fabs(abajo - (y + h)) < 0.01f);
    // Pasados los 90 grados se ve el dorso y cuelga hacia el jugador: el
    // recorte (el borde de las tapas) no deja que se salga por abajo.
    const float corte = y + h + 60.0f;
    const auto caida = PasarHoja::tiras(x, y, w, h, 160.0f, 24, corte);
    MOTOR_CHECK(!caida.empty());
    for (const auto& t : caida) {
        for (const Vector2& p : t.p) MOTOR_CHECK(p.y <= corte + 0.01f);
        MOTOR_CHECK(t.v0 >= -0.001f && t.v0 <= 1.001f && t.v1 >= -0.001f && t.v1 <= 1.001f);
    }
}

// --- El aura de la moral ----------------------------------------------------

MOTOR_TEST_CASE(aura_mismos_tramos_que_la_moral_del_juego) {
    for (int v = -100; v <= 100; ++v) {
        Morality m;
        m.record(v, "prueba");
        const std::string etiqueta = m.label();
        const Game::TramoMoral t = Game::tramoMoral(m.value());
        const char* esperado = t == Game::TramoMoral::Villano       ? "VILLANO"
                             : t == Game::TramoMoral::Desgraciado ? "DESGRACIADO"
                             : t == Game::TramoMoral::Canalla     ? "CANALLA"
                             : t == Game::TramoMoral::Neutral     ? "NEUTRAL"
                             : t == Game::TramoMoral::Respetado   ? "RESPETADO"
                             : t == Game::TramoMoral::Salvador    ? "SALVADOR"
                                                                  : "HÉROE";
        MOTOR_CHECK(etiqueta == esperado);
    }
    // MORAL-01: los cortes de D-MOR-03 y los de Oriol (08/10), simétricos.
    using T = Game::TramoMoral;
    MOTOR_CHECK(Game::tramoMoral(-75) == T::Villano && Game::tramoMoral(-74) == T::Desgraciado);
    MOTOR_CHECK(Game::tramoMoral(-60) == T::Desgraciado && Game::tramoMoral(-59) == T::Canalla);
    MOTOR_CHECK(Game::tramoMoral(-20) == T::Canalla && Game::tramoMoral(-19) == T::Neutral);
    MOTOR_CHECK(Game::tramoMoral(19) == T::Neutral && Game::tramoMoral(20) == T::Respetado);
    MOTOR_CHECK(Game::tramoMoral(59) == T::Respetado && Game::tramoMoral(60) == T::Salvador);
    MOTOR_CHECK(Game::tramoMoral(74) == T::Salvador && Game::tramoMoral(75) == T::Heroe);
}

MOTOR_TEST_CASE(aura_heroe_brilla_villano_humea_neutral_nada) {
    const Game::AspectoAura neutral = Game::aspectoAura(0);
    MOTOR_CHECK(!neutral.visible && neutral.alas == Game::Alas::Ninguna);
    MOTOR_CHECK(neutral.tinteSprite.x == 1.0f && neutral.tinteSprite.y == 1.0f && neutral.tinteSprite.z == 1.0f);
    // Héroe: halo grande, alas celestiales y motas de luz. Salvador: halo
    // grande. Respetado: halo tenue.
    const Game::AspectoAura heroe = Game::aspectoAura(90);
    MOTOR_CHECK(heroe.visible && heroe.luz && heroe.particulas > 0 && heroe.alas == Game::Alas::Celestiales);
    const Game::AspectoAura salvador = Game::aspectoAura(65);
    MOTOR_CHECK(salvador.visible && salvador.luz && salvador.alas == Game::Alas::Ninguna);
    const Game::AspectoAura respetado = Game::aspectoAura(50);
    MOTOR_CHECK(respetado.visible && respetado.luz && respetado.intensidad < heroe.intensidad);
    MOTOR_CHECK(respetado.radio < salvador.radio);
    // Canalla: humo rojo. Desgraciado: humo espeso y el sprite en sombra.
    // Villano: halo grande, alas demoníacas y motas de sombra.
    const Game::AspectoAura canalla = Game::aspectoAura(-30);
    const Game::AspectoAura desgraciado = Game::aspectoAura(-65);
    const Game::AspectoAura villano = Game::aspectoAura(-90);
    MOTOR_CHECK(canalla.visible && !canalla.luz && canalla.alas == Game::Alas::Ninguna);
    MOTOR_CHECK(desgraciado.visible && !desgraciado.luz && desgraciado.particulas > canalla.particulas);
    MOTOR_CHECK(desgraciado.tinteSprite.y < canalla.tinteSprite.y && canalla.tinteSprite.y < 1.0f);
    MOTOR_CHECK(villano.visible && !villano.luz && villano.motas && villano.alas == Game::Alas::Demoniacas);
    MOTOR_CHECK(villano.radio > desgraciado.radio);
    // Fuera de escala se comporta como el extremo.
    MOTOR_CHECK(Game::aspectoAura(500).tramo == Game::TramoMoral::Heroe);
    MOTOR_CHECK(Game::aspectoAura(-500).tramo == Game::TramoMoral::Villano);
}

MOTOR_TEST_CASE(aura_particulas_y_pulso_acotados) {
    for (const int valor : {-90, -65, -30, 90}) {
        const Game::AspectoAura a = Game::aspectoAura(valor);
        for (int i = 0; i < a.particulas; ++i)
            for (float s = 0.0f; s < 6.0f; s += 0.37f) {
                const Game::ParticulaAura p = Game::particulaAura(a, i, s);
                MOTOR_CHECK(p.alfa >= 0.0f && p.alfa <= 1.0f);
                MOTOR_CHECK(std::fabs(p.dx) < 2.0f && std::fabs(p.dy) < 3.0f);
            }
    }
    for (float s = 0.0f; s < 10.0f; s += 0.5f) MOTOR_CHECK(Game::pulsoAura(s) > 0.8f && Game::pulsoAura(s) < 1.2f);
}

// --- MORAL-02: los grados, las alas y los rayos ------------------------------

MOTOR_TEST_CASE(aura_quince_grados_por_lado_dentro_de_los_tramos) {
    // Los cortes: el primero de cada tramo abre un grado; simétricos.
    MOTOR_CHECK(Game::gradoAura(0) == 0 && Game::gradoAura(19) == 0 && Game::gradoAura(-19) == 0);
    MOTOR_CHECK(Game::gradoAura(20) == 1 && Game::gradoAura(59) == 4 && Game::gradoAura(60) == 5);
    MOTOR_CHECK(Game::gradoAura(74) == 7 && Game::gradoAura(75) == 8 && Game::gradoAura(100) == 15);
    MOTOR_CHECK(Game::gradoAura(500) == 15 && Game::gradoAura(-500) == -15);
    for (int v = -100; v <= 100; ++v) {
        MOTOR_CHECK(Game::gradoAura(-v) == -Game::gradoAura(v));
        if (v > -100) MOTOR_CHECK(Game::gradoAura(v) >= Game::gradoAura(v - 1));  // nunca baja al subir
        // Cada tramo, sus grados: Respetado 1-4, Salvador 5-7, Héroe 8-15.
        const int g = Game::gradoAura(v) < 0 ? -Game::gradoAura(v) : Game::gradoAura(v);
        using T = Game::TramoMoral;
        const T t = Game::tramoMoral(v);
        if (t == T::Neutral) MOTOR_CHECK(g == 0);
        if (t == T::Respetado || t == T::Canalla) MOTOR_CHECK(g >= 1 && g <= 4);
        if (t == T::Salvador || t == T::Desgraciado) MOTOR_CHECK(g >= 5 && g <= 7);
        if (t == T::Heroe || t == T::Villano) MOTOR_CHECK(g >= 8 && g <= 15);
    }
    // Héroe de +75 no es héroe de +100: cada grado brilla más que el anterior
    // y las alas crecen de nivel en nivel.
    float antes = 0.0f;
    for (int i = 0; i < Game::kGradosPorLado; ++i) {
        const Game::AspectoAura a = Game::aspectoAura(Game::kCorteGrado[i]);
        MOTOR_CHECK(a.grado == i + 1 && a.intensidad > antes);
        antes = a.intensidad;
        MOTOR_CHECK((a.alas == Game::Alas::Celestiales) == (a.grado >= 8));
        MOTOR_CHECK(a.nivelAlas == (a.grado >= 8 ? a.grado - 7 : 0));
        const Game::AspectoAura m = Game::aspectoAura(-Game::kCorteGrado[i]);
        MOTOR_CHECK(m.grado == -(i + 1) && (m.alas == Game::Alas::Demoniacas) == (i + 1 >= 8));
        MOTOR_CHECK(m.nivelAlas == a.nivelAlas && m.rayos == a.rayos);
    }
    // Los rayos: Salvador y Desgraciado desde su segundo grado.
    MOTOR_CHECK(Game::aspectoAura(60).rayos == 0 && Game::aspectoAura(65).rayos == 4);
    MOTOR_CHECK(Game::aspectoAura(70).rayos == 6);
    MOTOR_CHECK(Game::aspectoAura(50).rayos == 0 && Game::aspectoAura(80).rayos == 0);
}

MOTOR_TEST_CASE(aura_alas_hojas_y_celdas_del_atlas) {
    // «primer nomes 1 ala, despres 2 per cada banda, despres ja 4 despres 6 fins a 8».
    const int hojas[Game::kNivelesAla] = {1, 1, 2, 2, 4, 6, 8, 8};
    for (int n = 1; n <= Game::kNivelesAla; ++n) MOTOR_CHECK(Game::hojasPorAla(n) == hojas[n - 1]);
    MOTOR_CHECK(Game::hojasPorAla(0) == 0 && Game::hojasPorAla(9) == 0);
    MOTOR_CHECK(Game::filaAla(Game::Alas::Celestiales, 1) == 0 && Game::filaAla(Game::Alas::Celestiales, 8) == 7);
    MOTOR_CHECK(Game::filaAla(Game::Alas::Demoniacas, 1) == 8 && Game::filaAla(Game::Alas::Demoniacas, 8) == 15);
    MOTOR_CHECK(Game::filaAla(Game::Alas::Ninguna, 3) == -1 && Game::filaAla(Game::Alas::Celestiales, 0) == -1);
    for (float s = -3.0f; s < 6.0f; s += 0.11f) {
        MOTOR_CHECK(Game::frameAla(s) >= 0 && Game::frameAla(s) < Game::kFramesAla);
        MOTOR_CHECK(std::fabs(Game::bateAla(s)) <= 5.0f + 1e-4f);
    }
}

MOTOR_TEST_CASE(aura_alas_en_el_sprite_segun_hacia_donde_mira) {
    Game::AlaEnSprite alas[2];
    MOTOR_CHECK(Game::alasEnSprite(Game::aspectoAura(0), 17, 0.0f, alas) == 0);
    MOTOR_CHECK(Game::alasEnSprite(Game::aspectoAura(70), 17, 0.0f, alas) == 0);  // Salvador: sin alas
    for (const int valor : {75, 90, 100, -75, -100}) {
        const Game::AspectoAura a = Game::aspectoAura(valor);
        for (float s = 0.0f; s < 4.0f; s += 0.7f) {
            // De frente (17) y de espaldas (13): una a cada lado, simétricas
            // respecto a los hombros; detras del cuerpo de frente, delante de espaldas.
            for (const int frame : {17, 8, 13, 2}) {
                MOTOR_CHECK(Game::alasEnSprite(a, frame, s, alas) == 2);
                const bool espaldas = frame == 13 || frame == 2;
                MOTOR_CHECK(alas[0].delante == espaldas && alas[1].delante == espaldas);
                MOTOR_CHECK(alas[0].espejo && !alas[1].espejo);
                MOTOR_CHECK(alas[0].fila == Game::filaAla(a.alas, a.nivelAlas) && alas[0].alfa > 0.0f);
                for (int k = 0; k < 4; ++k) {
                    const int kk = k ^ 1;  // la esquina espejo: izquierda <-> derecha
                    MOTOR_CHECK(std::fabs((alas[0].x[kk] - Game::kHombroX) + (alas[1].x[k] - Game::kHombroX)) < 1e-3f);
                    MOTOR_CHECK(std::fabs(alas[0].y[kk] - alas[1].y[k]) < 1e-3f);
                    MOTOR_CHECK(std::fabs(alas[1].x[k] - Game::kHombroX) < 1.2f * Game::kCeldaAla);
                    MOTOR_CHECK(std::fabs(alas[1].y[k]) < 2.0f * Game::kCeldaAla);
                }
            }
            // De perfil: las dos hacia atras (espejo mirando a la derecha) y detras.
            for (const int frame : {15, 5}) {
                MOTOR_CHECK(Game::alasEnSprite(a, frame, s, alas) == 2);
                MOTOR_CHECK(alas[0].espejo && alas[1].espejo && !alas[0].delante && !alas[1].delante);
                const float ancho0 = std::fabs(alas[0].x[1] - alas[0].x[0]);
                const float ancho1 = std::fabs(alas[1].x[1] - alas[1].x[0]);
                // la lejana, mas estrecha
                MOTOR_CHECK(ancho0 < ancho1 && ancho1 < static_cast<float>(Game::kCeldaAla));
            }
            for (const int frame : {19, 11}) {
                MOTOR_CHECK(Game::alasEnSprite(a, frame, s, alas) == 2);
                MOTOR_CHECK(!alas[0].espejo && !alas[1].espejo && !alas[0].delante);
            }
        }
    }
}

// PL-JUEGO-01: con la hoja en diagonal de PixelLab (40 frames), las alas siguen hacia donde mira: la
// cercana casi entera y la lejana encogida y mas alta; mirando al sur, detras; al norte, delante.
MOTOR_TEST_CASE(aura_alas_en_diagonal) {
    using Game::Reparto;
    const Reparto d = Reparto::Diagonal;
    Game::AlaEnSprite alas[2];
    MOTOR_CHECK(Game::alasEnSprite(Game::aspectoAura(0), 13, 0.0f, alas, d) == 0);
    for (const int valor : {75, 92, 100, -80, -100}) {
        const Game::AspectoAura a = Game::aspectoAura(valor);
        // frames de andar y de respirar de los cuatro rumbos: noreste, sureste, suroeste, noroeste
        const int frames[8] = {1, 7, 13, 19, 25, 29, 33, 37};
        for (int i = 0; i < 8; ++i) {
            const int f = frames[i];
            const Game::Rumbo r = Game::rumboDeFrame(f, d);
            MOTOR_CHECK(Game::alasEnSprite(a, f, 1.3f, alas, d) == 2);
            const bool norte = r == Game::Rumbo::Arriba || r == Game::Rumbo::Izquierda;
            MOTOR_CHECK(alas[0].delante == norte && alas[1].delante == norte);
            // la lejana primero: mas estrecha y mas alta
            MOTOR_CHECK(alas[0].ancho < alas[1].ancho && alas[0].hombroY < alas[1].hombroY);
            MOTOR_CHECK(alas[0].espejo != alas[1].espejo);
            // la cercana va hacia la izquierda de la pantalla mirando al sureste y al noroeste
            const bool cercaIzquierda = r == Game::Rumbo::Derecha || r == Game::Rumbo::Izquierda;
            MOTOR_CHECK(alas[1].espejo == cercaIzquierda);
            MOTOR_CHECK(std::fabs(alas[1].hombroY - Game::kHombroDiagonalY) < 1e-3f);
        }
    }
}

MOTOR_TEST_CASE(aura_rayos_acotados) {
    for (const int valor : {65, 70, -65, -70}) {
        const Game::AspectoAura a = Game::aspectoAura(valor);
        MOTOR_CHECK(a.rayos > 0);
        for (int i = 0; i < a.rayos; ++i)
            for (float s = 0.0f; s < 8.0f; s += 0.61f) {
                const Game::RayoAura r = Game::rayoAura(a, i, s);
                MOTOR_CHECK(r.alfa > 0.0f && r.alfa <= 1.0f);
                for (int k = 0; k < 4; ++k) MOTOR_CHECK(std::fabs(r.x[k]) < 1.0f && std::fabs(r.y[k]) < 1.0f);
            }
        MOTOR_CHECK(Game::rayoAura(a, a.rayos, 0.0f).alfa == 0.0f);
    }
    MOTOR_CHECK(Game::rayoAura(Game::aspectoAura(90), 0, 0.0f).alfa == 0.0f);
}

MOTOR_TEST_CASE(aura_chispas_salen_de_las_plumas) {
    // Un atlas de juguete: en la fila 3, una pluma horizontal desde el hombro hacia fuera.
    imagen::ImagenRGBA atlas;
    atlas.ancho = Game::kCeldaAla * Game::kFramesAla;
    atlas.alto = Game::kCeldaAla * Game::kFilasAla;
    atlas.pixeles.assign(static_cast<std::size_t>(atlas.ancho) * static_cast<std::size_t>(atlas.alto) * 4, 0);
    const int fila = 3, largo = 40;
    for (int x = Game::kPivoteAlaX; x < Game::kPivoteAlaX + largo; ++x) {
        const std::size_t i = (static_cast<std::size_t>(fila * Game::kCeldaAla + Game::kPivoteAlaY) *
                                   static_cast<std::size_t>(atlas.ancho) + static_cast<std::size_t>(x)) * 4;
        atlas.pixeles[i + 3] = 255;
    }
    MOTOR_CHECK(Game::puntosDeAla(atlas, 2).empty());
    const std::vector<Game::PuntoAla> puntos = Game::puntosDeAla(atlas, fila);
    MOTOR_CHECK(!puntos.empty());
    for (const Game::PuntoAla& p : puntos)  // solo la mitad de fuera
        MOTOR_CHECK(p.x > Game::kPivoteAlaX + 0.4f * largo && std::fabs(p.y - Game::kPivoteAlaY - 0.5f) < 0.01f);
    MOTOR_CHECK(Game::puntosDeAla(imagen::ImagenRGBA{}, 0).empty());

    // Sin alas no hay chispas; con alas, mas en cada nivel, y se quedan cerca de la pluma.
    const int n = static_cast<int>(puntos.size());
    MOTOR_CHECK(Game::chispasPorAla(0) == 0 && Game::chispasPorAla(1) > 0);
    for (int nivel = 2; nivel <= Game::kNivelesAla; ++nivel)
        MOTOR_CHECK(Game::chispasPorAla(nivel) > Game::chispasPorAla(nivel - 1));
    MOTOR_CHECK(Game::chispaAla(Game::aspectoAura(70), 0, 1.0f, puntos.data(), n).alfa == 0.0f);
    MOTOR_CHECK(Game::chispaAla(Game::aspectoAura(100), 0, 1.0f, nullptr, 0).alfa == 0.0f);
    for (const int valor : {75, 100, -88}) {
        const Game::AspectoAura a = Game::aspectoAura(valor);
        const int total = Game::chispasPorAla(a.nivelAlas);
        MOTOR_CHECK(Game::chispaAla(a, total, 1.0f, puntos.data(), n).alfa == 0.0f);
        int vivas = 0;
        for (int i = 0; i < total; ++i)
            for (float s = 0.0f; s < 9.0f; s += 0.29f) {
                const Game::ChispaAla c = Game::chispaAla(a, i, s, puntos.data(), n);
                MOTOR_CHECK(c.alfa >= 0.0f && c.alfa <= 1.0f && c.tono >= 0 && c.tono < 3);
                MOTOR_CHECK(c.x > Game::kPivoteAlaX && c.x < Game::kPivoteAlaX + largo + 15.0f);
                MOTOR_CHECK(c.y <= Game::kPivoteAlaY + 0.5f && c.y > Game::kPivoteAlaY - 9.0f);  // suben
                if (c.alfa > 0.05f) ++vivas;
            }
        MOTOR_CHECK(vivas > 0);
    }

    // celdaASprite lleva las esquinas de la celda a las del cuadrilatero.
    Game::AlaEnSprite alas[2];
    for (const int frame : {17, 13, 15, 19})
        for (const float s : {0.0f, 0.7f, 1.9f}) {
            MOTOR_CHECK(Game::alasEnSprite(Game::aspectoAura(90), frame, s, alas) == 2);
            const float c = static_cast<float>(Game::kCeldaAla);
            for (const Game::AlaEnSprite& ala : alas) {
                const float cx[4] = {ala.espejo ? c : 0.0f, ala.espejo ? 0.0f : c, ala.espejo ? 0.0f : c,
                                     ala.espejo ? c : 0.0f};
                const float cy[4] = {0.0f, 0.0f, c, c};
                for (int k = 0; k < 4; ++k) {
                    float x = 0.0f, y = 0.0f;
                    Game::celdaASprite(ala, cx[k], cy[k], x, y);
                    MOTOR_CHECK(std::fabs(x - ala.x[k]) < 1e-3f && std::fabs(y - ala.y[k]) < 1e-3f);
                }
            }
        }
}

int main() { return motor_test::runAllTests(); }
