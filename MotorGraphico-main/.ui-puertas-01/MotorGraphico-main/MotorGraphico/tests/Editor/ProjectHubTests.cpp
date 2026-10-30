// H4-01 (Fase H) — La pantalla de proyectos: duplicar y borrar desde el hub.
//
// POR QUE ESTA PRUEBA EXISTE AQUI Y NO EN demo_proyectos. ProjectHub es la
// pantalla de arranque del editor y hasta hoy su unica prueba era
// examples/demo_proyectos.cpp, que CMakeLists.txt excluye de CTest a proposito:
// escribe sobre assets/ real —que varios agentes cambian a la vez— y lanza
// tools/build_proyecto.py, que no esta en el directorio de build. O sea: la
// pantalla no se probaba en ninguna ejecucion automatica, y el unico sitio
// donde se podia comprobar no se ejecutaba nunca. Es el mismo patron que dejo
// demo_mision_llave apagado en silencio durante dias (sesion H0-08, 29/09).
// FORMATO_PROYECTOS.md ademas dice que demo_proyectos recorre «crear... borrar»
// desde la pantalla, y no era verdad: borra llamando a ProjectIndex::remove
// directamente (examples/demo_proyectos.cpp:275), porque la pantalla no podia.
//
// Lo que se defiende aqui es la CAPA DE PANTALLA, no ProjectIndex: duplicate()
// y remove() ya tienen sus casos en tests/Project/ProjectNamingTests.cpp. Aqui
// se comprueba que las teclas llevan a la operacion, que una operacion
// destructiva pregunta antes, que el informe de la copia se ensena, y que el
// estado del hub (seleccion, lineas, modo) queda coherente despues. Todo contra
// un assetsRoot temporal, sin tocar assets/ y sin ejecutar el empaquetador.

#include "Editor/ProjectHub.h"
#include "Project/ProjectIndex.h"
#include "Project/ProjectSerializer.h"

#include "TestHarness.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;
using Editor::Project;
using Editor::ProjectHub;
using Editor::ProjectIndex;

// Mismo sandbox que ProjectNamingTests: una carpeta temporal CON ESPACIOS en el
// nombre, porque una ruta con espacios es como se rompen las que se montan
// concatenando.
struct Sandbox {
    fs::path root;
    Sandbox() {
        root = fs::temp_directory_path() /
               ("motor hub " + std::to_string(
                   std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(root / "proyectos");
        fs::create_directories(root / "levels");
        fs::create_directories(root / "maps");
        fs::create_directories(root / "adventures");
        fs::create_directories(root / "objects");
        std::ofstream indice(root / "proyectos" / "index.json");
        indice << R"({"proyectos":[]})";
    }
    ~Sandbox() {
        std::error_code ec;
        fs::remove_all(root, ec);
    }
    std::string str() const { return root.string(); }
    void escribe(const std::string& carpeta, const std::string& nombre,
                 const std::string& contenido = "{}") const {
        std::ofstream f(root / carpeta / nombre);
        f << contenido;
    }
    bool hay(const std::string& carpeta, const std::string& nombre) const {
        return fs::exists(root / carpeta / nombre);
    }
    std::string leeIndice() const {
        std::ifstream f(root / "proyectos" / "index.json");
        return std::string((std::istreambuf_iterator<char>(f)),
                           std::istreambuf_iterator<char>());
    }
};

ProjectHub hubDe(const Sandbox& s) {
    auto h = ProjectHub::load(s.str());
    MOTOR_CHECK(h.isOk());
    return h.value();
}

// Teclear un texto entero, como lo haria el autor.
void teclear(ProjectHub& hub, const std::string& texto) {
    for (char c : texto) {
        hub.key(c);
    }
}

bool elMensajeDice(const ProjectHub& hub, const std::string& aguja) {
    for (const std::string& linea : hub.message()) {
        if (linea.find(aguja) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// Un proyecto con dos ficheros suyos (con prefijo) y uno compartido, para que
// el informe de la copia tenga las dos listas. Misma forma que el fixture de
// ProjectNamingTests: si cambia el contrato de duplicate(), fallan los dos.
Project proyectoConAssets(const Sandbox& s, const std::string& nombre = "Este sur") {
    const auto creado = ProjectIndex::createNamed(s.str(), nombre, "es_", "test", "2000");
    MOTOR_CHECK(creado.isOk());
    Project p = creado.value();
    s.escribe("levels", "es_taurengrad.json");
    s.escribe("objects", "es_objetos.json");
    s.escribe("objects", "mapamundi_objetos.json");  // compartido: sin prefijo
    p.levels = {"es_taurengrad.json"};
    p.catalogs = {"es_objetos.json", "mapamundi_objetos.json"};
    p.entry = "es_taurengrad.json";
    const auto texto = ProjectSerializer::serializeProject(p);
    MOTOR_CHECK(texto.isOk());
    std::ofstream f(s.root / "proyectos" / (p.id + ".json"));
    f << texto.value();
    return p;
}

// ---------------------------------------------------------------------
// La pantalla vacia
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(un_hub_sin_proyectos_esta_en_lista_y_lo_dice) {
    Sandbox s;
    ProjectHub hub = hubDe(s);
    MOTOR_CHECK(hub.mode() == ProjectHub::Mode::List);
    MOTOR_CHECK(hub.lines().empty());
    MOTOR_CHECK(hub.size() == 0);
    MOTOR_CHECK(hub.current() == nullptr);
    const std::vector<std::string> det = hub.detail();
    MOTOR_CHECK(!det.empty());
    MOTOR_CHECK(det[0].find("No hay ningun proyecto") != std::string::npos);

    // Mover con la lista vacia no puede dejar la seleccion fuera de rango.
    hub.moveUp();
    hub.moveDown();
    MOTOR_CHECK(hub.selected() == 0);

    // Y sin proyecto marcado, ni duplicar ni borrar abren su pantalla: abrir
    // un campo de texto para acabar fallando al confirmar es peor que no
    // abrirlo, porque el autor escribe un nombre y lo pierde.
    hub.key('d');
    MOTOR_CHECK(hub.mode() == ProjectHub::Mode::List);
    hub.key('x');
    MOTOR_CHECK(hub.mode() == ProjectHub::Mode::List);
}

// ---------------------------------------------------------------------
// Los dos campos de texto no piden lo mismo
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(el_campo_de_duplicar_acepta_un_nombre_visible_y_el_de_crear_no) {
    Sandbox s;
    proyectoConAssets(s);
    ProjectHub hub = hubDe(s);

    // Crear sigue pidiendo un id. El filtro es el que ya habia y tiene su
    // motivo: create() rechazaria cualquier otra cosa, y el autor no se
    // enteraria hasta pulsar ENTER.
    hub.key('n');
    teclear(hub, "copia norte");
    MOTOR_CHECK(hub.draftId() == "copianorte");   // el espacio no es de id
    MOTOR_CHECK(hub.key(27) == ProjectHub::Action::None);
    MOTOR_CHECK(hub.mode() == ProjectHub::Mode::List);

    // Duplicar pide un NOMBRE: es la unica pantalla del editor donde el autor
    // escribe como habla. Los espacios y las mayusculas caben en el campo y
    // los pliega suggestId al confirmar, no el filtro de teclas.
    hub.key('d');
    MOTOR_CHECK(hub.mode() == ProjectHub::Mode::DuplicateProject);
    teclear(hub, "Copia Norte");
    MOTOR_CHECK(hub.draftId() == "Copia Norte");
    hub.key('\b');
    MOTOR_CHECK(hub.draftId() == "Copia Nort");
    hub.key(27);
    MOTOR_CHECK(hub.mode() == ProjectHub::Mode::List);
    MOTOR_CHECK(hub.draftId().empty());           // cancelar limpia el campo
}

// ---------------------------------------------------------------------
// Duplicar
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(duplicar_desde_la_pantalla_deja_la_copia_marcada_y_ensena_el_informe) {
    Sandbox s;
    proyectoConAssets(s);
    ProjectHub hub = hubDe(s);
    const std::size_t antes = hub.size();

    hub.key('d');
    teclear(hub, "Copia Norte");
    MOTOR_CHECK(hub.key('\n') == ProjectHub::Action::None);
    MOTOR_CHECK(hub.mode() == ProjectHub::Mode::Message);
    MOTOR_CHECK(hub.lastOk());
    MOTOR_CHECK(hub.size() == antes + 1);

    // La copia queda marcada: acabas de nombrarla, es lo que quieres abrir.
    MOTOR_CHECK(hub.current() != nullptr);
    MOTOR_CHECK(hub.current()->id == "copia_norte");
    MOTOR_CHECK(hub.current()->name == "Copia Norte");

    // El informe se ENSENA, que es para lo que duplicate() lo devuelve en vez
    // de imprimirlo: el autor tiene que saber que es suyo y que es prestado
    // antes de ponerse a editar la copia.
    MOTOR_CHECK(elMensajeDice(hub, "Copiados 2"));
    MOTOR_CHECK(elMensajeDice(hub, "Compartidos 1"));
    MOTOR_CHECK(elMensajeDice(hub, "mapamundi_objetos.json"));

    // En disco: la copia con sus ficheros renombrados, el original intacto.
    MOTOR_CHECK(s.hay("proyectos", "copia_norte.json"));
    MOTOR_CHECK(s.hay("levels", "copia_norte_taurengrad.json"));
    MOTOR_CHECK(s.hay("levels", "es_taurengrad.json"));
    MOTOR_CHECK(s.leeIndice().find("copia_norte") != std::string::npos);
}

MOTOR_TEST_CASE(duplicar_dos_veces_con_el_mismo_nombre_no_pisa_nada) {
    Sandbox s;
    proyectoConAssets(s);
    ProjectHub hub = hubDe(s);

    hub.key('d');
    teclear(hub, "Copia");
    hub.key('\n');
    MOTOR_CHECK(hub.lastOk());
    MOTOR_CHECK(hub.current()->id == "copia");

    hub.key(' ');                       // cerrar el mensaje y volver a la lista
    MOTOR_CHECK(hub.mode() == ProjectHub::Mode::List);

    // La segunda copia se hace sobre la primera: mismo nombre visible otra vez.
    hub.key('d');
    teclear(hub, "Copia");
    hub.key('\n');
    MOTOR_CHECK(hub.lastOk());
    // uniqueId resuelve la colision. Fallar obligaria al autor a inventarse
    // otro nombre para un proyecto que legitimamente se llama igual.
    MOTOR_CHECK(hub.current()->id == "copia_2");
    MOTOR_CHECK(hub.size() == 3);
    MOTOR_CHECK(s.hay("proyectos", "copia.json"));
    MOTOR_CHECK(s.hay("proyectos", "copia_2.json"));
    MOTOR_CHECK(s.hay("levels", "copia_taurengrad.json"));
    MOTOR_CHECK(s.hay("levels", "copia_2_taurengrad.json"));
}

MOTOR_TEST_CASE(una_duplicacion_que_falla_no_deja_el_hub_a_medias) {
    Sandbox s;
    const Project p = proyectoConAssets(s);
    // El manifiesto declara un nivel que ya no esta en disco. duplicate() lo
    // comprueba ANTES de escribir un solo byte.
    fs::remove(s.root / "levels" / "es_taurengrad.json");

    ProjectHub hub = hubDe(s);
    const std::size_t antes = hub.size();
    hub.key('d');
    teclear(hub, "Copia rota");
    hub.key('\n');

    MOTOR_CHECK(hub.mode() == ProjectHub::Mode::Message);
    MOTOR_CHECK(!hub.lastOk());
    // No queda un proyecto medio registrado, que es de lo poco que el autor no
    // puede arreglar desde el editor.
    MOTOR_CHECK(hub.size() == antes);
    MOTOR_CHECK(!s.hay("proyectos", "copia_rota.json"));
    MOTOR_CHECK(s.leeIndice().find("copia_rota") == std::string::npos);
    // Y el error dice el motivo, no solo que fallo.
    MOTOR_CHECK(elMensajeDice(hub, "no esta"));
    MOTOR_CHECK(hub.current() != nullptr && hub.current()->id == p.id);
}

// ---------------------------------------------------------------------
// Borrar
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(borrar_pide_confirmacion_y_esc_no_borra_nada) {
    Sandbox s;
    const Project p = proyectoConAssets(s);
    ProjectHub hub = hubDe(s);

    MOTOR_CHECK(hub.key('x') == ProjectHub::Action::None);
    MOTOR_CHECK(hub.mode() == ProjectHub::Mode::ConfirmDelete);
    // Preguntar no es borrar: mientras la pantalla espera, el manifiesto sigue
    // ahi. Sin este paso intermedio, un ENTER de mas en la lista se llevaria
    // un proyecto entero y no habria forma de deshacerlo.
    MOTOR_CHECK(s.hay("proyectos", p.id + ".json"));

    MOTOR_CHECK(hub.key(27) == ProjectHub::Action::None);
    MOTOR_CHECK(hub.mode() == ProjectHub::Mode::List);
    MOTOR_CHECK(hub.size() == 1);
    MOTOR_CHECK(s.hay("proyectos", p.id + ".json"));
    MOTOR_CHECK(hub.current() != nullptr);
    MOTOR_CHECK(hub.current()->id == p.id);
}

MOTOR_TEST_CASE(borrar_confirmado_saca_el_proyecto_del_hub_y_deja_su_contenido) {
    Sandbox s;
    const Project p = proyectoConAssets(s);
    ProjectHub hub = hubDe(s);

    hub.key('x');
    hub.key('\n');
    MOTOR_CHECK(hub.mode() == ProjectHub::Mode::Message);
    MOTOR_CHECK(hub.lastOk());
    MOTOR_CHECK(hub.size() == 0);
    MOTOR_CHECK(hub.current() == nullptr);
    MOTOR_CHECK(!s.hay("proyectos", p.id + ".json"));
    MOTOR_CHECK(s.leeIndice().find(p.id) == std::string::npos);

    // Lo que el mensaje promete, comprobado: el nivel y los catalogos SIGUEN
    // en disco. Un borrado en cascada desde el editor es justo la operacion que
    // no se puede deshacer, y otro proyecto puede estar referenciandolos.
    MOTOR_CHECK(s.hay("levels", "es_taurengrad.json"));
    MOTOR_CHECK(s.hay("objects", "es_objetos.json"));
    MOTOR_CHECK(s.hay("objects", "mapamundi_objetos.json"));

    // Pantalla y disco de acuerdo: el indice rescargado tambien esta vacio.
    const auto indice = ProjectIndex::scan(s.str());
    MOTOR_CHECK(indice.isOk());
    MOTOR_CHECK(indice.value().size() == 0);
}

MOTOR_TEST_CASE(borrar_el_unico_proyecto_no_deja_la_seleccion_fuera_de_rango) {
    Sandbox s;
    proyectoConAssets(s);
    ProjectHub hub = hubDe(s);
    hub.key('x');
    hub.key('\n');

    MOTOR_CHECK(hub.size() == 0);
    MOTOR_CHECK(hub.lines().empty());
    MOTOR_CHECK(hub.selected() == 0);
    hub.moveUp();
    hub.moveDown();
    MOTOR_CHECK(hub.selected() == 0);
    const std::vector<std::string> det = hub.detail();
    MOTOR_CHECK(!det.empty());
    MOTOR_CHECK(det[0].find("No hay ningun proyecto") != std::string::npos);
    // Con la lista vacia las dos teclas siguen sin abrir su pantalla.
    hub.key('d');
    hub.key('x');
    MOTOR_CHECK(hub.mode() == ProjectHub::Mode::List);
}

// ---------------------------------------------------------------------
// Lo que ya funcionaba, porque la rama de tecleo ahora es compartida
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(crear_desde_la_pantalla_sigue_funcionando) {
    Sandbox s;
    ProjectHub hub = hubDe(s);
    hub.key('n');
    MOTOR_CHECK(hub.mode() == ProjectHub::Mode::NewProject);
    teclear(hub, "prueba_hub");
    MOTOR_CHECK(hub.key('\n') == ProjectHub::Action::None);
    MOTOR_CHECK(hub.mode() == ProjectHub::Mode::Message);
    MOTOR_CHECK(hub.lastOk());
    MOTOR_CHECK(hub.current() != nullptr);
    MOTOR_CHECK(hub.current()->id == "prueba_hub");
    MOTOR_CHECK(s.hay("proyectos", "prueba_hub.json"));

    hub.key(' ');
    MOTOR_CHECK(hub.mode() == ProjectHub::Mode::List);
    MOTOR_CHECK(hub.key('\n') == ProjectHub::Action::Open);
    MOTOR_CHECK(hub.openId() == "prueba_hub");
}

MOTOR_TEST_CASE(la_lista_navega_abre_y_sale_como_antes) {
    Sandbox s;
    ProjectHub hub = hubDe(s);
    hub.key('n'); teclear(hub, "alfa"); hub.key('\n'); hub.key(' ');
    hub.key('n'); teclear(hub, "beta"); hub.key('\n'); hub.key(' ');
    MOTOR_CHECK(hub.size() == 2);

    // El orden del indice no se asume: se comprueba que al bajar se llega al
    // otro y que la segunda pulsacion da la vuelta, que es lo que el autor
    // espera de una lista de dos.
    const std::string marcado = hub.current()->id;
    hub.key('s');
    const std::string otro = hub.current()->id;
    MOTOR_CHECK(otro != marcado);
    hub.key('s');
    MOTOR_CHECK(hub.current()->id == marcado);
    hub.key('w');
    MOTOR_CHECK(hub.current()->id == otro);

    MOTOR_CHECK(hub.key('\n') == ProjectHub::Action::Open);
    MOTOR_CHECK(hub.openId() == otro);
    MOTOR_CHECK(hub.key(27) == ProjectHub::Action::Quit);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
