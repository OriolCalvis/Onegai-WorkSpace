> Snapshot importado de replica_motor.zip el 29/10/2026. Reproduce los estados
> declarados en el ZIP; no es una auditoría del motor actual ni una medida de
> rendimiento. Revisar código, integración y aceptación antes de usar esta cifra
> para priorizar. Véase INTEGRACION.md.

## Parity: 10.0 / 100

features 10.0  (25 counted, must-haves 0 of 15 done)

Not shippable yet: 15 must-have features are not done.

## By area, weakest first
- iluminacion                    0.0  (1 features)
- proyecto                       0.0  (4 features)
- animacion                      0.0  (3 features)
- flujo                          0.0  (4 features)
- camara                         0.0  (1 features)
- cinematicas                    0.0  (2 features)
- efectos                        0.0  (1 features)
- eventos                       12.5  (3 features)
- editor                        16.7  (3 features)
- audio                         50.0  (1 features)
- entidades                     50.0  (1 features)
- ia                            50.0  (1 features)

## Missing, in build order
- [must] animacion: Clips de animación con eventos de fotograma (Animation Events), no  (U3-05)
- [must] animacion: Máquina de estados de animación en datos (Animator Controller), no  (U3-06)
- [must] cinematicas: Cinemáticas con pistas en paralelo (Timeline), no  (U4-02)
- [must] editor: Prefabs de grupos de objetos, no  (U4-05 (MAP-08))
- [must] editor: Un solo editor para escenas y objetos, no  (U2-05)
- [must] eventos: Scripting visual sin código (grafos de órdenes), no  (U3-04 + E-05 + E-06)
- [must] eventos: Triggers con entrada / permanencia / salida (OnTriggerEnter/Stay/Exit), no  (U3-02)
- [must] flujo: Consola e inspector de estado en ejecución, no  (F-04)
- [must] flujo: Play Mode dentro del editor, no  (F-01 (espera DEC-06))
- [must] proyecto: El juego arranca desde el proyecto (Build Settings), no  (G-08)
- [must] proyecto: Módulos de juego separados del motor (packages y asmdef), no  (U2-03)
- [must] proyecto: Project Settings: escena inicial / catálogos / idioma, no  (U2-02)
- [must] audio: Audio por escena y zona con volúmenes separados (AudioSource + AudioMixer), partial  (U1-01: AudioDirector escrito y probado; el juego no lo usa)
- [must] editor: Ventana de proyecto y jerarquía de escena, partial  (E-11 (resta))
- [must] entidades: Identificadores estables de objetos de escena (instance ID), partial  (U1-02: HandlePool escrito; World no lo usa)
- [should] animacion: Curvas de interpolación compartidas (AnimationCurve), no  (U3-07)
- [should] camara: Cámara virtual: seguimiento / sacudida / límites / dead zone (Cinemachine), no  (U4-01)
- [should] cinematicas: Editor de línea de tiempo, no  (U4-03)
- [should] efectos: Sistema de partículas genérico, no  (U4-07)
- [should] flujo: Gizmos de depuración: colisiones / regiones / rutas, no  (U4-06)
- [should] flujo: Recarga en caliente de datos y scripts, no  (F-03)
- [should] iluminacion: Luz ambiental global configurable, no  (U1-05)
- [should] proyecto: Plantillas de proyecto al crear uno nuevo, no  (U2-04)
- [should] eventos: Depurar el scripting paso a paso, partial  (E-08 (resta))
- [should] ia: Agentes que navegan hacia un objetivo (NavMeshAgent), partial  (U1-03: estado kChase existe sin transición)

## Left out on purpose (not scored)
- Asset Store y paquetes de terceros: la tienda y su red no son una función del motor
- Render 3D / PBR / HDRP y URP: el motor es isométrico 2D
- Exportar a consolas y móvil: licencias y SDK de plataforma; fuera del alcance
- Servicios en la nube (Unity Cloud / Analytics / Ads): servicios de la empresa; no son el motor
- Scripting en C# con recompilación de dominio: el motor es C++ con datos JSON; el sustituto es el scripting visual

## Yours, not in the original (not scored)
- Guardar y cargar con ranuras y autosave
- Rutinas de PNJ por franja del día
- Páginas de evento con condiciones
- Probar desde la casilla seleccionada
- Saltar cinemáticas
