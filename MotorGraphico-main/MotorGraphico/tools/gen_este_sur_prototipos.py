"""Genera tres niveles aislados del Este-Sur para el experimento de Egaroth.

No enlaza el mapamundi ni modifica catalogos existentes: esos warps comparten
fronteras con Este-Norte. Reutiliza Mapa y sus comprobaciones de Boundington,
pero conecta ANTES de escribir para que el TMX conserve cualquier apertura.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from gen_ciudad import (
    Mapa, escribir, ADOQUIN, ADOQUIN_FINO, AGUA, ARENA, CARRIL, CASA_MADERA,
    CASA_NOBLE, CASA_PIEDRA, CESPED, GRAVA, MARMOL, MURALLA, PIEDRA_VIEJA,
    PUENTE, RUINA, TIERRA, UMBRAL,
)


def taurengrad():
    m = Mapa(64, 64, ADOQUIN)
    m.borde(MURALLA)
    m.avenida_h(30, 1, 63, ancho=4, calzada=ADOQUIN, lado=CARRIL)
    m.avenida_v(30, 1, 63, ancho=4, calzada=ADOQUIN, lado=CARRIL)
    # Anillos de oficio y vivienda: la ciudad nace de una aldea central.
    for offset in (8, 17, 25):
        m.rect(offset, offset, 64 - 2 * offset, 1, ADOQUIN_FINO)
        m.rect(offset, 63 - offset, 64 - 2 * offset, 1, ADOQUIN_FINO)
    for x in range(4, 60, 8):
        for y in (6, 15, 40, 51):
            m.casa(x, y, 4, 3, CASA_PIEDRA)
    m.rect(23, 22, 18, 17, MARMOL)                    # Plaza del Minotauro
    m.rect(26, 25, 12, 11, ADOQUIN)
    m.edificio(27, 10, 11, 7, CASA_NOBLE, "es_taurengrad_concordia")
    m.edificio(7, 43, 9, 6, PIEDRA_VIEJA, "es_taurengrad_forja")
    m.edificio(47, 43, 9, 6, PIEDRA_VIEJA, "es_taurengrad_paso")
    m.g[63][32] = UMBRAL
    return m, (32, 32), "Taurengrad - Plaza y Anillos"


def dhin_thyraxion():
    m = Mapa(56, 56, GRAVA)
    m.borde(MURALLA)
    # La ciudad se mira hacia dentro: el magma ocupa el eje, no la periferia.
    m.rect(22, 4, 12, 48, AGUA)
    m.rect(25, 4, 6, 48, PUENTE)
    for y, tile in ((10, ADOQUIN_FINO), (27, MARMOL), (43, ADOQUIN)):
        m.rect(3, y, 50, 3, tile)
    for x in (6, 42):
        for y in (5, 17, 34):
            m.casa(x, y, 7, 5, CASA_PIEDRA)
    m.edificio(4, 4, 12, 6, PIEDRA_VIEJA, "es_dhin_fundaciones")
    m.edificio(39, 24, 12, 7, CASA_NOBLE, "es_dhin_corazon")
    m.edificio(20, 45, 16, 6, CASA_NOBLE, "es_dhin_congreso")
    m.g[55][28] = UMBRAL
    return m, (28, 28), "Dhin Thyraxion - Ciudad Caldera"


def naka_tol():
    m = Mapa(64, 64, ADOQUIN_FINO)
    m.borde(MURALLA)
    # Seis estratos horizontales: cada uno es una puerta social distinta.
    strata = (TIERRA, GRAVA, CESPED, ADOQUIN, MARMOL, ADOQUIN_FINO)
    for index, tile in enumerate(strata):
        y = 3 + index * 10
        m.rect(3, y, 58, 8, tile)
        m.calle_h(y + 4, 3, 61, ADOQUIN_FINO)
    m.rect(3, 3, 58, 4, AGUA)                         # corriente de entrada
    for x in range(6, 58, 10):
        m.rect(x, 3, 2, 4, PUENTE)
    m.edificio(5, 13, 9, 5, CASA_MADERA, "es_naka_talleres")
    m.edificio(46, 23, 10, 6, CASA_NOBLE, "es_naka_cultura")
    m.edificio(5, 35, 11, 6, PIEDRA_VIEJA, "es_naka_bastiones")
    m.edificio(44, 45, 12, 6, CASA_NOBLE, "es_naka_casas")
    m.edificio(24, 54, 16, 6, CASA_NOBLE, "es_naka_trono")
    m.g[63][32] = UMBRAL
    return m, (32, 58), "Naka tol - Cupula de Mareas"


def main():
    for name, factory in (("es_taurengrad", taurengrad), ("es_dhin_thyraxion", dhin_thyraxion), ("es_naka_tol", naka_tol)):
        game_map, start, title = factory()
        # escribir() construye el TMX antes de su propia llamada a conectar().
        # La preconexión garantiza que el archivo en disco es el validado.
        _, isolated = game_map.conectar(game_map.cerca_libre(*start))
        assert isolated == 0, f"{name}: zonas transitables aisladas"
        escribir(name, game_map, start, title, mostrar=False)


if __name__ == "__main__":
    main()
