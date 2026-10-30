import sys
sys.path.insert(0, str(__import__("pathlib").Path(__file__).resolve().parent))
from ver import frames_de, tira
from PIL import Image
import senor  # noqa: F401

PROPOSTES = [
    ("El Duc", {"raza": "race_human_marches", "barba": None, "pelo": ("largo", "pelo_blanco"), "ojos": "violeta",
                "equipo": {"cabeza": "esb_diadema_duc", "torso": "esb_casaca_duc", "piernas": "esb_calces_duc",
                           "pies": "esb_botes_duc", "arma": "orbe"}},
     {"arma": [("cristal_azul", "mana_duc"), ("plata", "oro_duc")],
      "mano_izq": [("cristal_azul", "mana_duc"), ("plata", "oro_duc")]}),
    ("El Sobirà sense rostre", {"raza": "race_human_marches", "barba": None, "pelo": None,
                "equipo": {"cabeza": "esb_yelmo_sobira", "torso": "esb_placa_sobira",
                           "piernas": "esb_grebes_sobira", "pies": "esb_sabates_sobira", "arma": "orbe"}},
     {"arma": [("cristal_azul", "visera_sobira"), ("plata", "oro_sobira")],
      "mano_izq": [("cristal_azul", "visera_sobira"), ("plata", "oro_sobira")]}),
    ("El Rei de la Forja", {"raza": "race_human_marches", "barba": ("BARBA_CORTA", "pelo_pelirrojo"),
                "pelo": ("corto", "pelo_pelirrojo"), "ojos": "ambar",
                "equipo": {"cabeza": "esb_corona_forja", "torso": "esb_armadura_forja",
                           "piernas": "esb_grebes_forja", "pies": "esb_botes_forja", "arma": "orbe"}},
     {"arma": [("cristal_azul", "or_fos"), ("plata", "ferro_forja")],
      "mano_izq": [("cristal_azul", "or_fos"), ("plata", "ferro_forja")]}),
]

if __name__ == "__main__":
    tiras = []
    for nombre, spec, tintes in PROPOSTES:
        fr = frames_de(spec, tintes)
        tiras.append(tira(fr, [17, 15, 13, 19, 7, 9, 4, 6], esc=4, caja=(4, 0, 60, 64)))
    w, h = tiras[0].size
    img = Image.new("RGB", (w, h * len(tiras)))
    for i, t in enumerate(tiras):
        img.paste(t, (0, i * h))
    img.save(str(__import__("pathlib").Path(__file__).resolve().parent / "tres.png")); print(img.size)
