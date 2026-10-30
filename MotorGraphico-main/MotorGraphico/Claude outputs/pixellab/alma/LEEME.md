# ALMA-01 — la calle con alma (01/10/2026, tarde)

Oriol replantea la dirección artística: la captura del centro con el río y los
monumentos pixen «es de las mejores que tenemos»; lo que falla son las casas
(búnkeres de bloques) y la falta de alma. Decisión: el ancla de estilo son los
monumentos pixen y esa captura; las casas suben a su nivel. Los personajes
marcan la escala.

Prueba pequeña, montada en el mapa real (suroeste de la plaza del Casco Antiguo):

- `alma_01_calle_con_alma.png`: casas enteras por tamaño de huella (2×2 con cinco
  variantes, 3×2, 2×3, 3×3 con dos), la herrería de Aigren con patio (yunque,
  fragua, abrevadero), la Taberna Humilde con mesa y barriles fuera, la tienda
  del sur con toldo. A la derecha siguen las casas de bloques, para comparar.
- `alma_02_plaza_decorada.png`: decorado por zona: fanales de los Guardianes,
  bancos con brasero de cristal, paradas de mercado, el brocal; ninguna ropa
  tendida (solo entre casas, en los carrers de vivendes).
- `alma_03_piezas_lote14.png`: las piezas a escala real junto a Skilla.

Receta: `create_image_pixen` (high top-down, south, single color black outline,
medium detail, no_background) con el sufijo de estilo de la guía, canvas ≈ 1,15 ×
(w+h)·32, y `tools/pixellab/quitar_peana.py` para quitar la losa que pixen pinta
siempre bajo un edificio. 23 generaciones (485 de 2.000). La guía completa está
en Claude Docs («Guia d'estil de Boundington»).
