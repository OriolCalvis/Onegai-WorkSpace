# C-07 — Les aventures autònomes disparen els seus beats.
#
# Fragment a part i no una línia solta al CMakeLists: és la convenció del
# projecte per als tests nous (mateix criteri que NarrativeVariablesTests).
#
# NO enllaça motor_core tot i incloure Engine/Application.h: l'única cosa
# que el test agafa d'Application és narrativeWiring(), que és
# `static constexpr` i viu sencera al header. Per això aquest test SÍ
# s'executa sense X11/GLFW, que és el que faltava per tancar el paquet
# (els ApplicationTests/ApplicationSceneTests enllacen motor_core i
# queden «Not Run» sense finestra). motor_math hi entra perquè propaga
# glm, que Application.h necessita per als headers de render.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level AND TARGET motor_map)
    add_executable(test_application_narrative_wiring tests/Engine/ApplicationNarrativeWiringTests.cpp)
    target_include_directories(test_application_narrative_wiring PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_application_narrative_wiring
        PRIVATE motor_level motor_map motor_rpg motor_json motor_math)
    add_test(NAME test_application_narrative_wiring COMMAND test_application_narrative_wiring)
    set_tests_properties(test_application_narrative_wiring PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free"
    )
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_application_narrative_wiring)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_application_narrative_wiring)
    endif()
endif()
