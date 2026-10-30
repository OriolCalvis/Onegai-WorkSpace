# HEROE-01 — Lo que la historia sabe del heroe creado (raza, clase, origen,
# deidad, las siete respuestas y los atributos). Lee assets/ de la fuente.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level AND TARGET motor_rpg)
    add_executable(test_heroe_en_la_historia tests/RPG/HeroeEnLaHistoriaTests.cpp)
    target_include_directories(test_heroe_en_la_historia PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_definitions(test_heroe_en_la_historia PRIVATE MOTOR_FUENTE="${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(test_heroe_en_la_historia PRIVATE motor_level motor_map_document motor_map motor_rpg motor_json)
    add_test(NAME test_heroe_en_la_historia COMMAND test_heroe_en_la_historia)
    set_tests_properties(test_heroe_en_la_historia PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_heroe_en_la_historia)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_heroe_en_la_historia)
    endif()
endif()
