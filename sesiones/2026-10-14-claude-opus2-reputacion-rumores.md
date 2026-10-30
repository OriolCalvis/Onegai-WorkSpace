# 2026-10-14 · Claude (Opus, segunda sesión) · AV14-23 — La reputación por facción y los rumores

La SIGUIENTE de `orquestador/claude_opus2.md`, pedida por Oriol a las 11:23. D0015 §15: **D-MOR-09 sí**
(la reputación por facción, en el comercio y en el HUD) y **D-MIS-03 sí** (el rumor es un beat que salta
al hablar).

## Qué había

La reputación se movía y nadie la leía: cada acto de `assets/rules/actos.json` dice qué facción lo vio
(`perdidos`, `aegroum`, `barriada_surysal`) y `MoralDeLaSesion` la apuntaba en
`CharacterSheet::factionRep`. Además **no se guardaba**: al cargar una partida volvía a 0.

## Qué hay ahora

1. **Las facciones y sus tiendas** (`assets/rules/facciones.json`, nuevo): Aegroum (las tiendas de
   Boundington: Rosita, Aigren, Luisarda y los tenderos de la ciudad), los Perdidos y la Barriada de
   Surysal, con los topes del precio. Las tiendas de otras naciones no son de ninguna facción y no
   cambian.
2. **La tienda cobra según lo que su facción piensa de ti** (`Game/ComercioReputacion`, nuevo, usado
   por `GameSessionComercio`): el ajuste es el de `CharacterSheet::repPriceModPct` (−0,75 % por punto),
   acotado entre −25 % y +50 %. Si te detestan, también te pagan menos por lo que vendes; si te aprecian,
   no te pagan más. Así nunca sale a cuenta comprar y revender (lo comprueba la prueba con todas las
   tiendas publicadas).
3. **Se ve:** en la hoja de la tienda, a la derecha de las pestañas («Aegroum: te aprecian (+12). Pagas
   un 9 % menos.»), y en la ficha del cuaderno, un bloque **REPUTACIÓN** debajo de la historia personal,
   con cada facción con la que no estás a 0 y cómo te ve (te detestan, desconfían, recelan, neutral, te
   aprecian, te estiman, eres de los suyos).
4. **Se guarda:** `SavedHero::reputacion` (clave `hero.reputacion` del guardado, solo si hay algo; los
   guardados de antes se leen igual) y la ficha la recupera al cargar (`NewGameFlow::fichaGuardada`).
5. **Los rumores:** el formato y un ejemplo, en `FORMATO_AVENTURAS.md` («Rumores»): un beat `talk` con
   sus condiciones, `notFlags` de su propia flag `rumor_<tema>`, y detrás de la charla del PNJ. **No** se
   han tocado las aventuras: tres rumores para Boundington, listos para pegar, en
   `out/claude_reputacion01/rumores_propuestos.json`.

## Comprobado

- `test_comercio_reputacion` (nueva, 5 casos): las facciones casan con las tiendas de verdad y no hay
  reventa (mutación cazada: con un 60 % de descuento, el pan de Rosita se compraría a 3 y se revendería
  a 4); la tienda cobra y paga según la reputación, con topes, y no cambia sin facción; los textos de la
  tienda y de la ficha; la reputación en el guardado (y uno de antes sin ella); un rumor sale al hablar,
  una vez, y detrás de la charla no se la come.
- Siguen en verde `test_shop_flow`, `test_tienda_estado`, `test_save_game`, `test_pergamino` y las del
  diario.
- `ctest -L gl-free`: 221/223 con el `claude_dialogos04` de las 11:42. Fallan los mismos dos que en
  AV14-22: `test_marcar_paquetes` y `test_ruta_casillas` (mide tiempo). `linea_base_core`: 0 de deuda nueva. `juego` compila.
- Capturas de la tienda y de la ficha (pintadas en la CPU, con la fuente y los iconos de verdad):
  `out/claude_reputacion01/capturas/`.

## Cómo se aplica

Después de `claude_dialogos04` y de `claude_diario01`:

```
bash MotorGraphico/out/claude_reputacion01/aplicar.sh
```

No pisa ficheros: los cambios entran **por ancla** (cada trozo de antes tiene que estar una vez en su
fichero; si alguno no encaja, no toca nada y dice cuál). Así entra aunque `claude_dialogos04`, que se ha
publicado cuatro veces durante estas sesiones, cambie otras partes de los mismos ficheros. `claude_diario01` se ha rehecho
igual. Lo de antes queda en `antes_de_aplicar/`.

## Lo que queda

- **Pegar los rumores** propuestos en las aventuras (quien lleve `assets/adventures/`).
- **Que un beat pueda pedir reputación** (`requires` con una facción): no existe en
  `NarrativeCondition`. Serviría para que un tendero no te hable si te detestan, o para los rumores que
  solo cuentan a los de casa.
- **La Barriada y los Perdidos no tienen tiendas**: su reputación se ve en la ficha pero no cambia
  ningún precio hasta que tengan tendero.
