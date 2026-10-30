# G-08 — Titulo y arranque desde el manifiesto del proyecto (nucleo sin
# interfaz). test_title_flow: dos proyectos con nombres y entradas
# distintos, manifiesto invalido sin caer en Boundington, y «Continuar»
# apagado con motivo o apuntando al guardado mas reciente del proyecto.
#
# TitleMenuState.cpp y SaveSlotState.cpp (G-11, que usa para mirar los
# guardados) se compilan AQUI y no en motor_level: meterlos en la
# libreria obliga a editar el bloque compartido del CMakeLists. Quien
# cablee la escena de titulo los mueve.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_title_flow
        tests/Game/TitleFlowTests.cpp
        src/Game/TitleMenuState.cpp
        src/Game/SaveSlotState.cpp)
    target_include_directories(test_title_flow PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_title_flow PRIVATE
        motor_level motor_project motor_map_document motor_rpg motor_json motor_map)
    if(TARGET copiar_assets)
        add_dependencies(test_title_flow copiar_assets)
    endif()
    add_test(NAME test_title_flow COMMAND test_title_flow)
    set_tests_properties(test_title_flow PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_title_flow)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_title_flow)
    endif()
endif()
