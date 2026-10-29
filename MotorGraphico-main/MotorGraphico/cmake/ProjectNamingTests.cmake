# E-11 — Nombre visible arriba, id generado abajo; y duplicar sin pisar nada.
#
# La ficha del paquete señala el problema con estas palabras: «el
# recorregut continua depenent de noms tecnics». Crear un proyecto obligaba
# a teclear un id [a-z0-9_] sin acentos ni espacios. Ahora el autor escribe
# un NOMBRE y el id sale de el; los dos se guardan, y el id ya no cambia.
#
# La otra mitad es duplicar: lo que lleva el prefijo del proyecto es suyo y
# se copia renombrado; lo que no, se comparte. Y todo o nada — si algo
# falla a mitad, no queda un proyecto medio registrado, que es de lo poco
# que el autor no puede arreglar desde el editor.
#
# El sandbox de las pruebas monta su assetsRoot en una carpeta CON ESPACIOS
# a proposito: el criterio pide «dos projectes en carpetes amb espais».
#
# Fragmento aparte, como el resto de lo nuevo.
include(CTest)
if(BUILD_TESTING AND TARGET motor_project)
    add_executable(test_project_naming tests/Project/ProjectNamingTests.cpp)
    target_include_directories(test_project_naming PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_project_naming PRIVATE motor_project motor_json motor_map_document)
    add_test(NAME test_project_naming COMMAND test_project_naming)
    set_tests_properties(test_project_naming PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_project_naming)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_project_naming)
    endif()
endif()
