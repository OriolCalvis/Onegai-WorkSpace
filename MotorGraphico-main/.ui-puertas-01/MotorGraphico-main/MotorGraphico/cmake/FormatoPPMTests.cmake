# EST-55 — El formato PPM y la lectura del framebuffer, en un solo sitio.
#
# Once ficheros tenian su propia copia de «vuelca el framebuffer a un PPM»
# pegada dentro: los dos editores, el bucle del juego y ocho demos. Seis
# volteaban las filas y cinco no, asi que esas cinco llevaban anos
# escribiendo la imagen del reves y nadie lo vio -- nadie compara esas
# capturas, se miran a ojo, y un degradado boca abajo sigue pareciendo un
# degradado. Eso es lo que hace la duplicacion: el arreglo llega a unas
# copias y a otras no.
#
# DOS LIBRERIAS Y NO UNA, a proposito. El formato no necesita OpenGL, asi
# que vive aparte y SE PRUEBA; la lectura del framebuffer es la unica
# parte que necesita una ventana y se queda sin prueba, que es lo justo.
include(CTest)

add_library(motor_ppm src/Render/FormatoPPM.cpp)
target_include_directories(motor_ppm PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# No compila glad.c: los ejecutables que la usan ya lo traen (los demos lo
# compilan sueltos, motor_core lo lleva dentro). Compilarlo aqui duplicaria
# los simbolos de GL en cada enlace.
add_library(motor_captura_gl src/Render/CapturaGL.cpp)
target_include_directories(motor_captura_gl PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${GLAD_DIR}/include")
target_link_libraries(motor_captura_gl PUBLIC motor_ppm)

if(BUILD_TESTING)
    add_executable(test_formato_ppm tests/Render/FormatoPPMTests.cpp)
    target_include_directories(test_formato_ppm PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_formato_ppm PRIVATE motor_ppm)
    add_test(NAME test_formato_ppm COMMAND test_formato_ppm)
    set_tests_properties(test_formato_ppm PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_formato_ppm)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_formato_ppm)
    endif()
endif()
