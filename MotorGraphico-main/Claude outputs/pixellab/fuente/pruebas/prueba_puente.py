#!/usr/bin/env python3
"""Pruebas del puente PixelLab (server/index.js) contra un PixelLab falso.

El falso habla MCP por HTTP como el de verdad: initialize con Mcp-Session-Id,
notificaciones con 202, respuestas en JSON y en SSE (con \\r\\n y datos en
varias lineas), 401 sin token valido y 404 con una sesion caducada. Sin red.
"""
import json
import os
import shutil
import subprocess
import sys
import tempfile
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

AQUI = os.path.dirname(os.path.abspath(__file__))
PUENTE = os.path.join(AQUI, "..", "server", "index.js")
TOKEN = "tok-prueba"
ZIP = b"PK\x03\x04" + bytes(range(256)) * 40
fallos = []
registro = []  # (metodo, sesion, version) que ve el falso
estado = {"olvidada": False}  # el falso «olvida» la sesion (caducada)


def comprueba(cond, texto):
    print(("  ok  " if cond else "  MAL ") + texto)
    if not cond:
        fallos.append(texto)


class Falso(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def log_message(self, *a):
        pass

    def _json(self, codigo, cuerpo, cabeceras=None):
        datos = json.dumps(cuerpo).encode() if cuerpo is not None else b""
        self.send_response(codigo)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(datos)))
        for k, v in (cabeceras or {}).items():
            self.send_header(k, v)
        self.end_headers()
        self.wfile.write(datos)

    def do_GET(self):
        if self.path == "/descarga/mago.zip":
            self.send_response(200)
            self.send_header("Content-Type", "application/zip")
            self.send_header("Content-Length", str(len(ZIP)))
            self.end_headers()
            self.wfile.write(ZIP)
        else:
            self._json(404, {"error": "no"})

    def do_POST(self):
        largo = int(self.headers.get("Content-Length", "0"))
        msg = json.loads(self.rfile.read(largo))
        if self.headers.get("Authorization") != "Bearer " + TOKEN:
            return self._json(401, {"error": "token no valido"})
        ses = self.headers.get("Mcp-Session-Id")
        registro.append((msg.get("method"), ses, self.headers.get("MCP-Protocol-Version")))
        metodo = msg.get("method")
        if metodo == "initialize":
            estado["olvidada"] = False
            return self._json(200, {"jsonrpc": "2.0", "id": msg["id"], "result": {
                "protocolVersion": "2025-03-26", "capabilities": {"tools": {}},
                "serverInfo": {"name": "pixellab-falso", "version": "0"}}},
                {"Mcp-Session-Id": "ses-1"})
        if ses == "ses-1" and estado["olvidada"]:
            return self._json(404, {"error": "sesion desconocida"})
        if ses == "ses-vieja":
            return self._json(404, {"error": "sesion desconocida"})
        if ses != "ses-1":
            return self._json(400, {"error": "falta la sesion"})
        if "id" not in msg:  # notificacion (o respuesta del cliente)
            self.send_response(202)
            self.send_header("Content-Length", "0")
            self.end_headers()
            return
        if metodo == "tools/list":
            cuerpo = {"jsonrpc": "2.0", "id": msg["id"], "result": {"tools": [
                {"name": "create_character", "description": "Crea\npersonaje",
                 "inputSchema": {"type": "object"}}]}}
            texto = json.dumps(cuerpo, indent=1)  # JSON en varias lineas de data:
            sse = "event: message\r\n" + "".join("data: " + l + "\r\n" for l in texto.split("\n")) + "\r\n"
            datos = (": comentario\r\n\r\n" + sse).encode()
            self.send_response(200)
            self.send_header("Content-Type", "text/event-stream")
            self.send_header("Content-Length", str(len(datos)))
            self.end_headers()
            # en trozos pequenos: el puente tiene que juntarlos
            for i in range(0, len(datos), 7):
                self.wfile.write(datos[i:i + 7])
                self.wfile.flush()
            return
        if metodo == "tools/call":
            nombre = msg["params"]["name"]
            if nombre == "lento":
                time.sleep(0.6)
            if nombre == "olvidar":
                estado["olvidada"] = True
            puerto = self.server.server_address[1]
            texto = f"hecho {nombre}; descarga: http://127.0.0.1:{puerto}/descarga/mago.zip"
            return self._json(200, {"jsonrpc": "2.0", "id": msg["id"],
                                    "result": {"content": [{"type": "text", "text": texto}]}})
        return self._json(200, {"jsonrpc": "2.0", "id": msg["id"],
                                "error": {"code": -32601, "message": "no existe"}})


class Puente:
    def __init__(self, url, token=TOKEN, carpeta=None):
        env = dict(os.environ, PIXELLAB_MCP_URL=url, PIXELLAB_API_TOKEN=token,
                   PIXELLAB_CARPETA=carpeta or "", PIXELLAB_PRUEBAS="1")
        self.p = subprocess.Popen(["node", PUENTE], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE, env=env, text=True, bufsize=1)
        self.mensajes = []
        self.hilo = threading.Thread(target=self._lee, daemon=True)
        self.hilo.start()

    def _lee(self):
        for linea in self.p.stdout:
            self.mensajes.append(json.loads(linea))

    def manda(self, m):
        self.p.stdin.write(json.dumps(m) + "\n")
        self.p.stdin.flush()

    def espera(self, id_, segundos=5):
        fin = time.time() + segundos
        while time.time() < fin:
            for m in self.mensajes:
                if m.get("id") == id_:
                    return m
            time.sleep(0.02)
        return None

    def cierra(self):
        self.p.stdin.close()
        return self.p.wait(timeout=5)


def main():
    servidor = ThreadingHTTPServer(("127.0.0.1", 0), Falso)
    threading.Thread(target=servidor.serve_forever, daemon=True).start()
    url = f"http://127.0.0.1:{servidor.server_address[1]}/mcp"
    raiz = tempfile.mkdtemp(prefix="prueba_pixellab_")  # unica por ejecucion
    carpeta = os.path.join(raiz, "proyecto")
    os.makedirs(carpeta)
    try:
        print("1. sesion, notificaciones y SSE")
        b = Puente(url, carpeta=carpeta)
        b.manda({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {
            "protocolVersion": "2025-03-26", "capabilities": {}, "clientInfo": {"name": "t", "version": "0"}}})
        r = b.espera(1)
        comprueba(r and r["result"]["serverInfo"]["name"] == "pixellab-falso", "initialize llega y vuelve")
        b.manda({"jsonrpc": "2.0", "method": "notifications/initialized"})
        b.manda({"jsonrpc": "2.0", "id": 2, "method": "tools/list"})
        r = b.espera(2)
        nombres = [t["name"] for t in r["result"]["tools"]] if r and "result" in r else []
        comprueba(nombres == ["create_character", "guardar_en_proyecto"],
                  "tools/list por SSE (trozos, \\r\\n, varias lineas) + la herramienta local")
        comprueba(r and "result" in r and r["result"]["tools"][0]["description"] == "Crea\npersonaje", "el texto del SSE no se altera")
        local = r["result"]["tools"][1] if r and "result" in r and len(r["result"]["tools"]) > 1 else {}
        comprueba(carpeta in local.get("description", ""), "la herramienta local dice su carpeta")
        comprueba(not any(m.get("method") == "notifications/initialized" or
                          (m.get("id") is None and "result" in m) for m in b.mensajes),
                  "a la notificacion no se le inventa respuesta")
        comprueba(registro[1] == ("notifications/initialized", "ses-1", "2025-03-26"),
                  "tras initialize van la sesion y la version del protocolo")

        print("2. llamadas en paralelo")
        b.manda({"jsonrpc": "2.0", "id": 10, "method": "tools/call", "params": {"name": "lento", "arguments": {}}})
        b.manda({"jsonrpc": "2.0", "id": 11, "method": "tools/call", "params": {"name": "rapido", "arguments": {}}})
        r11, r10 = b.espera(11), b.espera(10)
        orden = [m.get("id") for m in b.mensajes if m.get("id") in (10, 11)]
        comprueba(r10 and r11 and orden == [11, 10], "la rapida no espera a la lenta")
        b.manda({"jsonrpc": "2.0", "id": 12, "method": "no/existe"})
        r = b.espera(12)
        comprueba(r and r["error"]["code"] == -32601, "los errores de PixelLab pasan tal cual")

        print("3. guardar_en_proyecto")
        url_zip = r10["result"]["content"][0]["text"].split("descarga: ")[1]
        llamada = lambda i, args: b.manda({"jsonrpc": "2.0", "id": i, "method": "tools/call",
                                           "params": {"name": "guardar_en_proyecto", "arguments": args}})
        llamada(20, {"url": url_zip, "ruta": "MotorGraphico/assets/textures/pixellab/mago.zip"})
        r = b.espera(20)
        destino = os.path.join(carpeta, "MotorGraphico/assets/textures/pixellab/mago.zip")
        comprueba(r and not r["result"].get("isError") and os.path.exists(destino), "guarda creando las carpetas")
        comprueba(os.path.exists(destino) and open(destino, "rb").read() == ZIP, "los bytes son los de la descarga")
        comprueba(not os.path.exists(destino + ".descargando"), "sin restos del temporal")
        llamada(21, {"url": url_zip, "ruta": "MotorGraphico/assets/textures/pixellab/mago.zip"})
        r = b.espera(21)
        comprueba(r and r["result"].get("isError") and "ya existe" in r["result"]["content"][0]["text"],
                  "no sobrescribe sin permiso")
        llamada(22, {"url": url_zip, "ruta": "MotorGraphico/assets/textures/pixellab/mago.zip", "sobrescribir": True})
        comprueba(not b.espera(22)["result"].get("isError"), "sobrescribe si se pide")
        fuera = os.path.join(os.path.dirname(carpeta), "fuera.zip")
        for i, ruta in ((23, "../fuera.zip"), (24, fuera), (25, "a/../../fuera.zip"), (26, ""), (27, ".")):
            llamada(i, {"url": url_zip, "ruta": ruta})
            r = b.espera(i)
            comprueba(r and r["result"].get("isError"), f"rechaza la ruta {ruta!r}")
        comprueba(not os.path.exists(fuera), "nada se ha escrito fuera de la carpeta")
        llamada(28, {"url": "http://example.com/x.zip", "ruta": "x.zip"})
        r = b.espera(28)
        comprueba(r and r["result"].get("isError") and "https" in r["result"]["content"][0]["text"],
                  "fuera de pruebas, solo https")
        llamada(29, {"url": url_zip.replace("mago.zip", "no.zip"), "ruta": "no.zip"})
        r = b.espera(29)
        comprueba(r and r["result"].get("isError") and "404" in r["result"]["content"][0]["text"],
                  "una descarga que falla no deja fichero")
        comprueba(not os.path.exists(os.path.join(carpeta, "no.zip")), "...y no hay no.zip")
        b.manda({"jsonrpc": "2.0", "id": 30, "method": "tools/call", "params": {"name": "olvidar", "arguments": {}}})
        b.espera(30)
        b.manda({"jsonrpc": "2.0", "id": 31, "method": "tools/list"})
        r = b.espera(31)
        comprueba(r and "caducado" in r["error"]["message"], "sesion caducada (404): error que pide reiniciar")
        b.manda({"jsonrpc": "2.0", "id": 32, "method": "initialize", "params": {}})
        b.espera(32)
        b.manda({"jsonrpc": "2.0", "id": 33, "method": "tools/list"})
        r = b.espera(33)
        comprueba(r and "result" in r, "tras volver a inicializar, sigue")
        comprueba(b.cierra() == 0, "al cerrar stdin termina con 0")
        err = b.p.stderr.read()
        comprueba(TOKEN not in err, "el token no sale en el registro")

        print("4. token y sesion")
        b = Puente(url, token="", carpeta=carpeta)
        b.manda({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {}})
        r = b.espera(1)
        comprueba(r and r["error"]["code"] == -32001 and "token" in r["error"]["message"], "sin token: error claro")
        b.cierra()
        b = Puente(url, token="otro", carpeta=carpeta)
        b.manda({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {}})
        r = b.espera(1)
        comprueba(r and "401" in r["error"]["message"] and "revisa el token" in r["error"]["message"],
                  "token malo: 401 con pista")
        b.cierra()
        b = Puente("http://127.0.0.1:9/mcp", carpeta=carpeta)  # puerto cerrado
        b.manda({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {}})
        r = b.espera(1)
        comprueba(r and "No se ha podido hablar con PixelLab" in r["error"]["message"], "sin red: error, no cuelgue")
        b.cierra()
    finally:
        servidor.shutdown()
        shutil.rmtree(raiz, ignore_errors=True)
    print(f"\n{'TODO BIEN' if not fallos else str(len(fallos)) + ' FALLO(S)'}")
    return 1 if fallos else 0


if __name__ == "__main__":
    sys.exit(main())
