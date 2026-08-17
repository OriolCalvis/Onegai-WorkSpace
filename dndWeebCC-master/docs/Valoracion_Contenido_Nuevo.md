# Valoración del contenido nuevo del catálogo

*Revisión de las 26 clases, 13 piezas de equipo, 38 razas, condiciones, dotes, deidades,
invocaciones, rasgos y personajes de `data/cartas` y `data/personatges`, contrastada con el GDD
edición 2 (`Sistema_Cartas_Tiers.md`) y el código Java actual.*

---

## 1. Inventario de lo nuevo

| Categoría | Cantidad | Estado general |
|---|---:|---|
| Clases (`clases/`) | 26 | Esquema técnico correcto; contenido incompleto en 17-24 de ellas |
| Equipo (`armas/`) | 13 | Bien formado, el mejor alineado con el esquema de la app |
| Razas (`razas/`) | 38 | Lore muy rico; bonos fuera del rango que fija el GDD |
| Condiciones (`condiciones/`) | 1 (+ tipo Java completo) | **Novedad excelente** — no existía ni en el GDD |
| Kit "Ancla del Vacío" | 1 clase + 1 pasiva + 1 condición + 7 habilidades + 1 rasgo | **El estándar de oro del catálogo** |
| Dotes, deidad, consumible, invocación, rasgos | 6+1+1+1+2 | Siguen bien los esquemas del GDD |
| Personajes (`personatges/`) | 2 | Datos de prueba, no contenido real (nombres tipo "sdfsdfsd", historia vacía) |
| Enemigos | 0 | **No existe carpeta ni modelo**: el Bloque G del plan sigue pendiente |

---

## 2. Valoración por categoría

### 2.1 El kit del Ancla del Vacío — la referencia a seguir

Es exactamente lo que la sección 10 del GDD pide ("una clase es un paquete de mecánicas, no una
lista de bonificadores"): una identidad mecánica (no moverse = poder), expresada como **pasiva de
clase** (`anclaje`) + **carta de condición** (`anclado`, con `mechanicHook` legibles por máquina:
`velocidad_0`, `bono_ca:2`) + **7 habilidades** escalonadas de tier 1 a 5 con rarezas y `recovery`
correctos + un **rasgo de tier 4** que respeta explícitamente la regla de pasiva única (9.4).
Las habilidades usan `requiredTags: ["anclado"]`, lo que convierte la condición en requisito
jugable — un patrón nuevo y potente que el GDD aún no documenta.

**Cómo nos ayuda:** es la plantilla viva para regenerar las otras 25 clases. Además, el tipo
Condición generaliza la tabla fija de la sección 7.4: las 10 condiciones del manual (Sangrado,
Caído, Agarrado...) pueden migrarse a cartas con `mechanicHook`, y el futuro motor de combate
(Bloque D) las consumiría directamente en vez de tener la tabla codificada a mano.

### 2.2 Clases (26) — esqueleto correcto, relleno pendiente

Lo bueno: roles variados, vidas base coherentes por rol (tanque 16/×4, equilibrado 12/×3, ágil
10/×2, mago 8/×1.5), nombres con muchísima personalidad, y un campo nuevo útil que el GDD no
tiene: **`learnableSkills`** (el pool de cartas aprendibles de la clase, separado de las
iniciales). Vale la pena adoptarlo oficialmente en el esquema 9.1.

Lo que falta, con números:

- **17/26 clases no tienen pasiva de clase** — la regla 9.4 la exige, y es la identidad de la clase.
- **24/26 tienen menos de 3 habilidades iniciales** (el estándar de Tier 1 es 3).
- **26/26 sin especializaciones** (la progresión de Tier 2+ queda vacía).
- **291 referencias rotas**: `startingEquipment` con texto libre ("Armadura: Ligeras", "Arma:
  Espadas largas") en vez de ids de cartas de equipo, y `learnableSkills` con nombres en prosa
  ("Cántico de Maldición") de habilidades que no existen como carta. Solo el Ancla del Vacío
  referencia ids reales.
- **Concentración de stat:** 17/26 clases tienen CAR como stat principal. Con CAR alimentando
  vida y Defensa mental, esto empuja a todo el catálogo hacia el mismo perfil — conviene
  redistribuir (el GDD dedica la sección 4 entera a evitar exactamente esto).

**Adaptación:** fácil pero laboriosa. El esqueleto JSON ya es el de la app (`TierClass`); es
rellenar pasiva + 3 habilidades como cartas reales por clase, al estilo Ancla del Vacío. Son
~25 × (1 pasiva + 2-3 habilidades) ≈ 80 cartas — perfectamente generable por tandas con el
modelo de prompt adjunto.

### 2.3 Equipo (13) — el más sólido

Esquema impecable contra `TierEquipment`: `statBonuses`/`penalties` anidados, `weightCategory`,
`requiredStats`, `linkedSkill` (el mandoble enlaza `golpe_doble` — patrón "pieza con habilidad
propia" del GDD 9.7 bien aplicado), restricciones textuales. Cubre bien torso y armas.

Mejoras menores: no hay ninguna pieza para `piernas` ni `pies` (2 de los 6 slots vacíos en todo
el catálogo); el `anillo_de_vitalidad` en slot `cabeza` es un apaño (¿slot `accesorio` futuro o
renombrar a diadema?); faltan `compatibilitySymbols` y `precisionStat` que el GDD edición 2
declara para armas (la app tampoco los tiene aún — van juntos, tarea 27 del plan).

### 2.4 Razas (38) — un mundo entero, pero sobredimensionado mecánicamente

El lore es la joya: 19 pueblos con dos variantes cada uno (aarakocra viento/montaña, drow
explorador/casa noble, yokai, nagas, minotauros, sagas...), con cultura, religión, edad y tamaño.
Esto es un *setting* completo, no una lista de razas.

Desajustes mecánicos:

- **Bono total medio de +4** (hasta +5), cuando el GDD 9.2 fija "bono menor, ej. +1 — nunca el
  rasgo principal". Con stats de creación en rango 1-6, un +3/+4 de raza domina la construcción.
- **38/38 sin rasgo activo** (`activeTrait` a null) — el esquema lo pide como opcional, pero su
  ausencia total hace a las razas pasivas.
- Texto mezclado catalán/castellano y metadatos (edad, tamaño, religión) embebidos en
  `flavorText` en vez de campos propios.

**Cómo nos ayuda:** además de como razas jugables (rebajando bonos a +1/+2 total), hay un grupo
que encaja mejor como **semilla del bestiario** (Bloque G): Revenants, Espectros, Traidores
Oscuros, Sagas de la Locura/del Abismo, Hechiceros Oscuros, Cazador Nocturno. Ya tienen lore de
facción enemiga; convertirlos en `Criatura`/`Elite` es más rápido que inventar un bestiario desde
cero.

### 2.5 Enemigos e historias — todavía no existen como tales

No hay carpeta `enemigos/` ni modelo Java (el árbol Enemigo→Criatura/Elite/Jefe del UML sigue
"propuesto"). Los dos personajes de `data/personatges` son pruebas de la interfaz (stats 1/1/1/1,
las mismas 10 habilidades copiadas, campo `historia` vacío) — útiles como test, no como contenido.
El campo `historia` en `Personatge` ya existe y está esperando: es el sitio natural para las
biografías cuando lleguen.

### 2.6 Detalles sueltos detectados

- `lobo_espectral.summonedBy` apunta a `spell_summon_spectral_wolf`, que no existe → crear ese
  hechizo o cambiar la referencia.
- `cicatriz_del_destino` (rasgo por Carga del Destino, GDD 7.5) y `pocion_de_vigor` (cura tier+4)
  están perfectos y ya conectados a reglas del manual — buen ejemplo de contenido "que cierra círculos".

---

## 3. El conflicto de fondo que hay que decidir: la fórmula de vida

Los 26 JSON de clase (y `CalculadoraVida.java`) usan **escalado por clase** (`healthScaling.CON`
de ×1.5 a ×4, sin CAR). El GDD edición 2 (sección 5) usa **multiplicador unificado**
(`CON×3 + CAR×1` para todas las clases; el rol se diferencia por vida base y equipo). Ambos
modelos son defendibles, pero hoy conviven y eso sí es un problema.

| Opción | Qué implica | Esfuerzo |
|---|---|---|
| **A. Adoptar la fórmula unificada del GDD** (recomendada) | Cambiar `CalculadoraVida` a `base + CON×3 + CAR×1 + tier + equipo`; el campo `healthScaling` de los 26 JSON pasa a legado ignorado. Las clases CAR ganan la vida que su diseño ya insinúa. | 1 método + tests (ya existen) + revisar vidas base |
| B. Adoptar el escalado por clase | Revertir la sección 5 del GDD (y los 5 personajes de ejemplo) al modelo por clase. | Reescribir GDD; los 26 JSON quedan como están |

Cualquiera de las dos es mejor que el estado actual. La app calcula hoy con B; el manual dice A.

---

## 4. Recomendaciones en orden

1. **Decidir la fórmula de vida** (§3) — bloquea el balance de todo lo demás.
2. **Adoptar oficialmente** `learnableSkills` (esquema 9.1) y el tipo **Condición** (nueva
   sección 9.13 del GDD), que son las dos mejores ideas del contenido nuevo.
3. **Completar las 25 clases** al estándar Ancla del Vacío por tandas de 5 (pasiva + 3 habilidades
   + ids reales en equipo), usando el modelo de prompt adjunto.
4. **Rebajar bonos raciales** a +1/+2 total y añadir un rasgo activo por raza; separar los
   metadatos del flavorText en campos.
5. **Reciclar las razas oscuras como bestiario** al arrancar el Bloque G.
6. Arreglar las referencias colgantes (`spell_summon_spectral_wolf`, equipo en texto libre).

---

*El modelo de prompt para generar contenido futuro con estos estándares está en
`docs/Plantilla_Prompt_Contenido.md`.*
