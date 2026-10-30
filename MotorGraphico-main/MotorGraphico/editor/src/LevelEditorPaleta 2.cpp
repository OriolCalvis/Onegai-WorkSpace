// EST-21 (relevo A) — Implementacion. Ver la cabecera para el criterio.

#include "LevelEditorPaleta.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>

#include "Core/Json/JsonValue.h"

namespace EditorNiveles {

const char* toolName(EditorTool tool) {
    switch (tool) {
        case EditorTool::PaintTile:
            return "PINTAR TILE";
        case EditorTool::EraseTile:
            return "BORRAR TILE";
        case EditorTool::PlaceObject:
            return "COLOCAR OBJETO";
        case EditorTool::RemoveObject:
            return "QUITAR OBJETO";
        case EditorTool::SetPlayerStart:
            return "INICIO JUGADOR";
        case EditorTool::FillTiles:
            return "RELLENAR ZONA";
        case EditorTool::LinkLevel:
            return "ENLAZAR NIVEL";
    }
    return "?";
}

// Las rutas son necesarias para el motor, pero no son un lenguaje de
// autoría. La UI siempre muestra el nombre que una persona reconoce:
// "interior_taberna.json" pasa a ser "Interior taberna".
std::string nombreEscenario(const std::string& path) {
    const std::size_t slash = path.find_last_of('/');
    std::string name = path.substr(slash == std::string::npos ? 0 : slash + 1);
    const std::size_t extension = name.rfind(".json");
    if (extension != std::string::npos) {
        name.erase(extension);
    }
    std::replace(name.begin(), name.end(), '_', ' ');
    return name;
}

std::string textoDeArchivo(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return {};
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

TilePaletteInfo semanticasTile(int gid, const std::string& name, bool collision) {
    static const char* fallbackNames[] = {
        "Baldosa clara", "Baldosa oscura", "Piedra", "Hierba", "Tierra", "Arena"};
    const std::string fallback = gid > 0 && gid <= static_cast<int>(std::size(fallbackNames))
                                     ? fallbackNames[gid - 1]
                                     : "Material sin nombre " + std::to_string(gid);
    TilePaletteInfo info{fallback, "CONSTRUCCION", "NEUTRO", "", "OTROS", collision};
    info.name = name.empty() ? fallback : name;
    const std::string n = name;
    auto has = [&](const char* token) { return n.find(token) != std::string::npos; };
    if (has("nieve") || has("hielo") || has("escarcha")) {
        info.affinity = "FRIO / NIEVE";
        info.effect = "FRIO";
        info.requirement = "ropa termica o bufo de frio";
    } else if (has("arena") || has("desierto") || has("arenisca") || has("canon")) {
        info.affinity = "ARIDO / DESIERTO";
        info.effect = "SED + CALOR";
        info.requirement = "agua, ropa ligera o bufo de calor";
    } else if (has("agua") || has("rio") || has("estanque")) {
        info.affinity = "AGUA / COSTA";
        info.effect = "HUMEDAD";
        info.requirement = "botas impermeables o equipo acuatico";
    } else if (has("ceniza") || has("volcan") || has("lava")) {
        info.affinity = "VOLCANICO / FUEGO";
        info.effect = "CALOR + DANIO";
        info.requirement = "resistencia al fuego o bufo ignifugo";
    } else if (has("barro") || has("pantano") || has("turba") || has("alcantarilla")) {
        info.affinity = "PANTANO / HUMEDAL";
        info.effect = "LENTO + HUMEDAD";
        info.requirement = "botas de pantano o bufo de movilidad";
    } else if (has("mina") || has("piedra") || has("roca") || has("adoquin")) {
        info.affinity = "ROCA / MINA";
        info.effect = "TERRENO DURO";
        info.requirement = "botas resistentes (opcional)";
    } else if (has("bosque") || has("hierba") || has("cesped") || has("musgo") ||
               has("hojarasca") || has("pradera")) {
        info.affinity = "BOSQUE / PRADERA";
        info.effect = "NATURAL";
        info.requirement = "ninguno";
    } else if (has("suelo_") || has("mosaico") || has("marmol") || has("pavimento") ||
               has("plaza") || has("calle") || has("vivienda") || has("taberna")) {
        info.affinity = "INTERIOR / URBANO";
        info.effect = "NATURAL";
        info.requirement = "ninguno";
    }
    return info;
}

std::unordered_map<int, TilePaletteInfo> cargarSemanticaTiles(const std::string& tilesetPath) {
    std::unordered_map<int, TilePaletteInfo> result;
    const std::string indexPath = tilesetPath.find("terreno_iso.png") != std::string::npos
                                      ? "assets/textures/terreno_iso.json"
                                      : "";
    if (!indexPath.empty()) {
        const auto parsed = JsonValue::parse(textoDeArchivo(indexPath));
        if (parsed.isOk()) {
            const JsonValue& tiles = parsed.value()["tiles"];
            for (std::size_t i = 0; i < tiles.size(); ++i) {
                const JsonValue& tile = tiles[i];
                const int gid = tile["gid"].asInt(0);
                if (gid > 0) {
                    result[gid] = semanticasTile(gid, tile["nombre"].asString(),
                                                  tile["colisiona"].asBool(false));
                }
            }
        }
    }
    const auto labels = JsonValue::parse(textoDeArchivo("assets/textures/terreno_iso_editor.json"));
    if (labels.isOk()) {
        const JsonValue& tiles = labels.value()["tiles"];
        for (const auto& entry : tiles.objectValues()) {
            const int gid = std::atoi(entry.first.c_str());
            auto found = result.find(gid);
            if (gid <= 0 || found == result.end()) continue;
            found->second.name = entry.second["nombre"].asString(found->second.name);
            found->second.tag = entry.second["etiqueta"].asString(found->second.tag);
        }
    }
    return result;
}

bool contieneCualquiera(const std::string& value,
                        std::initializer_list<const char*> tokens) {
    for (const char* token : tokens) {
        if (value.find(token) != std::string::npos) return true;
    }
    return false;
}

ObjectFilter filtroAfinidadObjeto(const std::string& group) {
    return [group](const ObjectDefinition& def) {
        const std::string& id = def.id;
        const std::string identity = def.id + "|" + def.raceId + "|" + def.name;
        if (group == "PNJ · HUMANOS") return def.category == ObjectCategory::Npc && contieneCualquiera(identity, {"human", "humano", "marches", "noble", "mercader"});
        if (group == "PNJ · ELFOS / DROWS") return def.category == ObjectCategory::Npc && contieneCualquiera(identity, {"elf", "elfo", "drow"});
        if (group == "PNJ · ENANOS / GNOMOS") return def.category == ObjectCategory::Npc && contieneCualquiera(identity, {"enan", "dwarf", "gnom"});
        if (group == "PNJ · ORCOS / GOBLINS") return def.category == ObjectCategory::Npc && contieneCualquiera(identity, {"orc", "orco", "goblin"});
        if (group == "PNJ · LAGARTOS / NAGAS") return def.category == ObjectCategory::Npc && contieneCualquiera(identity, {"lagarto", "lizard", "naga", "saur"});
        if (group == "PNJ · OTRAS RAZAS") return def.category == ObjectCategory::Npc;
        if (group == "RECOGIBLES · CONSUMIBLES") {
            return def.category == ObjectCategory::Pickup &&
                   !contieneCualquiera(id, {"arma", "espada", "arco", "baston", "escudo"});
        }
        if (group == "RECOGIBLES · EQUIPO") {
            return def.category == ObjectCategory::Pickup &&
                   contieneCualquiera(id, {"arma", "espada", "arco", "baston", "escudo", "botas", "tunica", "guante"});
        }
        if (group == "MONSTRUOS · ANIMALES") return def.category == ObjectCategory::Enemy &&
            contieneCualquiera(id, {"lobo", "oso", "ciervo", "jabali", "zorro", "rata", "serpiente", "aran", "murcielago", "ave", "cuervo", "animal"});
        if (group == "MONSTRUOS · DRAGONES") return def.category == ObjectCategory::Enemy && contieneCualquiera(id, {"dragon", "dracon"});
        if (group == "MONSTRUOS · NO MUERTOS") return def.category == ObjectCategory::Enemy && contieneCualquiera(id, {"zombi", "esqueleto", "draug", "reven", "fantasma", "espectro", "no_muerto"});
        if (group == "MONSTRUOS · ELEMENTALES") return def.category == ObjectCategory::Enemy && contieneCualquiera(id, {"elemental", "fuego", "hielo", "rayo", "trueno", "ceniza", "salamandra"});
        if (group == "MONSTRUOS · BIPEDOS") return def.category == ObjectCategory::Enemy && contieneCualquiera(id, {"goblin", "orc", "orco", "bandido", "humanoide", "naga", "tiefling", "cultista"});
        if (group == "MONSTRUOS · SAGRADOS / LEGENDARIOS") return def.category == ObjectCategory::Enemy && contieneCualquiera(id, {"sagrado", "santa", "legend", "matriarca", "guardian", "deidad", "angel"});
        if (group == "MONSTRUOS · OTROS") return def.category == ObjectCategory::Enemy;
        if (group == "DECORADO · INTERIOR") return def.category == ObjectCategory::Prop && contieneCualquiera(id, {"mesa", "silla", "taburete", "cama", "ventana", "puerta", "cofre", "jarr", "cerveza", "taberna", "posada", "casa", "estanter", "forja", "barril"});
        if (group == "DECORADO · BOSQUE") return def.category == ObjectCategory::Prop && contieneCualquiera(id, {"arbusto", "matorral", "arbol", "bosque", "hoja", "flor", "seto", "hierba", "helecho"});
        if (group == "DECORADO · MONTAÑA / MINA") return def.category == ObjectCategory::Prop && contieneCualquiera(id, {"mina", "mineral", "roca", "piedra", "cristal", "gema", "montana", "cueva", "adoquin"});
        if (group == "DECORADO · SELVA / PANTANO") return def.category == ObjectCategory::Prop && contieneCualquiera(id, {"selva", "pantano", "liana", "hongo", "musgo", "charco", "junco"});
        if (group == "DECORADO · DESIERTO / VOLCAN") return def.category == ObjectCategory::Prop && contieneCualquiera(id, {"desierto", "arena", "cactus", "volcan", "lava", "ceniza", "brasero", "antorcha"});
        if (group == "DECORADO · OTROS") return def.category == ObjectCategory::Prop;
        if (group == "EVENTOS / PUERTAS") return contieneCualquiera(id, {"puerta", "salida", "portal", "runa", "cofre", "guardado", "cartel", "evento", "transicion"});
        return true;
    };
}

const char* nombreCategoria(ObjectCategory category) {
    switch (category) {
        case ObjectCategory::Npc:    return "PNJ";
        case ObjectCategory::Enemy:  return "ENEMIGO";
        case ObjectCategory::Pickup: return "RECOGIBLE";
        case ObjectCategory::Prop:   return "OBJETO";
    }
    return "OBJETO";
}

std::string nombreDePaleta(const ObjectCatalog& catalog, const std::string& id) {
    const ObjectDefinition* definition = catalog.find(id);
    if (definition == nullptr) {
        return "SIN DEFINIR";
    }
    const std::string estado = definition->previewPng.empty() ? " · [PNG PENDIENTE]" : "";
    return std::string(nombreCategoria(definition->category)) + " · " + definition->name + estado;
}

// Posiciones 1..16 del atlas editor_object_icons.png. No son sprites de
// combate: son pictogramas de autoría, consistentes aunque un objeto aún no
// tenga arte final. Así una puerta, un PNJ o una llave se reconocen antes
// de leer una sola letra.
int iconoEditor(const ObjectDefinition& definition) {
    const std::string& id = definition.id;
    if (id.find("salida") != std::string::npos || id.find("puerta") != std::string::npos) return 1;
    if (id.find("puente") != std::string::npos) return 4;
    // Un prop natural no puede caer en el icono de casa (frame 3): el
    // icono de arbol/vegetacion (frame 12) es la representacion editorial
    // correcta para arbustos, plantas y arboles hasta que exista su PNG.
    if (id.find("arbusto") != std::string::npos || id.find("matorral") != std::string::npos ||
        id.find("planta") != std::string::npos || id.find("hierba") != std::string::npos ||
        id.find("arbol") != std::string::npos || id.find("bosque") != std::string::npos) {
        return 12;
    }
    if (definition.category == ObjectCategory::Npc) {
        if (id.find("luisarda") != std::string::npos) return 6;
        if (id.find("griffin") != std::string::npos) return 8;
        if (id.find("venides") != std::string::npos || id.find("guard") != std::string::npos) return 7;
        return 5;
    }
    if (definition.category == ObjectCategory::Enemy) {
        if (id.find("naga") != std::string::npos) return 10;
        if (id.find("perdido") != std::string::npos || id.find("mascara") != std::string::npos) return 11;
        if (id.find("draug") != std::string::npos || id.find("saga") != std::string::npos) return 12;
        return 9;
    }
    if (definition.category == ObjectCategory::Pickup) {
        if (id.find("llave") != std::string::npos) return 14;
        if (id.find("cofre") != std::string::npos) return 15;
        return 13;
    }
    if (id.find("antorcha") != std::string::npos) return 16;
    return 3;  // edificio/prop genérico
}

// Frame de la biblioteca visual de enemigos (4 columnas x 3 filas), o 0
// cuando el objeto usa el pictograma editorial general. Mantener este
// mapeo por ID evita depender de nombres traducidos: el autor puede
// renombrar un enemigo sin cambiar cómo se reconoce sobre el mapa.
int iconoBibliotecaEnemigos(const ObjectDefinition& definition) {
    if (definition.category != ObjectCategory::Enemy) return 0;
    const std::string& id = definition.id;
    if (id == "enemy_slime_verde") return 1;
    if (id == "enemy_slime_venenoso") return 2;
    if (id == "enemy_murcielago_cueva") return 3;
    if (id == "enemy_murcielago_gigante") return 4;
    if (id == "enemy_vampiro_noble") return 5;
    if (id == "enemy_vampiro_acorazado") return 6;
    if (id == "enemy_zombi_musgoso") return 7;
    if (id == "enemy_zombi_esqueletico") return 8;
    if (id == "enemy_dragon_rojo_joven") return 9;
    if (id == "enemy_dragon_negro_anciano") return 10;
    if (id == "enemy_arlequin_enmascarado") return 11;
    if (id == "enemy_maestro_arlequin") return 12;
    return 0;
}

RaceNpcPreview previewNpcRacial(const ObjectDefinition& definition) {
    if (definition.category != ObjectCategory::Npc) return {};
    const std::string& id = definition.raceId;
    static const std::unordered_map<std::string, RaceNpcPreview> previews{
        {"aarakocra_de_las_montanas", {1, 1}}, {"aarakocra_del_viento", {1, 2}},
        {"alquimistas_de_aegroum", {1, 3}}, {"cara_loca", {1, 4}},
        {"cazador_nocturno", {1, 5}}, {"diplomaticos_de_bastrea", {1, 6}},
        {"draconidos_de_fuego", {1, 7}}, {"draconidos_de_hielo", {1, 8}},
        {"drow_de_la_casa_noble", {1, 9}}, {"drow_explorador", {1, 10}},
        {"elfos_de_las_estrellas", {1, 11}}, {"elfos_del_bosque", {1, 12}},
        {"enanos_forjadores", {2, 1}}, {"enanos_historiadores", {2, 2}},
        {"espectros", {2, 3}}, {"guardianes_de_la_luz", {2, 4}},
        {"guardianes_del_elemento", {2, 5}}, {"hechiceros_oscuros", {2, 6}},
        {"hombres_lagarto_de_la_selva", {2, 7}}, {"hombres_lagarto_del_pantano", {2, 8}},
        {"medianos_del_bosque", {2, 9}}, {"medianos_urbanos", {2, 10}},
        {"mercenarios_de_ascaria", {2, 11}}, {"minotauros_de_la_llanura", {2, 12}},
        {"minotauros_del_laberinto", {3, 1}}, {"nagas_arcanos", {3, 2}},
        {"nagas_venenosos", {3, 3}}, {"nobles_de_ascaria", {3, 4}},
        {"orcos_del_consejo", {3, 5}}, {"orcos_dramaticos", {3, 6}},
        {"race_ascaria_fey_blood", {3, 7}}, {"race_dwarf_deepforge", {3, 8}},
        {"race_elf_canopy", {3, 9}}, {"race_human_marches", {3, 10}},
        {"race_orc_gongorguma", {3, 11}}, {"revenants", {3, 12}},
        {"sagas_de_la_locura", {4, 1}}, {"sagas_del_abismo", {4, 2}},
        {"tieflings_de_la_sombra", {4, 3}}, {"tieflings_del_vacio", {4, 4}},
        {"traidores_oscuros", {4, 5}}, {"yokai_de_las_sombras", {4, 6}},
        {"yokai_de_los_vientos", {4, 7}},
    };
    const auto found = previews.find(id);
    return found != previews.end() ? found->second : RaceNpcPreview{};
}

// TextureManager carga los PNG con Y invertida. Los previews se envían
// directamente al SpriteBatch (no pasan por Entity), por lo que necesitan
// invertir V aqui para que cabeza y pies no queden al revés.
UVRect uvPreviewPng(const TextureAtlas& atlas, int frame) {
    // Un pixel de seguridad evita que una pata, hoja o capa de la celda
    // superior entre en el preview por el borde de la region.
    UVRect uv = atlas.getUVInset(frame, 1.0f);
    std::swap(uv.v0, uv.v1);
    return uv;
}

// La barra de estado va en mayusculas (BitmapFont solo tiene caja alta).
std::string aMayus(const std::string& t) {
    std::string r = t;
    for (char& c : r) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return r;
}

}  // namespace EditorNiveles
