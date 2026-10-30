# Auditoría de campos de referencia — "nada de escribir ids a mano"

**Fecha:** 2026-07-16 · **Resultado:** ✅ AUDITORÍA OK (33 selectores activos, 15 catálogos de opciones)

## Por qué existe esta auditoría

Los campos que referencian cosas ya creadas (ids de clases, habilidades, equipo, tags...)
eran texto libre. Una falta de ortografía (`guardian_de_hierro` → `guardián_de_hierro`)
rompía la referencia **en silencio**: la app no fallaba, simplemente la carta dejaba de
encontrarse. Esta auditoría garantiza que eso no puede volver a pasar.

## Cómo re-ejecutarla (en cualquier momento)

```bash
python3 scripts/auditar_campos_referencia.py
```

Sale con código 0 y `AUDITORÍA OK` si todo está bien. Comprueba 4 reglas:

1. Ningún `<input>` de texto ligado a un campo de referencia (lista `CAMPOS_REFERENCIA`
   del script + cualquier campo acabado en `Id`/`Ids`).
2. Toda plantilla que usa el selector de chips carga `/js/selector-csv.js`.
3. Todo `${opciones...}` usado en plantillas lo aporta `OpcionesReferenciaAdvice`.
4. El advice cubre (assignableTypes) el controlador de cada plantilla que usa opciones.

**Al añadir un campo nuevo que referencie otro catálogo:** añádelo a `CAMPOS_REFERENCIA`
en el script (o llámalo `...Id`/`...Ids`) y la auditoría vigilará que sea seleccionable.

## Las piezas del sistema

| Pieza | Archivo | Qué hace |
|---|---|---|
| Advice | `controller/OpcionesReferenciaAdvice.java` | Inyecta en los editores las listas `Opcion(id, etiqueta)` de todo lo que ya existe + vocabularios de tags extraídos de los propios datos (no se inventa nada) |
| Fragmentos | `templates/Blocks/selector-referencias.html` | `csv(campo, opciones, vacio)` = selección múltiple con chips; `uno(...)` = desplegable único. El valor viaja como CSV → los controladores no cambian |
| JS | `static/js/selector-csv.js` | Pinta chips, añade desde el desplegable, quita con ×. Un valor que ya no esté en el catálogo se muestra con ⚠ (no se pierde al guardar) |
| CSS | `static/css/components.css` (`.gm-csvsel*`) | Estilo de chips coherente con el resto del editor |
| Auditoría | `scripts/auditar_campos_referencia.py` | Verificación automática de las 4 reglas |

## Qué se convirtió (campo → de dónde salen las opciones)

| Formulario | Campo | Opciones |
|---|---|---|
| Habilidades | classTags | clases existentes |
| | roleTags | roles ya usados en datos (agile, caster, tank, support, balanced) |
| | mechanicTags | mecánicas ya usadas en datos (~30) |
| | requiredTags / incompatibleTags | tags de equipo + mecánicas + ids de condición + tags de dotes |
| | effectScaling | escalados ya usados + none/con_tier/CON/DES/INT/CAR |
| | evolvesInto | habilidades existentes (con su tier) |
| Hechizos | school | escuelas ya usadas (7) |
| | classTags, mechanicTags, requiredTags, incompatibleTags, evolvesInto | igual que habilidades, evolvesInto → hechizos |
| Clases | startingEquipment | equipo existente (con slot) |
| | startingPassives | pasivas existentes |
| | startingSkills / learnableSkills | habilidades existentes |
| | startingSpells | hechizos existentes |
| | allowedEquipmentTags | tags de equipo ya usados |
| | restrictedTags | vocabulario combinado |
| Dotes | classTags, grantedTags, requiredTags, incompatibleTags, effectScaling | ídem |
| Pasivas | classTags, synergyTags, effectScaling | ídem |
| Armas/Equipo | grantedTags | tags de equipo |
| | linkedSkill | habilidades existentes (único) |
| Invocaciones | summonedBy | hechizos + habilidades (único, con prefijo Hechizo:/Habilidad:) |
| Deidades | compatibleWith | razas + clases (con prefijo Raza:/Clase:) |
| Eventos | continuesTo | eventos + historias (único) |
| | trigger.type | lista fija: manual/temporal/condicion/lugar/historia |

## Qué sigue siendo texto libre — y por qué está bien

Campos **narrativos**, sin referencia a nada: nombre, descripción, flavorText, trigger de
pasivas ("Mientras lleves escudo"), limitations, duración de habilidades, área de hechizos,
poblacion/gobernante/rasgo de ciudades, domain de deidades (cada una tiene el suyo propio),
cureConditions de condiciones (instrucciones de mesa, no ids), affinities de razas,
specializations de clases (nombres propios), campos del Prompt Maestro de trasfondos.

## Huecos detectados (aún no personalizable o tedioso desde la web)

1. **Mapa mundi**: crear/editar ciudades y zonas usa diálogos `prompt()` del navegador —
   funciona pero es tosco; la nación se autodetecta por polígono pero no se elige de lista.
   Las **naciones** (polígonos, colores, nombres) no tienen editor de formulario.
2. **Historias, PNJs, tesoros, enemigos**: se ven en la web pero **no tienen formulario de
   creación** — solo se crean por JSON/scripts. Ahí los prompts de la guía son la vía buena.
3. **Editor de trasfondos**: aún usa el esquema antiguo (no el characterCreation del
   Prompt Maestro §4b). Pendiente de rehacer.
4. **grantedSpells de deidades**: no editable desde el formulario web.

Para todos estos, la guía `docs/Guia_Prompts_Mundo_y_Creacion.md` da prompts listos que
generan el JSON correcto contra los catálogos reales.
