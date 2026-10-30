// EST-10 - Pruebas GL-free del sistema de logs (Core/Log).
//
// Cubren las tres piezas de la forma canonica (PLAN_ESTABILIZACION.md seccion 2):
// LogRecord y LogConfig son los DATOS, Logger es el CONTROLADOR, y LogFormat y
// los tres sinks son las PIEZAS que hacen el trabajo. Nada de esto necesita
// ventana; lo unico que toca el disco son los casos de FileSink, que escriben
// en el directorio temporal del sistema y se limpian al terminar.
//
// El reloj se inyecta en todos los casos: sin eso no se puede comparar una
// linea de log byte a byte y la prueba tendria que dormir o conformarse.

#include "Core/Log/ConsoleSink.h"
#include "Core/Log/FileSink.h"
#include "Core/Log/ILogSink.h"
#include "Core/Log/LogConfig.h"
#include "Core/Log/LogFormat.h"
#include "Core/Log/LogRecord.h"
#include "Core/Log/Logger.h"
#include "Core/Log/MemorySink.h"

#include "TestHarness.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

using logging::ConsoleSink;
using logging::FileSink;
using logging::FileSinkMode;
using logging::ILogSink;
using logging::LogCategory;
using logging::LogClock;
using logging::LogConfig;
using logging::LogLevel;
using logging::LogRecord;
using logging::Logger;
using logging::MemorySink;

const std::vector<LogCategory>& everyCategory() {
    static const std::vector<LogCategory> categories = {
        LogCategory::Sistema,   LogCategory::Nivel,   LogCategory::Combate,
        LogCategory::Guardado,  LogCategory::Editor,  LogCategory::Contenido,
    };
    return categories;
}

LogClock fixedClock(std::int64_t milliseconds) {
    return [milliseconds] { return milliseconds; };
}

LogRecord makeRecord(LogLevel level,
                     LogCategory category,
                     std::string message,
                     std::uint64_t sequence,
                     std::int64_t timestampMs = 0,
                     std::string sessionId = std::string()) {
    LogRecord record;
    record.level = level;
    record.category = category;
    record.message = std::move(message);
    record.sessionId = std::move(sessionId);
    record.sequence = sequence;
    record.timestampMs = timestampMs;
    return record;
}

std::filesystem::path temporaryDirectory(const char* label) {
    static int counter = 0;
    std::filesystem::path path = std::filesystem::temp_directory_path() /
                                 ("motor_log_tests_" + std::string(label) + "_" +
                                  std::to_string(++counter));
    std::error_code ignored;
    std::filesystem::remove_all(path, ignored);
    return path;
}

void removeQuietly(const std::filesystem::path& path) {
    std::error_code ignored;
    std::filesystem::remove_all(path, ignored);
}

std::vector<std::string> readLines(const std::filesystem::path& path) {
    std::vector<std::string> lines;
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

// Doble de prueba: un sink que falla siempre. Existe para comprobar que un
// fallo de log se cuenta y no se propaga: un disco lleno no puede tumbar una
// partida ni cruzar la frontera como excepcion (norma N4).
class FailingSink final : public ILogSink {
public:
    Result<void> write(const LogRecord&) override {
        ++writes;
        return Result<void>::Error("sink de prueba: escritura rechazada");
    }

    Result<void> flush() override {
        return Result<void>::Error("sink de prueba: vuelco rechazado");
    }

    int writes = 0;
};

// Doble de prueba: un sink que LANZA en vez de devolver Result, o sea que
// incumple el contrato de ILogSink. Existe porque la garantia de que ninguna
// excepcion sale del modulo de log no puede depender de que todos los sinks
// futuros se porten bien. Lo mismo que comprueba test_boundary_exceptions,
// pero desde dentro del carril de Core/Log y cubriendo tambien flush() y el
// destructor, que alli no se miran.
class ThrowingSink final : public ILogSink {
public:
    Result<void> write(const LogRecord&) override {
        throw std::runtime_error("sink de prueba: write() lanza");
    }

    Result<void> flush() override {
        throw std::runtime_error("sink de prueba: flush() lanza");
    }
};

// Doble de prueba: cuenta los vuelcos desde fuera del Logger, para poder
// comprobar que el destructor vuelca aunque el Logger ya se haya ido.
class CountingFlushSink final : public ILogSink {
public:
    explicit CountingFlushSink(std::shared_ptr<int> counter) : m_counter(std::move(counter)) {}

    Result<void> write(const LogRecord&) override { return Result<void>::Ok(); }

    Result<void> flush() override {
        ++(*m_counter);
        return Result<void>::Ok();
    }

private:
    std::shared_ptr<int> m_counter;
};

// ---------------------------------------------------------------------------
// DATOS: LogRecord
// ---------------------------------------------------------------------------

MOTOR_TEST_CASE(level_order_is_severity_order) {
    MOTOR_CHECK(logging::isAtLeast(LogLevel::Error, LogLevel::Info));
    MOTOR_CHECK(logging::isAtLeast(LogLevel::Warn, LogLevel::Info));
    MOTOR_CHECK(logging::isAtLeast(LogLevel::Info, LogLevel::Info));
    MOTOR_CHECK(!logging::isAtLeast(LogLevel::Trace, LogLevel::Info));
    MOTOR_CHECK(!logging::isAtLeast(LogLevel::Info, LogLevel::Warn));
    MOTOR_CHECK(logging::isAtLeast(LogLevel::Error, LogLevel::Trace));
}

MOTOR_TEST_CASE(level_and_category_names_are_stable) {
    MOTOR_CHECK(logging::levelName(LogLevel::Trace) == "TRACE");
    MOTOR_CHECK(logging::levelName(LogLevel::Info) == "INFO");
    MOTOR_CHECK(logging::levelName(LogLevel::Warn) == "WARN");
    MOTOR_CHECK(logging::levelName(LogLevel::Error) == "ERROR");

    MOTOR_CHECK(logging::categoryName(LogCategory::Sistema) == "sistema");
    MOTOR_CHECK(logging::categoryName(LogCategory::Nivel) == "nivel");
    MOTOR_CHECK(logging::categoryName(LogCategory::Combate) == "combate");
    MOTOR_CHECK(logging::categoryName(LogCategory::Guardado) == "guardado");
    MOTOR_CHECK(logging::categoryName(LogCategory::Editor) == "editor");
    MOTOR_CHECK(logging::categoryName(LogCategory::Contenido) == "contenido");
    // Count es el centinela, no una categoria: su nombre no puede colarse en
    // una linea de log como si lo fuera.
    MOTOR_CHECK(logging::categoryName(LogCategory::Count) == "desconocida");
}

MOTOR_TEST_CASE(category_indices_do_not_overlap) {
    std::vector<bool> seen(logging::kLogCategoryCount, false);
    for (const LogCategory category : everyCategory()) {
        const std::size_t index = logging::categoryIndex(category);
        MOTOR_CHECK(index < logging::kLogCategoryCount);
        MOTOR_CHECK(!seen[index]);
        seen[index] = true;
    }
    for (const bool used : seen) {
        MOTOR_CHECK(used);
    }
    MOTOR_CHECK(logging::categoryIndex(LogCategory::Count) == logging::kLogCategoryCount);
    MOTOR_CHECK(logging::kLogCategoryCount == everyCategory().size());
}

MOTOR_TEST_CASE(default_record_is_inert) {
    const LogRecord record;
    MOTOR_CHECK(record.level == LogLevel::Info);
    MOTOR_CHECK(record.category == LogCategory::Sistema);
    MOTOR_CHECK(record.message.empty());
    MOTOR_CHECK(record.sessionId.empty());
    MOTOR_CHECK(record.sequence == 0);
    MOTOR_CHECK(record.timestampMs == 0);
}

// ---------------------------------------------------------------------------
// DATOS: LogConfig (se valida a si misma)
// ---------------------------------------------------------------------------

MOTOR_TEST_CASE(default_config_accepts_info_and_every_category) {
    const LogConfig config;
    MOTOR_CHECK(config.minLevel() == LogLevel::Info);
    MOTOR_CHECK(config.sessionId().empty());
    MOTOR_CHECK(config.validate().isOk());

    for (const LogCategory category : everyCategory()) {
        MOTOR_CHECK(config.isEnabled(category));
        MOTOR_CHECK(config.accepts(LogLevel::Info, category));
        MOTOR_CHECK(config.accepts(LogLevel::Error, category));
    }
    // Trace fuera por defecto a proposito: es el nivel de "cada funcion" y la
    // norma N5 dice explicitamente que eso no se registra.
    MOTOR_CHECK(!config.accepts(LogLevel::Trace, LogCategory::Nivel));
}

MOTOR_TEST_CASE(disabled_category_is_filtered_even_at_error_level) {
    LogConfig config;
    config.disable(LogCategory::Combate);

    MOTOR_CHECK(!config.isEnabled(LogCategory::Combate));
    MOTOR_CHECK(!config.accepts(LogLevel::Error, LogCategory::Combate));
    MOTOR_CHECK(!config.accepts(LogLevel::Trace, LogCategory::Combate));
    MOTOR_CHECK(config.accepts(LogLevel::Error, LogCategory::Nivel));

    config.enableAll();
    for (const LogCategory category : everyCategory()) {
        MOTOR_CHECK(config.isEnabled(category));
    }
    MOTOR_CHECK(config.accepts(LogLevel::Error, LogCategory::Combate));
}

MOTOR_TEST_CASE(min_level_is_a_floor_not_a_ceiling) {
    LogConfig config;
    config.setMinLevel(LogLevel::Trace);
    MOTOR_CHECK(config.minLevel() == LogLevel::Trace);
    MOTOR_CHECK(config.accepts(LogLevel::Trace, LogCategory::Editor));

    config.setMinLevel(LogLevel::Error);
    MOTOR_CHECK(!config.accepts(LogLevel::Warn, LogCategory::Editor));
    MOTOR_CHECK(!config.accepts(LogLevel::Info, LogCategory::Editor));
    MOTOR_CHECK(config.accepts(LogLevel::Error, LogCategory::Editor));
}

MOTOR_TEST_CASE(session_id_with_control_characters_is_rejected_and_changes_nothing) {
    LogConfig config;
    MOTOR_CHECK(config.setSessionId("sesion-1").isOk());
    MOTOR_CHECK(config.sessionId() == "sesion-1");

    // Un salto de linea dentro del id partiria la linea de log en dos y el
    // que leyera el fichero veria un registro que nunca ocurrio.
    const Result<void> rejected = config.setSessionId("mala\nid");
    MOTOR_CHECK(!rejected.isOk());
    MOTOR_CHECK(!rejected.errorMessage().empty());
    MOTOR_CHECK(config.sessionId() == "sesion-1");

    const Result<void> rejectedDel = config.setSessionId(std::string("con\x7F"));
    MOTOR_CHECK(!rejectedDel.isOk());
    MOTOR_CHECK(config.sessionId() == "sesion-1");

    MOTOR_CHECK(config.setSessionId("sesion-2").isOk());
    MOTOR_CHECK(config.sessionId() == "sesion-2");
    MOTOR_CHECK(config.validate().isOk());
}

// ---------------------------------------------------------------------------
// PIEZA: LogFormat (funcion pura)
// ---------------------------------------------------------------------------

MOTOR_TEST_CASE(timestamp_is_utc_padded_and_wraps_before_epoch) {
    MOTOR_CHECK(logging::formatTimestampMs(0) == "00:00:00.000");
    MOTOR_CHECK(logging::formatTimestampMs(7) == "00:00:00.007");
    MOTOR_CHECK(logging::formatTimestampMs(1000) == "00:00:01.000");
    MOTOR_CHECK(logging::formatTimestampMs(3723456) == "01:02:03.456");
    MOTOR_CHECK(logging::formatTimestampMs(86399999) == "23:59:59.999");
    // Un reloj sin poner no puede dar horas negativas.
    MOTOR_CHECK(logging::formatTimestampMs(-1000) == "23:59:59.000");
}

MOTOR_TEST_CASE(format_record_is_one_line_in_a_fixed_order) {
    const LogRecord record = makeRecord(LogLevel::Error, LogCategory::Guardado,
                                        "ranura 3 escrita", 7, 3723456, "sesion-1");
    MOTOR_CHECK(logging::formatRecord(record) ==
                "[01:02:03.456] ERROR guardado #7 s=sesion-1 ranura 3 escrita");
}

MOTOR_TEST_CASE(shorter_level_is_padded_to_five_columns) {
    const LogRecord info = makeRecord(LogLevel::Info, LogCategory::Sistema, "hola", 1);
    MOTOR_CHECK(logging::formatRecord(info) == "[00:00:00.000] INFO  sistema #1 hola");

    const LogRecord warn = makeRecord(LogLevel::Warn, LogCategory::Nivel, "ojo", 2);
    MOTOR_CHECK(logging::formatRecord(warn) == "[00:00:00.000] WARN  nivel #2 ojo");

    const LogRecord trace = makeRecord(LogLevel::Trace, LogCategory::Editor, "detalle", 3);
    MOTOR_CHECK(logging::formatRecord(trace) == "[00:00:00.000] TRACE editor #3 detalle");
}

MOTOR_TEST_CASE(a_message_with_line_breaks_stays_on_one_line) {
    const LogRecord record =
        makeRecord(LogLevel::Warn, LogCategory::Contenido, "primera\nsegunda\rtercera", 4);
    const std::string line = logging::formatRecord(record);

    MOTOR_CHECK(line.find('\n') == std::string::npos);
    MOTOR_CHECK(line.find('\r') == std::string::npos);
    MOTOR_CHECK(line.find("primera\\nsegunda\\rtercera") != std::string::npos);
}

// ---------------------------------------------------------------------------
// PIEZAS: MemorySink y ConsoleSink
// ---------------------------------------------------------------------------

MOTOR_TEST_CASE(memory_sink_keeps_order_and_clears) {
    MemorySink sink;
    MOTOR_CHECK(sink.empty());
    MOTOR_CHECK(sink.size() == 0);

    MOTOR_CHECK(sink.write(makeRecord(LogLevel::Info, LogCategory::Nivel, "primera", 1)).isOk());
    MOTOR_CHECK(sink.write(makeRecord(LogLevel::Warn, LogCategory::Nivel, "segunda", 2)).isOk());

    MOTOR_CHECK(sink.size() == 2);
    MOTOR_CHECK(!sink.empty());
    MOTOR_CHECK(sink.at(0).message == "primera");
    MOTOR_CHECK(sink.at(1).message == "segunda");
    MOTOR_CHECK(sink.at(1).level == LogLevel::Warn);
    MOTOR_CHECK(sink.records().size() == 2);
    MOTOR_CHECK(sink.flush().isOk());

    sink.clear();
    MOTOR_CHECK(sink.empty());
}

MOTOR_TEST_CASE(console_sink_writes_the_formatted_line_to_the_injected_stream) {
    std::ostringstream stream;
    ConsoleSink sink(stream);
    const LogRecord record =
        makeRecord(LogLevel::Info, LogCategory::Nivel, "entrando a Boundington", 1, 1500);

    MOTOR_CHECK(sink.write(record).isOk());
    MOTOR_CHECK(sink.flush().isOk());
    MOTOR_CHECK(stream.str() == logging::formatRecord(record) + "\n");
    MOTOR_CHECK(stream.str() == "[00:00:01.500] INFO  nivel #1 entrando a Boundington\n");
}

MOTOR_TEST_CASE(console_sink_reports_a_broken_stream_as_a_result) {
    std::ostringstream stream;
    ConsoleSink sink(stream);
    stream.setstate(std::ios::badbit);

    const Result<void> written = sink.write(makeRecord(LogLevel::Error, LogCategory::Sistema, "x", 1));
    MOTOR_CHECK(!written.isOk());
    MOTOR_CHECK(!written.errorMessage().empty());
    MOTOR_CHECK(!sink.flush().isOk());
}

// ---------------------------------------------------------------------------
// CONTROLADOR: Logger
// ---------------------------------------------------------------------------

MOTOR_TEST_CASE(filtered_entries_never_reach_the_sink_nor_consume_sequence) {
    Logger logger(LogConfig(), fixedClock(1500));
    auto memory = std::make_unique<MemorySink>();
    MemorySink* sink = memory.get();
    logger.addSink(std::move(memory));

    logger.trace(LogCategory::Combate, "esto no se registra");
    MOTOR_CHECK(sink->empty());
    MOTOR_CHECK(logger.filteredOut() == 1);
    MOTOR_CHECK(logger.lastSequence() == 0);
    MOTOR_CHECK(logger.droppedBySink() == 0);

    logger.info(LogCategory::Combate, "turno 1");
    MOTOR_CHECK(sink->size() == 1);
    MOTOR_CHECK(logger.filteredOut() == 1);
    MOTOR_CHECK(logger.lastSequence() == 1);
    MOTOR_CHECK(sink->at(0).level == LogLevel::Info);
    MOTOR_CHECK(sink->at(0).category == LogCategory::Combate);
    MOTOR_CHECK(sink->at(0).message == "turno 1");
    MOTOR_CHECK(sink->at(0).timestampMs == 1500);
    MOTOR_CHECK(sink->at(0).sequence == 1);
}

MOTOR_TEST_CASE(sequence_is_monotonic_across_categories_and_levels) {
    Logger logger(LogConfig(), fixedClock(0));
    auto memory = std::make_unique<MemorySink>();
    MemorySink* sink = memory.get();
    logger.addSink(std::move(memory));

    logger.info(LogCategory::Nivel, "cargando nivel");
    logger.warn(LogCategory::Contenido, "asset sin nombre");
    logger.error(LogCategory::Guardado, "ranura llena");

    MOTOR_CHECK(sink->size() == 3);
    MOTOR_CHECK(sink->at(0).sequence == 1);
    MOTOR_CHECK(sink->at(1).sequence == 2);
    MOTOR_CHECK(sink->at(2).sequence == 3);
    MOTOR_CHECK(logger.lastSequence() == 3);
    MOTOR_CHECK(logger.filteredOut() == 0);
}

MOTOR_TEST_CASE(disabled_category_never_reaches_the_sink) {
    LogConfig config;
    config.disable(LogCategory::Editor);
    Logger logger(config, fixedClock(0));
    auto memory = std::make_unique<MemorySink>();
    MemorySink* sink = memory.get();
    logger.addSink(std::move(memory));

    MOTOR_CHECK(!logger.isEnabled(LogLevel::Error, LogCategory::Editor));
    MOTOR_CHECK(logger.isEnabled(LogLevel::Error, LogCategory::Nivel));

    logger.error(LogCategory::Editor, "documento roto");
    logger.error(LogCategory::Nivel, "nivel roto");

    MOTOR_CHECK(sink->size() == 1);
    MOTOR_CHECK(sink->at(0).category == LogCategory::Nivel);
    MOTOR_CHECK(logger.filteredOut() == 1);
    MOTOR_CHECK(logger.lastSequence() == 1);
}

MOTOR_TEST_CASE(begin_session_stamps_the_id_and_announces_itself) {
    Logger logger(LogConfig(), fixedClock(42));
    auto memory = std::make_unique<MemorySink>();
    MemorySink* sink = memory.get();
    logger.addSink(std::move(memory));

    MOTOR_CHECK(logger.beginSession("sesion-7").isOk());
    MOTOR_CHECK(logger.sessionId() == "sesion-7");
    MOTOR_CHECK(sink->size() == 1);
    MOTOR_CHECK(sink->at(0).message == "traza de sesion abierta");
    MOTOR_CHECK(sink->at(0).category == LogCategory::Sistema);
    MOTOR_CHECK(sink->at(0).level == LogLevel::Info);
    MOTOR_CHECK(sink->at(0).sessionId == "sesion-7");

    logger.info(LogCategory::Nivel, "dentro de la traza");
    MOTOR_CHECK(sink->size() == 2);
    MOTOR_CHECK(sink->at(1).sessionId == "sesion-7");
}

MOTOR_TEST_CASE(begin_session_rejects_a_control_character_and_keeps_the_previous_trace) {
    Logger logger(LogConfig(), fixedClock(0));
    auto memory = std::make_unique<MemorySink>();
    MemorySink* sink = memory.get();
    logger.addSink(std::move(memory));
    MOTOR_CHECK(logger.beginSession("buena").isOk());
    MOTOR_CHECK(sink->size() == 1);

    const Result<void> rejected = logger.beginSession("mala\nid");
    MOTOR_CHECK(!rejected.isOk());
    MOTOR_CHECK(!rejected.errorMessage().empty());
    MOTOR_CHECK(logger.sessionId() == "buena");
    // No se anuncia una sesion que no se ha abierto.
    MOTOR_CHECK(sink->size() == 1);
}

MOTOR_TEST_CASE(a_null_sink_is_ignored_instead_of_crashing) {
    Logger logger(LogConfig(), fixedClock(0));
    logger.addSink(nullptr);
    MOTOR_CHECK(logger.sinkCount() == 0);

    logger.info(LogCategory::Sistema, "sin destinos");
    MOTOR_CHECK(logger.lastSequence() == 1);
    MOTOR_CHECK(logger.droppedBySink() == 0);
    MOTOR_CHECK(logger.flush().isOk());
}

MOTOR_TEST_CASE(a_broken_sink_is_counted_and_never_throws) {
    Logger logger(LogConfig(), fixedClock(0));
    auto failing = std::make_unique<FailingSink>();
    FailingSink* sink = failing.get();
    logger.addSink(std::move(failing));

    logger.info(LogCategory::Nivel, "una");
    logger.error(LogCategory::Nivel, "dos");

    MOTOR_CHECK(sink->writes == 2);
    MOTOR_CHECK(logger.droppedBySink() == 2);
    // La secuencia avanza aunque el destino falle: el registro se sello.
    MOTOR_CHECK(logger.lastSequence() == 2);
    // El primer fallo se devuelve, no se lanza.
    const Result<void> flushed = logger.flush();
    MOTOR_CHECK(!flushed.isOk());
    MOTOR_CHECK(!flushed.errorMessage().empty());
}

MOTOR_TEST_CASE(a_throwing_sink_is_contained_and_counted) {
    Logger logger(LogConfig(), fixedClock(0));
    logger.addSink(std::make_unique<ThrowingSink>());

    logger.info(LogCategory::Nivel, "el sink va a lanzar");

    MOTOR_CHECK(logger.droppedBySink() == 1);
    // La entrada se sello igualmente: el fallo es del destino, no del registro.
    MOTOR_CHECK(logger.lastSequence() == 1);
}

MOTOR_TEST_CASE(a_throwing_sink_does_not_stop_the_other_sinks) {
    Logger logger(LogConfig(), fixedClock(0));
    logger.addSink(std::make_unique<ThrowingSink>());
    auto memory = std::make_unique<MemorySink>();
    MemorySink* sink = memory.get();
    logger.addSink(std::move(memory));

    logger.info(LogCategory::Nivel, "uno");
    logger.warn(LogCategory::Nivel, "dos");

    // Sin contener la excepcion, el primer sink se llevaba el bucle por
    // delante y el segundo no veia nada: un destino roto dejaba el log ciego,
    // que es justo cuando hace falta leerlo.
    MOTOR_CHECK(sink->size() == 2);
    MOTOR_CHECK(logger.droppedBySink() == 2);
}

MOTOR_TEST_CASE(flush_returns_an_error_when_a_sink_throws) {
    Logger logger(LogConfig(), fixedClock(0));
    logger.addSink(std::make_unique<ThrowingSink>());

    const Result<void> flushed = logger.flush();
    MOTOR_CHECK(!flushed.isOk());
    MOTOR_CHECK(!flushed.errorMessage().empty());
}

MOTOR_TEST_CASE(destructor_survives_a_throwing_sink) {
    int llegoAlFinal = 0;
    {
        Logger logger(LogConfig(), fixedClock(0));
        logger.addSink(std::make_unique<ThrowingSink>());
        logger.info(LogCategory::Sistema, "antes del cierre");
    }
    // El destructor vuelca los sinks. Si no contuviera la excepcion, saldria
    // de un destructor, y en C++ eso es std::terminate: no habria partida que
    // depurar. Llegar aqui ES la comprobacion.
    ++llegoAlFinal;
    MOTOR_CHECK(llegoAlFinal == 1);
}

MOTOR_TEST_CASE(a_null_clock_logs_timestamp_zero_instead_of_throwing) {
    const LogClock absentClock;  // funcion vacia: el caso que el Logger debe sobrevivir
    Logger logger(LogConfig(), absentClock);
    auto memory = std::make_unique<MemorySink>();
    MemorySink* sink = memory.get();
    logger.addSink(std::move(memory));

    logger.info(LogCategory::Sistema, "reloj ausente");
    MOTOR_CHECK(sink->size() == 1);
    MOTOR_CHECK(sink->at(0).timestampMs == 0);
}

MOTOR_TEST_CASE(set_config_applies_the_new_filter) {
    Logger logger(LogConfig(), fixedClock(0));
    auto memory = std::make_unique<MemorySink>();
    MemorySink* sink = memory.get();
    logger.addSink(std::move(memory));

    LogConfig config;
    config.setMinLevel(LogLevel::Trace);
    config.disable(LogCategory::Combate);
    MOTOR_CHECK(logger.setConfig(config).isOk());

    MOTOR_CHECK(logger.config().minLevel() == LogLevel::Trace);
    MOTOR_CHECK(logger.isEnabled(LogLevel::Trace, LogCategory::Nivel));
    MOTOR_CHECK(!logger.isEnabled(LogLevel::Trace, LogCategory::Combate));

    logger.trace(LogCategory::Nivel, "ahora si se registra");
    logger.trace(LogCategory::Combate, "este no");
    MOTOR_CHECK(sink->size() == 1);
    MOTOR_CHECK(sink->at(0).level == LogLevel::Trace);
    MOTOR_CHECK(logger.filteredOut() == 1);
}

MOTOR_TEST_CASE(the_same_record_reaches_every_sink_unchanged) {
    Logger logger(LogConfig(), fixedClock(99));
    std::ostringstream stream;
    logger.addSink(std::make_unique<ConsoleSink>(stream));
    auto memory = std::make_unique<MemorySink>();
    MemorySink* sink = memory.get();
    logger.addSink(std::move(memory));
    MOTOR_CHECK(logger.sinkCount() == 2);

    logger.warn(LogCategory::Contenido, "asset no encontrado: hacha");
    MOTOR_CHECK(sink->size() == 1);
    MOTOR_CHECK(stream.str() == logging::formatRecord(sink->at(0)) + "\n");
}

MOTOR_TEST_CASE(destructor_flushes_the_sinks) {
    auto flushes = std::make_shared<int>(0);
    {
        Logger logger(LogConfig(), fixedClock(0));
        logger.addSink(std::make_unique<CountingFlushSink>(flushes));
        logger.info(LogCategory::Sistema, "antes del cierre");
        MOTOR_CHECK(*flushes == 0);
    }
    // Un log que pierde la cola justo antes del cierre es precisamente el que
    // hacia falta: el vuelco vive en el destructor y no se puede olvidar.
    MOTOR_CHECK(*flushes == 1);
}

// ---------------------------------------------------------------------------
// PIEZA: FileSink (lo unico que toca el disco, en el directorio temporal)
// ---------------------------------------------------------------------------

MOTOR_TEST_CASE(file_sink_creates_the_missing_directory_and_appends) {
    const std::filesystem::path root = temporaryDirectory("append");
    const std::filesystem::path file = root / "logs" / "sesion.log";
    MOTOR_CHECK(!std::filesystem::exists(file));

    {
        Result<std::unique_ptr<FileSink>> opened = FileSink::open(file);
        MOTOR_CHECK(opened.isOk());
        FileSink& sink = *opened.value();
        MOTOR_CHECK(sink.path() == file);
        MOTOR_CHECK(sink.write(makeRecord(LogLevel::Info, LogCategory::Nivel, "primera", 1)).isOk());
        MOTOR_CHECK(sink.write(makeRecord(LogLevel::Error, LogCategory::Guardado, "segunda", 2)).isOk());
        MOTOR_CHECK(sink.flush().isOk());
    }

    const std::vector<std::string> first = readLines(file);
    MOTOR_CHECK(first.size() == 2);
    MOTOR_CHECK(first[0] == "[00:00:00.000] INFO  nivel #1 primera");
    MOTOR_CHECK(first[1] == "[00:00:00.000] ERROR guardado #2 segunda");

    {
        Result<std::unique_ptr<FileSink>> reopened = FileSink::open(file, FileSinkMode::Append);
        MOTOR_CHECK(reopened.isOk());
        MOTOR_CHECK(reopened.value()->write(
                        makeRecord(LogLevel::Info, LogCategory::Nivel, "tercera", 3)).isOk());
        MOTOR_CHECK(reopened.value()->flush().isOk());
    }

    const std::vector<std::string> second = readLines(file);
    MOTOR_CHECK(second.size() == 3);
    MOTOR_CHECK(second[2] == "[00:00:00.000] INFO  nivel #3 tercera");

    removeQuietly(root);
}

MOTOR_TEST_CASE(file_sink_truncate_starts_clean) {
    const std::filesystem::path root = temporaryDirectory("truncate");
    const std::filesystem::path file = root / "sesion.log";

    {
        Result<std::unique_ptr<FileSink>> opened = FileSink::open(file, FileSinkMode::Truncate);
        MOTOR_CHECK(opened.isOk());
        opened.value()->write(makeRecord(LogLevel::Info, LogCategory::Sistema, "vieja", 1));
        opened.value()->write(makeRecord(LogLevel::Info, LogCategory::Sistema, "vieja", 2));
        opened.value()->flush();
    }
    MOTOR_CHECK(readLines(file).size() == 2);

    {
        Result<std::unique_ptr<FileSink>> reopened = FileSink::open(file, FileSinkMode::Truncate);
        MOTOR_CHECK(reopened.isOk());
        reopened.value()->write(makeRecord(LogLevel::Info, LogCategory::Sistema, "nueva", 1));
        reopened.value()->flush();
    }

    const std::vector<std::string> lines = readLines(file);
    MOTOR_CHECK(lines.size() == 1);
    MOTOR_CHECK(lines[0] == "[00:00:00.000] INFO  sistema #1 nueva");

    removeQuietly(root);
}

MOTOR_TEST_CASE(file_sink_flushes_an_error_without_being_asked) {
    const std::filesystem::path root = temporaryDirectory("vuelco");
    const std::filesystem::path file = root / "sesion.log";

    Result<std::unique_ptr<FileSink>> opened = FileSink::open(file, FileSinkMode::Truncate);
    MOTOR_CHECK(opened.isOk());
    FileSink& sink = *opened.value();
    MOTOR_CHECK(sink.write(makeRecord(LogLevel::Error, LogCategory::Combate, "muerte", 1)).isOk());

    // Sin llamar a flush() y con el sink todavia vivo: lo ultimo que escribe
    // un motor antes de morir es un error, y ese no se puede quedar en el
    // buffer del stream.
    const std::vector<std::string> lines = readLines(file);
    MOTOR_CHECK(lines.size() == 1);
    MOTOR_CHECK(lines[0] == "[00:00:00.000] ERROR combate #1 muerte");

    removeQuietly(root);
}

MOTOR_TEST_CASE(file_sink_open_failure_is_returned_not_thrown) {
    const std::filesystem::path root = temporaryDirectory("fallo");
    std::error_code ignored;
    std::filesystem::create_directories(root, ignored);
    const std::filesystem::path blocker = root / "bloqueante";
    {
        std::ofstream file(blocker);
        file << "no soy un directorio";
    }

    const Result<std::unique_ptr<FileSink>> opened = FileSink::open(blocker / "sesion.log");
    MOTOR_CHECK(!opened.isOk());
    MOTOR_CHECK(!opened.errorMessage().empty());

    removeQuietly(root);
}

MOTOR_TEST_CASE(a_failed_open_removes_the_directory_it_just_created) {
    const std::filesystem::path root = temporaryDirectory("rollback");
    const std::filesystem::path parent = root / "logs";
    MOTOR_CHECK(!std::filesystem::exists(parent));

    // Un nombre por encima de NAME_MAX hace fallar la apertura DESPUES de que
    // open() haya creado la carpeta: es exactamente el caso que el ScopeGuard
    // de EST-11 limpia, y sin el quedaria un logs/ vacio de nadie.
    const std::string demasiadoLargo(300, 'a');
    const Result<std::unique_ptr<FileSink>> opened = FileSink::open(parent / demasiadoLargo);

    MOTOR_CHECK(!opened.isOk());
    MOTOR_CHECK(!std::filesystem::exists(parent));

    removeQuietly(root);
}

MOTOR_TEST_CASE(a_failed_open_leaves_an_existing_directory_alone) {
    const std::filesystem::path root = temporaryDirectory("sin_rollback");
    const std::filesystem::path parent = root / "logs";
    std::error_code ignored;
    std::filesystem::create_directories(parent, ignored);

    const std::string demasiadoLargo(300, 'a');
    const Result<std::unique_ptr<FileSink>> opened = FileSink::open(parent / demasiadoLargo);

    MOTOR_CHECK(!opened.isOk());
    // La carpeta ya estaba ahi antes: no es de este intento y no se toca.
    MOTOR_CHECK(std::filesystem::exists(parent));

    removeQuietly(root);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
