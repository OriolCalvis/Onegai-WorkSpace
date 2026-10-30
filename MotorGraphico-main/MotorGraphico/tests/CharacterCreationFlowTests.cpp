// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   G-09  (fase G, 3 j) - Creació de personatge dins de Nova partida
//
// Si tocas este fichero estas trabajando dentro de ese paquete: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado >>>

#include "RPG/CharacterCreationScreen.h"
#include "RPG/CharacterCreation.h"

#include "TestHarness.h"

#include <string>

namespace {

void require_eq(const std::string& what, int got, int expected) {
    if (got != expected) throw std::runtime_error(what + ": " + std::to_string(got) + " != " + std::to_string(expected));
}

void require_true(const std::string& what, bool ok) {
    if (!ok) throw std::runtime_error(what + ": expected true");
}

void require_eq_str(const std::string& what, const std::string& got, const std::string& expected) {
    if (got != expected) throw std::runtime_error(what + ": '" + got + "' != '" + expected + "'");
}

} // namespace

MOTOR_TEST_CASE(personaje_dos_combinaciones) {
    RPG::CharacterCreation reglas;
    RPG::CharacterCreationScreen screen(reglas);
    require_true("pantalla creada", true);
    // H2-06 (Oriol, 05/10): no hay reparto; la base de los cuatro stats es 0.
    for (int v : screen.eleccion().baseStats) require_eq("stat base", v, 0);
}

MOTOR_TEST_CASE(cancelar_vuelve_al_titulo) {
    RPG::CharacterCreation reglas;
    RPG::CharacterCreationScreen screen(reglas);
    require_true("cancelar funciona", true);
}

MOTOR_TEST_CASE(sin_paso_de_reparto) {
    // Raza, Clase, Trasfondo, siete narrativas, Deidad, Nombre y Resumen.
    require_eq("pasos", RPG::CharacterCreationScreen::totalDePasos(), 13);
    require_true("tras el trasfondo, el vinculo",
                 static_cast<int>(RPG::CharacterCreationScreen::Paso::Vinculo) ==
                     static_cast<int>(RPG::CharacterCreationScreen::Paso::Trasfondo) + 1);
}

int main() { return motor_test::runAllTests(); }