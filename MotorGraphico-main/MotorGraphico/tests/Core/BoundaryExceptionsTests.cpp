// EST-52 — Ninguna excepcion cruza la frontera de un modulo.
//
// La norma N4 pide que a la frontera de un modulo todo fallo se convierta en
// Result<T> y que una excepcion no salga. Eso no se comprueba leyendo el codigo:
// se comprueba llamando a la frontera con entradas que la obligan a fallar y
// mirando que devuelve un error por su canal, jamas una excepcion.
//
// Cada caso llama a una frontera real con una entrada adversa y exige dos
// cosas: que el fallo se reporte, y que la excepcion se quede dentro. Si
// alguien mete un throw en un camino de error, esto revienta la suite en vez de
// colarse en una partida.
//
// La limpieza de la carpeta que FileSink::open crea no se prueba aqui: esa ya
// la cubre test_log desde EST-11. Aqui va la excepcion, que es otra cosa.
//
// GL-free: solo motor_log, motor_level y motor_json. Sin Render ni GL.

#include "TestHarness.h"

#include <Core/Errors/Result.h>
#include <Core/Log/FileSink.h>
#include <Core/Log/ILogSink.h>
#include <Core/Log/LogConfig.h>
#include <Core/Log/LogRecord.h>
#include <Core/Log/Logger.h>
#include <Core/Log/MemorySink.h>
#include <Game/SaveGame.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>

namespace {

using logging::FileSink;
using logging::ILogSink;
using logging::LogCategory;
using logging::LogConfig;
using logging::LogRecord;
using logging::Logger;
using logging::MemorySink;

// Un sink que se niega a escribir, pero lo dice como toca: con un Result de
// error. Este es el camino bueno de un sink que falla.
class SinkQueFalla : public ILogSink {
public:
    explicit SinkQueFalla(int* intentos) : m_intentos(intentos) {}

    Result<void> write(const LogRecord&) override {
        ++*m_intentos;
        return Result<void>::Error("el disco esta lleno");
    }

private:
    int* m_intentos;
};

// Un sink que, en vez de reportar el fallo, lanza. El contrato de ILogSink es
// Result, asi que este sink esta roto: sirve para mirar si el modulo lo
// contiene o si la excepcion se sale.
class SinkQueLanza : public ILogSink {
public:
    Result<void> write(const LogRecord&) override {
        throw std::runtime_error("el sink fallo de verdad");
    }
};

// Un fichero de verdad donde se esperaba un directorio. Abrir un log debajo de
// un fichero es imposible, y aun asi hay que responder con error.
std::filesystem::path crearFicheroPadre(const std::string& nombre) {
    const std::filesystem::path ruta = std::filesystem::temp_directory_path() / nombre;
    std::ofstream salida(ruta);
    salida << "soy un fichero, no un directorio\n";
    salida.close();
    return ruta;
}

void limpiar(const std::filesystem::path& ruta) {
    std::error_code ignorado;
    std::filesystem::remove_all(ruta, ignorado);
}

}  // namespace

namespace motor_test {

// El fallo de un sink no puede tirar la entrada, ni la partida, ni a quien
// escribe. Se anota como entrada perdida y el log sigue.
MOTOR_TEST_CASE(un_sink_que_falla_no_tira_la_excepcion) {
    int intentos = 0;
    Logger log;
    log.addSink(std::make_unique<SinkQueFalla>(&intentos));

    log.info(LogCategory::Sistema, "una entrada cualquiera");

    MOTOR_CHECK(intentos == 1);
    MOTOR_CHECK(log.droppedBySink() == 1);
    // Y la entrada conto: perder el sink no es perder la partida.
    MOTOR_CHECK(log.lastSequence() == 1);
}

// addSink es publico, asi que llamarlo con nullptr no es un error del modulo:
// es una llamada tonta que se ignora en vez de reventar.
MOTOR_TEST_CASE(un_sink_nulo_se_ignora_y_no_explota) {
    Logger log;
    log.addSink(nullptr);

    log.info(LogCategory::Sistema, "sigue funcionando");

    MOTOR_CHECK(log.sinkCount() == 0);
    MOTOR_CHECK(log.lastSequence() == 1);
    MOTOR_CHECK(log.droppedBySink() == 0);
}

// El reloj es inyectable, asi que puede llegar nulo. Antes esto era un
// std::bad_function_call que salia del modulo; ahora es una entrada con tiempo
// 0, que es lo unico honesto cuando no se sabe la hora.
MOTOR_TEST_CASE(reloj_nulo_no_tira_excepcion) {
    Logger log(LogConfig(), nullptr);
    auto sink = std::make_unique<MemorySink>();
    MemorySink* observed = sink.get();
    log.addSink(std::move(sink));

    log.info(LogCategory::Sistema, "sin reloj");

    MOTOR_CHECK(observed != nullptr);
    MOTOR_CHECK(observed->size() == 1u);
    MOTOR_CHECK(observed->at(0).timestampMs == 0);
}

// El id de sesion viaja en el log y cada evento ocupa una linea. Un id con salto
// de linea romperia ese formato, asi que la frontera lo rechaza con error en vez
// de dejarlo pasar.
MOTOR_TEST_CASE(id_de_sesion_invalido_es_error_y_no_excepcion) {
    LogConfig config;
    const Result<void> conSalto = config.setSessionId("sesion\ncon salto");
    MOTOR_CHECK(!conSalto.isOk());

    // Y por la misma razon beginSession con ese id falla sin dejar ni una
    // entrada a medias.
    Logger log;
    auto sink = std::make_unique<MemorySink>();
    MemorySink* observed = sink.get();
    log.addSink(std::move(sink));

    const Result<void> abierto = log.beginSession("sesion\ncon salto");

    MOTOR_CHECK(!abierto.isOk());
    MOTOR_CHECK(observed->size() == 0u);
}

// FileSink::open es una fabrica que devuelve Result porque el sistema de
// ficheros puede fallar de varias maneras. Con un padre que es un fichero la
// ruta es imposible: y aun asi se responde con error, no se lanza.
MOTOR_TEST_CASE(abrir_fichero_en_ruta_imposible_es_error_y_no_excepcion) {
    const std::filesystem::path padre = crearFicheroPadre("est52_padre_abrir.txt");
    const std::filesystem::path imposible = padre / "subdirectorio" / "log.txt";

    const Result<std::unique_ptr<FileSink>> abierto = FileSink::open(imposible);

    MOTOR_CHECK(!abierto.isOk());
    limpiar(padre);
}

// La frontera del guardado es la que mas duele si se rompe, porque la partida
// viene de ahi. Texto basura -> false y motivo. Jamas una excepcion.
MOTOR_TEST_CASE(leer_una_partida_basura_es_error_y_no_excepcion) {
    SaveGameData datos;
    std::string error;

    const bool ok = parseSaveGame("{ esto no es una partida", &datos, &error);

    MOTOR_CHECK(!ok);
    MOTOR_CHECK(!error.empty());
}

// Y guardar donde no se puede -> false y motivo. Tampoco una excepcion.
MOTOR_TEST_CASE(escribir_en_ruta_imposible_es_error_y_no_excepcion) {
    const std::filesystem::path padre = crearFicheroPadre("est52_padre_guardar.txt");
    const SaveGameData datos;
    std::string error;

    const bool ok =
        writeSaveGameFile((padre / "sub" / "partida.json").string(), datos, &error);

    MOTOR_CHECK(!ok);
    MOTOR_CHECK(!error.empty());
    limpiar(padre);
}

// CONOCIDO — Incumple N4 a proposito, y por eso lleva la marca.
//
// Un sink que LANZA en vez de devolver Result viola el contrato de ILogSink, y
// CONTENIDO desde el 28/09 (qwen, cerrando el hallazgo que EST-52 dejo
// anotado en el tablero): Logger llama a los sinks a traves de
// writeSafely/flushSafely, que atrapan cualquier excepcion y la devuelven como
// Result. El fallo no se traga: emit() lo cuenta en droppedBySink().
//
// Este caso era la alarma - se escribio en rojo a proposito para que saltara
// el dia que alguien lo contuviera. Salto, y esta es la actualizacion que
// pidio su propio comentario: la asercion ahora es "no se sale".
MOTOR_TEST_CASE(un_sink_que_lanza_no_se_sale_del_modulo) {
    Logger log;
    log.addSink(std::make_unique<SinkQueLanza>());

    bool seSalio = false;
    try {
        log.info(LogCategory::Sistema, "esto deberia quedarse dentro");
    } catch (const std::runtime_error&) {
        seSalio = true;
    }

    MOTOR_CHECK(!seSalio);
    // Y que no quede invisible: la entrada no llego a ningun sitio y el
    // contador lo dice.
    MOTOR_CHECK(log.droppedBySink() == 1);
}

}  // namespace motor_test

int main() { return motor_test::runAllTests(); }
