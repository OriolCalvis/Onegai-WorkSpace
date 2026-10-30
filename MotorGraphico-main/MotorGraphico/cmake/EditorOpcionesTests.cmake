# EST-26 (relevo A) — La linea de comandos del editor, sin ventana.
#
# Vivia dentro de EditorApplication::run(), una funcion de 541 lineas, y por
# eso no se podia probar: cualquier comprobacion habria pedido abrir una
# ventana y un contexto OpenGL. Aqui es una libreria GL-free con su bateria,
# y de paso se cierra el fallo que EST-11 dejo anotado: `--frames abc`
# respondia «stoi: no conversion».
#
# Fragmento aparte, como el resto: cmake/MotorEditor.cmake lo tiene abierto
# otra sesion.
include(CTest)

add_library(motor_editor_opciones editor/src/EditorOpciones.cpp)
target_include_directories(motor_editor_opciones PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/editor/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(TARGET motor_editor)
    target_link_libraries(motor_editor PRIVATE motor_editor_opciones motor_captura_gl)
    # EST-26 — la fontaneria de ImGui y la vista del mapa SI hablan GL, asi
    # que no entran en la libreria GL-free: se compilan dentro del
    # ejecutable, pero como ficheros propios y no como 145 lineas encima de
    # un run() de 541. target_sources evita tocar cmake/MotorEditor.cmake,
    # que lo tiene abierto otra sesion.
    target_sources(motor_editor PRIVATE
        editor/src/EditorGui.cpp
        editor/src/EditorVistaDeMapa.cpp)
endif()

if(BUILD_TESTING)
    add_executable(test_editor_opciones tests/Editor/EditorOpcionesTests.cpp)
    target_include_directories(test_editor_opciones PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_editor_opciones PRIVATE motor_editor_opciones)
    add_test(NAME test_editor_opciones COMMAND test_editor_opciones)
    set_tests_properties(test_editor_opciones PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_editor_opciones)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_editor_opciones)
    endif()
endif()
