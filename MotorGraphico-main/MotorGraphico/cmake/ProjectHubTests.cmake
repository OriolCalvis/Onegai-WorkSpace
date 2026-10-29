# H4-01 (Fase H) — La pantalla de proyectos: duplicar y borrar desde el hub.
#
# ProjectHub vive en motor_level y es GL-free: todo el estado de la pantalla
# (seleccion, modo, lineas, mensajes) esta ahi a proposito, para que se pueda
# probar sin ventana (lo documenta la propia cabecera).
#
# POR QUE NO SE REUTILIZA demo_proyectos, que ya la recorre. Ese demo escribe
# sobre assets/ real —que varios agentes cambian a la vez— y lanza
# tools/build_proyecto.py, que no esta en el directorio de build; por eso
# CMakeLists.txt lo excluye de CTest y la pantalla se quedaba sin ninguna
# ejecucion automatica. Esta prueba monta su propio assetsRoot temporal y no
# toca nada del repo, asi que si entra en la suite.
#
# Sin copiar_assets: los cinco casos viven del sandbox, no de assets/.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level AND TARGET motor_project)
    add_executable(test_project_hub tests/Editor/ProjectHubTests.cpp)
    target_include_directories(test_project_hub PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    # motor_level ya arrastra motor_project y motor_json en PUBLIC (ahi vive
    # ProjectHub.cpp). Ponerlos otra vez solo gana un aviso del enlazador.
    target_link_libraries(test_project_hub PRIVATE motor_level)
    add_test(NAME test_project_hub COMMAND test_project_hub)
    set_tests_properties(test_project_hub PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_project_hub)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_project_hub)
    endif()
endif()
