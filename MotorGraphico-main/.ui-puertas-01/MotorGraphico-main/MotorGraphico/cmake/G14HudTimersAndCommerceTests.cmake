if (BUILD_TESTING AND TARGET motor_rpg)
    # MISMA ESTRATEGIA anticolisión que LocaleTests.cmake i G-17:
    # BitmapFont.cpp, HudTimer.cpp i ShopEngine.cpp es compilat AQUI
    # dins de lexecutable i NO s'enllaça motor_core — motor_core conté
    # Application.cpp (shared 14 paquets) que esta rosat per altres agents.
    # H1-02: HudTimer i ShopEngine són producció ara (motor_level); el test
    # els compila directament (mateix patró que G13 BattleMenuActions.cpp).
    add_executable(test_g14_hud_timers_commerce
        tests/Game/G14HudTimersAndCommerceTests.cpp
        tests/Game/StubsGLG14.cpp
        src/Render/BitmapFont.cpp
        src/HUD/HudTimer.cpp
        src/Game/ShopEngine.cpp)
    target_include_directories(test_g14_hud_timers_commerce PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyxml2"
        "${CMAKE_BINARY_DIR}/_deps/glfw-src/include"
        "${CMAKE_BINARY_DIR}/_deps/glm-src")
    target_link_libraries(test_g14_hud_timers_commerce PRIVATE
        motor_rpg motor_math)
    # macOS linker: symbols not found (render) son dynamic_lookup — NUNCA
    # se ejecutan en runner gl-free, solo stubs.
    if (APPLE)
        target_link_options(test_g14_hud_timers_commerce PRIVATE
            -Wl,-undefined,dynamic_lookup)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # PRUEBAS-01 — lo mismo con el enlazador de Linux: sin esto el test
        # no enlazaba fuera del Mac (~Texture, SpriteBatch::submit).
        target_link_options(test_g14_hud_timers_commerce PRIVATE
            -Wl,--unresolved-symbols=ignore-in-object-files)
    endif ()
    if (TARGET copiar_assets)
        add_dependencies(test_g14_hud_timers_commerce copiar_assets)
    endif ()
    add_test(NAME test_g14_hud_timers_commerce COMMAND test_g14_hud_timers_commerce)
    set_tests_properties(test_g14_hud_timers_commerce PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_g14_hud_timers_commerce)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_g14_hud_timers_commerce)
    endif ()
endif ()
