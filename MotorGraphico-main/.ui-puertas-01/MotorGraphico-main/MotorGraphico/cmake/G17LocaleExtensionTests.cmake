if (BUILD_TESTING AND TARGET motor_json)
    # G-17 complement: el Locale llega por motor_i18n (H1-05), la libreria
    # GL-free que lo guarda. Antes se compilaba suelto aqui, y antes de eso
    # estaba en motor_core, que lleva OpenGL y no servia para un objetivo
    # GL-free. Mismo reparto que cmake/LocaleTests.cmake.
    add_executable(test_locale_extension
        tests/Engine/LocaleExtensionTests.cpp)
    target_include_directories(test_locale_extension PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_locale_extension PRIVATE motor_json motor_i18n)
    if (TARGET copiar_assets)
        add_dependencies(test_locale_extension copiar_assets)
    endif ()
    add_test(NAME test_locale_extension COMMAND test_locale_extension)
    set_tests_properties(test_locale_extension PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_locale_extension)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_locale_extension)
    endif ()
endif ()
