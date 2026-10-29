"""PART-01 — Los elementos del juego (planificacion/CANON_PARTICULAS.md §1).

Oriol (07/10/2026), a «el rayo es Luz, como en NosTale»: «pues hay que cambiarlo porque no estamos
limitados a los 4 elementos del nostale aqui». Las SP y las clases propias usaban los cuatro de NosTale
(Fuego, Agua, Luz y Tinieblas) y el catalogo ya tenia algunos mas en sus cartas (Naturaleza, Veneno,
Hielo, Sangre, Vacio, Arcano...). Estos son los del juego: el elemento de una carta es **de lo que esta
hecha** (lo que dice su texto), y decide su color y sus particulas (y, cuando las haya, las
resistencias). Una carta fisica no lleva elemento.

Cada elemento: la palabra que va en el texto de la carta («3d6 de daño de rayo»), el color de su luz
y la familia de particulas con que se ve (Game::tipoDeParticula).
"""

# nombre -> (palabra en el texto, color de su luz, particulas)
ELEMENTOS = {
    "Fuego": ("de fuego", "ff8c3c", "llamas"),
    "Agua": ("de agua", "5ab4ff", "agua"),
    "Hielo": ("de hielo", "bff0ff", "escarcha"),
    "Rayo": ("de rayo", "fff27a", "rayos"),
    "Tierra": ("de tierra", "c89a5a", "piedras"),
    "Viento": ("de viento", "d8f5e6", "viento"),
    "Luz": ("de luz", "fff0b4", "luz"),
    "Tinieblas": ("de tinieblas", "7a4ab4", "sombra"),
    "Naturaleza": ("de naturaleza", "8cd25a", "hojas"),
    "Veneno": ("de veneno", "a0e650", "veneno"),
    "Sangre": ("de sangre", "d22828", "sangre"),
    "Arcano": ("arcano", "c88cff", "arcano"),
    "Vacio": ("del vacío", "5a3c8c", "vacio"),
}

# Las etiquetas del catalogo que dicen lo mismo con otra palabra (las cartas de las 63 clases de antes)
ALIAS = {"Sagrado": "Luz", "Oscuro": "Tinieblas", "Vacío": "Vacio"}


def nombre(elemento):
    """El nombre de un elemento para leerlo: «Vacío»."""
    return "Vacío" if elemento == "Vacio" else elemento


def de_la_etiqueta(etiqueta):
    """El elemento de una etiqueta de carta (mechanicTags), o None si no es un elemento."""
    e = ALIAS.get(etiqueta, etiqueta)
    return e if e in ELEMENTOS else None


def dano_de(elemento):
    """«de daño de rayo», «de daño arcano»: como lo dice el texto de una carta."""
    return f"de daño {ELEMENTOS[elemento][0]}"
