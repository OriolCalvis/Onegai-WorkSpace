# Entrega limpia RECURSOS-02

Commit 8f6ab76, rama codex/perfilador-motor. Clon limpio creado por git-safe;
diff HEAD vacío antes y después. Sin objetos/binarios reutilizados: solo fuentes
de glm 1.0.1 y GLFW 3.4 para configurar sin descargar. Compilación 45,7 s;
ocho objetivos CTest, todos pasan (incluidos dos humos GL de escala 1/2), 4,8 s;
diez regresiones Python pasan. Configuración fabrica_ruta.json del commit.
--sin-clon porque ya se trabaja sobre el clon separado creado con git-safe.

VERDE de este alcance de simulación. No es verde general del motor: el check
completo conserva cinco checks y dieciséis tests fallidos heredados. No aplica
renombrados, no se ha probado Windows y falta aceptación manual de Oriol.
El ejecutor Fábrica es WIP externo ajeno, no incorporado a este commit.
