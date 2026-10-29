# E-15 — Lista de problemas y busqueda de eventos, mitad sin interfaz.
#
# Defiende el criterio de cierre del paquete: «una referencia borrada no
# provoca navegacion a una entidad diferente». Los sitios se guardan por
# id de pagina y tipo de comando, no por indice, y navegar los vuelve a
# resolver contra el documento de hoy.
#
# ProblemsState.cpp se compila AQUI y no dentro de motor_level todavia:
# meterlo en la libreria obliga a editar el bloque compartido del
# CMakeLists, que varios agentes tocan a la vez. Cuando el panel de E-06
# lo necesite enlazado, ese paquete lo mueve. Mismo reparto que E-07.
#
# Fragmento aparte, como el resto de tests nuevos.
include(CTest)
if(BUILD_TESTING AND TARGET motor_map_document AND TARGET motor_rpg)
    add_executable(test_problems_state
        tests/Editor/ProblemsStateTests.cpp
        src/RPG/AdventureValidation.cpp)
    target_include_directories(test_problems_state PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_problems_state PRIVATE motor_editor_problems motor_map_document motor_rpg motor_json)
    add_test(NAME test_problems_state COMMAND test_problems_state)
    set_tests_properties(test_problems_state PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_problems_state)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_problems_state)
    endif()
endif()
