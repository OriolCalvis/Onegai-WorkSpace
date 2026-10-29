# AV14-22 (D0016) — El diario de misiones completo: H3-02 (la ficha) y
# MISIONES-01 (la vista de las flags) juntos en una sola fuente.
#
# La hoja de Misiones (apartados, decisiones y consecuencias) vive en su
# propio fichero y entra en motor_pergamino desde aqui: Pergamino.cmake es
# compartido por el carril del combate. La prueba juega el catalogo
# PUBLICADO (assets/catalogs/quests.json) contra las cuatro aventuras de
# Boundington de verdad: si una flag de una decision se escribe mal, o un
# objetivo de la aventura se queda fuera de los apartados, sale en rojo.
#
# GL-free: ni la pieza ni la hoja ni la prueba conocen GL.
if(TARGET motor_pergamino)
    target_sources(motor_pergamino PRIVATE src/HUD/Pergamino/PaginaMisiones.cpp)
endif()
include(CTest)
if(BUILD_TESTING AND TARGET motor_pergamino)
    add_executable(test_diario_completo
        tests/Game/DiarioCompletoTests.cpp)
    target_include_directories(test_diario_completo PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_diario_completo PRIVATE motor_pergamino)
    if(TARGET copiar_assets)
        add_dependencies(test_diario_completo copiar_assets)
    endif()
    add_test(NAME test_diario_completo COMMAND test_diario_completo)
    set_tests_properties(test_diario_completo PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_diario_completo)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_diario_completo)
    endif()
endif()
