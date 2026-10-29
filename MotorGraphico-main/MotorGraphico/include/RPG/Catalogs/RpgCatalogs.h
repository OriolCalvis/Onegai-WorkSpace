// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 8 paquetes del corte B:
//   D-01  (fase D, 3 j) - Un carregador que no troba res, falla
//   D-02  (fase D, 4 j) - Habilitats i encanteris
//   D-03  (fase D, 3 j) - Monstres
//   D-04  (fase D, 2 j) - Condicions
//   D-05  (fase D, 3 j) - Objectes, equipament i consumibles
//   D-06  (fase D, 3 j) - Classes, races i trasfons
//   D-07  (fase D, 5 j) - Els catàlegs restants
//   E-16  (fase E, 8 j) - Edició RPG mínima del projecte
//
// AVISO - FICHERO DE CRUCE. Con 8 paquetes encima, es de los
// sitios donde dos agentes se pisan sin enterarse. Antes de una
// refactorizacion grande aqui, mira si hay trabajo sin commitear
// (git status) y di en que paquete estas trabajando.
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

// EL PARAGUAS de los catalogos: Catalog<T> y los 22 alias con nombre.
//
// 23/09/2026 (EST-22): de 1922 lineas a ~200. Las 21 especializaciones de
// CatalogLoader salieron a siete cabeceras por familia; esta sigue siendo
// el include que usan los 34 consumidores, asi que ninguno tuvo que
// cambiar. Quien anada un campo a una carta toca ahora su familia y no
// hace recompilar a los otros seis.

#include "RPG/Catalogs/CatalogLoadersHabilidades.h"
#include "RPG/Catalogs/CatalogLoadersPersonaje.h"
#include "RPG/Catalogs/CatalogLoadersRasgos.h"
#include "RPG/Catalogs/CatalogLoadersObjetos.h"
#include "RPG/Catalogs/CatalogLoadersCriaturas.h"
#include "RPG/Catalogs/CatalogLoadersMundo.h"
#include "RPG/Catalogs/CatalogLoadersContenido.h"

namespace RPG {
namespace Catalogs {

// ============================================================
// Catalog<T> — implementación de ICatalog<T> con carga JSON.
//
// Todo es header-only (el coste de linkeo de 20 clases similares sería
// ridículo si no). Usa un std::unordered_map<string, unique_ptr<T>> para
// poseer las entradas y devolver const T* no propietarios.
// ============================================================
template <typename T>
class Catalog : public ICatalog<T> {
public:
    Catalog() = default;
    ~Catalog() override = default;

    // No se puede copiar (posee memoria); mover OK.
    Catalog(const Catalog&) = delete;
    Catalog& operator=(const Catalog&) = delete;
    Catalog(Catalog&&) noexcept = default;
    Catalog& operator=(Catalog&&) noexcept = default;

    // Carga desde texto JSON raw (wrapper {"entries": [...]}).
    // Result<int> = OK con número de entradas cargadas, o error.
    //
    // D-01 — Validación contra el contrato del cargador. Antes una entrada
    // sin id se descartaba con `continue` y cualquier excepción del cargador
    // caía en un catch(...) mudo: el catálogo se quedaba a medias y el
    // resultado seguía siendo Ok. Ahora:
    //   - falta un campo obligatorio (o cualquiera declarado, con
    //     AllDeclared) → Error con el campo, sus alias y la entrada;
    //   - el cargador lanza → Error con la entrada y el motivo;
    //   - todo-o-nada: se parsea a un buffer local antes de tocar m_store,
    //     mismo criterio que ObjectCatalog::loadFromString.
    Result<int> loadFromString(const std::string& jsonText,
                               CatalogStrictness strictness = CatalogStrictness::Structural) {
        auto parsed = JsonValue::parse(jsonText);
        if (!parsed.isOk()) return Result<int>::Error(parsed.errorMessage());
        const JsonValue& root = parsed.value();
        if (!root.isObject()) return Result<int>::Error("Catalog JSON root no es objeto");
        const JsonValue& arr = root["entries"];
        if (!arr.isArray()) return Result<int>::Error("Catalog JSON no tiene clave 'entries' (array)");

        const std::vector<CatalogField>& fields = CatalogLoader<T>::expected_fields();
        std::vector<std::unique_ptr<T>> pending;
        pending.reserve(arr.size());
        for (std::size_t i = 0; i < arr.size(); ++i) {
            const JsonValue& entry = arr[i];
            if (!entry.isObject()) {
                return Result<int>::Error("entries[" + std::to_string(i) + "] no es un objeto");
            }
            const std::string label = catalog_entry_label(entry, i);

            for (const CatalogField& field : fields) {
                if (!field.required || catalog_field_present(entry, field)) continue;
                return Result<int>::Error(label + ": falta el campo obligatorio \"" +
                                          field.names.front() + "\"" +
                                          catalog_field_hint(field));
            }
            if (strictness == CatalogStrictness::AllDeclared) {
                const std::vector<std::string> missing = catalog_missing_fields(entry, fields);
                if (!missing.empty()) {
                    return Result<int>::Error(label + ": faltan campos declarados: " +
                                              catalog_join(missing));
                }
            }

            std::unique_ptr<T> owned;
            try {
                owned = std::make_unique<T>(CatalogLoader<T>::from_json(entry));
            } catch (const std::exception& ex) {
                return Result<int>::Error(label + ": el cargador ha lanzado: " + ex.what());
            } catch (...) {
                return Result<int>::Error(label +
                                          ": el cargador ha lanzado una excepción no estándar");
            }
            if (owned->id.empty()) {
                return Result<int>::Error(label + ": la entrada no tiene id");
            }
            pending.push_back(std::move(owned));
        }

        m_store.clear();
        for (auto& owned : pending) {
            m_store[owned->id] = std::move(owned);
        }
        return Result<int>::Ok(static_cast<int>(pending.size()));
    }

    // Carga desde fichero; wrapper que llama a loadFromString.
    Result<int> loadFromFile(const std::string& path,
                             CatalogStrictness strictness = CatalogStrictness::Structural) {
        FILE* f = std::fopen(path.c_str(), "rb");
        if (!f) {
            return Result<int>::Error("Catalog: no se puede abrir fichero: " + path);
        }
        std::fseek(f, 0, SEEK_END);
        long sz = std::ftell(f);
        std::fseek(f, 0, SEEK_SET);
        std::string buf;
        if (sz > 0) {
            buf.resize(static_cast<std::size_t>(sz));
            (void)std::fread(buf.data(), 1, static_cast<std::size_t>(sz), f);
        }
        std::fclose(f);
        return loadFromString(buf, strictness);
    }

    // Programa (no JSON) — añade una entrada copiada o movida.
    const T* insert(T t) {
        if (t.id.empty()) return nullptr;
        auto owned = std::make_unique<T>(std::move(t));
        const std::string key = owned->id;
        const T* ptr = owned.get();
        m_store[key] = std::move(owned);
        return ptr;
    }

    // ICatalog<T>
    bool has(const std::string& id) const override {
        return m_store.find(id) != m_store.end();
    }
    const T* find(const std::string& id) const override {
        auto it = m_store.find(id);
        if (it == m_store.end()) return nullptr;
        return it->second.get();
    }
    std::size_t size() const override { return m_store.size(); }

    // Iteracion (const) — util para tests
    template <typename Fn>
    void forEach(Fn&& fn) const {
        for (const auto& [k, v] : m_store) fn(*v);
    }

    void clear() { m_store.clear(); }

private:
    std::unordered_map<std::string, std::unique_ptr<T>> m_store;
};

// ============================================================
// Tipos concretos de catálogos (P0 20+). La aplicación GameMachine
// hará:  ClassCatalog classes; classes.loadFromFile("assets/catalogs/classes.json");
// ============================================================
using SkillCatalog       = Catalog<SkillDefinition>;
using SpellCatalog       = Catalog<SkillDefinition>;   // hechizos comparten struct Skills
using ClassCatalog       = Catalog<ClassDefinition>;
using RaceCatalog        = Catalog<RaceDefinition>;
using BackgroundCatalog  = Catalog<BackgroundDefinition>;
using PassiveCatalog     = Catalog<PassiveDefinition>;
using FeatCatalog        = Catalog<FeatDefinition>;
using TraitCatalog       = Catalog<TraitDefinition>;
using DeityCatalog       = Catalog<DeityDefinition>;
using ConditionCatalog   = Catalog<ConditionDefinition>;
using EquipmentCatalog   = Catalog<EquipmentDefinition>;
using ConsumableCatalog  = Catalog<ConsumableDefinition>;
using MonsterCatalog     = Catalog<MonsterDefinition>;
using SummonCatalog      = Catalog<SummonDefinition>;
using MountCatalog       = Catalog<MountDefinition>;
using TrapCatalog        = Catalog<TrapDefinition>;
using LootTableCatalog   = Catalog<LootTableDefinition>;
using NpcCatalog         = Catalog<NpcDefinition>;
using EncounterTableCatalog = Catalog<RPG::EncounterTableDefinition>;
using AdventureCatalog   = Catalog<AdventureDefinition>;
using EventCatalog       = Catalog<EventDefinition>;
using LocationCatalog    = Catalog<LocationDefinition>;   // el mundo (B24)

}  // namespace Catalogs
}  // namespace RPG
