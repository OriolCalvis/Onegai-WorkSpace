# La moral que no se corrompe deprisa y las alternativas de Boundington · 08/10/2026

Sesión de Claude (Cowork). Oriol:

> «ahora plantea para cada escenario un par de alternativas que se conecten con la trama principal para dar
> variedad de opciones al jugador de pasarse el juego, así pudiendo trabajar el carma con las diferentes opciones
> que el jugador vaya marcando, lo ideal sería parametrizar bien la moral, porque al final, si nos corrompemos muy
> rápido no tendría gracia para nada...»

La propuesta fue el documento «Boundington: alternativas, karma y moral» (Claude Docs); Oriol contestó en el
mismo documento («ya he respondido en el fichero») y en el chat (las tiradas de «2 seises», Sandes y los
guardias). Seguimiento: `planificacion/seguimiento/MORAL-01.json` y `BND-ALT-01.json`. Canon:
`planificacion/CANON_BOUNDINGTON_CAMPANA01.md` §11.

## 1. Lo que se hizo

1. **MORAL-01, el motor**: la moral es un libro de actos. Lo de cada día se apunta y se asienta al dormir con
   seis números (`assets/rules/moral.json`): peso 2/5/12, la repetición del mismo tipo en el día, un tope de ±10
   por noche para menores y medios, la resistencia (cuanto más lejos del 0, menos mueve) y las marcas de los
   actos mayores contra inocentes. Siete tramos (Villano −75, Desgraciado −60, Canalla −20, Neutral, Respetado
   +20, Salvador +60, Héroe +75), la pureza aparte y la reputación por facción. En las aventuras: `acto`,
   `herida`, `requires.moral` y las tiradas de éxitos fijos (`skillCheck.exitos`). El aura de los siete tramos,
   con alas en Héroe y Villano, comprobada en el juego.
2. **BND-ALT-01, el primer día**: Rosita (antes Skilla) y su primer favor; Aigren con dos temas y cuatro salidas;
   Luisarda con su tercera tirada, la sala (silencio o Rosita) y la delación ante Sandes; la guarida de parte del
   herrero; la Barriada para la familia del gato o cobrarle al padre; Xila primero desconocido, sobrio o
   borracho; Venides borracho; la pelea (Rosita no deja matar dentro) y el carterista.

3. **BND-ALT-02, el segundo día**: los hombres de Venides por cuándo llevas las pruebas (el primer día, 20; el
   informe de la segunda noche, 10); el capitán de la guardia; Gorran de rodillas (dejarle huir o rematarle); la
   espada de Rosita (devolverla, devolverla y contarle todo, o mentir), y Rosita cuenta quién fue (confianza 2) y
   habla de Verina (3); Zarela, los seis saqueadores y el fuego del ritual; delatar a la Barriada; robar en la
   fiesta y señalar o chantajear al noble; el cobrador y entrar con Edul; el informe a Venides con tres respuestas;
   y lo que se cobra del primer día (Aigren, Luisarda, Xila, la familia del gato). Objetos nuevos en
   `assets/objects/boundington_objetos.json`; el modo `alternativas` de `simulador_dificultad` mide los combates
   nuevos.

## 2. Lo que se decidió (sin preguntar; para revisar)

En el canon, §11.2 (primer día) y §11.5 (segundo). Lo gordo: los hombres de Venides se tocan en el segundo día (donde lo puso Oriol); la A de
Aigren da un rumor y no una pista; el trago de Xila da una pista y la historia de los Perdidos queda en una flag. Los combates nuevos se miden con el
simulador: los seis saqueadores son esbirros (un 1 contra 3) y «acabar con todos» en la cripta son 3 Devotos y 1
Fanático (con 4 y 2 el héroe caía el 95 %).

## 3. Ojo

- **Éxitos a la letra**: con Carisma 3, 2 éxitos salen el 8,8 % de las veces y 4 éxitos nunca. En el juego, la
  C de Aigren y de Luisarda es, el primer día, casi imposible.
- `tools/normalitza_seguiment.py --escriure` toca **todos** los registros que no cumplen el contrato, no solo
  los tuyos: correrlo así cambió 85 ficheros de otros agentes (se deshizo con `git checkout`).
- La demo `demo_campana_boundington` no exige su cobertura: el `require(muertos.empty())` quedó dentro de un
  comentario (un `\n` literal). La cobertura de verdad la exige `demo_boundington`.
