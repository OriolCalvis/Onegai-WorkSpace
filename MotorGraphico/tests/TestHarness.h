#pragma once

#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// INF-01.2 — Runner mínimo para pruebas unitarias. Es header-only para
// mantener el bootstrap libre de dependencias: cada binario de tests define
// sus casos con MOTOR_TEST_CASE() y termina llamando a runAllTests().
namespace motor_test {

class AssertionFailure final : public std::exception {
public:
    AssertionFailure(const char* expression, const char* file, int line) {
        std::ostringstream message;
        message << file << ':' << line << " -- comprobación fallida: " << expression;
        m_message = message.str();
    }

    const char* what() const noexcept override { return m_message.c_str(); }

private:
    std::string m_message;
};

using TestFunction = void (*)();

struct TestCase {
    const char* name = "";
    TestFunction function = nullptr;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

class Registrar {
public:
    Registrar(const char* name, TestFunction function) {
        registry().push_back(TestCase{name, function});
    }
};

inline int runAllTests() {
    int failures = 0;
    for (const TestCase& test : registry()) {
        try {
            test.function();
            std::cout << "[PASS] " << test.name << '\n';
        } catch (const std::exception& error) {
            ++failures;
            std::cerr << "[FAIL] " << test.name << ": " << error.what() << '\n';
        } catch (...) {
            ++failures;
            std::cerr << "[FAIL] " << test.name << ": excepción desconocida\n";
        }
    }

    std::cout << "Ejecutados " << registry().size() << " test(s); " << failures << " fallo(s).\n";
    return failures == 0 ? 0 : 1;
}

}  // namespace motor_test

#define MOTOR_TEST_CASE(name)                                                                      \
    static void motor_test_function_##name();                                                      \
    static ::motor_test::Registrar motor_test_registrar_##name(#name, motor_test_function_##name); \
    static void motor_test_function_##name()

#define MOTOR_CHECK(condition)                                                    \
    do {                                                                          \
        if (!(condition)) {                                                       \
            throw ::motor_test::AssertionFailure(#condition, __FILE__, __LINE__); \
        }                                                                         \
    } while (0)
