"""Habitaciones mineras y continuación de la fuga del Cojo."""
from .comun import Mapa,_T

def mina():
    n=Mapa('mina_gorran','La mina de Gorran',42,42)
    for z in [('entrada','Control y relevo',17,35,9,6,'tierra_piedras'),
              ('maquinas','Sala de maquinas y elevador',15,25,13,8,'losa_oscura'),
              ('acopio','Acopio y carga',3,25,10,9,'tierra_piedras'),
              ('sectarios','Cuartos de los sectarios',29,25,10,9,'losa_oscura'),
              ('clasificacion','Clasificacion del mineral',14,16,14,7,'piedra_antigua'),
              ('taller','Taller de reparaciones',29,15,10,8,'losa_oscura'),
              ('extraccion','Camara de extraccion',15,4,13,10,'tierra_piedras'),
              ('literas','Literas y botiquin',3,4,10,10,'losa_oscura'),
              ('veta','Frente de veta y puntales',29,4,10,9,'tierra_piedras'),
              ('comedor','Comedor y cocina de mineros',3,16,10,7,'losa_oscura')]:n.sala(*z)
    n.camino([(21,2),(21,39)],3,'tierra_piedras')
    for y in (9,20,29):n.camino([(8,y),(34,y)],3,'tierra_piedras')
    for x in (8,34):n.camino([(x,9),(x,29)],3,'tierra_piedras')
    n.start=(21,39);n.reservar((21,39),(21,3))
    n.portal('paso_mina_a_aldea',21,40,'cueva_gorran',(31,3),'Bajar a la Aldea Baja')
    n.portal('paso_mina_a_mina2',21,2,'mina2_gorran',(21,38),'Seguir hacia la mina vieja')
    props=[('cabrestante',25,28),('rollo_cuerda',25,30),('pozo',18,27),('carro',6,27),('cajas_almacen',5,31),('sacos',10,31),
           ('circulo_velas',36,27),('velas_derretidas',37,31),('cama_pobre_cap',30,26),('cama_pobre_peus',30,27),('estandarte',37,26),
           ('mesa',17,17),('taburete',17,18),('carbon',24,17),('pico',25,18),('yunque',31,17),('banco_herrero',36,17),('pala',37,20),
           ('pico',24,5),('viga',25,12),('viga',17,12),('cama_pobre_cap',4,5),('cama_pobre_peus',4,6),('cama_pobre_cap',10,5),('cama_pobre_peus',10,6),
           ('baul',4,12),('cubo',11,12),('viga',37,6),('pico',30,6),('carro',30,10),('mesa',4,17),('barril',11,17),('brasero',4,21)]
    for oid,x,y in props:n.pon(oid,x,y)
    for x,y in [(19,36),(11,26),(31,24),(27,15),(13,5),(4,15),(28,34)]:
        if n.andable(x,y):n.pon('antorcha',x,y)
    for x,y,txt in [(21,37,'Control de relevo: se ve el acceso a la Aldea y el paso a la segunda mina.'),
                    (21,30,'Sala de maquinas: cabrestante, pozo y cuerda; espacio para un rescate o sabotaje.'),
                    (7,30,'Carga: mineral clasificado y carros, con paso por ambos extremos.'),
                    (34,30,'Los sectarios tienen aqui sus cuartos y material: extorsionan a Joel sin que Gorran lo sepa.'),
                    (21,20,'Clasificacion: libros de carga y turno; propone una pista del traslado del dinero.'),
                    (34,18,'Taller: reparar herramienta, conseguir una cuerda o negociar el paso.'),
                    (21,9,'Extraccion: frente amplio con varias entradas, no un corredor vacio.'),
                    (7,9,'Literas: mineros y herido; zona de conversacion y descanso propuesta.'),
                    (34,8,'Veta: restos del trabajo y puntales; hay vuelta por el taller.'),
                    (7,19,'Comedor: turno de mineros, pista del Cojo que cruza hacia la mina vieja.'),
                    (21,4,'FUGA DEL COJO: continua a mina2_gorran, despues galerias mineras, grieta y casa del Oeste.')]:n.nota(x,y,txt)
    return n

def mina2():
    n=Mapa('mina2_gorran','La mina vieja del Cojo',42,42)
    for z in [('relevo','Descenso de la segunda mina',16,34,11,7,'tierra_piedras'),
              ('corte','Gran corte de piedra',15,23,13,9,'tierra_piedras'),
              ('maquinista','Cuarto del maquinista',3,25,10,10,'losa_oscura'),
              ('refugio','Refugio y suministros del Cojo',29,26,10,10,'losa_oscura'),
              ('desague','Camara del desague',3,15,10,8,'barro'),
              ('puente','Paso de las traviesas',15,15,13,7,'tierra_piedras'),
              ('puntales','Bosque de puntales',29,14,10,9,'tierra_piedras'),
              ('registro','Antigua oficina de concesiones',3,3,10,10,'piedra_antigua'),
              ('desprendido','Frente antiguo y derrumbe',15,3,13,10,'tierra_piedras'),
              ('escape','Galeria de salida',29,2,11,10,'losa_oscura')]:n.sala(*z)
    n.camino([(21,38),(21,7)],3,'tierra_piedras')
    for y in (8,19,30):n.camino([(7,y),(35,y)],3,'tierra_piedras')
    for x in (7,35):n.camino([(x,8),(x,30)],3,'tierra_piedras')
    n.camino([(35,8),(38,8),(38,5)],3,'losa_oscura')
    # Un desague real con un puente; la ruta seca rodea la camara.
    n.rect(16,16,11,5,_T['rio']);n.rect(20,16,3,5,_T['puente_piedra'])
    n.start=(21,38);n.reservar((21,38),(37,5))
    n.portal('paso_mina2_a_mina',21,40,'mina_gorran',(21,3),'Volver a la primera mina')
    n.portal('paso_mina2_a_galeria',38,5,'galeria_minas_cojo',(2,20),'Cruzar las galerias mineras del Cojo')
    for oid,x,y in [('cabrestante',5,27),('mesa',10,27),('rollo_cuerda',10,33),('cama_pobre_cap',30,28),('cama_pobre_peus',30,29),
                    ('baul',37,28),('barril',37,34),('pila_agua',4,17),('cubo',11,20),('viga',30,16),('viga',37,16),('viga',37,21),
                    ('escritorio',5,5),('estanteria',10,4),('silla',5,6),('cajas',11,11),('huesos',18,4),('columna_rota',25,11),('cajas',31,4)]:n.pon(oid,x,y)
    for x,y,txt in [(21,36,'Segunda mina: la ruta de fuga del Cojo continua y puede seguirse sin combate obligatorio.'),
                    (21,27,'Gran corte: espacio amplio para un encuentro o una persecucion, con dos desvíos.'),
                    (7,30,'Cuarto de maquinaria antigua: herramientas y restos de turnos.'),
                    (34,32,'Refugio del Cojo: provisiones para salir; propuesta de indicios, no botin concedido.'),
                    (7,19,'Desague: rodeo por seco si no se cruza el puente central.'),
                    (21,19,'Puente sobre agua: el peligro esta delimitado; no se camina sobre el cauce.'),
                    (34,19,'Puntales: ruta lateral de mineros hacia la salida.'),
                    (7,8,'Oficina antigua: documento de concesiones y pistas de conexiones, por escribir.'),
                    (21,8,'Derrumbe: la veta esta agotada; el paso norte sigue abierto por el lateral.'),
                    (35,6,'La galeria de minas lleva a una grieta en la ladera y desde alli a la casa de Boundington.')]:n.nota(x,y,txt)
    return n

def galeria():
    n=Mapa('galeria_minas_cojo','Las galerias de salida del Cojo',42,42)
    for z in [('entrada','Portillo de la mina vieja',1,16,10,9,'tierra_piedras'),
              ('bifurcacion','Cambio de agujas',13,15,14,11,'piedra_antigua'),
              ('salida','Galeria hacia la ladera',29,16,12,9,'tierra_piedras'),
              ('deposito','Deposito de traviesas',4,4,11,8,'losa_oscura'),
              ('agua','Veta inundada y bombeo',18,3,9,10,'barro'),
              ('puesto','Puesto clandestino',30,4,10,8,'losa_oscura'),
              ('herramientas','Taller de entibacion',4,29,11,9,'losa_oscura'),
              ('escondite','Camara de mercancias selladas',18,29,10,9,'losa_oscura'),
              ('frente','Frente ciego',31,29,9,9,'tierra_piedras')]:n.sala(*z)
    for y in (8,20,33):n.camino([(8,y),(35,y)],3,'tierra_piedras')
    for x in (8,22,35):n.camino([(x,8),(x,33)],3,'tierra_piedras')
    n.camino([(2,20),(38,20)],3,'tierra_piedras')
    n.start=(2,20);n.reservar((2,20),(37,20))
    n.portal('paso_galeria_cojo_a_mina2',1,20,'mina2_gorran',(37,5),'Volver a la mina vieja')
    n.portal('paso_galeria_cojo_a_grieta',39,20,'grieta_cojo',(3,12),'Salir por la grieta de la montana')
    for oid,x,y in [('viga',5,5),('viga',13,5),('cajas_almacen',5,10),('cabrestante',20,5),('cubo',25,11),('mesa',32,5),('silla',33,5),
                    ('barril',38,10),('banco_herrero',5,30),('pico',12,30),('rollo_cuerda',12,36),('fardos',20,30),('sacos',26,36),('huesos',38,36),('carro',18,17)]:n.pon(oid,x,y)
    for z in n.zonas:
        x,y=z['centro'];n.nota(x,y,z['nombre']+': reserva de escena para la persecucion y los indicios del Cojo.')
    return n
