include(CTest)
# H1-05 — Locale.cpp y OptionsLocaleBridge ya no son fuentes sueltas aquí:
# Locale.cpp está en motor_core porque Application.cpp lo llama en init(), y
# OptionsLocaleBridge es header-only porque, sin .cpp en ninguna librería,
# `juego` no enlazaba el símbolo que Application.cpp invoca. Antes de H1-05 los
# dos se compilaban en tres objetivos y motor_core no los definía.
# Precisa copiar_assets por assets/i18n/ desde el WORKING_DIRECTORY del build.
if(BUILD_TESTING AND TARGET motor_core)
    add_executable(test_application_cableado
        tests/Engine/ApplicationCableadoTests.cpp)
    target_include_directories(test_application_cableado PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_application_cableado PRIVATE motor_core motor_math motor_level motor_saves)
    add_dependencies(test_application_cableado copiar_assets)
    add_test(NAME test_application_cableado COMMAND test_application_cableado)
    set_tests_properties(test_application_cableado PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl"
        TIMEOUT 60
        RUN_SERIAL TRUE
    )
endif()
