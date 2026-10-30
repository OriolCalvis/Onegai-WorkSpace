# D0009 — La tipografía del HUD no tiene dónde vivir su prueba: `BitmapFont` está atrapada en `motor_level`

- **Estado:** `proposed` — **espera a Oriol.** No se ha tocado nada fuera del carril
  propio: el arreglo de AV1-03 está commiteado y verificado, pero su prueba
  permanente no se puede escribir sin partir una librería que hoy tiene 14
  paquetes.
- **Fecha:** 2026-09-29 · **Autor:** opencode (sesión AV1-03).
- **Alcance:** la frontera GL de `src/Render/BitmapFont.cpp` y, con ella, la
  prueba de `HudCommandMenu`. **No cambia el plan AV0…AV12**: AV1-03 se cierra a
  medias por esta razón, y está dicho abajo por qué.

## Contexto

AV1-03 («tipografía UTF-8 limpia y wrapWidth automático») se cerró el 29/09
parcialmente: el menú de comandos ya se envuelve dentro de su caja y de su
pantalla (`5829895`), con 8 casos y 6 mutaciones de falsabilidad, pero **sin
prueba versionada**.

El motivo es una colisión entre dos leyes del repo que hoy no se pueden cumplir a
la vez:

1. **Ley 4c** — «ninguna fuente de `src/` compilada suelta dentro de un
   objetivo», y `linea_base_core.py` cuenta `fuentes_directas` con la regla de que
   **la línea base solo baja**. Un binario de prueba para `HudCommandMenu`
   necesitaría compilar `src/Render/BitmapFont.cpp` porque
   `BitmapFont::wrapText` y `measureText` son símbolos definidos ahí, no en la
   cabecera.
2. **Ley 2** — «ninguna librería mezcla fuentes que hablan con OpenGL con fuentes
   que no».

`BitmapFont.cpp` cumple las dos a la vez: sus métodos estáticos son **puros y
GL-free** (es lo que hace que hoy sea testeable sin contexto), pero el mismo
fichero contiene `generateAtlas()`, que hace `glGenTextures` de verdad. Y está
dentro de `motor_level`, que **también contiene `Application.cpp`** — 14 paquetes,
con filas vivas de H1-03/H1-05, H2-01 y H4-01. Enlazar `motor_level` para probar un
menú significa arrastrar `Application.cpp` al binario, que es justo lo que
`G14HudTimersAndCommerceTests.cmake` documenta como «estrategia anticolisión» y lo
que hoy son las 67 `fuentes_directas`.

## Opciones

| # | Opción | Coste | Por qué no se ha hecho sola |
|---|---|---|---|
| 1 | **Partir `BitmapFont.cpp`**: los estáticos puros a un `BitmapFontLayout.{h,cpp}` GL-free en biblioteca propia; `generateAtlas`/`drawText` se quedan con GL. | Bajo (es una traslación, no una reescritura) y **baja `fuentes_directas`**. Rompe el fichero de otro carril: el bloque generado lo marca G-10/G-14. | Es la que resuelve de verdad, pero es un cambio estructural en un fichero con trabajo ajeno en vuelo. Necesita dueño. |
| 2 | Subir `fuentes_directas` con un `add_executable` y justificándolo en el mensaje del commit. | Nulo, pero **deuda nueva** y la guardia veta el cierre. | La ley no admite atajo. |
| 3 | Probar la tipografía por un binario de ventana. | Nulo. | La prueba necesita una ventana en un repo sin ventanas: pierde todo su valor. |
| 4 | Dejar la pieza sin prueba versionada y registrarla aquí. | Nulo hoy; deuda que se paga sola. | **Es lo que se ha hecho**, como puente, no como solución. |

## Decisión que se pide a Oriol

**Adoptar la opción 1, y con ella dos cosas de AV1-03 que hoy están a medias:**

- **La prueba permanente de AV1-03.** En cuanto `wrapText`/`measureText` vivan en
  una biblioteca GL-free, `HudCommandLayoutTests` entra sin tocar `CMakeLists.txt`
  con fuentes sueltas y con `fuentes_directas` **bajando**. El harness ya está
  escrito y validado; solo hay que cambiar dónde vive la función que llama.
- **`HudText` sin techo de alto.** La mitad sin hacer de AV1-03 no es un problema
  de diseño: `HudText` recibe `size = {100, 20}` por defecto y varios *call sites*
  lo usan así (**`m_businessText`, `m_shopTitle`, `m_moralText`**,
  `Application.cpp:612-656`) confiando en que la altura se ignore. Si se limitara
  el alto sin tocar esos *call sites*, la ficha de texto del negocio passaría de
  varias líneas a una y se **perdería contenido visible** — una regresión. Como
  `Application.*` está reservado, el arreglo correcto necesita esas cajas reales y
  va en la tarea que toque ese carril, no en esta.

## Nota sobre la falsabilidad de lo que sí está hecho

De las 6 mutaciones aplicadas a `HudCommandMenu`, **5 se cazan**. La sexta —
revertir `render()` a una fila por opción sin tocar `usableWidth`/`layoutRows`/
`contentSize`— **no la caza ninguna prueba**, porque `SpriteBatch` no expone su
buffer de vértices y no hay forma de observar lo que un `render()` encola sin
abrir contexto. Está anotado en el propio `render()`: la lógica vive en
`layoutRows()` y el bucle de dibujado es su transcripción mecánica. Cerrar ese
hueco requiere o un accesor de prueba en `SpriteBatch` o mover el bucle a una
función pura; ninguna de las dos se ha hecho aquí.
