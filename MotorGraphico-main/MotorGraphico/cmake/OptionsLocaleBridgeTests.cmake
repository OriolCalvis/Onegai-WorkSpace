# H1-05 / G-17 — proves del bridge OptionsState ↔ Locale.
#
# OptionsState.cpp es compila directament (patró G-14 anticollisió): no
# s'afegeix a motor_level per evitar tocar CMakeLists.txt en ple de WIP
# d'altres agents. Locale.cpp és de motor_core des que H1-05 el va necessitar
# enlladat, i OptionsLocaleBridge és header-only des que el va cridar
# Application.cpp (abans el cos d'aquest vivia a un .cpp que no era de cap
# libreria i enllazia 'juego' amb un symbol not found).
# Precisa copiar_assets per accedir a assets/i18n/ des del WORKING_DIRECTORY
# del build.
if (BUILD_TESTING AND TARGET motor_json AND TARGET motor_level)
    add_executable(test_options_locale_bridge
        tests/Game/OptionsLocaleBridgeTests.cpp
        src/Game/OptionsState.cpp)
    target_include_directories(test_options_locale_bridge PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_options_locale_bridge
        PRIVATE motor_level motor_core motor_math motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_options_locale_bridge copiar_assets)
    endif()
    add_test(NAME test_options_locale_bridge COMMAND test_options_locale_bridge)
    set_tests_properties(test_options_locale_bridge PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_options_locale_bridge)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_options_locale_bridge)
    endif()
endif()
