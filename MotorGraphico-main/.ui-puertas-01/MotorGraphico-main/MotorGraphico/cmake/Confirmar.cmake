# AV15-05a (EH-22, EH-23, EH-29, EH-30) — Confirmar lo caro e irreversible:
# salir, volver al título, cargar, guardar encima, comprar un negocio y
# revenderlo. La pregunta y la hoja del negocio son GL-free (motor_level); su
# papiro, del pergamino (motor_pergamino, también sin GL).
if(TARGET motor_level)
    target_sources(motor_level PRIVATE src/Game/Confirmacion.cpp src/Game/NegocioEstado.cpp)
endif()
if(TARGET motor_pergamino)
    target_sources(motor_pergamino PRIVATE src/HUD/Pergamino/HojaNegocio.cpp)
endif()
include(CTest)
if(BUILD_TESTING AND TARGET motor_pergamino)
    add_executable(test_confirmar tests/Game/ConfirmarTests.cpp)
    target_include_directories(test_confirmar PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_confirmar PRIVATE motor_pergamino)
    if(TARGET copiar_assets)
        add_dependencies(test_confirmar copiar_assets)
    endif()
    add_test(NAME test_confirmar COMMAND test_confirmar)
    set_tests_properties(test_confirmar PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_confirmar)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_confirmar)
    endif()
endif()
