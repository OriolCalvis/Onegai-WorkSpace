# G-12 — Inventario, equipo y ficha utilizables (nucleo sin interfaz).
#
# test_inventory_menu: una pocion se consume una sola vez, un arma a dos
# manos no duplica el bonus y lo desplazado vuelve a la mochila, cancelar
# el destinatario no gasta, y lo imposible explica el motivo sin tocar
# nada. Reglas y modelo viven en motor_level, compartidos con juego.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_inventory_menu
        tests/Game/InventoryMenuTests.cpp)
    target_include_directories(test_inventory_menu PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_inventory_menu PRIVATE motor_level motor_map motor_rpg motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_inventory_menu copiar_assets)
    endif()
    add_test(NAME test_inventory_menu COMMAND test_inventory_menu)
    set_tests_properties(test_inventory_menu PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_inventory_menu)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_inventory_menu)
    endif()
endif()
