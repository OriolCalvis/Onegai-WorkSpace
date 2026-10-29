# Puente CLI al mismo generador del juego; no se compila src suelto.
add_executable(simular_contenedores tools/simular_contenedores.cpp)
target_link_libraries(simular_contenedores PRIVATE motor_level)
if(BUILD_TESTING AND MOTOR_PYTHON3)
    add_test(NAME test_simular_economia COMMAND "${MOTOR_PYTHON3}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_simular_economia.py"
        --generador "$<TARGET_FILE:simular_contenedores>")
    set_tests_properties(test_simular_economia PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS simular_contenedores)
endif()

# El recorrido usa el motor narrativo real; aventuras y misiones solo se leen.
add_executable(recorrer_recompensas tools/recorrer_recompensas.cpp)
target_link_libraries(recorrer_recompensas PRIVATE motor_level)
if(BUILD_TESTING AND MOTOR_PYTHON3)
    add_test(NAME test_recompensas_economia COMMAND "${MOTOR_PYTHON3}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_recompensas_economia.py"
        --recorrido "$<TARGET_FILE:recorrer_recompensas>")
    set_tests_properties(test_recompensas_economia PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS recorrer_recompensas)
endif()
