#include "TestHarness.h"
#include "CartasPrueba.h"
#include "Game/ContenedoresEquipo.h"
#include <algorithm>
#include <fstream>
#include <iterator>
#include "Game/Contenedores.h"
#include "Level/ObjectCatalog.h"
#include "Core/Json/JsonValue.h"
#include "RPG/Narrative/NarrativeState.h"

namespace {
const RPG::Catalogs::EquipmentCatalog& equipo() {
    static RPG::Catalogs::EquipmentCatalog e;
    if (!e.size() && !e.loadFromFile("assets/catalogs/equipment.json").isOk()) std::abort();
    return e;
}
bool tieneCarta(const Game::CartasEnCombate& mesa, const std::string& id) {
    return std::any_of(mesa.mano().begin(), mesa.mano().end(), [&](const auto& c){return c.carta->def->id == id;});
}
}
MOTOR_TEST_CASE(el_equipo_presta_la_carta_sin_aprenderla) {
    RPG::CharacterSheet ficha; ficha.knownSkillIds = {"sombra_cegadora"};
    ficha.equippedIds[0] = "bnd_unico_tiza"; Luchador heroe{30, 2, 5}; heroe.ficha = &ficha;
    auto mesa = Game::mesaConBotin(catalogo(), heroe, &equipo());
    MOTOR_CHECK(tieneCarta(*mesa,"bnd_unico_tiza_carta"));
    MOTOR_CHECK(ficha.knownSkillIds == std::vector<std::string>{"sombra_cegadora"});
    ficha.equippedIds.fill("");
    auto sinPieza = Game::mesaConBotin(catalogo(), heroe, &equipo());
    MOTOR_CHECK(!tieneCarta(*sinPieza,"bnd_unico_tiza_carta"));
}
MOTOR_TEST_CASE(dos_ranuras_no_duplican_la_carta) {
    RPG::CharacterSheet ficha; ficha.equippedIds.fill("bnd_unico_tiza");
    Luchador heroe{30,2,5}; heroe.ficha=&ficha;
    auto mesa=Game::mesaConBotin(catalogo(),heroe,&equipo());
    MOTOR_CHECK(mesa->mano().size()==1 && ficha.knownSkillIds.empty());
}
MOTOR_TEST_CASE(sin_ficha_o_catalogo_se_conserva_la_mesa_basica) {
    Luchador heroe{30,2,5};auto mesa=Game::mesaConBotin(catalogo(),heroe,nullptr);
    MOTOR_CHECK(mesa!=nullptr && mesa->mano().empty());
}
MOTOR_TEST_CASE(las_doce_unicas_ejecutan_su_efecto_en_batalla) {
    int jugadas=0;
    equipo().forEach([&](const RPG::EquipmentDefinition& eq){
        if (eq.id.compare(0,10,"bnd_unico_")!=0) return;
        RPG::CharacterSheet ficha; ficha.equippedIds[0]=eq.id;
        ficha.conditions.push_back({"cegado","prueba",2,1});
        Luchador heroe{30,2,5}, enemigo{60,1,1};heroe.ficha=&ficha;heroe.ponerVida(5);
        SkillCatalog legado;DadoFijo dado{6};
        BattleState batalla({{"Heroe",&heroe,nullptr,nullptr}},{{"Enemigo",&enemigo,nullptr,nullptr}},&legado);
        batalla.setRandomEngine(dado); batalla.setConditionCatalog(&catalogo().condiciones());
        auto mesa=Game::mesaConBotin(catalogo(),heroe,&equipo());auto* cartas=mesa.get();
        MOTOR_CHECK(cartas->mano().size()==1 && ficha.knownSkillIds.empty());
        MOTOR_CHECK(cartas->mano()[0].carta->bloqueo.empty());
        batalla.setMesa(std::move(mesa)); BattleAction a;
        MOTOR_CHECK(cartas->accionDeFila(0,a));batalla.resolveAllyAction(0,a);
        MOTOR_CHECK(cartas->mano()[0].gastada);
        const auto& g=cartas->mano()[0].carta->ganchos.front();
        if (g.tipo==T::Cura) MOTOR_CHECK(heroe.health()>5);
        else if(g.tipo==T::Limpia) MOTOR_CHECK(ficha.conditions.empty());
        else if(g.tipo==T::Escudo) {
            MOTOR_CHECK(cartas->situacion(true,0)->escudoPorGolpe==g.numero);
            batalla.tickRound(); batalla.resolveAllyAction(0,{BattleActionType::Attack,"",0,""});
            MOTOR_CHECK(cartas->situacion(true,0)->escudo>0);
        }
        else if(g.tipo==T::Propio) MOTOR_CHECK(!cartas->situacion(true,0)->estados.empty());
        else MOTOR_CHECK(!cartas->situacion(false,0)->estados.empty());
        ++jugadas;
    });
    MOTOR_CHECK(jugadas==12);
}
MOTOR_TEST_CASE(las_tablas_publicadas_generan_con_el_catalogo_real) {
    const auto lee=[](const char* ruta){std::ifstream f(ruta); return std::string(std::istreambuf_iterator<char>(f),{});};
    auto proyecto=JsonValue::parse(lee("assets/proyectos/boundington.json")); MOTOR_CHECK(proyecto.isOk());
    ObjectCatalog objetos;
    const auto& nombres=proyecto.value()["catalogs"];
    for(std::size_t i=0;i<nombres.size();++i) MOTOR_CHECK(objetos.loadFromFile("assets/objects/"+nombres[i].asString()).isOk());
    auto tablas=Game::leeTablasBotin(lee("assets/catalogs/contenedores_tablas.json")); MOTOR_CHECK(tablas.isOk());
    RPG::NarrativeState estado; int comprobados=0;
    for(const auto& id:objetos.ids()) {
        const auto& c=objetos.find(id)->contenedor;if(c.tipo.empty()) continue;
        auto it=std::find_if(tablas.value().begin(),tablas.value().end(),[&](const auto& t){return t.id==c.tabla;});
        MOTOR_CHECK(it!=tablas.value().end());
        for(int semilla=1;semilla<=32;++semilla) {
            auto r=Game::generaContenedor(c,*it,objetos,estado,Game::claveContenedor(id,std::to_string(semilla)),semilla);
            MOTOR_CHECK(r.isOk());int valor=r.value().oro;
            for(const auto& item:r.value().objetos) valor+=objetos.find(item)->pickup.price;
            MOTOR_CHECK(valor<=c.presupuesto);
        }
        ++comprobados;
    }
    MOTOR_CHECK(comprobados==26);
}
int main(){return motor_test::runAllTests();}
