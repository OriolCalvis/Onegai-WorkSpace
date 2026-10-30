# AV14-23 (D0015 §15) — La reputación por facción en la tienda y en la ficha
# (D-MOR-09) y los rumores como beats que saltan al hablar (D-MIS-03).
#
# ComercioReputacion entra en motor_level desde aquí (lo usa GameSessionComercio
# para el precio). La prueba juega las tiendas PUBLICADAS contra
# assets/rules/facciones.json y una tienda de prueba con la ficha del héroe.
#
# GL-free: ni la pieza ni la prueba conocen GL.
if(TARGET motor_level)
    target_sources(motor_level PRIVATE src/Game/ComercioReputacion.cpp)
endif()
include(CTest)
if(BUILD_TESTING AND TARGET motor_pergamino)
    add_executable(test_comercio_reputacion
        tests/Game/ComercioReputacionTests.cpp)
    target_include_directories(test_comercio_reputacion PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_comercio_reputacion PRIVATE motor_pergamino)
    if(TARGET copiar_assets)
        add_dependencies(test_comercio_reputacion copiar_assets)
    endif()
    add_test(NAME test_comercio_reputacion COMMAND test_comercio_reputacion)
    set_tests_properties(test_comercio_reputacion PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_comercio_reputacion)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_comercio_reputacion)
    endif()
endif()
