# D0017 — Los oficios: oro, experiencia y algo de reputación; el orden lo pone Claude

- **Estado:** `accepted` — Oriol, 15/10 07:02: «Dan las dos, incluso algo de reputación» (a «¿los oficios dan
  oro, experiencia o las dos cosas?») y «me da igual» (al orden).
- **Fecha:** 2026-10-15 · **Lo escribe:** Claude (Cowork).
- **Origen:** Oriol, 14/10 23:53: «implementar trabajos y acciones: estudiar en la biblioteca, luchar en el
  coliseo, forjar armas en la herrería para que las venda el tendero, bañarte en las termas, sentarte en los
  bancos y sillas, subirte a caballos u otros animales, ayudar con el huerto y cultivar, pescar, ayudar en el
  bar y trabajar allí sirviendo».

## Decisión

1. **Un oficio paga tres cosas a la vez**, según el grado de la tirada (pifia / pierde / roce / éxito / crítico):
   - **oro** (lo principal en los trabajos: servir, forjar, pescar para vender);
   - **experiencia del oficio** (maestría: cuanto más lo haces, mejor te sale y mejor pagan; AV0-07) y un poco de
     la progresión del héroe por la vía no violenta (AV5-10);
   - **algo de reputación** con la facción del sitio (D-MOR-09, la de `reputacion01`): poca, para que trabajar
     para alguien se note sin ser la vía rápida.
   - Lo que **no** es trabajo (bañarse, sentarse, estudiar por gusto) puede costar oro y dar otra cosa: cura,
     quitar fatiga, una carta de lore, subir una competencia.
2. **Generaliza ACTUAR-01**, no lo duplica: un sitio de oficio es un objeto del catálogo que nombra un oficio
   (como hoy `actuacion`), con su tirada enfrentada, su descanso en pasos y su resultado guardado en las flags.
3. **Antes de aceptar se ve lo que vas a ganar** (BOT-06 y EH-22: lo caro o largo se confirma), en una hoja de
   pergamino «Trabajar».
4. **El orden** (lo pone Claude): primero lo que reutiliza lo que ya hay; al final lo que necesita arte o
   sistemas nuevos.
   1. Servir en el bar, estudiar en la biblioteca, pescar.
   2. Termas y sentarse (pose de sustitución hasta que Pixel dibuje «sentado»).
   3. Forjar armas que luego vende el tendero.
   4. Huerto (plantar, esperar días del reloj del mundo, cosechar).
   5. Coliseo (combates por rondas, con apuesta y premio).
   6. Montar (velocidad distinta y arte de montura).

## Contrato del catálogo: `assets/catalogs/oficios.json`

```json
{
  "version": 1,
  "oficios": {
    "servir_bar": {
      "nombre": "Servir en la barra", "verbo": "Servir",
      "objetos": ["barra_taberna"],
      "tirada": { "caracteristica": "CAR", "competencia": "persuasion", "rival": 2 },
      "duracion": 1,
      "descanso": 120,
      "coste": { "oro": 0 },
      "paga": {
        "roce":    { "oro": 4,  "exp": 1, "reputacion": 0 },
        "exito":   { "oro": 8,  "exp": 2, "reputacion": 1 },
        "critico": { "oro": 12, "exp": 3, "reputacion": 2 }
      },
      "faccion": "",
      "efecto": { "pv": 0, "fatiga": 0 },
      "objeto": { "id": "", "cantidad": 0, "calidadPorGrado": false },
      "textos": { "aceptar": "Te pones el delantal…", "exito": "…", "fallo": "…" }
    }
  }
}
```

- `objetos`: los `objectId` donde se hace (el objeto lleva además `"oficio": "<id>"` en su catálogo, como hoy
  `actuacion`).
- `duracion`: franjas del día que pasan (mañana, tarde, noche); `descanso`: pasos antes de repetir en ese sitio.
- `paga` por grado; pifia y perder no pagan (y la pifia puede costar: `"pifia": {"oro": -2}` opcional).
- `faccion`: vacía = la del sitio (la de su tendero o su barrio).
- `objeto`: lo que da (un pez, un arma forjada); `calidadPorGrado` = mejor grado, mejor calidad.
- `efecto`: termas y bancos (PV que cura, fatiga que quita).
