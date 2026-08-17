# Pla d'habilitats per classe i tier

*ONEGAI — Sistema de Cartes i Tiers · juliol de 2026*

Aquest document defineix quantes habilitats falten, com han de repartir-se i amb quin
criteri s'han d'escriure, abans de generar-ne cap. El generador que l'implementa és
`scripts/generar_habilidades_clases.py`.

---

## 1. El buit real, comptat del catàleg

Hi ha **458 habilitats** en disc, però la distribució és molt desigual:

| Tier | Habilitats amb `classTags` | Situació |
|---:|---:|---|
| 1 | ~172 | 3 per classe: base sòlida |
| 2 | ~62 | 1 per classe: mínim |
| 3 | ~9 | pràcticament buit |
| 4 | 0 | **buit** |
| 5 | 1 | **buit** |

A més, **219 habilitats no tenen `classTags`**: són genèriques i qualsevol classe les pot
agafar. Estan bé com a fons comú, però no donen identitat a cap classe.

**Objectiu proposat: 4 habilitats pròpies per classe i tier.** Amb 56 classes reals
(la 57a, `vsdvsvsd`, és brossa de proves i s'ha d'esborrar) i 5 tiers, això són 1.120
habilitats de classe. En falten **881**.

> Per què 4 i no més: amb el límit de mà per tier del GDD (§11), un personatge de tier 1
> porta ~10 cartes. Si cada classe té 4 opcions pròpies per tier més el fons genèric,
> el jugador tria de debò sense que la selecció sigui aclaparadora.

---

## 2. Pressupost per fases

No cal escriure-les totes de cop. Ordre recomanat:

| Fase | Què | Habilitats | Per què primer |
|---|---|---:|---|
| **F1** | Completar tier 1 i 2 a 4 per classe | ~280 | El joc real comença al tier 1-2; és el que es provarà a taula. |
| **F2** | Tier 3 sencer | ~215 | Primer salt de potència; on es nota la identitat de classe. |
| **F3** | Tiers 4 i 5 | ~448 | Contingut de final de campanya; pot esperar. |

---

## 3. Identitat per rol

Les 56 classes es reparteixen en 5 rols. Cada rol té un patró d'habilitat propi que
el generador ha de respectar:

| Rol | Classes | Stat dominant | Patró d'habilitat |
|---|---:|---|---|
| `caster` | 15 | INT (8) / CAR (5) | Efectes a distància, àrea i control; `range: medium/long`, `duration: concentration` freqüent. |
| `agile` | 11 | DES (7) | Reposicionament, atacs múltiples i evasió; `actionType: movimiento` o `accion_menor`, `duration: instant`. |
| `tank` | 11 | CON (7) | Absorció, provocació i bloqueig; `range: self/melee`, sovint `reaccion`. |
| `support` | 10 | CAR (7) | Curació, bonificacions i neteja de condicions; afecta aliats, `range: short`. |
| `balanced` | 10 | CON/DES/CAR | Barreja d'atac i utilitat; sense un patró dominant. |

**Regla d'or:** l'habilitat ha de fer servir el `primaryStat` de la seva classe a
`requiredStats` i a `effect.scaling`. Una habilitat de `coloso_de_ceniza` (tank, CON)
que escali amb INT està mal generada.

---

## 4. Corba per tier

| Tier | `requiredStats` | Rareses | `recovery` predominant | Abast del efecte |
|---:|---|---|---|---|
| 1 | 1-2 | `common` | `descanso_corto` | Un objectiu, efecte simple |
| 2 | 2-3 | `common` / `uncommon` | `descanso_corto` | Un objectiu + condició lleu |
| 3 | 3-4 | `uncommon` | `descanso_corto` / `descanso_largo` | Àrea petita o dos objectius |
| 4 | 4-5 | `uncommon` / `rare` | `descanso_largo` | Àrea, o efecte persistent |
| 5 | 5-6 | `rare` / `epic` | `descanso_largo` | Canvia l'escena: àrea gran o control dur |

`recovery: activa` es reserva per a habilitats de poc impacte i ús repetit (avui només 21
del catàleg). `ninguno` és per a efectes purament narratius.

---

## 5. Camps obligatoris de cada habilitat generada

L'esquema real de `TierSkill` (secció §3 de `Plantilla_Prompt_Contenido.md`). Cap
habilitat generada pot deixar-ne cap buit sense motiu:

```
id, name, type: "skill", tier, rarity, classTags[], roleTags[], mechanicTags[],
requiredStats{}, requiredTags[], incompatibleTags[], recovery, actionType,
range, duration, defenseStat, effect{description, scaling}, limitations[],
evolvesInto, flavorText
```

- `cost` és **llegat** de l'edició 1 (`resource: none, amount: 0`): es manté per no
  trencar dades antigues, però no s'omple amb valors reals.
- `defenseStat` només s'omple si l'habilitat obliga a una tirada de defensa
  (`CA`, `resistencia_fisica` o `defensa_mental`). Una curació no en té.
- `evolvesInto` enllaça amb la versió de tier superior de la mateixa família. És el que
  farà possible la substitució automàtica en pujar de tier (Bloc C, ítem 20 del pla de
  tancament), avui encara sense implementar.

---

## 6. Criteris de qualitat, per evitar 881 cartes bessones

El risc real d'aquest volum és generar 881 variants de "fas 2 de dany". Tres regles:

1. **Cada tier de cada classe cobreix quatre intencions diferents:** ofensiva, defensiva,
   control/utilitat i moviment/recurs. Quatre habilitats, quatre respostes a la pregunta
   "què faig aquest torn".
2. **El text parteix de la fitxa de la classe**, no d'una plantilla neutra: `description`,
   `specializations` i `primaryResource` de la classe donen el vocabulari. Una habilitat
   de `sanadora_de_esporas` parla d'espores, no de "energia".
3. **`evolvesInto` encadena famílies:** cada habilitat de tier N apunta a la seva versió de
   tier N+1. Això obliga a pensar en línies de progressió i no en cartes soltes.

---

## 7. Com s'executa

```bash
# Veure què faria, sense escriure res
python3 scripts/generar_habilidades_clases.py --dry-run

# Fase 1: completar tiers 1 i 2 a 4 habilitats per classe
python3 scripts/generar_habilidades_clases.py --tiers 1,2

# Una classe concreta, per revisar el resultat abans d'anar a l'engròs
python3 scripts/generar_habilidades_clases.py --clase coloso_de_ceniza --tiers 3
```

El generador **mai sobreescriu** un fitxer existent i sempre completa fins al mínim de 4
per classe i tier, comptant les que ja hi ha.

> **Abans d'executar-lo, fes commit** (`docs/Guia_Control_Versions.md` §8): escriu
> centenars de fitxers a `data/cartas/habilidades/` d'una tacada, i `git restore data/`
> és la manera de desfer-ho si el resultat no convenç.

---

## 8. Revisió posterior obligatòria

Generar no és acabar. Un cop escrites, cal passar-hi:

1. `python3 scripts/exportar_catalogo_ids.py habilidades` — comprovar que no hi ha ids
   duplicats.
2. `/diagnostico` a l'aplicació — detecta referències trencades (`evolvesInto` que apunti
   a una carta que no existeix).
3. Lectura humana d'una mostra: 5 habilitats de 3 classes de rols diferents. Si les tres
   sonen intercanviables, el generador necessita més vocabulari per classe.
