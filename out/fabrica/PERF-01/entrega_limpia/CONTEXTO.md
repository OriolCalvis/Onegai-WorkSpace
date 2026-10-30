# Verificación limpia de PERF-01

Commit 7fa2117. Clon compartido creado mediante git-safe, sin cambios de archivos
versionados antes y después de comprobar. Se recompilaron juego y las cuatro
pruebas del lote desde fuentes del commit; solo se reutilizaron las fuentes de
glm 1.0.1 y GLFW 3.4, no objetos o binarios. Fábrica ejecutada con --sin-clon
porque el clon ya estaba creado con git-safe. Alcance: fabrica_perfil.json.

Build limpio: 34,9 s; cuatro objetivos CTest pasan (diez casos nuevos en el
perfilador) y diez regresiones de paridad pasan. Humo GL del binario limpio:
cuatro filas CSV, contadores positivos y salida 0. La comparación general completa
se conserva fuera de esta subcarpeta: 224/240 pasan, dieciséis fallos heredados,
ninguno nuevo; los cinco validadores con fallo también son heredados.

Prueba jugable para Oriol:
1. Desde el directorio del binario: MOTOR_PERFIL_CSV=perfil.csv ./juego 120.
2. Abrir un nivel, caminar y abrir inventario; cerrar la partida.
3. Ver CSV con frames consecutivos y medidas; render_present incluye presentación,
   no tiempo GPU. La aceptación manual no se ha atribuido a Oriol.
