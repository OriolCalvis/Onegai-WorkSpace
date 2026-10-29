# Tutorial de Boundington en la zona principal — la cadena de datos de
# boundington_primer_dia.json: tres comerciantes, emboscada de los Perdidos,
# combate y cierre por victoria o por derrota. GL-free: NarrativeEngine no
# tiene ni RNG ni GL (resuelve el combate GameSession), y el test lee el
# guion y la plaza desde el directorio de build, de ahi copiar_assets y
# WORKING_DIRECTORY.
include(CTest)
if(BUILD_TESTING AND TARGET motor_rpg)
    add_executable(test_boundington_tutorial tests/RPG/BoundingtonTutorialTests.cpp)
    target_include_directories(test_boundington_tutorial PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_boundington_tutorial PRIVATE motor_rpg)
    add_dependencies(test_boundington_tutorial copiar_assets)
    add_test(NAME test_boundington_tutorial COMMAND test_boundington_tutorial)
    set_tests_properties(test_boundington_tutorial PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_boundington_tutorial)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_boundington_tutorial)
    endif()
endif()
