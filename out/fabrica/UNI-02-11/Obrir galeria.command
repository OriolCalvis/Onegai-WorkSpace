#!/bin/bash
cd "$(dirname "$0")"
(sleep 1; open http://127.0.0.1:8765/) &
exec python3 -I servir.py 8765
