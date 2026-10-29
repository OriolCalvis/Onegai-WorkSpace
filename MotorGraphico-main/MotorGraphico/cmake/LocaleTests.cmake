# G-17 — Idioma seleccionable de la interfaz, nucleo sin pantalla.
#
# Comprueba lo que en cualquier proyecto traducido se rompe callado: una
# clave que falta y enseña otro idioma sin avisar, un hueco que se llama
# {dano} en una tabla y {damage} en otra, un parametro que nadie pasa.
# Y que la frase se guarde ENTERA con huecos: el log de combate de hoy se
# construye sumando trozos en orden castellano, que no vale en ingles.
#
# Lee assets/i18n/ de verdad, asi que necesita copiar_assets y el
# WORKING_DIRECTORY del build, como los demas tests de assets reales.
#
# Locale.cpp ya no se compila aqui: H1-05 creo motor_i18n (GL-free) y este
# objetivo la enlaza. Antes era fuente directa --una de las 67 que la ley 4c
# contabilizaba-- y motor_core no servia porque lleva OpenGL. Mismo reparto
# que E-07 y E-15.
include(CTest)
if(BUILD_TESTING AND TARGET motor_json)
    add_executable(test_locale
        tests/Engine/LocaleTests.cpp)
    target_include_directories(test_locale PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_locale PRIVATE motor_json motor_i18n)
    if(TARGET copiar_assets)
        add_dependencies(test_locale copiar_assets)
    endif()
    add_test(NAME test_locale COMMAND test_locale)
    set_tests_properties(test_locale PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_locale)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_locale)
    endif()
endif()
