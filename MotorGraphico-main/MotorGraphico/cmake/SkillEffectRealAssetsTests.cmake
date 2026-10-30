# D-11 — Vocabulari d'efectes d'habilitat sobre assets publicats.
# Complement a tests/RPG/SkillEffectTests.cpp (agent anterior). Target
# nou separat: NO toca SkillEffectTests.cpp ni SkillExecutor.cpp
# (shared D-09). GL-free.

if (BUILD_TESTING)
    add_executable(test_skill_effect_real_assets
        tests/RPG/SkillEffectRealAssetsTests.cpp)
    target_include_directories(test_skill_effect_real_assets PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_include_directories(test_skill_effect_real_assets PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/examples")
    target_link_libraries(test_skill_effect_real_assets PRIVATE
        motor_rpg
        motor_json
    )
    if (TARGET copiar_assets)
        add_dependencies(test_skill_effect_real_assets copiar_assets)
    endif ()
    add_test(NAME test_skill_effect_real_assets
        COMMAND test_skill_effect_real_assets)
    set_tests_properties(test_skill_effect_real_assets PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY
        MOTOR_GL_FREE_TARGETS test_skill_effect_real_assets)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_skill_effect_real_assets)
    endif ()
endif ()
