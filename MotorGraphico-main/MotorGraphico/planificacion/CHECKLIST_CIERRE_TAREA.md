# Checklist de cierre de tarea — las preguntas que avalan que algo está hecho

Para cualquier paquete del Gantt (C/D/E/F/G/H) y para las filas del `TABLERO.md`.
Se responde **antes** de escribir `estado: closed` en
`planificacion/seguimiento/<PAQUETE>.json`.

**Regla de uso.** Cada pregunta tiene al lado la orden que la responde. Si una
pregunta no se puede responder con una orden o con una prueba ejecutada, se
responde «no» — no «creo que sí». El historial del proyecto tiene cinco paquetes
cerrados que el jugador no ve (G-09, G-10, G-13, G-14, G-17): en los cinco, la
respuesta fue «creo que sí».

Motivo y rango formal: [`decisiones/D0006`](../../../decisiones/D0006-criterios-de-cierre-por-alcance.md).
Medición del problema: [`ALCANCE_REAL_2026-09-17.md`](ALCANCE_REAL_2026-09-17.md) §6.

---

## Bloque 0 — Las cuatro que vetan el cierre

Si alguna falla, **no se cierra**, aunque todo lo demás esté en verde. Son las
cuatro que se han incumplido de verdad en este repo.

| # | Pregunta | Cómo se responde | Por qué veta |
| --- | --- | --- | --- |
| **0.1** | ¿El producto de esta tarea lo usa algún ejecutable que una persona pueda abrir? | `python3 tools/medir_alcance.py --json /tmp/a.json` y mirar si los ficheros del paquete salen en `juego`, `motor_editor` o `level_editor` | Es la diferencia entre *probado* e *integrado*. `Locale` (G-17) y `CharacterCreationScreen` (G-09) tienen pruebas verdes y ningún consumidor |
| **0.2** | ¿La prueba prueba código de producción, o prueba algo que ella misma define? | `python3 tools/analizar_duplicados.py --solo LOGICA_EN_EL_TEST` | G-13 y G-14 están cerrados y su lógica (`buildBattleMenuSkillActions`, `HudTimer`, `ShopEngine`) solo existe dentro de su propio fichero de test: la prueba se prueba a sí misma |
| **0.3** | ¿Compila HEAD en un clon limpio, sin mi árbol de trabajo? | clon limpio + `cmake --preset macos-debug && cmake --build` + `ctest -L gl-free` | Ha pasado tres veces: registro commiteado antes que el código, o un `include(cmake/X.cmake)` subido sin el `.cmake`. HEAD dejó de configurar |
| **0.5** | ¿Mi cambio deja algún fichero de `src/`, `include/` o `editor/` por encima de 600 líneas, o hace crecer uno de los que ya lo pasan? | `python3 tools/linea_base_core.py` | LEY DEL REPOSITORIO desde el 21/09. `Application.cpp` creció de 2372 a 2472 líneas en una noche mientras se discutía cómo partirlo. Lo que añadas a un fichero de cruce va en una clase nueva con su prueba |
| **0.4** | ¿He reclamado fila en `TABLERO.md` **antes** de tocar el primer fichero, y he commiteado solo lo mío? | `git diff --stat HEAD -- <mis rutas>` y leer la fila propia | E-05/E-06/E-14/E-16/G-13 fueron sin fila. Sin fila, dos sesiones escriben lo mismo y nadie sabe de quién es el WIP |

---

## Bloque 1 — Probado (existe y se sostiene)

1. **¿Qué prueba nueva falla si deshago mi cambio?** — deshacerlo de verdad
   (`git stash`) y verla caer en rojo. Si no cae, la prueba no prueba nada.
2. **¿Cuántos casos pasan, sobre cuántos?** — `ctest -R <mi_test> -V`. Se escribe
   el par `count/pass/fail` en el JSON, no «verde».
3. **¿He usado `require()` y no `assert()`?** — en Release `NDEBUG` hace que
   `assert()` no evalúe: la prueba pasaría sin comprobar nada (Parte 3).
4. **¿La prueba es determinista?** — si usa RNG, con semilla fija
   (`ScriptedRng`). Una prueba que pasa 9 de 10 veces es una trampa.
5. **¿Mi `.cmake` está versionado en el mismo commit que su `include()`?** —
   `git show --stat HEAD | grep cmake`.
6. **¿Quedan rojos ajenos que yo haya causado?** — comparar la suite antes y
   después. Si un rojo es de otra sesión, se registra en `TABLERO.md`
   «Pendiente» y **no se toca** (regla 2).

## Bloque 2 — Integrado (se llega desde la interfaz)

7. **¿Qué tecla, botón o menú lleva a esto?** — se escribe la ruta exacta:
   «Título → Campaña → Día 1 → E sobre el tendero». Si la ruta pasa por editar
   un JSON o por una flag de línea de órdenes, **no está integrado**.
8. **¿La acción se completa, o solo empieza?** — abrir un panel no es editar;
   colocar un objeto no es guardar sus propiedades.
9. **¿Sobrevive a cerrar y volver a abrir?** — round-trip por el serializador.
   Aquí han muerto varios paquetes: el dato se escribe y no se vuelve a leer.
10. **¿Funciona con el contenido real, no solo con el fixture?** — cargar
    `assets/` de verdad. `test_boundington_tutorial` pasaba sin catálogo y el
    tutorial no se podía jugar.
11. **¿Qué pasa si el dato falta o está mal?** — el motor no puede petar: o
    `Result<T>` con error legible, o el problema sale en el panel de E-15.
12. **¿Algo de lo que he escrito duplica lo que ya había?** —
    `python3 tools/analizar_duplicados.py --fichero <mi fichero>`. Si sale
    `DUPLICADO`, `CLON_PARCIAL` o `SOLAPAMIENTO_FUNCIONAL`, se justifica por
    escrito o se reutiliza lo que ya existe.

## Bloque 3 — Terminado (lo usa alguien que no sabe cómo está hecho)

13. **¿Una persona nueva lo usa sin leer el código, sin terminal, sin IDs
    técnicos y sin que nadie se lo explique?** — se prueba de verdad, y se dice
    quién lo ha probado.
14. **¿Todo lo que se ve está en el idioma correcto y sin mojibake?** — la
    fuente dibuja acentos desde el 17/08; los JSON de aventura siguen vetados a
    tildes/ñ en `lines`/`speaker`/`text` hasta que Oriol levante la veda.
15. **¿Los nombres que ve el usuario vienen del dato, no de una tabla escrita a
    mano?** — el menú de combate fijo de `Application.cpp:654` es el
    contraejemplo: cinco entradas hardcodeadas que G-13 debía sustituir.
16. **¿Hay texto visible sin envolver/recortar?** — `BitmapFont::wrapText` y
    `clipText` existen; usarlas.
17. **¿El rendimiento es jugable?** — si se mide con el contador de FPS del HUD,
    se dice el número y se quita el contador antes de cerrar (está en la bandeja
    de Oriol como posible resto de depuración).

---

## Puertas extra según el tipo de tarea

**Motor / núcleo (fases C, D).**
- ¿El contrato está escrito donde lo busca el compilador (`include/`), no en un
  comentario del `.cpp`?
- ¿`comprobar_declaraciones_rpg.py` sigue en 0 declaraciones sin cuerpo nuevas?
  (`python3 tools/comprobar_declaraciones_rpg.py`)
- ¿Los nombres respetan las trampas de la Parte 3? (`RPG::Catalogs::X`,
  `Catalog<T>::find()` devuelve puntero, `forEach` en vez de `begin/end`,
  `Stat` en orden CON/DES/INT/CAR, `Result` en `Core/Errors/`).

**Editor (fase E).**
- ¿Lo que he hecho está en `motor_editor`, o solo en el editor antiguo
  `level_editor`? Son dos ejecutables y el bueno es el primero.
- ¿El panel nuevo se abre, se ve y se cierra sin romper el viewport?
- ¿Las acciones pasan por `EditorActions` (deshacer/rehacer gratis) o escriben
  el documento a mano (y entonces no se pueden deshacer)?

**Juego (fases F, G, H).**
- ¿La pantalla nueva entra por el manifiesto del proyecto o por una lista fija?
- ¿Se puede **salir** de donde he entrado? (G-10: «Reanudar» no salía de Pause.)
- ¿El guardado antiguo sigue cargando? Romper `savegame.json` es romper las
  partidas de Oriol.

**Contenido / assets.**
- `python3 tools/validar_todo.py` → 0 errores nuevos (los 14 avisos conocidos
  siguen ahí; no se usan de excusa).
- `python3 tools/conectividad.py` y `tools/validar_enlaces.py` en verde: una
  puerta sin vuelta encierra al jugador.
- ¿Los ids son estables y no se han renombrado en silencio? (`tools/renombrar_id.py`
  con migración, nunca a mano.)
- ¿Respeto el canon? `canon → cartas → motor`, sin aristas de vuelta. Si el
  motor necesita un dato que no existe aguas arriba, es un bug de contenido y se
  registra, no se inventa en el motor.

**Herramientas (`tools/`).**
- ¿No existe ya una que haga esto? →
  `python3 tools/analizar_duplicados.py --fichero tools/<nuevo>.py`
  y mirar también la tabla de *casi solapamientos* del informe.
- ¿Tiene `--comprobar` o prueba propia sembrada (casos plantados + negativos)?
  Un validador con la lista escrita a mano deja de validar (Parte 3).
- ¿Rutas relativas a `Path(__file__).resolve()`, nunca `/Users/admin/...`?
- ¿`ensure_ascii=False` si escribe JSON?
- ¿Está registrada en CTest/CI? Si no, se dice explícitamente en la sesión:
  hoy `validar_todo.py` no lo está y por eso sobreviven errores de contenido.

**Documentación.**
- ¿Cita ficheros que existen? `analizar_duplicados.py --solo DOC_DESACTUALIZADO`
  lo comprueba: hoy hay 25 docs cuyo código citado cambió después.
- Si no se va a mantener, ¿lo dice en la cabecera (CONGELADA / histórico)? Es lo
  que hizo la Parte 0 de la bitácora, y por eso no miente.

---

## Lo que hay que dejar escrito al cerrar

En `planificacion/seguimiento/<PAQUETE>.json`, además de lo que ya se escribe:

```json
{
  "estado": "closed",
  "alcance": {
    "ejecutable_que_lo_usa": "motor_editor",
    "ruta_de_usuario": "Abrir proyecto → árbol → botón derecho → Nuevo mapa",
    "medido_con": "tools/medir_alcance.py (HEAD <sha>)",
    "llega_al_producto": true
  },
  "cierre": {
    "bloque_0": ["0.1 ok", "0.2 ok", "0.3 ok en clon limpio <sha>", "0.4 fila del 17/09"],
    "prueba_que_falla_al_deshacer": "test_x <sha>",
    "duplicados_comprobados": "analizar_duplicados --fichero ... → SIN_PAREJA_CONOCIDA",
    "probado_por_una_persona_nueva": "quién y cuándo, o NO"
  },
  "deuda_que_dejo": ["lo que no he hecho y queda apuntado en TABLERO"]
}
```

Las tres claves nuevas son `alcance`, `cierre` y `deuda_que_dejo`. Si
`alcance.llega_al_producto` es `false`, el estado no puede ser `closed`: se usa
`integrado_parcial` o `abierto`, y se añade la fila correspondiente en
`TABLERO.md` «Pendiente».

**Firma.** Cerrar es afirmar que otra persona puede usarlo. Si no se ha
comprobado, se escribe «probado por mí, no por nadie más» — que es honesto y
sirve; «closed» a secas no.
