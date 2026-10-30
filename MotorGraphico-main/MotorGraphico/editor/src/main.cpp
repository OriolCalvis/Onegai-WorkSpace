// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-10  (fase E, 6 j) - Espai de treball visual i controls coherents
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "EditorApplication.h"

#include "Core/Log/ConsoleSink.h"
#include "Core/Log/FileSink.h"
#include "Core/Log/LogConfig.h"
#include "Core/Log/Logger.h"

#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// EST-11 - Frontera de excepciones del editor. Aqui llega todo lo que
// EditorApplication::run() lanza (un argumento invalido, la ventana, el
// contexto GL, un error de OpenGL) y aqui se convierte en un codigo de salida:
// es exactamente el sitio donde la norma N4 permite un try/catch, y desde hoy
// la captura ademas registra con su categoria (N5) en vez de solo imprimir.
//
// El log sale por std::cerr (destino por defecto de ConsoleSink) y el mensaje
// de la excepcion sigue en la linea, asi que quien leyera "Editor: <que>"
// encuentra el mismo texto dentro del registro. El harness de humo afirma sobre
// stdout, que no cambia.

// EST-11 - `--log <ruta>` se consume AQUI y no en EditorApplication::run().
// No es por gusto: run() vive en uno de los diecisiete ficheros por encima del
// techo y el trinquete de la guardia prohibe hacerle crecer ni una linea, asi
// que la opcion se retira de argv antes de delegar. De paso el log a fichero es
// opt-in: el editor no escribe nada en disco si nadie se lo pide.
int main(int argc, char** argv) {
    // Sin parentesis: `Logger logger(LogConfig())` se leeria como la
    // declaracion de una funcion (most vexing parse), y los dos parametros
    // del constructor ya tienen valor por defecto.
    logging::Logger logger;
    logger.addSink(std::make_unique<logging::ConsoleSink>());

    std::string logPath;
    std::vector<char*> forwarded;
    forwarded.reserve(static_cast<std::size_t>(argc));
    for (int i = 0; i < argc; ++i) {
        if (std::string(argv[i]) == "--log") {
            if (i + 1 >= argc) {
                logger.error(logging::LogCategory::Editor,
                             "Editor: --log necesita una ruta detras");
                return 1;
            }
            logPath = argv[++i];
            continue;
        }
        forwarded.push_back(argv[i]);
    }

    if (!logPath.empty()) {
        // Un log que no se puede abrir es un aviso, no un veto: el editor sigue
        // con la consola. Perder el log no debe impedir trabajar.
        Result<std::unique_ptr<logging::FileSink>> file = logging::FileSink::open(logPath);
        if (file.isOk()) {
            logger.addSink(std::move(file.value()));
            logger.info(logging::LogCategory::Editor, "motor_editor: log en " + logPath);
        } else {
            logger.error(logging::LogCategory::Editor,
                         "Editor: sin log en fichero, " + file.errorMessage());
        }
    }

    logger.info(logging::LogCategory::Editor, "motor_editor: arranque");

    try {
        const int code =
            Editor::EditorApplication{}.run(static_cast<int>(forwarded.size()), forwarded.data());
        logger.info(logging::LogCategory::Editor,
                    "motor_editor: salida normal, codigo " + std::to_string(code));
        return code;
    } catch (const std::exception& e) {
        logger.error(logging::LogCategory::Editor, std::string("Editor: ") + e.what());
        return 1;
    }
}
