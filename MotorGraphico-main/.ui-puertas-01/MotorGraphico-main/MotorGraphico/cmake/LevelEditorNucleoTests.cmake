# EST-21 (relevo A) — El nucleo del editor de niveles, sin ventana.
#
# editor/src/level_editor.cpp tenia 2777 lineas y 10 paquetes encima, y
# dentro vivian dos cosas que no necesitan OpenGL para nada: el
# vocabulario de la paleta (nombres, semantica de tiles, iconos, filtros
# de familia) y la linea de comandos. Mientras estuvieron ahi no se
# podian probar: cualquier comprobacion habria pedido una ventana.
#
# Ahora son una libreria GL-free con su prueba, y `level_editor` la
# enlaza en vez de compilarlas dentro. La libreria vive en este fragmento
# y no en el bloque compartido del CMakeLists por la misma razon que
# motor_editor_problems en su dia: varios agentes tocan ese bloque a la
# vez. Cuando el editor nuevo (motor_editor) la necesite, ese paquete la
# mueve.
include(CTest)

add_library(motor_editor_niveles
    editor/src/LevelEditorPaleta.cpp
    editor/src/LevelEditorOpciones.cpp
    editor/src/LevelEditorEditState.cpp
    # EST-21 (relevo B) — las dos salidas rapidas (--proyectos / --nuevo):
    # imprimen y salen, sin abrir ventana, asi que son GL-free como el resto.
    editor/src/LevelEditorArranque.cpp)
target_include_directories(motor_editor_niveles PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/editor/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(motor_editor_niveles PUBLIC motor_level motor_map)

if(TARGET level_editor)
    target_link_libraries(level_editor PRIVATE motor_editor_niveles)
    # EST-21 (relevo B) — la pantalla de proyectos SI habla GL, asi que no
    # entra en la libreria: se compila dentro del ejecutable, pero como
    # fichero propio y no como 160 lineas dentro de main. target_sources
    # evita tocar el bloque compartido del CMakeLists.
    target_sources(level_editor PRIVATE editor/src/LevelEditorHub.cpp)
endif()

if(BUILD_TESTING)
    add_executable(test_editor_nucleo tests/Editor/LevelEditorNucleoTests.cpp)
    target_include_directories(test_editor_nucleo PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_editor_nucleo PRIVATE motor_editor_niveles)
    add_test(NAME test_editor_nucleo COMMAND test_editor_nucleo)
    set_tests_properties(test_editor_nucleo PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_editor_nucleo)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_editor_nucleo)
    endif()
endif()
