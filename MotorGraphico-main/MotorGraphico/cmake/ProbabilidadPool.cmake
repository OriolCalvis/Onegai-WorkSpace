# AV13-04 (COMBATE_BG3.md §6, D0014 §2) — La probabilidad exacta de una
# tirada enfrentada, para la ayuda de dados. Y D0013: el crítico son dos 6.
#
# ProbabilidadPool entra en motor_rpg desde aquí (GL-free, como
# DicePoolEngine). La prueba cuenta todas las tiradas posibles y compara los
# 729 pares (0..8 x 0..8 dados, con ventaja y desventaja) con 200 000
# tiradas de roll_pool cada uno. Sin GL y sin assets.
if(TARGET motor_rpg)
    target_sources(motor_rpg PRIVATE src/RPG/ProbabilidadPool.cpp)
endif()
include(CTest)
if(BUILD_TESTING AND TARGET motor_rpg)
    add_executable(test_probabilidad_pool tests/RPG/ProbabilidadPoolTests.cpp)
    target_include_directories(test_probabilidad_pool PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_probabilidad_pool PRIVATE motor_rpg)
    add_test(NAME test_probabilidad_pool COMMAND test_probabilidad_pool)
    set_tests_properties(test_probabilidad_pool PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_probabilidad_pool)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_probabilidad_pool)
    endif()
endif()
