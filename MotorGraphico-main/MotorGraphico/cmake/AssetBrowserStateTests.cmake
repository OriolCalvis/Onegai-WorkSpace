# E-13 — Navegador de recursos e importacion, mitad sin interfaz.
#
# Defiende el criterio de cierre del paquete: busqueda por nombre y tipo con
# acentos plegados (DEC-15), seleccion por identidad que sobrevive al
# refresco, dimensiones y frames leidos del PNG sin GL, e importacion que
# genera una referencia relativa estable y NUNCA sobreescribe (colision =>
# reutilizar o cancelar, y mover la carpeta del proyecto conserva el recurso).
#
# AssetBrowserState.cpp se compila AQUI y no dentro de una libreria, igual que
# ProblemsState.cpp (E-15) y CatalogPickerState.cpp (E-14): meterlo en
# motor_level obliga a editar el bloque compartido del CMakeLists, que varios
# agentes tocan a la vez. Cuando el panel (AssetBrowserPanel.cpp) lo necesite
# enlazado, ese paquete lo mueve.
#
# ProblemsState.cpp va dentro por NormalizeForSearch (DEC-15): plegar acentos
# es la misma regla que en E-15 y no se copia una tercera vez. Y enlaza
# motor_project por ProjectIndex::suggestId (E-11): el id y la referencia del
# recurso salen de la MISMA regla que los proyectos.
include(CTest)
if(BUILD_TESTING AND TARGET motor_project)
    add_executable(test_asset_browser_state
        tests/Editor/AssetBrowserStateTests.cpp
        src/Editor/AssetBrowserState.cpp
        src/RPG/AdventureValidation.cpp)
    target_include_directories(test_asset_browser_state PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    # motor_project ya arrastra motor_json y motor_map_document (PUBLIC):
    # repetirlos aqui solo anadia el aviso de librerias duplicadas al enlazar.
    target_link_libraries(test_asset_browser_state PRIVATE motor_editor_problems motor_project motor_rpg)
    add_test(NAME test_asset_browser_state COMMAND test_asset_browser_state)
    set_tests_properties(test_asset_browser_state PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_asset_browser_state)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_asset_browser_state)
    endif()
endif()
