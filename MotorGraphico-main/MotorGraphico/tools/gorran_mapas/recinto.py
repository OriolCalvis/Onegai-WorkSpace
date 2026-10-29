"""Fortaleza de Gorran: edificio continuo excavado en roca, en una planta.

El patio articula las cuatro alas; las puertas son huecos anchos en muros,
no transiciones ni pasillos de cueva. Los nombres y encuentros siguen en
notas: este plano no crea PNJ ni escribe la aventura.
"""
from .comun import Mapa


def _huecos_y(x, *tramos):
    return {(x, y) for inicio, fin in tramos for y in range(inicio, fin + 1)}


def _huecos_x(y, *tramos):
    return {(x, y) for inicio, fin in tramos for x in range(inicio, fin + 1)}


def _planta(n):
    # Una única huella rectangular, con la roca como envolvente exterior.
    n.piso(12, 8, 55, 57)
    n.piso(2, 32, 10, 9, 'tierra_canon')
    n.muro(11, 7, 57, 1)
    n.muro(11, 65, 57, 1)
    n.muro(11, 8, 1, 57, _huecos_y(11, (34, 38)))
    n.muro(67, 8, 1, 57)

    # Ala norte: despacho y tesoro, sala de audiencia, cámara de la Furia.
    n.muro(25, 8, 1, 16, _huecos_y(25, (12, 14)))
    n.muro(53, 8, 1, 16, _huecos_y(53, (12, 14)))
    n.muro(12, 16, 13, 1, _huecos_x(16, (17, 19)))
    n.muro(12, 24, 55, 1, _huecos_x(24, (18, 20), (37, 41), (59, 61)))

    # Alas laterales pegadas al patio; acceso directo y paso entre salas.
    n.muro(25, 25, 1, 26, _huecos_y(25, (27, 29), (34, 38), (45, 47)))
    n.muro(53, 25, 1, 26, _huecos_y(53, (29, 31), (44, 46)))
    n.muro(12, 32, 13, 1, _huecos_x(32, (17, 19)))
    n.muro(12, 41, 13, 1, _huecos_x(41, (17, 19)))
    n.muro(54, 38, 13, 1, _huecos_x(38, (59, 61)))

    # Dependencias domésticas y de servicio, también en esta planta.
    n.muro(12, 51, 55, 1, _huecos_x(51, (17, 19), (30, 32), (44, 46), (58, 60)))
    n.muro(25, 52, 1, 13, _huecos_y(25, (58, 60)))
    n.muro(38, 52, 1, 13, _huecos_y(38, (58, 60)))
    n.muro(52, 52, 1, 13, _huecos_y(52, (54, 56)))
    n.muro(39, 59, 13, 1, _huecos_x(59, (44, 46)))
    n.muro(53, 58, 14, 1, _huecos_x(58, (55, 57), (62, 64)))
    n.muro(60, 59, 1, 6)


def _salas(n):
    salas = (
        ('oficina', 'Oficina y archivo', 12, 8, 13, 8, 'int_taulons_foscos'),
        ('tesoreria', 'Tesorería del recinto', 12, 17, 13, 7, 'losa_clara'),
        ('gorran', 'Sala de Gorran y piedra-altar', 26, 8, 27, 16, 'piedra_antigua'),
        ('furia', 'Cámara de la Furia de Alaro', 54, 8, 13, 16, 'losa_clara'),
        ('guardia', 'Cuerpo de guardia', 12, 25, 13, 7, 'losa_oscura'),
        ('entrada', 'Portón y vestíbulo de recepción', 12, 33, 13, 8, 'losa_clara'),
        ('armeria', 'Armería y equipo de la guardia', 12, 42, 13, 9, 'losa_oscura'),
        ('patio', 'Patio de armas y distribución', 26, 25, 27, 26, 'adoquin_gastado'),
        ('subastas', 'Salón de subastas y del trato', 54, 25, 13, 13, 'int_taulons_foscos'),
        ('fiesta', 'Salón de fiestas', 54, 39, 13, 12, 'int_taulons'),
        ('calabozo', 'Calabozo y custodia', 12, 52, 13, 13, 'losa_oscura'),
        ('barracas', 'Barracas de la guardia', 26, 52, 12, 13, 'int_taulons_foscos'),
        ('cocina', 'Cocina y servicio del salón', 39, 52, 13, 7, 'losa_clara'),
        ('despensa', 'Despensa y provisiones', 39, 60, 13, 5, 'int_taulons_foscos'),
        ('casa', 'Casa de Gorran: estancia privada', 53, 52, 14, 6, 'int_taulons'),
        ('dormitorio', 'Dormitorio de Gorran', 53, 59, 7, 6, 'int_taulons'),
        ('bano', 'Baño y aseo de la casa', 61, 59, 6, 6, 'losa_clara'),
    )
    for clave, titulo, x, y, w, h, suelo in salas:
        n.sala(clave, titulo, x, y, w, h, suelo)
    # Una alfombra en la misma cota conduce al jefe; no hay gradas ni pisos.
    n.piso(38, 16, 3, 8, 'int_catifa_vermella_c')
    n.piso(36, 12, 7, 4, 'piedra_antigua')
    for z in n.zonas:
        n.reservar(tuple(z['centro']))
    # Todos los accesos conservan carriles de tres casillas sin mobiliario.
    for x in range(3, 53):
        n.reservar((x, 35), (x, 36), (x, 37))
    for y in range(16, 51):
        n.reservar((38, y), (39, y), (40, y))


def _mesa(n, x, y, largo, familia='mesa_larga'):
    for k in range(largo):
        pieza = 'ini' if k == 0 else 'fi' if k == largo - 1 else 'mig'
        n.pon(f'{familia}_{pieza}', x + k, y)


def _mobiliario(n):
    # Despacho y tesorería: expedientes, caja y botín separados de la audiencia.
    for oid, x, y in (
        ('mesa_libro', 16, 11), ('silla_noble', 16, 10),
        ('estanteria_libros', 13, 9), ('estanteria_libros', 14, 9),
        ('estanteria_libros', 22, 9), ('caja_fuerte', 22, 19),
        ('cofre_tesoro', 14, 19), ('cofre', 14, 22), ('sacos_monedas', 22, 22),
        ('trono', 39, 10), ('estandarte', 33, 10), ('estandarte', 45, 10),
        ('banco_piedra', 37, 14), ('banco_piedra', 41, 14),
        ('candelabro', 31, 19), ('candelabro', 47, 19),
        ('cofre', 49, 10), ('estatua_santa', 61, 10),
        ('velas_derretidas', 57, 13), ('velas_derretidas', 63, 13),
        ('cofre_tesoro', 64, 20), ('estanteria_cadenas', 55, 20),
        ('mesa_cartas', 16, 27), ('taburete', 15, 27), ('taburete', 16, 29),
        ('barril', 22, 26), ('baul', 22, 30),
        ('estandarte', 13, 33), ('estandarte', 13, 40),
        ('banco', 16, 39), ('banco', 21, 39),
        ('armero_espadas', 13, 43), ('armero_espadas', 16, 43),
        ('armero_lanzas', 20, 43), ('armero_lanzas', 23, 43),
        ('barril_flechas', 14, 48), ('barril_flechas', 22, 48),
        ('maniqui_jubon', 16, 49), ('cofre', 21, 50),
        ('muneco', 28, 28), ('muneco', 31, 28), ('muneco', 34, 28),
        ('banco', 28, 48), ('banco', 34, 48),
        ('pozo', 45, 42), ('brasero', 29, 42), ('brasero', 49, 28),
        ('estandarte', 27, 26), ('estandarte', 51, 26),
        ('atril', 60, 27), ('candelabro', 55, 26), ('candelabro', 65, 26),
        ('cofre', 65, 29), ('maniqui_azul', 64, 27),
        ('cajas_almacen', 55, 37), ('cofre', 65, 37),
        ('estanteria_botellas', 55, 40), ('barril', 56, 40),
        ('atril_musica', 64, 40), ('candelabro', 65, 49),
        ('jaula', 13, 54), ('jaula', 16, 54), ('jaula', 22, 54),
        ('estanteria_cadenas', 13, 62), ('cubo', 22, 63), ('huesos', 14, 63),
        ('mesa', 22, 56), ('silla', 23, 56),
        ('baul', 27, 63), ('baul', 36, 63), ('perchero', 36, 56),
        ('hoguera', 40, 53), ('pila_agua', 50, 53), ('mesa_corte', 47, 56),
        ('cubo', 50, 57), ('barriles_pila', 40, 61), ('barriles_pila', 41, 61),
        ('sacos', 49, 61), ('sacos', 50, 61), ('cajas_almacen', 50, 63),
        ('mesa_redonda', 56, 54), ('silla_noble', 55, 54),
        ('silla_noble', 56, 55), ('estanteria_libros', 65, 53),
        ('perchero_ropa', 54, 57), ('mesa_vela', 64, 55),
        ('cama_noble_cap', 54, 60), ('cama_noble_peus', 54, 61),
        ('baul', 58, 60), ('armario_noble', 58, 63),
        ('pila_agua', 62, 60), ('cubo', 65, 60), ('estante_toallas', 65, 63),
    ):
        n.pon(oid, x, y)
    # Filas de compradores con pasillos a ambos lados y detrás del atril.
    for y in (30, 33):
        for x in (57, 59, 61, 63):
            n.pon('silla_noble', x, y)
    # Banquete con dos mesas; conserva un pasillo central en y45.
    for y in (42, 47):
        _mesa(n, 57, y, 6, 'mesa_banquete')
        for x in (58, 61):
            n.pon('silla_noble', x, y - 1)
            n.pon('silla_noble', x, y + 1)
    for x in (27, 30, 33, 36):
        n.pon('cama_pobre_cap', x, 53)
        n.pon('cama_pobre_peus', x, 54)
        n.pon('cama_pobre_cap', x, 60)
        n.pon('cama_pobre_peus', x, 61)


def construir():
    n = Mapa('recinto_gorran', 'La fortaleza de Gorran', 72, 72)
    n.start = (4, 36)
    _planta(n)
    _salas(n)
    n.portal('paso_recinto_a_aldea', 3, 36, 'cueva_gorran', (54, 39),
             'Volver a la Aldea Baja')
    n.nota(9, 36, 'Portón de la fortaleza: dos bandidos comprueban el salvoconducto.')
    n.nota(39, 14, 'Gorran junto a la piedra-altar. Audiencia y enfrentamiento, sin cambiar la trama.')
    n.nota(60, 15, 'La Furia de Alaro se expone en su pedestal. Cámara contigua a Gorran.')
    n.nota(60, 34, 'Subastas y comedia del arte: Joel comprador; Victor y Asdrugal le reconocen.')
    n.nota(18, 59, 'Calabozo: custodia de los personajes capturados; no cambia las condiciones narrativas.')
    n.nota(48, 45, 'Patio grande para recibir, entrenar y combatir. Anillo de puertas a las cuatro alas.')
    n.nota(57, 56, 'Casa del jefe: estancia privada, dormitorio y baño, todo en esta planta.')
    n.nota(43, 57, 'Servicio: cocina y despensa conectan las barracas con la casa sin cruzar la audiencia.')
    n.nota(59, 49, 'Salón de fiestas: banquete y músicos; conectado con la subasta y el patio.')
    _mobiliario(n)
    return n
