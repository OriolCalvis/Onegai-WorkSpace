# Sesión 15/10/2026 — Claude, carril `ciudad` (AV15-03b, 2.ª vuelta del ruido de la ciudad)

Lanzado por Claude Cowork en la [ronda del 15/10](../orquestador/ronda_15-10.md). Informe:
[planificacion_alcance_y_vision/RUIDO_CIUDAD_2.md](../planificacion_alcance_y_vision/RUIDO_CIUDAD_2.md).
Paquete: `MotorGraphico-main/MotorGraphico/out/claude_ciudad02/` (`aplicar.sh`, preimagen md5).

## Qué pasó

- Compilé `juego` en `/tmp/ag/ciudad/build` (282/282). Justo después, la máquina virtual dejó de responder:
  device_bash falló en todas las llamadas durante más de diez minutos (el contador subía también por otros
  agentes). Seguí en mi contenedor con copias de los ficheros de Oriol (traídas con device_stage_files).
- Medí el caos con una herramienta nueva de solo lectura (`herramientas/medir_ruido.py`) y lo miré con una vista
  esquemática isométrica hecha desde los datos (`herramientas/vista_ciudad.py`). **No son capturas del juego.**
- Cambié el generador (`gen_ciudad.py`, `gen_ciudad_deco.py`) y `despejar_calles.py`; regeneré `ciudad_centro.json`:
  297 → 190 objetos, el suelo idéntico, más la clave `"carriles"` (232 casillas) para `pnj_motor`.
- Guardas en la copia limpia: `medir_calles --comprobar` bien (19/19, 7/7 carriles), `conectividad` 100 %,
  `despejar_calles --carriles` bien. `aplicar.sh` aplica y se niega la 2.ª vez.

## Lo que falta (no se pudo sin la máquina virtual)

- `ctest -R "ciudad|boundington|nivel"` con el paquete aplicado.
- Capturas del juego antes/después: `herramientas/hazla_sonda_ciudad.sh <build> antes_|despues_` y `hoja.py`.
- La prueba de `aplicar.sh` en `/tmp/ag/ciudad/prueba` (se hizo en el contenedor).

## Decisiones que quedan para Oriol

Ropa fuera del Distrito Comercial; seis `anciana` (¿todas con papel?); banderas de la puerta sur; el suelo
rayado de las calles del norte (pide regenerar el TMX). Detalle en el informe, §7.
