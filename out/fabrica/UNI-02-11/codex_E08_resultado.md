Revisado **solo `ba50c70c`**, sin modificar ficheros ni compilar. Encontré **seis hallazgos de severidad media y uno bajo**. Las mutaciones propuestas abajo no se han ejecutado; las conclusiones se basan en el código y los logs del commit.

1. **Media — El hash omite estado relevante y los 30 ticks adicionales no garantizan cubrirlo.**  
   [EmisorCargaTests.cpp:37](/Users/admin/onegai-ramas/UNI-02-11/MotorGraphico-main/MotorGraphico/tests/Game/EmisorCargaTests.cpp:37)

   Incluye todos los bytes de cada `Particula`, tiempo, nacimientos y presupuesto, pero omite `origen`, `grupo`, definición, tipo, estado del RNG y residuo. Cambiar `origen` o `grupo` altera la imagen sin alterar el hash, tampoco tras seguir simulando.

   La afirmación de que continuar 30 ticks «delata» cualquier residuo distinto es falsa: reproduje en memoria la fórmula de emisión con residuos iniciales `0` y `0.0001`, tasa 500 y paso 1/60. Los 30 recuentos de nacimientos coinciden y los residuos siguen siendo distintos.

   **Cómo demostrarlo:** comparar el hash antes y después de `v[0].origen.x += 100`; será igual aunque cambie la proyección. Para el residuo, añadir una prueba de sensibilidad con esos dos valores y verificar que la continuación usada aquí no los distingue.

2. **Media — El presupuesto global no comprueba admisión exacta en 2048.**  
   [EmisorCargaTests.cpp:182](/Users/admin/onegai-ramas/UNI-02-11/MotorGraphico-main/MotorGraphico/tests/Game/EmisorCargaTests.cpp:182)

   Solo exige `maxVivasGlobal <= 2048` y `>1024`. Detecta una eliminación completa del límite, pero permite rechazar antes de llenarlo. Que el log observe 2048 no convierte esa observación en una condición de éxito.

   **Cómo demostrarlo:** mutar `PresupuestoParticulas::reserva()` de `vivas >= maximo` a `vivas >= maximo - 1`. E08 puede seguir pasando con 2047 vivas. Falta comprobar que el nacimiento 2048 se admite y el 2049 se rechaza, sin caducidades intermedias.

3. **Media — Las calaveras no prueban la frontera 8192/8193.**  
   [EmisorCargaTests.cpp:218](/Users/admin/onegai-ramas/UNI-02-11/MotorGraphico-main/MotorGraphico/tests/Game/EmisorCargaTests.cpp:218)

   Se comprueban **8160 y 8194**, no 8192 y 8193. Además, esos dos vectores se pasan al contador, no al mecanismo de recorte. La carga proyectada también avanza en múltiplos de 34.

   **Cómo demostrarlo:** cambiar el rechazo a `r.quads + qLote > quadsMax + 1`. E08 no distingue esa tolerancia indebida: siguen entrando 240 calaveras. Lo mismo sucede recortando una unidad antes. Una prueba exacta puede combinar **240 calaveras + 32 puntos = 8192**, y solicitar después otro punto; debe admitir los primeros y rechazar el último.

4. **Media — La prueba del polvo permite incluso que no nazca ninguna nube.**  
   [EmisorCargaTests.cpp:289](/Users/admin/onegai-ramas/UNI-02-11/MotorGraphico-main/MotorGraphico/tests/Game/EmisorCargaTests.cpp:289)

   Comprueba ≤16 emisores y ≤256 vivas, pero no los **≤7/≤31** solicitados. Tampoco exige actividad positiva: dos estados vacíos tienen el mismo hash, cero quads cumple el límite y el contador de pasos puede avanzar sin crear emisores.

   El contrato escrito fija 16/256; los valores 7/31 aparecen como medidas en el informe. Con los criterios de esta revisión, falta convertirlos en comprobaciones.

   **Cómo demostrarlo:** eliminar únicamente el `m_emisores.emplace_back(...)` de `PasosDePolvo::creaNube`, conservando `m_pasos++`. Las condiciones de polvo de E08 siguen satisfechas. Otra prueba necesaria debe detectar una regresión que eleve la carga habitual a 8 nubes o 32 vivas.

5. **Media — No se demuestra que recortar la proyección conserve el estado de simulación.**  
   [EmisorCargaTests.cpp:134](/Users/admin/onegai-ramas/UNI-02-11/MotorGraphico-main/MotorGraphico/tests/Game/EmisorCargaTests.cpp:134), [informe:30](/Users/admin/onegai-ramas/UNI-02-11/out/fabrica/REPLICA-R1/U4-07.E08_medida.md:30)

   Todas las ejecuciones comparadas realizan la misma proyección con el mismo presupuesto. Una modificación determinista del estado durante el descarte produciría hashes iguales entre ejecuciones. Los 30 ticks finales sin proyectar no aportan una ejecución de control equivalente.

   **Cómo demostrarlo:** introducir una mutación que ponga `Particula::k = 0` al descartar una partícula. La igualdad entre ejecuciones puede mantenerse pese a alterar simulación. La prueba adecuada compara emisores idénticos proyectados con presupuesto cero, 8192 y suficiente para todos, verificando estado antes/después y evolución posterior idénticos.

6. **Media — El log verde no corresponde al fichero final y no respalda todas las medidas publicadas.**  
   [U4-07.E08_ctest.log:153](/Users/admin/onegai-ramas/UNI-02-11/out/fabrica/REPLICA-R1/U4-07.E08_ctest.log:153), [informe:15](/Users/admin/onegai-ramas/UNI-02-11/out/fabrica/REPLICA-R1/U4-07.E08_medida.md:15)

   El código registra **seis casos**, pero el log termina con **cinco**. Falta `aforo_de_un_emisor_con_pocos_emisores_llega_a_256` y su salida. Por tanto, ese log no acredita el caso que sostiene la medida de 256 ni las 6030 rechazadas.

   También difieren los tiempos: el informe publica máximos de avance/proyección **0,155/0,190 ms**, frente a **0,111/0,132 ms** en el log; polvo **0,0125 ms**, frente a **0,0056 ms**.

   **Cómo demostrarlo:** contar `MOTOR_TEST_CASE` en el blob del commit y contrastarlo con «Ejecutados 5». El log de mutaciones sí menciona el sexto caso, lo que confirma que las evidencias recogen estados distintos.

7. **Baja — Debug no establece una «cota alta» de CPU.**  
   [U4-07.E08_medida.md:3](/Users/admin/onegai-ramas/UNI-02-11/out/fabrica/REPLICA-R1/U4-07.E08_medida.md:3)

   Medir una ejecución Debug permite publicar muestras observadas, pero no garantiza un límite superior del coste futuro, en otras máquinas o bajo otra carga. Mantener los tiempos como informativos es correcto; llamarlos «cota alta» excede la evidencia.

   **Cómo demostrarlo:** repetir el mismo binario bajo contención de CPU puede superar el máximo registrado sin cambiar el sistema de partículas.

En los demás puntos:

- **Aforo 256:** el caso de cuatro emisores sí exige alcanzar 256 y observar rechazos; ejercita saturación, aunque no aísla la secuencia exacta 256/257. Su ejecución final no está acreditada por el log verde adjunto.
- **Recuento independiente:** **sin hallazgo**. No llama a `quadsDeDestellos` ni a su auxiliar `quadsDeForma`; comparte las tablas de formas que también usa el pintor, conforme al contrato.
- **Mutaciones:** `a`, `b`, `c`, `d1` y `e` muestran fallos coherentes con lo mutado. `e` mata la pérdida del límite de **16** mediante los 40 pasos rápidos; no demuestra 7/31. `d2` registra fallo, pero omite el diagnóstico: acredita un resultado rojo, no su causa concreta. Ninguna acredita los errores de frontera anteriores.
- **Determinismo temporal:** **sin hallazgo** en la entrada de simulación: semillas explícitas y paso fijo. El reloj solo alimenta medidas informativas.
- **GPU/FPS:** **sin afirmación indebida de medición realizada**. El informe declara pendiente el coste real de GPU/driver y la observación GL; el volumen de vértices es una estimación.
- **`require`/`assert` y CMake:** **sin hallazgos**. `MOTOR_CHECK` evalúa también en Release; no introduce `assert()`. El target queda registrado en CTest y en la suite GL-free. El límite genérico de 64 está reservado según el contrato, por lo que no corresponde exigir aquí un rechazo del emisor 65.