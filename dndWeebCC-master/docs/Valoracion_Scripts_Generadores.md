# Valoración de los 4 scripts generadores (onegai)

*Revisión de `convert_onegai_classes.py`, `generate_onegai_enemies.py`,
`generate_onegai_equipment.py` y `generate_onegai_side_missions.py` frente al GDD edición 2 y a
la app. Complementa a `Valoracion_Contenido_Nuevo.md` (que cubre el catálogo de `data/cartas`).
Los scripts aún no se han ejecutado en el repo: no existe `src/main/resources/JSONS/onegai/`.*

---

## Qué genera cada script

| Script | Salida | Volumen |
|---|---|---|
| `convert_onegai_classes.py` | Convierte las 26 clases legacy de `JSONS/classes` en definiciones de clase + pasiva + habilidades por tier + 2 cartas de estado | ~26 clases + ~300 habilidades |
| `generate_onegai_enemies.py` | 50 enemigos como **mini-mazos**: carta base + pasiva + 3 habilidades + IA por prioridades + fases (elite/jefe) + invocación eco + tabla de loot + carta de historia | 50 × 8 archivos |
| `generate_onegai_equipment.py` | 50 armas iniciales (25 bases × 2 prefijos) + 100 armas "de juego" (5 bases × 20 aspectos con rareza/habilidad) + 50 kits iniciales, con carta imprimible ASCII | 200 JSON |
| `generate_onegai_side_missions.py` | 50 misiones secundarias con PNJ, ubicación, decisión de 3 vías, complicaciones, consecuencias, cadenas de desbloqueo y estado de fallo, ligadas a "La Tomba del Rei Llop" | 50 JSON |

## Lo que encaja muy bien (conservar como patrón)

1. **Enemigos como mini-mazos** — es "todo es una carta" aplicado al bestiario y coincide con el
   árbol `Enemigo → Criatura/Elite/Jefe` + `FaseJefe` del UML. Resuelve de golpe el Bloque G del
   plan de cierre, con curva Tier 1-5, 12 roles y 25 mecánicas firmadas (curse, bleed, anchored...).
2. **Misiones con consecuencias** — gancho → decisión Proteger/Negociar/Romper → complicaciones →
   consecuencias luz/sombra → cadena de submisiones. El `failureState` ("la misión no desaparece:
   cambia de dueño, se encarece o se vuelve amenaza") es una regla de diseño excelente. Cubre el
   Bloque H.
3. **`printableCard` ASCII** — alinea con la exportación PDF (Bloque I) y la plantilla de 5 zonas.
4. **Generación determinista + index.json por dominio** — misma higiene de datos que ya usa el
   catálogo. Mantener.
5. **Escala de tiers correcta** — `level_to_tier` (1-4→1 … 17+→5) y dificultad
   MINION/STANDARD/ELITE/BOSS respetan la sección 3 del GDD.

## Desajustes con la edición 2 (adaptar antes de integrar)

| # | Desajuste | Dónde | Adaptación |
|---|---|---|---|
| 1 | Salvaciones **STR/WIS** (no existen FUE ni SAB) | `SAVE_MAP`; `savingThrow` de enemigos | Mapear a defensas: físico→`resistencia_fisica`, reflejo→`CA`, mental→`defensa_mental` |
| 2 | **Recursos por puntos** (`{"type":"RAGE","max":3}`) | `infer_resource` de clases | Eliminar; traducir `cooldown`→`recovery` (`SHORT_REST→descanso_corto`, `LONG_REST/DAILY→descanso_largo`, `NONE→activa/ninguno`, `LIMITED→descanso_corto`) |
| 3 | **Dados de daño d4-d12 por arma** (el GDD no da dado a las armas) | armas y habilidades de enemigo | Decisión de diseño pendiente (ver abajo). Mientras: dado al texto del efecto, no como campo mecánico |
| 4 | Slots **HANDS/WAIST/BACK/NECK/TOOL** | accesorios | Solo existen los 6 slots del GDD. Guantes/cinturón/mochila/amuleto → consumibles o rasgos sin slot |
| 5 | Rareza **LEGENDARY** y símbolos nuevos **◇ ⬟** | equipo | LEGENDARY→`mythic`; arco = △/○ + `[Proyectil]`, escudo = `arma_secundaria` + `[Escudo]` |
| 6 | Enums/tags **en inglés** (ACTION, SHADOW, BLEED...) | todos | Tabla de equivalencias única al español del catálogo |
| 7 | `startingSkills[:2]` | conversor de clases | Tier 1 = 1 pasiva + 3 habilidades (o el reparto declarado, p. ej. 2+3) |
| 8 | Vida de enemigos con fórmula propia (`mult×tier + CON×3`) | enemigos | Aceptable, pero hay que documentarla en el GDD (sección de bestiario) |
| 9 | Salida a `JSONS/onegai/` pero la app lee `data/cartas/` | los 4 scripts | Decidir destino único: script puente onegai→`data/cartas`, o repositorios que lean ambos |

**✅ Decisión tomada (dados de daño):** daño = **éxitos de la tirada de ataque × multiplicador
del arma** (△ ×1 · ○ ×1,5 · □ ×2; hechizos ×1,5), sin dados de daño separados. Regla escrita en
la **sección 7.6 del GDD**. Los dados legacy se mapean d4/d6→△ · d8→○ · d10/d12→□ y quedan solo
como texto de sabor.

## Veredicto

Material **muy aprovechable**: enemigos y misiones rellenan los dos bloques (G y H) que estaban
solo "propuestos", y el patrón mini-mazo + índices es el correcto. El coste de adaptación es bajo
y casi todo mecánico (tablas de mapeo). Los prompts canónicos para producir el resto del contenido
están en `docs/Plantilla_Prompt_Contenido.md`, junto con el plan de producción por cantidades.

**✅ Decisión tomada (destino de archivos):** los generadores emiten directamente el esquema de
la app en `data/cartas/` — una sola fuente de verdad, el árbol `JSONS/onegai/` no se usa.
Referencia de adaptación: `scripts/generar_equipo_data.py` (armas y armaduras al esquema
`TierEquipment`, sin dados de daño, sin pisar ids existentes). Enemigos y misiones se adaptarán
igual cuando exista su modelo Java (Bloques G y H).
