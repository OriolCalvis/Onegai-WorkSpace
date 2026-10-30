#include "Project/ProjectIndex.h"
#include "Project/ProjectSerializer.h"

#include "TestHarness.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

// C-15 — ProjectIndex::addLevelToProject.
//
// Esta funcion estuvo DECLARADA SIN CUERPO: un primer intento la escribio
// contra una API de JsonValue que no existe y, al no compilar, se retiro
// el cuerpo y se dejo la firma. Una declaracion huerfana no da error
// hasta que alguien la llama, y entonces el fallo sale en el enlazado de
// otro paquete. Estas pruebas existen para que eso no pueda repetirse en
// silencio: si el cuerpo desaparece otra vez, aqui falla el enlazado.

namespace {

namespace fs = std::filesystem;

// Cada caso monta su propio assetsRoot en un temporal y se lo lleva al
// salir: los tests no pueden depender del contenido de assets/ real, que
// varios agentes estan cambiando a la vez.
struct Sandbox {
    fs::path root;
    Sandbox() {
        root = fs::temp_directory_path() /
               ("motor-project-levels-" +
                std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(root / "proyectos");
        fs::create_directories(root / "levels");
        // scan() exige el indice; sin el, create() ni siquiera llega a
        // mirar el manifiesto. Es el mismo esqueleto minimo que monta
        // ManifestTests.
        std::ofstream indice(root / "proyectos" / "index.json");
        indice << R"({"proyectos":[]})";
    }
    ~Sandbox() {
        std::error_code ec;
        fs::remove_all(root, ec);
    }
    std::string str() const { return root.string(); }

    void escribeNivel(const std::string& nombre) const {
        std::ofstream f(root / "levels" / nombre);
        f << "{}";
    }
    Editor::Project creaProyecto(const std::string& id) const {
        const auto creado = Editor::ProjectIndex::create(str(), id, "Proyecto", "p_", "test", "2000");
        MOTOR_CHECK(creado.isOk());
        return creado.value();
    }
    Editor::Project lee(const std::string& id) const {
        const auto p = ProjectSerializer::loadProjectFile(
            (root / "proyectos" / (id + ".json")).string());
        MOTOR_CHECK(p.isOk());
        return p.value();
    }
};

MOTOR_TEST_CASE(anadir_un_nivel_lo_deja_en_el_manifiesto) {
    Sandbox caja;
    caja.creaProyecto("demo");
    caja.escribeNivel("plaza.json");

    const auto r = Editor::ProjectIndex::addLevelToProject(caja.str(), "demo", "plaza.json");
    MOTOR_CHECK(r.isOk());

    const Editor::Project p = caja.lee("demo");
    MOTOR_CHECK(p.levels.size() == 1);
    MOTOR_CHECK(p.levels[0] == "plaza.json");
}

MOTOR_TEST_CASE(anadir_dos_veces_el_mismo_nivel_no_lo_duplica) {
    // Idempotente a proposito: un generador de contenido que se ejecuta
    // dos veces es lo normal, no la excepcion.
    Sandbox caja;
    caja.creaProyecto("demo");
    caja.escribeNivel("plaza.json");

    MOTOR_CHECK(Editor::ProjectIndex::addLevelToProject(caja.str(), "demo", "plaza.json").isOk());
    MOTOR_CHECK(Editor::ProjectIndex::addLevelToProject(caja.str(), "demo", "plaza.json").isOk());

    MOTOR_CHECK(caja.lee("demo").levels.size() == 1);
}

MOTOR_TEST_CASE(un_nivel_que_no_existe_en_disco_se_rechaza) {
    // Anadir al manifiesto un fichero que no esta deja un proyecto que el
    // editor lista y no puede abrir, y el fallo aparece mucho despues.
    Sandbox caja;
    caja.creaProyecto("demo");

    const auto r = Editor::ProjectIndex::addLevelToProject(caja.str(), "demo", "fantasma.json");
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(caja.lee("demo").levels.empty());
}

MOTOR_TEST_CASE(se_respeta_la_carpeta_de_niveles_que_declara_el_proyecto) {
    // No es "levels" por constante: el proyecto la fija en
    // settings.levelsDirectory. Con una constante esto pasaria con los
    // tres proyectos de hoy y se rompería con el primero que la cambie.
    Sandbox caja;
    Editor::Project p = caja.creaProyecto("demo");
    p.settings.levelsDirectory = "mis_niveles";
    MOTOR_CHECK(ProjectSerializer::saveProjectAtomic(
                    (caja.root / "proyectos" / "demo.json").string(), p).isOk());

    // En "levels" ya no vale...
    caja.escribeNivel("plaza.json");
    MOTOR_CHECK(!Editor::ProjectIndex::addLevelToProject(caja.str(), "demo", "plaza.json").isOk());

    // ...y en la carpeta que el proyecto declara, si.
    fs::create_directories(caja.root / "mis_niveles");
    { std::ofstream f(caja.root / "mis_niveles" / "plaza.json"); f << "{}"; }
    MOTOR_CHECK(Editor::ProjectIndex::addLevelToProject(caja.str(), "demo", "plaza.json").isOk());
    MOTOR_CHECK(caja.lee("demo").levels.size() == 1);
}

MOTOR_TEST_CASE(un_nombre_con_separadores_se_rechaza) {
    // El manifiesto guarda un NOMBRE DE FICHERO, no una ruta. Aceptar
    // separadores lo convertiria en otra cosa y permitiria salir de la
    // carpeta de niveles.
    Sandbox caja;
    caja.creaProyecto("demo");
    caja.escribeNivel("plaza.json");

    for (const char* malo : {"../plaza.json", "sub/plaza.json", "sub\\plaza.json", ""}) {
        const auto r = Editor::ProjectIndex::addLevelToProject(caja.str(), "demo", malo);
        MOTOR_CHECK(!r.isOk());
    }
    MOTOR_CHECK(caja.lee("demo").levels.empty());
}

MOTOR_TEST_CASE(un_proyecto_que_no_existe_da_error_y_no_crea_nada) {
    Sandbox caja;
    caja.escribeNivel("plaza.json");
    const auto r = Editor::ProjectIndex::addLevelToProject(caja.str(), "no_existe", "plaza.json");
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(!fs::exists(caja.root / "proyectos" / "no_existe.json"));
}

}  // namespace

int main() { return motor_test::runAllTests(); }
