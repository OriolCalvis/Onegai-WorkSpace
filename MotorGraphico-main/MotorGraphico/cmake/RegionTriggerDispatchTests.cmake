# U3-02.E06b — dispatchRegionTriggers sobre el intérprete real. GL-free.
# Mismos enlaces que MapEventRunnerTests (el test monta un MapEventRunner para
# probar el orden respecto a update()).
include(CTest)
if(BUILD_TESTING AND TARGET motor_level AND TARGET motor_map_document)
    add_executable(test_region_trigger_dispatch
        tests/RPG/RegionTriggerDispatchTests.cpp)
    target_include_directories(test_region_trigger_dispatch PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_region_trigger_dispatch PRIVATE
        motor_level motor_map_document motor_rpg motor_json motor_map)
    add_test(NAME test_region_trigger_dispatch COMMAND test_region_trigger_dispatch)
    set_tests_properties(test_region_trigger_dispatch PROPERTIES
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_region_trigger_dispatch)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_region_trigger_dispatch)
    endif()
endif()
