"""Láminas antes/después: captura de hoy frente a la maqueta propuesta.

Cada lámina cita los errores (E..) y decisiones de MODELO_VISUAL.md §7.
"""
import html
import os
import sys

from lib import FUENTE, SALIDA, px

CAPTURAS = os.environ.get("MODELO_CAPTURAS", os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "capturas_menus_2026-09-28"))
DESTINO = os.path.join(SALIDA, "antes_despues")

PARES = [
    ("M01_titulo", "01_juego_titulo/01-01_titulo_al_arrancar.png", "Título",
     ["E11 El panel ya no cambia de ancho: el motivo va en una línea secundaria.",
      "E12 Sin partida guardada, el foco nace en «Nueva partida» y es lo que ejecuta ENTER.",
      "E10 Portada con un lugar real (Plaza de las Razas) y velo para leer el menú."]),
    ("M02a_creacion_raza", "02_juego_nueva_partida/02-01_raza.png", "Nueva partida · Raza",
     ["E26 Cabecera con etapas, pregunta completa y «Paso 1 de 14»; nada invade la lista.",
      "E03 Nombres y descripción en Lora; el pixel queda para títulos, etapas e insignias.",
      "E09 Lista 38 % / detalle 62 %: bonificaciones, rasgo y cultura separados."]),
    ("M02b_creacion_atributos", "02_juego_nueva_partida/02-08_reparto_a_medias.png", "Nueva partida · Atributos",
     ["E26 Cuatro filas con nombre completo, −/+, bonificación de raza y total.",
      "E27 El pie dice las teclas que funcionan (←/→), no «A/D».",
      "E15 «Continuar» deshabilitado con el motivo al lado: te quedan 4 puntos."]),
    ("M02c_creacion_revision", "02_juego_nueva_partida/02-22_resumen.png", "Nueva partida · Revisión",
     ["E28 Revisión con las 14 decisiones en tres grupos y «Editar» por grupo.",
      "E06 La personalidad sale como «Acompaña en silencio», no como PERSONALITY_DE_LAGONIA_1.",
      "C01 El nombre de ejemplo ya no coincide con un PNJ canónico (Luisarda)."]),
    ("M03a_hud_exploracion", "03_juego_prologo_y_ciudad/03-14_caminando_junto_a_un_npc.png", "Exploración",
     ["E21 Nombre, PV, PM y moral con la etiqueta encima de la barra; sin FPS.",
      "E22 E24 Objetivo seguido con contador y minimapa real con leyenda de formas.",
      "E23 «[E] Hablar con Anciana» y rombo en su baldosa: se sabe qué hará la tecla."]),
    ("M03b_dialogo", "03_juego_prologo_y_ciudad/03-15_dialogo_con_npc.png", "Diálogo",
     ["E25 Pestaña con el hablante y pie dentro del panel: página 1/2 y «Continuar».",
      "E03 Texto en caja alta y baja, 22 px, ancho de lectura limitado.",
      "El HUD se reduce a PV y PM durante la conversación."]),
    ("M04a_tab_inventario", "04_juego_menu_jugador_TAB/04-01_inventario.png", "Menú del jugador · Inventario",
     ["E08 E13 Seis pestañas horizontales dentro de la ventana y un único foco.",
      "E15 E16 «Usar» deshabilitado con motivo («PV al máximo, no se gasta nada»).",
      "Éter usa una variante azul del icono de consumible para no confundirse con la poción."]),
    ("M04b_tab_ficha", "04_juego_menu_jugador_TAB/04-05_estado_ficha.png", "Menú del jugador · Ficha",
     ["La sección «Estado» pasa a llamarse «Ficha» (decisión I2).",
      "E04 Nombres con tildes y ñ; atributos con su origen (base + raza).",
      "El rasgo muestra su condición y su texto canónico; historia personal completa."]),
    ("M05a_pausa_guardado", "05_juego_pausa_y_opciones/05-05_pausa_desar_sin_confirmacion_SIMULADO.png", "Pausa",
     ["E36 Pausa visible con título y velo: el mundo queda detrás.",
      "E05 Todo en castellano: «Reanudar», no «Reprender» (que significa regañar).",
      "E17 «Partida guardada» con lugar y hora junto a la acción."]),
    ("M05b_opciones", "05_juego_pausa_y_opciones/05-09_opciones_SIMULADO.png", "Opciones",
     ["E37 Volúmenes con deslizador y porcentaje; ayuda de la fila con foco abajo.",
      "E05 Idioma con su nombre propio: «Español», «Català», «English».",
      "Lista agrupada en lugar de pestañas: ←/→ ya ajustan valores (decisión I5)."]),
    ("M06_tienda", "06_juego_tienda/06-04_comprar_pocion.png", "Tienda",
     ["E29 Comprar y Vender en pestañas: la lista no cambia de longitud al comprar.",
      "E07 Cabecera propia para comerciante y oro; nada se superpone.",
      "E18 «Poción comprada · −30 de oro» dentro de la tienda, junto a la acción."]),
    ("M07a_negocio_en_venta", "07_juego_negocio/07-03_comprar_sin_oro.png", "Negocio en venta",
     ["E07 Ficha en rejilla: precio, renta, ciclo y tu oro sin solapes.",
      "E32 «Ciclo» explicado con la regla real del motor: cada 40 pasos.",
      "E15 Comprar deshabilitado y el motivo antes de intentarlo: te faltan 850."]),
    ("M07b_tu_negocio", "07_juego_negocio/07-07_renta_subida.png", "Tu negocio",
     ["E31 «TU NEGOCIO» sustituye a «EN VENTA»: los dos estados no conviven.",
      "E32 Alquiler con −/+ y efecto calculado con las fórmulas del juego (ocupación, oro, moral).",
      "E18 El resultado aparece junto al control, no en el registro."]),
    ("M08a_combate", "08_juego_combate/08-03_combate_empieza.png", "Combate",
     ["E33 Franja inferior: personaje, acciones, detalle y últimas acciones juntos.",
      "E34 «Poción ×2» y detalle de la acción; el objetivo lleva placa y rombo.",
      "Escena de campaña (atraco del tutorial), no la galería de pruebas."]),
    ("M08b_victoria", "08_juego_combate/08-08_tras_la_victoria.png", "Victoria",
     ["E35 El combate se cierra con un resultado; «Continuar» da paso a la narración.",
      "Muestra lo que cambió de verdad: PV y objetivo cumplido.",
      "Sin botín inventado: «No hay botín»."]),
    ("M09_derrota", "09_juego_game_over/09-01_game_over_SIMULADO.png", "Derrota",
     ["E36 Derrota visible, con título en castellano y velo sobre el mundo apagado.",
      "E38 Tarjeta con la partida que se cargará: lugar, hora y tipo de guardado.",
      "E05 «Cargar último guardado · Elegir partida · Volver al título»."]),
]


def lamina(clave, captura, titulo, notas):
    antes = os.path.join(CAPTURAS, captura)
    despues = os.path.join(SALIDA, clave + ".png")
    li = "".join(f"<li>{html.escape(n)}</li>" for n in notas)
    nombre_cap = os.path.basename(captura)
    return f"""<!doctype html><html lang="es"><head><meta charset="utf-8">
<link rel="stylesheet" href="file://{FUENTE}/sistema/tokens.css">
<link rel="stylesheet" href="file://{FUENTE}/sistema/componentes.css">
<style>
body{{width:1328px;background:var(--fondo)}}
.l{{padding:20px 24px 22px;display:flex;flex-direction:column;gap:14px}}
.c{{display:flex;align-items:baseline;gap:16px}}
.g{{display:grid;grid-template-columns:640px 640px;gap:0 16px}}
figure{{display:flex;flex-direction:column;gap:8px}}
figure img{{width:640px;height:360px;display:block;box-shadow:0 0 0 2px var(--marco-ext)}}
figcaption{{display:flex;align-items:center;gap:10px;font-size:15px;color:var(--texto-3)}}
ul{{margin:2px 0 0 20px;font-size:17px;line-height:1.5;color:var(--texto-2)}}
</style></head><body><div class="l">
<div class="c">{px(clave.split("_")[0], 3)}<span style="font-size:24px;font-weight:600">{html.escape(titulo)}</span></div>
<div class="g">
<figure><figcaption><span class="insignia i-error">{px("HOY", 2)}</span>{html.escape(nombre_cap)}</figcaption><img src="file://{antes}"></figure>
<figure><figcaption><span class="insignia i-exito">{px("PROPUESTA", 2)}</span>{html.escape(clave)}.png</figcaption><img src="file://{despues}"></figure>
</div>
<ul>{li}</ul>
</div></body></html>"""


def main(claves=None):
    from playwright.sync_api import sync_playwright
    os.makedirs(DESTINO, exist_ok=True)
    import tempfile
    tmp = tempfile.mkdtemp(prefix="comparativas_")
    with sync_playwright() as p:
        nav = p.chromium.launch()
        pag = nav.new_page(viewport={"width": 1328, "height": 600}, device_scale_factor=1)
        for clave, cap, titulo, notas in PARES:
            if claves and clave.split("_")[0] not in claves:
                continue
            assert os.path.exists(os.path.join(CAPTURAS, cap)), cap
            ruta = os.path.join(tmp, clave + ".html")
            open(ruta, "w", encoding="utf-8").write(lamina(clave, cap, titulo, notas))
            pag.goto("file://" + ruta)
            pag.evaluate("document.fonts.ready")
            pag.wait_for_timeout(120)
            pag.screenshot(path=os.path.join(DESTINO, clave + ".png"), full_page=True)
            print("ok", clave)
        nav.close()


if __name__ == "__main__":
    main(sys.argv[1:] or None)
