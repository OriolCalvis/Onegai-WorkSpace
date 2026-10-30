# E-09 — La misión de la llave, la puerta y el cofre.
#
# Es la prueba de aceptación de toda la fase E. Cada eslabón (E-01 el
# evento dentro del mapa, E-02 el estado, E-03 el intérprete, E-04 las
# comandas, E-07 la validación) tiene ya su prueba y todas pasan; ninguna
# demuestra que con eso se pueda hacer UNA MISIÓN. Esto lo demuestra.
#
# Lee assets/misiones/mision_llave.json de verdad —el mismo fichero que
# se publica— y comprueba que dice lo mismo que el código que lo generó.
# Por eso necesita copiar_assets y el WORKING_DIRECTORY del build.
#
# Regenerar el asset:
#   ./demo_mision_llave --escribir assets/misiones/mision_llave.json
#
# Fragmento aparte, como el resto de lo nuevo: varios agentes escriben a
# la vez en este repositorio.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level AND TARGET motor_map_document AND TARGET motor_saves)
    add_executable(demo_mision_llave
        examples/demo_mision_llave.cpp
        src/RPG/AdventureValidation.cpp
        src/RPG/SessionCommandHost.cpp)
    target_include_directories(demo_mision_llave PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/examples"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    # motor_saves es obligatorio desde que SessionCommandHost ejecuta las
    # comandas de ranura (G-11): sin el el demo no enlaza.
    target_link_libraries(demo_mision_llave PRIVATE
        motor_level motor_map_document motor_rpg motor_json motor_map motor_saves)
    if(TARGET copiar_assets)
        add_dependencies(demo_mision_llave copiar_assets)
    endif()
    add_test(NAME demo_mision_llave COMMAND demo_mision_llave)
    set_tests_properties(demo_mision_llave PROPERTIES
        LABELS "gl-free;smoke;aceptacion"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS demo_mision_llave)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free demo_mision_llave)
    endif()
endif()
