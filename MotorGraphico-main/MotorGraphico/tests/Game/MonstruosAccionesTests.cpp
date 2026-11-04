#include "TestHarness.h"
#include "Game/EnemyCatalogBridge.h"
#include "Game/CatalogoCartas.h"
#include "Core/Json/JsonValue.h"
#include <fstream>
#include <iterator>

namespace {
JsonValue documento(const char* ruta) {
    std::ifstream f(ruta); auto d=JsonValue::parse(std::string(std::istreambuf_iterator<char>(f),{}));
    if(!d.isOk()) MOTOR_CHECK_FAIL(std::string("No se lee el documento"));return std::move(d.value());
}
}
MOTOR_TEST_CASE(las_novecientas_cifras_llegan_al_cargador_nativo) {
    RPG::Catalogs::MonsterCatalog m;MOTOR_CHECK(m.loadFromFile("assets/catalogs/monsters.json").isOk());
    int ataques=0,control=0;
    m.forEach([&](const RPG::MonsterDefinition& d){
        for(std::size_t i=0;i<d.attacks.size();++i){
            if(d.attacks[i].magnitudeHint==0){++control;}
            else MOTOR_CHECK(d.attacks[i].magnitudeHint>=2 && d.attacks[i].magnitudeHint<=14);
            ++ataques;
        }
    });
    MOTOR_CHECK(ataques==900 && control==434 && m.size()==432);
}
MOTOR_TEST_CASE(los_actores_conservan_la_fuerza_y_conocen_su_repertorio) {
    ObjectCatalog objetos;MOTOR_CHECK(objetos.loadFromFile("assets/objects/boundington_enemigos.json").isOk());
    RPG::Catalogs::MonsterCatalog m;MOTOR_CHECK(m.loadFromFile("assets/catalogs/monsters.json").isOk());
    const auto d=documento("assets/catalogs/monstruos_boundington_cartas.json");int actores=0;
    const auto& es=d["entries"];
    for(std::size_t i=0;i<es.size();++i){
        if(es[i]["tipo_ficha"].asString()!="objeto")continue;
        const std::string id=es[i]["id"].asString();const auto* def=objetos.find(id);MOTOR_CHECK(def!=nullptr);
        auto enemigo=EnemyCatalog::createEnemy(id,def,&m,{1,1},{1,1},{1,1});
        MOTOR_CHECK(enemigo.brain->maxHealth()==def->combat.maxHealth);
        for(std::size_t k=0;k<4;++k)MOTOR_CHECK(enemigo.brain->stat(static_cast<RPG::Stat>(k))==def->combat.stats[k]);
        MOTOR_CHECK(enemigo.skills->knownSkillIds().size()==es[i]["skillIds"].size());
        for(const auto& skill:def->combat.skillIds)MOTOR_CHECK(enemigo.skills->knows(skill));
        ++actores;
    }
    MOTOR_CHECK(actores==15);  // GORRAN-DIA2: +5 enemigos de las misiones del dia 2
}
MOTOR_TEST_CASE(las_treinta_y_tres_cartas_se_cargan_con_ganchos_entendidos) {
    auto catalogo=Game::CatalogoCartas::cargar("assets/catalogs");MOTOR_CHECK(catalogo.isOk());
    const auto d=documento("assets/catalogs/monstruos_boundington_cartas.json");const auto& es=d["entries"];int n=0;
    for(std::size_t i=0;i<es.size();++i){
        const auto& ids=es[i]["skillIds"];
        for(std::size_t k=0;k<ids.size();++k){
            const auto* carta=catalogo.value()->carta(ids[k].asString());
            MOTOR_CHECK(carta!=nullptr && !carta->ganchos.empty() && carta->bloqueo.empty());
            for(const auto& g:carta->ganchos)MOTOR_CHECK(g.tipo!=Game::Gancho::Tipo::Desconocido);
            ++n;
        }
    }
    MOTOR_CHECK(n==44);  // GORRAN-DIA2: +11 cartas de esos cinco
}
int main(){return motor_test::runAllTests();}
