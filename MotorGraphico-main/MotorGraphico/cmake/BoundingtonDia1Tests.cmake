# Dia 1 de Boundington: encargo de Venides y objetivo de la plaza (E-G4-05).
include(CTest)
if(BUILD_TESTING AND TARGET motor_rpg AND TARGET motor_level)
    add_executable(test_boundington_dia1 tests/RPG/BoundingtonDia1Tests.cpp)
    target_include_directories(test_boundington_dia1 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests" "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_boundington_dia1 PRIVATE motor_level motor_map motor_rpg motor_json)
    add_dependencies(test_boundington_dia1 copiar_assets)
    add_test(NAME test_boundington_dia1 COMMAND test_boundington_dia1)
    set_tests_properties(test_boundington_dia1 PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_boundington_dia1)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_boundington_dia1)
    endif()
endif()
