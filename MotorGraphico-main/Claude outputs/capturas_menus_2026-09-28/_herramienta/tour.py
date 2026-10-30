#!/usr/bin/env python3
"""Tour de capturas de MotorGraphico.

Cada escena genera un guion para la plataforma null de GLFW (ver
glfw-cap/src/capture_script.c), lanza el binario y convierte las
capturas PPM a PNG con el nombre final. Uso: tour.py [escena ...]
"""
import json, os, shutil, subprocess, sys
sys.path.insert(0, "/home/claude/work")
from gen import G, crear_personaje, andar
from nav import Level

BUILD = "/home/claude/build"
OUT = "/home/claude/capturas"
EJEMPLOS = ["--nivel", "assets/levels/ejemplos_objetos.json",
            "--catalogo", "assets/objects/test_objects.json",
            "--catalogo-extra", "assets/objects/ejemplos_interacciones.json"]

CIUDAD = Level("assets/levels/ciudad_centro.json")
SASTRERIA = Level("assets/levels/interior_sastreria.json")
EJ = Level("assets/levels/ejemplos_objetos.json")


def run(name, binary, g, outdir, args=(), env=None, keep_saves=False, timeout=600):
    os.makedirs(outdir, exist_ok=True)
    tmp = f"/home/claude/work/tmp_{name}"
    shutil.rmtree(tmp, ignore_errors=True); os.makedirs(tmp)
    script = g.save(f"/home/claude/work/guion_{name}.txt")
    if not keep_saves:
        shutil.rmtree(os.path.join(BUILD, "saves"), ignore_errors=True)
    e = dict(os.environ)
    e.update({"EGL_PLATFORM": "surfaceless", "GLFW_SCRIPT": script,
              "GLFW_SHOT_DIR": tmp, "GLFW_SCRIPT_QUIT": "1",
              "XDG_CONFIG_HOME": "/home/claude/work/xdg"})
    if env:
        e.update(env)
    with open(f"{tmp}/_log.txt", "w") as log:
        try:
            r = subprocess.run([f"./{binary}", *args], cwd=BUILD, env=e, stdout=log,
                               stderr=subprocess.STDOUT, timeout=timeout)
        except subprocess.TimeoutExpired:
            class R: returncode = "timeout"
            r = R()
    n = 0
    for f in sorted(os.listdir(tmp)):
        if f.endswith(".ppm"):
            subprocess.run(["convert", f"{tmp}/{f}", f"{outdir}/{f[:-4]}.png"], check=True)
            n += 1
    shutil.copy(f"{tmp}/_log.txt", f"{outdir}/_log_{name}.txt")
    print(f"[{name}] exit={r.returncode} capturas={n}")
    return n


def prologo_rapido(g):
    for _ in range(13):
        g.k("ENTER", gap=12)
    g.sl(1200)
    g.k("ENTER", 4, gap=12)
    g.sl(600)
    return g


# ---------------------------------------------------------------- escenas

def s01_titulo():
    d = f"{OUT}/01_juego_titulo"
    g = G().w(25)
    g.shot("01-01_titulo_al_arrancar")
    g.k("S", gap=10).shot("01-02_titulo_cursor_opciones")
    g.k("ENTER", gap=10).shot("01-03_titulo_opciones_sin_pantalla")
    g.k("S", gap=10).shot("01-04_titulo_cursor_salir")
    g.k("S", gap=10).shot("01-05_titulo_cursor_continuar")
    g.k("ENTER", gap=10).shot("01-06_titulo_continuar_sin_partida")
    g.k("S", gap=10).shot("01-07_titulo_cursor_nueva_partida")
    run("s01", "juego", g, d)


def s02_nueva_partida():
    d = f"{OUT}/02_juego_nueva_partida"
    g = G().w(25).k("ENTER", gap=10)
    g.shot("02-01_raza")
    g.k("S", 3, gap=6).shot("02-02_raza_otra_opcion")
    g.k("ENTER", gap=10).shot("02-03_clase")
    g.k("S", 2, gap=6).shot("02-04_clase_otra_opcion")
    g.k("ENTER", gap=10).shot("02-05_trasfondo")
    g.k("ENTER", gap=10).shot("02-06_reparto_inicial")
    g.k("ENTER", gap=10).shot("02-07_reparto_aviso_puntos_pendientes")
    g.k("RIGHT", 3, gap=5).k("S", gap=5).k("RIGHT", 3, gap=5).shot("02-08_reparto_a_medias")
    g.k("S", gap=5).k("RIGHT", 3, gap=5).k("S", gap=5).k("RIGHT", 3, gap=5)
    g.shot("02-09_reparto_completo")
    g.k("RIGHT", gap=8).shot("02-10_reparto_sin_puntos")
    pasos = ["02-11_vinculo", "02-12_miedo", "02-13_defecto", "02-14_meta",
             "02-15_ideal", "02-16_personalidad", "02-17_virtud", "02-18_deidad"]
    for p in pasos:
        g.k("ENTER", gap=10).shot(p)
    g.k("S", 2, gap=6).shot("02-19_deidad_elegida")
    g.k("ENTER", gap=10).shot("02-20_nombre")
    g.k("RIGHT", 2, gap=8).shot("02-21_nombre_otro")
    g.k("ENTER", gap=10).shot("02-22_resumen")
    g.k("ENTER", gap=10).w(30).shot("02-23_empieza_la_partida")
    run("s02", "juego", g, d)
    # atras y cancelar
    g = G().w(25).k("ENTER", gap=10).k("ENTER", gap=10)
    g.shot("02-24_clase_antes_de_volver")
    g.k("ESC", gap=10).shot("02-25_esc_vuelve_al_paso_anterior")
    g.k("ESC", gap=10).w(10).shot("02-26_esc_en_el_primer_paso_cancela")
    run("s02b", "juego", g, d)


def s03_prologo_ciudad():
    d = f"{OUT}/03_juego_prologo_y_ciudad"
    g = crear_personaje(G()).w(30)
    for i in range(11):
        g.sl(300).shot(f"03-{i+1:02d}_prologo_dialogo_{i+1:02d}")
        g.k("ENTER", gap=12)
    g.sl(1500).shot("03-12_llegada_a_boundington")
    g.k("ENTER", gap=12).sl(400).shot("03-13_encargo_del_tutorial")
    g.k("ENTER", 3, gap=12).sl(500).shot("03-14_exploracion_ciudad")
    a = CIUDAD.find("anciana")[0]
    andar(g, CIUDAD.path(CIUDAD.start, CIUDAD.adjacent_free(a)[0]))
    g.shot("03-15_caminando_junto_a_un_npc")
    g.k("E", gap=12).shot("03-16_dialogo_con_npc")
    g.k("ENTER", gap=12)
    # puerta a la plaza de las razas
    here = CIUDAD.adjacent_free(a)[0]
    puerta = CIUDAD.find("puerta_plaza_de_las_razas")[0]
    andar(g, CIUDAD.path(here, puerta))
    g.shot("03-17_sobre_una_puerta")
    g.k("E", gap=10).sl(1500).shot("03-18_otra_zona_plaza_de_las_razas")
    g.k("ENTER", 3, gap=12).sl(500).shot("03-19_plaza_tras_el_dialogo")
    run("s03", "juego", g, d)


def comprar_en_sastreria(g, compras=2, shots=None):
    """Desde la llegada a la ciudad: entra en la sastreria y compra pociones."""
    andar(g, CIUDAD.path(CIUDAD.start, (42, 27)))
    g.k("E", gap=10).sl(1500)
    andar(g, SASTRERIA.path(SASTRERIA.start, (6, 5)))
    return g


def s04_menu_jugador():
    d = f"{OUT}/04_juego_menu_jugador_TAB"
    g = prologo_rapido(crear_personaje(G()).w(30))
    comprar_en_sastreria(g)
    g.k("E", gap=12)            # abre tienda de Luisarda
    g.k("ENTER", gap=10).k("ENTER", gap=10).k("S", gap=8).k("ENTER", gap=10)  # 2 pociones + 1 eter
    g.k("ESC", gap=12).k("ENTER", 2, gap=10)
    g.k("TAB", gap=15).shot("04-01_inventario")
    g.k("S", gap=8).shot("04-02_inventario_otra_fila")
    g.k("W", gap=8).k("ENTER", gap=12).shot("04-03_inventario_usar_objeto")
    for i, n in enumerate(["equipo", "estado_ficha", "acciones", "misiones"]):
        g.k("RIGHT", gap=12).shot(f"04-{i+4:02d}_{n}")
    g.k("S", 3, gap=8).shot("04-08_misiones_otra_fila")
    g.k("RIGHT", gap=12).shot("04-09_codice")
    g.k("S", 4, gap=8).shot("04-10_codice_otra_fila")
    g.k("TAB", gap=12).shot("04-11_menu_cerrado")
    run("s04", "juego", g, d)


def s05_pausa():
    d = f"{OUT}/05_juego_pausa_y_opciones"
    # hoy (sin parche de visibilidad)
    g = prologo_rapido(crear_personaje(G()).w(30))
    g.k("ESC", gap=15).shot("05-00_pausa_HOY_no_se_dibuja")
    run("s05a", "juego", g, d, env={"CAPTURAS_COMO_HOY": "1"})
    g = prologo_rapido(crear_personaje(G()).w(30))
    g.k("ESC", gap=15).shot("05-01_pausa")
    g.k("S", gap=8).shot("05-02_pausa_cursor_personaje")
    g.k("ENTER", gap=12).shot("05-03_pausa_personaje_no_abre_nada")
    g.k("S", 2, gap=8).shot("05-04_pausa_cursor_desar")
    g.k("ENTER", gap=12).shot("05-05_pausa_desar_sin_confirmacion")
    g.k("S", gap=8).shot("05-06_pausa_cursor_carregar")
    g.k("ENTER", gap=12).sl(1200).shot("05-07_pausa_carregar_y_aparece_el_menu_de_personaje")
    g.k("ESC", gap=15).k("ESC", gap=15).k("S", gap=8).shot("05-08_pausa_cursor_opcions")
    g.k("ENTER", gap=12).shot("05-09_opciones")
    g.k("RIGHT", gap=8).k("RIGHT", gap=8).shot("05-10_opciones_musica_subida")
    g.k("S", gap=8).k("LEFT", 3, gap=6).shot("05-11_opciones_efectos_bajados")
    g.k("S", 2, gap=8).k("RIGHT", gap=10).shot("05-12_opciones_idioma_castella")
    g.k("RIGHT", gap=10).shot("05-13_opciones_idioma_english")
    g.k("S", gap=8).k("ENTER", gap=10).shot("05-14_opciones_controles_pendiente")
    g.k("S", gap=8).k("ENTER", gap=10).shot("05-15_opciones_volver_a_pausa")
    g.k("S", gap=8).shot("05-16_pausa_cursor_tornar_al_titol_CUELGA")
    run("s05b", "juego", g, d)
    # titulo con la partida que se acaba de guardar
    g = G().w(25).shot("05-17_titulo_con_partida_guardada")
    g.k("ENTER", gap=12).sl(1500).shot("05-18_continuar_carga_la_partida")
    run("s05c", "juego", g, d, keep_saves=True)


def s06_tienda():
    d = f"{OUT}/06_juego_tienda"
    g = prologo_rapido(crear_personaje(G()).w(30))
    comprar_en_sastreria(g)
    g.shot("06-01_interior_de_la_sastreria")
    g.k("E", gap=12).shot("06-02_tienda_abierta")
    g.k("S", gap=8).shot("06-03_cursor_en_eter")
    g.k("W", gap=8).k("ENTER", gap=12).shot("06-04_comprar_pocion")
    g.k("S", 2, gap=8).shot("06-05_cursor_en_vender")
    g.k("ENTER", gap=12).shot("06-06_vender_pocion")
    g.k("W", 2, gap=8)
    g.k("ENTER", 5, gap=10).shot("06-07_sin_oro_suficiente")
    g.k("ESC", gap=12).shot("06-08_tienda_cerrada")
    run("s06", "juego", g, d)


def s07_negocio():
    d = f"{OUT}/07_juego_negocio"
    g = prologo_rapido(crear_personaje(G()).w(30))
    andar(g, CIUDAD.path(CIUDAD.start, (42, 29)))
    g.shot("07-01_junto_al_cartel_de_la_sastreria")
    g.k("E", gap=12).shot("07-02_negocio_en_venta")
    g.k("ENTER", gap=12).shot("07-03_comprar_sin_oro")
    g.k("S", gap=8).shot("07-04_cursor_salir")
    run("s07a", "juego", g, d)
    # negocio asequible en la galeria de ejemplos
    g = crear_personaje(G()).w(30)
    andar(g, EJ.path(EJ.start, (5, 4)))
    g.k("E", gap=12).shot("07-05_negocio_ejemplo_en_venta")
    g.k("ENTER", gap=12).shot("07-06_negocio_comprado")
    g.k("ENTER", gap=12).shot("07-07_renta_subida")
    g.k("S", gap=8).k("ENTER", gap=12).k("ENTER", gap=12).shot("07-08_renta_bajada")
    g.k("S", gap=8).k("ENTER", gap=12).shot("07-09_negocio_cerrado")
    run("s07b", "juego", g, d, args=EJEMPLOS, env={"CAPTURAS_TITULO": "1"})


def s08_combate():
    d = f"{OUT}/08_juego_combate"
    g = crear_personaje(G()).w(30)
    g.shot("08-01_galeria_de_ejemplos")
    p = EJ.path(EJ.start, (6, 1))
    andar(g, p[:-1])
    g.shot("08-02_junto_al_slime")
    g.raw(f"key {p[-1]} 2"); g.w(30).shot("08-03_combate_empieza")
    g.k("S", gap=10).shot("08-04_cursor_pocion")
    g.k("S", gap=10).shot("08-05_cursor_huir")
    g.k("S", gap=10).shot("08-06_cursor_vuelve_arriba")
    g.k("ENTER", gap=12).sl(400).shot("08-07_tras_atacar")
    g.k("ENTER", gap=12).sl(600).shot("08-08_tras_la_victoria")
    g.k("TAB", gap=15).shot("08-09_inventario_tras_recoger_objetos")
    g.k("TAB", gap=12)
    # puerta a la mazmorra
    andar(g, EJ.path((5, 1), (6, 4)))
    g.k("E", gap=10).sl(1500).shot("08-10_mazmorra_de_rendimiento")
    run("s08", "juego", g, d, args=EJEMPLOS, env={"CAPTURAS_TITULO": "1"})


def s09_game_over():
    d = f"{OUT}/09_juego_game_over"
    # 1) partida guardada en la galeria; 2) se deja a 1 PV con un slime duro
    g = crear_personaje(G()).w(30).k("F5", gap=20)
    run("s09a", "juego", g, "/home/claude/work/tmp_s09_save", args=EJEMPLOS,
        env={"CAPTURAS_TITULO": "1"})
    save = os.path.join(BUILD, "saves")
    files = [os.path.join(r, f) for r, _, fs in os.walk(save) for f in fs]
    assert files, "no se guardo la partida"
    for f in files:
        data = json.load(open(f))
        data["playerHealth"] = 1
        for en in data.get("enemies", []):
            en["health"] = 999
        json.dump(data, open(f, "w"))
    backup = "/home/claude/work/saves_gameover"
    shutil.rmtree(backup, ignore_errors=True); shutil.copytree(save, backup)

    def pelea(g):
        crear_personaje(g).w(30).k("F9", gap=30).sl(800)
        p = EJ.path(EJ.start, (6, 1))
        andar(g, p)
        g.w(30).k("S", gap=10)          # POCION: no hay, pero pasa el turno
        for _ in range(30):
            g.k("ENTER", gap=14).sl(250)
        return g

    g = pelea(G()).shot("09-00_game_over_HOY_no_se_dibuja")
    shutil.rmtree(save, ignore_errors=True); shutil.copytree(backup, save)
    run("s09b", "juego", g, d, args=EJEMPLOS, keep_saves=True, env={"CAPTURAS_TITULO": "1", "CAPTURAS_COMO_HOY": "1"})
    g = pelea(G()).shot("09-01_game_over")
    g.k("S", gap=10).shot("09-02_game_over_cursor_titulo")
    g.k("W", gap=10).k("ENTER", gap=12).sl(1200).shot("09-03_game_over_cargar_partida")
    shutil.rmtree(save, ignore_errors=True); shutil.copytree(backup, save)
    run("s09c", "juego", g, d, args=EJEMPLOS, keep_saves=True, env={"CAPTURAS_TITULO": "1"})


ESCENAS = {"s01": s01_titulo, "s02": s02_nueva_partida, "s03": s03_prologo_ciudad,
           "s04": s04_menu_jugador, "s05": s05_pausa, "s06": s06_tienda,
           "s07": s07_negocio, "s08": s08_combate, "s09": s09_game_over}



def s10_motor_editor():
    d = f"{OUT}/10_editor_motor_editor"
    g = G().w(45)
    g.shot("10-01_editor_al_abrir_boundington")
    g.raw("click 45 11").w(12).shot("10-02_menu_archivo")
    g.raw("move 60 44").w(30).shot("10-03_menu_archivo_tooltip_guardar")
    g.raw("click 118 11").w(12).shot("10-04_menu_editar")
    g.raw("click 172 11").w(12).shot("10-05_menu_ver")
    g.raw("click 240 68").w(15).shot("10-06_ayuda_de_controles")
    g.k("F1", gap=10)
    g.raw("move 25 419").w(40).shot("10-07_tooltip_herramienta_pintar")
    g.raw("click 1090 33").w(12).shot("10-08_inspector_del_mapa")
    g.raw("click 25 475").w(12).raw("move 700 60").w(10).shot("10-09_herramienta_objetos")
    g.raw("click 100 508").w(8).raw("char taber").w(12).shot("10-10_buscar_objeto")
    g.raw("click 60 543").w(12).raw("move 700 60").w(8).shot("10-11_objeto_elegido")
    g.raw("click 560 250").w(15).shot("10-12_objeto_colocado_cambios_sin_guardar")
    g.raw("move 549 253").w(30).shot("10-13_tooltip_objeto_en_el_mapa")
    g.raw("click 25 419").w(10).raw("click 90 413").w(10).raw("click 610 300").w(15)
    g.raw("move 700 60").w(10).shot("10-14_pintar_baldosa")
    g.raw("click 1165 33").w(12).raw("click 1083 171").w(15).shot("10-15_capas_nueva_capa")
    g.raw("click 1100 351").w(15).shot("10-16_capas_borrar_capa_modal")
    g.raw("click 518 355").w(12)
    g.raw("click 45 11").w(12).raw("click 60 68").w(15).shot("10-17_cerrar_mapa_con_cambios_modal")
    g.raw("click 595 351").w(15).raw("move 700 400").w(8).shot("10-18_sin_mapa_abierto")
    g.raw("click 158 67").w(8).raw("char interior").w(12).shot("10-19_buscar_mapa")
    g.raw("click 90 150").w(25).raw("move 700 400").w(8).shot("10-20_abrir_otro_mapa")
    run("s10", "motor_editor", g, d, args=["--proyecto", "boundington"], timeout=100)


def s10b_motor_editor_variantes():
    d = f"{OUT}/10_editor_motor_editor"
    g = G().w(45).shot("10-21_editor_ui_scale_2")
    run("s10b", "motor_editor", g, d, args=["--proyecto", "boundington", "--ui-scale", "2"], timeout=100)
    g = G().w(45).shot("10-22_editor_proyecto_ejemplos")
    g.raw("click 25 475").w(12).raw("move 700 60").w(10).shot("10-23_editor_ejemplos_objetos")
    run("s10c", "motor_editor", g, d, args=["--proyecto", "ejemplos"], timeout=100)
    g = G().w(45).shot("10-24_editor_sin_proyecto")
    run("s10d", "motor_editor", g, d, args=[], timeout=100)

ESCENAS.update({"s10": s10_motor_editor, "s10b": s10b_motor_editor_variantes})


def s11_level_editor():
    d = f"{OUT}/11_editor_level_editor"
    g = G().w(40).shot("11-01_hub_de_proyectos")
    g.k("S", gap=10).shot("11-02_hub_otro_proyecto")
    g.k("N", gap=10).shot("11-03_nuevo_proyecto")
    for ch in "MIJUEGO":
        g.k(ch, gap=4)
    g.shot("11-04_nuevo_proyecto_tecleando_id")
    g.k("ESC", gap=10).k("W", gap=10)
    g.k("B", gap=10).sl(4000).shot("11-05_compilar_build")
    g.k("ENTER", gap=15)                      # cierra el mensaje
    g.k("ENTER", gap=15).sl(2500)             # abre Boundington
    g.raw("move 900 560").w(10).shot("11-06_edicion_al_abrir")
    g.k("3", gap=10).shot("11-07_colocar_objeto_categorias")
    g.k("E", 2, gap=6).shot("11-08_categoria_elegida")
    g.k("ENTER", gap=10).shot("11-09_subcategorias")
    g.k("ENTER", gap=10).shot("11-10_objetos_de_la_categoria")
    g.k("B", gap=10)
    for ch in "TENDERO":
        g.k(ch, gap=4)
    g.shot("11-11_buscar_tendero")
    g.k("ENTER", gap=10)
    g.k("ESC", 3, gap=10).shot("11-12_menu_de_objetos_cerrado")
    g.k("1", gap=10).k("E", 4, gap=6).shot("11-13_pintar_tile_paleta")
    g.raw("click 900 560").w(10).shot("11-14_tile_pintado")
    g.k("C", gap=10).shot("11-15_capa_siguiente")
    g.k("CTRL+N", gap=10).shot("11-16_capa_nueva")
    g.raw("move 560 520").w(4).k("R", gap=8).raw("move 720 600").w(4).k("R", gap=10)
    g.shot("11-17_seleccion_rectangulo")
    g.k("CTRL+C", gap=10).raw("move 1000 620").w(6).k("CTRL+V", gap=10).shot("11-18_copiar_y_pegar")
    g.k("CTRL+Z", gap=10).shot("11-19_deshacer")
    g.raw("click 795 212 right").w(15).shot("11-20_propiedades_de_instancia")
    g.k("S", gap=8).shot("11-21_propiedades_otro_campo")
    g.k("ENTER", gap=10).shot("11-22_propiedades_editar_campo")
    g.k("ESC", gap=10).k("ESC", gap=12)
    g.k("V", gap=12).shot("11-23_validar")
    g.k("KP_ADD", gap=10).k("KP_ADD", gap=10).shot("11-24_zoom")
    g.k("0", gap=10).k("HOME", gap=10)
    g.k("G", gap=12).shot("11-25_guardar")
    g.k("PERIOD", gap=20).sl(1500).shot("11-26_siguiente_escenario")
    g.raw("click 700 450").w(10)
    g.k("M", gap=15).shot("11-27_volver_con_cambios_pide_confirmar")
    g.k("ESC", gap=15).sl(800).shot("11-28_de_vuelta_en_el_hub")
    g.k("ESC", gap=15)
    run("s11", "level_editor", g, d, timeout=150)


ESCENAS.update({"s11": s11_level_editor})


TABERNA = Level("assets/levels/interior_taberna.json")


def s03b_mapa_del_mundo():
    d = f"{OUT}/03_juego_prologo_y_ciudad"
    g = prologo_rapido(crear_personaje(G()).w(30))
    g.k("E", gap=10).sl(2500).shot("03-20_camino_a_egaroth_mapa_del_mundo")
    g.k("ENTER", 3, gap=12).sl(500)
    andar(g, ["S", "S", "D", "D", "D"])
    g.w(20).shot("03-21_mapa_del_mundo_caminando")
    run("s03b", "juego", g, d)


def s12_precuela():
    d = f"{OUT}/12_juego_precuela_taberna"
    g = G().w(40).shot("12-01_precuela_llegada")
    for i in range(2, 5):
        g.k("ENTER", gap=12).sl(300).shot(f"12-{i:02d}_precuela_dialogo")
    g.k("ENTER", 3, gap=12).sl(400).shot("12-05_taberna_exploracion")
    andar(g, TABERNA.path(TABERNA.start, (6, 5)))
    g.shot("12-06_junto_a_skilla")
    g.k("E", gap=12).shot("12-07_hablar_con_skilla")
    g.k("ENTER", gap=12).shot("12-08_dialogo_cerrado")
    g.k("E", gap=12).shot("12-09_skilla_segunda_vez_tienda")
    g.k("E", gap=12).w(10)
    ben = TABERNA.find("ben_kafka")[0]
    andar(g, TABERNA.path((6, 5), TABERNA.adjacent_free(ben, prefer=("E", "S", "N", "W"))[0]))
    g.shot("12-10_junto_a_ben_kafka")
    g.k("E", gap=12).shot("12-11_encargo_de_ben_kafka")
    g.k("ENTER", gap=12).shot("12-12_tras_el_encargo")
    g.k("TAB", gap=15).shot("12-13_TAB_sin_titulo")
    run("s12", "juego", g, d, args=["--precuela"])


ESCENAS.update({"s03b": s03b_mapa_del_mundo, "s12": s12_precuela})


if __name__ == "__main__":
    pedidas = sys.argv[1:] or list(ESCENAS)
    for n in pedidas:
        ESCENAS[n]()
