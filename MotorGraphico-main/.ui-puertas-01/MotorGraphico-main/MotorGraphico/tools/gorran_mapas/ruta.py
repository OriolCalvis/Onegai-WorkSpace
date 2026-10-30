"""La grieta de la ladera y la casa abandonada del barrio del Oeste."""
from .comun import Mapa

PUERTA_OESTE=(25,35)
LLEGADA_OESTE=(25,36)

def grieta():
    n=Mapa('grieta_cojo','La grieta oculta de la ladera',26,26)
    n.sala('cornisa','Cornisa resguardada de la montana',2,8,22,10,'tierra_piedras')
    n.sala('bosque','Refugio entre la maleza',6,3,9,6,'pradera')
    n.sala('entrada_casa','Entrada oculta tras la casa',18,17,6,6,'piedra_antigua')
    n.camino([(3,12),(22,12),(22,20)],3,'tierra_piedras')
    n.camino([(10,5),(10,12)],3,'sendero_tierra')
    n.start=(3,12);n.reservar((3,12),(21,20))
    n.portal('paso_grieta_a_galeria_cojo',2,12,'galeria_minas_cojo',(38,20),'Volver a las galerias mineras')
    n.portal('paso_grieta_a_casa_cojo',22,21,'casa_abandonada_cojo',(14,3),'Entrar por la trasera de la casa abandonada')
    for oid,x,y in [('arbol_seco',7,4),('mata',13,5),('barriles_rotos',5,15),('rollo_cuerda',19,18),('cajas',23,18)]:n.pon(oid,x,y)
    n.nota(10,6,'La grieta se abre a una cornisa de la montana; se ve la vegetacion y la salida trasera de la casa.')
    n.nota(17,13,'Ruta del Cojo: la galeria desemboca en la ladera; no es el rio ni una salida nueva del Ocaso.')
    n.nota(21,19,'La puerta oculta conecta con la casa abandonada del barrio del Oeste.')
    return n

def casa():
    n=Mapa('casa_abandonada_cojo','La casa abandonada del Oeste',18,16)
    n.sala('sala','Sala abandonada',2,2,8,12,'int_taulons_foscos')
    n.sala('dormitorio','Antiguo dormitorio',11,8,5,6,'int_taulons_foscos')
    n.sala('trasera','Cuarto de la entrada oculta',11,2,5,5,'int_lloses_fosques')
    n.camino([(3,12),(13,12),(13,4)],3,'int_taulons_foscos')
    n.muro(1,1,16,1);n.muro(1,2,1,13)
    n.muro(10,2,1,12,huecos={(10,5),(10,6),(10,11),(10,12)})
    n.start=(3,13);n.reservar((3,13),(14,3))
    n.portal('salida_casa_abandonada_cojo',2,13,'ciudad_oeste',LLEGADA_OESTE,'Salir al barrio del Oeste')
    n.portal('paso_casa_cojo_a_grieta',15,2,'grieta_cojo',(21,20),'Abrir la puerta trasera oculta')
    for oid,x,y in [('barriles_rotos',3,3),('cajas',5,3),('mesa',4,8),('silla',5,8),('cama_pobre_cap',14,9),('cama_pobre_peus',14,10),('baul',11,9),('rollo_cuerda',12,3)]:n.pon(oid,x,y)
    n.nota(5,11,'Casa abandonada nueva: el Cojo puede entrar desde la montana y salir al barrio del Oeste.')
    n.nota(13,5,'Paso secreto de la trasera: galerias de minas y la grieta de la ladera. Descubrimiento y fuga se escriben en la trama.')
    return n
