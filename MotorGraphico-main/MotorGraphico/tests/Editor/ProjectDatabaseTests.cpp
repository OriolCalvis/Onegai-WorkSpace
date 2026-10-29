// E-16 — Edició RPG mínima del projecte: proves del model in-memory
// i de serialització (save/load round-trip).

#include "Editor/ProjectDatabaseState.h"

#include "TestHarness.h"

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

namespace {

void require_true(const std::string& what, bool ok) {
    if (!ok) throw std::runtime_error(what + ": expected true");
}

void require_eq(const std::string& what, int got, int expected) {
    if (got != expected) throw std::runtime_error(
        what + ": " + std::to_string(got) + " != " + std::to_string(expected));
}

void require_eq_str(const std::string& what, const std::string& got, const std::string& expected) {
    if (got != expected) throw std::runtime_error(
        what + ": '" + got + "' != '" + expected + "'");
}

std::string tmpPath(const std::string& name) {
    return "/tmp/mg_e16_" + name + ".json";
}

void removeFile(const std::string& path) { std::remove(path.c_str()); }

}  // namespace

// ─── Model in-memory ────────────────────────────────────────────

MOTOR_TEST_CASE(crear_entry_genera_id_amb_tipus) {
    RPG::ProjectDatabaseState db;
    auto r = db.createEntry("objecte");
    require_true("createEntry ok", r.isOk());
    require_eq_str("id comença amb tipus", r.value().substr(0, 7), "objecte");
    require_eq("count 1", static_cast<int>(db.count("objecte")), 1);
}

MOTOR_TEST_CASE(crear_dos_entries_ids_diferents) {
    RPG::ProjectDatabaseState db;
    auto r1 = db.createEntry("habilitat");
    auto r2 = db.createEntry("habilitat");
    require_true("r1 ok", r1.isOk());
    require_true("r2 ok", r2.isOk());
    require_true("ids diferents", r1.value() != r2.value());
    require_eq("count 2", static_cast<int>(db.count("habilitat")), 2);
}

MOTOR_TEST_CASE(duplicar_entry_copia_camps) {
    RPG::ProjectDatabaseState db;
    auto r = db.createEntry("objecte");
    require_true("create ok", r.isOk());
    const std::string id = r.value();

    db.updateField(id, "name", JsonValue::parse("\"Espada\"").value());
    db.updateField(id, "price", JsonValue::parse("100").value());

    auto dup = db.duplicateEntry(id);
    require_true("duplicate ok", dup.isOk());
    require_true("nou id diferent", dup.value() != id);

    const auto* original = db.getEntry(id);
    const auto* copy = db.getEntry(dup.value());
    require_true("original trobat", original != nullptr);
    require_true("copia trobada", copy != nullptr);
    require_eq_str("copia mateix tipus", copy->type, original->type);
    require_true("copia té name", copy->fields.has("name"));
    require_true("copia té price", copy->fields.has("price"));
}

MOTOR_TEST_CASE(duplicar_entry_inexistent_retorna_error) {
    RPG::ProjectDatabaseState db;
    auto r = db.duplicateEntry("no_existeix");
    require_true("error", !r.isOk());
}

MOTOR_TEST_CASE(updateField_modifica_camp) {
    RPG::ProjectDatabaseState db;
    auto r = db.createEntry("enemic");
    require_true("create ok", r.isOk());

    require_true("update ok", db.updateField(r.value(), "life", JsonValue::parse("50").value()));
    const auto* e = db.getEntry(r.value());
    require_true("entry trobat", e != nullptr);
    require_true("camp existeix", e->fields.has("life"));
    require_eq("life == 50", e->fields["life"].asInt(0), 50);
}

MOTOR_TEST_CASE(updateField_entry_inexistent_retorna_false) {
    RPG::ProjectDatabaseState db;
    require_true("update inexistent", !db.updateField("no_existeix", "x", JsonValue()));
}

MOTOR_TEST_CASE(validateEntry_posa_valid) {
    RPG::ProjectDatabaseState db;
    auto r = db.createEntry("objecte");
    require_true("create ok", r.isOk());

    require_true("valida", db.validateEntry(r.value()));
    const auto* e = db.getEntry(r.value());
    require_true("és valid", e != nullptr);
    require_true("flag valid", e->valid);
}

MOTOR_TEST_CASE(validateEntry_inexistent_retorna_false) {
    RPG::ProjectDatabaseState db;
    require_true("validate inexistent", !db.validateEntry("no_existeix"));
}

MOTOR_TEST_CASE(hasId_detecta_colisions) {
    RPG::ProjectDatabaseState db;
    auto r = db.createEntry("objecte");
    require_true("create ok", r.isOk());
    require_true("hasId true", db.hasId(r.value()));
    require_true("hasId false", !db.hasId("inexistent"));
}

MOTOR_TEST_CASE(count_filtra_per_tipus) {
    RPG::ProjectDatabaseState db;
    db.createEntry("objecte");
    db.createEntry("objecte");
    db.createEntry("habilitat");
    require_eq("objects", static_cast<int>(db.count("objecte")), 2);
    require_eq("habilitats", static_cast<int>(db.count("habilitat")), 1);
    require_eq("enemics buit", static_cast<int>(db.count("enemic")), 0);
}

// ─── Serialització (save/load) ──────────────────────────────────

MOTOR_TEST_CASE(save_roundtrip_simple) {
    const std::string path = tmpPath("roundtrip_simple");
    removeFile(path);

    RPG::ProjectDatabaseState original;
    original.createEntry("objecte");
    original.createEntry("habilitat");
    require_true("save ok", original.save(path));

    RPG::ProjectDatabaseState loaded;
    require_true("load ok", loaded.load(path));
    require_eq("mateix nombre", static_cast<int>(loaded.count("objecte")), 1);
    require_eq("mateix nombre", static_cast<int>(loaded.count("habilitat")), 1);

    removeFile(path);
}

MOTOR_TEST_CASE(save_roundtrip_camps_complexos) {
    const std::string path = tmpPath("roundtrip_camps");
    removeFile(path);

    RPG::ProjectDatabaseState original;
    auto r = original.createEntry("objecte");
    require_true("create ok", r.isOk());
    const std::string id = r.value();

    original.updateField(id, "name", JsonValue::parse("\"Espada Luminosa\"").value());
    original.updateField(id, "price", JsonValue::parse("250").value());
    original.updateField(id, "stackable", JsonValue::parse("true").value());
    original.validateEntry(id);

    require_true("save ok", original.save(path));

    RPG::ProjectDatabaseState loaded;
    require_true("load ok", loaded.load(path));
    const auto* e = loaded.getEntry(id);
    require_true("entry trobada", e != nullptr);
    require_eq_str("name", e->fields["name"].asString(), "Espada Luminosa");
    require_eq("price", e->fields["price"].asInt(0), 250);
    require_true("stackable", e->fields["stackable"].asBool(false));
    require_true("valid", e->valid);

    removeFile(path);
}

MOTOR_TEST_CASE(save_roundtrip_buit) {
    const std::string path = tmpPath("roundtrip_buit");
    removeFile(path);

    RPG::ProjectDatabaseState original;
    require_true("save buit", original.save(path));

    RPG::ProjectDatabaseState loaded;
    require_true("load buit", loaded.load(path));
    require_eq("0 entries", static_cast<int>(loaded.count("objecte")), 0);

    removeFile(path);
}

MOTOR_TEST_CASE(load_fitxer_inexistent_retorna_false) {
    RPG::ProjectDatabaseState db;
    require_true("load inexistent", !db.load("/tmp/mg_e16_no_existeix.json"));
}

MOTOR_TEST_CASE(load_json_invalid_retorna_false) {
    const std::string path = tmpPath("json_invalid");
    {
        std::ofstream f(path);
        f << "{ esto no es json }";
    }
    RPG::ProjectDatabaseState db;
    require_true("load invalid", !db.load(path));
    removeFile(path);
}

MOTOR_TEST_CASE(load_json_sense_entries_retorna_false) {
    const std::string path = tmpPath("sense_entries");
    {
        std::ofstream f(path);
        f << "{\"other\": true}";
    }
    RPG::ProjectDatabaseState db;
    require_true("load sense entries", !db.load(path));
    removeFile(path);
}

MOTOR_TEST_CASE(save_roundtrip_tots_els_tipus) {
    const std::string path = tmpPath("roundtrip_tips");
    removeFile(path);

    RPG::ProjectDatabaseState original;
    original.createEntry("objecte");
    original.createEntry("equip");
    original.createEntry("habilitat");
    original.createEntry("enemic");
    require_true("save ok", original.save(path));

    RPG::ProjectDatabaseState loaded;
    require_true("load ok", loaded.load(path));
    require_eq("objectes", static_cast<int>(loaded.count("objecte")), 1);
    require_eq("equip", static_cast<int>(loaded.count("equip")), 1);
    require_eq("habilitats", static_cast<int>(loaded.count("habilitat")), 1);
    require_eq("enemics", static_cast<int>(loaded.count("enemic")), 1);

    removeFile(path);
}

MOTOR_TEST_CASE(save_roundtrip_id_preservat) {
    const std::string path = tmpPath("roundtrip_id");
    removeFile(path);

    RPG::ProjectDatabaseState original;
    auto r = original.createEntry("objecte");
    require_true("create ok", r.isOk());
    const std::string id = r.value();

    require_true("save ok", original.save(path));

    RPG::ProjectDatabaseState loaded;
    require_true("load ok", loaded.load(path));
    require_true("mateix id", loaded.hasId(id));

    removeFile(path);
}

int main() { return motor_test::runAllTests(); }
