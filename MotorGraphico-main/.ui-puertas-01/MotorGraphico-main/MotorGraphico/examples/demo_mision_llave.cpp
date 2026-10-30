// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-09  (fase E, 3 j) - La missió de la clau, la porta i el cofre
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// E-09 — La misión de la llave, la puerta y el cofre.
//
// QUE ES ESTO. La prueba de aceptación de toda la fase E: la misión
// canónica de cualquier creador de juegos —un cofre con una llave, una
// puerta cerrada que la pide, un PNJ que habla de ello y un final— montada
// con las MISMAS llamadas que hará el editor, guardada, reabierta y
// recorrida de principio a fin sin que nadie toque el código.
//
// POR QUE EXISTE. Hasta aquí, cada eslabón de la cadena tiene su prueba:
// E-01 guarda el evento dentro del mapa, E-02 el estado, E-03 lo ejecuta,
// E-04 dice qué significa cada comando, E-07 lo valida y E-15 lo enseña.
// Todas pasan. Ninguna demuestra que con eso se pueda hacer UNA MISION.
// Esto lo demuestra o no lo demuestra.
//
// LO QUE COMPRUEBA, EN ORDEN:
//   1. La misión se construye con la API pública de MapDocument.
//   2. E-07 no le encuentra ni un error.
//   3. Se GUARDA y se REABRE antes de jugarla, que es lo que pide el
//      criterio con esas palabras. Lo que se recorre es el documento
//      reabierto, no el que está en memoria: si el serializador perdiera
//      una condición, la misión se rompería aquí y no en las manos de
//      alguien tres meses después.
//   4. El fichero publicado en assets/misiones/ dice lo mismo que el
//      código que lo generó. Si alguien cambia el formato y no regenera
//      el asset, esto lo dice.
//   5. El recorrido entero, determinista: la puerta antes de la llave no
//      se abre, el cofre da UNA llave aunque se insista, la puerta se
//      abre con ella, el PNJ cambia de frase en cada tramo y el altar
//      solo termina la misión al final.
//
// Regenerar el asset:  ./demo_mision_llave --escribir assets/misiones/mision_llave.json

#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "Core/Check.h"
#include "Level/ObjectCatalog.h"
#include "Map/MapDocument.h"
#include "Project/ProjectSerializer.h"
#include "RPG/AdventureValidation.h"
#include "RPG/EventCommand.h"
#include "RPG/EventInterpreter.h"
#include "RPG/SessionCommandHost.h"
#include "Render/ICombatant.h"

namespace {

using namespace RPG;

const char* kRutaAsset = "assets/misiones/mision_llave.json";

// --- Construcción de la misión ---------------------------------------

ids::EventId evt(const std::string& v) { return ids::EventId::parse(v).value(); }
ids::LayerId lyr(const std::string& v) { return ids::LayerId::parse(v).value(); }

MapEventCommand cmd(const std::string& tipo, std::map<std::string, std::string> params = {}) {
    MapEventCommand c;
    c.type = tipo;
    c.params = std::move(params);
    return c;
}

MapEventCondition bandera(const std::string& nombre, bool negada = false) {
    MapEventCondition c;
    c.kind = EventConditionKind::Flag;
    c.reference = nombre;
    c.negated = negada;
    return c;
}

MapEventPage pagina(const std::string& id, std::vector<MapEventCondition> condiciones,
                    std::vector<MapEventCommand> comandos) {
    MapEventPage p;
    p.id = evt(id);
    p.name = id;
    p.trigger = EventTrigger::Interact;
    p.conditions = std::move(condiciones);
    p.commands = std::move(comandos);
    return p;
}

MapEventDocument evento(const std::string& id, const std::string& nombre, GridCoord donde,
                        std::vector<MapEventPage> paginas) {
    MapEventDocument e;
    e.id = evt(id);
    e.name = nombre;
    e.position = donde;
    e.pages = std::move(paginas);
    return e;
}

// Una rama de un condicional. Las ramas son hijos del comando con sus
// propios hijos dentro (E-01); el tipo las nombra para que el editor las
// pueda enseñar.
MapEventCommand rama(const std::string& tipo, std::vector<MapEventCommand> dentro) {
    MapEventCommand r;
    r.type = tipo;
    r.children = std::move(dentro);
    return r;
}

MapDocument construirMision() {
    MapDocument mapa(ids::MapId::parse("mision_llave").value(), "La llave del santuario",
                     16, 12, 64, 32);
    MapLayerMeta capa;
    capa.id = lyr("eventos");
    capa.name = "Eventos";
    require(mapa.addLayer(MapLayer::events(capa, EventLayerDocument())).isOk());

    // --- El cofre. Dos páginas: sin abrir y ya vacío. Es el mecanismo
    // entero de DEC-10 en su forma más pequeña.
    require(mapa.addEvent(lyr("eventos"), evento("cofre", "Cofre del rincón", GridCoord{3, 4}, {
        pagina("cofre_cerrado", {},
               {cmd("texto.mostrar", {{"texto", "El cofre cede. Dentro hay una llave oxidada."},
                                      {"hablante", ""}}),
                cmd("inventario.dar", {{"objeto", "llave_oxidada"}, {"cantidad", "1"}}),
                cmd("bandera.encender", {{"bandera", "cofre_abierto"}})}),
        pagina("cofre_vacio", {bandera("cofre_abierto")},
               {cmd("texto.mostrar", {{"texto", "El cofre está vacío."}})}),
    })).isOk());

    // --- La puerta. Una sola página que RAMIFICA por el inventario, más
    // una segunda para cuando ya está abierta. Las dos maneras de hacer
    // lo mismo conviven a propósito: el autor va a usar las dos.
    MapEventCommand miraLlave = cmd("control.si", {{"clase", "item"},
                                                   {"referencia", "llave_oxidada"}});
    miraLlave.children = {
        rama("rama.si", {cmd("texto.mostrar", {{"texto", "La llave gira con un chirrido."}}),
                         cmd("bandera.encender", {{"bandera", "puerta_abierta"}})}),
        rama("rama.no", {cmd("texto.mostrar",
                             {{"texto", "Está cerrada. Hace falta una llave."}})}),
    };
    require(mapa.addEvent(lyr("eventos"), evento("puerta", "Puerta del santuario",
                                                 GridCoord{9, 4}, {
        pagina("puerta_cerrada", {}, {miraLlave}),
        pagina("puerta_ya_abierta", {bandera("puerta_abierta")},
               {cmd("texto.mostrar", {{"texto", "La puerta ya está abierta."}})}),
    })).isOk());

    // --- El PNJ. Tres páginas: una por tramo de la misión. Es lo que
    // hace que el mundo conteste a lo que el jugador ha hecho, y la razón
    // de que la regla de selección tenga que ser inequívoca.
    require(mapa.addEvent(lyr("eventos"), evento("anciano", "Anciano del portal",
                                                 GridCoord{2, 2}, {
        pagina("anciano_inicio", {},
               {cmd("texto.mostrar", {{"texto", "Dicen que el cofre del rincón guarda la llave."},
                                      {"hablante", "Anciano"}})}),
        pagina("anciano_con_llave", {bandera("cofre_abierto")},
               {cmd("texto.mostrar", {{"texto", "¿Ya la tienes? La puerta del fondo te espera."},
                                      {"hablante", "Anciano"}})}),
        pagina("anciano_final", {bandera("puerta_abierta")},
               {cmd("texto.mostrar", {{"texto", "Has abierto el santuario. Nadie lo había logrado."},
                                      {"hablante", "Anciano"}})}),
    })).isOk());

    // --- El altar. El final. Su primera página existe para que el
    // jugador que llega antes de tiempo reciba una respuesta y no el
    // silencio de un evento sin página activa.
    require(mapa.addEvent(lyr("eventos"), evento("altar", "Altar del santuario",
                                                 GridCoord{13, 4}, {
        pagina("altar_lejos", {},
               {cmd("texto.mostrar", {{"texto", "La puerta cerrada te separa del altar."}})}),
        pagina("altar_abierto", {bandera("puerta_abierta")},
               {cmd("texto.mostrar", {{"texto", "Dejas la llave en el altar. La misión termina."}}),
                cmd("bandera.encender", {{"bandera", "mision_completada"}})}),
    })).isOk());

    return mapa;
}

// --- El recorrido -----------------------------------------------------

class Cuerpo : public ICombatant {
public:
    void takeDamage(int) override {}
    void heal(int) override {}
    int health() const override { return 10; }
    int maxHealth() const override { return 10; }
    bool isAlive() const override { return true; }
    int stat(RPG::Stat) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "jugador"; }
};

LevelDefinition nivelVacio() {
    LevelDefinition l;
    l.name = "Santuario";
    l.playerStart = GridCoord{0, 0};
    return l;
}

// Un jugador de mentira con lo justo para que las comandas de partida
// (dar objeto) tengan dónde escribir.
struct Partida {
    ObjectCatalog catalogo;
    SkillCatalog habilidades;
    Cuerpo cuerpo;
    SkillSet skills{10};
    std::vector<std::string> inventario;
    GameSession sesion{nullptr, &catalogo, &habilidades, nivelVacio(), &cuerpo, &skills,
                       &inventario};
};

const MapEventDocument* buscar(const MapDocument& mapa, const std::string& id) {
    for (const MapLayer& capa : mapa.layers()) {
        const EventLayerDocument* eventos = std::get_if<EventLayerDocument>(&capa.data());
        if (eventos == nullptr) continue;
        for (const MapEventDocument& e : eventos->events()) {
            if (e.id.value() == id) return &e;
        }
    }
    return nullptr;
}

// Interactuar con un evento, como hará el jugador al pulsar la tecla de
// acción mirando a su casilla: se elige página, se arranca y se deja
// terminar. Devuelve el índice de la página que se ejecutó.
std::size_t interactuar(const MapDocument& mapa, const std::string& id, NarrativeState& estado,
                        const std::vector<std::string>& inventario, EventInterpreter& interprete) {
    const MapEventDocument* e = buscar(mapa, id);
    require(e != nullptr);

    const ActivePageResult activa = ActivePageWithInventory(*e, estado, inventario);
    // Si alguna página no se pudo evaluar, la elección no es de fiar y
    // seguir sería jugar a ciegas.
    require(activa.trustworthy());
    require(activa.page != nullptr);

    require(interprete.start(*e, activa.index).isOk());
    // Presupuesto de ticks: la misión no tiene esperas, así que uno basta,
    // pero se dan varios para no depender de eso. Un tope explícito es lo
    // que impide que un fallo se convierta en un cuelgue.
    for (int i = 0; i < 32 && interprete.isRunning(id); ++i) {
        interprete.tick(16);
    }
    interprete.collectFinished();
    require(!interprete.isRunning(id));
    return activa.index;
}

std::string serializar(const MapDocument& mapa) {
    const Result<std::string> r = ProjectSerializer::serializeMap(mapa);
    require(r.isOk());
    return r.value();
}

// --- Las comprobaciones ----------------------------------------------

void laMisionEsValida(const MapDocument& mapa) {
    const ValidationReport informe = ValidateEvents(mapa);
    for (const Diagnostic& d : informe.diagnostics) {
        std::printf("  %s %s (%s)\n", d.isError() ? "ERROR" : "aviso", d.message.c_str(),
                    d.eventId.c_str());
    }
    // Errores: ninguno. Avisos: tampoco, y eso es más exigente de lo que
    // pide el criterio. Una misión de ejemplo con avisos enseña a
    // ignorarlos.
    require(informe.errorCount() == 0);
    require(informe.canPlaytest());
    require(informe.diagnostics.empty());
    std::printf("[E-09] validación estática: 0 errores, 0 avisos. OK\n");
}

MapDocument guardarYReabrir(const MapDocument& mapa) {
    const std::string texto = serializar(mapa);
    const Result<MapDocument> vuelta = ProjectSerializer::deserializeMap(texto);
    require(vuelta.isOk());
    // Ida y vuelta sin pérdidas: si el serializador olvidara una
    // condición o una rama, los dos textos no coincidirían.
    require(serializar(vuelta.value()) == texto);
    std::printf("[E-09] guardada y reabierta sin perder nada (%zu bytes). OK\n", texto.size());
    return vuelta.value();
}

void elAssetPublicadoDiceLoMismo(const MapDocument& mapa) {
    std::ifstream fh(kRutaAsset, std::ios::binary);
    if (!fh) {
        std::printf("[E-09] aviso: no está %s. Regenéralo con --escribir.\n", kRutaAsset);
        return;
    }
    std::ostringstream buf;
    buf << fh.rdbuf();
    const Result<MapDocument> guardada = ProjectSerializer::deserializeMap(buf.str());
    require(guardada.isOk());
    // Se comparan las SERIALIZACIONES y no los textos crudos: así un
    // cambio de sangrado no da un falso positivo, pero un cambio de
    // contenido sí.
    require(serializar(guardada.value()) == serializar(mapa));
    std::printf("[E-09] el asset publicado dice lo mismo que el código. OK\n");
}

void elRecorridoEntero(const MapDocument& mapa) {
    Partida p;
    NarrativeState estado;
    SessionCommandHost partida(p.sesion, p.inventario);
    NarrativeCommandHost host(estado, &partida);
    EventInterpreter interprete(host);

    std::size_t leidas = 0;
    auto ultimaFrase = [&host, &leidas]() {
        require(host.log().size() > leidas);
        leidas = host.log().size();
        return host.log().back();
    };

    // 1. El anciano, antes de nada: página 0.
    require(interactuar(mapa, "anciano", estado, p.inventario, interprete) == 0);
    require(ultimaFrase().find("guarda la llave") != std::string::npos);

    // 2. La puerta ANTES de la llave. No se abre, y lo dice.
    require(interactuar(mapa, "puerta", estado, p.inventario, interprete) == 0);
    require(ultimaFrase().find("Hace falta una llave") != std::string::npos);
    require(!estado.hasFlag("puerta_abierta"));

    // 3. El cofre. Da UNA llave.
    require(interactuar(mapa, "cofre", estado, p.inventario, interprete) == 0);
    require(p.inventario.size() == 1);
    require(p.inventario[0] == "llave_oxidada");
    require(estado.hasFlag("cofre_abierto"));

    // 4. Insistir en el cofre no da una segunda llave. Es la propiedad
    //    por la que existen las páginas: ahora gana la última válida.
    require(interactuar(mapa, "cofre", estado, p.inventario, interprete) == 1);
    require(ultimaFrase().find("vacío") != std::string::npos);
    require(p.inventario.size() == 1);

    // 5. El anciano ya dice otra cosa. El mundo contesta.
    require(interactuar(mapa, "anciano", estado, p.inventario, interprete) == 1);
    require(ultimaFrase().find("La puerta del fondo") != std::string::npos);

    // 6. El altar todavía no: la primera página responde y NO termina
    //    la misión.
    require(interactuar(mapa, "altar", estado, p.inventario, interprete) == 0);
    require(!estado.hasFlag("mision_completada"));

    // 7. La puerta, ahora con la llave. Se abre por la rama del SÍ.
    require(interactuar(mapa, "puerta", estado, p.inventario, interprete) == 0);
    require(ultimaFrase().find("chirrido") != std::string::npos);
    require(estado.hasFlag("puerta_abierta"));

    // 8. Y a partir de aquí la puerta tiene otra página.
    require(interactuar(mapa, "puerta", estado, p.inventario, interprete) == 1);
    require(ultimaFrase().find("ya está abierta") != std::string::npos);

    // 9. El anciano, tercer tramo.
    require(interactuar(mapa, "anciano", estado, p.inventario, interprete) == 2);
    require(ultimaFrase().find("Has abierto el santuario") != std::string::npos);

    // 10. El altar cierra la misión.
    require(interactuar(mapa, "altar", estado, p.inventario, interprete) == 1);
    require(estado.hasFlag("mision_completada"));

    // La llave no se ha gastado: abrir no es consumir, y nadie ha escrito
    // que lo sea.
    require(p.inventario.size() == 1);
    std::printf("[E-09] recorrido completo en %zu interacciones, %zu frases. OK\n",
                static_cast<std::size_t>(10), host.log().size());
}

void esDeterminista(const MapDocument& mapa) {
    // Dos recorridos idénticos tienen que dar el MISMO log. Un demo de
    // aceptación que a veces pase no sirve de puerta para nada.
    auto recorrer = [&mapa]() {
        Partida p;
        NarrativeState estado;
        SessionCommandHost partida(p.sesion, p.inventario);
        NarrativeCommandHost host(estado, &partida);
        EventInterpreter interprete(host);
        for (const char* id : {"anciano", "puerta", "cofre", "cofre", "anciano", "altar",
                               "puerta", "puerta", "anciano", "altar"}) {
            interactuar(mapa, id, estado, p.inventario, interprete);
        }
        std::string todo;
        for (const std::string& linea : host.log()) {
            todo += linea;
            todo += '\n';
        }
        return todo;
    };
    const std::string uno = recorrer();
    const std::string dos = recorrer();
    require(uno == dos);
    require(!uno.empty());
    std::printf("[E-09] dos recorridos dan exactamente el mismo texto. OK\n");
}

}  // namespace

int main(int argc, char** argv) {
    const MapDocument construida = construirMision();

    if (argc >= 3 && std::strcmp(argv[1], "--escribir") == 0) {
        std::ofstream salida(argv[2], std::ios::binary);
        if (!salida) {
            std::printf("no se puede escribir %s\n", argv[2]);
            return 1;
        }
        salida << serializar(construida);
        std::printf("escrito %s\n", argv[2]);
        return 0;
    }

    laMisionEsValida(construida);
    const MapDocument reabierta = guardarYReabrir(construida);
    elAssetPublicadoDiceLoMismo(construida);

    // A partir de aquí se juega la REABIERTA, no la de memoria: es lo que
    // pide el criterio («es desa i reobre abans de provar-la»).
    elRecorridoEntero(reabierta);
    esDeterminista(reabierta);

    std::printf("\n[E-09] La misión de la llave, la puerta y el cofre: completa.\n");
    return 0;
}
