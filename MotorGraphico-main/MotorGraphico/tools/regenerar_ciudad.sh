#!/usr/bin/env bash
# Regenera Boundington entera, en orden.
#
# EXISTE PORQUE EL ORDEN IMPORTA Y NO ESTABA ESCRITO EN NINGUN SITIO. Cada
# script pisa lo que hizo el anterior, asi que ejecutarlos sueltos o en otro
# orden deja el mapa a medias sin dar ningun error:
#
#   - gen_ciudad reescribe los .json de nivel desde cero -> se lleva por
#     delante los enlaces de puerta, los carteles y los PNJs de Boundington.
#   - gen_interiores necesita que los niveles YA existan para enlazar las
#     puertas en ambos sentidos.
#   - gen_carteles necesita las puertas puestas para plantar el cartel
#     delante de cada negocio.
#   - ambientar_boundington va EL ULTIMO porque sustituye PNJs de relleno
#     por los canonicos, y cualquier regeneracion posterior los borraria.
#
# Ya paso: tras rehacer la ciudad, demo_ciudad fallaba con "la ciudad deberia
# tener carteles de negocio" simplemente porque gen_carteles no se habia
# vuelto a pasar.
set -euo pipefail
cd "$(dirname "$0")/.."

echo "1/5  tileset (36 tiles)"
python3 tools/gen_tileset.py

echo "2/5  los cuatro mapas"
python3 tools/gen_ciudad.py

echo "3/5  interiores y enlazado de puertas en ambos sentidos"
python3 tools/gen_interiores.py

echo "4/5  carteles de negocio"
python3 tools/gen_carteles.py

echo "5/5  ambientacion canonica (Skilla, Venides, Aigren, Luisarda, Xila...)"
python3 ../../Onegai-Core/bridge/ambientar_boundington.py

echo
echo "comprobaciones"
python3 tools/conectividad.py | grep -c "100%" | sed 's/^/  niveles 100% alcanzables: /'
python3 tools/validar_enlaces.py | tail -1 | sed 's/^/  /'
