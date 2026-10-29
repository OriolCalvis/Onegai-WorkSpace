#pragma once

#include <array>
#include <utility>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/CharacterSheet.h"
#include "RPG/Stat.h"
#include "RPG/TierRules.h"

// ---------------------------------------------------------------------
// CharacterCreation — de "quiero ser un enano forjador" a una ficha
// jugable, con las formulas del GDD y sin inventarse ninguna.
//
// GL-free a proposito, como el resto del nucleo: la pantalla de creacion
// puede ser HUD propio hoy y otra cosa manana, y las reglas no se enteran.
// Lo que se prueba en demo_creacion_personaje es esto, no la pantalla.
//
// NO CALCULA VIDA NI DEFENSAS. Eso ya estaba: CharacterSheet::healthCap()
// y ::defenses() implementan las formulas del GDD, y TierRules tiene la
// tabla de bonos y techos. La primera version de este fichero las
// reescribia -- mismas formulas, segunda copia -- que es exactamente lo
// que ARCHITECTURE.md prohibe: un concepto, una representacion, un
// propietario. Aqui se rellena la ficha y se llama a
// recompute_derived(); si el GDD cambia una formula, se cambia en un
// sitio.
//
// Lo que SI es de aqui: validar que la eleccion sea legal y traducirla a
// una ficha. Es la parte que no existia.
//
// EL ORDEN DE LOS STATS es [CON, DES, INT, CAR], el mismo de
// RaceDefinition::statBonuses. No se reordena en ningun sitio: mezclar
// dos ordenes de cuatro enteros es un bug que no da error, solo un
// personaje raro.
// ---------------------------------------------------------------------
namespace RPG {

// Los indices de los cuatro stats. NO se declara un enum nuevo: ya existe
// RPG::Stat (include/RPG/Stat.h) y tener dos enumeraciones del mismo
// concepto es como acaban desalineandose dos ordenes de cuatro enteros.
constexpr std::size_t kCON = static_cast<std::size_t>(Stat::CON);
constexpr std::size_t kDES = static_cast<std::size_t>(Stat::DES);
constexpr std::size_t kINT = static_cast<std::size_t>(Stat::INT);
constexpr std::size_t kCAR = static_cast<std::size_t>(Stat::CAR);

using StatBlock = std::array<int, 4>;

// Lo que elige el jugador. Los stats son los BASE, antes de la raza.
struct CreationChoice {
    std::string displayName;
    std::string raceId;
    std::string classId;
    std::string backgroundId;
    std::string deityId;        // opcional: no toda clase adora a nadie
    int tier = 1;
    // H2-06 — Oriol (05/10): al crear no se reparte nada; la base es 0 y los
    // stats los suben la raza y el equipo activo.
    StatBlock baseStats = {0, 0, 0, 0};

    // Del trasfondo se elige UNA de cada lista (el JSON declara
    // chooseBond/chooseFear/... = 1). Vacio = sin elegir todavia.
    std::string bondId, fearId, flawId, goalId, idealId, personalityId, virtueId;
};

class CharacterCreation {
public:
    // Los stats al crear (Oriol, 05/10): NO se reparten puntos. La base de
    // los cuatro es 0; la raza suma sus bonos y despues solo los sube el
    // equipo activo (CharacterSheet::equipBonuses). Antes habia un paso de
    // reparto con una bolsa de 12 puntos que el GDD no fijaba; se retiro.
    //
    // kStatMaxTier01 sigue siendo el techo del GDD para una base de tier
    // 0-1 («puntuados normalmente entre 1 y 6»): solo lo puede pasar una
    // ficha que no venga de esta pantalla (un pregenerado).
    static constexpr int kStatMin = 0;
    static constexpr int kStatMaxTier01 = 6;

    // Carga races/classes/backgrounds/deities de una carpeta de catalogos.
    static Result<CharacterCreation> load(const std::string& catalogsRoot = "assets/catalogs");

    const Catalogs::Catalog<RaceDefinition>& races() const { return m_races; }
    const Catalogs::Catalog<ClassDefinition>& classes() const { return m_classes; }
    const Catalogs::Catalog<BackgroundDefinition>& backgrounds() const { return m_backgrounds; }
    const Catalogs::Catalog<DeityDefinition>& deities() const { return m_deities; }

    // Las siete listas de eleccion del trasfondo (vinculo, miedo, defecto,
    // meta, ideal, personalidad, virtud), con sus cinco opciones cada una.
    //
    // Se cargan AQUI y no en BackgroundDefinition porque esa estructura las
    // aplana a tres strings sueltos (virtue/defect/goal) y es compartida
    // con otras tareas. Ampliarla para esto seria tocar el terreno de
    // otro; leerlas donde se usan no le rompe nada a nadie.
    //
    // Cada opcion lleva su id Y su nombre: el canon ya les puso nombre
    // ("La balanza de la familia") y la pantalla debe pintar ESO, no el id
    // interno (barrido F0 de etiquetas: el usuario ve nombres).
    struct OpcionNarrativa {
        std::string id;
        std::string name;   // vacio = sin nombre en el canon: se pinta el id
        // UI-BG3-01 — La frase que la explica («Tres generaciones pesando
        // justo.»): la tarjeta de la historia la ensena debajo del nombre.
        std::string description;
    };
    struct OpcionesTrasfondo {
        std::vector<OpcionNarrativa> bonds, fears, flaws, goals, ideals, personalities, virtues;
        bool vacio() const { return bonds.empty() && fears.empty() && flaws.empty(); }
    };
    const OpcionesTrasfondo* opcionesDe(const std::string& backgroundId) const;

    // Todo lo que esta mal, en frases que se puedan pintar en pantalla.
    // Vacio = la eleccion es valida. Se devuelven TODOS los problemas y no
    // solo el primero: una pantalla de creacion que corrige de uno en uno
    // es una pantalla que se abandona.
    std::vector<std::string> problemas(const CreationChoice& e) const;

    // Stats finales = base + bonos de raza. Sin tocar el techo por arriba:
    // un bono racial SI puede pasar de 6, es el premio de la raza.
    StatBlock statsFinales(const CreationChoice& e) const;
    // AV16-02b — Oriol (15/10): todo empieza a 0 y la clase suma +1 a sus DOS atributos
    // importantes (primaryStat y secondaryStat), para facilitar las tiradas del inicio.
    // Sin clase (o una desconocida): ceros. Va a ficha.classBonuses.
    StatBlock bonosDeClase(const std::string& classId) const;

    // El equipo de arranque: el de la clase y, sin repetir, el del trasfondo.
    // construir() lo mete en la mochila; la creacion lo usa para ensenar
    // con que empiezas puesto (APARIENCIA-01).
    std::vector<std::string> equipoInicial(const std::string& classId,
                                           const std::string& backgroundId) const;

    // La ficha. Falla si problemas() no esta vacio: construir una ficha
    // invalida "a medias" es peor que no construirla.
    Result<CharacterSheet> construir(const CreationChoice& e) const;

    // Deja la vida y las defensas al dia en la ficha, delegando en
    // CharacterSheet::recompute_derived(). Se ofrece aqui solo para que
    // quien crea un personaje no tenga que buscar el ClassDefinition por
    // su cuenta: el catalogo ya esta cargado dentro.
    void recalcular(CharacterSheet& ficha, const TierRules& reglas) const;

private:
    Catalogs::Catalog<RaceDefinition> m_races;
    Catalogs::Catalog<ClassDefinition> m_classes;
    Catalogs::Catalog<BackgroundDefinition> m_backgrounds;
    Catalogs::Catalog<DeityDefinition> m_deities;
    std::vector<std::pair<std::string, OpcionesTrasfondo>> m_opciones;
};

}  // namespace RPG
