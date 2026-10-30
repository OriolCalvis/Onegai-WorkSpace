# MISIONES-01 — el diario de misiones como vista de las flags.
# Vive en Game/ y no toca Application, GameSession, CharacterSheet ni el
# contrato de misiones (ver planificacion/MAPA_DEL_CODIGO.md §1).
#
# Este target estaba declarado dentro de cmake/FichaVistaTests.cmake, que es
# un fichero compartido por cuatro sesiones (FICHA-01, MISIONES-01, CODICE-01 y
# CLASES-03). Versionar aqui solo mis fuentes dejaba el clon limpio
# configurando contra tres fuentes que ningun agente tiene commiteadas; asi que
# el registro de esta prueba vive en su propio fichero y el bloque se salio del
# compartido. Cuando las cuatro se junten, FichaVistaTests.cmake vuelve a ser
# el hogar natural y este fichero se retira.
#
# GL-free: la pieza no conoce GL y la prueba tampoco. Sin assets: los datos
# vienen de RPG::AdventureScript y RPG::NarrativeState de verdad.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_diario_misiones
        tests/Game/DiarioMisionesTests.cpp)
    target_include_directories(test_diario_misiones PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_diario_misiones PRIVATE
        motor_level motor_map motor_rpg motor_json)
    add_test(NAME test_diario_misiones COMMAND test_diario_misiones)
    set_tests_properties(test_diario_misiones PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_diario_misiones)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_diario_misiones)
    endif()
endif()