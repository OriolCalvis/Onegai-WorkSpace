// demo_proyectos — los proyectos vivos del motor, sin abrir una ventana.
//
// Es el nucleo de la pantalla de arranque del editor: descubrir que packs
// de contenido hay, si estan completos, y poder crear uno nuevo. Que sea
// GL-free no es casualidad -- mismo criterio que EditorState: el editor
// puede migrar de UI algun dia sin tocar nada de esto, y mientras tanto
// se prueba de verdad en CI.
//
// GL-free: no abre ventana. Ejecutar desde build/ o desde MotorGraphico/.

#include <cstdio>
#include <string>

#include "Check.h"
#include "Editor/ProjectHub.h"
#include "Editor/ProjectIndex.h"

using Editor::Project;
using Editor::ProjectCheck;
using Editor::ProjectIndex;

int main() {
    std::printf("=== demo_proyectos: los proyectos vivos ===\n\n");

    auto r = ProjectIndex::scan("assets");
    require(r);
    const ProjectIndex idx = r.value();
    require(idx.size() >= 2);

    std::printf("  %-13s %-34s %5s %5s %4s %4s  %s\n",
                "id", "nombre", "niv", "map", "av", "cat", "autor");
    std::printf("  %s\n", std::string(96, '-').c_str());
    for (const Project& p : idx.projects()) {
        std::printf("  %-13s %-34s %5d %5d %4d %4d  %s%s\n",
                    p.id.c_str(), p.name.substr(0, 34).c_str(),
                    (int)p.levels.size(), (int)p.maps.size(),
                    (int)p.adventures.size(), (int)p.catalogs.size(),
                    p.author.c_str(), p.playable() ? "" : "  (sin entrada)");
    }

    // --- Boundington es el pack de referencia: tiene que estar entero ---
    const Project* b = idx.find("boundington");
    require(b != nullptr);
    require(b->playable());
    require(b->epoch == "1981 b.f.");        // el mundo esta fechado

    // La campana tiene que estar entera. Se comprueba POR NOMBRE y no por
    // cuenta: la version anterior exigia "== 4" y salto sola en cuanto
    // Oriol anadio boundington_precuela_taberna.json. Un proyecto vivo
    // gana contenido; una prueba que se rompe porque el trabajo avanza no
    // esta comprobando nada util, solo estorbando.
    const char* kCampana[] = {"boundington_prologo.json", "boundington_primer_dia.json",
                              "boundington_segundo_dia.json", "boundington_ocaso.json"};
    for (const char* av : kCampana) {
        bool esta = false;
        for (const std::string& s : b->adventures) {
            if (s == av) {
                esta = true;
            }
        }
        require(esta);
    }
    require(b->adventures.size() >= 4);

    std::printf("\n[revision de cada proyecto]\n");
    int completos = 0;
    for (const Project& p : idx.projects()) {
        const ProjectCheck c = idx.check(p.id);
        std::printf("  %-13s %s", p.id.c_str(), c.ok() ? "completo" : "");
        if (c.ok()) {
            ++completos;
            std::printf("  (%d niveles)\n", c.levelsChecked);
        } else {
            std::printf("%d problema(s):\n", (int)c.problems.size());
            for (const std::string& s : c.problems) {
                std::printf("      %s\n", s.c_str());
            }
        }
    }
    // Boundington SI tiene que estar completo; los cuadrantes del
    // experimento pueden estar a medias, que para eso es un experimento.
    require(idx.check("boundington").ok());

    if (!idx.orphans().empty()) {
        std::printf("\n[huerfanos] %d\n", (int)idx.orphans().size());
        for (const std::string& o : idx.orphans()) {
            std::printf("    %s\n", o.c_str());
        }
    }

    // --- Crear uno nuevo, y que las reglas se cumplan ---
    std::printf("\n[crear proyecto nuevo]\n");
    // El id con mayusculas o espacios se rechaza: acaba siendo nombre de
    // fichero y prefijo de ids.
    require(!ProjectIndex::create("assets", "Con Mayusculas", "x", "cm_", "test", "0").isOk());
    // Un prefijo sin '_' final pega el prefijo al id y produce cosas como
    // "cmtabernero", que ni se lee ni se puede buscar.
    require(!ProjectIndex::create("assets", "prueba_tmp", "x", "pt", "test", "0").isOk());
    // Y no se puede pisar un proyecto existente.
    require(!ProjectIndex::create("assets", "boundington", "x", "b_", "test", "0").isOk());
    std::printf("    id con mayusculas, prefijo sin '_' y duplicado: rechazados. OK\n");

    auto nuevo = ProjectIndex::create("assets", "prueba_tmp", "Proyecto de prueba",
                                      "pt_", "demo_proyectos", "2000 b.f.");
    require(nuevo);
    std::printf("    creado '%s' con prefijo '%s'\n",
                nuevo.value().id.c_str(), nuevo.value().prefix.c_str());

    // Y aparece al volver a escanear: si no se anadiera al indice, el
    // proyecto existiria en disco y no lo veria nadie.
    auto r2 = ProjectIndex::scan("assets");
    require(r2);
    require(r2.value().find("prueba_tmp") != nullptr);
    require(r2.value().size() == idx.size() + 1);
    std::printf("    reaparece al reescanear: %d -> %d proyectos. OK\n",
                (int)idx.size(), (int)r2.value().size());

    // Recien creado no es jugable, y el editor tiene que poder decirlo.
    const ProjectCheck cn = r2.value().check("prueba_tmp");
    require(!cn.ok());
    require(!r2.value().find("prueba_tmp")->playable());
    std::printf("    recien creado no es jugable (%s). OK\n", cn.problems[0].c_str());

    // --- Y se recoge: la demo no deja basura en assets ---
    // La primera version dejaba 'prueba_tmp' puesto, y acabo commiteado.
    // Una prueba que ensucia el arbol de trabajo se paga cada vez.
    require(ProjectIndex::remove("assets", "prueba_tmp"));
    require(!ProjectIndex::remove("assets", "prueba_tmp").isOk());  // ya no esta
    auto r3 = ProjectIndex::scan("assets");
    require(r3);
    require(r3.value().find("prueba_tmp") == nullptr);
    require(r3.value().size() == idx.size());
    require(r3.value().find("boundington") != nullptr);   // no se llevo por delante a nadie
    std::printf("    borrado: vuelve a %d proyectos y Boundington sigue. OK\n",
                (int)r3.value().size());

    // =================================================================
    // La PANTALLA de arranque. Esto es lo que ve el editor, y se prueba
    // aqui justo porque lo que vive dentro de level_editor.cpp no se
    // puede comprobar sin GL: la primera version de esta pantalla se
    // entrego siendo un printf a stdout precisamente por eso.
    // =================================================================
    std::printf("\n[pantalla de arranque]\n");
    auto h = Editor::ProjectHub::load("assets");
    require(h);
    Editor::ProjectHub hub = h.value();
    require(hub.mode() == Editor::ProjectHub::Mode::List);
    require(hub.lines().size() == idx.size());
    for (const std::string& l : hub.lines()) {
        std::printf("    %s\n", l.c_str());
    }

    // Navegar: 's' baja, 'w' sube, y da la vuelta por los dos extremos.
    const std::size_t n = hub.size();
    require(hub.selected() == 0);
    hub.key('s');
    require(hub.selected() == 1);
    hub.key('w');
    require(hub.selected() == 0);
    hub.key('w');
    require(hub.selected() == n - 1);   // desde el primero al ultimo
    hub.key('s');
    require(hub.selected() == 0);       // y vuelta
    std::printf("    navegacion con vuelta en los dos extremos. OK\n");

    // --- 'entrada' se escribio de dos formas y las dos tienen que valer ---
    // boundington puso la ruta entera; este_norte (otro autor, otro dia)
    // puso solo el nombre del fichero. Nunca se documento cual era, asi
    // que las dos se resuelven y el proyecto anota a que resolvio. Esta
    // comprobacion existe para que no se rompa el trabajo de nadie el dia
    // que alguien decida que solo vale una forma.
    const Project* pb = idx.find("boundington");
    require(pb->entry.find('/') != std::string::npos);            // ruta entera
    require(pb->entryKind == Project::EntryKind::Adventure);
    const Project* pe = idx.find("este_norte");
    require(pe->entry.find('/') == std::string::npos);            // nombre suelto
    require(pe->entryKind != Project::EntryKind::Missing);        // y aun asi resuelve
    std::printf("    entrada '%s' (ruta) -> aventura\n", pb->entry.c_str());
    std::printf("    entrada '%s' (nombre suelto) -> %s\n", pe->entry.c_str(),
                pe->entryPath.c_str());

    // El detalle del marcado dice lo que hay que saber ANTES de abrirlo:
    // no lo que pone el manifiesto, sino a que resolvio de verdad.
    while (hub.current() != nullptr && hub.current()->id != "este_norte") {
        hub.key('s');
    }
    bool dicePorDondeArranca = false;
    for (const std::string& l : hub.detail()) {
        if (l.find("arranque:") != std::string::npos && l.find(pe->entryPath) != std::string::npos) {
            dicePorDondeArranca = true;
        }
    }
    require(dicePorDondeArranca);
    std::printf("    el detalle muestra la ruta resuelta, no la del manifiesto. OK\n");

    // Abrir: la pantalla NO abre nada, dice a quien hay que abrir.
    while (hub.current() != nullptr && hub.current()->id != "boundington") {
        hub.key('s');
    }
    require(hub.key('\n') == Editor::ProjectHub::Action::Open);
    require(hub.openId() == "boundington");
    require(hub.key(27) == Editor::ProjectHub::Action::Quit);
    std::printf("    ENTER devuelve Open('%s'), ESC devuelve Quit. OK\n", hub.openId().c_str());

    // Crear desde la pantalla: 'n', teclear, ENTER.
    require(hub.key('n') == Editor::ProjectHub::Action::None);
    require(hub.mode() == Editor::ProjectHub::Mode::NewProject);
    const char* tecleado = "Mi Proy 2!";   // mayusculas, espacios y '!' no entran
    for (const char* c = tecleado; *c; ++c) {
        hub.key(*c);
    }
    require(hub.draftId() == "iroy2");     // solo [a-z0-9_] sobrevive
    hub.key('\b');
    require(hub.draftId() == "iroy");
    std::printf("    el campo filtra a [a-z0-9_]: \"%s\" -> '%s'\n", tecleado, hub.draftId().c_str());
    hub.key('\n');
    require(hub.mode() == Editor::ProjectHub::Mode::Message);
    require(hub.lastOk());
    require(hub.current() != nullptr && hub.current()->id == "iroy");  // queda marcado
    std::printf("    creado y marcado: %s\n", hub.message()[0].c_str());

    // Cualquier tecla cierra el mensaje y vuelve a la lista.
    hub.key(' ');
    require(hub.mode() == Editor::ProjectHub::Mode::List);
    require(hub.size() == idx.size() + 1);

    // Y no se puede crear dos veces el mismo: el mensaje tiene que decirlo.
    hub.key('n');
    for (const char* c = "iroy"; *c; ++c) {
        hub.key(*c);
    }
    hub.key('\n');
    require(hub.mode() == Editor::ProjectHub::Mode::Message);
    require(!hub.lastOk());
    std::printf("    duplicado rechazado en pantalla: %s\n", hub.message()[0].c_str());
    hub.key(' ');

    // Compilar el marcado, de verdad: lanza tools/build_proyecto.py.
    while (hub.current() != nullptr && hub.current()->id != "boundington") {
        hub.key('s');
    }
    hub.key('b');
    require(hub.mode() == Editor::ProjectHub::Mode::Message);
    require(hub.lastOk());
    require(hub.message().size() > 1);   // la salida del script, no solo el titulo
    std::printf("    build desde la pantalla: %s\n", hub.message()[0].c_str());
    hub.key(' ');

    // Y se recoge tambien lo que creo la pantalla.
    require(ProjectIndex::remove("assets", "iroy"));
    hub.refresh();
    require(hub.size() == idx.size());

    std::printf("\n  %d de %d proyectos completos\n", completos, (int)idx.size());
    std::printf("\ntodas las comprobaciones han pasado\n");
    return 0;
}
