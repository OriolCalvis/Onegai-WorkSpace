// Copyright 2026 Motor Grafico Isometrico Pixel Art
// Licensed under the MIT License

#include "Core/Errors/Result.h"

#include "TestHarness.h"

#include <string>
#include <utility>

// --- Tests de Result<T> con T = int ---

MOTOR_TEST_CASE(result_int_ok_creates_success) {
    Result<int> r = Result<int>::Ok(42);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(static_cast<bool>(r) == true);
}

MOTOR_TEST_CASE(result_int_error_creates_failure) {
    Result<int> r = Result<int>::Error("Test error");
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(static_cast<bool>(r) == false);
}

MOTOR_TEST_CASE(result_int_ok_stores_value) {
    Result<int> r = Result<int>::Ok(42);
    MOTOR_CHECK(r.value() == 42);
}

MOTOR_TEST_CASE(result_int_error_stores_message) {
    Result<int> r = Result<int>::Error("Test error");
    MOTOR_CHECK(r.errorMessage() == "Test error");
}

MOTOR_TEST_CASE(result_int_ok_using_lowercase_ok) {
    Result<int> r = Result<int>::ok(42);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value() == 42);
}

MOTOR_TEST_CASE(result_int_error_using_lowercase_error) {
    Result<int> r = Result<int>::error("Test error");
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage() == "Test error");
}

MOTOR_TEST_CASE(result_int_value_throws_on_error) {
    Result<int> r = Result<int>::Error("Test error");
    
    bool threw = false;
    try {
        r.value();
    } catch (const std::logic_error&) {
        threw = true;
    }
    MOTOR_CHECK(threw);
}

MOTOR_TEST_CASE(result_int_const_value_throws_on_error) {
    const Result<int> r = Result<int>::Error("Test error");
    
    bool threw = false;
    try {
        r.value();
    } catch (const std::logic_error&) {
        threw = true;
    }
    MOTOR_CHECK(threw);
}

MOTOR_TEST_CASE(result_int_value_returns_reference) {
    Result<int> r = Result<int>::Ok(42);
    int& ref = r.value();
    ref = 100;
    MOTOR_CHECK(r.value() == 100);
}

// --- Tests de Result<T> con T = std::string ---

MOTOR_TEST_CASE(result_string_ok_stores_value) {
    Result<std::string> r = Result<std::string>::Ok("hello");
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value() == "hello");
}

MOTOR_TEST_CASE(result_string_error_stores_message) {
    Result<std::string> r = Result<std::string>::Error("load failed");
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage() == "load failed");
}

MOTOR_TEST_CASE(result_string_move_semantics) {
    Result<std::string> r = Result<std::string>::Ok("hello");
    std::string value = std::move(r.value());
    MOTOR_CHECK(value == "hello");
}

// --- Tests de Result<void> ---

MOTOR_TEST_CASE(result_void_ok_creates_success) {
    Result<void> r = Result<void>::Ok();
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(static_cast<bool>(r) == true);
}

MOTOR_TEST_CASE(result_void_error_creates_failure) {
    Result<void> r = Result<void>::Error("Operation failed");
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(static_cast<bool>(r) == false);
}

MOTOR_TEST_CASE(result_void_ok_using_lowercase) {
    Result<void> r = Result<void>::ok();
    MOTOR_CHECK(r.isOk());
}

MOTOR_TEST_CASE(result_void_error_using_lowercase) {
    Result<void> r = Result<void>::error("Failed");
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage() == "Failed");
}

MOTOR_TEST_CASE(result_void_error_stores_message) {
    Result<void> r = Result<void>::Error("No operation performed");
    MOTOR_CHECK(r.errorMessage() == "No operation performed");
}

// --- Tests de Result<T> con T = float ---

MOTOR_TEST_CASE(result_float_ok_stores_value) {
    Result<float> r = Result<float>::Ok(3.14f);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(std::abs(r.value() - 3.14f) < 0.001f);
}

MOTOR_TEST_CASE(result_float_error) {
    Result<float> r = Result<float>::Error("Not a number");
    MOTOR_CHECK(!r.isOk());
}

// --- Tests de Result<T> con move ---

MOTOR_TEST_CASE(result_move_constructor_from_ok) {
    Result<int> r1 = Result<int>::Ok(42);
    Result<int> r2 = std::move(r1);
    MOTOR_CHECK(r2.isOk());
    MOTOR_CHECK(r2.value() == 42);
}

MOTOR_TEST_CASE(result_move_constructor_from_error) {
    Result<int> r1 = Result<int>::Error("error");
    Result<int> r2 = std::move(r1);
    MOTOR_CHECK(!r2.isOk());
    MOTOR_CHECK(r2.errorMessage() == "error");
}

// --- Tests de operador bool ---

MOTOR_TEST_CASE(result_bool_operator_true_for_ok) {
    Result<int> r = Result<int>::Ok(42);
    if (r) {
        MOTOR_CHECK(true);
    } else {
        MOTOR_CHECK(false); // No debería llegar aquí
    }
}

MOTOR_TEST_CASE(result_bool_operator_false_for_error) {
    Result<int> r = Result<int>::Error("error");
    if (r) {
        MOTOR_CHECK(false); // No debería llegar aquí
    } else {
        MOTOR_CHECK(true);
    }
}

// --- Tests de error message con move ---

MOTOR_TEST_CASE(result_error_move_message) {
    std::string msg = "Error message";
    Result<int> r = Result<int>::Error(std::move(msg));
    MOTOR_CHECK(r.errorMessage() == "Error message");
}

// --- Tests de value con move ---

MOTOR_TEST_CASE(result_ok_move_value) {
    std::string val = "Value";
    Result<std::string> r = Result<std::string>::Ok(std::move(val));
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value() == "Value");
}

int main() { return motor_test::runAllTests(); }
