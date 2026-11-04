#!/usr/bin/env python3
"""E-DISPARADORES-1 (D0026.8) — Que objetos tapan la vista (campo `tapaVista` del catalogo de objetos).

Tapan la vista, ademas de bloquear el paso: las paredes de los interiores (paret_*), las vallas y empalizadas
(tanca_*, empalizada*), los carros y carromatos, los armarios, estanterias y vitrinas. NO la tapan: PNJ, enemigos,
pickups, farolas, bolardos, matas, taburetes, sillas, mesas, sacos, cajas ni las puertas (son el hueco).
Los generadores de los kits (exportar_mobles.py, exportar_decorats.py) llaman a tapa_vista(id): la regla vive aqui.
"""
import re

_TAPA = re.compile(r"^(paret_|tanca_|empalizada|carromato|carro(_|$)|armario|estanteria|vitrina|biombo)")


def tapa_vista(ident, bloquea=True):
    """True si el objeto `ident` (que bloquea el paso: `bloquea`) tapa la vista."""
    return bool(bloquea and _TAPA.match(ident))
