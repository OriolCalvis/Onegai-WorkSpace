"""Actualiza las tres guias de sprites con lo entregado en ARTE-01.

Edicion in situ con anclas exactas: si un ancla no esta (alguien cambio la
guia), no toca ese fichero y lo dice.
"""
from pathlib import Path

ROOT = Path("MotorGraphico")
cambios = {
    "SPRITE_BACKLOG.md": [
        (
            "| `personaje.png` | Jugador y fallback temporal | No representa raza, profesión ni criatura final. |",
            "| `personaje.png` | Jugador y fallback temporal | No representa raza, profesión ni criatura final. El motor lo lee con un reparto de frames equivocado: ver [actores_capas.md](assets/textures/actores_capas.md). |\n"
            "| `actores_capas.png` | 9 actores con caminar en 4 direcciones (ARTE-01) | Arte + manifiesto: el motor aún no lo carga. Mismo cuerpo que `personaje.png` (40 px). |\n"
            "| `props_p0.png` / `pickups_p0.png` | 33 props y 20 pickups P0 (ARTE-01) | Arte + manifiesto: hoy props y pickups se pintan contra el tileset del nivel. |",
        ),
        (
            "Lo que sigue pendiente para esos mismos actores es caminar, no otra lista de\nretratos estáticos.",
            "Lo que sigue pendiente para esos mismos actores es caminar, no otra lista de\nretratos estáticos.\n\n"
            "**Avance ARTE-01 (25/09):** los actores genéricos que se mueven ya caminan\n"
            "en cuatro direcciones en `assets/textures/actores_capas.png`, vestidos por\n"
            "capas sobre el cuerpo base: aventurero jugador y tres tintes, guardia,\n"
            "aldeano, aldeana, cultista perdido y carcelero ([actores_capas.md](assets/textures/actores_capas.md)).\n"
            "Props y pickups P0 están en `props_p0.png` y `pickups_p0.png`\n"
            "([props_p0.md](assets/textures/props_p0.md)). Faltan: caminar para los\n"
            "retratos de historia (Luisarda, Ben, Griffin: hay que redibujarlos sobre el\n"
            "cuerpo base), niños, saga, naga y duende; pozo, alcantarilla, jaula,\n"
            "huesos, saco y hongo corrupto; carta, monedas, gema, pan y hierba. Para\n"
            "estos no hay dibujo fuente en las láminas de autoría.",
        ),
    ],
    "SPRITE_AUTHORING_GUIDE.md": [
        (
            "| `assets/textures/personaje.png` | 768×64, 12 frames de 64×64 | Runtime temporal | Jugador, PNJ y enemigos mientras no tengan hoja propia. Frames 1–4 abajo, 5–8 arriba y 9–12 lateral. |",
            "| `assets/textures/personaje.png` | 768×64, 12 frames de 64×64 | Runtime temporal | Jugador, PNJ y enemigos mientras no tengan hoja propia. Frames 1–3 arriba, 4–6 derecha, 7–9 abajo y 10–12 izquierda; el quieto es el del medio (`personaje.json`). |\n"
            "| `assets/textures/actores_capas.png` | 768×576, 12×9 frames de 64×64 | Runtime (sin cablear) | 9 actores con caminar en 4 direcciones, vestidos sobre `personaje.png`. Ver `actores_capas.md`. |\n"
            "| `assets/textures/props_p0.png` | 512×320, 8×5 celdas de 64×64 | Runtime (sin cablear) | 33 props P0 a escala de mundo, base en la fila 61. Ver `props_p0.md`. |\n"
            "| `assets/textures/pickups_p0.png` | 512×192, 8×3 celdas de 64×64 | Runtime (sin cablear) | 20 pickups P0 a escala de lectura. Ver `props_p0.md`. |",
        ),
    ],
    "SPRITE_DIMENSIONS_GUIDE.md": [
        (
            "| `personaje.png` | 768×64 | **64×64**, 12 frames | 64×64, ancla Y −32 | Actor temporal: jugador, PNJ y enemigos. Sus pies se apoyan en la base de una celda 64×32. |",
            "| `personaje.png` | 768×64 | **64×64**, 12 frames | 64×64, ancla Y −32 | Actor temporal: jugador, PNJ y enemigos. Sus pies se apoyan en la base de una celda 64×32. Frames: arriba 1–3, derecha 4–6, abajo 7–9, izquierda 10–12. |\n"
            "| `actores_capas.png` | 768×576 | **64×64**, 12×9 | 64×64, ancla Y −32 | Runtime sin cablear: 9 actores con caminar, mismo reparto que `personaje.png` por fila. Persona = 40 px. |\n"
            "| `props_p0.png` | 512×320 | **64×64**, 8×5 | 64×64, ancla Y −32 | Runtime sin cablear: props P0 a escala de mundo, base en la fila 61. |\n"
            "| `pickups_p0.png` | 512×192 | **64×64**, 8×3 | 64×64, ancla Y −32 | Runtime sin cablear: pickups a escala de lectura, base en la fila 61. |",
        ),
    ],
}

for nombre, reemplazos in cambios.items():
    ruta = ROOT / nombre
    texto = ruta.read_text(encoding="utf-8")
    if all(n in texto for _, n in reemplazos):
        print(f"{nombre}: ya estaba actualizado")
        continue
    faltan = [a for a, _ in reemplazos if a not in texto]
    if faltan:
        print(f"{nombre}: SIN CAMBIOS, no encuentro {len(faltan)} ancla(s):")
        for a in faltan:
            print("   ", a[:90])
        continue
    for ancla, nuevo in reemplazos:
        texto = texto.replace(ancla, nuevo, 1)
    ruta.write_text(texto, encoding="utf-8")
    print(f"{nombre}: actualizado ({len(reemplazos)} cambio(s))")
