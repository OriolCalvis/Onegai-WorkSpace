// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-11  (fase E, 5 j) - Projectes i arbre de mapes
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Project/ProjectIndex.h"
#include "Project/ProjectSerializer.h"

#include "TestHarness.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

// E-11 — Nombre visible arriba, id generado abajo; y duplicar sin pisar
// nada.
//
// Lo que la ficha señala como el problema: «el recorregut continua
// depenent de noms tecnics». Hoy crear un proyecto obliga a teclear un id
// [a-z0-9_] sin acentos ni espacios. El autor quiere escribir «La llave
// del santuario» y no tener que saber que la ñ no vale.
//
// Y lo que defiende la otra mitad: duplicar un proyecto tiene que dejar
// una copia EDITABLE sin tocar el original, y si algo falla a mitad no
// puede quedar medio registrada. Un proyecto a medias en el indice es de
// las pocas cosas que el autor no puede arreglar desde el editor.

namespace {

namespace fs = std::filesystem;
using Editor::Project;
using Editor::ProjectIndex;

// Cada caso monta su propio assetsRoot en un temporal: los tests no
// pueden depender de assets/ real, que varios agentes cambian a la vez.
// La carpeta lleva un ESPACIO en el nombre a proposito — el criterio de
// cierre pide «dos projectes en carpetes amb espais», y una ruta con
// espacio es como se rompen las que se montan concatenando.
struct Sandbox {
    fs::path root;
    Sandbox() {
        root = fs::temp_directory_path() /
               ("motor proyectos " +
                std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
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
        return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    }
};

// ---------------------------------------------------------------------
// El id sale del nombre
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(el_id_sale_del_nombre_sin_acentos_ni_espacios) {
    MOTOR_CHECK(ProjectIndex::suggestId("La llave del santuario").value() ==
                "la_llave_del_santuario");
    // Acentos y ñ: DEC-15 dice que el texto visible los lleva; el
    // identificador interno, no.
    MOTOR_CHECK(ProjectIndex::suggestId("Misión de la Mañana").value() ==
                "mision_de_la_manana");
    MOTOR_CHECK(ProjectIndex::suggestId("Camí de l'Èxit").value() == "cami_de_l_exit");
    // Varios separadores seguidos son uno solo, y no sobra ninguno ni al
    // principio ni al final.
    MOTOR_CHECK(ProjectIndex::suggestId("  ¡¡Egaroth --- 2000 b.f.!!  ").value() ==
                "egaroth_2000_b_f");
    // Lo que ya era un id se queda igual.
    MOTOR_CHECK(ProjectIndex::suggestId("este_sur").value() == "este_sur");
}

MOTOR_TEST_CASE(un_nombre_del_que_no_sale_nada_es_un_error_y_no_un_id_raro) {
    // Devolver "" o "_" dejaria crear un proyecto sin identidad y el
    // fallo saldria mucho mas tarde, al no encontrar su manifiesto.
    MOTOR_CHECK(!ProjectIndex::suggestId("¿?").isOk());
    MOTOR_CHECK(!ProjectIndex::suggestId("   ").isOk());
    MOTOR_CHECK(!ProjectIndex::suggestId("").isOk());
    MOTOR_CHECK(ProjectIndex::suggestId("¿?").errorMessage().find("identificador") !=
                std::string::npos);
}

MOTOR_TEST_CASE(dos_proyectos_pueden_llamarse_igual_y_el_id_los_separa) {
    // Fallar por colision obligaria al autor a inventarse otro nombre
    // para un proyecto que legitimamente se llama igual que otro.
    const std::vector<std::string> ocupados = {"cofre", "cofre_2"};
    MOTOR_CHECK(ProjectIndex::uniqueId("cofre", ocupados) == "cofre_3");
    MOTOR_CHECK(ProjectIndex::uniqueId("puerta", ocupados) == "puerta");
}

MOTOR_TEST_CASE(crear_por_nombre_guarda_el_nombre_y_genera_el_id) {
    Sandbox s;
    const auto uno = ProjectIndex::createNamed(s.str(), "La llave del santuario", "llave_",
                                               "Oriol", "2000 b.f.");
    MOTOR_CHECK(uno.isOk());
    if (!uno.isOk()) return;
    MOTOR_CHECK(uno.value().id == "la_llave_del_santuario");
    // El nombre visible se conserva TAL CUAL, con sus mayusculas.
    MOTOR_CHECK(uno.value().name == "La llave del santuario");
    MOTOR_CHECK(s.hay("proyectos", "la_llave_del_santuario.json"));

    // Otro proyecto con el MISMO nombre visible: se permite, y el id lo
    // separa.
    const auto dos = ProjectIndex::createNamed(s.str(), "La llave del santuario");
    MOTOR_CHECK(dos.isOk());
    if (!dos.isOk()) return;
    MOTOR_CHECK(dos.value().id == "la_llave_del_santuario_2");
    MOTOR_CHECK(dos.value().name == "La llave del santuario");
    MOTOR_CHECK(s.hay("proyectos", "la_llave_del_santuario_2.json"));
}

MOTOR_TEST_CASE(crear_por_nombre_esquiva_un_manifiesto_suelto_sin_indexar) {
    // Un fichero que esta en disco y no en el indice: sin comprobarlo,
    // createNamed elegiria ese id y create() fallaria despues de haberlo
    // elegido, con un mensaje que no dice que hacer.
    Sandbox s;
    s.escribe("proyectos", "cofre.json", "{}");
    const auto p = ProjectIndex::createNamed(s.str(), "Cofre");
    MOTOR_CHECK(p.isOk());
    if (p.isOk()) {
        MOTOR_CHECK(p.value().id == "cofre_2");
    }
}

// ---------------------------------------------------------------------
// Duplicar
// ---------------------------------------------------------------------

// Un proyecto con tres ficheros suyos (con prefijo) y uno compartido.
Project montaProyectoConAssets(const Sandbox& s) {
    const auto creado = ProjectIndex::createNamed(s.str(), "Este sur", "es_", "test", "2000");
    MOTOR_CHECK(creado.isOk());
    Project p = creado.value();
    s.escribe("levels", "es_taurengrad.json");
    s.escribe("maps", "es_taurengrad.tmx", "<map/>");
    s.escribe("objects", "es_objetos.json");
    s.escribe("objects", "mapamundi_objetos.json");  // compartido: sin prefijo
    p.levels = {"es_taurengrad.json"};
    p.maps = {"es_taurengrad.tmx"};
    p.catalogs = {"es_objetos.json", "mapamundi_objetos.json"};
    p.entry = "es_taurengrad.json";
    const auto texto = ProjectSerializer::serializeProject(p);
    MOTOR_CHECK(texto.isOk());
    std::ofstream f(s.root / "proyectos" / (p.id + ".json"));
    f << texto.value();
    return p;
}

MOTOR_TEST_CASE(duplicar_copia_lo_suyo_y_comparte_lo_que_no_lo_es) {
    Sandbox s;
    const Project original = montaProyectoConAssets(s);

    const auto copia = ProjectIndex::duplicate(s.str(), original.id, "Este sur revisado");
    MOTOR_CHECK(copia.isOk());
    if (!copia.isOk()) return;
    const auto& r = copia.value();

    MOTOR_CHECK(r.project.id == "este_sur_revisado");
    MOTOR_CHECK(r.project.name == "Este sur revisado");
    MOTOR_CHECK(r.project.prefix == "este_sur_revisado_");

    // Lo que llevaba el prefijo del original es suyo: copiado y renombrado.
    MOTOR_CHECK(r.copied.size() == 3);
    MOTOR_CHECK(s.hay("levels", "este_sur_revisado_taurengrad.json"));
    MOTOR_CHECK(s.hay("maps", "este_sur_revisado_taurengrad.tmx"));
    MOTOR_CHECK(s.hay("objects", "este_sur_revisado_objetos.json"));

    // Lo que no lo llevaba se COMPARTE, no se copia: seria quedarse con
    // una version congelada del trabajo de otro.
    MOTOR_CHECK(r.shared.size() == 1);
    MOTOR_CHECK(r.shared[0] == "mapamundi_objetos.json");

    // El manifiesto de la copia apunta a los ficheros nuevos, y la
    // entrada tambien.
    MOTOR_CHECK(r.project.levels[0] == "este_sur_revisado_taurengrad.json");
    MOTOR_CHECK(r.project.entry == "este_sur_revisado_taurengrad.json");

    // Y el ORIGINAL no se ha tocado.
    MOTOR_CHECK(s.hay("levels", "es_taurengrad.json"));
    const auto vuelta = ProjectSerializer::loadProjectFile(
        (s.root / "proyectos" / (original.id + ".json")).string());
    MOTOR_CHECK(vuelta.isOk());
    if (vuelta.isOk()) {
        MOTOR_CHECK(vuelta.value().levels[0] == "es_taurengrad.json");
        MOTOR_CHECK(vuelta.value().prefix == "es_");
    }
}

MOTOR_TEST_CASE(duplicar_deja_la_copia_registrada_y_reabrible) {
    Sandbox s;
    const Project original = montaProyectoConAssets(s);
    const auto copia = ProjectIndex::duplicate(s.str(), original.id, "Copia");
    MOTOR_CHECK(copia.isOk());
    if (!copia.isOk()) return;

    MOTOR_CHECK(s.leeIndice().find("copia") != std::string::npos);
    const auto indice = ProjectIndex::scan(s.str());
    MOTOR_CHECK(indice.isOk());
    if (indice.isOk()) {
        MOTOR_CHECK(indice.value().size() == 2);
        MOTOR_CHECK(indice.value().find("copia") != nullptr);
    }
}

MOTOR_TEST_CASE(duplicar_no_sobreescribe_un_fichero_que_ya_este) {
    Sandbox s;
    const Project original = montaProyectoConAssets(s);
    // Alguien ya tiene un nivel con el nombre que le tocaria a la copia.
    s.escribe("levels", "copia_taurengrad.json", "{\"mio\":true}");

    const auto copia = ProjectIndex::duplicate(s.str(), original.id, "Copia");
    MOTOR_CHECK(!copia.isOk());
    MOTOR_CHECK(copia.errorMessage().find("no sobreescribe") != std::string::npos);

    // Y no ha quedado NADA a medias: ni manifiesto, ni indice tocado, ni
    // los otros ficheros copiados.
    MOTOR_CHECK(!s.hay("proyectos", "copia.json"));
    MOTOR_CHECK(!s.hay("maps", "copia_taurengrad.tmx"));
    MOTOR_CHECK(!s.hay("objects", "copia_objetos.json"));
    MOTOR_CHECK(s.leeIndice().find("copia") == std::string::npos);
    // El fichero ajeno sigue intacto.
    std::ifstream f(s.root / "levels" / "copia_taurengrad.json");
    const std::string contenido((std::istreambuf_iterator<char>(f)),
                                std::istreambuf_iterator<char>());
    MOTOR_CHECK(contenido == "{\"mio\":true}");
}

MOTOR_TEST_CASE(duplicar_un_proyecto_con_un_fichero_que_falta_no_registra_nada) {
    // El manifiesto declara un nivel que no esta en disco. Copiar «lo que
    // haya» dejaria una copia incompleta que el autor descubriria al
    // abrirla.
    Sandbox s;
    Project p = montaProyectoConAssets(s);
    p.levels.push_back("es_fantasma.json");
    const auto texto = ProjectSerializer::serializeProject(p);
    MOTOR_CHECK(texto.isOk());
    { std::ofstream f(s.root / "proyectos" / (p.id + ".json")); f << texto.value(); }

    const auto copia = ProjectIndex::duplicate(s.str(), p.id, "Copia");
    MOTOR_CHECK(!copia.isOk());
    MOTOR_CHECK(!s.hay("proyectos", "copia.json"));
    MOTOR_CHECK(!s.hay("levels", "copia_taurengrad.json"));
    MOTOR_CHECK(s.leeIndice().find("copia") == std::string::npos);
}

MOTOR_TEST_CASE(duplicar_un_proyecto_que_no_existe_falla_diciendolo) {
    Sandbox s;
    const auto copia = ProjectIndex::duplicate(s.str(), "no_existe", "Copia");
    MOTOR_CHECK(!copia.isOk());
    MOTOR_CHECK(copia.errorMessage().find("no_existe") != std::string::npos);
}

// ---------------------------------------------------------------------
// Mapa y casilla iniciales, y titulo del juego
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(el_titulo_y_la_casilla_inicial_sobreviven_a_guardar_y_reabrir) {
    Sandbox s;
    const auto creado = ProjectIndex::createNamed(s.str(), "Con entrada", "ce_");
    MOTOR_CHECK(creado.isOk());
    if (!creado.isOk()) return;
    Project p = creado.value();
    // initialMap tiene que pertenecer a maps: el validador no deja
    // apuntar la entrada a un mapa que el proyecto no declara.
    p.maps = {"ce_plaza"};
    p.settings.initialMap = "ce_plaza";
    p.settings.title = "La Llave del Santuario";  // lo que ve el JUGADOR
    p.settings.hasInitialCell = true;
    p.settings.initialCell = GridCoord{7, 3};

    const auto texto = ProjectSerializer::serializeProject(p);
    MOTOR_CHECK(texto.isOk());
    if (!texto.isOk()) return;
    const auto vuelta = ProjectSerializer::deserializeProject(texto.value());
    MOTOR_CHECK(vuelta.isOk());
    if (!vuelta.isOk()) return;
    MOTOR_CHECK(vuelta.value().settings.initialMap == "ce_plaza");
    MOTOR_CHECK(vuelta.value().settings.title == "La Llave del Santuario");
    MOTOR_CHECK(vuelta.value().settings.hasInitialCell);
    MOTOR_CHECK(vuelta.value().settings.initialCell.x == 7);
    MOTOR_CHECK(vuelta.value().settings.initialCell.y == 3);
    // El nombre del proyecto (lo que ve el AUTOR) es otra cosa distinta.
    MOTOR_CHECK(vuelta.value().name == "Con entrada");
}

MOTOR_TEST_CASE(un_proyecto_sin_titulo_ni_casilla_se_escribe_como_siempre) {
    // Las claves nuevas son ADITIVAS: si no estan puestas, no se
    // escriben, y el manifiesto sale byte a byte como antes. Es lo que
    // impide que añadir un campo cambie todos los ficheros que ya hay
    // (regla 1 de FORMATO_PROYECTOS.md).
    Sandbox s;
    const auto creado = ProjectIndex::createNamed(s.str(), "Sin entrada");
    MOTOR_CHECK(creado.isOk());
    if (!creado.isOk()) return;
    const auto texto = ProjectSerializer::serializeProject(creado.value());
    MOTOR_CHECK(texto.isOk());
    if (!texto.isOk()) return;
    MOTOR_CHECK(texto.value().find("\"title\"") == std::string::npos);
    MOTOR_CHECK(texto.value().find("\"initialCell\"") == std::string::npos);

    // Y un manifiesto SIN esas claves se abre con los valores por defecto.
    const auto vuelta = ProjectSerializer::deserializeProject(texto.value());
    MOTOR_CHECK(vuelta.isOk());
    if (vuelta.isOk()) {
        MOTOR_CHECK(vuelta.value().settings.title.empty());
        MOTOR_CHECK(!vuelta.value().settings.hasInitialCell);
    }
}

MOTOR_TEST_CASE(una_casilla_inicial_a_medias_es_un_error_y_no_un_cero) {
    // "initialCell": {"x": 3} sin y. Rellenar y=0 pondria al jugador en
    // un sitio que nadie eligio.
    const std::string texto = R"({
      "schemaVersion": 1, "id": "p", "name": "P", "description": "", "epoch": "",
      "author": "", "prefix": "",
      "settings": {"schemaVersion": 1, "mapsDirectory": "maps", "levelsDirectory": "levels",
                   "adventuresDirectory": "adventures", "catalogsDirectory": "objects",
                   "initialMap": "", "initialCell": {"x": 3}},
      "entry": "", "levels": [], "maps": [], "adventures": [], "catalogs": []
    })";
    const auto p = ProjectSerializer::deserializeProject(texto);
    MOTOR_CHECK(!p.isOk());
    MOTOR_CHECK(p.errorMessage().find("initialCell") != std::string::npos);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
