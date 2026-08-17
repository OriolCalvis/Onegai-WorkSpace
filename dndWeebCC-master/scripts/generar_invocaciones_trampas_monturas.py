#!/usr/bin/env python3
"""Completa la cola de producción: invocaciones (hasta 200), 50 trampas y 50 monturas.

  - Invocaciones (§6): 14 familias temáticas × 14 criaturas/tiers = 196 nuevas (+4 existentes
    = 200). CADA invocación lleva su hechizo real de invocación (invocar_<id>, tipo skill
    universal con tag [Invocacion], INT como requisito — el límite simultáneo lo gobierna INT).
  - Trampas (§14): 10 conceptos × 5 tiers, con detección/desarme (dos pruebas distintas),
    onFail peor que no intentarlo, condiciones por id real y 1 de cada 5 narrativa.
  - Monturas (§13): 10 especies × 5 tiers con mountedSkill propio y afinidad ♞; vida 8+4×tier;
    no ocupan mano; derribo con prueba de DES.
"""

from __future__ import annotations

import json
import re
import unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CARTAS = ROOT / "data/cartas"


def slugify(v: str) -> str:
    v = "".join(c for c in unicodedata.normalize("NFKD", v) if not unicodedata.combining(c)).lower()
    return re.sub(r"_+", "_", re.sub(r"[^a-z0-9]+", "_", v)).strip("_")


def escribir(path: Path, payload: dict) -> bool:
    if path.exists():
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    return True


# ------------------------------------------------------------------ INVOCACIONES
# 14 familias: (tema, tag, control, pasiva de familia, 14 criaturas (nombre, tier, ataque, efecto extra))
FAMILIAS = [
("Manada Espectral", "Naturaleza", "jugador", "Cazan en grupo: +1 al daño si otra invocación tuya está adyacente al objetivo.",
 [("Cachorro de Niebla", 1, "Mordisqueo", "el objetivo pierde 5 pies de movimiento"),
  ("Lobo de Ceniza", 1, "Dentellada", "Sangrado si falla Resistencia física"),
  ("Zorro de Dos Sombras", 1, "Mordisco Doble", "puede reposicionarse 5 pies tras atacar"),
  ("Perro de Turba", 2, "Presa Firme", "Agarrado si falla"),
  ("Loba Vigía", 2, "Tarascada", "revela a los Ocultos a 15 pies"),
  ("Jabalí de Brezo", 2, "Embestida", "Caído si falla Resistencia física"),
  ("Ciervo de Cornamenta Fría", 3, "Cornada", "empuja 10 pies"),
  ("Oso de Escarcha", 3, "Zarpazo Pesado", "reduce 1 la CA del objetivo 1 turno"),
  ("Alfa Fantasma", 3, "Dentellada de Mando", "tus otras bestias ganan +0,5 a su próximo ataque"),
  ("Lince de Auroras", 4, "Zarpa Boreal", "Cegado 1 turno si falla"),
  ("Bisonte Sepulcral", 4, "Carga Fúnebre", "atraviesa: daño a dos enemigos en línea"),
  ("Madre de la Manada", 4, "Mordisco Ancestral", "invoca un Cachorro de Niebla al abatir a un enemigo"),
  ("Behemot de Niebla", 5, "Aplastamiento", "Caído y Agarrado si falla"),
  ("El Primer Lobo", 5, "Dentellada Primigenia", "Asustado a los enemigos adyacentes si falla Defensa mental")]),
("Corte Menor de Espinas", "Naturaleza", "jugador", "Raíces solidarias: el terreno adyacente a ellas es difícil para tus enemigos.",
 [("Brote Mordedor", 1, "Mordisco Verde", "1 de veneno extra al herido"),
  ("Zarcillo Ladrón", 1, "Latigazo", "desarma objetos sueltos pequeños"),
  ("Flor Centinela", 1, "Escupitajo de Savia", "marca al objetivo (visible aunque se oculte)"),
  ("Espino Andante", 2, "Abrazo Espinado", "Agarrado y 1 de daño por turno"),
  ("Seta Tambor", 2, "Onda de Esporas", "Envenenado si falla"),
  ("Enredadera Portera", 2, "Cierre de Zarzas", "bloquea 5 pies de paso 1 ronda"),
  ("Sauce Doliente", 3, "Ramalazo", "alcance largo, empuja 5 pies"),
  ("Rosal de Guerra", 3, "Ráfaga de Pétalos", "área corta, 1 de daño y Sangrado si fallan"),
  ("Tocón Ancestral", 3, "Puñetazo de Raíz", "Caído si falla"),
  ("Dríade Prestada", 4, "Beso de Clorofila", "cura 2 a un aliado en vez de dañar"),
  ("Roble Caminante", 4, "Brazo de Tronco", "empuja 15 pies"),
  ("Corona de Ortigas", 4, "Aureola Urticante", "los enemigos adyacentes sufren 1 al inicio de su turno"),
  ("Señora del Claro", 5, "Juicio Verde", "Agarrado a hasta 2 enemigos si fallan"),
  ("El Bosque en Miniatura", 5, "Crecimiento Súbito", "convierte área mediana en espesura 2 rondas")]),
("Taller Animado", "Mecanico", "jugador", "Cuerda y engranaje: inmunes a Encantado, Asustado y Envenenado.",
 [("Escoba Diligente", 1, "Barrido", "Caído si falla (resbala)"),
  ("Tetera de Vapor", 1, "Chorro Caliente", "1 de fuego"),
  ("Soldadito de Cuerda", 1, "Bayoneta Mínima", "+1 si otro soldadito está adyacente"),
  ("Farol Rodante", 2, "Destello", "Cegado 1 turno si falla"),
  ("Yunque con Patas", 2, "Cabezazo de Hierro", "Guardia Rota si falla"),
  ("Caja de Música Marcial", 2, "Compás", "un aliado gana +0,5 a su próxima tirada"),
  ("Perchero Esgrimista", 3, "Cuatro Estocadas", "dos ataques leves"),
  ("Estufa Iracunda", 3, "Vaharada", "área corta: 1 de fuego"),
  ("Reloj de Torre Portátil", 3, "Campanada", "el objetivo pierde su reacción"),
  ("Telar Estratega", 4, "Hilo Tenso", "Agarrado si falla; tira 5 pies"),
  ("Imprenta Beligerante", 4, "Edición de Combate", "copia el último efecto menor aliado"),
  ("Fragua Andante", 4, "Rescoldo", "las armas aliadas adyacentes ganan +1 1 ronda"),
  ("Autómata Mayordomo", 5, "Servicio Completo", "cura 2 y limpia 1 condición a un aliado"),
  ("El Taller Entero", 5, "Cadena de Montaje", "dos ataques y una reparación por ronda")]),
("Coro de la Tomba", "Oscuro", "car_invocador", "Susurros de cripta: los enemigos adyacentes restan 0,5 a sus tiradas de Defensa mental.",
 [("Vela Doliente", 1, "Gota de Cera", "1 de fuego"),
  ("Susurro Encadenado", 1, "Palabra Fría", "−0,5 a la próxima tirada del objetivo"),
  ("Mano Amiga", 1, "Apretón", "Agarrado si falla (solo tobillos)"),
  ("Plañidera Menor", 2, "Lamento", "Asustado si falla"),
  ("Paje Traslúcido", 2, "Recado Urgente", "entrega/roba un objeto pequeño"),
  ("Monaguillo de Ceniza", 2, "Incensario", "área: −0,5 a ataques enemigos 1 ronda"),
  ("Dama de Compañía", 3, "Abanicazo Espectral", "empuja 5 pies e impone frío (1 daño)"),
  ("Guardia de Honor", 3, "Alabarda Fúnebre", "Guardia Rota si falla"),
  ("Chantre Hueco", 3, "Antífona", "un aliado recupera 1; un enemigo pierde 0,5 en su próxima tirada"),
  ("Notario de Difuntos", 4, "Fe de Vida", "el objetivo no puede recuperar vida 1 turno"),
  ("Confesor Errante", 4, "Absolución Inversa", "transfiere 1 condición de un aliado a un enemigo si falla Defensa mental"),
  ("Heraldo del Velatorio", 4, "Anuncio", "Paralizado 1 instante si falla (pierde acción menor)"),
  ("La Primera Plañidera", 5, "Llanto Total", "área: Asustado a quienes fallen"),
  ("El Cortejo Completo", 5, "Procesión", "atraviesa el campo: 2 de frío a los que toque")]),
("Enjambre y Plaga", "Veneno", "jugador", "Nube dispersa: mitad de daño de ataques a un solo objetivo.",
 [("Nube de Tábanos", 1, "Picaduras", "−0,5 al próximo ataque del objetivo"),
  ("Rata Mensajera", 1, "Mordisquito", "roba un consumible si falla DES"),
  ("Sapo Alquímico", 1, "Lengua Ácida", "1 de veneno"),
  ("Enjambre de Avispas de Tinta", 2, "Aguijonazo", "Envenenado si falla"),
  ("Colonia de Escarabajos Pala", 2, "Socavar", "el suelo bajo el objetivo es difícil 1 ronda"),
  ("Moscas de Fiebre", 2, "Zumbido", "desventaja en concentración 1 ronda"),
  ("Serpiente de Alambique", 3, "Colmillo Destilado", "Envenenado, y −1 CA si ya lo estaba"),
  ("Enjambre Cargado", 3, "Nube Crepitante", "1 de rayo a los adyacentes al objetivo"),
  ("Rata Madre", 3, "Llamada", "invoca una Rata Mensajera"),
  ("Langosta de Hierro", 4, "Siega", "área corta: 1 de daño"),
  ("Avispero Portátil", 4, "Descarga Total", "3 ataques de 1 repartidos"),
  ("Sanguijuela Real", 4, "Drenaje", "roba 1 de vida al objetivo para tu aliado más herido"),
  ("La Plaga Bailarina", 5, "Danza Macabra", "área: Envenenado a quienes fallen"),
  ("El Millón Diminuto", 5, "Marea Quitinosa", "cubre área mediana: enemigos dentro con desventaja")]),
("Elementales del Cielo Roto", "Magico", "jugador", "Astillas de cielo: sus ataques ignoran cobertura ligera.",
 [("Chispa Curiosa", 1, "Toque Estático", "1 de rayo"),
  ("Gota Perpetua", 1, "Salpicón", "apaga fuegos pequeños; 1 de daño"),
  ("Brasa Tímida", 1, "Chamusquina", "1 de fuego"),
  ("Racha Embotellada", 2, "Soplo", "empuja 10 pies si falla"),
  ("Escarcha Puntual", 2, "Beso Frío", "mitad de movimiento 1 turno"),
  ("Terrón Leal", 2, "Puñado de Grava", "Cegado 1 turno si falla"),
  ("Nubarrón de Bolsillo", 3, "Rayo Menor", "1 de rayo a dos enemigos adyacentes entre sí"),
  ("Charco Viajero", 3, "Tragar", "Agarrado si falla (hasta la rodilla)"),
  ("Fogata Insolente", 3, "Lengüetazo", "1 de fuego y prende lo inflamable"),
  ("Fragmento de Aurora", 4, "Haz Prismático", "Cegado si falla; 2 de daño"),
  ("Peña Rodante", 4, "Aplastar", "Caído si falla"),
  ("Géiser Domesticado", 4, "Erupción", "lanza al objetivo 10 pies hacia arriba (cae: 1 de daño)"),
  ("Tormenta Cachorra", 5, "Primer Trueno", "área: 1 de rayo y sin reacciones si fallan"),
  ("Esquirla del Primer Cielo", 5, "Recuerdo de Altura", "2 de daño; el objetivo 'cae hacia arriba' y queda expuesto")]),
("Guardianes de Tinta", "Magico", "jugador", "Papel y voluntad: pueden plegarse (entrar/salir por rendijas) una vez por ronda.",
 [("Grulla de Papel", 1, "Picotazo de Pliegue", "1 de daño"),
  ("Perro de Tinta", 1, "Mordisco Borroso", "mancha: −0,5 a su próxima tirada"),
  ("Marcapáginas Fiel", 1, "Corte de Canto", "1 de daño; guarda la 'página' (posición) del objetivo"),
  ("Soldado de Origami", 2, "Lanza Doblada", "+1 si el objetivo fue 'marcado' por el Marcapáginas"),
  ("Mariposa de Códice", 2, "Polvo de Glosa", "revela ilusiones a 15 pies"),
  ("Dragón Plegado", 2, "Aliento de Confeti", "área mínima: 1 de daño"),
  ("Bibliotecaria de Bolsillo", 3, "Silencio", "el objetivo no puede canalizar 1 turno si falla"),
  ("Tomo Guardián", 3, "Portazo de Tapas", "Agarrado (entre páginas) si falla"),
  ("Cita Textual", 3, "Réplica Exacta", "copia el último ataque menor enemigo"),
  ("Atlas Doblado", 4, "Doblar el Mapa", "intercambia a dos criaturas dispuestas o que fallen"),
  ("Erratas Vengativa", 4, "Corrección", "anula el próximo bono del objetivo"),
  ("Pliego Imperial", 4, "Edicto", "un enemigo pierde su acción de movimiento si falla"),
  ("El Manuscrito Original", 5, "Reescritura", "repite una tirada aliada 1/combate"),
  ("La Biblioteca Andante", 5, "Peso del Saber", "área: mitad de movimiento a enemigos")]),
("Sombra Doméstica", "Oscuro", "car_invocador", "Contraluz: en penumbra ganan +1 a todas sus tiradas.",
 [("Sombra de Gato", 1, "Zarpazo Frío", "1 de daño"),
  ("Silueta Prestada", 1, "Imitación", "distrae: −0,5 al próximo ataque contra tu aliado"),
  ("Farol Apagado", 1, "Trago de Luz", "apaga una luz pequeña"),
  ("Perro de Medianoche", 2, "Mordisco Mudo", "sin ruido: no rompe sigilo aliado"),
  ("Doble de Perfil", 2, "Amago", "el próximo ataque contra él falla (se desvanece 1 vez)"),
  ("Sombra de Escalera", 2, "Zancadilla Larga", "Caído si falla"),
  ("Manto Errante", 3, "Envolver", "Cegado 1 turno si falla"),
  ("Eco de Pasos", 3, "Confusión", "el objetivo se mueve 5 pies en dirección errónea si falla"),
  ("Sombra del Verdugo", 3, "Tajo sin Dueño", "+1 contra objetivos bajo media vida"),
  ("Noche de Bolsillo", 4, "Despliegue", "área corta en penumbra 2 rondas"),
  ("Reflejo Independizado", 4, "Golpe Gemelo", "repite el último ataque de tu personaje (mitad de daño)"),
  ("Susurro con Garras", 4, "Miedo Concreto", "Asustado si falla"),
  ("La Sombra del Rey", 5, "Corona Invertida", "los enemigos adyacentes pierden su reacción"),
  ("El Contraluz Total", 5, "Eclipse Portátil", "área mediana ciega a los enemigos que fallen")]),
("Servidores del Pacto", "Oscuro", "car_invocador", "Cláusula de servicio: si su invocador cae Moribundo, atacan con ventaja hasta que se levante.",
 [("Diablillo Contable", 1, "Pluma Afilada", "apunta la deuda: +0,5 al próximo cobro"),
  ("Testigo Mudo", 1, "Mirada Fija", "el objetivo no puede Ocultarse 1 ronda"),
  ("Mensajero de Ceniza", 1, "Recado Ardiente", "1 de fuego"),
  ("Cobrador Menor", 2, "Embargo", "roba un objeto suelto pequeño si falla DES"),
  ("Escriba de Humo", 2, "Contrato Volátil", "Endeudado si falla"),
  ("Portasellos", 2, "Sello Candente", "marca: tus cartas oscuras +0,5 contra él"),
  ("Abogado del Foso", 3, "Objeción", "anula una reacción enemiga"),
  ("Fiador de Hueso", 3, "Aval Roto", "2 de daño a un Endeudado"),
  ("Notaria Bicéfala", 3, "Doble Firma", "dos ataques de 1"),
  ("Ejecutor de Cláusulas", 4, "Cobro Forzoso", "2 de daño; +1 por Interés acumulado del invocador"),
  ("Auditor Nocturno", 4, "Revisión", "el objetivo pierde un bono activo si falla"),
  ("Padrino de Empeños", 4, "Préstamo Sucio", "un aliado gana +1 a su próxima tirada; tú pagas 1 de vida"),
  ("El Socio Silencioso", 5, "Fusión Hostil", "Agarrado y Endeudado si falla"),
  ("La Letra Pequeña", 5, "Ejecución de Aval", "daño igual al Interés acumulado ×2 (consume todo)")]),
("Reliquias Animadas", "Sagrado", "jugador", "Fe sólida: los aliados adyacentes ganan +0,5 a Defensa mental.",
 [("Cirio Valiente", 1, "Gota Bendita", "1 de fuego sagrado"),
  ("Rosario Rodante", 1, "Cuenta Certera", "1 de daño; no falla contra [Oscuro]"),
  ("Estandarte Huérfano", 1, "Ondeo", "un aliado gana +0,5 a su próxima tirada"),
  ("Campanilla de Altar", 2, "Tañido Limpio", "los no-muertos adyacentes pierden 0,5 en su próxima tirada"),
  ("Icono Portátil", 2, "Mirada Piadosa", "cura 1 a un aliado"),
  ("Misal de Combate", 2, "Cita Contundente", "1 de daño +1 contra [Oscuro]"),
  ("Cáliz Andante", 3, "Rebosar", "cura 2 a un aliado adyacente"),
  ("Incensario Volador", 3, "Órbita de Humo", "área mínima: enemigos con −0,5 al atacar"),
  ("Custodia Menor", 3, "Destello Sagrado", "Cegado a un no-muerto si falla"),
  ("Retablo Guardián", 4, "Escudo de Historias", "un aliado reduce 2 el próximo daño"),
  ("Campana de Consagración", 4, "Volteo Completo", "área: no-muertos con desventaja 1 ronda"),
  ("Sepulcro Portátil", 4, "Cerrar la Losa", "un no-muerto criatura que falle queda Agarrado dentro 1 ronda"),
  ("El Relicario Mayor", 5, "Exposición", "área: 2 de daño sagrado a [Oscuro] y no-muertos"),
  ("La Fe Hecha Mueble", 5, "Bendición Estructural", "los aliados en área ganan +1 CA 1 ronda")]),
("Bestias de Silla", "Fisico", "jugador", "Adiestradas: pueden llevar a un aliado (cuentan como montura improvisada, sin mountedSkill).",
 [("Poni Valiente", 1, "Coz", "1 de daño"),
  ("Mula de Hierro", 1, "Testarazo", "empuja 5 pies"),
  ("Cabra Tozuda", 1, "Embestida Vertical", "Caído si falla (en cuestas, siempre)"),
  ("Perro de Tiro", 2, "Tirón de Arnés", "arrastra a un aliado Caído 10 pies (rescate)"),
  ("Corcel de Posta", 2, "Galope Corto", "reposiciona a su jinete 15 pies"),
  ("Buey de Vanguardia", 2, "Empuje Lento", "empuja 10 pies, imparable"),
  ("Camello Malaleche", 3, "Escupitajo", "Cegado 1 turno si falla"),
  ("Alce de Ribera", 3, "Cornamenta Ancha", "dos enemigos adyacentes: 1 de daño"),
  ("Yegua Nocturna", 3, "Casco Silencioso", "no rompe el sigilo del grupo"),
  ("Rinoceronte de Feria", 4, "Carga de Cartel", "2 de daño y Caído si falla"),
  ("Águila de Silla", 4, "Picado", "lleva al jinete 20 pies en vuelo corto"),
  ("Oso Ensillado", 4, "Abrazo de Viaje", "Agarrado si falla"),
  ("El Semental de los Doce Vientos", 5, "Coz Huracanada", "empuja 15 pies a dos enemigos"),
  ("La Montura del Rey Olvidado", 5, "Paso Real", "su jinete gana +1 a CA y CAR mientras monte")]),
("Ecos Arcanos", "Magico", "automatica", "Programados: actúan solos según su regla, no gastan tu control.",
 [("Glifo Zumbón", 1, "Descarga", "1 de daño al primer enemigo que se acerque"),
  ("Runa Paciente", 1, "Brillo", "avisa cuando algo invisible pasa a 10 pies"),
  ("Orbe Cascarrabias", 1, "Chispazo", "1 de rayo al que lo toque"),
  ("Torreta de Cuarzo", 2, "Rayo Fino", "1 de daño al enemigo más cercano cada ronda"),
  ("Espejo de Vigilancia", 2, "Reflejar", "devuelve el próximo proyectil menor"),
  ("Sello Cantor", 2, "Nota Sostenida", "los aliados a 10 pies ganan +0,5 a concentración"),
  ("Columna de Luz Fría", 3, "Foco", "un enemigo iluminado no puede Ocultarse"),
  ("Prisma Errante", 3, "Refracción", "divide un rayo aliado en dos objetivos (mitad)"),
  ("Ancla Rúnica", 3, "Gravedad Local", "los enemigos a 10 pies mueven 5 pies menos"),
  ("Obelisco Portátil", 4, "Pulso", "área: 1 de daño a enemigos cada 2 rondas"),
  ("Constelación Doméstica", 4, "Mapa Vivo", "el grupo ve posiciones enemigas aunque no las vea"),
  ("Motor de Mareas", 4, "Vaivén", "empuja o atrae 5 pies a todos en área corta"),
  ("El Teorema Guardián", 5, "Demostración", "anula la primera carta enemiga de área por combate"),
  ("La Ecuación Amiga", 5, "Despeje Favorable", "1/combate convierte una pifia aliada en tirada normal")]),
("Parientes del Umbral", "Oscuro", "car_invocador", "De la familia de los yokai: cuentan como en ambos lados de una puerta a la vez.",
 [("Duende de Bisagra", 1, "Pellizco", "1 de daño; chirría (rompe sigilo enemigo)"),
  ("Llave Vieja", 1, "Giro", "abre/cierra una cerradura mundana a 10 pies"),
  ("Felpudo Rencoroso", 1, "Tropiezo", "Caído si falla al cruzar"),
  ("Aldaba Parlante", 2, "Golpeteo", "Asustado 1 turno si falla (los golpes vienen de dentro)"),
  ("Mirilla Andante", 2, "Ojo Único", "ve a través de una puerta/pared fina"),
  ("Cerrojo Leal", 2, "Cierre", "una puerta queda sellada 1 ronda"),
  ("Umbral Plegable", 3, "Puerta Portátil", "dos aliados intercambian posición"),
  ("Peldaño Fantasma", 3, "Escalón Traidor", "Caído y 1 de daño si falla"),
  ("Cortina de Cuentas", 3, "Repiqueteo", "los proyectiles que la cruzan fallan con 0,5 menos"),
  ("Portal Doméstico", 4, "Mudanza", "lleva a un aliado a 20 pies (cruza paredes finas)"),
  ("El Pasillo Encogido", 4, "Acortar", "un enemigo que corra llega 10 pies antes de lo que cree (a tu melé)"),
  ("Guardián de la Casa", 4, "Desahucio", "empuja 15 pies hacia la salida más cercana"),
  ("La Puerta Principal", 5, "Invitación Retirada", "un enemigo que falle es expulsado del área 20 pies"),
  ("El Umbral Definitivo", 5, "Cruce Forzoso", "intercambia a dos enemigos que fallen Defensa mental")]),
("Vástagos de la Forja", "Fuego", "jugador", "Calor residual: quien los destruye en melé sufre 1 de fuego.",
 [("Ascua Fiel", 1, "Salto de Chispa", "1 de fuego"),
  ("Clavo Ardiendo", 1, "Puntada", "1 de daño; se aferra (el objetivo lo nota siempre)"),
  ("Lingote Rodante", 1, "Atropello Mínimo", "1 de daño"),
  ("Tenazas Vivas", 2, "Pellizco de Herrero", "Agarrado si falla"),
  ("Fuelle Incansable", 2, "Racha Caliente", "aviva: +1 al próximo efecto de fuego aliado"),
  ("Molde Errante", 2, "Vaciado", "copia la forma: +1 CA a un aliado 1 ronda"),
  ("Martillo sin Mango", 3, "Golpe Franco", "Guardia Rota si falla"),
  ("Crisol Andante", 3, "Salpicadura", "área mínima: 1 de fuego"),
  ("Cadena Templada", 3, "Enroscarse", "Agarrado; el agarre no se rompe con empujes"),
  ("Bigornia de Guerra", 4, "Peso Muerto", "2 de daño y Caído si falla"),
  ("Horno Peregrino", 4, "Puerta Abierta", "cono corto: 2 de fuego"),
  ("Escoria Noble", 4, "Recubrir", "un aliado gana resistencia al fuego 1 escena"),
  ("El Primer Molde", 5, "Reforjar", "repara/re-templa: un aliado recupera 3 de vida (constructos: 5)"),
  ("La Llama de Taller", 5, "Encargo Urgente", "dos ataques de 2 de fuego")]),
]

TRAMPAS = [
("Foso de Estacas Corteses", "mecánica", "pisar la losa floja", "INT", "DES",
 "Caída con estacas: 2 de daño y Caído.", "caido", "La losa cede del todo: 3 de daño y Agarrado entre estacas.", "única"),
("Aguja del Cerrojo Mentiroso", "mecánica", "forzar la cerradura equivocada", "INT", "DES",
 "Aguja untada: 1 de daño y Envenenado si falla Resistencia física.", "envenenado", "El veneno entra en vena: Envenenado sin tirada y −1 a DES hasta descansar.", "recargable"),
("Red de Contrapesos", "mecánica", "cruzar el pasillo a media altura", "DES", "DES",
 "Red lastrada: Agarrado hasta liberarse.", "agarrado", "El contrapeso sube al atrapado al techo: Agarrado y colgado (los ataques contra él tienen ventaja).", "recargable"),
("Alarma de Campanillas Muertas", "narrativa", "mover el hilo a la altura del tobillo", "INT", "DES",
 "No daña: toda la estancia sabe que estáis aquí (refuerzos en 2 rondas).", None, "La campanilla no calla: desventaja en sigilo el resto de la escena.", "única"),
("Losa de Bendición Invertida", "mágica", "pisar el sello borrado", "INT", "INT",
 "Marca profana: −1 a Defensa mental hasta descansar.", None, "El sello se completa: Asustado y la marca no se va hasta un rito.", "única"),
("Boca de Fuego de Mantenimiento", "mecánica", "abrir la puerta sin purgar la presión", "INT", "INT",
 "Vaharada: 2 de fuego en cono corto.", None, "La caldera revienta: 3 de fuego y la sala se incendia (1 por ronda dentro).", "única"),
("Espejo que Cobra Peaje", "mágica", "mirarse más de un instante", "INT", "CAR",
 "El reflejo roba 0,5 éxitos de tu próxima tirada.", None, "El reflejo sale: un Doble de Perfil (invocación t2) hostil aparece.", "persistente"),
("Suelo de Escarcha Cantora", "mágica", "correr sobre el hielo grabado", "INT", "DES",
 "Resbalón musical: Caído y la melodía delata la posición.", "caido", "El hielo se abre: Agarrado en agua helada, 1 de frío por ronda.", "persistente"),
("Cepo del Recaudador", "mecánica", "recoger la bolsa de monedas cebo", "INT", "DES",
 "Cepo al brazo: 1 de daño y Agarrado.", "agarrado", "El cepo lleva cadena y la cadena campana: Agarrado + refuerzos avisados.", "recargable"),
("Círculo de Hongos Pacientes", "narrativa", "dormir dentro del corro", "INT", "INT",
 "No daña: los durmientes sueñan la misma deuda (gancho con las Sagas).", None, "El corro se cierra: 1 nivel de Fatiga y una deuda onírica real (el DJ la apunta).", "persistente"),
]

MONTURAS = [
("Poni de las Marcas", "llano", 8, "Trote Incansable", "1/día: dobla la velocidad de viaje del jinete durante una hora."),
("Caballo de Posta", "llano", 10, "Sprint de Entrega", "1/combate: 20 pies extra de movimiento en línea recta."),
("Mula de Montaña", "montana", 7, "Paso Seguro", "Ignora terreno difícil de pendiente; el jinete no puede ser derribado en cuestas."),
("Carnero de Udrax", "montana", 8, "Embestida de Cumbre", "1/combate: carga que empuja 10 pies y Caído si falla Resistencia física."),
("Lobo de Silla del Norte", "nieve", 9, "Caza Compartida", "El jinete gana +1 al primer ataque contra objetivos que sangran."),
("Felino de las Dunas", "desierto", 10, "Zancada Silenciosa", "El sigilo del jinete no se rompe al moverse montado."),
("Lagarto de Pantano", "agua", 7, "Nado de Emboscada", "Nada sin penalización; el jinete gana ventaja en su primer ataque desde el agua."),
("Alce de Ribera", "bosque", 8, "Cornamenta Escolta", "Los ataques de oportunidad contra el jinete sufren desventaja."),
("Ave de Silla Aarakocra", "aire", 9, "Vuelo Corto", "1/combate: planea 30 pies ignorando el suelo y sus peligros."),
("Jabalí de Guerra de Gongorguma", "llano", 8, "Arremetida Tozuda", "1/combate: atraviesa una línea enemiga; 1 de daño a cada uno.")]

EPITETOS_MONTURA = ["", "Curtido", "de Guerra", "Veterano", "Legendario"]


def main() -> None:
    inv_dir = CARTAS / "invocaciones"
    hab_dir = CARTAS / "habilidades"
    trap_dir = CARTAS / "trampas"
    mount_dir = CARTAS / "monturas"

    n_inv = n_spell = n_trap = n_mount = 0

    # ---- INVOCACIONES + sus hechizos de invocación ----
    for tema, tag, control, pasiva_fam, criaturas in FAMILIAS:
        for nombre, tier, atk, extra in criaturas:
            sid = slugify(nombre)
            invocar_id = f"invocar_{sid}"
            summon = {
                "id": sid, "name": nombre, "type": "summon",
                "summonedBy": invocar_id, "tier": tier, "health": 6 + 3 * tier,
                "attacks": [{"name": atk, "effect": f"Daño leve ({tag.lower()}); {extra}."}],
                "movement": 6, "passive": {"name": f"Familia: {tema}", "description": pasiva_fam},
                "duration": "concentration" if tier >= 3 else "1_min",
                "control": control,
                "flavorText": f"De la familia de {tema}.",
            }
            carta_invocar = {
                "id": invocar_id, "name": f"Invocar {nombre}", "type": "skill",
                "tier": tier, "rarity": ["common", "common", "uncommon", "uncommon", "rare", "rare"][tier],
                "classTags": [], "roleTags": ["caster"], "mechanicTags": ["Invocacion", tag],
                "requiredStats": {"INT": max(2, tier)},
                "requiredTags": [], "incompatibleTags": [],
                "cost": {"resource": "none", "amount": 0},
                "recovery": "descanso_largo" if tier >= 4 else "descanso_corto",
                "actionType": "accion", "range": "short",
                "duration": summon["duration"], "defenseStat": None,
                "effect": {"description": f"Invocas {nombre} (carta de invocación propia) en un punto a alcance corto. El número de invocaciones simultáneas lo gobierna tu INT (GDD 4).", "scaling": "INT"},
                "limitations": [], "upgradePath": [], "evolvesInto": None,
                "flavorText": f"De la familia de {tema}.",
            }
            if escribir(inv_dir / f"{sid}.json", summon):
                n_inv += 1
            if escribir(hab_dir / f"{invocar_id}.json", carta_invocar):
                n_spell += 1

    # ---- TRAMPAS: 10 conceptos × 5 tiers ----
    for ti, (nombre, tipo, trigger, det_stat, dis_stat, efecto, cond, onfail, reset) in enumerate(TRAMPAS):
        for tier in range(1, 6):
            cd = [0.5, 1, 1.5, 2, 2.5][tier - 1]
            sufijo = ["", " Mejorada", " Maestra", " Antigua", " Legendaria"][tier - 1]
            trampa = {
                "id": slugify(nombre + sufijo), "name": nombre + sufijo, "type": "trap",
                "tier": tier, "rarity": ["common", "uncommon", "rare", "epic", "mythic"][tier - 1],
                "category": tipo,
                "trigger": trigger,
                "detection": {"stat": det_stat, "cd": cd},
                "disarm": {"stat": dis_stat, "cd": cd, "herramienta": "herramientas de ladrón" if tipo == "mecánica" else None},
                "effect": {"description": efecto + (f" (escala: +{tier - 1} de daño en este tier)" if tier > 1 else ""),
                           "defensa": "resistencia_fisica" if tipo != "mágica" else "defensa_mental",
                           "condicionInfligida": cond},
                "onFail": onfail,
                "reset": reset,
                "flavorText": "Quien la montó pensaba volver. No volvió.",
            }
            if escribir(trap_dir / f"{trampa['id']}.json", trampa):
                n_trap += 1

    # ---- MONTURAS: 10 especies × 5 tiers ----
    for mi, (nombre, terreno, mov, skill_nombre, skill_desc) in enumerate(MONTURAS):
        for tier in range(1, 6):
            epiteto = EPITETOS_MONTURA[tier - 1]
            nombre_full = f"{nombre} {epiteto}".strip()
            montura = {
                "id": slugify(nombre_full), "name": nombre_full, "type": "mount",
                "tier": tier, "rarity": ["common", "uncommon", "rare", "epic", "mythic"][tier - 1],
                "health": 8 + 4 * tier, "armor": 10 + tier,
                "movement": mov + (tier - 1),
                "stats": {"CON": 2 + tier, "DES": 2 + tier},
                "capacity": 1 if mov >= 9 else 2,
                "mountedTags": ["Montado"],
                "ridingRequirements": {"stat": "DES", "minimo": max(1, tier - 1)},
                "mountedSkill": {"name": skill_nombre,
                                 "description": skill_desc + (f" A partir de tier 3, su efecto numérico sube en +{tier - 2}." if tier >= 3 else ""),
                                 "recovery": "descanso_corto"},
                "dismountTrigger": "Si la montura recibe un golpe de 3+ de daño, el jinete prueba DES o queda Caído",
                "terrain": [terreno, "llano"] if terreno != "llano" else ["llano"],
                "upkeep": "Requiere forraje/caza diaria y descanso: una jornada sin cuidados le impone 1 nivel de Fatiga",
                "flavorText": f"Afinidad ♞: activa el equipo y las cartas de jinete (GDD sección 8).",
            }
            if escribir(mount_dir / f"{montura['id']}.json", montura):
                n_mount += 1

    print(f"Invocaciones: {n_inv} · hechizos de invocación: {n_spell} · trampas: {n_trap} · monturas: {n_mount}")


if __name__ == "__main__":
    main()
