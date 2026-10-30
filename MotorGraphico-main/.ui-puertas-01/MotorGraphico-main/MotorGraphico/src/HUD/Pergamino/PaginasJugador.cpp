// PERGAMINO-01 — ver include/HUD/Pergamino/PaginasJugador.h.
#include "HUD/Pergamino/PaginasJugador.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>

#include "Core/Json/JsonValue.h"
#include "Game/AuraMoral.h"
#include "Game/Codice.h"
#include "Game/ComercioReputacion.h"
#include "Game/DiarioMisiones.h"
#include "HUD/Pergamino/PaginaMisiones.h"
#include "Game/FichaVista.h"
#include "Game/GameSession.h"
#include "Game/InventoryMenuState.h"
#include "Game/InventoryScreen.h"
#include "Level/ObjectCatalog.h"
#include "RPG/CharacterSheet.h"

namespace Pergamino {

namespace {

const char* const kNombresStat[4] = {"Constitución", "Destreza", "Inteligencia", "Carisma"};

std::string unir(const std::vector<std::string>& partes, const char* sep) {
    std::string out;
    for (const std::string& p : partes) {
        if (p.empty()) continue;
        if (!out.empty()) out += sep;
        out += p;
    }
    return out;
}

const char* singularCodice(const std::string& tipo) {
    if (tipo == "personaje") return "Personaje";
    if (tipo == "lugar") return "Lugar";
    if (tipo == "deidad") return "Deidad";
    if (tipo == "lore") return "Lore";
    if (tipo == "curiosidad") return "Curiosidad";
    return "";
}

// Los motivos y resultados del motor, como los pide el modelo visual: una
// frase con mayuscula y punto (P04a: «Tus PV ya están al máximo. No se gasta
// nada.») y sin nombrar el maná, que ya no sale en la interfaz (29/09).
std::string frase(const std::string& texto) {
    if (texto.empty()) return texto;
    if (texto == "ya tienes la vida al maximo") return "Tus PV ya están al máximo. No se gasta nada.";
    if (texto.find("mana") != std::string::npos) return "Ahora no tendría efecto. No se gasta nada.";
    std::string s = texto;
    if (s[0] >= 'a' && s[0] <= 'z') s[0] = static_cast<char>(s[0] - 'a' + 'A');
    const char fin = s.back();
    if (fin != '.' && fin != '?' && fin != '!') s += '.';
    return s;
}

// La linea de estado de la hoja: el resultado de la ultima accion, salvo
// que repita el motivo que ya se lee junto al boton.
std::string estadoHoja(const std::string& mensaje, const std::string& motivo) {
    const std::string e = frase(mensaje);
    return e == motivo ? std::string() : e;
}

Game::FichaDatos fichaDe(const FuentesCuaderno& f) {
    return f.pantalla != nullptr ? f.pantalla->ficha() : Game::FichaDatos{};
}

PaginaCuaderno paginaVacia(const char* titulo, const char* icono, const char* t, const char* d) {
    PaginaCuaderno pg;
    pg.tipo = PaginaCuaderno::Tipo::Vacia;
    pg.titulo = titulo;
    pg.vacioIcono = icono;
    pg.vacioTitulo = t;
    pg.vacioTexto = d;
    return pg;
}

PaginaCuaderno inventario(const FuentesCuaderno& f) {
    PaginaCuaderno pg;
    pg.titulo = "INVENTARIO";
    pg.cabeceraIzq = "OBJETO";
    pg.cabeceraDer = "CANT.";
    pg.elegir = "Objeto";
    Game::InventoryScreen& pantalla = *f.pantalla;
    // El modelo da el orden y el permiso; InventoryMenuState, el detalle.
    Game::InventoryMenuState listado(*f.sesion);
    std::map<std::string, Game::InventoryRow> filas;
    for (auto c : {Game::ItemCategory::Consumable, Game::ItemCategory::Equipment, Game::ItemCategory::Quest,
                   Game::ItemCategory::Other}) {
        listado.setCategory(c);
        for (const Game::InventoryRow& r : listado.rows()) filas[r.objectId] = r;
    }
    const auto& entradas = pantalla.entriesInFocus();
    // La mochila vacia no es una fila con foco: es la pagina vacia, como
    // ACCIONES (MODELO_VISUAL.md, estados vacios).
    if (entradas.size() == 1 && entradas.front().itemId.empty() &&
        entradas.front().action == Game::InventoryAction::None) {
        PaginaCuaderno vacia = paginaVacia("INVENTARIO", "equipo", "La mochila está vacía",
                                           "Lo que compres en las tiendas o recojas por el camino "
                                           "aparecerá aquí.");
        vacia.elegir.clear();
        return vacia;
    }
    for (const Game::InventoryEntry& e : entradas) {
        FilaPagina fila;
        const auto it = filas.find(e.itemId);
        if (!e.itemId.empty() && it != filas.end()) {
            fila.icono = iconoCategoria(it->second.category);
            fila.nombre = it->second.name;
            fila.valor = "×" + std::to_string(it->second.quantity);
        } else {
            fila.nombre = e.label == "(vacío)" ? "La mochila está vacía" : e.label;
            fila.apagada = e.action == Game::InventoryAction::None;
        }
        pg.filas.push_back(fila);
    }
    pg.foco = static_cast<int>(std::min(pantalla.selectedInFocus(), pg.filas.empty() ? 0 : pg.filas.size() - 1));
    const Game::InventoryEntry& e = pantalla.selectedEntry();
    DetallePagina& d = pg.detalle;
    const auto it = filas.find(e.itemId);
    if (e.itemId.empty() || it == filas.end()) {
        d.titulo = pg.filas.empty() ? "" : pg.filas[static_cast<std::size_t>(pg.foco)].nombre;
        if (e.action == Game::InventoryAction::ConfirmTarget) {
            d.parrafos.push_back("¿Sobre quién se usa?");
            d.boton = "ELEGIR";
            d.botonActivo = true;
            pg.verbo = "Elegir";
        } else if (e.action == Game::InventoryAction::CancelTarget) {
            d.parrafos.push_back("No se gasta nada.");
            d.boton = "CANCELAR";
            d.botonActivo = true;
            pg.verbo = "Cancelar";
        } else {
            d.parrafos.push_back(e.reason.empty() ? "Compra en las tiendas de la plaza o recoge lo que encuentres."
                                                  : frase(e.reason));
        }
        d.estado = frase(pantalla.message());
        return pg;
    }
    const Game::InventoryRow& r = it->second;
    d.icono = iconoCategoria(r.category);
    d.titulo = r.name;
    d.insignia = insigniaCategoria(r.category);
    const ObjectCatalog* catalogo = f.sesion->objectCatalog();
    const ObjectDefinition* def = catalogo != nullptr ? catalogo->find(r.objectId) : nullptr;
    const bool cura = def != nullptr && def->pickup.effect == PickupEffect::Heal && def->pickup.power > 0;
    if (cura) {
        d.efectoIcono = "pv";
        d.efecto = "Recupera " + std::to_string(def->pickup.power) + " PV.";
    } else if (r.category != Game::ItemCategory::Consumable && !r.description.empty()) {
        // El efecto de maná no se nombra: el maná ya no sale en la interfaz.
        d.parrafos.push_back(r.description);
    }
    d.datos.push_back({"Tienes", std::to_string(r.quantity)});
    if (cura) d.datos.push_back({"Tus PV", std::to_string(pantalla.health()) + " de " + std::to_string(pantalla.maxHealth())});
    if (e.action == Game::InventoryAction::UseItem) {
        d.boton = "USAR";
        pg.verbo = "Usar";
    } else if (e.action == Game::InventoryAction::EquipItem) {
        d.boton = "EQUIPAR";
        pg.verbo = "Equipar";
    }
    d.botonActivo = e.reason.empty();
    d.motivo = frase(e.reason);
    d.estado = estadoHoja(pantalla.message(), d.motivo);
    return pg;
}

PaginaCuaderno equipo(const FuentesCuaderno& f) {
    const Game::FichaDatos ficha = fichaDe(f);
    if (ficha.vacia || ficha.ranuras.empty())
        return paginaVacia("EQUIPO", "ranura_torso", "Tu personaje no tiene ficha.",
                           "Empieza una partida nueva para tener ficha y ranuras de equipo.");
    PaginaCuaderno pg;
    pg.titulo = "EQUIPO";
    pg.cabeceraIzq = "RANURA";
    pg.elegir = "Ranura";
    Game::InventoryScreen& pantalla = *f.pantalla;
    const auto& entradas = pantalla.entriesInFocus();
    for (std::size_t i = 0; i < entradas.size(); ++i) {
        const int ranura = entradas[i].slotIndex;
        const Game::FichaRanura* r =
            ranura >= 0 && static_cast<std::size_t>(ranura) < ficha.ranuras.size() ? &ficha.ranuras[static_cast<std::size_t>(ranura)] : nullptr;
        FilaPagina fila;
        fila.escalaIcono = 1;
        fila.icono = r ? r->icono : "";
        fila.nombre = r ? r->etiqueta : entradas[i].label;
        fila.valorPixel = false;
        fila.valor = (r == nullptr || r->objetoId.empty()) ? "Vacío" : r->nombre;
        // PART-02: la que el tier aun no abre (los anillos, el collar y el cinturón, desde el 2)
        if (r != nullptr && r->objetoId.empty() && r->abreEnTier > 0)
            fila.valor = "Desde el Tier " + std::to_string(r->abreEnTier);
        pg.filas.push_back(fila);
    }
    pg.foco = static_cast<int>(std::min(pantalla.selectedInFocus(), pg.filas.empty() ? 0 : pg.filas.size() - 1));
    const Game::InventoryEntry& e = pantalla.selectedEntry();
    const Game::FichaRanura* r = e.slotIndex >= 0 && static_cast<std::size_t>(e.slotIndex) < ficha.ranuras.size()
                                     ? &ficha.ranuras[static_cast<std::size_t>(e.slotIndex)]
                                     : nullptr;
    DetallePagina& d = pg.detalle;
    d.icono = r ? r->icono : "";
    d.titulo = r ? r->etiqueta : e.label;
    if (r != nullptr && !r->objetoId.empty()) {
        d.parrafos.push_back("Llevas " + r->nombre + ".");
        d.boton = "DESEQUIPAR";
        d.botonActivo = e.action == Game::InventoryAction::UnequipItem && e.reason.empty();
        d.motivo = frase(e.reason);
        pg.verbo = "Desequipar";
    } else if (r != nullptr && r->abreEnTier > 0) {
        d.parrafos.push_back("Esta ranura se abre en el Tier " + std::to_string(r->abreEnTier) + ".");
        d.nota = "En el Tier 1 solo hay casco, pechera, brazos, piernas, zapatos y armas; los anillos, el "
                 "collar y el cinturón llegan en el Tier 2.";
    } else {
        d.parrafos.push_back("No llevas nada en esta ranura.");
        d.nota = "Las piezas se equipan desde la hoja de Inventario.";
    }
    d.estado = estadoHoja(pantalla.message(), d.motivo);
    return pg;
}

std::string detalleStat(const Game::FichaStat& s) {
    std::vector<std::string> partes;
    auto suma = [&](int v, const char* que) {
        if (v != 0) partes.push_back((v > 0 ? "+ " : "- ") + std::to_string(std::abs(v)) + " " + que);
    };
    suma(s.raza, "raza");
    suma(s.clase, "clase");
    suma(s.equipo, "equipo");
    suma(s.dotes, "dotes");
    suma(s.condiciones, "estados");
    if (partes.empty()) return "base " + std::to_string(s.base);
    return std::to_string(s.base) + " " + unir(partes, " ");
}

PaginaCuaderno fichaPagina(const FuentesCuaderno& f) {
    const Game::FichaDatos ficha = fichaDe(f);
    if (ficha.vacia)
        return paginaVacia("FICHA", "personaje", "Tu personaje no tiene ficha.",
                           "Empieza una partida nueva y crea a tu personaje: aquí verás su raza, su clase y su historia.");
    PaginaCuaderno pg;
    pg.tipo = PaginaCuaderno::Tipo::Ficha;
    pg.titulo = "FICHA";
    FichaPagina& fp = pg.ficha;
    fp.identidad = {{"Raza", ficha.raza}, {"Clase", ficha.clase}, {"Trasfondo", ficha.trasfondo},
                    {"Deidad", ficha.deidad.empty() ? "Sin deidad" : ficha.deidad}};
    std::vector<std::string> condiciones;
    for (const Game::FichaCondicion& c : ficha.condiciones) condiciones.push_back(c.nombre);
    const Morality& moralidad = f.sesion->morality();
    fp.estado = {{"Moral", textoMoral(moralidad.value())},
                 {"Condiciones", condiciones.empty() ? "Ninguna" : unir(condiciones, ", ")}};
    // MORAL-01: la pureza, cuando algo profano te ha tocado; y lo que no se va.
    if (moralidad.pureza() < Morality::kPurezaMax) fp.estado.push_back({"Pureza", std::to_string(moralidad.pureza())});
    if (!moralidad.marcas().empty()) fp.estado.push_back({"Marcas", moralidad.marcas().back().motivo});
    // AV14-23 (D-MOR-09): con quien te llevas bien y con quien mal.
    for (const auto& [faccion, como] : Game::reputacionEnLaFicha(Game::facciones(), f.sesion->characterSheet()))
        fp.reputacion.push_back({faccion, como});
    anotaCicatrices(fp.estado, f.cicatrices);  // AV13-33
    for (std::size_t i = 0; i < 4; ++i) {
        const Game::FichaStat& s = ficha.stats[i];
        fp.stats[i] = StatPagina{s.icono, kNombresStat[i], s.clave, detalleStat(s), s.total};
    }
    if (!ficha.rasgos.empty()) {
        fp.rasgoIcono = ficha.rasgos.front().icono;
        fp.rasgoNombre = ficha.rasgos.front().nombre;
        fp.rasgoDescripcion = ficha.rasgos.front().descripcion;
    }
    const RPG::CharacterSheet* hoja = f.sesion->characterSheet();
    if (hoja != nullptr && f.nombres != nullptr) {
        const std::pair<const char*, const std::string*> historia[] = {
            {"Vínculo", &hoja->bondId}, {"Miedo", &hoja->fearId}, {"Defecto", &hoja->flawId},
            {"Meta", &hoja->goalId}, {"Ideal", &hoja->idealId}, {"Personalidad", &hoja->personalityId},
            {"Virtud", &hoja->virtueId}};
        for (const auto& [clave, id] : historia)
            if (!id->empty()) fp.historia.push_back({clave, f.nombres->nombre(*id)});
    }
    return pg;
}

PaginaCuaderno misiones(const FuentesCuaderno& f) {
    if (f.diario == nullptr || f.diario->misiones.empty()) {
        if (f.pantalla) f.pantalla->setExternalRowCount(0);
        return paginaVacia("MISIONES", "mision_activa", "No sigues ninguna misión.",
                           "Habla con la gente de la ciudad: los encargos que aceptes se anotarán aquí.");
    }
    // AV14-22: misiones, sus apartados y sus decisiones (HUD/Pergamino/PaginaMisiones).
    f.pantalla->setExternalRowCount(filasDelDiario(*f.diario).size());
    return paginaMisiones(*f.diario, f.pantalla->selectedInFocus());
}

PaginaCuaderno codice(const FuentesCuaderno& f) {
    if (f.codice == nullptr || f.codice->entradas.empty()) {
        if (f.pantalla) f.pantalla->setExternalRowCount(0);
        return paginaVacia("CÓDICE", "lore", "Todavía no has descubierto nada.",
                           "Habla con la gente y visita lugares: lo que aprendas quedará anotado aquí.");
    }
    PaginaCuaderno pg;
    pg.titulo = "CÓDICE";
    pg.cabeceraIzq = "DESCUBIERTO";
    pg.elegir = "Entrada";
    for (const Game::CodiceEntrada& e : f.codice->entradas) {
        FilaPagina fila;
        fila.escalaIcono = 1;
        fila.icono = e.tipo;
        fila.nombre = e.titulo;
        fila.valor = singularCodice(e.tipo);
        fila.valorPixel = false;
        pg.filas.push_back(fila);
    }
    f.pantalla->setExternalRowCount(pg.filas.size());
    pg.foco = static_cast<int>(std::min(f.pantalla->selectedInFocus(), pg.filas.size() - 1));
    const Game::CodiceEntrada& e = f.codice->entradas[static_cast<std::size_t>(pg.foco)];
    DetallePagina& d = pg.detalle;
    d.icono = e.tipo;
    d.titulo = e.titulo;
    std::string insignia = singularCodice(e.tipo);
    std::transform(insignia.begin(), insignia.end(), insignia.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    d.insignia = insignia;
    if (!e.texto.empty() && e.texto != e.titulo) d.parrafos.push_back(e.texto);
    if (!e.origen.empty()) d.nota = "Visto en: " + e.origen + ".";
    return pg;
}

}  // namespace

const char* iconoCategoria(Game::ItemCategory c) {
    switch (c) {
        case Game::ItemCategory::Consumable: return "consumible";
        case Game::ItemCategory::Equipment: return "equipo";
        case Game::ItemCategory::Quest: return "objeto_mision";
        case Game::ItemCategory::Other: break;
    }
    return "";
}

const char* insigniaCategoria(Game::ItemCategory c) {
    switch (c) {
        case Game::ItemCategory::Consumable: return "CONSUMIBLE";
        case Game::ItemCategory::Equipment: return "EQUIPO";
        case Game::ItemCategory::Quest: return "OBJETO CLAVE";
        case Game::ItemCategory::Other: break;
    }
    return "OTRO";
}

Result<bool> NombresHistoria::carga(const std::string& ruta) {
    std::ifstream entrada(ruta, std::ios::binary);
    if (!entrada) return Result<bool>::Error("NombresHistoria: no puedo abrir " + ruta);
    std::stringstream texto;
    texto << entrada.rdbuf();
    auto leido = JsonValue::parse(texto.str());
    if (!leido.isOk()) return Result<bool>::Error("NombresHistoria: " + leido.errorMessage());
    const JsonValue& entradas = leido.value()["entries"];
    for (std::size_t i = 0; i < entradas.size(); ++i) {
        const JsonValue& cc = entradas[i]["characterCreation"];
        if (!cc.isObject()) continue;
        for (const char* lista : {"bonds", "fears", "flaws", "goals", "ideals", "personalities", "virtues"}) {
            const JsonValue& opciones = cc[lista];
            for (std::size_t k = 0; k < opciones.size(); ++k) {
                const std::string id = opciones[k]["id"].asString();
                const std::string nombre = opciones[k]["name"].asString(opciones[k]["title"].asString());
                if (!id.empty() && !nombre.empty()) m_nombres[id] = nombre;
            }
        }
    }
    return Result<bool>::Ok(true);
}

std::string NombresHistoria::nombre(const std::string& id) const {
    const auto it = m_nombres.find(id);
    return it != m_nombres.end() ? it->second : Game::FichaVista::nombreLegible(id);
}

std::string textoMoral(int valor) {
    // MORAL-01: los siete tramos, con su nombre de Morality.
    return std::string(moral::nombreTramo(Game::tramoMoral(valor))) + " (" + std::to_string(valor) + ")";
}

void anotaCicatrices(std::vector<DatoPagina>& estado, int cuantas) {
    if (cuantas > 0) estado.push_back({"Cicatrices", std::to_string(cuantas)});
}

CabeceraCuaderno cabeceraCuaderno(const FuentesCuaderno& f) {
    CabeceraCuaderno c;
    const Game::FichaDatos ficha = fichaDe(f);
    c.nombre = ficha.vacia || ficha.nombre.empty() ? "Aventurero" : ficha.nombre;
    c.tier = ficha.vacia ? 1 : ficha.tier;
    c.subtitulo = ficha.vacia ? "" : unir({ficha.raza, ficha.clase}, " · ");
    if (f.pantalla != nullptr) {
        c.pv = f.pantalla->health();
        c.pvMax = f.pantalla->maxHealth();
        c.oro = f.pantalla->gold();
    } else if (f.sesion != nullptr) {
        // Sin pantalla del cuaderno (arranque con --nivel): lo mismo, de la sesion.
        if (const ICombatant* cuerpo = f.sesion->playerBody()) {
            c.pv = cuerpo->health();
            c.pvMax = cuerpo->maxHealth();
        }
        c.oro = f.sesion->gold();
        if (const RPG::CharacterSheet* hoja = f.sesion->characterSheet()) {
            if (!hoja->displayName.empty()) c.nombre = hoja->displayName;
            c.tier = hoja->tier;
        }
    }
    return c;
}

PaginaCuaderno paginaActual(const FuentesCuaderno& f) {
    if (f.sesion == nullptr || f.pantalla == nullptr) return PaginaCuaderno{};
    switch (f.pantalla->focus()) {
        case Game::InventoryFocus::Inventory: return inventario(f);
        case Game::InventoryFocus::Equipment: return equipo(f);
        case Game::InventoryFocus::Status: return fichaPagina(f);
        case Game::InventoryFocus::Actions:
            return paginaVacia("ACCIONES", "dote", "No tienes acciones disponibles fuera de combate.",
                               "Cuando puedas usar una habilidad explorando, aparecerá aquí con su efecto y su coste.");
        case Game::InventoryFocus::Quests: return misiones(f);
        case Game::InventoryFocus::Codex: return codice(f);
        case Game::InventoryFocus::COUNT: break;
    }
    return PaginaCuaderno{};
}

}  // namespace Pergamino
