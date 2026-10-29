"""SETS-03 — El set de tier 2 de las seis clases de la 3.ª iteración (CLASES-03).

Oriol (08/10/2026): «con los conceptos que hicimos los sets del Nostale me hagas los sets del apóstata,
señor de las armas, furia de los creadores, bardo de las 4 estaciones de tier 2, siguiendo los mismos
criterios, y el mago de los chasquidos y el monje que hice».

Los mismos criterios que los sets de las SP y de las clases propias (planificacion/CANON_SP_CLASES.md §8 y
CANON_CLASES_PROPIAS.md §7):
- un set entero por clase: el arma y la cabeza, el torso, las piernas y los pies, objetos raros de tier 2
  (nivel 20, 550; sp_reglas.pieza) con la etiqueta «Set del <nombre corto>»;
- la clase NO lo lleva puesto: empieza como hasta ahora (su equipo inicial y el kit de su arquetipo); son
  objetos sueltos que cualquiera puede ponerse y combinar;
- el peso de la armadura, el de su rol (media el apoyo y el equilibrado, ligera el ágil, el lanzador y el
  monje, que no puede llevar más); el arma, la de su tipo (el sable, el orbe, la maza) o, si la clase no
  trae ninguna (el Monje y el Mago Chasqueador), la que dice qué es: los guanteletes del artista marcial y
  los guantes con que chasquea;
- bonos: el arma, la cabeza, las piernas y los pies, +1 a los dos atributos de la clase; el torso, +1 al
  principal;
- lo que se ve, dibujado desde la identidad de la clase y sus partículas (tools/particulas.py: el Apóstata,
  sangre; el Señor de las Armas, oro; la Furia, furia dorada; el Bardo, las estaciones; el Monje, ki; el
  Mago Chasqueador, arcano), sin pisar ningún set de los 64 que ya hay. El del Bardo, además, desde su origen:
  la cultura de los hombres gato (Oriol, 08/10).

Las piezas propias se dibujan en tools/actores_trajes_iteracion3.py; las del catálogo que se reutilizan (las
grebas, las calzas, las botas, los zapatos, las sandalias) las tiñe el tema del set
(actores_estetica.TEMAS_POR_NOMBRE). Lo escribe en los catálogos tools/importar_clases_companeros.py (el
importador de estas seis clases).
"""
from sp_reglas import DOS_MANOS, pieza

# El Apóstata: «un exguerrero sagrado que ha renunciado a la luz para abrazar el poder oscuro». Su
# armadura de paladín, ennegrecida; el tabardo rasgado con el sol de la orden boca abajo; la capucha granate
# y, encima, la aureola partida. Apoyo (support): armadura media. CAR / CON.
APOSTATA = {
    "id": "el_apostata", "name": "El Apóstata", "set_de": "Apóstata",
    "set": [
        pieza("Sable del Apóstata", "MainHand", "ligera", {"CAR": 1, "CON": 1},
              "Sable de hoja negra con el filo rojo, como una herida abierta, y la guarda de hierro "
              "ennegrecido.", restricciones=["Precisión: DES"]),
        pieza("Coraza del Apóstata", "Torso", "media", {"CAR": 1},
              "Coraza de paladín ennegrecida, con hombreras de pinchos, bajo un tabardo gris rasgado: el sol de "
              "la orden, bordado en rojo, cuelga boca abajo. Ciñe una cadena por cinturón."),
        pieza("Capucha del Apóstata", "Head", "media", {"CAR": 1, "CON": 1},
              "Capucha honda de lana granate. Encima flota, ennegrecida y partida en dos, la aureola que llevó "
              "de paladín."),
        pieza("Grebas del Apóstata", "Legs", "media", {"CAR": 1, "CON": 1},
              "Grebas de acero ennegrecido."),
        pieza("Botas del Apóstata", "Feet", "media", {"CAR": 1, "CON": 1},
              "Botas altas negras."),
    ],
}

# El Señor de las Armas: «hechicero-guerrero que invoca armas místicas y reliquias antiguas [...] convierte
# el campo de batalla en su arsenal». SETS-04 (Oriol, 08/10): «vull modificar estèticament la classe del Senyor de
# les armes» (con el oro, el maná rojo que brilla, las armas que flotan y el aura dorada, en un diseño propio); de
# los tres esbozos, «estan perfectes els 3». Tiene tres sets, con los mismos bonos y el mismo peso: el Duque, el
# Soberano sin Rostro y el Rey de la Forja. Sustituyen al de SETS-03 (el manto carmesí con armiño, la corona de
# espadas y las calzas y los zapatos del catálogo; RETIRADOS). Lanzador (caster): armadura ligera; su arma es el
# orbe. INT / CON.
SENOR_DUQUE = {
    "id": "senor_de_las_armas", "name": "Señor de las Armas", "set_de": "Duque",
    "set": [
        pieza("Orbe del Duque", "MainHand", "ligera", {"INT": 1, "CON": 1},
              "Orbe de maná rojo encendido en su montura de oro.", restricciones=["Precisión: INT"]),
        pieza("Casaca del Duque", "Torso", "ligera", {"INT": 1},
              "Casaca negra cerrada de cuello alto, con los cantos y los botones de oro y líneas de maná rojo por "
              "los brazos; hombreras de oro, cinturón ancho de oro con una gema oscura, placas de oro y tiras rojas "
              "en las caderas, y una capa roja que le nace de la cintura. A los lados le flotan cuatro hojas de "
              "filo rojo."),
        pieza("Diadema del Duque", "Head", "ligera", {"INT": 1, "CON": 1},
              "Aro fino de oro con un rubí en la frente."),
        pieza("Calzas del Duque", "Legs", "ligera", {"INT": 1, "CON": 1},
              "Calzas negras con un rombo de oro en cada rodilla."),
        pieza("Botas del Duque", "Feet", "ligera", {"INT": 1, "CON": 1},
              "Botas negras con la greba de oro en punta y la puntera de oro."),
    ],
}
SENOR_SOBERANO = {
    "id": "senor_de_las_armas", "name": "Señor de las Armas", "set_de": "Soberano sin Rostro",
    "set": [
        pieza("Orbe del Soberano sin Rostro", "MainHand", "ligera", {"INT": 1, "CON": 1},
              "Orbe rojo como la rendija de su yelmo, en su montura de oro viejo.",
              restricciones=["Precisión: INT"]),
        pieza("Placas del Soberano sin Rostro", "Torso", "ligera", {"INT": 1},
              "Placas negras, finas como hojas, con los cantos de oro y tres placas de oro en cada hombro; el "
              "tabardo rojo con la corona de espadas bordada y una capa larga roja con el bajo de oro. Cuatro hojas "
              "espectrales rojas le rondan la cintura."),
        pieza("Yelmo del Soberano sin Rostro", "Head", "ligera", {"INT": 1, "CON": 1},
              "Yelmo negro cerrado con una corona de oro de cinco puntas de espada y una rendija en T que brilla de "
              "rojo. No se le ve la cara."),
        pieza("Grebas del Soberano sin Rostro", "Legs", "ligera", {"INT": 1, "CON": 1},
              "Grebas negras con un rombo de oro en cada rodilla."),
        pieza("Escarpes del Soberano sin Rostro", "Feet", "ligera", {"INT": 1, "CON": 1},
              "Escarpes negros con la puntera de oro."),
    ],
}
SENOR_FORJA = {
    "id": "senor_de_las_armas", "name": "Señor de las Armas", "set_de": "Rey de la Forja",
    "set": [
        pieza("Orbe del Rey de la Forja", "MainHand", "ligera", {"INT": 1, "CON": 1},
              "Una bola de oro fundido que nunca se enfría, en su montura de hierro.",
              restricciones=["Precisión: INT"]),
        pieza("Armadura del Rey de la Forja", "Torso", "ligera", {"INT": 1},
              "Láminas finas de hierro oscuro con grietas por las que brilla el oro fundido, hombreras con el canto "
              "al rojo y guanteletes con los nudillos de oro fundido. A la espalda lleva un mandoble recién forjado, "
              "con el filo que aún brilla."),
        pieza("Corona del Rey de la Forja", "Head", "ligera", {"INT": 1, "CON": 1},
              "Aro de hierro con cinco hojas rotas de punta, las puntas todavía al rojo y oro fundido brillando por "
              "las grietas."),
        pieza("Grebas del Rey de la Forja", "Legs", "ligera", {"INT": 1, "CON": 1},
              "Grebas de hierro con una grieta de oro fundido en cada espinilla."),
        pieza("Botas del Rey de la Forja", "Feet", "ligera", {"INT": 1, "CON": 1},
              "Botas de hierro con la puntera de oro fundido."),
    ],
}

# SETS-04: el set de SETS-03 del Señor de las Armas, que sustituyen los tres de arriba. El importador
# (tools/importar_clases_companeros.py) los quita de los catálogos.
RETIRADOS = ["Orbe del Señor de las Armas", "Manto del Señor de las Armas", "Corona del Señor de las Armas",
             "Calzas del Señor de las Armas", "Zapatos del Señor de las Armas"]

# La Furia de los Creadores: «guerrero sagrado que fusiona la furia bárbara con la devoción paladín». Medio
# paladín (el peto blanco con los cantos de oro, el yelmo con la corona de rayos) y medio bárbaro (los brazos al
# aire, la piel de oso negra, la faja roja de guerra, las botas de piel). Equilibrado (balanced): armadura
# media; su arma es la maza. CAR / CON. Set «de la Furia» (femenino).
FURIA = {
    "id": "furia_de_los_creadores", "name": "Furia de los Creadores", "set_de": "Furia",
    "set": [
        pieza("Maza de la Furia", "MainHand", "media", {"CAR": 1, "CON": 1},
              "Maza de cabeza de aletas de oro, con el mango forrado de cuero rojo y el collar de acero blanco.",
              restricciones=["Precisión: CON"]),
        pieza("Coraza de la Furia", "Torso", "media", {"CAR": 1},
              "Peto de placas blancas con los cantos de oro, una piel de oso negra sobre los hombros y la faja "
              "roja de guerra; los brazos, al aire."),
        pieza("Yelmo de la Furia", "Head", "media", {"CAR": 1, "CON": 1},
              "Yelmo blanco de cara abierta; detrás se le abre una corona de rayos de oro, como un sol."),
        pieza("Grebas de la Furia", "Legs", "media", {"CAR": 1, "CON": 1},
              "Grebas blancas con los cantos de oro."),
        pieza("Botas de la Furia", "Feet", "media", {"CAR": 1, "CON": 1},
              "Botas de cuero rojo con el borde de piel negra."),
    ],
}

# El Bardo de las Cuatro Estaciones: «canaliza el poder de la naturaleza a través de canciones estacionales».
# Oriol (08/10): «el origen del bardo de las 4 estaciones salió en la cultura de los hombres gato». En su vault
# (Gran Compendio de los Creadores) los Hombres Gato se asentaron en las Praderas de Oro de Bosmurg, nómadas, con
# una cultura «basada en la tradición oral, en las canciones y las historias que contaban alrededor de fuegos»,
# y la Marcha de las Cuatro Estaciones (8790 b.f.; verano, otoño, invierno y primavera) quedó en la Balada de las
# Cuatro Estaciones, que «narra la marcha y el destino de cada estación»; su ficha de raza los hace nómadas, en
# contacto con la naturaleza y silenciosos al moverse. Así que es un bardo de camino: el poncho de lana dorada
# como las praderas, con la Marcha tejida, la corona de hojas que cambia de estación al dar la vuelta y deja
# libres las orejas, y las botas blandas. Ágil (agile): armadura ligera; su arma es el sable. CAR / DES. Set
# «de las Cuatro Estaciones» (femenino plural).
CUATRO_ESTACIONES = {
    "id": "bardo_de_las_cuatro_estaciones", "name": "Bardo de las Cuatro Estaciones",
    "set_de": "Cuatro Estaciones",
    "set": [
        pieza("Sable de las Cuatro Estaciones", "MainHand", "ligera", {"CAR": 1, "DES": 1},
              "Sable de hoja escarchada con el puño verde de primavera, la guarda de oro de verano y una gema "
              "roja de otoño.", restricciones=["Precisión: DES"]),
        pieza("Poncho de las Cuatro Estaciones", "Torso", "ligera", {"CAR": 1},
              "Poncho de lana dorada como las Praderas de Oro, con flecos, sobre una camisa parda. En las "
              "cenefas del pecho y del bajo lleva tejida la Marcha de las Cuatro Estaciones, en el orden en que "
              "la canta su balada: el oro del verano, el rojo del otoño, el blanco del invierno y el verde de la "
              "primavera."),
        pieza("Corona de las Cuatro Estaciones", "Head", "ligera", {"CAR": 1, "DES": 1},
              "Corona de hojas que cambia de estación al dar la vuelta a la cabeza: verde con flores por "
              "delante a un lado y dorada al otro; roja de otoño y blanca de escarcha por detrás. Ciñe la "
              "frente y deja libres las orejas."),
        pieza("Pantalones de las Cuatro Estaciones", "Legs", "ligera", {"CAR": 1, "DES": 1},
              "Pantalones de lana parda, de viaje."),
        pieza("Botas de las Cuatro Estaciones", "Feet", "ligera", {"CAR": 1, "DES": 1},
              "Botas cortas de cuero pardo, blandas y de suela fina, para moverse en silencio."),
    ],
}

# El Mago Chasqueador: «mago de reflejos y suerte: sus dados vuelven a rodar y su magia llega antes que la
# respuesta». Rápido y elegante, como un húsar: la chaqueta corta de cuello alto con los alamares de plata, el
# bombín y, por arma, los guantes con que chasquea (uno en cada mano, como los guanteletes). Lanzador
# (caster): armadura ligera. INT / DES.
CHASQUEADOR = {
    "id": "mago_chasqueador", "name": "Mago Chasqueador", "set_de": "Chasqueador",
    "set": [
        pieza("Guantes del Chasqueador", "MainHand", "ligera", {"INT": 1, "DES": 1},
              "Guantes blancos de mago con las yemas bordadas de runas violeta, que chispean cada vez que "
              "chasqueas.", restricciones=["Precisión: INT", DOS_MANOS]),
        pieza("Chaqueta del Chasqueador", "Torso", "ligera", {"INT": 1},
              "Chaqueta corta azul noche de cuello alto, con los alamares de plata cruzándole el pecho y una faja "
              "violeta."),
        pieza("Bombín del Chasqueador", "Head", "ligera", {"INT": 1, "DES": 1},
              "Bombín azul noche con una cinta violeta y una estrella de plata prendida a un lado."),
        pieza("Pantalón del Chasqueador", "Legs", "ligera", {"INT": 1, "DES": 1},
              "Pantalón azul noche con una franja violeta por fuera de cada pierna."),
        pieza("Botas del Chasqueador", "Feet", "ligera", {"INT": 1, "DES": 1},
              "Botas altas negras, lustrosas."),
    ],
}

# El Monje del Despertar: «artista marcial que acumula Ki con cada golpe certero». El que ha despertado: el
# tercer ojo de jade en la frente, el hábito de marfil sin mangas (Abdomen de Marfil) con las solapas y la faja
# de jade (Rejuvenecimiento de Jade), las vendas en los antebrazos y los guanteletes de jade. Su clase solo
# lleva armadura y arma ligeras. DES / CON.
DESPERTAR = {
    "id": "monje_del_despertar", "name": "Monje del Despertar", "set_de": "Despertar",
    "set": [
        pieza("Guanteletes del Despertar", "MainHand", "ligera", {"DES": 1, "CON": 1},
              "Guanteletes de jade con los nudillos de oro, uno en cada mano.",
              restricciones=["Precisión: DES", DOS_MANOS]),
        pieza("Hábito del Despertar", "Torso", "ligera", {"DES": 1},
              "Hábito de marfil sin mangas, cruzado al pecho con las solapas de jade, la faja de jade de cabos "
              "largos y vendas de marfil en los antebrazos."),
        pieza("Diadema del Despertar", "Head", "ligera", {"DES": 1, "CON": 1},
              "Aro fino de oro con un tercer ojo de jade abierto en la frente."),
        pieza("Pantalones del Despertar", "Legs", "ligera", {"DES": 1, "CON": 1},
              "Pantalones anchos de jade, atados en los tobillos con tiras de marfil."),
        pieza("Sandalias del Despertar", "Feet", "ligera", {"DES": 1, "CON": 1},
              "Sandalias de cuerda."),
    ],
}

SETS = [APOSTATA, SENOR_DUQUE, SENOR_SOBERANO, SENOR_FORJA, FURIA, CUATRO_ESTACIONES, CHASQUEADOR,
        DESPERTAR]
