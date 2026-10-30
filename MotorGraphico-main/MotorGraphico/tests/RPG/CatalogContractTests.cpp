// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   D-01  (fase D, 3 j) - Un carregador que no troba res, falla
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// D-01 — El contrato de campos de los cargadores de catálogo.
//
// Antes: Catalog<T>::loadFromString daba por buena cualquier entrada que
// produjera un id, descartaba en silencio con `continue` las que no lo
// producían y tragaba cualquier excepción del cargador en un catch(...)
// mudo. El resultado seguía siendo Ok: un cargador roto era indistinguible
// de uno que funciona. Medido el 07/09/2026, los 20 cargadores declaran 308
// campos y 150 no los lleva ninguna entrada de los 21 assets publicados.
//
// Aquí: cada cargador declara los campos que lee; una entrada escrita con
// los nombres de la versión anterior falla diciendo qué campo falta; un
// fallo no deja el catálogo a medias; y los 20 assets publicados siguen
// cargando en el modo de producción.

#include "RPG/Catalogs/RpgCatalogs.h"

#include "TestHarness.h"

#include <cstddef>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace {

using RPG::Catalogs::Catalog;
using RPG::Catalogs::CatalogField;
using RPG::Catalogs::CatalogLoader;
using RPG::Catalogs::CatalogStrictness;
using RPG::Catalogs::ConditionCatalog;

// Las condiciones con los nombres del CARGADOR VIEJO: desde D-04 el
// cargador lee los del asset (stackable/duration/effects, DEC-02) y los
// antiguos (stacking/defaultRounds/damagePerRound) ya no se aceptan.
const char* const kNombresCargadorViejo = R"({
  "entries": [
    {
      "id": "cond_sangrado",
      "name": "Sangrado",
      "description": "Pierde vida cada ronda",
      "stacking": "stacks",
      "defaultRounds": 3,
      "damagePerRound": 2
    }
  ]
})";

bool contiene(const std::string& texto, const std::string& aguja) {
    return texto.find(aguja) != std::string::npos;
}

MOTOR_TEST_CASE(los_nombres_del_cargador_viejo_fallan_nombrando_el_campo) {
    ConditionCatalog cat;
    const auto r = cat.loadFromString(kNombresCargadorViejo, CatalogStrictness::AllDeclared);
    MOTOR_CHECK(!r.isOk());
    // El error sitúa la entrada y nombra los tres campos que el cargador
    // sí lee desde D-04 (los del asset) y el JSON ya no lleva.
    MOTOR_CHECK(contiene(r.errorMessage(), "cond_sangrado"));
    MOTOR_CHECK(contiene(r.errorMessage(), "stackable"));
    MOTOR_CHECK(contiene(r.errorMessage(), "duration"));
    MOTOR_CHECK(contiene(r.errorMessage(), "effects"));
    MOTOR_CHECK(cat.size() == 0);
}

MOTOR_TEST_CASE(una_entrada_sin_id_es_un_error_no_un_descarte_silencioso) {
    ConditionCatalog cat;
    const auto r = cat.loadFromString(R"({
        "entries": [
            {"id": "cond_ok", "name": "Ok"},
            {"name": "Sin id"}
        ]
    })");
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(contiene(r.errorMessage(), "entries[1]"));
    MOTOR_CHECK(contiene(r.errorMessage(), "id"));
    MOTOR_CHECK(cat.size() == 0);
}

MOTOR_TEST_CASE(una_entrada_sin_name_es_un_error_que_situa_la_entrada) {
    ConditionCatalog cat;
    const auto r = cat.loadFromString(R"({"entries": [{"id": "cond_sin_nombre"}]})");
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(contiene(r.errorMessage(), "cond_sin_nombre"));
    MOTOR_CHECK(contiene(r.errorMessage(), "name"));
}

MOTOR_TEST_CASE(el_modo_estructural_solo_exige_los_campos_obligatorios) {
    // Es el modo de producción: los assets llevan id y name en las 2650
    // entradas, pero no el resto de campos que el cargador lee. Exigirlos
    // todos hoy rompería los 21 assets; de migrarlos se ocupan D-02..D-09
    // (D-04 ya migró las condiciones).
    ConditionCatalog cat;
    const auto r = cat.loadFromString(R"({
        "entries": [{"id": "cond_min", "name": "Mínima"}]
    })");
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value() == 1);
    const RPG::ConditionDefinition* c = cat.find("cond_min");
    MOTOR_CHECK(c != nullptr);
    // Y el modo estructural no arregla los datos, solo deja de tirarlos:
    // los campos que el JSON no lleva siguen cayendo en su default. Que
    // esto sea visible es el motivo de que AllDeclared exista.
    MOTOR_CHECK(c->stacking == RPG::ConditionDefinition::Stacking::NONE);
    MOTOR_CHECK(c->durationMode == RPG::ConditionDefinition::Duration::ROUNDS);
    MOTOR_CHECK(c->defaultRounds == 3);
    MOTOR_CHECK(c->damagePerRound == 0);
}

MOTOR_TEST_CASE(un_fallo_deja_el_catalogo_anterior_intacto) {
    ConditionCatalog cat;
    const auto buena = cat.loadFromString(R"({
        "entries": [{"id": "cond_a", "name": "A"}, {"id": "cond_b", "name": "B"}]
    })");
    MOTOR_CHECK(buena.isOk());
    MOTOR_CHECK(buena.value() == 2);
    MOTOR_CHECK(cat.size() == 2);

    // La segunda entrada rompe el contrato: el catálogo debe quedarse como
    // estaba, mismo criterio todo-o-nada que ObjectCatalog::loadFromString.
    const auto mala = cat.loadFromString(R"({
        "entries": [{"id": "cond_c", "name": "C"}, {"id": "cond_d"}]
    })");
    MOTOR_CHECK(!mala.isOk());
    MOTOR_CHECK(cat.size() == 2);
    MOTOR_CHECK(cat.has("cond_a") && cat.has("cond_b"));
    MOTOR_CHECK(!cat.has("cond_c"));
}

template <typename T>
void comprueba_declaracion() {
    const std::vector<CatalogField>& campos = CatalogLoader<T>::expected_fields();
    MOTOR_CHECK(!campos.empty());
    std::set<std::string> vistos;
    bool id_obligatorio = false;
    bool name_obligatorio = false;
    for (const CatalogField& campo : campos) {
        MOTOR_CHECK(!campo.names.empty());
        for (const char* nombre : campo.names) {
            MOTOR_CHECK(nombre != nullptr && std::string(nombre).size() > 0);
            // Un nombre declarado en dos campos a la vez haría que el
            // canónico del error y la detección de ausencias se contradijeran.
            MOTOR_CHECK(vistos.insert(nombre).second);
        }
        const std::string canonico = campo.names.front();
        if (canonico == "id") id_obligatorio = campo.required;
        if (canonico == "name") name_obligatorio = campo.required;
    }
    MOTOR_CHECK(id_obligatorio);
    MOTOR_CHECK(name_obligatorio);
}

MOTOR_TEST_CASE(los_veinte_cargadores_declaran_sus_campos) {
    comprueba_declaracion<RPG::SkillDefinition>();
    comprueba_declaracion<RPG::ClassDefinition>();
    comprueba_declaracion<RPG::RaceDefinition>();
    comprueba_declaracion<RPG::BackgroundDefinition>();
    comprueba_declaracion<RPG::PassiveDefinition>();
    comprueba_declaracion<RPG::FeatDefinition>();
    comprueba_declaracion<RPG::TraitDefinition>();
    comprueba_declaracion<RPG::DeityDefinition>();
    comprueba_declaracion<RPG::ConditionDefinition>();
    comprueba_declaracion<RPG::EquipmentDefinition>();
    comprueba_declaracion<RPG::ConsumableDefinition>();
    comprueba_declaracion<RPG::MonsterDefinition>();
    comprueba_declaracion<RPG::SummonDefinition>();
    comprueba_declaracion<RPG::MountDefinition>();
    comprueba_declaracion<RPG::TrapDefinition>();
    comprueba_declaracion<RPG::LootTableDefinition>();
    comprueba_declaracion<RPG::NpcDefinition>();
    comprueba_declaracion<RPG::AdventureDefinition>();
    comprueba_declaracion<RPG::EventDefinition>();
    comprueba_declaracion<RPG::LocationDefinition>();
}

template <typename T>
void carga_asset(const char* const ruta) {
    Catalog<T> cat;
    const auto r = cat.loadFromFile(ruta);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value() > 0);
    // El contador devuelto es el catálogo: ninguna entrada se descarta en
    // silencio por el camino (antes ok y size() podían discrepar).
    MOTOR_CHECK(static_cast<std::size_t>(r.value()) == cat.size());
}

MOTOR_TEST_CASE(los_assets_publicados_cargan_en_modo_estructural) {
    carga_asset<RPG::SkillDefinition>("assets/catalogs/skills.json");
    carga_asset<RPG::SkillDefinition>("assets/catalogs/spells.json");
    carga_asset<RPG::ClassDefinition>("assets/catalogs/classes.json");
    carga_asset<RPG::RaceDefinition>("assets/catalogs/races.json");
    carga_asset<RPG::BackgroundDefinition>("assets/catalogs/backgrounds.json");
    carga_asset<RPG::PassiveDefinition>("assets/catalogs/passives.json");
    carga_asset<RPG::FeatDefinition>("assets/catalogs/feats.json");
    carga_asset<RPG::TraitDefinition>("assets/catalogs/traits.json");
    carga_asset<RPG::DeityDefinition>("assets/catalogs/deities.json");
    carga_asset<RPG::ConditionDefinition>("assets/catalogs/conditions.json");
    carga_asset<RPG::EquipmentDefinition>("assets/catalogs/equipment.json");
    carga_asset<RPG::ConsumableDefinition>("assets/catalogs/consumables.json");
    carga_asset<RPG::MonsterDefinition>("assets/catalogs/monsters.json");
    carga_asset<RPG::SummonDefinition>("assets/catalogs/summons.json");
    carga_asset<RPG::MountDefinition>("assets/catalogs/mounts.json");
    carga_asset<RPG::TrapDefinition>("assets/catalogs/traps.json");
    carga_asset<RPG::LootTableDefinition>("assets/catalogs/loot_tables.json");
    carga_asset<RPG::NpcDefinition>("assets/catalogs/npcs.json");
    carga_asset<RPG::AdventureDefinition>("assets/catalogs/adventures.json");
    carga_asset<RPG::EventDefinition>("assets/catalogs/events.json");
    carga_asset<RPG::LocationDefinition>("assets/catalogs/locations.json");
}

}  // namespace

int main() { return motor_test::runAllTests(); }
