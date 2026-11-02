Encontré **un hallazgo de severidad media**, heredado de nube y conservado por la resolución. No encontré incompatibilidades de integración en los otros cuatro puntos. Revisión estática sobre `a4d0b6d9`: sin modificar ficheros, compilar ni ejecutar pruebas.

1. **Interpolación — hallazgo P2: desbordamiento intermedio ocultado por el clamp.**

   En [Interpolacion.cpp:54](/Users/admin/onegai-ramas/UNI-02-11/MotorGraphico-main/MotorGraphico/src/Core/Math/Interpolacion.cpp:54), la rama para `b-a` infinito vuelve a multiplicar por dos **antes** de sumar `a`. Esa multiplicación puede desbordar aunque el resultado matemático sea representable.

   **Cómo demostrarlo:** añadir esta comprobación y evaluarla con contracción FMA desactivada (`-ffp-contract=off`):

   ```cpp
   const double v = Interpolacion::tweenEscalar(-1.7e308, 1.7e308, 0.75);
   MOTOR_CHECK(std::abs(v - 8.5e307) <= 1e-12 * 1.7e308);
   ```

   El intermedio resulta aproximadamente `2.55e308 → inf`; el clamp devuelve `1.7e308`, frente al esperado `8.5e307`. La prueba actual `tween_extremos_opuestos_enormes_son_finitos_y_acotados` acepta ese resultado incorrecto.

   Por tanto, el comentario de [Interpolacion.h:45](/Users/admin/onegai-ramas/UNI-02-11/MotorGraphico-main/MotorGraphico/include/Core/Math/Interpolacion.h:45) **no es completamente cierto**: la cota de error «~4 ulp» no se cumple. Las guardas, monotonía y acotación sí describen la implementación.

   **Respecto a las pruebas posteriores citadas:** no encontré ninguna que vaya a fallar por exigir la fórmula convexa. `5099ab2b` introdujo esa referencia en `viaDouble`, pero **`6d1d1305` la sustituyó por una llamada a `tweenEscalar`** ([InterpolacionTests.cpp:283](/Users/admin/onegai-ramas/UNI-02-11/MotorGraphico-main/MotorGraphico/tests/Core/InterpolacionTests.cpp:283)). Los commits de `EstadoTween` comprueban progreso y curvas; los de cámara emplean tolerancias. El defecto anterior tampoco afecta al rango de entrada de `tweenVec2`, cuyas componentes son `float`.

2. **Extracción de cinemáticas — sin hallazgos.**

   [CMakeLists.txt:850](/Users/admin/onegai-ramas/UNI-02-11/MotorGraphico-main/MotorGraphico/CMakeLists.txt:850) conserva los tres `.cpp`, cada uno una vez, en `motor_core`. `titulo` y `alSalirDelTitulo` desaparecen del fichero original y quedan en `ApplicationCinematicaTitulo.cpp`; `escena` y `emocionesVista` permanecen en `ApplicationCinematicaEscena.cpp`.

   Este último fichero no tiene diferencias entre `caa7565a` y HEAD. No encontré funciones duplicadas o perdidas ni eliminación del cableado C2-04b. El cambio posterior del título a `actualizaTitulo` es coherente con U4-01.

3. **Polvo antes de la cámara — sin hallazgos.**

   El orden de [ApplicationBucle.cpp:157](/Users/admin/onegai-ramas/UNI-02-11/MotorGraphico-main/MotorGraphico/src/Engine/ApplicationBucle.cpp:157) es compatible: vida → polvo → cámara. El polvo usa celda, alturas y tiempo; no consume la vista ni el estado interno de la cámara antigua.

   `dtPolvo` cubre pausa crítica y menú; el argumento de batalla aplica D-O7. Las expresiones actuales coinciden con las que exige `pausa_del_polvo_en_application_bucle` ([PolvoDePasosTests.cpp:357](/Users/admin/onegai-ramas/UNI-02-11/MotorGraphico-main/MotorGraphico/tests/Game/PolvoDePasosTests.cpp:357)).

   **Límite de cobertura:** esa prueba textual comprueba presencia y argumentos, **no el orden relativo** respecto a la cámara. No considero incorrecto el orden actual.

4. **CursorClip — sin hallazgos.**

   Las pruebas procedentes de nube siguen siendo compatibles:

   - La frontera exacta cruza porque la condición para permanecer es `resto < libre - tolerancia`; con presupuesto disponible entra en el siguiente fotograma y emite su evento ([CursorClip.cpp:98](/Users/admin/onegai-ramas/UNI-02-11/MotorGraphico-main/MotorGraphico/src/Game/CursorClip.cpp:98)).
   - El desbordamiento de milisegundos se rechaza explícitamente antes de iniciar o modificar el cursor ([CursorClip.cpp:20](/Users/admin/onegai-ramas/UNI-02-11/MotorGraphico-main/MotorGraphico/src/Game/CursorClip.cpp:20)).

   `pendiente` también representa cortes con tiempo restante cero. Las pruebas de frontera de nube usan el presupuesto por defecto y no entran en conflicto con esa semántica.

5. **U3-06.E06b frente a U4-07.E07 — sin hallazgos.**

   Actúan sobre estados distintos: U3-06 selecciona máquinas/clips de PNJ y enemigos después de `refreshWorldMarkers`; U4-07 actualiza el emisor del jugador y lo registra mediante `VidaNivel`.

   Las máquinas conservan las guardas de cinemática y ruta de combate ([ApplicationBucle.cpp:301](/Users/admin/onegai-ramas/UNI-02-11/MotorGraphico-main/MotorGraphico/src/Engine/ApplicationBucle.cpp:301)). El polvo se dibuja en la pasada `renderShadow`, anterior al dibujo de entidades; no modifica sus clips, posturas ni destinos de animación. No encontré pisado entre ambos paquetes.