// UI-BG3-01 — Las hojas de rejilla del cuaderno de creacion: Raza (C02a),
// Clase (C02b), Origen y Deidad (la misma rejilla, con iconos). A la
// izquierda la rejilla, en el centro el personaje y sus atributos y a la
// derecha la ficha de lo senalado.
#include <algorithm>
#include <cmath>

#include "Game/CatalogoCartas.h"
#include "PaginasCreacionInterno.h"

namespace Pergamino::creacion {

namespace {

const char* const kStats[4] = {"CON", "DES", "INT", "CAR"};
const char* const kIconosStat[4] = {"con", "des", "int", "car"};

int indiceStat(const std::string& s) {
    for (int i = 0; i < 4; ++i)
        if (s == kStats[i]) return i;
    return 0;
}

const char* rareza(RPG::ItemRarity r) {
    switch (r) {
        case RPG::ItemRarity::Common: return "COMÚN";
        case RPG::ItemRarity::Uncommon: return "POCO COMÚN";
        case RPG::ItemRarity::Rare: return "RARA";
        case RPG::ItemRarity::Epic: return "ÉPICA";
        case RPG::ItemRarity::Legendary: return "LEGENDARIA";
        case RPG::ItemRarity::Mythic: return "MÍTICA";
    }
    return "";
}

std::string unidas(const std::vector<std::string>& v, const char* sep = " · ") {
    std::string s;
    for (const std::string& x : v) s += (s.empty() ? "" : sep) + x;
    return s;
}

// Lo que no se puede explicar aun (nada senalado).
void fichaVacia(const Contexto& k, const char* texto) {
    k.p.parrafo(k.l, Letra::Lora18i, texto, kFichaX, 130.0f, kFichaW, 26.0f, Color::kTinta3, 3);
}

const char* iconoRanura(const std::string& slot) {
    if (slot == "Head") return "ranura_cabeza";
    if (slot == "Torso") return "ranura_torso";
    if (slot == "Legs") return "ranura_piernas";
    if (slot == "Feet") return "ranura_pies";
    if (slot == "Accessory") return "ranura_accesorio";
    // PART-02: hasta que tengan icono propio, el del accesorio (y los brazos, el del torso)
    if (slot == "Ring" || slot == "Ring2" || slot == "Neck" || slot == "Belt") return "ranura_accesorio";
    if (slot == "Arms") return "ranura_torso";
    return "ranura_mano_principal";
}

// Glifo y color del tipo de una carta: ■ principal, ▶ libre, ◆ reaccion,
// ● canaliza, ▲ movimiento (los de la barra de combate).
void tipoDeCarta(RPG::ActionType t, const char*& glifo, Vector4& color) {
    switch (t) {
        case RPG::ActionType::FREE_ACTION: glifo = "▶"; color = Color::kExito; break;
        case RPG::ActionType::REACTION: glifo = "◆"; color = Color::kCarmin; break;
        case RPG::ActionType::CHANNEL: glifo = "●"; color = Color::kInfo; break;
        case RPG::ActionType::MOVEMENT: glifo = "▲"; color = Color::kAviso; break;
        default: glifo = "■"; color = Color::kTinta; break;
    }
}

}  // namespace

// --- Raza (C02a) ----------------------------------------------------------

void pintaRaza(const Contexto& k) {
    k.p.pixel(k.l, "ELIGE TU PUEBLO", kIzqX, 122.0f, 2, Color::kTinta3);
    // Buscar: clic en la caja y escribir (las letras no mueven el foco).
    const Rect caja{410.0f, 114.0f, 224.0f, 36.0f};
    const bool buscando = k.c.buscando();
    k.l.color(caja, buscando ? Color::kPapelLuz : rgb(242, 228, 193, 0.7f));
    k.p.contorno(k.l, caja, buscando ? 2.0f : 1.0f, buscando ? Color::kCarmin : rgb(107, 80, 56, 0.6f));
    k.p.pixel(k.l, "/", caja.x + 10.0f, caja.y + 9.0f, 2, Color::kTinta3);
    if (k.c.busqueda().empty() && !buscando) {
        k.p.texto(k.l, Letra::Lora16, "Buscar raza…", caja.x + 30.0f, caja.y + 6.0f, 24.0f, Color::kTinta3);
    } else {
        const float w = k.p.texto(k.l, Letra::Lora18, k.c.busqueda(), caja.x + 30.0f, caja.y + 5.0f, 26.0f,
                                  Color::kTinta);
        if (buscando && std::fmod(k.c.segundos(), 1.0) < 0.6)
            k.l.color(Rect{caja.x + 32.0f + w, caja.y + 8.0f, 2.0f, 20.0f}, Color::kCarmin);
    }
    k.l.zona(caja, tipo(ZonaC::Buscar));
    pintaRejilla(k, kIzqX + 2.0f, 158.0f, true, "personaje");
    pintaCuenta(k, kIzqX, 590.0f, "rueda del ratón o S para ver más");

    const Cuaderno::Vista v = k.c.vista();
    pintaCentro(k, v, 350.0f, 488.0f);
    k.p.texto(k.l, Letra::Lora16, "Todo empieza a 0; tu clase suma +1 a sus dos principales.", kCentroX, 584.0f, 22.0f, Color::kTinta3);

    const RPG::RaceDefinition* r = k.c.datos().reglas->races().find(k.c.senalado());
    if (r == nullptr) {
        fichaVacia(k, "Pasa el ratón por un pueblo para ver su ficha.");
        return;
    }
    float y = fichaTitulo(k, r->name, 116.0f);
    k.p.insignia(k.l, rareza(r->rarity), kFichaX, y, Color::kTinta2);
    y += 40.0f;  // AV16-02: la raza ya no suma (todo empieza a 0): sin linea de bonos
    const Cuaderno::Rasgo* rasgo = nullptr;
    if (k.c.datos().rasgos != nullptr) {
        const auto it = k.c.datos().rasgos->find(r->id);
        if (it != k.c.datos().rasgos->end()) rasgo = &it->second;
    }
    if (rasgo != nullptr) y = fichaBloque(k, "RASGO", rasgo->nombre, rasgo->texto, y + 4.0f, 4);
    y = fichaBloque(k, "CULTURA", "", Cuaderno::culturaDe(r->description), y, 5);  // RAZAS-01
    if (!r->affinities.empty() && y < 590.0f) {
        k.p.texto(k.l, Letra::Lora16, "Afinidades", kFichaX, y, 22.0f, Color::kTinta3);
        k.p.parrafo(k.l, Letra::Lora16, unidas(r->affinities), kFichaX + 90.0f, y, kFichaW - 90.0f, 20.0f,
                    Color::kTinta, 2);
    }
}

// --- Clase (C02b) ---------------------------------------------------------

void pintaClase(const Contexto& k) {
    k.p.pixel(k.l, "ELIGE TU OFICIO", kIzqX, 118.0f, 2, Color::kTinta3);
    k.p.texto(k.l, Letra::Lora16, "El icono de la esquina es su atributo principal", 312.0f, 114.0f, 24.0f,
              Color::kTinta3);
    float x = kIzqX + 2.0f;
    const auto& roles = Cuaderno::roles();
    for (std::size_t i = 0; i < roles.size(); ++i) {
        std::string t = roles[i].second;
        if (i == 0) {  // DIALOGOS-01: las que se juegan
            int jugables = 0;
            k.c.datos().reglas->classes().forEach([&](const RPG::ClassDefinition& d) { jugables += d.playable ? 1 : 0; });
            t += " " + std::to_string(jugables);
        }
        const float w = chip(k, t, x, 146.0f, k.c.rol() == roles[i].first);
        k.l.zona(Rect{x, 146.0f, w, 30.0f}, tipo(ZonaC::Rol), static_cast<int>(i));
        x += w + 8.0f;
    }
    pintaRejilla(k, kIzqX + 2.0f, 192.0f, true, "personaje");
    pintaCuenta(k, kIzqX, 520.0f, "filtra por papel o baja con la rueda");

    const Cuaderno::Vista v = k.c.vista();
    const float cx = kCentroX + kCentroW * 0.5f;
    pintaVista(k, v, cx, 318.0f, 3);
    // El kit del cofre de esta clase (AV16-02: sin decir lo que sube). TUTORIAL-01: con cinco piezas (el arma y cuatro) empieza
    // mas arriba y mas junto, para no pisar «CON EL KIT PUESTO».
    const std::string id = k.c.senalado();
    const RPG::ClassDefinition* clase = k.c.datos().reglas->classes().find(id);
    const std::vector<std::string> piezas = clase != nullptr ? k.c.kit(id).piezas : std::vector<std::string>{};
    const float paso = piezas.size() > 4 ? 21.0f : 24.0f;
    float y = piezas.size() > 4 ? 360.0f : 372.0f;
    if (clase != nullptr && k.c.datos().equipo != nullptr) {
        k.p.tituloBloque(k.l, "KIT DEL COFRE", kCentroX, y, kCentroW);
        y += 26.0f;
        for (const std::string& pieza : piezas) {
            const RPG::EquipmentDefinition* e = k.c.datos().equipo->find(pieza);
            if (e == nullptr) continue;
            k.p.icono(k.l, iconoRanura(e->slot), kCentroX, y + 4.0f, 1);
            const std::string bono;  // AV16-02: las prendas del kit no dicen que sumen
            const float wb = static_cast<float>(k.p.letras().ancho(Letra::Lora16, bono));
            k.p.texto(k.l, Letra::Lora16, k.p.letras().recorta(Letra::Lora16, e->name, kCentroW - 30.0f - wb - 8.0f),
                      kCentroX + 22.0f, y, 24.0f, Color::kTinta);
            k.p.texto(k.l, Letra::Lora16, bono, kCentroX + kCentroW - wb, y, 24.0f, Color::kExito);
            y += paso;
        }
    }
    k.p.tituloBloque(k.l, "AL EMPEZAR", kCentroX, 498.0f, kCentroW);
    pintaAtributos(k, v, kCentroX, 526.0f);

    if (clase == nullptr) {
        fichaVacia(k, "Pasa el ratón por una clase para ver su ficha.");
        return;
    }
    y = fichaTitulo(k, clase->name, 112.0f);
    // ÁGIL · ★ CAR · ✎ DES · ♥ 10
    std::string rol = Cuaderno::nombreRol(clase->role);
    for (char& ch : rol) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    float xc = kFichaX + k.p.insignia(k.l, rol.empty() ? std::string("CLASE") : rol, kFichaX, y, Color::kTinta2) + 12.0f;
    for (const std::string& s : {clase->primaryStat, clase->secondaryStat}) {
        k.p.icono(k.l, kIconosStat[indiceStat(s)], xc, y + 5.0f, 1);
        xc += 20.0f + k.p.texto(k.l, Letra::Lora18, s, xc + 20.0f, y, 26.0f, Color::kTinta) + 12.0f;
    }
    k.p.icono(k.l, "pv", xc, y + 5.0f, 1);
    k.p.texto(k.l, Letra::Lora18, std::to_string(clase->baseHealth), xc + 20.0f, y, 26.0f, Color::kTinta);
    y += 36.0f;
    y += k.p.parrafo(k.l, Letra::Lora18, clase->description, kFichaX, y, kFichaW, 23.0f, Color::kTinta2, 2) + 8.0f;
    const Game::CatalogoCartas* cartas = k.c.datos().cartas;
    if (cartas != nullptr) {
        if (const RPG::PassiveDefinition* pas = cartas->pasiva(clase->passiveId))
            y = fichaBloque(k, "PASIVA", pas->name, pas->description, y, 2);
    }
    // La mano: las de tier 1 se juegan; las de creador (tier 2) esperan.
    const std::vector<std::string>& todas =
        clase->learnableSkillIds.empty() ? clase->startingSkillIds : clase->learnableSkillIds;
    std::vector<std::string> mano, creador;
    for (const std::string& c : todas) {
        const Game::CatalogoCartas::Carta* carta = cartas != nullptr ? cartas->carta(c) : nullptr;
        if (carta != nullptr && carta->def->tier_min >= 2) creador.push_back(carta->def->name);
        else mano.push_back(c);
    }
    k.p.tituloBloque(k.l, "TU MANO · " + std::to_string(mano.size()) + " + " + std::to_string(creador.size()), kFichaX,
                     y, kFichaW);
    y += 24.0f;
    for (const std::string& c : mano) {
        if (y > 596.0f) break;
        const Game::CatalogoCartas::Carta* carta = cartas != nullptr ? cartas->carta(c) : nullptr;
        const char* glifo = "■";
        Vector4 color = Color::kTinta;
        if (carta != nullptr) tipoDeCarta(carta->def->action_type, glifo, color);
        k.p.pixel(k.l, glifo, kFichaX + 2.0f, y + 3.0f, 2, color);
        k.p.texto(k.l, Letra::Lora16,
                  k.p.letras().recorta(Letra::Lora16, carta != nullptr ? carta->def->name : c, kFichaW - 22.0f),
                  kFichaX + 22.0f, y, 21.0f, Color::kTinta);
        y += 21.0f;
    }
    if (!creador.empty() && y < 600.0f) {
        k.p.pixel(k.l, "T2", kFichaX, y + 6.0f, 2, Color::kTinta3);
        k.p.parrafo(k.l, Letra::Lora16, unidas(creador), kFichaX + 30.0f, y + 2.0f, kFichaW - 30.0f, 20.0f,
                    Color::kTinta3, 2);
    }
}

// --- Origen y Deidad --------------------------------------------------------

void pintaOrigen(const Contexto& k) {
    k.p.pixel(k.l, "ELIGE TU ORIGEN", kIzqX, 122.0f, 2, Color::kTinta3);
    k.p.texto(k.l, Letra::Lora16, "Tu mes de nacimiento: de él salen las preguntas de tu historia", kIzqX, 148.0f,
              24.0f, Color::kTinta3);
    pintaRejilla(k, kIzqX + 2.0f, 186.0f, false, "lore");
    pintaCuenta(k, kIzqX, 514.0f, "baja con la rueda");
    pintaCentro(k, k.c.vista(), 360.0f, 500.0f);
    const RPG::BackgroundDefinition* b = k.c.datos().reglas->backgrounds().find(k.c.senalado());
    if (b == nullptr) {
        fichaVacia(k, "Pasa el ratón por un origen para ver su ficha.");
        return;
    }
    float y = fichaTitulo(k, b->name, 116.0f);
    k.p.insignia(k.l, rareza(b->rarity), kFichaX, y, Color::kTinta2);
    y = fichaBloque(k, "QUIÉN NACE AQUÍ", "", b->description, y + 40.0f, 6);
    if (const auto* o = k.c.datos().reglas->opcionesDe(b->id); o != nullptr && !o->vacio())
        fichaBloque(k, "TU HISTORIA", "", "Siete preguntas te esperan en la hoja Historia: vínculo, miedo, defecto, "
                                          "meta, ideal, carácter y virtud.",
                    y, 4);
}

void pintaDeidad(const Contexto& k) {
    k.p.pixel(k.l, "A QUIÉN REZAS", kIzqX, 122.0f, 2, Color::kTinta3);
    k.p.texto(k.l, Letra::Lora16, "Es opcional: «Ninguna» también vale", kIzqX, 148.0f, 24.0f, Color::kTinta3);
    pintaRejilla(k, kIzqX + 2.0f, 186.0f, false, "deidad");
    pintaCuenta(k, kIzqX, 514.0f, "baja con la rueda");
    pintaCentro(k, k.c.vista(), 360.0f, 500.0f);
    const std::string id = k.c.senalado();
    if (id.empty()) {
        float y = fichaTitulo(k, "Ninguna", 116.0f);
        k.p.parrafo(k.l, Letra::Lora18, "Empiezas sin deidad. Podrás encontrar fe por el camino.", kFichaX, y + 4.0f,
                    kFichaW, 24.0f, Color::kTinta2, 3);
        return;
    }
    const RPG::DeityDefinition* d = k.c.datos().reglas->deities().find(id);
    if (d == nullptr) return;
    float y = fichaTitulo(k, d->name, 116.0f);
    if (!d->title.empty()) {
        k.p.texto(k.l, Letra::Lora18i, k.p.letras().recorta(Letra::Lora18i, d->title, kFichaW), kFichaX, y, 26.0f,
                  Color::kTinta2);
        y += 30.0f;
    }
    if (!d->domains.empty()) {
        k.p.parrafo(k.l, Letra::Lora16, "Dominio: " + unidas(d->domains, ", "), kFichaX, y, kFichaW, 21.0f,
                    Color::kTinta3, 2);
        y += 46.0f;
    }
    // DIOSES-01: los Creadores no traen obligación ni favor; su dogma tiene sitio.
    y = fichaBloque(k, "DOGMA", "", d->description, y, d->obligations.empty() ? 10 : 3);
    // El mensaje de cada Creador (Astrologia.docx), si lo tiene.
    if (!d->flavorText.empty() && d->flavorText != d->description)
        y = fichaBloque(k, "MENSAJE", "", "«" + d->flavorText + "»", y, 2);
    if (!d->obligations.empty()) y = fichaBloque(k, "OBLIGACIÓN", "", d->obligations.front(), y, 3);
    if (!d->favorDescription.empty() && y < 560.0f) fichaBloque(k, "FAVOR", "", d->favorDescription, y, 3);
}

}  // namespace Pergamino::creacion
