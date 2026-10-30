// demo_boundington_alternativas — las escenas del prologo, del segundo dia y del Ocaso
// (ver demo_boundington.cpp). Cada una empieza con dos dias rapidos (la cama,
// el encargo de Rosita, a dormir) y pone a mano lo que haria falta de antes
// («pon»), igual que si se hubiera jugado: asi cada escena prueba una sola
// cosa.
// (Generado por genera.py: no editar a mano sin cambiar tambien la lista.)

#include <cstdio>
#include <string>
#include <vector>

#include "demo_boundington_partida.h"

using namespace demo_bnd;


// PROLOGO-02: el prado, el barquero y los recuerdos de Venides.
void escenasPrologo(const Guiones& g, std::vector<std::string>* vistos) {
    {
        Escena e(g, "PROLOGO: el que no quiere subir, pregunta todo y vuelve", Suerte::Buena, {},
                 {"Mirar alrededor [Percepción]", "Observarle [Conocimiento arcano]", "No pienso subir ahí.",
                  "¿Quién eres?", "¿Por qué yo?", "¿Dónde estoy?", "¿Adónde lleva este río?", "¿Qué vamos a ver?",
                  "Sho-Noco, ¿quiénes son?", "Mirar a Sandes", "Mirar a Verina y Venides",
                  "¿Qué pasó en las montañas?", "¿Por qué no fue con ellos?", "¿Qué escuchan, ellos?", "¿Y Verina?",
                  "¿Lo sabía el capitán?", "¿Quiénes le esperan allí?", "¿Qué tengo que hacer yo?"}, vistos);
        e.j.fase("prologo");
        e.j.entra("prado_del_rio");
        e.pon({"kit_basico_recogido", "kit_basico_puesto", "bnd_prologo_kit_listo"});
        e.j.drena();
        e.ve({"bnd_prologo_percibe_exito"});
        e.visto({"prologo_percepcion_exito", "prologo_rio"});
        e.nove({"bnd_prologo_barquero"});
        e.j.cinematica(GridCoord{12, 9});
        e.ve({"bnd_prologo_arcano_exito", "bnd_prologo_barquero", "cine_barquero_vista"});
        e.j.habla("sho_noco");
        e.nove({"bnd_pr_sube"});
        e.ve({"bnd_p_barquero_t_no", "bnd_p_barquero_t_quien", "bnd_p_barquero_t_porque", "bnd_p_barquero_t_ver"});
        e.di({"Subir a la barca"});
        e.j.habla("sho_noco");
        e.visto({"p_barquero_p0"});
        e.ve({"bnd_pr_sube"});
        while (e.j.cinematica(GridCoord{-1, -1})) {
        }
        e.ve({"bnd_prologo_terminado", "bnd_prologo_pasado", "bnd_p_vision_t_sandes", "bnd_p_r2_t_verina",
              "bnd_p_r3_t_yo"});
        e.visto({"p_r1_brindis", "p_r1_capa", "p_r1_sandes", "p_r2_puerta", "p_r2_tranca", "p_r3_orden",
                 "p_r4_shonoco", "prologo_despertar"});
        e.fin();
    }

    {
        Escena e(g, "PROLOGO: grita, retrocede, sube sin preguntar y se queda mirando", Suerte::Mala, {},
                 {"Gritar «¿Hola?»", "Retroceder", "No pienso subir ahí.", "Subir a la barca", "Devuélveme a casa.",
                  "Mirar un poco más"}, vistos);
        e.j.fase("prologo");
        e.j.entra("prado_del_rio");
        e.pon({"kit_basico_recogido", "kit_basico_puesto", "bnd_prologo_kit_listo"});
        e.j.drena();
        e.ve({"bnd_prologo_grito", "bnd_prologo_rio"});
        e.nove({"bnd_prologo_percibe_botch", "bnd_prologo_percibe_exito"});
        e.j.cinematica(GridCoord{12, 9});
        e.ve({"bnd_prologo_retrocede", "bnd_prologo_barquero"});
        e.nove({"bnd_prologo_arcano_botch"});
        e.j.habla("sho_noco");
        e.ve({"bnd_pr_sube"});
        e.nove({"bnd_p_barquero_t_quien"});
        while (e.j.cinematica(GridCoord{-1, -1})) {
        }
        e.ve({"bnd_prologo_terminado"});
        e.nove({"bnd_p_r2_t_porque", "bnd_p_r3_t_saben"});
        e.visto({"p_r1_brindis", "p_r1_capa", "p_r1_sandes", "p_r2_puerta", "p_r2_tranca", "p_r3_orden",
                 "p_r4_shonoco", "prologo_despertar"});
        e.visto({"p_r1_libre", "p_vision_p0", "p_r2_libre", "p_r2_p0", "p_r3_libre", "p_r3_p0"});
        e.fin();
    }

    {
        Escena e(g, "PROLOGO: las dos tiradas, pifia", Suerte::Mala, {},
                 {"Mirar alrededor [Percepción]", "Observarle [Conocimiento arcano]", "No pienso subir ahí.",
                  "Subir a la barca"}, vistos);
        e.j.fase("prologo");
        e.j.entra("prado_del_rio");
        e.pon({"kit_basico_recogido", "kit_basico_puesto", "bnd_prologo_kit_listo"});
        e.j.drena();
        e.ve({"bnd_prologo_percibe_botch"});
        e.visto({"prologo_percepcion_botch"});
        e.j.cinematica(GridCoord{12, 9});
        e.ve({"bnd_prologo_arcano_botch"});
        e.visto({"prologo_arcano_botch"});
        e.j.habla("sho_noco");
        while (e.j.cinematica(GridCoord{-1, -1})) {
        }
        e.ve({"bnd_prologo_terminado"});
        e.fin();
    }

    {
        Escena e(g, "PROLOGO: las dos tiradas, critico", Suerte::Critica, {},
                 {"Mirar alrededor [Percepción]", "Observarle [Conocimiento arcano]", "No pienso subir ahí.",
                  "Subir a la barca"}, vistos);
        e.j.fase("prologo");
        e.j.entra("prado_del_rio");
        e.pon({"kit_basico_recogido", "kit_basico_puesto", "bnd_prologo_kit_listo"});
        e.j.drena();
        e.ve({"bnd_prologo_percibe_critico"});
        e.visto({"prologo_percepcion_critico"});
        e.j.cinematica(GridCoord{12, 9});
        e.ve({"bnd_prologo_arcano_critico"});
        e.visto({"prologo_arcano_critico"});
        e.j.habla("sho_noco");
        while (e.j.cinematica(GridCoord{-1, -1})) {
        }
        e.ve({"bnd_prologo_terminado"});
        e.fin();
    }

    {
        Escena e(g, "PROLOGO: lejos del rio no baja nadie", Suerte::Buena, {},
                 {"Quedarte quieto y esperar"}, vistos);
        e.j.fase("prologo");
        e.j.entra("prado_del_rio");
        e.pon({"kit_basico_recogido", "kit_basico_puesto", "bnd_prologo_kit_listo"});
        e.j.drena();
        e.ve({"bnd_prologo_espera"});
        e.j.cinematica(GridCoord{4, 9});
        e.nove({"bnd_prologo_barquero", "bnd_prologo_barca_viene"});
        e.j.habla("sho_noco");
        e.nove({"bnd_p_barquero_visto"});
        e.j.cinematica(GridCoord{13, 3});
        e.ve({"bnd_prologo_barquero"});
        e.fin();
    }

    {
        Escena e(g, "PROLOGO y primera noche: Venides reconoce al que vio la barca", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Pagar la cama", "Anoche soñé con una barca."}, vistos);
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("venides");
        e.ve({"bnd_prologo_pasado", "bnd0_noche"});
        e.fin();
    }
}

// El segundo dia.
void escenasDiaDos(const Guiones& g, std::vector<std::string>* vistos) {
    {
        Escena e(g, "DIA 2: Gorran, de buenas", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Aceptar el encargo", "Pagar el paso (5)", "Esa espada.", "¿Quién te la encargó?",
                  "Pelear por la espada", "Acabar con él", "Devolvérsela"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.ve({"bnd2_skilla_encargo"});
        e.j.entra("mazmorra_64x64");
        e.visto({"b2_gorran", "b2_gorran_victoria"});
        e.ve({"bnd2_furia_alaro"});
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.visto({"b2_skilla_espada_devuelta"});
        e.ve({"bnd2_aliado_skilla"});
        e.min("confianza_rosita", 1);
        e.fin();
    }

    {
        Escena e(g, "DIA 2: Gorran, a escondidas", Suerte::Critica, {"heroe_pueblo_humano"},
                 {"Aceptar el encargo", "Colarte por detrás [Sigilo]", "Decirle que no la encontraste"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.ve({"bnd2_skilla_encargo"});
        e.j.entra("mazmorra_64x64");
        e.visto({"b2_gorran_sigilo", "b2_gorran_robo_limpio"});
        e.ve({"bnd2_furia_alaro"});
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.nove({"bnd2_aliado_skilla"});
        e.fin();
    }

    {
        Escena e(g, "DIA 2: Gorran gana", Suerte::Mala, {"heroe_pueblo_humano"},
                 {"Aceptar el encargo", "Vengo a hablar con Gorran.", "Pelear por la espada"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.ve({"bnd2_skilla_encargo"});
        e.j.entra("mazmorra_64x64");
        e.visto({"b2_gorran_derrota"});
        e.nove({"bnd2_furia_alaro"});
        e.fin();
    }

    {
        Escena e(g, "DIA 2: Rosita cuenta su historia", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Aceptar el encargo", "Vengo a hablar con Gorran.", "Pelear por la espada",
                  "Si peleas contra ellos, estamos en paz.", "Devolvérsela y contarle todo",
                  "¿Qué fue de los otros tres?"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.pon({"bnd1_verina_salvada_guarida"});
        e.j.fase("dia2");
        e.j.drena();
        e.var("confianza_rosita", 1);
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.ve({"bnd2_skilla_encargo"});
        e.j.entra("mazmorra_64x64");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.visto({"b2_rosita_revela", "b2_rosita_verina_arriba"});
        e.ve({"bnd2_rosita_semilla", "bnd2_rosita_revelada"});
        e.fin();
    }

    {
        Escena e(g, "DIA 2: el castillo, con el camorrista", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Vengo con el caballero Venides.", "Subir a ver al capitán",
                  "Soy el que te trajo al camorrista anoche."}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.pon({"bnd1_camorrista_entregado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_castillo");
        e.visto({"b2_puerta", "b2_capitan", "b2_castillo_salida"});
        e.fin();
    }

    {
        Escena e(g, "DIA 2: la tienda abandonada y la Barriada", Suerte::Buena, {"heroe_pueblo_humano", "heroe_elemento_fuego"},
                 {"De actor", "Vengo a preguntar si necesitáis algo", "Preguntarle qué necesitan", "Cuenta conmigo."}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("ciudad_oeste");
        e.visto({"b2_tienda_abandonada", "b2_disfraz_hallado"});
        e.ve({"bnd2_disfraz"});
        e.j.entra("ciudad_surysal");
        e.j.habla("mercader");
        e.visto({"b2_forjador"});
        e.j.habla("viejo_surysal");
        e.visto({"b2_zarela", "b2_zarela_pide", "b2_saqueadores_vencidos", "b2_ritual_fuego"});
        e.ve({"bnd2_aliado_gitanos"});
        e.fin();
    }

    {
        Escena e(g, "DIA 2: la Barriada sin ayuda", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Vengo a preguntar si necesitáis algo", "Preguntarle qué necesitan", "No es mi pelea."}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("ciudad_oeste");
        e.j.entra("ciudad_surysal");
        e.j.habla("viejo_surysal");
        e.visto({"b2_gitanos_alianza", "b2_zarela_pide"});
        e.nove({"bnd2_saqueadores_vencidos", "bnd2_guerreros_gitanos"});
        e.fin();
    }

    {
        Escena e(g, "DIA 2: la Barriada, de los Perdidos", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Quiero hablar de los Perdidos.", "La Barriada se arma: empalizada y guerreros"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("ciudad_surysal");
        e.j.habla("viejo_surysal");
        e.j.entra("ciudad_centro");
        e.j.habla("sectario_perdido");
        e.ve({"bnd2_barriada_delatada"});
        e.fin();
    }

    {
        Escena e(g, "DIA 2: la fiesta, por la ventana y con el traje roto", Suerte::Regular, {"heroe_pueblo_humano"},
                 {"Esquivar al vigilante [Sigilo]", "Buscar otra ruta por los jardines",
                  "Saltar la verja y entrar por una ventana [Atletismo]", "Mezclarte con los invitados",
                  "Sonsacarle, con el traje roto [Engaño]"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.ve({"bnd2_skilla_encargo"});
        e.j.anochece();
        e.j.entra("ciudad_este");
        e.visto({"b2_fiesta_puerta", "b2_vigilante_te_para", "b2_trepa_roto"});
        e.fin();
    }

    {
        Escena e(g, "DIA 2: la fiesta, por la puerta y hablando", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"De noble excéntrico de Ascaria", "Hablar con el vigilante", "¿Muchos años de vigilante?",
                  "Convencerle de que te deje pasar [Persuasión]",
                  "Entrar por la puerta, con disfraz [Interpretación]", "Mezclarte con los invitados",
                  "Sonsacar al noble que brinda [Engaño]", "Señalárselo a Venides", "Contárselo todo"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("ciudad_oeste");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.ve({"bnd2_skilla_encargo"});
        e.j.anochece();
        e.j.entra("ciudad_este");
        e.visto({"b2_vigilante_charla", "b2_mansion", "b2_dentro_disfraz", "b2_invitados", "b2_sonsaca_exito"});
        e.ve({"bnd2_fiesta_nobles"});
        e.j.entra("interior_taberna");
        e.j.habla("venides");
        e.ve({"bnd2_informe_dado", "bnd_venides_10"});
        e.j.habla("venides");
        e.visto({"b2_venides_tras"});
        e.di({"Subir a dormir"});
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.fin();
    }

    {
        Escena e(g, "DIA 2: la fiesta, el cobrador y Edul", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Acercarte al cobrador", "¿Quién no paga en este barrio? [Persuasión]",
                  "Buscar a Edul para entrar como seguidor", "Hablarle de Venides", "No contarle nada"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.pon({"bnd1_aigren_se_va"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_herreria");
        e.j.habla("aigren");
        e.visto({"b2_aigren_se_va"});
        e.j.habla("edul");
        e.visto({"b2_edul_herreria"});
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.ve({"bnd2_skilla_encargo"});
        e.j.anochece();
        e.j.entra("ciudad_este");
        e.visto({"b2_cobrador", "b2_cobrador_libro"});
        e.j.entra("interior_taberna");
        e.j.habla("bordadora");
        e.visto({"b2_vecina"});
        e.j.habla("venides");
        e.nove({"bnd2_informe_dado"});
        e.fin();
    }

    {
        Escena e(g, "DIA 2: Aigren en la cripta, Edul en el yunque", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"¿Y Aigren?", "¿Qué hay en esos frascos?"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.pon({"bnd1_invitado_por_aigren"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_herreria");
        e.j.habla("aigren");
        e.nove({"bnd2_aigren_despedido"});
        e.j.habla("edul");
        e.visto({"b2_edul_herreria"});
        e.ve({"bnd2_edul_frascos", "bnd1_edul_mezcla"});
        e.fin();
    }

    {
        Escena e(g, "DIA 2: Aigren y Luisarda, segun el primer dia", Suerte::Buena, {"heroe_pueblo_humano"},
                 {}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.pon({"bnd1_aigren_redimido", "bnd1_luisarda_silencio"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_herreria");
        e.j.habla("aigren");
        e.visto({"b2_aigren_daga"});
        e.j.entra("interior_sastreria");
        e.j.habla("luisarda");
        e.visto({"b2_luisarda_pociones"});
        e.fin();
    }

    {
        Escena e(g, "DIA 2: Luisarda delatada", Suerte::Buena, {"heroe_pueblo_humano"},
                 {}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.pon({"bnd1_luisarda_delatada"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_sastreria");
        e.j.habla("luisarda");
        e.visto({"b2_luisarda_cerrada"});
        e.j.habla("sastre");
        e.visto({"b2_sastre_delatada"});
        e.fin();
    }

    {
        Escena e(g, "DIA 2: Xila en el escalon", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"¿Qué sabes hoy?", "De músico"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.pon({"bnd1_aliado_xila", "bnd1_xila_honor"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("ciudad_oeste");
        e.nove({"bnd2_tienda_vista"});
        e.j.habla("xila");
        e.visto({"b2_xila_escalon", "b2_tienda_con_pista", "b2_disfraz_hallado_xila"});
        e.ve({"bnd2_disfraz_musico"});
        e.j.entra("interior_taberna");
        e.di({"Subir a dormir"});
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.visto({"b2_cierre_xila"});
        e.fin();
    }

    {
        Escena e(g, "DIA 2: Xila en el escalon, sin pedirle nada", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Te veo bien."}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.pon({"bnd1_aliado_xila", "bnd1_xila_honor"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("ciudad_oeste");
        e.j.habla("xila");
        e.nove({"bnd2_tienda_vista"});
        e.j.entra("ciudad_oeste");
        e.visto({"b2_tienda_tras_xila"});
        e.fin();
    }
}

// El Ocaso: la Matanza, Verina, Edul, Sandes y la barca.
void escenasOcaso(const Guiones& g, std::vector<std::string>* vistos) {
    {
        Escena e(g, "OCASO: veinte hombres, Verina, Edul y los carros", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Correr a avisar a Venides", "Al oeste", "Vamos a por él.", "No.", "Liberarle"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.pon({"bnd_venides_20", "bnd2_aliado_gitanos", "bnd2_aliado_skilla", "bnd2_documentos", "bnd2_fiesta_nobles"});
        e.j.fase("ocaso");
        e.j.entra("ciudad_centro");
        e.j.entra("interior_cuartel");
        e.visto({"b3_sermon", "b3_campanas", "b3_venides_arenga_20", "b3_saqueadores_20", "b3_verina_salvada",
                 "b3_edul_con_verina", "b3_huida_gitanos", "b3_epilogo", "b3_barca"});
        e.ve({"bnd3_final_oeste", "bnd3_verina_con_grupo", "bnd3_edul_salvado", "bnd3_epilogo"});
        e.visto({"b3_epi_verina_viva"});
        e.fin();
    }

    {
        Escena e(g, "OCASO: Verina sola, Edul dentro", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Correr a avisar a Venides", "Al oeste", "No podemos. No hay tiempo.", "Dejarle"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.pon({"bnd_venides_20", "bnd2_documentos"});
        e.j.fase("ocaso");
        e.j.entra("ciudad_centro");
        e.j.entra("interior_cuartel");
        e.visto({"b3_epi_verina_sola"});
        e.nove({"bnd3_edul_salvado"});
        e.fin();
    }

    {
        Escena e(g, "OCASO: diez hombres, al norte", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Correr a avisar a Venides", "Al norte"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.pon({"bnd_venides_10", "bnd2_documentos"});
        e.j.fase("ocaso");
        e.j.entra("ciudad_centro");
        e.j.entra("interior_cuartel");
        e.visto({"b3_venides_arenga_10"});
        e.ve({"bnd3_ruta_norte", "bnd3_fuera", "bnd3_epilogo"});
        e.fin();
    }

    {
        Escena e(g, "OCASO: al norte, y no se llega", Suerte::Mala, {"heroe_pueblo_humano"},
                 {"Correr a avisar a Venides", "Al norte"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.pon({"bnd_venides_10", "bnd2_documentos"});
        e.j.fase("ocaso");
        e.j.entra("ciudad_centro");
        e.j.entra("interior_cuartel");
        e.ve({"bnd3_final_caido", "bnd3_epilogo"});
        e.fin();
    }

    {
        Escena e(g, "OCASO: quedarse a defender la base", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Correr a avisar a Venides", "Quedarse y luchar"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.pon({"bnd_venides_20", "bnd2_documentos"});
        e.j.fase("ocaso");
        e.j.entra("ciudad_centro");
        e.j.entra("interior_cuartel");
        e.visto({"b3_oleada"});
        e.ve({"bnd3_fuera", "bnd3_epilogo"});
        e.fin();
    }

    {
        Escena e(g, "OCASO: unirse a Sandes", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Correr a avisar a Venides", "Al oeste", "Seguir tu camino", "Acepto."}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.pon({"bnd_venides_20", "bnd2_documentos"});
        e.j.fase("ocaso");
        e.j.entra("ciudad_centro");
        e.j.entra("interior_cuartel");
        e.ve({"bnd3_aliado_sandes", "bnd3_fuera", "bnd3_epilogo"});
        e.fin();
    }

    {
        Escena e(g, "OCASO: entregarle Verina a Sandes", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Correr a avisar a Venides", "Al oeste", "Vamos a por él.", "Entregársela"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.pon({"bnd_venides_20", "bnd2_documentos"});
        e.j.fase("ocaso");
        e.j.entra("ciudad_centro");
        e.j.entra("interior_cuartel");
        e.ve({"bnd3_verina_entregada", "bnd3_fuera"});
        e.visto({"b3_epi_verina_muerta"});
        e.fin();
    }

    {
        Escena e(g, "OCASO: Verina, mirada y dejada", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Correr a avisar a Venides", "Al oeste", "Mirarla [Percepción]", "Seguir tu camino", "Dejarle"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.pon({"bnd2_documentos", "bnd2_venides_sin_aviso"});
        e.j.fase("ocaso");
        e.j.entra("ciudad_centro");
        e.j.entra("interior_cuartel");
        e.visto({"b3_venides_retenido", "b3_verina_reconocida", "b3_epi_verina_muerta"});
        e.ve({"bnd3_fuera"});
        e.fin();
    }

    {
        Escena e(g, "OCASO: Venides de rodillas, y le ayudas", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Correr a avisar a Venides", "Al oeste", "Ayudarle a escapar"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.pon({"bnd2_documentos", "bnd2_venides_sin_aviso"});
        e.j.fase("ocaso");
        e.j.entra("ciudad_centro");
        e.j.entra("interior_cuartel");
        e.visto({"b3_venides_retenido", "b3_venides_rodillas"});
        e.ve({"bnd3_fuera"});
        e.fin();
    }

    {
        Escena e(g, "OCASO: nadie avisa a Venides", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Esperar a que anochezca", "Al oeste", "Ayudarle a escapar"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.j.fase("ocaso");
        e.j.entra("ciudad_centro");
        e.visto({"b3_plaza_vacia"});
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.entra("interior_cuartel");
        e.ve({"bnd3_campanas", "bnd3_fuera", "bnd3_epilogo"});
        e.fin();
    }

    {
        Escena e(g, "OCASO: la barricada, por el rodeo", Suerte::Mala, {"heroe_pueblo_humano"},
                 {"Correr a avisar a Venides", "Al oeste", "Buscar dónde no hay frascos"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.pon({"bnd_venides_20", "bnd2_documentos"});
        e.j.fase("ocaso");
        e.j.pierde.insert("bnd3_rodeo_vencidos");
        e.j.entra("ciudad_centro");
        e.j.entra("interior_cuartel");
        e.ve({"bnd3_fuera"});
        e.fin();
    }

    {
        Escena e(g, "OCASO: la barricada, a golpes", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Correr a avisar a Venides", "Al oeste", "Derribarla a golpes", "Dejarle"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.pon({"bnd_venides_20", "bnd2_documentos"});
        e.j.fase("ocaso");
        e.j.suerte = Suerte::Buena;
        e.j.entra("ciudad_centro");
        e.j.entra("interior_cuartel");
        e.ve({"bnd3_edul_abandonado", "bnd3_fuera"});
        e.fin();
    }

    {
        Escena e(g, "OCASO: Pulga conoce un atajo", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Correr a avisar a Venides", "Al oeste", "Seguir a Pulga"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.pon({"bnd_venides_20", "bnd2_documentos", "bnd1_carterista_amigo"});
        e.j.fase("ocaso");
        e.j.entra("ciudad_centro");
        e.j.entra("interior_cuartel");
        e.visto({"b3_pulga"});
        e.ve({"bnd3_fuera"});
        e.fin();
    }

    {
        Escena e(g, "OCASO: la antigua compania de Rosita", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Correr a avisar a Venides", "Al oeste", "Vamos a por él.", "Ven con nosotros.", "No.", "Liberarle"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.pon({"bnd_venides_20", "bnd2_aliado_gitanos", "bnd2_aliado_skilla", "bnd2_rosita_semilla",
               "bnd1_xila_honor", "bnd1_aigren_redimido", "bnd2_guardia_preparada", "bnd2_documentos"});
        e.j.fase("ocaso");
        e.var("confianza_rosita", 2);
        e.j.pierde.insert("bnd3_sandes_vencido");
        e.j.entra("ciudad_centro");
        e.j.entra("interior_cuartel");
        e.visto({"b3_capitan_ronda", "b3_aigren_carro", "b3_xila_vendas", "b3_rosita_calle", "b3_compania",
                 "b3_sandes_vencido", "b3_epi_xila"});
        e.nove({"bnd3_compania_tuyo"});
        e.ve({"bnd3_con_xila", "bnd3_compania", "bnd3_final_oeste"});
        e.fin();
    }

    {
        Escena e(g, "OCASO: la compania, con un guerrero demoniaco", Suerte::Buena, {"heroe_pueblo_humano", "heroe_clase_guerrero_demoniaco"},
                 {"Correr a avisar a Venides", "Al oeste", "Vamos a por él.", "No.", "Liberarle"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.pon({"bnd_venides_20", "bnd2_aliado_gitanos", "bnd2_aliado_skilla", "bnd2_rosita_semilla",
               "bnd1_xila_honor", "bnd2_documentos"});
        e.j.fase("ocaso");
        e.var("confianza_rosita", 2);
        e.j.pierde.insert("bnd3_sandes_vencido");
        e.j.entra("ciudad_centro");
        e.j.entra("interior_cuartel");
        e.visto({"b3_compania", "b3_compania_de_los_tuyos"});
        e.fin();
    }

    {
        Escena e(g, "OCASO: la compania llega a mitad del combate", Suerte::Buena, {"heroe_pueblo_humano", "heroe_clase_guerrero_demoniaco"},
                 {"Correr a avisar a Venides", "Al oeste", "Vamos a por él.", "Ven con nosotros.", "No.", "Liberarle"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.pon({"bnd_venides_20", "bnd2_aliado_gitanos", "bnd2_aliado_skilla", "bnd2_rosita_semilla",
               "bnd1_xila_honor", "bnd1_aigren_redimido", "bnd2_guardia_preparada", "bnd2_documentos"});
        e.j.fase("ocaso");
        e.var("confianza_rosita", 2);
        e.j.entra("ciudad_centro");
        e.j.entra("interior_cuartel");
        e.visto({"b3_sandes_vencido"});
        e.ve({"bnd3_compania", "bnd3_compania_vista", "bnd3_compania_tuyo", "bnd3_sandes_vencido"});
        e.nove({"bnd3_sandes_gana"});
        e.fin();
    }

    {
        Escena e(g, "OCASO: sin la compania, Sandes gana", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Correr a avisar a Venides", "Al oeste", "Vamos a por él.", "No.", "Liberarle"}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.pon({"bnd_venides_20", "bnd2_aliado_gitanos", "bnd2_aliado_skilla", "bnd1_xila_honor", "bnd2_documentos"});
        e.j.fase("ocaso");
        e.var("confianza_rosita", 2);
        e.j.pierde.insert("bnd3_sandes_vencido");
        e.j.entra("ciudad_centro");
        e.j.entra("interior_cuartel");
        e.nove({"bnd3_compania"});
        e.ve({"bnd3_sandes_gana", "bnd3_fuera", "bnd3_epilogo"});
        e.fin();
    }

    {
        Escena e(g, "OCASO: Edul, con la furia de Sandes", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Correr a avisar a Venides", "Al oeste", "Seguir tu camino", "Acepto."}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.pon({"bnd_venides_20", "bnd2_documentos"});
        e.j.fase("ocaso");
        e.j.entra("ciudad_centro");
        e.j.entra("interior_cuartel");
        e.visto({"b3_edul_furia"});
        e.ve({"bnd3_fuera"});
        e.fin();
    }

    {
        Escena e(g, "OCASO: Edul sabe lo de Venides", Suerte::Buena, {"heroe_pueblo_humano"},
                 {"Correr a avisar a Venides", "Al oeste", "No podemos. No hay tiempo.", "No.", "Liberarle",
                  "Es verdad."}, vistos);
        e.di({"Pagar la cama", "Subir a dormir"});
        e.j.prologo();
        e.j.fase("dia1");
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd_dia_cerrado"});
        e.j.fase("dia2");
        e.j.drena();
        e.j.entra("interior_taberna");
        e.j.habla("skilla");
        e.j.habla("skilla");
        e.ve({"bnd2_dia_cerrado"});
        e.pon({"bnd_venides_20", "bnd2_documentos", "bnd2_edul_sabe_venides"});
        e.j.fase("ocaso");
        e.j.entra("ciudad_centro");
        e.j.entra("interior_cuartel");
        e.visto({"b3_edul_fiesta"});
        e.ve({"bnd3_edul_salvado"});
        e.fin();
    }
}
