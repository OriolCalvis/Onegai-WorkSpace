// Sustituir un rasgo quita el natural; recuperarlo no duplica capas ni pierde la eleccion.
#include "TestHarness.h"
#include "Game/Personajes/RasgosApariencia.h"
#include "Game/Personajes/VisibilidadRasgos.h"
#include <algorithm>
MOTOR_TEST_CASE(rasgo_natural_se_conserva_se_quita_y_se_sustituye) {
    Game::RazaVisual r; r.id="prueba"; r.capas={"cuerpo","cola","cuernos"};
    auto a=Game::Apariencia::deRaza(r);
    MOTOR_CHECK(Game::RasgosApariencia::actual(a,&r,"cola")=="__raza__");
    Game::RasgosApariencia::poner(a,&r,"cola","cola_lobo");
    MOTOR_CHECK(a.ropa.at("cola")=="cola_lobo");
    MOTOR_CHECK(std::count(a.quitarCapas.begin(),a.quitarCapas.end(),"cola")==1);
    Game::RasgosApariencia::poner(a,&r,"cola","cola_gato");
    MOTOR_CHECK(std::count(a.quitarCapas.begin(),a.quitarCapas.end(),"cola")==1);
    Game::RasgosApariencia::poner(a,&r,"cola","");
    MOTOR_CHECK(a.ropa.count("cola")==0);
    MOTOR_CHECK(Game::RasgosApariencia::actual(a,&r,"cola").empty());
    Game::RasgosApariencia::poner(a,&r,"cola","__raza__");
    MOTOR_CHECK(a.quitarCapas.empty());
    MOTOR_CHECK(Game::RasgosApariencia::actual(a,&r,"cola")=="__raza__");
}
MOTOR_TEST_CASE(occlusion_se_mide_por_frame_y_no_por_presencia_del_png) {
    Game::Apariencia a;a.ropa["alas"]="alas_plumas";
    Game::MapaDeCapas mapa;mapa.ranuras={"cuerpo","alas"};mapa.arriba.assign(128*64,1);
    mapa.arriba[64]=2;
    auto avisos=Game::VisibilidadRasgos::avisos(a,mapa,2);
    MOTOR_CHECK(!avisos[0].empty());MOTOR_CHECK(avisos[1].empty());
    a.ropa.clear();avisos=Game::VisibilidadRasgos::avisos(a,mapa,2);
    MOTOR_CHECK(avisos[0].empty());
}
MOTOR_TEST_CASE(vista_previa_respeta_rumbo_y_alterna_andar_y_reposo) {
    for(auto rumbo:{Game::Rumbo::Arriba,Game::Rumbo::Derecha,Game::Rumbo::Abajo,Game::Rumbo::Izquierda}) {
        for(double t:{0.0,0.5,1.0,2.0,2.7,3.9}) {
            int f=Game::VisibilidadRasgos::frame(t,rumbo,Game::Reparto::Diagonal);
            MOTOR_CHECK(Game::rumboDeFrame(f,Game::Reparto::Diagonal)==rumbo);
        }
    }
}
MOTOR_TEST_CASE(minotauro_tiene_familia_propia_y_no_viste_las_patas_como_piernas_humanas) {
    auto m=Game::ManifiestoPersonajes::cargar("assets/textures");MOTOR_CHECK(m.isOk());
    if(!m.isOk())return;
    for(const char* id:{"minotauros_de_la_llanura","minotauros_del_laberinto","aeronautas_de_esmua"}) {
        auto r=m.value().raza(id);MOTOR_CHECK(r!=nullptr);if(!r)continue;
        MOTOR_CHECK(r->talla=="minotauro");MOTOR_CHECK(!r->admite("piernas"));MOTOR_CHECK(!r->admite("pies"));
        auto a=Game::Apariencia::deRaza(*r);Game::MontajePersonajes montaje(m.value(),"assets/textures");
        a.ropa["cuernos"]="cuernos_carnero";  // tambien los guardados anteriores sin quitarCapas

        auto capas=montaje.capas(a);MOTOR_CHECK(capas.isOk());if(!capas.isOk())continue;
        int cuernos=0;for(const auto& c:capas.value())if(c.ranura=="cuernos")++cuernos;
        MOTOR_CHECK(cuernos==1);
    }
}

MOTOR_TEST_CASE(las_partidas_anteriores_del_minotauro_migran_sin_cambiar_otra_raza) {
    Game::Apariencia a; a.raza="minotauros_del_laberinto"; a.talla="grande";
    a.ropa["alas"]="alas_dragon"; Game::RasgosApariencia::migrarTallaAnterior(a);
    MOTOR_CHECK(a.talla=="minotauro"); MOTOR_CHECK(a.ropa.at("alas")=="alas_dragon");
    a.raza="orcos_del_consejo"; a.talla="grande"; Game::RasgosApariencia::migrarTallaAnterior(a);
    MOTOR_CHECK(a.talla=="grande");
}

MOTOR_TEST_CASE(las_partidas_con_raya_o_barba_trenzada_cargan_como_corto_y_barba_larga) {  // NET-E2
    Game::Apariencia a; a.raza="race_human_marches"; a.peloEstilo="raya"; a.barbaEstilo="BARBA_TRENZADA";
    Game::RasgosApariencia::migrarEstilosRetirados(a);
    MOTOR_CHECK(a.peloEstilo=="corto"); MOTOR_CHECK(a.barbaEstilo=="BARBA_LARGA");
    a.peloEstilo="largo"; a.barbaEstilo="BARBA_CORTA"; Game::RasgosApariencia::migrarEstilosRetirados(a);
    MOTOR_CHECK(a.peloEstilo=="largo"); MOTOR_CHECK(a.barbaEstilo=="BARBA_CORTA");
    auto m=Game::ManifiestoPersonajes::cargar("assets/textures"); MOTOR_CHECK(m.isOk()); if(!m.isOk()) return;
    MOTOR_CHECK(m.value().pelo.buscar("raya/pelo_castano")==nullptr);
    MOTOR_CHECK(m.value().barba.buscar("BARBA_TRENZADA/pelo_castano")==nullptr);
    a.peloEstilo="raya"; a.peloColor="pelo_rubio"; a.barbaEstilo="BARBA_TRENZADA"; a.barbaColor="pelo_rubio";
    Game::MontajePersonajes montaje(m.value(),"assets/textures");
    MOTOR_CHECK(!montaje.capas(a).isOk());  // sin migrar, ya no hay fila: por eso la migracion al cargar
    Game::RasgosApariencia::migrarEstilosRetirados(a);
    MOTOR_CHECK(montaje.capas(a).isOk());
}

MOTOR_TEST_CASE(la_piel_tine_las_orejas_pero_no_los_cuernos) {
    auto cargado=Game::ManifiestoPersonajes::cargar("assets/textures"); MOTOR_CHECK(cargado.isOk());
    if(!cargado.isOk()) return;
    auto m=cargado.value(); auto r=m.raza("orcos_del_consejo"); MOTOR_CHECK(r!=nullptr); if(!r)return;
    Game::Apariencia a=Game::Apariencia::deRaza(*r); a.piel="piel_clara";
    Game::MontajePersonajes montaje(m,"assets/textures"); auto capas=montaje.capas(a); MOTOR_CHECK(capas.isOk());
    if(!capas.isOk())return;
    bool orejas=false;
    for(const auto& c:capas.value()) {
        if(c.ranura=="orejas") { orejas=true; MOTOR_CHECK(!c.cambios.empty()); }
        if(c.ranura=="cuernos") MOTOR_CHECK(c.cambios.empty());
    }
    MOTOR_CHECK(orejas);
}
