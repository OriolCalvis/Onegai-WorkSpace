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
    require(b->adventures.size() == 4);      // prologo + tres dias
    require(b->epoch == "1981 b.f.");        // el mundo esta fechado

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

    std::printf("\n  %d de %d proyectos completos\n", completos, (int)idx.size());
    std::printf("\ntodas las comprobaciones han pasado\n");
    return 0;
}
