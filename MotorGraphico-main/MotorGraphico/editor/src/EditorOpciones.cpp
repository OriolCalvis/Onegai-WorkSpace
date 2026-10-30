// EST-26 (relevo A) — Implementacion. Ver la cabecera para el criterio.

#include "EditorOpciones.h"

#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace Editor {
namespace {

// std::stoi lanza, y su mensaje ("stoi: no conversion") no le dice nada a
// quien escribio mal una opcion. strtol no lanza y ademas dice DONDE dejo
// de entender, que es lo que permite exigir que se haya comido el texto
// entero: "--frames 12x" no es 12.
Result<int> EnteroDe(const std::string& texto, const std::string& opcion) {
    if (texto.empty()) {
        return Result<int>::Error(opcion + " espera un numero y no ha recibido nada");
    }
    errno = 0;
    char* fin = nullptr;
    const long valor = std::strtol(texto.c_str(), &fin, 10);
    if (fin == texto.c_str() || fin == nullptr || *fin != '\0') {
        return Result<int>::Error(
            opcion + " espera un numero entero y ha recibido '" + texto + "'");
    }
    if (errno == ERANGE || valor < std::numeric_limits<int>::min() ||
        valor > std::numeric_limits<int>::max()) {
        return Result<int>::Error(
            opcion + ": el numero '" + texto + "' no cabe en un entero");
    }
    return Result<int>::Ok(static_cast<int>(valor));
}

Result<float> DecimalDe(const std::string& texto, const std::string& opcion) {
    if (texto.empty()) {
        return Result<float>::Error(opcion + " espera un numero y no ha recibido nada");
    }
    errno = 0;
    char* fin = nullptr;
    const float valor = std::strtof(texto.c_str(), &fin);
    if (fin == texto.c_str() || fin == nullptr || *fin != '\0') {
        return Result<float>::Error(
            opcion + " espera un numero y ha recibido '" + texto + "'");
    }
    // NaN e infinito pasan strtof sin queja ("nan", "inf"): se paran aqui,
    // porque una escala de interfaz que no es finita revienta mas tarde y
    // lejos de donde se escribio.
    if (errno == ERANGE || !std::isfinite(valor)) {
        return Result<float>::Error(
            opcion + ": el numero '" + texto + "' no es utilizable");
    }
    return Result<float>::Ok(valor);
}

}  // namespace

const char* AyudaDeUso() {
    return "motor_editor [--proyecto ID | --load nivel.json] [--frames N] "
           "[--capture salida.ppm] [--ui-scale 1..2]\n";
}

Result<EditorOpciones> AnalizaArgumentos(int argc, char** argv) {
    EditorOpciones opciones;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help") {
            opciones.ayuda = true;
            return Result<EditorOpciones>::Ok(opciones);
        }
        if (arg == "--smoke-check") {
            opciones.smoke = true;
            continue;
        }
        // Las que llevan valor. Se comprueba ANTES de consumirlo: un
        // "--frames" al final de la linea no puede leer argv[argc].
        const bool conValor = arg == "--load" || arg == "--proyecto" ||
                              arg == "--frames" || arg == "--capture" ||
                              arg == "--ui-scale";
        if (!conValor) {
            return Result<EditorOpciones>::Error("Opcion desconocida: " + arg);
        }
        if (++i >= argc) {
            return Result<EditorOpciones>::Error("Falta valor para " + arg);
        }
        const std::string valor = argv[i];
        if (arg == "--load") {
            opciones.initial = valor;
        } else if (arg == "--proyecto") {
            opciones.projectId = valor;
        } else if (arg == "--capture") {
            opciones.screenshot = valor;
        } else if (arg == "--frames") {
            const auto leido = EnteroDe(valor, arg);
            if (!leido.isOk()) {
                return Result<EditorOpciones>::Error(leido.errorMessage());
            }
            opciones.frames = leido.value();
        } else {
            const auto leido = DecimalDe(valor, arg);
            if (!leido.isOk()) {
                return Result<EditorOpciones>::Error(leido.errorMessage());
            }
            opciones.uiScale = leido.value();
        }
    }

    // Las dos reglas que ya habia, con el motivo dicho en vez de un
    // "invalidos" que obligaba a adivinar cual de los dos era.
    if (opciones.frames == 0 || opciones.frames < -1) {
        return Result<EditorOpciones>::Error(
            "--frames espera un numero de fotogramas mayor que 0, o -1 para "
            "no poner limite; ha recibido " + std::to_string(opciones.frames));
    }
    if (opciones.uiScale < 1.0f || opciones.uiScale > 2.0f) {
        return Result<EditorOpciones>::Error(
            "--ui-scale espera un numero entre 1 y 2; por debajo la interfaz "
            "no se lee y por encima no cabe");
    }
    return Result<EditorOpciones>::Ok(opciones);
}

}  // namespace Editor
