# PERF-1 — La cache de atlas del montaje de personajes (GL-free): no relee atlas
# compartidos, recorta antes de leer uno nuevo sin soltar los del montaje en
# curso, y los pixeles no dependen del presupuesto. Lee assets/ desde build/.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_montaje_cache tests/Game/MontajeCacheTests.cpp)
    target_include_directories(test_montaje_cache PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_montaje_cache PRIVATE motor_level motor_imagen motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_montaje_cache copiar_assets)
    endif()
    add_test(NAME test_montaje_cache COMMAND test_montaje_cache)
    set_tests_properties(test_montaje_cache PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_montaje_cache)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_montaje_cache)
    endif()
endif()

# PERF-3 — Los atlas trozeados en filas (.filas): tools/trocejar_atlas_personajes.py y su lectura.
find_program(MOTOR_PYTHON3 python3)
if(BUILD_TESTING AND TARGET motor_level AND MOTOR_PYTHON3)
    add_executable(test_montaje_filas tests/Game/MontajeFilasTests.cpp)
    target_include_directories(test_montaje_filas PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_definitions(test_montaje_filas PRIVATE
        MOTOR_PYTHON3="${MOTOR_PYTHON3}"
        MOTOR_TOOLS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tools")
    target_link_libraries(test_montaje_filas PRIVATE motor_level motor_imagen motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_montaje_filas copiar_assets)
    endif()
    add_test(NAME test_montaje_filas COMMAND test_montaje_filas)
    set_tests_properties(test_montaje_filas PROPERTIES
        LABELS "unit;gl-free" TIMEOUT 600
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_montaje_filas)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_montaje_filas)
    endif()
    add_test(NAME test_trocejar_atlas_personajes
        COMMAND ${MOTOR_PYTHON3} "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_trocejar_atlas_personajes.py")
    set_tests_properties(test_trocejar_atlas_personajes PROPERTIES LABELS "static;gl-free" TIMEOUT 300)
endif()
