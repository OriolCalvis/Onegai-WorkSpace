#!/bin/bash
cd "$(dirname "$0")"
PORT=4100
if ! curl -s -o /dev/null "http://127.0.0.1:$PORT/api/status"; then
  python3 server.py &
  sleep 2
fi
open "http://127.0.0.1:$PORT"
echo "Panel Onegai en marcha. Cierra esta ventana para apagarlo."
wait
