#!/usr/bin/env python3
"""Genera 300 historias (data/historias) y 100 NPCs (data/npcs).

Historias (Plantilla §10): cada gancho nombra a ALGUIEN (antagonista con id real del
bestiario), en un SITIO, queriendo ALGO, con recompensa real (tabla de loot) y afinidad
con meses. 20 por facción: 4 tramas madre × 5 escalones de tier que forman una cadena.

NPCs (§12): agenda + palanca + secreto conectado a una historia real + mes de nacimiento
real (su virtud/defecto sale gratis del trasfondo). Los 12 PNJ de las misiones del
usuario se respetan como canon y se completan hasta 100.
"""

from __future__ import annotations

import json
import re
import unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HIST = ROOT / "data/historias"
NPCS = ROOT / "data/npcs"
CARTAS = ROOT / "data/cartas"
LOOT = ROOT / "data/loot"

MESES = ["mes_de_lescarcha", "mes_de_la_siembra", "mes_de_laigua", "mes_de_loracle",
         "mes_del_sol", "mes_de_la_cosecha", "mes_del_ferro", "mes_de_lagonia",
         "mes_de_la_terra", "mes_dels_aullits", "mes_de_les_ombres", "mes_de_les_estrelles"]


def slugify(v: str) -> str:
    v = "".join(c for c in unicodedata.normalize("NFKD", v) if not unicodedata.combining(c)).lower()
    return re.sub(r"_+", "_", re.sub(r"[^a-z0-9]+", "_", v)).strip("_")


def escribir(path: Path, payload: dict) -> bool:
    if path.exists():
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    return True


# Por facción: (lugar emblema, 4 tramas madre [título base, gancho con {ant} antagonista, decisión],
#               complicaciones [2], meses afines [2])
TRAMAS = {
"manada_del_rei_llop": ("los bosques del norte de Llerba",
 [("El Rastro que Vuelve", "Los ganaderos juran que {ant} arrastra las reses hacia la Tomba, no lejos de ella. Nadie sigue el rastro dos noches seguidas.", "Seguir el rastro de noche o esperar refuerzos y perderlo."),
  ("Tres Noches de Aullido", "{ant} lleva tres noches aullando en círculo alrededor de la aldea. Los viejos dicen que la cuarta noche entra.", "Salir a romper el círculo o fortificar la aldea y aguantar la cuarta noche."),
  ("La Loba Herida", "Una loba enorme sangra junto al molino; {ant} la busca para rematarla. La loba lleva un collar con el sello real.", "Salvar a la loba (y heredar su perseguidor) o entregarla."),
  ("Nieve en Julio", "Nieva sobre un solo valle, en pleno verano. En el centro de la nevada, {ant} monta guardia ante algo enterrado.", "Excavar lo que guarda o sellar el valle antes de que despierte.")],
 ["El frío llega antes que la manada: cada noche al raso cuesta abrigo o Fatiga.",
  "Quien mate a un lobo hereda su aullido: la manada lo reconocerá para siempre."],
 ["mes_dels_aullits", "mes_de_lescarcha"]),
"cofradia_de_mascaras_de_plata": ("los barrios de plata de Bastrea",
 [("La Máscara del Alcalde", "El alcalde lleva una semana sin quitarse una máscara 'ceremonial'. {ant} cobra sus deudas mientras tanto.", "Desenmascararlo en público o negociar con quien lleve la máscara."),
  ("Contrato en Blanco", "Un contrato firmado y en blanco circula por la ciudad; {ant} paga fortunas por recuperarlo. Nadie sabe qué compromete.", "Venderlo, destruirlo o rellenarlo antes que nadie."),
  ("El Taller de Rostros", "Desaparecen retratistas. {ant} encarga cuadros de gente que aún no ha muerto.", "Infiltrarse como modelo o seguir al mensajero de los encargos."),
  ("Plata a la Deriva", "Un cargamento de máscaras vírgenes llega sin remitente. {ant} lo espera en el muelle... igual que la guardia.", "Quedarse el cargamento, entregarlo o hundirlo con el barco.")],
 ["Toda máscara puesta más de un día deja marca: la Cofradía sabrá quién la llevó.",
  "Los testigos cambian su versión al ver plata: ningún interrogatorio dura dos días."],
 ["mes_de_les_ombres", "mes_de_loracle"]),
"corte_de_espinas": ("la linde del Bosque que Reclama",
 [("La Valla que Amaneció Dentro", "La valla nueva del terrateniente amaneció veinte pasos bosque adentro, con {ant} esperando en la puerta.", "Defender la valla, moverla de verdad o juzgar al terrateniente."),
  ("Flores en el Trigal", "Flores espinadas brotan en mitad del trigal maduro; {ant} las riega con algo que no es agua.", "Quemar la parcela (y ofender a la Corte) o negociar el diezmo verde."),
  ("El Novio del Espino", "Un joven volvió del bosque comprometido con 'una dama de espinas'. {ant} viene a buscar al novio para la boda.", "Impedir la boda, negociar la dote o asistir como testigos."),
  ("Raíces en el Pozo", "El pozo del pueblo da savia en vez de agua. Abajo, {ant} teje algo con las raíces.", "Bajar a cortar o averiguar qué sed tiene el bosque.")],
 ["Cortar madera viva durante la historia enfada a la Corte: +1 a todos sus miembros.",
  "El bosque cambia de sitio los caminos: cada viaje repite prueba de orientación."],
 ["mes_de_la_siembra", "mes_del_sol"]),
"renacidos_de_la_fosa": ("los campos de la Última Carga",
 [("El Censo de los Caídos", "Los muertos de la guerra vieja marchan de noche hacia el pueblo. {ant} los pasa lista, y faltan tres nombres.", "Encontrar a los tres desertores vivos o falsificar el censo."),
  ("La Paga Pendiente", "{ant} asalta el convoy de pagas... para cobrar sueldos de hace cuarenta años. Con intereses.", "Pagar la deuda del reino, negociar o dispersar a los acreedores."),
  ("El Tambor Enterrado", "Un tambor suena bajo tierra cada amanecer. {ant} desentierra soldados al ritmo.", "Silenciar el tambor o averiguar qué orden espera."),
  ("Armisticio", "{ant} envía un emisario: los Renacidos ofrecen la paz... si alguien firma la rendición de un reino que ya no existe.", "Falsificar la firma, buscar al heredero legítimo o rechazar y prepararse.")],
 ["Cada caído sin enterrar en la zona se une a ellos al anochecer.",
  "Los Renacidos no persiguen: esperan en el camino de vuelta."],
 ["mes_dels_aullits", "mes_de_lagonia"]),
"plaga_de_san_lazaro": ("el lazareto del camino real",
 [("La Cuarentena Alegre", "Nadie quiere salir del lazareto: dentro se está 'mejor que nunca'. {ant} oficia las admisiones.", "Entrar a sacar a los tuyos o cortar el suministro de nuevos fieles."),
  ("El Pan Que Tose", "La harina del molino viene con esporas. {ant} bendice los sacos personalmente.", "Quemar la cosecha, purificarla o rastrear al proveedor nocturno."),
  ("Peregrinación Febril", "Cien enfermos marchan a 'sanar' al santuario. {ant} los guía... lejos de toda medicina.", "Desviar la columna, curar en marcha o desenmascarar al guía."),
  ("El Milagro Documentado", "Un escriba tiene pruebas de que los milagros del santo funcionan. {ant} quiere el documento; media iglesia también.", "Publicarlo, destruirlo o subastarlo.")],
 ["Toda cura en la zona atrae atención: los fieles ven blasfemia en la medicina.",
  "El grupo debe vigilar su comida: prueba diaria o alguien amanece Envenenado."],
 ["mes_de_lagonia", "mes_de_la_cosecha"]),
"forjados_sin_amo": ("las ruinas del Gran Taller",
 [("La Orden Perdida", "Un Forjado llama a las puertas pidiendo 'instrucciones nuevas'. {ant} viene a buscarlo antes de que aprenda a elegir.", "Darle órdenes, liberarlo o devolverlo al Taller."),
  ("Herramientas que Vuelven", "Las herramientas robadas del Taller regresan solas, arrastrando a sus ladrones. {ant} las recibe en la puerta.", "Devolver lo robado, romper el vínculo o negociar con el capataz."),
  ("El Corazón de Cuerda", "En el mercado venden un 'corazón mecánico que concede deseos'. {ant} ofrece el triple por recuperarlo.", "Comprarlo, robarlo o averiguar de qué pecho salió."),
  ("Turno de Noche", "El Taller ruge de noche: {ant} fabrica algo grande. Los pedidos, dicen, los firma un muerto.", "Sabotear la cadena, ver qué se fabrica o encontrar al firmante.")],
 ["Los Forjados no duermen: toda infiltración es a taller funcionando.",
  "El hierro de la zona recuerda su forma: las armas rotas aquí se reparan solas... con opiniones."],
 ["mes_del_ferro", "mes_de_les_estrelles"]),
"nagas_del_pozo_azul": ("los canales del Pozo Azul",
 [("Intereses de Fondo", "Media aldea firmó 'préstamos de lluvia'. Vence el plazo y {ant} sube a cobrar casa por casa.", "Pagar el rescate común, renegociar o secar el pozo."),
  ("La Novia del Canal", "Cada década, el pueblo 'casa' a alguien con el pozo. Este año la elegida ha huido y {ant} exige a la sustituta.", "Esconder a la novia, romper el pacto o descubrir qué pasó con las anteriores."),
  ("Moneda Marcada", "Toda moneda que toca el agua vuelve marcada con escamas. {ant} rastrea las marcas hasta las bolsas.", "Devolver el dinero al pozo, lavarlo con rito o gastar más rápido de lo que rastrea."),
  ("El Fondo del Fondo", "Un buzo volvió del pozo con una llave de oro y sin recuerdos. {ant} ofrece devolvérselos... por la llave.", "Cambiar la llave, copiarla o bajar a abrir lo que abra.")],
 ["Todo trato verbal junto al agua cuenta como firmado.",
  "Llueve cuando las nagas cobran: la lluvia es su notario."],
 ["mes_de_laigua", "mes_de_la_terra"]),
"yokai_del_umbral": ("las puertas viejas del barrio antiguo",
 [("La Casa que Sobra", "En la calle hay una casa más que ayer. Sus vecinos juran que siempre estuvo. {ant} cobra el alquiler.", "Entrar, censarla o tapiarla con sus inquilinos dentro."),
  ("El Peaje de los Espejos", "Los espejos del pueblo muestran las habitaciones con un segundo de retraso. {ant} vive en ese segundo.", "Romper todos los espejos o negociar el peaje del reflejo."),
  ("Llaves Prestadas", "Un manojo de llaves abre cualquier puerta... y cada uso deja entrar algo. {ant} quiere sus llaves de vuelta.", "Devolverlas, pagarlas o cerrar lo que entró."),
  ("La Puerta del Mediodía", "A mediodía exacto, la puerta de la iglesia da a otro sitio. {ant} hace de portero y cobra en recuerdos.", "Pagar el peaje, engañar al portero o clausurar el mediodía.")],
 ["Toda puerta cerrada con llave en la zona puede abrirse a otro sitio: entrar exige valor o prueba.",
  "Los nombres dichos en voz alta cerca de un umbral se los queda el umbral."],
 ["mes_de_les_ombres", "mes_de_loracle"]),
"clan_de_gongorguma_renegado": ("las colinas rotas del este",
 [("El Tributo de Hierro", "El clan exige el hierro del pueblo: cada reja, cada azada. {ant} pasa a recogerlo al alba.", "Pagar, esconder el metal o retar a duelo al recaudador."),
  ("La Prueba del Forastero", "El clan ofrece paso seguro a quien supere 'la prueba'. Nadie ha vuelto a contarla. {ant} es el examinador.", "Presentarse a la prueba, espiarla o comprar las respuestas."),
  ("El Hijo Blando", "El hijo del jefe quiere ser escriba, no guerrero. {ant} tiene orden de traerlo 'curado'.", "Esconder al chico, negociar con el padre o falsificar una hazaña."),
  ("Tambores de Boda", "El clan celebra una boda-alianza con mercenarios del sur. {ant} organiza la seguridad. Media región quiere reventarla.", "Impedir la alianza, protegerla o infiltrar la fiesta.")],
 ["Rechazar un duelo formal aquí es declararse presa.",
  "Todo lo dicho ante testigos del clan se considera juramento."],
 ["mes_del_ferro", "mes_de_la_terra"]),
"hechiceros_del_vacio": ("el cráter del Colegio Hundido",
 [("La Beca del Hueco", "El Colegio 'beca' a niños con talento: desaparecen un año y vuelven... espaciados. {ant} recluta esta semana.", "Sabotear la selección, seguir a un becado o entrar como aspirantes."),
  ("El Pasillo Robado", "Al ayuntamiento le falta un pasillo entero: puertas que ahora dan a pared. {ant} lo está usando de despensa.", "Recuperar el pasillo o negociar el alquiler retroactivo."),
  ("Cinco Pasos de Menos", "La plaza mide cinco pasos menos cada semana. {ant} archiva los pasos sobrantes.", "Devolver el espacio, detener la mengua o mudar el pueblo."),
  ("La Tesis Prohibida", "Un doctorando escondió su tesis: 'Cómo cerrar el Vacío'. {ant} quema bibliotecas buscándola.", "Encontrarla primero, protegerla o subastar capítulos.")],
 ["Los mapas de la zona caducan en días: orientarse cuesta el doble.",
  "Hablar del Vacío en voz alta cerca del cráter... lo acerca."],
 ["mes_de_les_estrelles", "mes_de_lescarcha"]),
"bandidos_del_camino_de_ceniza": ("el Camino de Ceniza",
 [("El Peaje Honrado", "{ant} cobra peaje con recibo y sonrisa. El dinero, dicen, paga a la guardia... la misma que debería echarlo.", "Seguir el dinero, montar caravana señuelo o comprar el peaje."),
  ("La Diligencia Fantasma", "Una diligencia recorre el camino sin caballos ni cochero. {ant} la asaltó una vez: no se habla de ello.", "Asaltarla, escoltarla o subirse."),
  ("Ceniza en los Bolsillos", "Todo lo robado en el camino aparece días después... convertido en ceniza. {ant} busca al 'competidor' furioso.", "Aliarse contra el fenómeno, investigarlo o dejar que se arruinen."),
  ("El Último Golpe", "{ant} planea retirarse con un golpe final: el carro del diezmo. Busca socios y ya vendió el plan dos veces.", "Unirse, delatarlo o robar el plan y darlo antes.")],
 ["Todo el mundo en el camino es informante de alguien.",
  "La guardia cobra por ambos bandos: cualquier arresto es negociable."],
 ["mes_de_les_ombres", "mes_de_la_cosecha"]),
"espectros_de_la_tomba": ("los pasillos exteriores de la Tomba",
 [("La Audiencia Pendiente", "{ant} concede audiencias a medianoche: resuelve pleitos de hace un siglo. El pueblo ha empezado a llevarle los de ahora.", "Usar el tribunal espectral, cerrarlo o apelar una sentencia."),
  ("El Baile Anual", "Una noche al año, la Tomba celebra baile y faltan parejas vivas. {ant} reparte las invitaciones.", "Aceptar la invitación, colarse o rescatar a los invitados del año pasado."),
  ("Frío de Archivo", "Los recuerdos del pueblo se están archivando solos: la gente olvida bodas, deudas, rencores. {ant} es el archivero.", "Robar los recuerdos de vuelta o negociar qué se olvida."),
  ("La Corona Prestada", "La corona funeraria del Rei Llop apareció en una casa de empeños. {ant} viene a recuperarla sin prisa y sin pausa.", "Devolverla con protocolo, venderla rápido o probársela.")],
 ["Toda promesa hecha en la Tomba la escuchan sus notarios muertos.",
  "El frío de dentro sale con vosotros: 1 de Fatiga por visita sin despedirse."],
 ["mes_dels_aullits", "mes_de_lescarcha"]),
"sagas_de_la_luna_hueca": ("la colina de las Tres Chimeneas",
 [("La Feria de los Favores", "Una feria nocturna vende 'favores pequeños'. {ant} apunta cada compra. Los precios se cobran en años.", "Comprar con cabeza, sabotear la feria o robar el libro de cuentas."),
  ("El Niño Cambiado", "Los padres juran que su hijo 'volvió distinto' del bosque. {ant} ofrece devolver al original... por otro a cambio.", "Aceptar el trato, rastrear al original o quedarse al cambiado."),
  ("Lana de Luna", "Las ovejas del valle dan lana que abriga sueños. {ant} compra toda la producción y el insomnio sube.", "Cortar el negocio, regular el precio o averiguar quién teje con ella."),
  ("La Receta de la Abuela", "Una sopa cura cualquier mal... y quien la toma olvida un nombre. {ant} reparte raciones gratis.", "Analizar la receta, prohibirla o pedir la contra-receta.")],
 ["Aceptar cualquier regalo en la colina crea deuda (lo sepa el jugador o no).",
  "Los gatos de la zona informan a las Sagas. Todos."],
 ["mes_de_la_siembra", "mes_de_laigua"]),
"milicia_corrupta_de_llerba": ("la Puerta Norte de Llerba",
 [("El Impuesto del Miedo", "La milicia cobra 'tasa de protección contra el Rei Llop'. {ant} sube la tarifa cada luna. El Rei, de momento, ni aparece.", "Demostrar el fraude, organizar impago o desviar la tasa a defensa real."),
  ("Reclutamiento Forzoso", "Se llevan a los jóvenes 'para instrucción'. Vuelven distintos o no vuelven. {ant} firma las levas.", "Falsificar exenciones, rescatar una leva o seguir a los reclutas."),
  ("La Puerta de Pago", "La única puerta abierta cobra entrada. La cerrada esconde por qué. {ant} guarda las dos.", "Pagar y callar, forzar la cerrada o comprar al guardián."),
  ("El Motín del Cuartel", "Media guarnición quiere amotinarse contra {ant}... y pide ayuda externa discreta.", "Apoyar el motín, delatarlo o negociar la rendición del mando.")],
 ["Todo altercado da excusa para subir la tasa: la violencia visible os cuesta aliados.",
  "Los honestos de la milicia son los que más vigilados están."],
 ["mes_del_ferro", "mes_del_sol"]),
"horrores_del_cielo_fragmentado": ("el páramo bajo el Cielo Roto",
 [("Lluvia de Anteayer", "Sobre el páramo llueve la lluvia de hace dos días, con los ecos de sus conversaciones. {ant} escucha algo que no debía oírse.", "Recuperar el eco, borrarlo o venderlo."),
  ("El Trozo que Falta", "A la luna le falta un trozo esta semana. {ant} lo arrastra por el páramo hacia el cráter.", "Devolverlo al cielo, esconderlo o dejar que llegue al cráter."),
  ("Brújulas Locas", "Toda brújula del reino señala al páramo desde el martes. {ant} monta guardia en el punto exacto.", "Llegar primero, desviar las brújulas o vigilar quién más viene."),
  ("La Grieta Doméstica", "Una familia vive con una grieta del cielo en la cocina: da buena luz y malos sueños. {ant} quiere comprar la casa.", "Sellar la grieta, proteger a la familia o pujar más.")],
 ["El cielo del páramo miente: la hora y el clima no son de fiar.",
  "Los éxitos críticos brillan aquí: cada uno atrae una astilla curiosa."],
 ["mes_de_les_estrelles", "mes_de_loracle"]),
}

# 100 NPCs: los 12 canon de las misiones + 88 generados con nombre propio
NPC_CANON = [
("Nara la Serena", "contacto", "Calle de los Faroles Rojos", "Proteger la red de refugio de las trabajadoras del barrio", "aliado", "Que amenacen a sus chicas", "mes_del_sol"),
("Damaso Clavijo", "informante", "Banco Mercantil de Bastrea", "Limpiar su conciencia sin perder la cabeza", "neutral", "Pruebas de que el banco caería con él dentro", "mes_de_loracle"),
("Madre Iru", "victima", "Camino de Ceniza", "Llevar a su gente a un lugar donde nadie diga 'refugiados'", "aliado", "Cualquier riesgo para los niños de la caravana", "mes_de_la_siembra"),
("Tonet el Panadero", "contacto", "Horno Comunal de Llerba", "Que el pan alcance para todos aunque la milicia robe", "aliado", "Tocarle el horno comunal", "mes_de_la_cosecha"),
("Velia Chispa", "rival", "Galería de los Vitrales", "Reventar la farsa de las reliquias falsas, con público", "neutral", "Ofrecerle un escenario mejor", "mes_de_les_ombres"),
("Eudald Cartograf", "informante", "Archivo del Anciano del Pueblo", "Terminar el mapa que cambia antes de que el mapa lo termine a él", "neutral", "Datos nuevos del mapa viviente", "mes_de_les_estrelles"),
("La Viuda Aloma", "misterio", "Puente del Último Aullido", "Terminar la lista de nombres que canta cada noche", "hostil", "Averiguar quién escribe la canción", "mes_dels_aullits"),
("Biel el Menut", "victima", "Pozo Azul de Llerba", "Recuperar la moneda que susurra... porque le habla a él", "aliado", "Que un adulto lo tome en serio por fin", "mes_de_laigua"),
("Ferran Mano Clara", "victima", "Plaza de las Campanas", "Sobrevivir a su condena sin delatar a los que escondió", "aliado", "Una salida que no manche a su familia", "mes_del_ferro"),
("Ona la Molinera", "contacto", "Molino del Río Negro", "Que el agua vuelva a bajar limpia y nadie pregunte más", "neutral", "Miedo a lo que hay canal arriba", "mes_de_la_terra"),
("Matrona Sibil", "autoridad", "Casa de Té del Farol Rojo", "Convertir la casa de té en santuario intocable", "aliado", "Dinero o músculo para la causa", "mes_del_sol"),
("Capitana Bruna", "autoridad", "Puerta Norte de Llerba", "Que la puerta aguante cerrada hasta cobrar lo prometido", "hostil", "El artículo del reglamento que la obliga a abrir", "mes_del_ferro"),
]

NOMBRES = ["Guifré", "Serena", "Old Marc", "Petra", "Ilun", "Bastià", "Roser", "Cosme", "Delia", "Ferran",
           "Guillema", "Hug", "Isabeu", "Jofre", "Laia", "Miquel", "Neus", "Ot", "Pau", "Quima",
           "Ramon", "Sança", "Tomàs", "Urraca", "Vidal"]
OFICIOS = [("mercader", "vender caro lo que compró barato en el peor sitio posible"),
           ("mentor", "encontrar un último aprendiz que no muera pronto"),
           ("informante", "saberlo todo y cobrar por partes"),
           ("autoridad", "conservar el cargo una temporada más, cueste lo que calle"),
           ("rival", "llegar antes que el grupo a lo mismo que el grupo"),
           ("contacto", "mantener su red viva un invierno más"),
           ("victima", "que alguien crea su versión antes de que sea tarde"),
           ("traidor", "vender al mejor postor lo que juró guardar")]


def main() -> None:
    enemigos = {p.stem: json.loads(p.read_text(encoding="utf-8")) for p in (CARTAS / "enemigos").glob("*.json")}
    loot_ids = sorted(p.stem for p in LOOT.glob("*.json"))
    por_faccion: dict[str, list[str]] = {}
    for eid, e in sorted(enemigos.items()):
        por_faccion.setdefault(e["faction"], []).append(eid)

    escritas = 0
    ids_historias: list[str] = []
    for fac, (lugar, tramas, complicaciones, meses) in TRAMAS.items():
        candidatos = por_faccion[fac]
        elites = [i for i in candidatos if enemigos[i]["rank"] in ("elite", "jefe")]
        for ti, (titulo, gancho, decision) in enumerate(tramas):
            for esc in range(5):  # 5 escalones de tier por trama madre
                tier = min(5, 1 + esc)
                pool = elites if tier >= 3 else candidatos
                ant_id = pool[(ti * 5 + esc * 3) % len(pool)]
                ant = enemigos[ant_id]
                sufijo = ["El Rumor", "La Pista", "El Encargo", "La Verdad", "El Ajuste"][esc]
                nombre = f"{titulo} — {sufijo}"
                reward = f"loot_{fac}_t{tier}"
                if reward not in loot_ids:
                    reward = loot_ids[(ti + esc) % len(loot_ids)]
                historia = {
                    "id": f"hist_{slugify(nombre)}", "title": nombre, "type": "story",
                    "hook": gancho.format(ant=ant["name"]) + f" (Escalón {esc + 1} de la trama '{titulo}'.)",
                    "location": lugar,
                    "antagonist": ant_id,
                    "tierRange": [tier, min(5, tier + 1)],
                    "reward": reward,
                    "decision": decision,
                    "complication": complicaciones[esc % 2],
                    "months": meses,
                    "faction": fac,
                    "chain": {"trama": slugify(titulo), "step": esc + 1, "of": 5},
                    "flavorText": ant.get("flavorText", ""),
                }
                if escribir(HIST / f"{historia['id']}.json", historia):
                    escritas += 1
                ids_historias.append(historia["id"])

    # --- NPCs ---
    escritos_npc = 0
    todos = []
    for (nombre, rol, lugar, agenda, actitud, palanca, mes) in NPC_CANON:
        todos.append((nombre, rol, lugar, agenda, actitud, palanca, mes, None))
    i = 0
    facs = sorted(TRAMAS)
    while len(todos) < 100:
        fac = facs[i % len(facs)]
        lugar = TRAMAS[fac][0]
        oficio, agenda = OFICIOS[i % len(OFICIOS)]
        APODOS = ['el Breve', 'la Justa', 'de la Esquina', 'Dosveces', 'el Callado',
                  'la Prestada', 'del Vado', 'Sinmiedo', 'la Vieja', 'el Nuevo']
        # combinación (i % 25, i // 25) única hasta 250: sin colisiones de nombre
        nombre = f"{NOMBRES[i % len(NOMBRES)]} {APODOS[(i // len(NOMBRES)) % 10]}"
        mes = MESES[i % 12]
        hist = ids_historias[(i * 7) % len(ids_historias)]
        todos.append((nombre, oficio, lugar, agenda, ["aliado", "neutral", "hostil"][i % 3],
                      "Lo que descubra el grupo sobre su secreto", mes, hist))
        i += 1
    for j, (nombre, rol, lugar, agenda, actitud, palanca, mes, hist) in enumerate(todos[:100]):
        hist = hist or ids_historias[(j * 11) % len(ids_historias)]
        npc = {
            "id": f"npc_{slugify(nombre)}", "name": nombre, "type": "npc", "role": rol,
            "location": lugar, "faction": None,
            "agenda": agenda,
            "attitude": {"inicial": actitud, "palanca": palanca},
            "dialogue": [
                "No necesito santos. Necesito gente que termine lo que empieza.",
                f"Si preguntas por {lugar.split()[0].lower()}... pregunta bajito.",
                "Esta ciudad recuerda los métodos tanto como los resultados.",
            ],
            "services": None,
            "secretHook": hist,
            "month": mes,
            "flavorText": f"Nacido en {mes.replace('_', ' ').replace('mes de ', 'el mes de ')}.",
        }
        if escribir(NPCS / f"{npc['id']}.json", npc):
            escritos_npc += 1

    print(f"Historias escritas: {escritas} · NPCs escritos: {escritos_npc}")


if __name__ == "__main__":
    main()
