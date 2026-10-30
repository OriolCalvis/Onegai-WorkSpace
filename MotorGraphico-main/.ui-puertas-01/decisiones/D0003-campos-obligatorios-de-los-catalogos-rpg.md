# D0003 — Qué campos de un catálogo RPG son obligatorios, y quién los sube

- **Estado:** `proposed` — espera a Oriol.
- **Fecha:** 2026-09-07 · **Autor:** qwen (sesión D-01).
- **Alcance:** el contrato de carga de los 20 catálogos tipados de
  `MotorGraphico-main/MotorGraphico/include/RPG/Catalogs/RpgCatalogs.h` y el
  orden en que D-02..D-10 los migran. No toca canon ni reglamento.
- **Fuente operativa:** `planificacion/seguimiento/D-01.json`; el código lleva
  el contrato en `CatalogField` / `CatalogStrictness`.

## Contexto

Hasta el 07/09 `Catalog<T>::loadFromString` envolvía cada entrada en un
`catch(...)` mudo, descartaba con `continue` las que no dieran un id y contaba
como buena cualquier otra. Cada `CatalogLoader<T>` leía sus claves con
`asInt(default)` / `asString(default)`, así que un nombre de campo cambiado
producía defaults en vez de un error: **un cargador roto era indistinguible de
uno que funciona**. Eso es lo que el plan llama la raíz de la Fase D.

Medido sobre el código y los assets publicados el 07/09/2026:

| Dato | Valor |
|---|---|
| Cargadores especializados | 20 |
| Campos declarados (alias camelCase/snake_case fusionados) | 308 |
| Campos que **ninguna** entrada de ningún asset lleva | 150 |
| Entradas en los 21 assets de `assets/catalogs/` | 2650 |
| Entradas con `id` y con `name` | 2650 (100%) |
| Entradas que incumplirían exigir todos los campos declarados | 2650 (100%) |

El caso que cita el plan (D-04) es real y no es el peor: `conditions.json`
lleva `stackable`/`duration`/`effects` y el cargador lee
`stacking`/`defaultRounds`/`damagePerRound`. Las 16 condiciones entraban
idénticas. `NpcDefinition` lee 26 campos y el asset lleva 4.

## Decisión propuesta

1. **Modo de producción = estructural.** Solo fallan la carga los campos
   marcados `required`. Hoy son `id` y `name` en los 20 cargadores (40 campos),
   porque son los únicos que los assets llevan en el 100% de las entradas.
2. **Nadie sube un `required` sin arreglar su asset en el mismo cambio.**
   Subirlo solo rompe la carga del juego: los 20 catálogos dejarían de entrar y
   con ellos `demo_catalogos_rpg` y cinco demos más. Cada paquete D-02..D-09 es
   propietario de su catálogo y entrega las tres cosas juntas: asset corregido,
   `required` subido a los campos que llevan sus datos, y prueba de contenido
   (la que el plan ya pide en cada paquete).
3. **Exigir todos los campos declarados (`AllDeclared`) es un instrumento de
   auditoría, no un estado al que se aspire.** La mayoría de campos declarados
   son legítimamente opcionales: `channelRounds` es 0 en casi todas las
   habilidades y `minGold` no aplica a una tabla de botín sin oro. Sirve para
   medir el hueco de un catálogo entero de una vez, que es como salió la tabla
   de arriba.
4. **Una carga fallida es todo-o-nada.** Una entrada inválida no deja el
   catálogo a medias: se parsea a un buffer local antes de tocar el almacén,
   el mismo criterio que ya usa `ObjectCatalog::loadFromString`.
5. **Ninguna excepción del cargador se traga.** Se convierte en un error que
   nombra la entrada (índice e id) y el motivo.

## La discrepancia que no resuelvo en silencio

Los nombres canónicos que declaro salen del cargador C++, pero los assets
vienen de dndWeebCC por los conversores. Cuando cargador y asset no están de
acuerdo, **la Parte 2.1 dice que gana la fuente del dato** (`canon → cartas →
motor`, sin aristas de vuelta), y el criterio de D-04 está redactado en esa
dirección: «apilamiento, duración y efectos por ronda **se leen**».

Eso tiene una consecuencia que necesita decisión explícita:

- ¿El nombre canónico de un campo lo fija el JSON de dndWeebCC (y entonces el
  cargador C++ se adapta, y `expected_fields()` debe renombrar su canónico a lo
  que lleva el asset), o lo fija el C++ (y entonces hay que reescribir los
  assets, lo que sería una arista de vuelta)?
- Si gana el JSON: ¿los 150 campos que ninguna entrada lleva son campos que el
  C++ se inventó durante la conversión y hay que **retirar** de la declaración,
  o campos que el contenido todavía no tiene y hay que pedir aguas arriba como
  bug de contenido?

D-01 no decide nada de esto: declara lo que el cargador lee hoy, que es lo que
permite medir el hueco. La respuesta cambia el trabajo de D-02..D-09.

## Alternativas descartadas

- **Exigir ya todos los campos declarados.** Rompe la carga de los 2650
  registros: el juego no arranca y la suite gl-free cae. Es justo el trabajo de
  D-02..D-09, no el de D-01.
- **Avisos en vez de errores (sin modo de auditoría).** El hueco seguiría sin
  ser visible en CI y la prueba que pide el plan —«un JSON con los nombres de
  campo de la versión anterior falla con un mensaje que dice qué campo falta»—
  no sería ejecutable sin romper los assets.
- **`required` por catálogo sin modo de auditoría.** No hay forma de medir los
  150 campos de una vez, que es la evidencia con la que se prioriza la Fase D.
