# Plan de implementación web — Constructor de Aventuras y Mazo por Actos (GDD §20)

*Aterriza en el código real de la app (Spring Boot, `cat.dnd.cc`) el sistema de mazos de
historia por actos del GDD (`Sistema_Cartas_Tiers.md §20`), sus prompts de generación
(`Plantilla_Prompt_Contenido.md §18`) y los wireframes T3/T4 de `Wireframes ONEGAI.dc.html`
(pantallas 10a/10b jugar · 11–13 construir). No inventa arquitectura nueva: extiende el CRUD
de Aventura que ya existe, siguiendo el patrón Modelo→Form→Repositorio→Servicio→Controlador→
plantillas que usan las 17 familias de cartas.*

---

## 0. Punto de partida (lo que ya existe)

El dominio Aventura está construido y en producción, pero es **plano**:

| Capa | Fichero actual | Qué hace hoy |
|---|---|---|
| Modelo | `model/Aventura.java` | `id`, `nom`, `descripcion`, `historiaIds` (lista plana), `npcIds`/`deidadIds`/`villanoIds`/`enemigoIds`/`lootIds`/`trampaIds`, `List<Combate>`, `notas` |
| Repositorio | `repository/AventuraRepository.java` | Un JSON por aventura en `data/aventuras/{id}.json`, id numérico, mapa en memoria (patrón `PersonatgeRepository`) |
| Servicio | `service/AventuraService.java` | CRUD + `historiasDe()` (resuelve y ordena por facción→trama→escalón) |
| Controlador | `controller/AventuraController.java` | `/aventuras` listar·crear·detalle·editar·eliminar + `/{id}/imprimir` (9 cartas/A4 vía `CartaImpresion`) |
| Catálogo | `repository/CatalogoAventuraRepository.java` | Índice ligero (id·nombre·tier·rango·facción) para pickers |
| Plantillas | `templates/aventuras/{llista,formulari,detall,imprimir}.html` | Galería, formulario, detalle, hoja de impresión |
| Datos | `data/aventuras/1..52.json` | 52 aventuras, todas `historiaIds` planas (ej: `1.json` = 10 ids sin estructura) |

**El hueco exacto que marcan los wireframes** (notas ⚠ de 10a, 10b, 12a, 13): no hay `acto`,
`cardKind`, requisito de entrada (ficha Verde/Roja), ramas de inyección ni texto de Balance
Final. Ese es el eslabón que este plan construye primero; todo lo demás cuelga de él.

**Principio rector de compatibilidad:** las 52 aventuras existentes deben seguir cargando y
la pantalla `llista`/`detall`/`imprimir` deben seguir funcionando sin tocar sus JSON. La
estructura de actos es **aditiva y opcional**: una aventura sin actos es una aventura legacy
válida.

---

## Fase 0 — Modelo de datos (el eslabón que falta)

Objetivo: que una Aventura pueda contener tres mazos de **Cartas de Historia** con toda la
mecánica de §20, sin romper las planas. Alinea con `Arquitectura_Datos_Onegai §5.15` (que hoy
define `adventure.storyDeck` como lista plana de `story_id` — insuficiente) y con el esquema de
carta de `Plantilla_Prompt_Contenido.md §18b`.

### 0.1 Nuevo modelo `model/CartaAventura.java`

Espeja 1:1 el esquema del prompt §18b para que lo generado por IA se deserialice directo.

```java
public class CartaAventura {
    private String code;                 // "A1-03"  (acto+numero, único dentro de la aventura)
    private int acto;                    // 1 | 2 | 3
    private String titulo;
    private CardKind cardKind;           // BASE | CONDICIONAL | INYECTADA | CADENA | BALANCE_FINAL
    private Cadena cadena;               // {orden, total}  solo si CADENA (🔗), si no null
    private Requisito activacion;        // {requiereCode, estado}  solo CONDICIONAL, máx 1
    private String escena;               // texto que lee/parafrasea el director (cara jugador)
    private List<Rama> ramas;            // {cuandoCode, cuandoEstado, texto} ≥2 en INYECTADA
    private String ganchoIgnorar;        // línea "Si ignoran esto…" (copia del Director)
    private Referencias referencias;     // npcIds, enemigoIds, locationId, lootTableId, storyId
    private List<FilaBalance> balance;   // {condicion, epilogo} solo BALANCE_FINAL
    // getters/setters + inner classes Cadena, Requisito, Rama, Referencias, FilaBalance
}
public enum CardKind { BASE, CONDICIONAL, INYECTADA, CADENA, BALANCE_FINAL }
public enum FichaEstado { VERDE, ROJA }   // Requisito.estado y Rama.cuandoEstado
```

Notas de diseño: `CardKind.INYECTADA` es `BASE` a efectos de filtro (entra siempre) pero lleva
`ramas`; el validador (Fase 2) lo comprueba. `Requisito` y `Rama.cuandoCode` referencian el
`code` de otra carta de un acto **anterior** — nunca del mismo acto ni posterior.

### 0.2 Extensión de `model/Aventura.java` (aditiva)

```java
// nuevos campos, todos con default que preserva las aventuras planas
private List<CartaAventura> cartasHistoria = new ArrayList<>();  // vacío = aventura legacy
private String tema;                     // para la galería (Fantasía/Misterio…)
private String estado = "borrador";      // borrador | en_curso | completa  (chips de 11a/11b)
private String arquitectura;             // espina_de_pescado | facciones | crawler (§20.5)
```

Jackson ignora campos ausentes, así que los 52 JSON planos cargan con `cartasHistoria` vacío.
Métodos de conveniencia en el modelo: `cartasDeActo(int)`, `esPorActos()` (`!cartasHistoria.
isEmpty()`), `porcentajeBase(int acto)` — los consumen las plantillas y el validador.

### 0.3 Persistencia

`AventuraRepository` **no cambia**: sigue serializando el objeto entero a `data/aventuras/
{id}.json`. Los nuevos campos aparecen solos en el JSON al guardar una aventura por actos. No
hace falta carpeta `cartas/` por aventura (el prompt §18b la sugería para generación externa;
al importar, se colapsan en el array `cartasHistoria`). **Regla de compatibilidad histórica**
de `Arquitectura §schema`: nunca borrar un id publicado; una carta obsoleta se marca y no se
elimina.

### 0.4 Entregable de la fase

Modelo compilando + las 52 aventuras cargan + un test `AventuraModelTest` que verifica: (a) un
JSON plano legacy deserializa con `cartasHistoria` vacío; (b) un JSON con actos deserializa las
cartas y sus `cardKind`. Sin UI todavía.

---

## Fase 1 — Constructor: editor de aventura (wireframes 11–12)

La portada (pantalla 11) **ya existe** como `templates/aventuras/llista.html`; solo se enriquece
con los chips de progreso/estado/tema. El trabajo real es el editor de la pantalla 12.

### 1.1 Form `web/form/CartaAventuraForm.java`

Siguiendo el patrón de `TierSkillForm` etc.: un DTO plano que Thymeleaf bindea con `th:field`
y el servicio convierte a `CartaAventura`. Campos: `code`, `acto`, `titulo`, `cardKind`,
`cadenaOrden`/`cadenaTotal`, `requiereCode`, `requiereEstado`, `escena`, `ganchoIgnorar`,
`ramasCsv` (o lista), y los `*IdsCsv` de referencias — mismo estilo CSV que ya usa
`Aventura.Combate.enemigoIdsCsv`.

### 1.2 Servicio `service/ConstructorAventuraService.java`

Nuevo servicio (deja `AventuraService` para el CRUD base). Responsabilidades:

- `añadirCarta(Long aventuraId, CartaAventuraForm)` / `editarCarta` / `eliminarCarta(code)` /
  `moverDeActo(code, actoDestino)` (el drag entre columnas de 12b).
- `siguienteCodigo(aventura, acto)` → genera `A1-04`, `B2-06`… sin colisión.
- `pickers()` → reusa `CatalogoAventuraRepository` (npcs, enemigos, loot) + `HistoriaService`
  para el desplegable de requisitos "carta de acto anterior" (12a nota ⚡).
- Cada operación termina en `aventuraService.guardar(aventura)` (persistencia ya resuelta).

### 1.3 Controlador — ampliar `AventuraController`

Añadir bajo `/aventuras/{id}`:

```
GET  /aventuras/{id}/constructor          → editor 3 paneles (12a) / kanban (12b)
POST /aventuras/{id}/cartas               → añadir carta (form)
POST /aventuras/{id}/cartas/{code}/editar → guardar carta
POST /aventuras/{id}/cartas/{code}/mover  → cambiar de acto (drag)
POST /aventuras/{id}/cartas/{code}/borrar
```

Mantiene la convención existente (`@ModelAttribute`, `redirect:`, `model.addAttribute
("currentPage","aventuras")`).

### 1.4 Plantillas

- `templates/aventuras/constructor.html` — layout de la pantalla 12: columna de Actos con
  contador en vivo `n / mín. 6` (verde/rojo según umbral §20.4), tablero central de cartas con
  icono por `cardKind` (🔵 Base · ⚡ Condicional · 🔗 Cadena), y editor de carta a la derecha
  (12a fijo) o modal (12b). El semáforo por carta (dependencia sana/rota) se calcula en
  servidor y se pinta como borde, sin JS pesado.
- Enriquecer `llista.html` con los chips de `estado`/`tema` y la barra de progreso `cartas
  definidas / mínimo` (11a/11b). El botón "＋ nueva aventura" ya existe.

### 1.5 Entregable

Se puede crear una aventura por actos entera desde la web, moverla entre columnas y guardarla;
recargar la app la reconstruye desde disco. Aún sin validación estricta ni exportación.

---

## Fase 2 — Validación y exportación (wireframe 13)

### 2.1 Servicio `service/ValidadorAventuraService.java`

Implementa en Java las reglas duras del prompt §18c y del GDD §20.4. Devuelve
`List<Incidencia>` con `{code, regla, gravedad(ERROR|AVISO), mensaje}`:

| Regla | Chequeo | Fuente |
|---|---|---|
| Filtro sano | todo `activacion.requiereCode` apunta a una carta de acto **anterior** que existe | §20.3 · 13b "A1-09 no existe" |
| Efecto mariposa | toda carta ignorable tiene consecuencia Verde y Roja reales (carta filtrada o rama) | §20.4.2 |
| Válvula de escape | `porcentajeBase(acto) ≥ 40%` en cada acto; simular peor caso (todo Rojas) → ≥3 cartas sobreviven | §20.4.4 · 13b "Acto III sin Base" |
| Cadena | numeraciones `orden/total` completas y sin huecos; ninguna 🔗 con `activacion` | §20.3 |
| Balance Final | exactamente 1; condiciones contables | §20.3 |
| Referencias | todo id de `referencias` existe en catálogo | §18c.6 |

Se llama al abrir la pantalla 13 y (opcional) antes de exportar. El grafo/sandbox de 13a
(marcar `A1-03=Roja` y ver qué cartas del Acto II se apagan/encienden) es el mismo motor de
filtrado de la Fase 3 llamado en modo simulación — se comparte, no se duplica.

### 2.2 Exportación

- **JSON para jugar**: la propia serialización del repositorio ya lo da; endpoint `GET
  /aventuras/{id}/exportar.json` que descarga el `data/aventuras/{id}.json`. Reutilizable por
  el modo Partida (Fase 3) y por herramientas externas.
- **PDF de doble cara** (13b: cara jugador `escena` vs. copia del Director con `ramas`/
  `ganchoIgnorar` en rojo): **extender `imprimir`** que ya existe. Añadir a `CartaImpresion`
  un mapeo `mapCartaHistoria(CartaAventura)` y una segunda cara "Director". El troceado 9/A4 y
  el `PdfService` ya están resueltos; solo se añade el tipo y la variante de cara.

### 2.3 Controlador y plantilla

```
GET /aventuras/{id}/dependencias   → grafo + sandbox (13a) y panel de validación (13b)
GET /aventuras/{id}/exportar.json
GET /aventuras/{id}/imprimir?cara=jugador|director   (amplía el imprimir actual)
```

Plantilla `templates/aventuras/dependencias.html`. El grafo puede empezar como las 3 columnas
de texto de 13a (barato) y subir a líneas SVG reales en una iteración posterior.

---

## Fase 3 — Mazo por Actos en Partida (wireframes 10a/10b)

Es el modo **jugar**, no diseñar: robar carta, marcar 🟢/🔴, filtrar el acto siguiente en vivo,
resolver cartas inyectadas y Balance Final. Requiere **estado transitorio de sesión** — igual
que la "sesión de combate en vivo" ya prevista en `Arquitectura_Datos_Onegai` como entidad
separada de las definiciones.

### 3.1 Estado de sesión `model/SesionAventura.java` (transitorio)

No se persiste como definición; vive mientras dura la partida (memoria + snapshot opcional en
`data/sesiones/{id}.json` para "seguir jugando" del dashboard 8a). Guarda: `aventuraId`,
`actoActual`, `mazoActual` (codes barajados), `fichas` (`Map<code, FichaEstado>` de lo ya
resuelto), `descartadas` (Historia Perdida).

### 3.2 Motor `service/FiltroActoService.java` (el corazón)

Función pura, testeable, reutilizada por el sandbox de 13a y por el juego real:

```java
ResultadoFiltro filtrar(Aventura av, int acto, Map<String,FichaEstado> fichasPrevias)
// → {entran: List<CartaAventura>, descartadas: List<CartaAventura>}
```

Reglas §20.3: Base entra siempre; Condicional entra si su `activacion` se cumple contra
`fichasPrevias` (Acto II solo mira fichas del Acto I; Acto III mira I+II); Cadena entra en orden
fijo encima del mazo, el resto barajado debajo. `evaluarInyeccion(carta, fichas)` elige la
`Rama` activa al robarla. `resolverBalance(cartaBalance, todasLasFichas)` cuenta y devuelve el
epílogo (§20.3).

### 3.3 Controlador `controller/PartidaAventuraController.java`

```
POST /partida/aventura/{id}/empezar       → crea SesionAventura, baraja Acto I
GET  /partida/sesion/{sid}                 → mesa del Director (10a) / mesa compartida (10b)
POST /partida/sesion/{sid}/robar
POST /partida/sesion/{sid}/marcar          → {code, VERDE|ROJA} → guarda ficha
POST /partida/sesion/{sid}/siguiente-acto  → filtra + baraja (transición automática de 10b)
```

Al marcar, la vista muestra las dos ramas del efecto mariposa **antes** de decidir (10a nota
⚡). La transición de acto corre el `FiltroActoService` y enseña "6 de 12 válidas, ≥40% Base"
(10b). La rama no elegida se ve tachada, no oculta (10b).

### 3.4 Plantillas

`templates/partida/mesa-director.html` (10a) y `mesa-compartida.html` (10b). Se enlazan desde
Campañas (`detall` "▶ lanzar en partida") y desde el dashboard "▶ continuar" (8a).

---

## Orden de entrega y por qué

1. **Fase 0** (modelo) — nada funciona sin esto; es barato y de bajo riesgo (aditivo).
2. **Fase 1** (constructor) — permite *crear* contenido por actos; sin él no hay nada que jugar
   ni validar.
3. **Fase 2** (validación/export) — hace el contenido fiable y ya deja **imprimir cartas
   físicas** (cierra el bucle con el §20 físico aunque la Fase 3 no exista).
4. **Fase 3** (partida por actos) — la más cara (estado transitorio, barajado, tiempo real);
   se puede aplazar porque con Fases 0–2 ya se diseña, valida e imprime para jugar en mesa
   real.

Cada fase replica el patrón que la app ya usa 17 veces, así que el riesgo técnico es bajo; el
riesgo está en el **alcance de la Fase 3** (sesión en vivo), no en la mecánica.

## Checklist de arranque (Fase 0)

- [ ] `model/CartaAventura.java` + enums `CardKind`, `FichaEstado`
- [ ] Campos aditivos en `model/Aventura.java` (+ `cartasDeActo`, `esPorActos`, `porcentajeBase`)
- [ ] Test: 52 JSON legacy siguen cargando; un JSON con actos deserializa
- [ ] Actualizar `Arquitectura_Datos_Onegai §5.15` para que `adventure` refleje el mazo por
      actos (hoy describe `storyDeck` plano) y apunte a este plan
- [ ] Confirmar que `llista`/`detall`/`imprimir` no se rompen con el modelo ampliado

---

*Referencias: `Sistema_Cartas_Tiers.md §20` (reglas) · `Plantilla_Prompt_Contenido.md §18`
(generación) · `Arquitectura_Datos_Onegai.md §5.15` (esquema a actualizar) · `Wireframes
ONEGAI.dc.html` T3 (10a/10b) y T4 (11–13) · patrón CRUD: dominio `Aventura` existente.*
