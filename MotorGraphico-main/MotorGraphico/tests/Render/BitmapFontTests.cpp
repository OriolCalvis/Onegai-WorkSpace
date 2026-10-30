// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 2 paquetes del corte B:
//   C-06  (fase C, 1 j) - L'ajust de text no es penja
//   C-24  (fase C, 5 j) - Xarxa de seguretat al camí que es juga
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// C-06 — wrapText deja de colgarse con cajas muy estrechas.
//
// El bug: con una caja de UN glifo de ancho (maxWidth == glyphWidth), el
// corte de palabra tomaba maxGlyphs-1 == 0 glifos y anadia un '-' sin
// consumir nada de la palabra. El bucle no avanzaba nunca y el vector de
// salida crecia con guiones hasta agotar la memoria. Le llegaba cualquier
// HudText lo bastante estrecho.
//
// Unit y sin GL: wrapText/clipText/measureText son estaticas y puras (ver
// BitmapFont.h). El binario enlaza Texture/SpriteBatch/glad solo para
// resolver simbolos que no se llaman, igual que demo_bitmap_font.

#include "TestHarness.h"

#include "Render/BitmapFont.h"

#include <string>
#include <vector>

namespace {

const std::string kFrase = "EL CAMINO DE BOUNDINGTON ES LARGO Y ESTRECHO";

int glifos(const std::string& line) {
    // Cuenta codepoints UTF-8: las acentuadas ocupan dos bytes y un solo
    // glifo (mismo criterio que wrapText).
    int count = 0;
    for (unsigned char c : line) {
        if ((c & 0xC0) != 0x80) {
            ++count;
        }
    }
    return count;
}

}  // namespace

// El caso del bug. Antes de C-06 esta llamada no volvia: la prueba se
// quedaba colgada o moria por falta de memoria.
MOTOR_TEST_CASE(una_caja_de_un_glifo_no_cuelga_ni_inventa_guiones) {
    const std::vector<std::string> lines = BitmapFont::wrapText(kFrase, 1, 1);

    MOTOR_CHECK(!lines.empty());
    for (const std::string& line : lines) {
        MOTOR_CHECK(glifos(line) <= 1);
        MOTOR_CHECK(line != "-");  // no se inventan guiones donde no caben
    }
}

// Anchos 0, 1 y 2: los tres devuelven en tiempo finito y ninguno produce
// una linea mas ancha que la caja (salvo el 0, que por contrato del
// header es "no envolver").
MOTOR_TEST_CASE(anchos_degenerados_devuelven_en_tiempo_finito) {
    const std::vector<std::string> cero = BitmapFont::wrapText(kFrase, 0, 1);
    MOTOR_CHECK(cero.size() == 1);
    MOTOR_CHECK(cero[0] == kFrase);  // contrato documentado: <= 0 no envuelve

    for (int width = 1; width <= 2; ++width) {
        const std::vector<std::string> lines = BitmapFont::wrapText(kFrase, width, 1);
        MOTOR_CHECK(!lines.empty());
        for (const std::string& line : lines) {
            MOTOR_CHECK(glifos(line) <= width);
        }
    }
}

// Ninguna letra se pierde por el camino: cortar a un glifo por linea debe
// devolver el texto entero, sin espacios (que son el punto de corte).
MOTOR_TEST_CASE(cortar_a_un_glifo_no_pierde_letras) {
    const std::vector<std::string> lines = BitmapFont::wrapText("HOLA MUNDO", 1, 1);

    std::string juntas;
    for (const std::string& line : lines) {
        juntas += line;
    }
    MOTOR_CHECK(juntas == "HOLAMUNDO");
}

// Con sitio para el guion, el comportamiento de antes no cambia: una
// palabra larga se parte con guion al final de cada trozo.
MOTOR_TEST_CASE(con_sitio_para_el_guion_se_sigue_partiendo_con_guion) {
    const std::vector<std::string> lines = BitmapFont::wrapText("BOUNDINGTON", 40, 8);

    MOTOR_CHECK(lines.size() >= 2);
    MOTOR_CHECK(lines[0] == "BOUN-");
    for (const std::string& line : lines) {
        MOTOR_CHECK(glifos(line) <= 5);
    }
}

// Las acentuadas siguen contando como un glifo tras el arreglo (UTF-8:
// dos bytes, un caracter).
MOTOR_TEST_CASE(las_acentuadas_cuentan_como_un_glifo) {
    const std::vector<std::string> lines = BitmapFont::wrapText("MISSIÓ", 1, 1);

    MOTOR_CHECK(lines.size() == 6);
    for (const std::string& line : lines) {
        MOTOR_CHECK(glifos(line) == 1);
    }
}

int main() { return motor_test::runAllTests(); }
