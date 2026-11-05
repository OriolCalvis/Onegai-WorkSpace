// G-08 (mitja sense interfície): demo GL-free que verifica el motor de títol
// (TitleMenuState) contra manifests reals del projecte.
//
// TitleMenuState és el model pur: carrega un projecte, construeix el menú
// (Nueva partida / Continuar / Opciones / Salir) i diu quin nivell d'entrada
// té. Sense OpenGL, sense Application.cpp, sense TitleScene. Els .cpp de
// TitleMenuState i SaveSlotState es compilen directament (mateix patró que
// cmake/TitleFlowTests.cmake), no van a cap libreria encara: ningú té fila
// sobre aquests fitxers.
//
// Verifica, sobre dades reals:
//   1. Un projecte vàlid: títol des del nom (settings.title buit), subtítol
//      de la descripció, entryPath apuntant a un fitxer real, i menús amb
//      la configuració correcta (Continuar deshabilitat sense guardats,
//      Nueva partida habilitada si l'entrada existeix).
//   2. Dos projectes diferents donen títulos i entrades diferents.
//   3. Un manifest que no existeix: error en paraules, només Salir, no
//      cai en Boundington per accident.
//   4. Navegació: moveDown/Up cicla, confirm() sobre una entrada
//      deshabilitada retorna None + message, sobre una habilitada retorna
//      la comanda.
//
// GL-free: només motor_json + motor_project + motor_rpg + motor_level +
// motor_map + motor_map_document. S'executa en CI.
#include "Game/TitleMenuState.h"
#include "Game/SaveSlotState.h"
#include "Project/ProjectSerializer.h"

#include "Core/Check.h"

#include <filesystem>
#include <iostream>
#include <string>

namespace {

namespace fs = std::filesystem;
using Game::TitleCommand;
using Game::TitleMenuState;

void verificarProjecteReal() {
    std::cout << "\n--- Projecte real: boundington.json ---\n";

    TitleMenuState t = TitleMenuState::fromManifest(
        "assets/proyectos/boundington.json", "", "");
    require(t.ok());
    require(t.error().empty());
    std::cout << "  títol: \"" << t.title() << "\"\n";
    std::cout << "  subtítol: \"" << t.subtitle() << "\"\n";
    // settings.title és buit: cau en project.name.
    require(t.title() == "Los Perdidos de Boundington");
    require(t.subtitle() == t.project().description);

    // El menú té 5 entrades: Continue, Cargar partida, Nueva partida, Opciones, Salir.
    const auto& items = t.items();
    require(items.size() == 5);
    require(items[0].command == TitleCommand::Continue);
    require(items[1].command == TitleCommand::LoadGame);
    require(items[2].command == TitleCommand::NewGame);
    require(items[3].command == TitleCommand::Options);
    require(items[4].command == TitleCommand::Quit);
    // Sense guardats: Continuar està deshabilitat amb motiu.
    require(!items[0].enabled);
    require(!items[0].reason.empty());
    std::cout << "  Continuar: DISABLE (" << items[0].reason << ")\n";
    // Nueva partida: habilitada perquè l'entrada existeix.
    require(items[2].enabled);
    require(items[2].reason.empty());
    require(!t.entryPath().empty());
    // L'entrada existeix fisicament.
    require(fs::exists(t.entryPath()));
    std::cout << "  Nueva partida: ENABLE (entry=" << t.entryPath() << ")\n";

    // El cursor comença a la primera entrada habilitada (Nueva partida).
    require(t.selectedItem().command == TitleCommand::NewGame);
}

void verificarSegonProjecte() {
    std::cout << "\n--- Projecte real: ejemplos.json ---\n";

    TitleMenuState t = TitleMenuState::fromManifest(
        "assets/proyectos/ejemplos.json", "", "");
    require(t.ok());
    require(t.title() == "Ejemplos del motor");
    require(t.entryPath() == "assets/levels/ejemplos_objetos.json");
    require(fs::exists(t.entryPath()));
    std::cout << "  títol: \"" << t.title() << "\" | entryPath: " << t.entryPath() << "\n";
    std::cout << "  diferent de boundington: "
              << (t.title() != "Los Perdidos de Boundington" ? "OK" : "FALLA")
              << "\n";
}

void verificarManifestInvalid() {
    std::cout << "\n--- Manifest invàlid (no existeix) ---\n";

    TitleMenuState t = TitleMenuState::fromManifest(
        "assets/proyectos/no_existe.json", "", "");
    require(!t.ok());
    require(!t.error().empty());
    // No cai en Boundington: el títol diu el error, només Salir.
    require(t.title() == "No se puede abrir el juego");
    require(t.items().size() == 1);
    require(t.items()[0].command == TitleCommand::Quit);
    // Salir está habilitat (motivo buit): el jugador ha de poder eixir.
    require(t.items()[0].enabled);
    std::cout << "  error: \"" << t.error() << "\"\n";
    std::cout << "  items: " << t.items().size() << " (només Salir)\n";
}

void verificarNavegacio() {
    std::cout << "\n--- Navegació: moveDown/moveUp/confirm ---\n";

    TitleMenuState t = TitleMenuState::fromManifest(
        "assets/proyectos/boundington.json", "", "");
    require(t.ok());

    // Inicia en Nueva partida (primera habilitada).
    require(t.selectedItem().command == TitleCommand::NewGame);

    // moveDown: Nueva partida → Opciones.
    t.moveDown();
    require(t.selectedItem().command == TitleCommand::Options);
    std::cout << "  moveDown: NewGame -> Options\n";

    // Un movés més: Opciones → Salir.
    t.moveDown();
    require(t.selectedItem().command == TitleCommand::Quit);
    std::cout << "  moveDown: Options -> Quit\n";

    // Wrap al principi: Salir → Continue.
    t.moveDown();
    require(t.selectedItem().command == TitleCommand::Continue);
    std::cout << "  moveDown (wrap): Quit -> Continue\n";

    // confirm() sobre Continue (deshabilitat): None + message.
    TitleCommand cmd = t.confirm();
    require(cmd == TitleCommand::None);
    require(!t.message().empty());
    std::cout << "  confirm() sobre Continue (disabled): None + \"" << t.message() << "\"\n";

    // moveDown (passant per Cargar partida, apagada) a Nueva partida i confirm: NewGame.
    t.moveDown();
    t.moveDown();
    require(t.selectedItem().command == TitleCommand::NewGame);
    cmd = t.confirm();
    require(cmd == TitleCommand::NewGame);
    require(t.message().empty());
    std::cout << "  confirm() sobre NewGame (enabled): NewGame\n";

    // moveUp torna enrere, passant per Cargar partida.
    t.moveUp();
    t.moveUp();
    require(t.selectedItem().command == TitleCommand::Continue);
    std::cout << "  moveUp: NewGame -> Continue\n";
}

}  // namespace

int main() {
    std::cout << "[TÍTOL G-08] carrega de manifests reals + navegació GL-free\n";

    verificarProjecteReal();
    verificarSegonProjecte();
    verificarManifestInvalid();
    verificarNavegacio();

    std::cout << "\n[TÍTOL G-08] totes les comprobacions han pasat.\n";
    return 0;
}
