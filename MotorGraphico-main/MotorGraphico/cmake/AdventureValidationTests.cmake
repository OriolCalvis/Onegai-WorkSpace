# E-07 — Validacion estatica de eventos: banderas que nadie enciende,
# comandos inalcanzables tras un cambio de mapa (DEC-30), interruptores de
# eventos que no existen, comandos declarados y no cableados, y parametros
# a medio rellenar. Cada hallazgo sabe donde esta, que es lo que E-15
# necesita para su accion «Ir ahi».
#
# Fragmento aparte, como el resto de tests nuevos: evita que varios
# agentes se pisen editando el mismo bloque del CMakeLists.
include(CTest)
if(BUILD_TESTING AND TARGET motor_map_document AND TARGET motor_rpg)
    add_executable(test_adventure_validation
        tests/RPG/AdventureValidationTests.cpp
        src/RPG/AdventureValidation.cpp
        src/RPG/AdventureBeatValidation.cpp)
    target_include_directories(test_adventure_validation PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_adventure_validation PRIVATE motor_map_document motor_rpg motor_json)
    add_test(NAME test_adventure_validation COMMAND test_adventure_validation)
    set_tests_properties(test_adventure_validation PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_adventure_validation)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_adventure_validation)
    endif()
endif()
