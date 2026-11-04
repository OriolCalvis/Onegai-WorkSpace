"""Animaciones de los personajes, que se editan en la Forja de Onegai (pestaña Animaciones).

Oriol (23/10): «déjame escoger entre diferentes animaciones para cada raza + ropa y editar yo las
animaciones; esto lo querré con todas las que vendrán, así podré corregir y explicarte cómo lo quiero»,
y para editarlas, «mover partes del cuerpo».

Una animación es, en cada dirección, una lista de frames. Cada frame sale de uno de los 20 frames de
siempre (el «base»: andar, quieto o reposo) moviendo partes del cuerpo unos píxeles:

    {"nombre": "Atacar", "espejo": true,
     "frames": {"abajo": [{"b": 8, "ms": 120, "m": {"brazo_arma": [0, -3], "arma": [0, 0, -45]}}, ...],
                "derecha": [...], "arriba": [...], "izquierda": [...]}}

- «m» son los grupos que se mueven (GRUPOS): [dx, dy] en píxeles y, en «arma», «escudo» y «todo», un
  giro en grados. Lo que se mueve de cada capa (cuerpo, ropa, pelo...) lo dice el mapa de zonas de la
  talla: cada píxel va con la parte del cuerpo más cercana (la ropa sigue al brazo, a la pierna...);
  el pelo, la barba, el casco y los ojos van con la cabeza; la capa y las alas, con el torso; la cola,
  con la cadera. El arma va con la mano que la coge (actores_armas.MANO) y gira alrededor del puño.
- «espejo»: la izquierda sale de la derecha (los frames base 4-6 y 15-16 pasan a 10-12 y 19-20, y los
  dx y los giros cambian de signo).
- Al mover un brazo, el hueco que deja junto al torso se rellena (hasta dos píxeles) con el torso.

Las que vienen de serie están en ANIMACIONES; las de la Forja, en tools/ajustes_animaciones.json
(«animaciones»: {id: animación entera}), y las sustituyen. La Forja hace lo mismo en el navegador
(forja_plantilla.html, «figuraAnimada»); gen_animaciones.py hornea las hojas.
"""
import json
import math
from collections import deque
from pathlib import Path

import numpy as np

import actores_armas as armas
import actores_partes as partes

RUTA_AJUSTES = Path(__file__).resolve().with_name("ajustes_animaciones.json")
DIRS = ("abajo", "derecha", "arriba", "izquierda")

# los frames base de cada dirección (actores_partes): paso A, quieto, paso B, reposo 1 y 2
BASE = {
    "abajo": {"A": 7, "Q": 8, "B": 9, "R1": 17, "R2": 18},
    "derecha": {"A": 4, "Q": 5, "B": 6, "R1": 15, "R2": 16},
    "arriba": {"A": 1, "Q": 2, "B": 3, "R1": 13, "R2": 14},
    "izquierda": {"A": 10, "Q": 11, "B": 12, "R1": 19, "R2": 20},
}
ESPEJO_BASE = {4: 10, 5: 11, 6: 12, 15: 19, 16: 20}

ETIQUETAS = "hntpaAmMlLfF"
BRAZO = {"m": "a", "M": "A"}
# zona fija de las capas que no siguen el mapa de zonas
ZONA_FIJA = {"pelo": "h", "barba": "h", "cabeza": "h", "ojos": "h", "capa": "t", "alas": "t", "cola": "p"}
ARMAS = ("arma", "mano_izq")
# el orden en que se pintan las zonas al moverlas: de lo de atrás a lo de delante
ORDEN_ZONAS = {"lado": "amlftpnhLFAM", "frente": "tpnlfLFhamAM"}
RELLENO = 2  # píxeles de torso que se rellenan donde estaba un brazo


def grupos(direccion):
    """{grupo: etiquetas que mueve}. El brazo del arma depende de la dirección (actores_armas.MANO)."""
    ma = armas.MANO["arma"][direccion]
    mo = "M" if ma == "m" else "m"
    return {
        "tronco": "hntpaAmM", "cabeza": "h", "torso": "tn", "cadera": "p",
        "brazo_arma": BRAZO[ma] + ma, "mano_arma": ma, "brazo_otro": BRAZO[mo] + mo, "mano_otra": mo,
        "pierna_1": "lf", "pie_1": "f", "pierna_2": "LF", "pie_2": "F",
    }


GRUPOS = ["todo", "tronco", "cabeza", "torso", "cadera", "brazo_arma", "mano_arma", "brazo_otro", "mano_otra",
          "pierna_1", "pie_1", "pierna_2", "pie_2", "arma", "escudo"]


def _f(*frames):
    return list(frames)


def _anim(nombre, por_dir, espejo=True):
    """por_dir(d) -> lista de frames de la dirección d (abajo, derecha, arriba)."""
    frames = {d: por_dir(d) for d in ("abajo", "derecha", "arriba")}
    frames["izquierda"] = []
    return {"nombre": nombre, "espejo": espejo, "frames": frames}


def _fr(b, ms, **m):
    return {"b": b, "ms": ms, "m": {k: list(v) for k, v in m.items()}}


def _defecto():
    lado = lambda d: d == "derecha"  # noqa: E731
    return {
        "andar": _anim("Andar", lambda d: [_fr(BASE[d]["A"], 150), _fr(BASE[d]["Q"], 150), _fr(BASE[d]["B"], 150),
                                           _fr(BASE[d]["Q"], 150)]),
        "reposo": _anim("Quieto (respirar)", lambda d: [_fr(BASE[d]["R1"], 600), _fr(BASE[d]["R2"], 600)]),
        "correr": _anim("Correr", lambda d: [
            _fr(BASE[d]["A"], 90, tronco=(1 if lado(d) else 0, -1), brazo_arma=(-2 if lado(d) else 0, -1)),
            _fr(BASE[d]["Q"], 90, tronco=(1 if lado(d) else 0, 0)),
            _fr(BASE[d]["B"], 90, tronco=(1 if lado(d) else 0, -1), brazo_arma=(2 if lado(d) else 0, -1)),
            _fr(BASE[d]["Q"], 90, tronco=(1 if lado(d) else 0, 0))]),
        "atacar": _anim("Atacar (tajo)", lambda d: [
            _fr(BASE[d]["Q"], 120, tronco=(-1 if lado(d) else 0, 0), brazo_arma=(-1 if lado(d) else 0, -3), arma=(0, 0, -45)),
            _fr(BASE[d]["Q"], 70, tronco=(1 if lado(d) else 0, 0), brazo_arma=(3 if lado(d) else 1, 1), arma=(0, 0, 60)),
            _fr(BASE[d]["Q"], 110, tronco=(1 if lado(d) else 0, 1), brazo_arma=(2 if lado(d) else 1, 2), arma=(0, 0, 90)),
            _fr(BASE[d]["Q"], 160, brazo_arma=(1 if lado(d) else 0, 0), arma=(0, 0, 20))]),
        "conjurar": _anim("Lanzar un hechizo", lambda d: [
            _fr(BASE[d]["Q"], 200, brazo_arma=(0, -2), brazo_otro=(0, -2)),
            _fr(BASE[d]["Q"], 150, tronco=(0, -1), brazo_arma=(0, -4), brazo_otro=(0, -4)),
            _fr(BASE[d]["Q"], 300, brazo_arma=(3 if lado(d) else 1, -2), brazo_otro=(3 if lado(d) else -1, -2)),
            _fr(BASE[d]["Q"], 200, brazo_arma=(0, -1), brazo_otro=(0, -1))]),
        "golpe": _anim("Recibir un golpe", lambda d: [
            _fr(BASE[d]["Q"], 80, todo=(-2 if lado(d) else 0, 0 if lado(d) else -1), cabeza=(-1 if lado(d) else 0, 0)),
            _fr(BASE[d]["Q"], 120, todo=(-1 if lado(d) else 0, 0)),
            _fr(BASE[d]["Q"], 150)]),
        "morir": _anim("Morir", lambda d: [
            _fr(BASE[d]["Q"], 100, todo=(-1 if lado(d) else 0, 0), cabeza=(-1 if lado(d) else 0, 1)),
            _fr(BASE[d]["Q"], 120, todo=(4, -1, -25)),
            _fr(BASE[d]["Q"], 140, todo=(9, -4, -55)),
            _fr(BASE[d]["Q"], 600, todo=(16, -8, -90))]),
        "sentarse": _anim("Sentarse", lambda d: [
            _fr(BASE[d]["Q"], 200, tronco=(-1 if lado(d) else 0, 2)),
            _fr(BASE[d]["Q"], 900, tronco=(-1 if lado(d) else 0, 4), pierna_2=(2 if lado(d) else 0, -1))]),
        "hablar": _anim("Hablar", lambda d: [
            _fr(BASE[d]["R1"], 200), _fr(BASE[d]["R1"], 200, cabeza=(0, 1)),
            _fr(BASE[d]["R1"], 220, brazo_otro=(1, -2)), _fr(BASE[d]["R1"], 220, brazo_otro=(1, -1), cabeza=(0, 1))]),
    }


ANIMACIONES = _defecto()


def ajustes():
    return json.loads(RUTA_AJUSTES.read_text(encoding="utf-8")) if RUTA_AJUSTES.exists() else {}


def todas():
    """Las de serie con las de la Forja encima."""
    out = dict(ANIMACIONES)
    out.update(ajustes().get("animaciones", {}))
    return out


def frames_de(anim, direccion):
    """Los frames de una dirección, con el espejo de la derecha si toca."""
    if direccion == "izquierda" and anim.get("espejo"):
        out = []
        for fr in anim["frames"].get("derecha", []):
            m = {}
            for g, v in fr.get("m", {}).items():
                v = list(v)
                v[0] = -v[0]
                if len(v) > 2:
                    v[2] = -v[2]
                m[g] = v
            out.append({"b": ESPEJO_BASE.get(fr["b"], fr["b"]), "ms": fr.get("ms", 120), "m": m})
        return out
    return anim["frames"].get(direccion, [])


# --- el mapa de zonas ---------------------------------------------------------------------

def zonas(lab):
    """Cada píxel del frame, con la etiqueta de la parte del cuerpo más cercana (por pasos de 8 vecinos,
    empezando por las etiquetas en el orden de lectura): lo que lleva encima va con esa parte."""
    alto, ancho = lab.shape
    z = np.where(np.isin(lab, list(ETIQUETAS)), lab, "")
    cola = deque((y, x) for y in range(alto) for x in range(ancho) if z[y, x])
    vecinos = ((-1, 0), (0, -1), (0, 1), (1, 0), (-1, -1), (-1, 1), (1, -1), (1, 1))
    while cola:
        y, x = cola.popleft()
        for dy, dx in vecinos:
            ny, nx = y + dy, x + dx
            if 0 <= ny < alto and 0 <= nx < ancho and not z[ny, nx]:
                z[ny, nx] = z[y, x]
                cola.append((ny, nx))
    return z


def zonas_texto(lab):
    return "".join(c or "." for c in zonas(lab).ravel())


def base_forja(tallas):
    """Lo que la Forja necesita de cada frame base para animar: el mapa de zonas, los puños y el pie."""
    out = {}
    for tk, t in tallas.items():
        out[tk] = {}
        for f in range(1, partes.N_TOTAL + 1):
            lab = t.labs[f]
            ys = np.nonzero(np.isin(lab, list(ETIQUETAS)).any(axis=1))[0]
            out[tk][str(f)] = {"z": zonas_texto(lab), "m": ancla(lab, "m"), "M": ancla(lab, "M"),
                               "pie": int(ys.max()) if len(ys) else t.alto - 1}
    return out


# --- mover una capa -------------------------------------------------------------------------

def _redondo(v):
    return int(math.floor(v + 0.5))


def desplazamientos(m, direccion):
    """{etiqueta: (dx, dy)} de las partes del cuerpo, sumando los grupos (y «todo»)."""
    g = grupos(direccion)
    todo = m.get("todo", [0, 0])
    off = {e: [todo[0], todo[1]] for e in ETIQUETAS}
    for nombre, v in m.items():
        for e in g.get(nombre, ""):
            off[e][0] += v[0]
            off[e][1] += v[1]
    return off


def ancla(lab, mano):
    """El puño de la mano (actores_armas._ancla), redondeado igual que en la Forja."""
    otra = "M" if mano == "m" else "m"
    ys, xs = np.nonzero(lab == mano)
    if not len(ys):
        ys, xs = np.nonzero(lab == otra)
    if not len(ys):
        return None
    return _redondo((int(xs.min()) + int(xs.max())) / 2), int(ys.max()) - 2


def girar(px, cx, cy, grados):
    """La capa girada alrededor de (cx, cy), al píxel más cercano (como en la Forja)."""
    if not grados:
        return px
    a = math.radians(grados)
    c, s = math.cos(a), math.sin(a)
    alto, ancho = px.shape[:2]
    ys, xs = np.mgrid[0:alto, 0:ancho]
    dx, dy = (xs - cx).astype(float), (ys - cy).astype(float)
    sx = np.floor(cx + c * dx + s * dy + 0.5).astype(int)
    sy = np.floor(cy - s * dx + c * dy + 0.5).astype(int)
    dentro = (sx >= 0) & (sx < ancho) & (sy >= 0) & (sy < alto)
    out = np.zeros_like(px)
    out[dentro] = px[sy[dentro], sx[dentro]]
    return out


def mover_capa(px, zona, nombre, lab, direccion, m):
    """La capa (64x64 RGBA) de un frame base, con las partes movidas según m."""
    alto, ancho = px.shape[:2]
    off = desplazamientos(m, direccion)
    todo = m.get("todo", [0, 0, 0])
    out = np.zeros_like(px)
    if nombre in ARMAS:
        grupo = "arma" if nombre == "arma" else "escudo"
        mano = armas.MANO[nombre][direccion]
        g = m.get(grupo, [0, 0, 0])
        dx, dy = g[0] + off[mano][0], g[1] + off[mano][1]
        ys, xs = np.nonzero(px[..., 3] > 0)
        for y, x in zip(ys, xs):
            X, Y = x + dx, y + dy
            if 0 <= X < ancho and 0 <= Y < alto:
                out[Y, X] = px[y, x]
        giro = g[2] if len(g) > 2 else 0
        p = ancla(lab, mano)
        if giro and p is not None:
            out = girar(out, p[0] + dx, p[1] + dy, giro)
    else:
        fija = ZONA_FIJA.get(nombre)
        zonas_px = np.full((alto, ancho), fija) if fija else zona
        zout = np.full((alto, ancho), "", dtype="<U1")
        orden = ORDEN_ZONAS["lado" if direccion in ("derecha", "izquierda") else "frente"]
        opaco = px[..., 3] > 0
        for e in orden:
            dx, dy = off[e]
            ys, xs = np.nonzero(opaco & (zonas_px == e))
            for y, x in zip(ys, xs):
                X, Y = x + dx, y + dy
                if 0 <= X < ancho and 0 <= Y < alto:
                    out[Y, X] = px[y, x]
                    zout[Y, X] = e
        # el hueco que deja un brazo junto al torso: el torso se alarga hasta RELLENO píxeles
        hueco = opaco & np.isin(zonas_px, list("aAmM")) & (out[..., 3] == 0)
        for e in "aAmM":
            if off[e] == [off["t"][0], off["t"][1]]:
                hueco &= zonas_px != e
        for y in range(alto):
            x = 0
            while x < ancho:
                if not hueco[y, x]:
                    x += 1
                    continue
                x0 = x
                while x < ancho and hueco[y, x]:
                    x += 1
                x1 = x - 1
                for lado, borde, paso in ((-1, x0 - 1, -1), (1, x1 + 1, 1)):
                    if not (0 <= borde < ancho) or zout[y, borde] not in ("t", "p"):
                        continue
                    dentro = borde + paso
                    interior = out[y, dentro] if 0 <= dentro < ancho and zout[y, dentro] in ("t", "p") else out[y, borde]
                    run = list(range(x0, x1 + 1))
                    if lado == 1:  # el torso está a la derecha: se rellena desde x1 hacia la izquierda
                        run = run[::-1]
                    run = run[:RELLENO]
                    for i, xx in enumerate(run):
                        out[y, xx] = out[y, borde] if i == len(run) - 1 else interior
                        zout[y, xx] = "t"
                    break
    if len(todo) > 2 and todo[2]:
        ys = np.nonzero(np.isin(lab, list(ETIQUETAS)).any(axis=1))[0]
        cy = int(ys.max()) if len(ys) else alto - 1
        out = girar(out, 32 + todo[0], cy + todo[1], todo[2])
    return out


def frame_animado(capas, talla, fr, direccion):
    """capas: [(orden, frames, nombre)] de actores_personajes.capas_nombradas. El frame compuesto (RGBA)."""
    from PIL import Image
    from actores_capas import ORDEN
    b = fr["b"]
    lab = talla.labs[b]
    zona = zonas(lab)
    d = partes.DIRECCION[b]
    pila = []
    for i, (orden, frames, nombre) in enumerate(capas):
        z = ORDEN[orden][d] if isinstance(orden, str) else orden.get(f"f{b}", orden[d])
        pila.append((z, i, mover_capa(frames[b - 1], zona, nombre, lab, direccion, fr.get("m", {}))))
    base = Image.new("RGBA", (talla.ancho, talla.alto), (0, 0, 0, 0))
    for _z, _i, px in sorted(pila, key=lambda t: (t[0], t[1])):
        base.alpha_composite(Image.fromarray(px, "RGBA"))
    return np.array(base)


def animacion(spec, tallas, anim):
    """{dirección: [(frame RGBA, ms)]} de un personaje."""
    import actores_personajes as pers
    import actores_razas as razas
    raza = razas.POR_ID[spec["raza"]]
    talla = tallas[spec.get("talla", raza["talla"])]
    capas = pers.capas_nombradas(spec, tallas)
    return {d: [(frame_animado(capas, talla, fr, d), fr.get("ms", 120)) for fr in frames_de(anim, d)] for d in DIRS}
