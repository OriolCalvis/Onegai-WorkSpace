#pragma once

#include <memory>

#include "Core/Errors/Result.h"
#include "Game/Personajes/EditorApariencia.h"
#include "Game/Personajes/ManifiestoPersonajes.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/CharacterCreation.h"
#include "RPG/CharacterCreationScreen.h"
#include "RPG/CharacterSheet.h"
#include "RPG/TierRules.h"

struct SavedHero;  // Game/SaveGame.h (GUARDAR-FICHA)

namespace Game {

// H1-04 (Gantt Fase H) — el tramo GL-free de «Nueva partida» (G-09).
//
// CharacterCreationScreen ya recorre los 13 pasos y sabe construir la
// ficha, pero se queda a dos pasos del jugador:
//   1. su construir() NO llama a recompute_derived(): la ficha sale con
//      healthCap()==0 y defensas a cero, y Player::set_character_sheet()
//      sincroniza health = maxHealth = ese cero. Instalarla tal cual es
//      un personaje muerto de nacimiento.
//   2. habla en teclas crudas ('w'/'s'/'\n'/ESC): traducir eso desde
//      InputAction es el mismo mapa para cualquier consumidor.
//
// Esta clase cierra los dos: navegación al estilo OptionsState (el
// consumidor traduce InputAction fuera) y finalizar() que devuelve la
// ficha YA recalculada y lista para Player::set_character_sheet(). El
// montaje en Application (crear el flow, pintar la pantalla, cambiar de
// modo al terminar) es el cruce de la zona H y va aparte, como en
// H1-03/H1-05.
//
// La pantalla guarda una referencia a las reglas de creación: quien cree
// el flow responde de que vivan más que él (mismo contrato que la
// pantalla suelta).
class NewGameFlow {
public:
    enum class Estado { Creando, Terminado, Cancelado };

    explicit NewGameFlow(const RPG::CharacterCreation& reglas);
    // APARIENCIA-01 — Con el manifiesto de personajes, la creacion gana un
    // paso «Apariencia» justo despues de la raza (14 pasos): el cuerpo sale
    // de la raza, asi que se viste al personaje en cuanto se sabe de que
    // pueblo es. Sin manifiesto (nullptr) el flujo es el de siempre, de 13
    // (H2-06: sin el paso de reparto de stats, que se retiro).
    // El manifiesto tiene que vivir mas que el flow, como las reglas.
    NewGameFlow(const RPG::CharacterCreation& reglas, const ManifiestoPersonajes* personajes);

    // --- Navegación (InputAction se traduce fuera, como en OptionsState) ---
    void moveUp();
    void moveDown();
    // Solo hacen algo en Apariencia (cambian el valor de la fila). En los
    // pasos de lista no son navegación.
    void adjustLeft();
    void adjustRight();
    // Avanza de paso; desde Resumen termina la creación si no quedan
    // problemas (si quedan, el aviso de la pantalla los explica).
    void confirm();
    // ESC: en Raza cancela la creación; en los demás pasos retrocede uno
    // (desde Clase, a Apariencia si la hay).
    void cancel();
    // Texto crudo para el filtro y el nombre; retroceso para borrar.
    void textChar(char c);
    void backspace();

    Estado estado() const { return m_estado; }

    // La pantalla envuelta: pasos, opciones, aviso y resumen se pintan
    // desde ella, que ya expone todo lo que el HUD necesita.
    const RPG::CharacterCreationScreen& pantalla() const { return m_pantalla; }

    // --- APARIENCIA-01 ---
    // El paso Apariencia vive aqui y no en CharacterCreationScreen: la
    // pantalla es de REGLAS (lo que la ficha necesita) y la apariencia no
    // cambia ni un numero. Mientras dura, pantalla() ya esta en Clase.
    bool enApariencia() const { return m_enApariencia; }
    const EditorApariencia* editorApariencia() const { return m_apariencia.get(); }
    // Numeracion y titulo contando Apariencia ("2/14 QUÉ ASPECTO TIENES").
    int numeroDePaso() const;
    int totalDePasos() const;
    const char* tituloPaso() const;

    // Lo que la vista previa tiene que ensenar ahora mismo: al elegir raza,
    // la raza MARCADA tal cual viste su pueblo; al elegir clase, la clase
    // marcada con su equipo inicial puesto (igual con el trasfondo); y
    // despues, lo elegido. conEquipo=false mientras no hay clase.
    struct VistaPrevia {
        Apariencia apariencia;
        RPG::CreationChoice eleccion;
        bool conEquipo = false;
    };
    VistaPrevia vistaPrevia() const;

    // El tramo que faltaba: construir() + recalcular(). Error con TODOS
    // los problemas si la elección no es válida, y Error también si el
    // flujo no está Terminado (cancelado o aún creando: no hay ficha).
    // La ficha devuelta lleva healthCap()/defenseValues() al día y su
    // equipo inicial en inventoryEquipment; instalarla en juego es un
    // Player::set_character_sheet(&ficha) en el consumidor.
    //
    // APARIENCIA-01: con el catalogo de equipo, el equipo inicial sale
    // PUESTO (lo que la clase permita, ver Game::Vestuario) y la ficha ya
    // cuenta sus bonos. Sin catalogo, todo se queda en la mochila.
    Result<RPG::CharacterSheet> finalizar(const RPG::TierRules& reglas,
                                          const RPG::Catalogs::EquipmentCatalog* equipo = nullptr) const;

    // UI-BG3-01 — La ficha de una eleccion ya hecha, recalculada y SIN
    // equipo: el personaje empieza sin nada y el kit de su clase se lo da el
    // cofre de El Vacio (Oriol, 05/10). La usa el cuaderno de creacion de
    // siete hojas (Game/CuadernoCreacion.h), que no pasa por los 14 pasos.
    static Result<RPG::CharacterSheet> fichaSinEquipo(const RPG::CharacterCreation& reglas,
                                                      const RPG::CreationChoice& eleccion,
                                                      const RPG::TierRules& tiers);

    // GUARDAR-FICHA — El heroe de un guardado (SavedHero): su ficha, rehecha
    // con lo elegido al crearlo (fichaSinEquipo, con `nombre`) y lo que
    // llevaba puesto con sus bonos (lo que ya no esta en el catalogo, fuera);
    // y su aspecto, de ida y de vuelta. El nivel y los hitos los pone despues
    // GameSession::restoreSaveGame.
    static Result<RPG::CharacterSheet> fichaGuardada(const RPG::CharacterCreation& reglas,
                                                     const RPG::TierRules& tiers,
                                                     const RPG::Catalogs::EquipmentCatalog* equipo,
                                                     const SavedHero& heroe, const std::string& nombre);
    static Apariencia aparienciaGuardada(const SavedHero& heroe);
    static void guardaApariencia(const Apariencia& apariencia, SavedHero& heroe);

private:
    const RPG::CharacterCreation& m_reglas;
    RPG::CharacterCreationScreen m_pantalla;
    Estado m_estado = Estado::Creando;
    std::unique_ptr<EditorApariencia> m_apariencia;  // nullptr = sin paso Apariencia
    bool m_enApariencia = false;
};

}  // namespace Game
