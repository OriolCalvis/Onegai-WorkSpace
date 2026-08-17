#!/usr/bin/env python3
"""Segunda ola de equipo: completa los 300 equipamientos normales (sin contar artefactos).

Rellena exactamente los huecos detectados en el catálogo:
  - torso/cabeza/piernas/pies y arma_secundaria en rarezas uncommon/rare/epic (108 piezas)
  - 19 armas con nombre propio (rare/epic únicas, no de aspecto)
  - 12 habilidades de objeto (cartas skill reales para linkedSkill — 1 de cada ~4 piezas la lleva)
  - 30 consumibles (10 conceptos × 3 grados), varios curan condiciones reales

Presupuesto de bonos (Plantilla §1): uncommon ≤2 · rare ≤3 · epic ≤4. Tiers: 1/2/3.
Nunca sobrescribe ids existentes.
"""

from __future__ import annotations

import json
import re
import unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ARMAS = ROOT / "data/cartas/armas"
HABS = ROOT / "data/cartas/habilidades"
CONS = ROOT / "data/cartas/consumibles"


def slugify(v: str) -> str:
    v = "".join(c for c in unicodedata.normalize("NFKD", v) if not unicodedata.combining(c)).lower()
    return re.sub(r"_+", "_", re.sub(r"[^a-z0-9]+", "_", v)).strip("_")


def escribir(path: Path, payload: dict) -> bool:
    if path.exists():
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    return True


# ---- 12 habilidades de objeto (para linkedSkill) ----
HABILIDADES_OBJETO = [
("golpe_de_pomo", "Golpe de Pomo", "accion_menor", "melee", "Golpe seco con el pomo: 1 de daño y el objetivo pierde 0,5 éxitos en su próxima tirada.", "Un final de discusión portátil."),
("parada_firme", "Parada Firme", "reaccion", "self", "Bloqueas con la pieza: reduces el daño de un golpe en 2.", "El acero opina mejor que tú bajo presión."),
("destello_del_engaste", "Destello del Engaste", "accion_menor", "short", "La gema engastada destella: un enemigo adyacente queda Cegado hasta el final de su turno si falla Resistencia física.", "La joyería también sabe defenderse."),
("paso_amortiguado", "Paso Amortiguado", "accion_menor", "self", "Las suelas se ablandan: tu próximo movimiento no provoca reacciones.", "Nadie oye llegar al calzado caro."),
("cierre_de_yelmo", "Cierre de Yelmo", "reaccion", "self", "Bajas la visera a tiempo: anulas un efecto de Cegado o reduces 1 el daño de un crítico.", "Clac. El mundo, más pequeño y más seguro."),
("empuje_de_rodela", "Empuje de Rodela", "accion_menor", "melee", "Empellón con el borde: empuja 5 pies a un enemigo adyacente.", "El borde también es escudo."),
("bolsillo_secreto", "Bolsillo Secreto", "accion_menor", "self", "Escondes o sacas un objeto pequeño sin que nadie lo vea (ni con registro casual).", "El sastre cobró el doble. Lo valía."),
("temple_recordado", "Temple Recordado", "accion_menor", "self", "La pieza recuerda su mejor día: +1 a tu próxima tirada con ella.", "El metal viejo tiene memoria selectiva."),
("agarre_de_trepador", "Agarre de Trepador", "accion_menor", "self", "Garfios finos en las palmas: trepas este turno a velocidad normal sin prueba.", "La pared es una opinión."),
("capucha_de_nadie", "Capucha de Nadie", "accion_menor", "self", "Te calas la capucha: +1 a tu próxima prueba de sigilo o para pasar desapercibido.", "Ser nadie es un oficio."),
("brindis_de_valor", "Brindis de Valor", "accion_menor", "short", "Alzas la pieza: un aliado que te vea gana +0,5 a su próxima tirada contra el miedo.", "Salud, y que el miedo pague la ronda."),
("anclaje_de_bota", "Anclaje de Bota", "reaccion", "self", "Clavas los tacos: ignoras un empuje o derribo.", "El suelo y ella llegaron a un acuerdo."),
]

# ---- piezas por slot: (nombre base, bonos_uncommon, tags, flavor) — se escala a rare/epic ----
TORSO = [
("Gambesón del Veterano", {"health": 1, "CON": 1}, ["Armadura Ligera"], "Cosido con hilo de más guerras que su dueño."),
("Coraza de Escamas de Río", {"armor": 1, "DES": 1}, ["Armadura Media"], "Las nagas venden caro y curten mejor."),
("Brigantina del Gremio", {"armor": 1, "CON": 1}, ["Armadura Media"], "Remaches con sello: garantía de por vida. La suya."),
("Túnica del Archivista", {"INT": 2}, ["Armadura Ligera", "Arcano"], "Los bolsillos ordenados alfabéticamente."),
("Sobrevesta del Peregrino", {"CAR": 1, "health": 1}, ["Armadura Ligera", "Sagrado"], "Doce parches, doce meses, un camino."),
("Peto de Caldera", {"armor": 2}, ["Armadura Pesada"], "Antes hervía sopa. Ahora hierve amenazas."),
("Chaleco de Cazatormentas", {"DES": 1, "health": 1}, ["Armadura Ligera"], "Huele a ozono y a apuestas ganadas."),
("Cota del Sepulturero", {"CON": 1, "armor": 1}, ["Armadura Media", "Oscuro"], "Los muertos respetan al que viste de faena."),
]
CABEZA = [
("Yelmo del Alba", {"armor": 1, "CAR": 1}, ["Sagrado"], "El primer rayo del día siempre rebota aquí."),
("Capucha del Contrabandista", {"DES": 1, "INT": 1}, ["Sigilo"], "Cosida para mirar sin ser mirado."),
("Diadema de Lectora", {"INT": 2}, ["Arcano"], "Aprieta justo donde nacen las ideas."),
("Casco de Abordaje", {"CON": 1, "armor": 1}, [], "Abolladuras de tres mares distintos."),
("Máscara del Duelista", {"CAR": 1, "DES": 1}, [], "La sonrisa pintada nunca pierde."),
("Corona de Ramas Secas", {"CAR": 2}, ["Naturaleza"], "El bosque también nombra reyes de paja."),
("Antifaz de la Vigilia", {"INT": 1, "DES": 1}, ["Sigilo"], "Dos agujeros y ninguna excusa."),
("Bacinete del Miliciano", {"armor": 2}, [], "Reglamentario, feo y agradecido."),
]
PIERNAS = [
("Grebas del Corredor de Muros", {"DES": 2}, ["Sigilo"], "Las rodillas nuevas del oficio viejo."),
("Quijotes del Baluarte", {"armor": 2}, ["Armadura Pesada"], "Piernas de asedio para gente de asedio."),
("Perneras del Vadeador", {"CON": 1, "DES": 1}, [], "Secas por dentro, legendarias por fuera."),
("Faldón del Oficiante", {"CAR": 2}, ["Sagrado"], "Pliegues que saben estar en un funeral."),
("Calzas del Ratero Fino", {"DES": 1, "INT": 1}, ["Sigilo"], "Silenciosas hasta en escaleras de madera."),
("Grebas Rúnicas Menores", {"INT": 1, "armor": 1}, ["Arcano"], "Las runas brillan cuando conviene correr."),
]
PIES = [
("Botas del Correo Real", {"DES": 2}, [], "Llevan cartas y se llevan récords."),
("Escarpes del Rompefilas", {"CON": 1, "armor": 1}, ["Armadura Pesada"], "Pisan como argumento final."),
("Sandalias del Vado Sagrado", {"CAR": 1, "DES": 1}, ["Sagrado"], "Cruzaron el río bendito y no se enteraron."),
("Botines del Tejadista", {"DES": 1, "INT": 1}, ["Sigilo"], "Suela de fieltro, alma de gato."),
("Zuecos del Alquimista", {"INT": 1, "CON": 1}, [], "Inmunes a ácidos menores y a la vergüenza."),
("Botas de la Última Guardia", {"CON": 2}, [], "Aguantaron la noche entera. Y la siguiente."),
]
SECUNDARIA = [
("Escudo del Faro", {"armor": 1, "CAR": 1}, ["Escudo", "Bloqueo"], "Pintado con la luz que salvó a su dueño."),
("Rodela del Duelista Zurdo", {"armor": 1, "DES": 1}, ["Escudo"], "Pequeña, insolente y siempre a tiempo."),
("Pavés del Asedio", {"armor": 2}, ["Escudo", "Bloqueo"], "Una pared con asa."),
("Foco de Cuarzo Ahumado", {"INT": 2}, ["Arcano"], "Ve el maná como otros ven el humo."),
("Símbolo del Alba Menor", {"CAR": 2}, ["Sagrado"], "Latón barato, fe cara."),
("Talismán del Mes Propio", {"CAR": 1, "health": 1}, ["Sagrado"], "Cada uno lleva grabado su mes. Este acierta siempre."),
("Grimorio de Bolsillo", {"INT": 1, "CAR": 1}, ["Arcano"], "Cabe una biblioteca si la letra es humilde."),
("Broquel de Taberna", {"armor": 1, "CON": 1}, ["Escudo"], "Ha parado sillas. Las espadas le parecen educadas."),
]
# ---- 19 armas con nombre propio (rare/epic, arma_principal) ----
ARMAS_NOMBRE = [
("Colmillo de la Primera Nevada", "ligera", "DES", "rare", "Daga que nunca se calienta: sus Sangrados no se curan con descanso corto en invierno.", ["Sigilo"]),
("Réplica del Juicio", "media", "CON", "rare", "Espada de juez: +1 contra quien haya roto un juramento esta aventura.", []),
("Susurradora", "ligera", "DES", "rare", "Ballesta silenciosa: disparar no rompe tu sigilo.", ["Proyectil", "Sigilo"]),
("Mazo del Deshaucio", "pesada", "CON", "rare", "Contra puertas, muros y escudos cuenta como éxito crítico al romper.", []),
("Lanza del Vado", "media", "CON", "rare", "En agua o lluvia, su alcance aumenta 5 pies.", []),
("Arco del Censo", "media", "DES", "rare", "Marca: su primer impacto por combate deja Marcado al objetivo.", ["Proyectil"]),
("Vara del Interés Compuesto", "ligera", "INT", "rare", "Foco usurero: +1 contra objetivos Endeudados.", ["Arcano"]),
("Hacha de la Cosecha Tardía", "pesada", "CON", "rare", "Contra objetivos bajo media vida suma +2 (la siega no espera).", []),
("Estoque del Perdón", "ligera", "DES", "rare", "Puede declarar daño no letal sin penalización: deja a 1 de vida en vez de Moribundo.", []),
("Cetro de la Audiencia", "ligera", "CAR", "rare", "Foco divino: tus curas a alcance corto curan 1 extra si el aliado puede oírte.", ["Sagrado"]),
("Martillo de Fe de Erratas", "media", "CON", "epic", "Una vez por combate, repite una tirada de ataque fallida con él (corrige la errata).", ["Sagrado"]),
("La Impagable", "ligera", "DES", "epic", "Daga de la Serpiente: sus críticos aplican Endeudado sin tirada.", ["Oscuro"]),
("Espadón del Puente Roto", "pesada", "CON", "epic", "Sus empujes lanzan 10 pies; contra enemigos en bordes o puentes, ventaja.", []),
("Arco de la Lista Corta", "media", "DES", "epic", "Elige un nombre al alba: +2 contra ese enemigo hasta el siguiente alba.", ["Proyectil"]),
("Báculo del Coro Hueco", "media", "INT", "epic", "Foco lunar: tus cartas de sueño y encantamiento ganan +1.", ["Arcano", "Oscuro"]),
("Guadaña del Barbecho", "pesada", "CON", "epic", "Los enemigos que abate no pueden ser reanimados como no-muertos.", ["Naturaleza"]),
("Puñal del Doceavo Mes", "ligera", "DES", "epic", "En el mes de nacimiento de su portador, todos sus críticos suman +2.", []),
("Tridente de la Marea Firme", "media", "CON", "epic", "Sus Agarrados no pueden liberarse con empujes externos (la marea sujeta).", []),
("El Argumento Final", "pesada", "CON", "epic", "Una vez por combate, tras impactar, el objetivo pierde su reacción y su próxima acción menor.", []),
]

RAREZAS = [("uncommon", 1), ("rare", 2), ("epic", 3)]
SUFIJO = {"uncommon": "", "rare": " Insigne", "epic": " Sublime"}

CONSUMIBLES = [
("Poción de Curación", "accion_menor", "Recuperas 2 de vida.", "Recuperas 4 de vida.", "Recuperas 6 de vida y limpias 1 nivel de Fatiga.", "Dulce por fuera, milagro por dentro."),
("Antídoto de Vial Verde", "accion_menor", "Limpia la condición Envenenado.", "Limpia Envenenado y da resistencia al veneno 1 escena.", "Limpia Envenenado a todos los aliados adyacentes.", "Amargo como la lección que lo hizo necesario."),
("Vendas del Campamento", "accion", "Detienen un Sangrado.", "Detienen Sangrado y curan 1.", "Detienen Sangrado, curan 2 y estabilizan a un Moribundo.", "Lino honesto y nudos aprendidos a la fuerza."),
("Bomba de Humo Gris", "accion_menor", "Humo en área mínima: los disparos a través fallan con 0,5 menos.", "Área corta y además rompe la línea de visión.", "Área corta 2 rondas; salir de ella permite Ocultarse gratis.", "La salida de emergencia embotellada."),
("Aceite de Filo", "accion_menor", "Tu arma suma +1 al daño durante 2 impactos.", "+1 durante todo el combate.", "+2 durante el combate y los críticos aplican Sangrado.", "El desayuno favorito del acero."),
("Sales del Despertar", "accion", "Despierta a un Inconsciente (sueño no mágico).", "Despierta incluso de sueño mágico menor.", "Despierta y el objetivo gana +1 a su próxima tirada.", "Huelen a bofetada respetuosa."),
("Petardo de Feria", "accion_menor", "Ruido: rompe el sigilo enemigo a 15 pies.", "Además, un enemigo pierde su reacción si falla Resistencia física.", "Además, área mínima queda Cegada 1 turno si fallan.", "Diversión garantizada. Dirección de la diversión, no."),
("Ración Templada de Viaje", "accion_menor", "Fuera de combate: cura 1 y quita el hambre del día.", "Cura 2 y limpia 1 de Fatiga.", "Cura 3, limpia 1 de Fatiga y da +0,5 a la primera tirada del día siguiente.", "Sabe a hogar alquilado, pero sabe."),
("Tinta de Revelación", "accion", "Arrojada: revela huellas y marcas ocultas en área mínima.", "Además vuelve visibles 1 ronda a los Ocultos del área.", "Además las ilusiones del área se deshacen.", "La verdad, en frasco de 30 mililitros."),
("Amuleto de un Solo Uso", "reaccion", "Anula 1 punto de daño de un golpe.", "Anula 2 puntos.", "Anula 3 puntos o convierte un crítico recibido en golpe normal.", "Baratija hasta el instante exacto en que no lo es."),
]
GRADOS = [("common", 1, ""), ("uncommon", 2, " Mayor"), ("rare", 3, " Suprema")]


def pieza(nombre, slot, bonos, tags, flavor, rareza, tier, peso, linked=None, extra_restr=None):
    stat = {"CON": 0, "DES": 0, "CAR": 0, "INT": 0, "health": 0, "armor": 0}
    stat.update(bonos)
    restr = list(extra_restr or [])
    return {"id": slugify(nombre), "name": nombre, "type": "equipment", "slot": slot,
            "tier": tier, "rarity": rareza, "statBonuses": stat,
            "penalties": {"CON": 0, "DES": 0, "CAR": 0, "INT": 0},
            "grantedTags": tags, "linkedSkill": linked,
            "requiredStats": {"CON": 0, "DES": 0, "CAR": 0, "INT": 0},
            "restrictions": restr, "flavorText": flavor, "weightCategory": peso}


def main() -> None:
    n_hab = n_eq = n_con = 0

    # habilidades de objeto
    for hid, nombre, accion, rng, desc, flv in HABILIDADES_OBJETO:
        carta = {"id": hid, "name": nombre, "type": "skill", "tier": 1, "rarity": "common",
                 "classTags": [], "roleTags": [], "mechanicTags": ["Objeto"],
                 "requiredStats": {}, "requiredTags": [], "incompatibleTags": [],
                 "cost": {"resource": "none", "amount": 0},
                 "recovery": "descanso_corto", "actionType": accion, "range": rng,
                 "duration": "instant", "defenseStat": None,
                 "effect": {"description": desc + " (Habilidad vinculada: solo con su objeto equipado.)", "scaling": "none"},
                 "limitations": ["Requiere el objeto que la otorga equipado"],
                 "upgradePath": [], "evolvesInto": None, "flavorText": flv}
        n_hab += escribir(HABS / f"{hid}.json", carta)

    linked_ids = [h[0] for h in HABILIDADES_OBJETO]
    contador = 0

    def lote(bases, slot, peso_por_tags):
        nonlocal n_eq, contador
        for nombre, bonos, tags, flavor in bases:
            for rareza, tier in RAREZAS:
                contador += 1
                escala = {k: (v + (1 if rareza == "epic" and k != "armor" else 0)) for k, v in bonos.items()}
                if rareza == "rare":
                    escala["health"] = escala.get("health", 0) + 1
                linked = linked_ids[contador % len(linked_ids)] if contador % 4 == 0 else None
                peso = peso_por_tags(tags)
                restr = ["Solo un objeto épico/mítico equipado a la vez (GDD sección 3)"] if rareza == "epic" else []
                n_eq += escribir(ARMAS / f"{slugify(nombre + SUFIJO[rareza])}.json",
                                 pieza(nombre + SUFIJO[rareza], slot, escala, tags, flavor, rareza, tier, peso, linked, restr))

    peso_arm = lambda tags: "pesada" if "Armadura Pesada" in tags else ("media" if "Armadura Media" in tags or "Escudo" in tags else "ligera")
    lote(TORSO, "torso", peso_arm)
    lote(CABEZA, "cabeza", peso_arm)
    lote(PIERNAS, "piernas", peso_arm)
    lote(PIES, "pies", peso_arm)
    lote(SECUNDARIA, "arma_secundaria", peso_arm)

    for nombre, peso, precision, rareza, regla, tags in ARMAS_NOMBRE:
        tier = 2 if rareza == "rare" else 3
        bonos = {"DES" if precision == "DES" else ("INT" if precision == "INT" else ("CAR" if precision == "CAR" else "CON")): 2 if rareza == "rare" else 3}
        restr = [f"Precisión: {precision}", f"Regla propia: {regla}"]
        if rareza == "epic":
            restr.append("Solo un objeto épico/mítico equipado a la vez (GDD sección 3)")
        etiqueta = "Arma Ligera" if peso == "ligera" else ("Arma Media" if peso == "media" else "Arma Pesada")
        n_eq += escribir(ARMAS / f"{slugify(nombre)}.json",
                         pieza(nombre, "arma_principal", bonos, [etiqueta] + tags,
                               "Un arma con nombre se gana el nombre.", rareza, tier, peso, None, restr))

    # consumibles: 10 conceptos × 3 grados
    for nombre, accion, e1, e2, e3, flv in CONSUMIBLES:
        for (rareza, tier, suf), desc in zip(GRADOS, (e1, e2, e3)):
            carta = {"id": slugify(nombre + suf), "name": nombre + suf, "type": "consumable",
                     "tier": tier, "rarity": rareza, "actionType": accion,
                     "effect": {"description": desc}, "uses": 1, "flavorText": flv}
            n_con += escribir(CONS / f"{carta['id']}.json", carta)

    print(f"Habilidades de objeto: {n_hab} · piezas de equipo: {n_eq} · consumibles: {n_con}")


if __name__ == "__main__":
    main()
