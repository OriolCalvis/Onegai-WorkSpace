# AV18-02: reglas y adaptación de sesión sin OpenGL, alcanzadas por juego.
target_sources(motor_level PRIVATE
    src/Game/Contenedores.cpp
    src/Game/ContenedoresCatalogo.cpp
    src/Game/ContenedoresSesion.cpp)
if(BUILD_TESTING)
    add_executable(test_contenedores tests/Game/ContenedoresTests.cpp)
    target_include_directories(test_contenedores PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_contenedores PRIVATE motor_level)
    add_test(NAME test_contenedores COMMAND test_contenedores)
    set_tests_properties(test_contenedores PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_contenedores)
endif()
