# Corrección del creador: minotauros, rasgos y previsualización — 26/10/2026

Autor: Codex. Oriol autoriza aplicar el orden del estudio y confirma expresamente la anatomía: tors i braços sobre cos de toro.

## Base y aislamiento

Rama codex/revision-reciente-20261026, base 358d255. Worktree /tmp/onegai-reciente-20261026. La copia original sigue en claude/escenas01 con trabajo ajeno (aura, shaders, Application, documentación y sesiones); wt_integ conserva su WIP de muebles. No se modifica ni se commitea trabajo ajeno. Reclamo CREADOR-FIX-26 en TABLERO y relevo en planificacion/seguimiento/CREADOR-FIX-26.json.

## Producto

- Familia minotauro autónoma para los dos minotauros y aeronautas: datos locales existentes de composición centauro, 40 frames, 4 diagonales, torso con brazos y cuatro patas. Sin nuevas generaciones ni llamadas a PixelLab. El fallback a orco se rechaza al generar si falta la familia.
- Máscaras propias: ropa del torso separada del lomo y las patas; pantalones y botas humanos no se ofrecen ni pintan sobre las cuatro patas. Anclas sobre el dibujo aun cuando una mano queda ocluida. Reserva superior para pelo, capuchas y cuernos; ningún complemento del minotauro toca el borde tras la regeneración.
- RasgosApariencia: conservar los naturales, quitar o sustituir; cuernos separados de rasgos genéricos. Los complementos explícitos sustituyen su capa natural también en apariencias antiguas sin quitarCapas. Migración de talla grande a minotauro en guardados/composiciones anteriores de esas tres razas.
- Doble vista de Aspecto, frente y espalda según el giro, alternando caminar y respirar. VisibilidadRasgos mide los píxeles superiores y avisa por frame cuando una pieza elegida queda completamente oculta. Avisos cacheados junto al retrato, conservados al reservar el atlas.
- Alas con extensión corporal y perspectiva de lado cercano/lejano; colas con longitud corporal y ancla de grupa del centauro. Orejas separadas para seguir el tinte de piel; cuernos conservan material propio.
- Generados manifiesto y atlas coherentes: 72 razas, 8 capas raciales y 8 tallas. Diez recortes residuales pertenecen a las colas de tres hombres lagarto; ninguno del minotauro. No se cambian mapas ni contenido canónico.

## Forma y alcance

Dos clases GL-free nuevas enlazadas por motor_level y alcanzadas por juego (medir_alcance.py, informe /tmp/creador-fix-alcance.json). No hay src compilados sueltos. EditorApariencia.cpp 623 -> 622 líneas; MontajePersonajes.cpp 500 -> 499; PaginasCreacion.cpp 447. No se aumenta ningún fichero heredado por encima del techo. No se actualiza la línea base.

## Verificación

- Release juego compila. Humo de título y ciudad de la carpeta de entrega, 3 frames, cierre normal.
- Cinco pruebas Python: 40 frames/direcciones, reconstrucción exacta del dibujo por capas, anclas sobre cuerpo, patas sin botas/pantalones y cuernos sin recorte.
- Pruebas de personajes, editor (incluidas seis nuevas), guardado, nueva partida, cuaderno y tutorial; resultado final adjunto en la entrega.
- test_pergamino mantiene exactamente los cuatro fallos históricos de MesaCombateTests (136, 214, 299, 359), cotejados con base-ctest.log de UI-PUERTAS-01. Las comprobaciones nuevas de doble vista y cache de avisos pasan.
- validar_todo: 0 errores, 24 avisos, igual que la base.
- Guardia rápida: 49 problemas heredados; completa: 49 instantáneos y 52 de deuda respecto de la línea base histórica. EditorApariencia ya tenía 623 líneas en HEAD antes de esta tarea; ahora 622. Las clases nuevas no aparecen como huérfanas. Duplicados '* 2' heredados no se tocan. El hook versionado supone la raíz antigua MotorGraphico/ y no localiza el motor en este monorepo; guardias ejecutadas expresamente, sin saltarlas ni cambiar sus reglas.
- Láminas del creador obtenidas desde el lienzo y composición reales del motor, rasterizadas fuera de GPU; inspeccionadas. Ventana nativa contrastada: doble vista presente. La captura/acción posterior de CUA dio error del servicio de macOS; la evidencia reproducible está en las láminas y pruebas.

## Entrega

MotorGraphico-main/MotorGraphico/out/juego-creador-corregido-20261026/: juego, assets, Obrir joc corregit.command, VERSION.txt y verificacio/ con láminas, logs y RESULTAT.html. Fuente recuperable en la rama; no se fusiona sobre el árbol original sucio ni se hace push. TABLERO original solo recibe la fila propia y la sesión, sin incluir su WIP ajeno en el commit.

## Ajuste final de distribución

La carpeta sincronizada bloqueó dos copias redundantes sobre recursos de monumentos sin cambios y ralentizó el arranque. Se detuvieron únicamente esos procesos propios. Se verificaron binario, manifiesto y 128 atlas referenciados contra la build. Para jugar sin esa espera se deja una copia materializada sin atributos sincronizados en ~/.cache/onegai/juego-creador-20261026/; el lanzador permanente del paquete apunta a ella. Título y ciudad desde esa copia: tres frames, salida normal. Producto: 3efc504; los commits documentales posteriores no cambian el binario.
