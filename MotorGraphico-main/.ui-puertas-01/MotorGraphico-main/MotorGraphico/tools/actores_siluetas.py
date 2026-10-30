"""Sombreado automatico de plantillas: de una silueta a los cinco tonos.

Dibujar un casco o un peinado pixel a pixel con luz, base, sombra y
contorno en tres direcciones es lento y facil de descuadrar. Aqui se
dibuja solo la SILUETA y este modulo pone los tonos con la misma regla
que el cuerpo base: luz desde arriba a la izquierda, contorno duro por
fuera y una franja de sombra en el borde de abajo y de la derecha.

Caracteres de la silueta:
    #  material 1          +  material 2          .  nada
    z Z w                  se copian tal cual (sombra translucida, sombra
                           opaca y brillo; ver actores_capas.pintar_plantilla)
    1-9                    tono puesto a mano: se respeta
"""

TONOS_1 = {"luz": "1", "base": "2", "sombra": "3", "contorno": "4"}
TONOS_2 = {"luz": "6", "base": "7", "sombra": "8", "contorno": "9"}
PASA = set("zZw123456789")


def _region(filas, c):
    return {(y, x) for y, f in enumerate(filas) for x, ch in enumerate(f) if ch == c}


def sombrear(fila0, filas, textura=None, contorno_interior=False):
    """Silueta -> plantilla con tonos. Devuelve (fila0, filas).

    textura(y, x, tono, u, v) -> tono: vetas de pelo, anillas de malla...;
    se llama con coordenadas del frame (y absoluta, x relativa a x0), el
    tono que tocaria ('luz', 'base', 'sombra') y la posicion relativa (u, v)
    dentro de la caja de la region (0..1).
    contorno_interior: el borde entre material 1 y 2 tambien se contornea.
    """
    ancho = max((len(f) for f in filas), default=0)
    filas = [f.ljust(ancho, ".") for f in filas]
    lleno = {(y, x) for y, f in enumerate(filas) for x, ch in enumerate(f) if ch != "."}
    out = [list(f) for f in filas]
    for c, tonos in (("#", TONOS_1), ("+", TONOS_2)):
        reg = _region(filas, c)
        if not reg:
            continue
        ys = [y for y, _ in reg]
        xs = [x for _, x in reg]
        y0, y1, x0, x1 = min(ys), max(ys), min(xs), max(xs)
        for (y, x) in reg:
            vecinos = [(y + dy, x + dx) for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1))]
            fuera = [v for v in vecinos if v not in lleno]
            ajeno = [v for v in vecinos if v in lleno and v not in reg
                     and filas[v[0]][v[1]] in "#+"]
            if fuera or (contorno_interior and ajeno):
                out[y][x] = tonos["contorno"]
                continue
            u = (x - x0) / max(1, x1 - x0)
            v = (y - y0) / max(1, y1 - y0)
            luz = (1 - u) * 0.55 + (1 - v) * 0.45
            # franja de sombra junto al contorno de abajo y de la derecha
            borde_sombra = ((y + 1, x) not in reg) or ((y, x + 1) not in reg)
            if borde_sombra and luz < 0.75:
                tono = "sombra"
            elif luz > 0.72:
                tono = "luz"
            elif luz < 0.3:
                tono = "sombra"
            else:
                tono = "base"
            if textura is not None:
                tono = textura(fila0 + y, x, tono, u, v)
            out[y][x] = tonos[tono]
    return fila0, ["".join(f).rstrip(".") for f in out]


def espejo_mitad(mitades, eje=10):
    """Mitad izquierda (indices 0..eje) -> filas completas simetricas."""
    filas = []
    for m in mitades:
        if len(m) > eje + 1:
            raise ValueError(f"mitad demasiado larga ({len(m)} > {eje + 1}): {m!r}")
        m = m.ljust(eje + 1, ".")
        filas.append((m + m[:eje][::-1]).rstrip("."))
    return filas


def plantilla(siluetas, textura=None, contorno_interior=False):
    """{direccion: (fila0, filas_silueta)} -> {direccion: (fila0, filas)}."""
    return {d: sombrear(f0, fs, textura, contorno_interior) for d, (f0, fs) in siluetas.items()}
