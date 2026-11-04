---
name: fabrica
description: Ciclo de trabajo con validadores automáticos para cualquier encargo de Software/. Fija una línea base antes de tocar nada, vigila con checks continuos, depura y entrega desde un clon limpio con registros y evidencia. Úsala al empezar, durante y al cerrar todo encargo que cambie código o datos.
---

# Fábrica: validadores automáticos, registro total y entrega con pruebas

Una sola herramienta para todos los agentes, `fabrica.py`, con solo biblioteca estándar. Funciona igual
en Mac, Windows y Linux. Pone en práctica la regla 6 y el «hecho = evidencia» de `AGENTS.md`. **No
sustituye** al «visto» de Oriol: lo prepara.

Lo que garantiza:

- **Nada se da por bueno sin registro.** Cada orden, cada check, su salida entera y su veredicto
  quedan en `out/fabrica/<ENCARGO>/`. `fabrica.log` lleva una línea JSON por evento.
- **Lo heredado no te bloquea; lo nuevo, sí.** La línea base se toma antes de tocar nada. Después,
  un fallo cuenta como tuyo si no estaba en la base: una prueba que antes pasaba, un error de
  validación nuevo o un contador de avisos que sube. Por eso hoy los 22 avisos de `validar_todo` no
  molestan, y el aviso número 23 sí.
- **La entrega se prueba como la verá otro.** Se hace un clon limpio del commit. Lo que no está
  commiteado no cuenta.

```
F=".agents/skills/fabrica/fabrica.py"      # desde la raíz del repo; en Windows, py en vez de python3
```

## El ciclo, siempre en este orden

### 1. Al reclamar el encargo, antes de tocar ningún fichero

```bash
python3 $F inicio PLAU-01
```

- Guarda el entorno (`entorno.json`: rama, commit, ficheros sucios y versiones de git, cmake,
  compilador y python).
- Guarda la **línea base** (`linea_base.json`): todos los checks de la fase completa.
- Si tarda mucho, usa `--rapida`, pero entonces la entrega comparará la compilación y las pruebas
  contra nada y cualquier fallo contará como nuevo.

### 2. Mientras trabajas: fase de checks continuada

```bash
python3 $F vigilar --max 20          # agentes: SIEMPRE con --max, para no colgar la shell
python3 $F check                     # una pasada rápida a mano (validadores)
python3 $F check --fase completa     # + compilar + ctest gl-free
```

- **`vigilar`** vuelve a pasar la fase rápida cada vez que cambia un fichero vigilado (`.cpp`, `.h`,
  `.py`, `.json`, `.tmx`…) y escribe el veredicto: **VERDE** si no hay fallos nuevos, **ROJO** si los
  hay.
- **Regla:** no se sigue escribiendo con un ROJO abierto. Se pasa al paso 3.
- Oriol puede dejar `vigilar` sin `--max` en una terminal mientras prueba.

### 3. Cuando sale ROJO: depurar

```bash
python3 $F depurar ctest_gl_free     # o validar_todo, compilar, linea_base_core
```

- Repite el check con su orden de depuración (ctest `-V --rerun-failed`, cmake `--verbose -j1`).
- Marca cada fallo como **NUEVO** o heredado.
- Saca **el primer error de los nuevos** con contexto y lo cruza con lo que has cambiado desde la
  línea base (`git diff --stat`, más los ficheros sin seguimiento).
- Deja el informe en `depuracion/<fecha>_<check>.md`, con la salida entera al lado.

Para seguir:

- **Si el fallo está en un fichero que has tocado:** arréglalo y `check` hasta VERDE.
- **Si no es tuyo ni de tu zona de ficheros:** no lo arregles. Apúntalo en el encargo (`AGENTS.md`
  §2.4).

### 4. Al cerrar: entregar

```bash
git add … && herramientas/git-safe.sh commit …     # primero el commit: el clon solo ve lo commiteado
python3 $F entregar --encargo-md orquestador/encargos/PLAU-01_los_enemigos_persiguen.md
```

- Clona el commit en un directorio temporal y pasa la fase completa allí.
- Escribe `EVIDENCIA.md`: el veredicto, una tabla de checks (ok, heredado o **NUEVO**, con los
  contadores antes y después), los commits del encargo y el diff.
- La pega en el encargo, entre `<!-- fabrica:evidencia -->` y su cierre. Si vuelves a entregar, la
  sustituye.
- Genera `HUELLAS.sha256` de todos los registros y `entrega_<ENCARGO>.zip` con todo.
- Si sale VERDE, imprime la línea para `RESULTADOS.md` en estado *producido*.
- Sale con código 1 si hay fallos nuevos. **Con ROJO no se entrega.**

**Después, a mano:** rellena en la evidencia las tres líneas de la **prueba jugable**: cómo se lanza,
qué hacer y qué se ve. La fábrica deja el hueco porque no puede jugar. Pasa el zip a Oriol junto con
el encargo.

### Disco

- Toda fase que compila (checks con `necesita_disco`: hoy, `compilar`) y todo clon de entrega exigen
  `disco_minimo_gb` libres (3 GB en `fabrica.json`; `FABRICA_DISCO_MIN_GB` lo cambia para una orden).
- Afecta a `inicio` (salvo con `--rapida`), `check --fase completa`, `vigilar --fase completa` y
  `entregar`. Se comprueba **antes** de ejecutar nada: no queda una línea base ni un clon a medias.
- Si no hay espacio, la orden sale con código **3** y deja `bloqueada_disco` en `fabrica.log`. Es un
  bloqueo, no un ROJO: en el encargo, `BLOQUEADA: disco (N GB libres)`.
- El clon de la entrega es `--shared --no-checkout`: reutiliza los objetos del repo (no copia los ~2 GB
  de `.git`) y saca un solo árbol, el del commit.
- Todas las órdenes git de la fábrica llevan `-c core.trustctime=false -c core.checkStat=minimal`
  (repo en iCloud).

### Siempre que dudes

```bash
python3 $F estado        # encargo activo, línea base, último veredicto, últimos eventos
```

## Configuración: `fabrica.json`

- Cada check tiene estos campos:
  - `id`, `fase` (`rapida` o `completa`), `cwd` y `cmd` (admite `{motor}`, `{preset}` y `{python}`);
  - `depurar` (opcional);
  - `items_regex`: los nombres de lo que falla. Es lo que se compara con la línea base.
  - `errores_regex` y `avisos_regex`: el último número que casa es el contador.
  - `corta: true`: si falla, no sigue. Por ejemplo, sin compilar no hay pruebas.
- `preset` va por plataforma: `darwin`, `win32` o `linux`.
- `vigilar` e `ignorar` son globs.
- `clon_copiar` lista lo que hace falta en el clon y no está en git.
- **Las regex `_ajustar` se escribieron sin ver la salida real** de `validar_todo` ni de
  `linea_base_core`. La primera vez, compara `depurar <id>` con su `.log`. Si un fallo sale como
  «código N» en vez de con nombre, la regex no casa: arréglala aquí, no en el script.
- Añadir un check, por ejemplo `validar_cinematicas.py` o el auditor de recompensas, es añadir una
  entrada al JSON.

## Lo que no hace

- No hace push, merge ni commits: eso sigue con `herramientas/git-safe.sh` y lo junta Oriol.
- No decide si algo es de tu zona: eso lo dice el encargo.
- No juega. La prueba jugable y el «visto» son de Oriol.

## Dónde queda cada registro

```
out/fabrica/<ENCARGO>/
  fabrica.log                 una línea JSON por evento (inicio, check, veredicto, depurar, entregar…)
  entorno.json                rama, commit, ficheros sucios y versiones al empezar
  linea_base.json             todos los checks antes de tocar nada
  ultimo.json                 el último check y su comparación
  ejecuciones/<fecha>_<fase>/ <check>.log con la salida entera, más resumen.json
  depuracion/<fecha>_<check>.md + .log
  EVIDENCIA.md · HUELLAS.sha256 · entrega_<ENCARGO>.zip
```

`out/` tiene que estar en `.gitignore`. Los registros se entregan con el zip, no por git.
