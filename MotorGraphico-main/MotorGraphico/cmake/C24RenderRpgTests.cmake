# C-24 — Xarxa de seguretat al camí que es juga.
#
# 24/09/2026 (EST-33) — ESTE FRAGMENTO REGISTRABA 42 PRUEBAS Y NINGUNA SE
# CONSTRUÍA. add_motor_unit_test() no enlaza ninguna librería (sirve para
# baterías de sólo cabeceras), así que los 42 ejecutables quedaban con
# símbolos sin resolver. CTest los listaba como «Not Run»: parecían
# cobertura y no vigilaban nada. Además, add_motor_unit_test() sólo apunta
# el objetivo en la propiedad MOTOR_GL_FREE_TARGETS, y este fragmento se
# incluye DESPUÉS de que se cree suite_gl_free — el objetivo agregado que
# construye CI —, así que ni siquiera se intentaban construir.
#
# 34 DE LAS 42 ERAN DUPLICADOS. El mismo fichero de prueba ya estaba
# registrado, y bien cableado, en CMakeLists.txt o en otro fragmento
# (tests/RPG/EventCommandsTests.cpp ya corre como test_event_commands, y
# así 33 más). Se comprobó construyendo y ejecutando los 34 equivalentes:
# 32 pasan; los dos que no (test_animated_entity, test_post_processing) no
# dependen de esto. Quitarlos de aquí no pierde ni una comprobación: deja
# de mentir la lista de CTest.
#
# QUEDAN LAS 8 QUE SÓLO ESTABAN AQUÍ, ahora enlazadas de verdad y colgadas
# de suite_gl_free, para que CI las construya. Siete pasaron a la primera.
# La octava (test_render_hud_transform) falló en cinco casos: ver la nota
# dentro de tests/Render/HudTransformTests.cpp.
if(BUILD_TESTING)
    # --- Aritmética pura: cámara, culling y la regla de oclusión. ---
    add_motor_unit_test(test_render_camera tests/Render/CameraTests.cpp)
    target_link_libraries(test_render_camera PRIVATE motor_math)

    add_motor_unit_test(test_render_culling tests/Render/CullingTests.cpp)
    target_link_libraries(test_render_culling PRIVATE motor_math motor_map)

    add_motor_unit_test(test_render_occlusion_rules tests/Render/OcclusionRulesTests.cpp)
    target_link_libraries(test_render_occlusion_rules PRIVATE motor_math motor_map)

    # --- Las cinco que tienen que enlazar motor_core sin necesitarlo. ---
    #
    # NO abren ventana: se ejecutan enteras en un contenedor sin pantalla.
    # Pero Entity, Player, BitmapFont y HudTransform viven DENTRO de la
    # librería con GL, así que hay que arrastrarla para probarlos. Son
    # cinco de los rehenes que mide EST-40/41, y la prueba de que esa deuda
    # no es teórica: el día que motor_core se parta por la frontera GL,
    # estas cinco líneas pierden el motor_core y no cambia nada más.
    if(TARGET motor_core)
        foreach(_c24 "enemy;EnemyTests" "entity;EntityTests" "hud_transform;HudTransformTests"
                     "player;PlayerTests" "static_entity;StaticEntityTests")
            list(GET _c24 0 _nombre)
            list(GET _c24 1 _fuente)
            add_motor_unit_test(test_render_${_nombre} tests/Render/${_fuente}.cpp)
            target_link_libraries(test_render_${_nombre} PRIVATE motor_core motor_math motor_level)
        endforeach()
    endif()

    # add_motor_unit_test() sólo apunta el objetivo en la propiedad global,
    # y suite_gl_free ya está creada a estas alturas del CMakeLists: sin
    # este bloque, CI no construye ninguna de las ocho y volvemos a tener
    # pruebas que sólo existen en la lista.
    if(TARGET suite_gl_free)
        foreach(_c24 test_render_camera test_render_culling test_render_occlusion_rules
                     test_render_enemy test_render_entity test_render_hud_transform
                     test_render_player test_render_static_entity)
            if(TARGET ${_c24})
                add_dependencies(suite_gl_free ${_c24})
            endif()
        endforeach()
    endif()
endif()
