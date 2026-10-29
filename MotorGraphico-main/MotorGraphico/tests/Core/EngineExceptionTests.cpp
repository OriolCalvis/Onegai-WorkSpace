// Copyright 2026 Motor Grafico Isometrico Pixel Art
// Licensed under the MIT License

#include "Core/Errors/EngineException.h"

#include "TestHarness.h"

#include <string>

// --- Tests de EngineException ---

MOTOR_TEST_CASE(engine_exception_constructor_with_message) {
    std::string msg = "Test error message";
    EngineException ex(msg);
    MOTOR_CHECK(std::string(ex.what()) == msg);
}

MOTOR_TEST_CASE(engine_exception_constructor_move_message) {
    std::string msg = "Test error message";
    EngineException ex(std::move(msg));
    MOTOR_CHECK(std::string(ex.what()) == "Test error message");
}

// --- Tests de ResourceLoadException ---

MOTOR_TEST_CASE(resource_load_exception_constructor) {
    std::string path = "assets/textures/missing.png";
    ResourceLoadException ex(path);
    
    std::string what = ex.what();
    MOTOR_CHECK(what.find("No se pudo cargar el recurso") != std::string::npos);
    MOTOR_CHECK(what.find(path) != std::string::npos);
}

MOTOR_TEST_CASE(resource_load_exception_inherits_from_engine_exception) {
    ResourceLoadException ex("test/path");
    EngineException* base = &ex;
    MOTOR_CHECK(base != nullptr);
}

// --- Tests de ShaderCompileException ---

MOTOR_TEST_CASE(shader_compile_exception_constructor) {
    std::string log = "Vertex shader: error in line 42";
    ShaderCompileException ex(log);
    
    std::string what = ex.what();
    MOTOR_CHECK(what.find("Fallo de compilacion/enlazado de shader") != std::string::npos);
    MOTOR_CHECK(what.find(log) != std::string::npos);
}

MOTOR_TEST_CASE(shader_compile_exception_compile_log_accessor) {
    std::string log = "Fragment shader: linking failed";
    ShaderCompileException ex(log);
    
    MOTOR_CHECK(ex.compileLog() == log);
}

MOTOR_TEST_CASE(shader_compile_exception_inherits_from_engine_exception) {
    ShaderCompileException ex("log");
    EngineException* base = &ex;
    MOTOR_CHECK(base != nullptr);
}

// --- Tests de RenderException ---

MOTOR_TEST_CASE(render_exception_constructor) {
    std::string msg = "GL context lost";
    RenderException ex(msg);
    
    std::string what = ex.what();
    MOTOR_CHECK(what.find("Error fatal de render") != std::string::npos);
    MOTOR_CHECK(what.find(msg) != std::string::npos);
}

MOTOR_TEST_CASE(render_exception_inherits_from_engine_exception) {
    RenderException ex("test");
    EngineException* base = &ex;
    MOTOR_CHECK(base != nullptr);
}

// --- Tests de MapParseException ---

MOTOR_TEST_CASE(map_parse_exception_constructor) {
    std::string reason = "Invalid tile ID";
    int line = 42;
    MapParseException ex(reason, line);
    
    std::string what = ex.what();
    MOTOR_CHECK(what.find("Error de parseo de mapa") != std::string::npos);
    MOTOR_CHECK(what.find(reason) != std::string::npos);
    MOTOR_CHECK(what.find("linea 42") != std::string::npos);
}

MOTOR_TEST_CASE(map_parse_exception_line_accessor) {
    MapParseException ex("reason", 99);
    MOTOR_CHECK(ex.line() == 99);
}

MOTOR_TEST_CASE(map_parse_exception_inherits_from_engine_exception) {
    MapParseException ex("reason", 1);
    EngineException* base = &ex;
    MOTOR_CHECK(base != nullptr);
}

// --- Tests de polimorfismo ---

MOTOR_TEST_CASE(all_exceptions_are_std_exception) {
    EngineException ex1("msg");
    ResourceLoadException ex2("path");
    ShaderCompileException ex3("log");
    RenderException ex4("msg");
    MapParseException ex5("reason", 1);
    
    std::exception* e1 = &ex1;
    std::exception* e2 = &ex2;
    std::exception* e3 = &ex3;
    std::exception* e4 = &ex4;
    std::exception* e5 = &ex5;
    
    MOTOR_CHECK(e1 != nullptr);
    MOTOR_CHECK(e2 != nullptr);
    MOTOR_CHECK(e3 != nullptr);
    MOTOR_CHECK(e4 != nullptr);
    MOTOR_CHECK(e5 != nullptr);
}

MOTOR_TEST_CASE(all_exceptions_have_what) {
    EngineException ex1("msg");
    ResourceLoadException ex2("path");
    ShaderCompileException ex3("log");
    RenderException ex4("msg");
    MapParseException ex5("reason", 1);
    
    MOTOR_CHECK(ex1.what() != nullptr);
    MOTOR_CHECK(ex2.what() != nullptr);
    MOTOR_CHECK(ex3.what() != nullptr);
    MOTOR_CHECK(ex4.what() != nullptr);
    MOTOR_CHECK(ex5.what() != nullptr);
}

int main() { return motor_test::runAllTests(); }
