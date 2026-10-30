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
#   - ambientar_boundington va tras los generadores porque sustituye PNJs
#     de relleno por los canonicos; otra generacion posterior los borraria.
#   - enganchar_boundington repone el enlace a Egaroth al final, eligiendo
#     una celda libre con todos los carteles y PNJs ya colocados.
#
# Ya paso: tras rehacer la ciudad, demo_ciudad fallaba con "la ciudad deberia
# tener carteles de negocio" simplemente porque gen_carteles no se habia
# vuelto a pasar.
set -euo pipefail
cd "$(dirname "$0")/.."

echo "1/6  tileset (36 tiles)"
python3 tools/gen_tileset.py

echo "2/6  los cuatro mapas"
python3 tools/gen_ciudad.py

echo "3/6  interiores y enlazado de puertas en ambos sentidos"
python3 tools/gen_interiores.py

echo "4/6  carteles de negocio"
python3 tools/gen_carteles.py

echo "5/6  ambientacion canonica (Skilla, Venides, Aigren, Luisarda, Xila...)"
python3 ../../Onegai-Core/bridge/ambientar_boundington.py

echo "6/6  enlace de ida y vuelta con Egaroth"
python3 tools/enganchar_boundington.py

echo
echo "comprobaciones"
python3 tools/conectividad.py | grep -c "100%" | sed 's/^/  niveles 100% alcanzables: /'
python3 tools/validar_enlaces.py | tail -1 | sed 's/^/  /'
