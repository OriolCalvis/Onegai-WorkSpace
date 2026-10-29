"""Galerías de contrabando: trabajo, rutas de servicio y embarcadero.

Se conservan las tres conexiones del primer borrador. Los posibles papeles
narrativos se exportan como notas para revisión, sin añadir beats ni PNJ nuevos.
"""
from .comun import Mapa
from gen_niveles_av19 import _T


def construir():
    n = Mapa('galerias_gorran', 'Galerías de contrabando', 42, 42)
    n.start = (37, 7)
    # Cámaras de tamaños y siluetas distintos, excavadas alrededor de dos
    # circuitos. El espacio intermedio sigue siendo roca, no una cuadrícula.
    salas = [
        ('gola', 'Vestíbulo de la Gola', 2, 2, 7, 7, 'tierra_piedras'),
        ('almacen', 'Almacén seco de fardos', 3, 11, 9, 8, 'losa_oscura'),
        ('cuentas', 'Mesa de cuentas y manifiestos', 16, 2, 9, 6, 'int_taulons_foscos'),
        ('aduana', 'Aduana de la Aldea Baja', 30, 3, 10, 8, 'losa_clara'),
        ('clasificacion', 'Nave de clasificación de mercancías', 14, 11, 13, 9, 'losa_oscura'),
        ('cojo', 'Caja y reservas del Cojo', 3, 21, 8, 7, 'int_taulons_foscos'),
        ('clandestino', 'Apartadero clandestino', 13, 22, 7, 5, 'tierra_piedras'),
        ('taller', 'Taller de embalajes y aparejos', 29, 15, 10, 9, 'int_taulons'),
        ('transbordo', 'Patio de transbordo', 22, 24, 7, 7, 'tierra_piedras'),
        ('refugio', 'Refugio de los trabajadores', 5, 30, 9, 6, 'tierra_musgosa'),
        ('embarcadero', 'Embarcadero del río subterráneo', 16, 32, 14, 5, 'tarima_madera'),
        ('reserva', 'Reserva de carga y víveres', 31, 28, 8, 6, 'losa_oscura'),
    ]
    for clave, titulo, x, y, w, h, suelo in salas:
        n.sala(clave, titulo, x, y, w, h, suelo)
    # Los lóbulos rompen los rectángulos y hacen reconocibles los espacios.
    n.piso(11, 13, 3, 4, 'losa_oscura')
    n.piso(19, 9, 5, 3, 'losa_oscura')
    n.piso(32, 12, 5, 3, 'int_taulons')
    n.piso(3, 23, 2, 4, 'int_taulons_foscos')
    n.piso(17, 30, 8, 3, 'tarima_madera')

    # Anillo de mercancías: aduana -> cuentas -> almacén -> nave -> aduana.
    # Anillo del agua: nave -> taller/reserva -> muelle -> refugio/Cojo -> nave.
    # El apartadero corta el segundo anillo sin obligar a pasar por la aduana.
    rutas = [
        [(37, 7), (34, 7), (34, 5), (20, 5)],
        [(20, 5), (10, 5), (10, 6), (5, 6)],
        [(7, 6), (7, 15), (20, 15)],
        [(20, 5), (20, 15)],
        [(34, 7), (34, 19), (20, 19), (20, 15)],
        [(7, 15), (7, 24), (16, 24), (16, 18)],
        [(7, 24), (9, 24), (9, 33), (23, 33)],
        [(20, 18), (25, 18), (25, 27), (25, 34), (21, 34), (21, 36)],
        [(34, 19), (34, 31), (25, 31), (25, 34)],
        [(16, 24), (16, 28), (25, 28)],
    ]
    for puntos in rutas:
        n.camino(puntos, ancho=3)
    # Conservar la tarima del muelle y el material distintivo de cada cámara
    # después de excavar las uniones; nunca se tapan los pasos con muros.
    for clave, titulo, x, y, w, h, suelo in salas:
        n.piso(x, y, w, h, suelo)
    n.piso(20, 35, 3, 2, 'tarima_madera')

    # El agua ocupa el extremo sur completo. La orilla es inequívoca: no es
    # otro pasillo. La barca reposa sobre una única celda somera junto al muelle.
    for y in range(37, 41):
        for x in range(11, 33):
            n.g[y][x] = _T['rio']
    n.g[37][24] = _T['agua_somera']
    n.g[36][15] = _T['rio']
    n.g[36][30] = _T['rio']

    # Reserva del eje de cada recorrido: las cargas no forman tapones, incluso
    # con ambos guardias bloqueando su casilla. Centros y llegadas quedan libres.
    for puntos in rutas:
        for (x, y), (bx, by) in zip(puntos, puntos[1:]):
            for xx in range(min(x, bx), max(x, bx) + 1):
                for yy in range(min(y, by), max(y, by) + 1):
                    n.reservar((xx, yy))
    n.reservar((37, 7), (5, 5), (21, 36))
    for zona in n.zonas:
        n.reservar(tuple(zona['centro']))
    n.portal('paso_galerias_a_aldea', 38, 5, 'cueva_gorran', (11, 53), 'Volver a la Aldea Baja')
    n.portal('paso_galerias_a_gola', 4, 4, 'antecova_gorran', (10, 8), 'Volver a la Gola Fosca')
    n.portal('paso_galerias_a_rio', 21, 36, 'rio_gorran', (11, 3), 'Embarcar río abajo')

    # Atrezzo por oficio: cada cámara se entiende sin depender de su rótulo.
    objetos = [
        # Aduana: registro, inspección y carga retenida, fuera de la llegada.
        ('escritorio', 31, 4), ('silla', 31, 6), ('estanteria', 36, 4),
        ('cajas_almacen', 38, 9), ('sacos', 37, 9), ('bandido_gorran', 32, 8),
        # Contabilidad: archivo, mesa y una caja separada del Cojo.
        ('escritorio', 17, 3), ('silla', 18, 3), ('estanteria', 23, 3),
        ('cofre', 23, 6), ('banco', 17, 7),
        # Mercancía seca en estibas; se puede rodear por los dos costados.
        ('fardos', 4, 12), ('fardos', 5, 12), ('sacos', 9, 12),
        ('cajas_almacen', 4, 17), ('barriles_pila', 10, 17), ('fardos', 10, 14),
        # Nave: mesas de clasificar, lotes entrantes y salientes en extremos.
        ('mesa', 15, 12), ('silla', 16, 12), ('cajas', 25, 12),
        ('sacos', 24, 14), ('fardos', 15, 17), ('barril', 24, 17),
        # Caja del Cojo: pequeña sala de apuestas y caja, no un cofre aislado.
        ('mesa_cartas', 4, 22), ('taburete', 5, 22), ('cofre', 4, 26),
        ('barril', 9, 22), ('baul', 10, 26),
        # Apartadero sin guardar: mercancía oculta, claramente una ruta menor.
        ('fardos', 14, 23), ('cajas', 18, 25), ('rollo_cuerda', 14, 25),
        # Taller: herramientas y material de reparación, nunca en el carril.
        ('yunque', 30, 16), ('mesa', 32, 16), ('rollo_cuerda', 37, 16),
        ('barril', 37, 21), ('cajas', 30, 22), ('cubo', 32, 22),
        # Área de transbordo: carga lista, pasillo ancho y segundo guardia.
        ('cajas_almacen', 23, 25), ('fardos', 27, 25), ('sacos', 27, 29),
        ('bandido_de_gorran', 23, 29),
        # Refugio: camas, mesa, agua y baúl; no es otro almacén.
        ('cama_pobre_cap', 6, 31), ('cama_pobre_peus', 6, 32),
        ('cama_pobre_cap', 12, 31), ('cama_pobre_peus', 12, 32),
        ('mesa', 6, 34), ('taburete', 7, 34), ('cubo', 12, 34),
        # Reserva fría: barriles y sacos junto al segundo acceso al muelle.
        ('barriles_pila', 32, 29), ('barril', 37, 29), ('sacos', 37, 32),
        ('cajas_almacen', 32, 32),
        # Carga y barca del río; las dos piezas de pontón flanquean la puerta.
        ('ponton', 20, 36), ('ponton', 22, 36), ('barca', 24, 37),
        ('rollo_cuerda', 17, 35), ('fardos', 28, 35), ('barril', 28, 33),
    ]
    for oid, x, y in objetos:
        n.pon(oid, x, y)

    notas = [
        (36, 8, 'ADUANA: propuesta de control de manifiestos, soborno o disfraz; no hay nueva regla implementada.'),
        (21, 6, 'CUENTAS: manifiestos y registro de cargamentos; posible pista propuesta para conectar negocios de Gorran.'),
        (8, 16, 'ALMACEN: dos contrabandistas trabajando; uno puede vender el paso. PNJ con nombre pendiente de validar.'),
        (21, 16, 'CLASIFICACION: la carga sigue cuentas -> nave -> transbordo -> embarcadero; ruta de trabajo visible.'),
        (8, 25, 'COJO: caja de las apuestas y reservas. Se conserva su funcion propuesta; no se mueve al Cojo desde su escena.'),
        (17, 23, 'APARTADERO: propuesta de contrabando sin registrar; permite evitar el transbordo por el oeste.'),
        (35, 20, 'TALLER: reparacion de cajones y cuerdas; posible ayuda o intercambio propuesto, sin activar una aventura.'),
        (26, 26, 'TRANSBORDO: inspeccion del camino de carga; puede rodearse por taller/reserva o por apartadero/refugio.'),
        (10, 34, 'REFUGIO: trabajadores agotados, comida y camas; posible conversacion de ayuda, pendiente de canon.'),
        (36, 30, 'RESERVA: bultos para el rio; segundo acceso al muelle para que el guardia no sea el unico paso.'),
        (21, 34, 'EMBARCADERO: Victor y Asdrugal y su barca; aqui empieza el rio hacia Boundington. PNJ solo como nota.'),
        (5, 6, 'GOLA: llegada desde el paso oculto, conectada al almacen y al circuito de cuentas; no es un callejon sin salida.'),
    ]
    for x, y, texto in notas:
        n.nota(x, y, texto)
    # Los circuitos se exportan también como evidencia geométrica para la
    # revisión del generador, sin darles efectos de aventura.
    n.circuitos = [
        {'id': 'mercancias', 'puntos': [(34, 7), (34, 5), (20, 5), (7, 5), (7, 15), (20, 15), (20, 19), (34, 19), (34, 7)]},
        {'id': 'servicio_rio', 'puntos': [(20, 19), (34, 19), (34, 31), (25, 31), (25, 33), (9, 33), (9, 24), (7, 24), (7, 15), (20, 15), (20, 19)]},
    ]
    return n
