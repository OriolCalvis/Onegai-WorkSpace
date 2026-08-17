#!/usr/bin/env python3
"""Devuelve a las 10 cartas de deidad su identidad real.

EL PROBLEMA. Las 10 cartas de data/cartas/deidades llevan pegados nombres de
Creadores de Egaroth (Chronos, Sofia, Vida, Muerte...) sobre contenidos que no
tienen nada que ver: la carta llamada "Chronos", Guardian del Tiempo, tiene
dominio "forja, artesania y juramentos" y diez hechizos con prefijo forja_.
NINGUNA de las diez tiene un dominio que corresponda a su nombre.

LA PRUEBA. Los 12 trasfondos (los meses de nacimiento) referencian 17 deidades
por id, y NINGUNO de esos ids existe como carta. Al cruzar dominio + prefijo de
hechizo con el trasfondo que la invoca, las diez casan 1 a 1 con deidades reales
del canon. Los pares lo confirman: el Mes del Hierro invoca a la_forja_del_trueno
Y a deity_forge_father, que son justamente las dos cartas de trueno_ y forja_.

QUE HACE. Renombra id, name y fichero. No toca dominio, hechizos, favor,
obligaciones ni compatibilidades: ese contenido siempre fue correcto, lo que
estaba mal era la etiqueta.

Los Creadores (Egaroth, Chronos, Gurkazaal...) son otro nivel de divinidad y
siguen sin existir como carta. Eso es contenido nuevo, no un renombrado, y no
lo hace este script.
"""
import json, os, sys

AQUI = os.path.dirname(os.path.abspath(__file__))
DIR = os.path.join(AQUI, "..", "..", "dndWeebCC-master", "data", "cartas", "deidades")

# fichero_actual -> (id_real, nombre_real, evidencia)
MAPA = {
    "death.json":     ("deity_wolf_king",          "El Rey Lobo",
                       "dominio 'invierno, muerte y manada' + lobo_* · Mes de los Aullidos"),
    "egos.json":      ("el_caminante_gris",        "El Caminante Gris",
                       "dominio 'caminos, fronteras y despedidas' + gris_* · Mes de la Escarcha"),
    "envidia.json":   ("la_espiga_dorada",         "La Espiga Dorada",
                       "dominio 'cosecha, abundancia y paciencia' + dorada_* · Cosecha, Siembra, Tierra"),
    "eros.json":      ("el_oraculo_fragmentado",   "El Oraculo Fragmentado",
                       "dominio 'conocimiento, secretos y memoria' + bibliotecario_* · Estrellas, Oraculo"),
    "knowledge.json": ("deity_veiled_queen",       "La Reina Velada",
                       "dominio 'velos, ilusion y umbrales' + velada_* · Mes de las Sombras"),
    "life.json":      ("deity_wild_court",         "La Corte Salvaje",
                       "dominio 'caza, espesura y lo indomito' + salvaje_* · Escarcha, Sol"),
    "morpheo.json":   ("la_forja_del_trueno",      "La Forja del Trueno",
                       "dominio 'tormenta, cambio y voz alzada' + trueno_* · Mes del Hierro"),
    "sastre.json":    ("la_tejedora_de_estrellas", "La Tejedora de Estrellas",
                       "dominio 'destino, hilos y constelaciones' + estrellas_* · Mes de las Estrellas"),
    "time.json":      ("deity_forge_father",       "El Padre de la Forja",
                       "dominio 'forja, artesania y juramentos' + forja_* · Mes del Hierro"),
    "war.json":       ("la_dama_del_farol_rojo",   "La Dama del Farol Rojo",
                       "dominio 'noche urbana y proteccion de los vulnerables' + rojo_* · Mes de las Sombras"),
}


# Ficheros que referencian deidades por su id viejo y hay que actualizar tambien,
# o quedan apuntando al vacio. (Detectado al validar: geografia.json daba a
# Bastrea las deidades 'life' y 'knowledge'.)
REFERENTES = [
    (os.path.join("mapa", "geografia.json"), ("naciones", "deidadIds")),
]
VIEJO_A_NUEVO = {viejo: nid for viejo, (nid, _n, _e) in
                 {"death": MAPA["death.json"], "self": MAPA["egos.json"],
                  "envy": MAPA["envidia.json"], "eros": MAPA["eros.json"],
                  "knowledge": MAPA["knowledge.json"], "life": MAPA["life.json"],
                  "morpheo": MAPA["morpheo.json"], "deitrok": MAPA["sastre.json"],
                  "time": MAPA["time.json"], "war": MAPA["war.json"]}.items()}


def actualiza_referencias():
    """Reescribe las referencias por id viejo que hay fuera de las cartas."""
    raiz = os.path.join(DIR, "..", "..")
    hechos = []
    for rel, (coleccion, campo) in REFERENTES:
        ruta = os.path.join(raiz, rel)
        if not os.path.exists(ruta):
            continue
        d = json.load(open(ruta, encoding="utf-8"))
        toco = False
        for item in d.get(coleccion, []):
            ids = item.get(campo) or []
            nuevos = [VIEJO_A_NUEVO.get(i, i) for i in ids]
            if nuevos != ids:
                item[campo] = nuevos
                hechos.append((rel, item.get("nombre", "?"), ids, nuevos))
                toco = True
        if toco:
            json.dump(d, open(ruta, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
    return hechos


def main():
    if not os.path.isdir(DIR):
        sys.exit(f"no existe {DIR}")
    hechos = []
    for fichero, (nid, nombre, evid) in MAPA.items():
        ruta = os.path.join(DIR, fichero)
        if not os.path.exists(ruta):
            continue                                    # ya renombrado
        d = json.load(open(ruta, encoding="utf-8"))
        antes = (d.get("id"), d.get("name"))
        d["id"], d["name"] = nid, nombre
        d["rank"] = "deidad_local"                      # frente a los Creadores
        d["_correccion"] = {
            "identidad_anterior": f"{antes[0]} / {antes[1]}",
            "motivo": "El nombre de Creador estaba pegado sobre contenido ajeno.",
            "evidencia": evid,
            "aplicado_por": "Onegai-Core/bridge/restaurar_identidad_deidades.py",
        }
        json.dump(d, open(ruta, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
        destino = os.path.join(DIR, nid + ".json")
        if os.path.abspath(destino) != os.path.abspath(ruta):
            os.rename(ruta, destino)      # rename, no unlink: hay mounts que
                                          # permiten renombrar pero no borrar
        hechos.append((fichero, antes, nid, nombre))
    for f, antes, nid, nombre in hechos:
        print(f"  {f:16} {antes[1]:10} -> {nombre:26} ({nid})")
    refs = actualiza_referencias()
    for rel, quien, antes, despues in refs:
        print(f"  ref      {rel} · {quien}: {antes} -> {despues}")
    if hechos or refs:
        print(f"\n{len(hechos)} cartas devueltas a su identidad real, "
              f"{len(refs)} referencia(s) actualizada(s)")
    else:
        print("nada que hacer: ya estaban corregidas")


if __name__ == "__main__":
    main()
