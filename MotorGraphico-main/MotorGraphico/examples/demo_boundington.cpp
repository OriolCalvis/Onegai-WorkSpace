// demo_boundington — los tres dias en Boundington, 1981 b.f., escena a escena.
//
// Juega la aventura por muchos caminos cortos y comprueba que cada uno llega
// a donde dice el canon («Boundington: los diálogos»). La Matanza de
// Boundington no se puede evitar -- ocurre en EV-1981-BOUNDINGTON y es canon --
// pero el jugador decide cuanta gente sobrevive.
//
// La regla de los tres cierres del primer dia:
//   pruebas suficientes  -> Venides reune 20 hombres  (Punto de los Creadores)
//   algo, pero no basta  -> Venides espera (y el segundo dia, 10 hombres)
//   nada                 -> Venides estara solo
// «Pruebas suficientes», en el juego: una prueba (la iglesia o la guarida) y
// tres testimonios (la variable "pistas").
//
// DIALOGOS-01 (13/10/2026): con los dialogos nuevos (charlas a lo D&D, la
// primera noche en la taberna, el Ocaso entero) las rutas se reescribieron
// como ESCENAS: cada una dice que hace el jugador (donde va, con quien
// habla), que prefiere contestar, y que tiene que haber pasado. El jugador
// esta en demo_boundington_partida.h. Las escenas salen de una lista
// (out/claude_dialogos02/guiones/esc_d*.py) que se juega tambien con la copia
// del motor en Python: si se cambia una, se cambian las dos.
//
// Las del segundo dia y el Ocaso estan en demo_boundington_alternativas.cpp.
// La cobertura de todos los beats la mide explorar_boundington (al azar).
//
// GL-free: no abre ventana. Corre en CI.
// (Generado por genera.py: no editar a mano sin cambiar tambien la lista.)

#include <cstdio>
#include <set>
#include <string>
#include <vector>

#include "demo_boundington_partida.h"

using namespace demo_bnd;

void escenasPrologo(const Guiones& g, std::vector<std::string>* vistos);
void escenasDiaDos(const Guiones& g, std::vector<std::string>* vistos);
void escenasOcaso(const Guiones& g, std::vector<std::string>* vistos);

namespace {

// La primera noche (noche 0) y el primer dia.
void escenasDiaUno(const Guiones& g, std::vector<std::string>* vistos) {
    {
        Escena e(g, "DIA 1: el forastero que pregunta", Suerte::Buena, {"heroe_pueblo_humano", "heroe_pueblo_de_aegroum", "heroe_rol_tank"},
                 {"¿Qué te trae a Boundington?", "¿Y qué piden a cambio del pan?", "Pagar la cama",
                  "Preguntar por los Perdidos", "Bajar la voz y preguntar por los Perdidos", "Subir a dormir",
                  "Anoche soñé con una barca.", "¿Qué sabes de los Perdidos?", "Mostrar interés [Persuasión]",
                  "Preguntarle por los Perdidos", "Convencerle de que son el mal [Persuasión]", "Busco antorchas.",
                  "Volver a sacar las antorchas", "Convencerla de que estás de su lado [Persuasión]",
                  "Escuchar lo que te pide", "No diré nada. Tranquila.", "Sentarte con él, sin decir nada",
                  "Invitarle a desayunar [Persuasión]", "Hablar de lo que has averiguado", "Contárselo todo",
                  "Separarlos [pelea sin matar]", "Hacerle hablar", "¿Quién te manda?"}, vistos);
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("venides");
        e.j.habla("tomas_jornalero");
        e.visto({"b0_tomas", "b0_rincon"});
        e.ve({"bnd0_pista_brazos"});
        e.j.habla("skilla");
        e.ve({"bnd_rumor", "bnd0_cama", "bnd_venides_reconocido", "bnd1_desayuno_hecho"});
        e.j.entra("ciudad_centro");
        e.j.habla("sectario_perdido");
        e.j.entra("interior_herreria");
        e.j.habla("aigren");
        e.ve({"bnd1_aigren_redimido"});
        e.j.entra("interior_sastreria");
        e.j.habla("luisarda");
        e.ve({"bnd1_luisarda_sala", "bnd1_luisarda_silencio"});
        e.j.entra("ciudad_oeste");
        e.j.habla("xila");
        e.ve({"bnd1_xila_honor", "bnd1_aliado_xila"});
        e.j.entra("interior_iglesia");
        e.ve({"bnd_pruebas_suficientes"});
        e.min("pistas", 3);
        e.j.entra("interior_taberna");
        e.j.anochece();
        e.j.habla("venides");
        e.ve({"bnd_venides_20"});
        e.j.duerme();
        e.ve({"bnd1_pelea_ganada", "bnd_dia_cerrado"});
        e.visto({"b1_camorrista_habla", "cierre_creadores"});
        e.fin();
    }

    {
        Escena e(g, "DIA 1: el forastero que oye rumores", Suerte::Regular, {"heroe_pueblo_oscuro", "heroe_pueblo_forastero_visible", "heroe_fe_sombria", "heroe_rol_caster"},
                 {"Pagar la cama", "Preguntar por los Perdidos", "¿Qué se dice de los Perdidos?",
                  "Mostrar interés [Persuasión]", "Preguntarle por los Perdidos",
                  "Convencerle de que son el mal [Persuasión]", "Hablar de lo que has averiguado",
                  "Contarle lo que tienes", "Defenderte [pelea sin matar]"}, vistos);
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.visto({"b0_se_oye_forastero"});
        e.j.habla("skilla");
        e.ve({"bnd0_oido_por_devoto", "bnd_rumor"});
        e.j.habla("encapuchado");
        e.j.habla("encapuchado");
        e.visto({"b0_capucha", "b0_capucha_te_mira"});
        e.di({"Subir a dormir"});
        e.j.habla("skilla");
        e.visto({"b0_subir_oido"});
        e.j.entra("ciudad_centro");
        e.j.habla("sectario_perdido");
        e.visto({"b1_secta_v1"});
        e.j.entra("interior_herreria");
        e.j.habla("aigren");
        e.nove({"bnd1_aigren_redimido"});
        e.nove({"bnd_pruebas_suficientes"});
        e.j.entra("interior_taberna");
        e.j.anochece();
        e.j.habla("venides");
        e.ve({"bnd1_venides_espera"});
        e.nove({"bnd_venides_20", "bnd_venides_10"});
        e.j.duerme();
        e.visto({"b1_pelea_oido", "cierre_espera"});
        e.ve({"bnd_dia_cerrado"});
        e.fin();
    }

    {
        Escena e(g, "DIA 1: el forastero que se acuesta temprano", Suerte::Buena, {},
                 {"Nada más", "Quedarte mirando"}, vistos);
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.nove({"bnd0_cama"});
        e.j.entra("ciudad_centro");
        e.visto({"b0_calle_sin_cama"});
        e.ve({"bnd0_cama"});
        e.j.entra("interior_taberna");  // G4-02: Rosita te sube desde la taberna
        e.j.habla("skilla");
        e.di({"Subir a dormir"});
        e.j.habla("skilla");  // G4-02: Rosita te sube; ya no duermes al pagar en la calle
        e.ve({"bnd0_dormido"});
        e.j.duerme();
        e.nove({"bnd_aviso_dado", "bnd_venides_20", "bnd_venides_10", "bnd1_pelea_ganada"});
        e.visto({"cierre_silencio"});
        e.ve({"bnd_dia_cerrado"});
        e.fin();
    }

    {
        Escena e(g, "DIA 1: la primera noche, mesa por mesa", Suerte::Buena, {"heroe_pueblo_bestial", "heroe_pueblo_forastero_visible"},
                 {"Beber con él sin preguntar nada", "Contestar de dónde vienes", "Reírte de él", "Pagar la cama",
                  "Subir a dormir"}, vistos);
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("venides");
        e.visto({"b0_venides_v1"});
        e.ve({"bnd0_venides_bebido"});
        e.j.habla("encapuchado");
        e.visto({"b0_capucha"});
        e.j.habla("encapuchado");
        e.visto({"b0_capucha_otra"});
        e.j.habla("marido_bordadora");
        e.visto({"b0_marido", "b0_ventana_v0"});
        e.j.habla("jornalero_callado");
        e.visto({"b0_tercero", "b0_rincon"});
        e.ve({"bnd0_tomas_ofendido"});
        e.j.habla("tomas_jornalero");
        e.visto({"b0_tomas_ofendido"});
        e.j.habla("berto_jornalero");
        e.ve({"bnd0_berto_cunado"});
        e.j.habla("berto_jornalero");
        e.visto({"b0_berto_ya"});
        e.j.habla("skilla");
        e.ve({"bnd0_dormido"});
        e.j.entra("interior_taberna");
        e.j.habla("tomas_jornalero");
        e.visto({"b1_rincon_ofendido"});
        e.fin();
    }

    {
        Escena e(g, "DIA 1: el tutorial, el carterista y la Pulga", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Pagar la cama", "Subir a dormir", "Dejarle ir"}, vistos);
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.entra("ciudad_centro");
        e.j.entra("interior_mercado");
        e.j.habla("tendero_mercado");
        e.j.entra("interior_joyeria");
        e.j.habla("joyero");
        e.j.entra("interior_sastreria");
        e.j.habla("sastre");
        e.j.entra("ciudad_centro");
        e.ve({"bnd_tut_cerrado"});
        e.j.entra("ciudad_centro");
        e.visto({"b1_carterista", "b1_carterista_pillado", "b1_pulga"});
        e.ve({"bnd1_carterista_amigo"});
        e.j.habla("carterista");
        e.visto({"b1_pulga_amigo"});
        e.fin();
    }

    {
        Escena e(g, "DIA 1: la cripta, invitado por los sectarios", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Pagar la cama", "Subir a dormir", "Mostrar interés [Persuasión]", "Cuenta conmigo.",
                  "Dejar las armas", "Mirar la estatua [Conocimiento arcano]", "Escuchar el canto",
                  "Hablar con alguien [Persuasión]", "¿Qué mezclas?", "¿Conoces a un caballero llamado Venides?",
                  "Callar", "Sembrar dudas en Verina"}, vistos);
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.entra("ciudad_centro");
        e.j.habla("sectario_perdido");
        e.ve({"bnd1_invitado_guarida"});
        e.j.entra("interior_iglesia_s1");
        e.j.entra("santuario_subterraneo");
        e.visto({"b1_guarida_ceremonia", "b1_edul"});
        e.ve({"bnd_pruebas_suficientes", "bnd1_dudas_verina", "bnd1_guarida_resuelta"});
        e.fin();
    }

    {
        Escena e(g, "DIA 1: la cripta, de parte del herrero", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Pagar la cama", "Subir a dormir", "Preguntarle por los Perdidos", "Decirle que te interesan",
                  "Vengo de parte del herrero.", "Ir a por lo tuyo", "Sacar a Verina de aquí"}, vistos);
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.entra("interior_herreria");
        e.j.habla("aigren");
        e.ve({"bnd1_invitado_por_aigren"});
        e.j.entra("interior_iglesia_s1");
        e.j.entra("santuario_subterraneo");
        e.visto({"b1_guarida_por_aigren"});
        e.ve({"bnd1_verina_salvada_guarida"});
        e.j.entra("interior_herreria");
        e.j.habla("aigren");
        e.fin();
    }

    {
        Escena e(g, "DIA 1: la cripta, a golpes", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Pagar la cama", "Subir a dormir", "Preguntarle por los Perdidos", "Decirle que te interesan",
                  "Vengo de parte del herrero.", "Ir a por lo tuyo", "Intentar acabar con todos"}, vistos);
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.entra("interior_herreria");
        e.j.habla("aigren");
        e.j.entra("interior_iglesia_s1");
        e.j.entra("santuario_subterraneo");
        e.ve({"bnd1_guarida_resuelta"});
        e.visto({"b1_guarida_exterminada"});
        e.fin();
    }

    {
        Escena e(g, "DIA 1: la cripta, perdida", Suerte::Mala, {"heroe_pueblo_humano"},
                 {"Pagar la cama", "Subir a dormir", "Preguntarle por los Perdidos", "Decirle que te interesan",
                  "Vengo de parte del herrero.", "Ir a por lo tuyo", "Intentar acabar con todos"}, vistos);
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.entra("interior_herreria");
        e.j.habla("aigren");
        e.j.entra("interior_iglesia_s1");
        e.j.entra("santuario_subterraneo");
        e.ve({"bnd1_guarida_resuelta"});
        e.visto({"b1_guarida_perdida"});
        e.fin();
    }

    {
        Escena e(g, "DIA 1: Pico Dragon, el gato de Mira", Suerte::Buena, {"heroe_pueblo_bestial"},
                 {"Pagar la cama", "Subir a dormir", "¿Cómo se llama?", "Probar tres nombres",
                  "Subir al árbol [Atletismo]", "¿De dónde sois?", "¿Por qué Boundington?"}, vistos);
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.entra("ciudad_oeste");
        e.j.habla("nina_del_gato");
        e.visto({"b1_mira_v0", "b1_gato_bajado"});
        e.ve({"bnd1_familia_invita"});
        e.visto({"b1_familia_mesa_manana", "b1_fuego"});
        e.ve({"bnd1_fuego_despedida"});
        e.fin();
    }

    {
        Escena e(g, "DIA 1: Pico Dragon, los Verdaderos", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Pagar la cama", "Subir a dormir", "Comer algo mientras pasa la mañana (5)", "Plantarles cara"}, vistos);
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.entra("ciudad_oeste");
        e.j.habla("nina_del_gato");
        e.visto({"b1_verdaderos", "b1_verdaderos_vencidos"});
        e.ve({"bnd1_familia_defendida"});
        e.fin();
    }

    {
        Escena e(g, "DIA 1: Pico Dragon, sumarse a los Verdaderos", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Pagar la cama", "Subir a dormir", "Comer algo mientras pasa la mañana (5)", "Sumarte a ellos"}, vistos);
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.entra("ciudad_oeste");
        e.j.habla("nina_del_gato");
        e.ve({"bnd1_verdaderos_unido"});
        e.nove({"bnd1_familia_defendida"});
        e.fin();
    }

    {
        Escena e(g, "DIA 1: Xila, despreciado y emborrachado", Suerte::Buena, {"heroe_pueblo_humano", "heroe_rol_agile"},
                 {"Pagar la cama", "Subir a dormir", "Tratarle con desprecio", "Invitarle a un trago (1) y hacer las paces"}, vistos);
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.entra("ciudad_oeste");
        e.j.habla("xila");
        e.ve({"bnd1_xila_maltratado"});
        e.j.habla("xila");
        e.visto({"b1_xila_resentido", "b1_xila_habla"});
        e.ve({"bnd1_xila_borracho"});
        e.nove({"bnd1_xila_honor"});
        e.fin();
    }

    {
        Escena e(g, "DIA 1: Aigren presionado, Luisarda engañada", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Pagar la cama", "Subir a dormir", "¿Trabajas solo?", "Pues suena muy sano.",
                  "Presionarle con su hijo [Intimidación]", "Busco antorchas.", "Volver a sacar las antorchas",
                  "Hacerle creer que ya lo sabes todo [Engaño]", "Escuchar lo que te pide",
                  "Ven a cenar a la taberna. Rosita te espera."}, vistos);
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.entra("interior_herreria");
        e.j.habla("aigren");
        e.ve({"bnd1_aigren_delata"});
        e.j.habla("aigren");
        e.visto({"b1_aigren_vuelve_presionado"});
        e.j.entra("interior_sastreria");
        e.j.habla("luisarda");
        e.ve({"bnd1_luisarda_con_rosita"});
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.visto({"b1_rosita_luisarda"});
        e.min("confianza_rosita", 1);
        e.fin();
    }

    {
        Escena e(g, "DIA 1: la noche fuera (vigilante, cuartel, Xila dormido)", Suerte::Regular, {"heroe_pueblo_humano"},
                 {"Pagar la cama", "Subir a dormir", "Pasar sin que te vea [Sigilo]",
                  "Convencerle de que te deje pasar [Persuasión]"}, vistos);
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.entra("interior_taberna");
        e.j.anochece();
        e.j.entra("interior_cuartel");
        e.visto({"b1_cuartel_noche"});
        e.j.entra("ciudad_oeste");
        e.j.habla("xila");
        e.visto({"b1_xila_noche"});
        e.j.entra("ciudad_este");
        e.visto({"b1_vigilante", "b1_vigilante_te_para"});
        e.j.entra("interior_taberna");
        e.di({"Subir a dormir"});
        e.j.duerme();
        e.ve({"bnd_dia_cerrado"});
        e.fin();
    }

    {
        Escena e(g, "DIA 1: la pelea, hasta el final", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Pagar la cama", "Subir a dormir", "Separarlos [pelea sin matar]", "Rematarle",
                  "Sacarlo y matarlo en la calle"}, vistos);
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.entra("interior_taberna");
        e.j.duerme();
        e.visto({"b1_pelea_rosita"});
        e.ve({"bnd1_camorrista_muerto", "bnd_dia_cerrado"});
        e.fin();
    }

    {
        Escena e(g, "DIA 1: la pelea, perdida", Suerte::Mala, {"heroe_pueblo_humano"},
                 {"Pagar la cama", "Subir a dormir", "Separarlos [pelea sin matar]"}, vistos);
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.entra("interior_taberna");
        e.j.duerme();
        e.visto({"b1_pelea_perdida"});
        e.ve({"bnd_dia_cerrado"});
        e.fin();
    }
}
}  // namespace

int main() {
    const Guiones g = cargaTodo();
    std::vector<std::string> vistos;
    std::printf("=== Boundington, escena a escena ===\n\n-- El prologo --\n");
    escenasPrologo(g, &vistos);
    std::printf("\n-- La primera noche y el primer dia --\n");
    escenasDiaUno(g, &vistos);
    std::printf("\n-- El segundo dia (Gorran) y el tercero --\n");
    escenasDiaDos(g, &vistos);
    std::printf("\n-- El Ocaso --\n");
    escenasOcaso(g, &vistos);

    int total = 0;
    int n = 0;
    for (const AdventureScript* a : {&g.prologo, &g.dia1, &g.gorran, &g.dia2, &g.ocaso}) {
        for (const auto& b : a->beats) {
            ++total;
            for (const auto& v : vistos)
                if (v == b.id) { ++n; break; }
        }
    }
    std::printf("\nbeats que salen en estas escenas: %d de %d (todos: explorar_boundington)\n", n, total);
    std::printf("todas las escenas han pasado\n");
    return 0;
}
