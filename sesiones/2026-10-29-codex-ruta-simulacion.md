# RECURSOS-02 — previsualizar cambio de fichero desde el editor

- **Fecha:** 2026-10-29
- **Autor:** Codex
- **Commits:** `8f6ab76` (implementación probada); cierre documental posterior en codex/perfilador-motor.
- **Deja esperando a Oriol:** aceptación jugable y Windows; aplicación de migraciones queda fuera de este lote.

## Encargo e implementación

«Continua» después de RECURSOS-01. Fila reclamada antes de archivos. PlanRutaData,
PlanificadorRuta y vista propia; biblioteca motor_plan_ruta sin GL, conectada a
ProjectPanelState → ProjectPanel → motor_editor. CMake/include juntos.

Simula cambio del fichero de nivel dentro del mismo directorio. Relee manifiesto,
comprueba propiedad y colisiones físicas/declaradas (incluidos symlinks rotos),
presenta campos/valores anteriores/propuestos y observa el MapId nativo existente.
No inventa IDs ni mueve/escribe archivos. El informe conserva límites/avisos.
Lector conserva literal y resuelve entrada legacy como ProjectIndex (aventura
antes de nivel); normalizador compartido, sin duplicar reglas.

## Verificación y límites

15 casos nuevos del planificador y uno del controlador pasan. Humo real 1/2 con
mapa limpio y destino ausente. Diez casos Python de paridad. Fábrica general:
226/242 pasan, los mismos 16 tests y cinco checks fallidos de la base 9349d3e;
ningún fallo nuevo. Guardia contrastada: 49 duplicados idénticos heredados.
EditorApplication permanece en 500 líneas; nuevos archivos pequeños, lógica sin
GL en biblioteca y llamador real. No se actualiza la línea base para ocultar deuda.

Regresiones reproducidas y corregidas: botón en barra desplazaba buscador a
escala 2; se pasó al menú/Referencias sin relajar la búsqueda. Captura del plan
era prematura; se espera el fotograma visible y se borra geometría anterior.
Revisión visual detectó botones parcialmente cortados: listado reducido y
comprobación de borde inferior añadida. Windows y visto manual pendientes.

Rama propia preservada; no se integra ni commitea WIP ajeno del árbol compartido
(auras, Application, contenido y herramientas Fábrica). Fábrica se usa externamente
con raíz explícita; no se incorpora su ejecutor ajeno. Canon/cartas sin cambios.
Guía: MotorGraphico/planificacion/replica/RECURSOS-02.md. Evidencia fuera de Git,
out/fabrica/RECURSOS-02/. Entrega limpia confirmada abajo.

## Entrega desde commit limpio

8f6ab76 compilado desde clon limpio con fuentes de dependencias, sin reutilizar
objetos: 45,7 s. Ocho objetivos CTest pasan (6 puros y 2 humos GL), 4,8 s; diez
regresiones Python pasan. Fábrica entregar VERDE de alcance fabrica_ruta.json;
ZIP/logs/contexto en out/fabrica/RECURSOS-02/entrega_limpia/. --sin-clon se usa
sobre el clon previamente creado por git-safe, con diff HEAD vacío antes/después.
Capturas finales revisadas en visual/ruta_modal1.png y ruta_modal2.png.
