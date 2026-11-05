# D0029 — Las ramas a las que se llega huyendo cuentan una huida (05/11/2026)

Estado: **proposed**. Oriol respondió «Continúa» a la recomendación del Brain de reescribirlas; los textos están en la rama y esperan a que los lea. Consecuencia de D0027 y sigue a D0028.

| # | Decisión |
|---|---|
| 1 | Cambia el texto de `bgd_perdida`, `bgb_detras_perdida`, `b2_saqueadores_ganan`, `b1_secta_ganan`, `b3_sandes_gana_casco` y `b3_sandes_sigue`: el héroe huye, no cae. Ningún efecto cambia. |
| 2 | Si la historia necesita que te atrapen, te atrapan huyendo: `bgd_perdida` sigue atando al héroe y `b3_compania` añade una frase antes de «Vas a morir» en la que las cadenas de Sandes lo arrastran de vuelta. |
| 3 | `b2_saqueadores_ganan` conserva a Zarela («Has sangrado por nosotros»), el amuleto y el acto `barriada_sangre`: peleaste antes de huir. *Por defecto del Brain, a confirmar.* |
| 4 | Dos entradas del diario que no se encendían pasan a la flag de huir: `bnd1_guarida_huida` («...y huiste.») y `bnd2_saqueadores_huido` («...y huiste.»). |
| 5 | No se tocan `b1_verdaderos_ganan`, `b1_guarida_perdida`, `b3_bandidos_otros`, `b3_verina_perdida` ni `b3_edul_perdido`: su texto no contradice una huida. Los beats muertos y las flags sin lector siguen en `REVIVIR_1.md`. |
