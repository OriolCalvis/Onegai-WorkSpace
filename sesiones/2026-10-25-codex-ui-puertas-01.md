# UI-PUERTAS-01 — titulo con foco y un cartel por acceso

Fecha: 25/10/2026. Autor: Codex. C++ autorizado expresamente por Oriol.

## Base y aislamiento

Rama `codex/ui-puertas-01`, worktree `MotorGraphico-main/.ui-puertas-01` (raiz del workspace completo). Base `claude/gorran-trama-tests` (`a87d5f5`), mas los paquetes aprobados PL-JUEGO-01 (`524ad21`) y PL-JUEGO-02 (`6db03a9`) registrados por separado. Sin push ni merge; rama de Claude intacta.

El checkout original estaba en `claude/escenas01` con WIP ajeno: aura/moral, shaders, Application.cpp/ApplicationCierre.cpp, PergaminoTests, test_validar_sprites, assets y documentos compartidos. No se deshace ni se incluye en los commits. La fila se reclamo en TABLERO antes de crear archivos.

## Implementacion

- Solo bloque de entrada del titulo en Application.cpp: recordar seleccion, procesar teclas y raton, reconstruir HUD si cambia y despues confirmar. Se reduce una linea. Oriol verifica en el chat: «el menu ya bien, continua».
- CartelesPuertas.cpp y nueva cabecera: grupos de casillas contiguas (incluye diagonales) por targetLevel, con representante real cercano al centro. La misma agrupacion alimenta carteles y flechas de suelo. Conserva accesos separados al mismo destino. No cambia puertas ni transiciones.
- Sustitucion del rectangulo/contorno negro por luz rasante translucida en el umbral. Propuesta estetica mostrada en captura real antes de cerrar.
- lugares.json: propuestas explicitamente marcadas de emblema para la Gola Fosca y seis pasos de Gorran. Se conserva nombre y destino; libro de secretos para la entrada y mapa para los pasos subterraneos.
- Cinco pruebas nuevas GL-free en CartelesPuertasTests.cpp, integradas en test_pergamino mediante su cmake existente; enlace a motor_saves, sin fuentes de src compiladas sueltas ni nuevas librerias mixtas.

## Evidencia

Paquete `MotorGraphico-main/MotorGraphico/out/ui-puertas-01/`: plan y evidencia, capturas titulo.png/gorran.png, logs de base y finales. Gorran se captura desde copia temporal del nivel con jugador en (24,9), tras 120 frames para centrar camara; niveles originales intactos.

Build completa correcta. validar_todo: 0 errores y 24 avisos, igual que la base. Las cinco pruebas nuevas pasan. Los cuatro fallos de MesaCombate en test_pergamino ya existen sin los cambios de este encargo.

linea_base_core: 49 problemas instantaneos y 51 de deuda respecto de la linea base historica, exactamente iguales en los blobs de base PL-JUEGO-02 y finales; unica diferencia: Application.cpp 1283 -> 1282 lineas. CartelesPuertas.cpp 148 lineas; cabecera nueva 11. No se actualiza la linea base ni se tocan los duplicados `* 2.cpp`.

## Entorno y reproducibilidad

Los intentos normales de git commit quedaron esperando en refresco del indice: se detuvieron exclusivamente esos procesos propios y se registro con git-safe write-tree/commit-tree/update-ref, como prescribe la bitacora. Sin saltarse ni modificar las guardias; se ejecutaron expresamente, con los fallos heredados documentados.

Hubo timeouts de lectura del filesystem sincronizado y de pruebas. Para contrastar la guardia se ejecutaron tambien copias temporales de los blobs exactos de base/final: los informes solo difieren en una linea menos de Application. Tras restaurar la base para la comparacion se regenero CMake explicitamente y se recompilaron los fuentes restaurados: la ultima bateria incluye las cinco pruebas nuevas.

El aviso visual y los emblemas siguen identificados como propuesta para Oriol. Sin cambios de canon, nombres, destinos, entrada de andar propia ni copias antiguas.

Resultado final de CTest: **226/241 pasan; los mismos 15 fallos de la base (incluido test_personajes_capas por timeout), ninguno nuevo**. La ultima bateria incluye los cinco casos nuevos en verde. Timeout de base 60 s; de entrega 180 s. Registros completos adjuntos.
