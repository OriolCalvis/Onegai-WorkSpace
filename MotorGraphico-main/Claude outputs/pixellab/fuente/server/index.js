#!/usr/bin/env node
// PixelLab para Claude (MotorGraphico) — puente MCP stdio <-> HTTP.
//
// La app de escritorio de Claude solo arranca servidores MCP locales (stdio).
// PixelLab es un servidor MCP remoto por HTTP que pide «Authorization: Bearer»,
// y los entornos de Claude (contenedor y VM de Cowork) no pueden llegar a
// api.pixellab.ai. Este puente corre en el Mac, dentro de la app: lee los
// mensajes JSON-RPC de Claude por stdin, los reenvia a PixelLab y devuelve sus
// respuestas (JSON o SSE) por stdout. Sin dependencias: Node >= 18.
//
// Anade una herramienta local, guardar_en_proyecto: descarga lo que PixelLab
// genera (sus URLs de descarga) y lo guarda dentro de la carpeta del proyecto
// configurada en la extension. Nunca escribe fuera de esa carpeta.
//
// El token llega por PIXELLAB_API_TOKEN (la app lo guarda en el llavero) y
// no se escribe nunca en ningun registro.
'use strict';

const fs = require('fs');
const path = require('path');
const readline = require('readline');

const URL_MCP = process.env.PIXELLAB_MCP_URL || 'https://api.pixellab.ai/mcp';
const TOKEN = (process.env.PIXELLAB_API_TOKEN || '').trim();
const CARPETA = (process.env.PIXELLAB_CARPETA || '').trim();
// Solo para las pruebas del puente: permite descargar de http://127.0.0.1.
const PRUEBAS = process.env.PIXELLAB_PRUEBAS === '1';
const MAX_BYTES = 200 * 1024 * 1024;
const ESPERA_PIXELLAB_MS = 15 * 60 * 1000;  // chat_send_message puede tardar 10 min
const ESPERA_DESCARGA_MS = 5 * 60 * 1000;
const VERSION = '1.0.0';

let sesion = null;            // Mcp-Session-Id que da PixelLab al inicializar
let versionProtocolo = null;  // la que PixelLab acepta en initialize
let pendientes = 0;
let entradaCerrada = false;

function log(...partes) {
  process.stderr.write('[pixellab] ' + partes.join(' ') + '\n');
}

function enviar(mensaje) {
  // Un mensaje por linea y en una sola escritura: las respuestas pueden
  // llegar en cualquier orden y no deben mezclarse.
  process.stdout.write(JSON.stringify(mensaje) + '\n');
}

function errorRpc(id, codigo, texto) {
  if (id === undefined || id === null) return;  // a una notificacion no se responde
  enviar({ jsonrpc: '2.0', id, error: { code: codigo, message: texto } });
}

function esPeticion(m) {
  return m && typeof m === 'object' && typeof m.method === 'string' && 'id' in m;
}

// --- La herramienta local ---------------------------------------------------

function herramientaLocal() {
  const donde = CARPETA ? ` Carpeta configurada: ${CARPETA}.` : ' (Falta configurar la carpeta.)';
  return {
    name: 'guardar_en_proyecto',
    title: 'Guardar en el proyecto',
    description:
      'Descarga un fichero generado por PixelLab (la URL de descarga de un personaje, objeto, ' +
      'tileset, imagen o UI) y lo guarda dentro de la carpeta del proyecto configurada en la ' +
      'extension. «ruta» es relativa a esa carpeta, por ejemplo ' +
      '"MotorGraphico/assets/textures/pixellab/mago.zip". No sobrescribe nada salvo ' +
      'sobrescribir=true y nunca escribe fuera de la carpeta.' + donde,
    inputSchema: {
      type: 'object',
      properties: {
        url: { type: 'string', description: 'URL https del fichero (la de descarga de PixelLab).' },
        ruta: { type: 'string', description: 'Ruta relativa dentro de la carpeta del proyecto.' },
        sobrescribir: { type: 'boolean', description: 'Reemplazar si ya existe.', default: false },
      },
      required: ['url', 'ruta'],
    },
  };
}

function resultado(texto, esError = false) {
  const r = { content: [{ type: 'text', text: texto }] };
  if (esError) r.isError = true;
  return r;
}

// Devuelve la ruta absoluta de destino o lanza un Error con el motivo.
function destinoSeguro(ruta) {
  if (!CARPETA) throw new Error('la extension no tiene carpeta del proyecto configurada');
  if (typeof ruta !== 'string' || !ruta.trim()) throw new Error('falta la ruta');
  if (path.isAbsolute(ruta)) throw new Error('la ruta tiene que ser relativa a la carpeta del proyecto');
  const base = path.resolve(CARPETA);
  const destino = path.resolve(base, ruta);
  if (destino !== base && !destino.startsWith(base + path.sep)) {
    throw new Error('la ruta sale de la carpeta del proyecto');
  }
  if (destino === base) throw new Error('la ruta tiene que nombrar un fichero');
  return destino;
}

function urlPermitida(texto) {
  let u;
  try {
    u = new URL(texto);
  } catch {
    throw new Error('URL no valida');
  }
  const local = u.hostname === '127.0.0.1' || u.hostname === 'localhost';
  if (u.protocol === 'https:') return u;
  if (PRUEBAS && u.protocol === 'http:' && local) return u;
  throw new Error('solo se descargan URLs https');
}

async function guardarEnProyecto(argumentos) {
  const { url, ruta, sobrescribir } = argumentos || {};
  let destino, u;
  try {
    destino = destinoSeguro(ruta);
    u = urlPermitida(url);
  } catch (e) {
    return resultado('No se ha guardado: ' + e.message + '.', true);
  }
  if (fs.existsSync(destino) && !sobrescribir) {
    return resultado(`No se ha guardado: ${ruta} ya existe (pasa sobrescribir=true para reemplazarlo).`, true);
  }
  // Las descargas de PixelLab no piden token (el UUID es la llave); a su
  // propio dominio se le manda igualmente, nunca a otro.
  const cabeceras = {};
  if (TOKEN && (u.hostname === 'api.pixellab.ai' || u.hostname.endsWith('.pixellab.ai'))) {
    cabeceras.Authorization = 'Bearer ' + TOKEN;
  }
  let r;
  try {
    r = await fetch(u, { headers: cabeceras, redirect: 'follow', signal: AbortSignal.timeout(ESPERA_DESCARGA_MS) });
  } catch (e) {
    return resultado('No se ha podido descargar: ' + e.message + '.', true);
  }
  if (!r.ok) return resultado(`No se ha podido descargar: el servidor respondio ${r.status}.`, true);
  const declarado = Number(r.headers.get('content-length') || 0);
  if (declarado > MAX_BYTES) return resultado('No se ha guardado: el fichero pasa de 200 MB.', true);
  const datos = Buffer.from(await r.arrayBuffer());
  if (datos.length > MAX_BYTES) return resultado('No se ha guardado: el fichero pasa de 200 MB.', true);
  fs.mkdirSync(path.dirname(destino), { recursive: true });
  const temporal = destino + '.descargando';
  fs.writeFileSync(temporal, datos);
  fs.renameSync(temporal, destino);  // nunca queda un fichero a medias
  const tipo = r.headers.get('content-type') || 'desconocido';
  return resultado(`Guardado: ${ruta} (${datos.length} bytes, ${tipo}).`);
}

// --- El puente ----------------------------------------------------------------

// Lo que vuelve de PixelLab pasa por aqui antes de llegar a Claude.
function salida(mensaje, peticion) {
  if (peticion && mensaje && mensaje.result) {
    if (peticion.method === 'initialize' && mensaje.result.protocolVersion) {
      versionProtocolo = mensaje.result.protocolVersion;
    }
    const primeraPagina = !(peticion.params && peticion.params.cursor);
    if (peticion.method === 'tools/list' && primeraPagina && Array.isArray(mensaje.result.tools)) {
      if (!mensaje.result.tools.some((t) => t && t.name === 'guardar_en_proyecto')) {
        mensaje.result.tools.push(herramientaLocal());
      }
    }
  }
  enviar(mensaje);
}

function despachar(texto, peticion) {
  if (!texto.trim()) return;
  let datos;
  try {
    datos = JSON.parse(texto);
  } catch {
    log('PixelLab envio algo que no es JSON (' + texto.length + ' caracteres)');
    return;
  }
  for (const m of [].concat(datos)) salida(m, peticion);
}

// text/event-stream: cada evento es un mensaje JSON-RPC en sus lineas data:.
async function leerEventos(respuesta, peticion) {
  const decodificador = new TextDecoder();
  let pendiente = '';
  let datos = [];
  const linea = (l) => {
    if (l === '') {
      if (datos.length) despachar(datos.join('\n'), peticion);
      datos = [];
    } else if (l.startsWith('data:')) {
      datos.push(l.slice(5).replace(/^ /, ''));
    }  // event:, id:, retry: y los comentarios (:) no hacen falta aqui
  };
  for await (const trozo of respuesta.body) {
    pendiente += decodificador.decode(trozo, { stream: true });
    let i;
    while ((i = pendiente.indexOf('\n')) >= 0) {
      linea(pendiente.slice(0, i).replace(/\r$/, ''));
      pendiente = pendiente.slice(i + 1);
    }
  }
  pendiente += decodificador.decode();
  if (pendiente) linea(pendiente.replace(/\r$/, ''));
  linea('');
}

async function reenviar(mensaje) {
  const peticion = esPeticion(mensaje) ? mensaje : null;
  if (!TOKEN) {
    if (peticion) errorRpc(mensaje.id, -32001, 'Falta el token de PixelLab: ponlo en los ajustes de la extension.');
    return;
  }
  const cabeceras = {
    'Content-Type': 'application/json',
    Accept: 'application/json, text/event-stream',
    Authorization: 'Bearer ' + TOKEN,
  };
  if (sesion) cabeceras['Mcp-Session-Id'] = sesion;
  if (versionProtocolo) cabeceras['MCP-Protocol-Version'] = versionProtocolo;
  let r;
  try {
    r = await fetch(URL_MCP, {
      method: 'POST',
      headers: cabeceras,
      body: JSON.stringify(mensaje),
      signal: AbortSignal.timeout(ESPERA_PIXELLAB_MS),
    });
  } catch (e) {
    if (peticion) errorRpc(mensaje.id, -32000, 'No se ha podido hablar con PixelLab: ' + e.message);
    return;
  }
  const nuevaSesion = r.headers.get('mcp-session-id');
  if (nuevaSesion) sesion = nuevaSesion;
  if (r.status === 202 || r.status === 204) {
    await r.arrayBuffer().catch(() => {});
    return;  // notificacion o respuesta aceptada
  }
  if (r.status === 404 && sesion && !(peticion && peticion.method === 'initialize')) {
    sesion = null;  // PixelLab olvido la sesion: hay que volver a inicializar
    if (peticion) errorRpc(mensaje.id, -32000, 'La sesion con PixelLab ha caducado: reinicia la extension.');
    return;
  }
  if (!r.ok) {
    const cuerpo = (await r.text().catch(() => '')).slice(0, 300);
    const motivo = r.status === 401 || r.status === 403 ? ' (revisa el token en los ajustes de la extension)' : '';
    if (peticion) errorRpc(mensaje.id, -32000, `PixelLab respondio ${r.status}${motivo}: ${cuerpo}`);
    return;
  }
  const tipo = r.headers.get('content-type') || '';
  if (tipo.includes('text/event-stream')) {
    await leerEventos(r, peticion);
  } else {
    despachar(await r.text(), peticion);
  }
}

async function atender(mensaje) {
  if (Array.isArray(mensaje)) {
    await Promise.all(mensaje.map(atender));
    return;
  }
  if (!mensaje || typeof mensaje !== 'object') return;
  if (mensaje.method === 'tools/call' && mensaje.params && mensaje.params.name === 'guardar_en_proyecto') {
    const r = await guardarEnProyecto(mensaje.params.arguments).catch((e) =>
      resultado('No se ha guardado: ' + e.message + '.', true));
    enviar({ jsonrpc: '2.0', id: mensaje.id, result: r });
    return;
  }
  await reenviar(mensaje);
}

function terminarSiPuedo() {
  if (entradaCerrada && pendientes === 0) process.exit(0);
}

const entrada = readline.createInterface({ input: process.stdin, crlfDelay: Infinity });
entrada.on('line', (texto) => {
  const limpio = texto.trim();
  if (!limpio) return;
  let mensaje;
  try {
    mensaje = JSON.parse(limpio);
  } catch {
    log('linea que no es JSON en stdin; se ignora');
    return;
  }
  pendientes += 1;
  atender(mensaje)
    .catch((e) => {
      log('error: ' + e.message);
      if (esPeticion(mensaje)) errorRpc(mensaje.id, -32603, 'Error interno del puente: ' + e.message);
    })
    .finally(() => {
      pendientes -= 1;
      terminarSiPuedo();
    });
});
entrada.on('close', () => {
  entradaCerrada = true;
  terminarSiPuedo();
});

log(`puente ${VERSION} listo hacia ${URL_MCP}` + (TOKEN ? '' : ' (SIN TOKEN)') + (CARPETA ? '' : ' (sin carpeta)'));
