// demo_campana_boundington — "Los Perdidos de Boundington" de punta a punta.
//
// Las cuatro aventuras (prologo + tres dias) jugadas SEGUIDAS, con UN SOLO
// NarrativeState. Eso es lo que las convierte en una campana y no en cuatro
// demos sueltas: las flags del prologo siguen encendidas el tercer dia, y por
// eso Venides sabe que tu tambien has visto la barca, y por eso la antigua
// compania de Rosita puede aparecer en Las Afueras.
//
// EL PATRON, que es lo unico que hay que saber para anadir mas dias:
//
//     NarrativeState estado;                  // uno, para toda la campana
//     for (cada guion en orden) {
//         motor.setAdventure(&guion);         // el motor cambia de libreto
//         ... jugar ...                       // el estado NO se toca
//     }
//
// DIALOGOS-01 — Con los dialogos nuevos casi todo es una CHARLA: temas que se
// preguntan en el orden que se quiera, paginas («Hablar de otra cosa») y una
// salida. Por eso las partidas de aqui ya no son una lista de clics: son un
// JUGADOR que, cada vez que tiene que contestar, coge la primera respuesta de
// SU lista que este en pantalla (y no haya usado ya en esa misma pantalla);
// si ninguna esta, pasa de pagina; y si ya las ha visto todas, se despide.
// Asi una ruta es «que haces» (donde vas, con quien hablas) y «que dices» (lo
// que prefieres contestar), y aguanta que se anadan temas a una charla.
//
// La cobertura de TODOS los beats (y los atascos, los finales y la moral) la
// mide explorar_boundington, que juega cientos de partidas al azar con
// heroes de la creacion de personaje. Este demo prueba las RUTAS con nombre:
// que lo que el canon dice que pasa, pasa.
//
// GL-free: no abre ventana, no necesita GameSession. Las tiradas Nd6 y los
// combates que piden los beats se resuelven de forma determinista (ver
// Jugador::procesa, en demo_boundington_partida.h), que es lo que hace
// GameSession pero sin dados ni BattleState: a este demo le interesa la
// RAMIFICACION, no el azar.

#include <cstdio>
#include <string>
#include <vector>

#include "demo_boundington_partida.h"

using namespace demo_bnd;

namespace {

// Ruta A — el heroe que lo hace todo: investiga, ayuda a todo el mundo,
// devuelve la espada y salva a Verina y a Edul.
const std::vector<std::string> kHeroe = {
    // prologo
    "Mirar alrededor [Percepción]", "Observarle [Conocimiento arcano]", "Callar y escuchar",
    "¿Qué pasó en las montañas?",
    // noche 0 y desayuno
    "Pagar la cama", "Subir a dormir", "Anoche soñé con una barca.",
    // primer dia
    "Mostrar interés [Persuasión]", "Preguntarle por los Perdidos", "Convencerle de que son el mal [Persuasión]",
    "Busco antorchas.", "Volver a sacar las antorchas", "Insistir, de malos modos [Intimidación]",
    "Escuchar lo que te pide", "No diré nada. Tranquila.", "Sentarte con él, sin decir nada",
    "Invitarle a desayunar [Persuasión]", "Hablar de lo que has averiguado", "Contárselo todo",
    "Separarlos [pelea sin matar]", "Hacerle hablar", "¿Quién te manda?",
    // segundo dia (la montaña de Gorran)
    "Aceptar el encargo", "Pagar el paso (5)", "Cogerla sin que te oigan [Sigilo]",
    "Suelta a los presos y estamos en paz.",
    // tercer dia (boundington_segundo_dia); los textos, tal cual estan en el JSON
    "Devolvérsela y contarle todo", "¿Qué fue de los otros tres?", "Vengo con el caballero Venides.",
    "Ir con Venides a ver al capitan", "Soy el que te trajo al camorrista anoche.",
    "De noble excéntrico de Ascaria", "Vengo a preguntar si necesitáis algo", "Preguntarle qué necesitan",
    "Me quedo hasta que vengan.", "Plantarte delante.", "Hablar con el vigilante",
    "Convencerle de que te deje pasar [Persuasion]", "Entrar por la puerta, con disfraz [Interpretacion]",
    "Mezclarte con los invitados", "Prefiero hacerle preguntas", "Sonsacar al noble que brinda [Engano]",
    "Senalarselo a Venides", "Contarselo todo", "Subir a dormir",
    // el Ocaso
    "Correr a avisar a Venides", "Al oeste", "Vamos a por el.", "Ven con nosotros.", "Liberarle",
};

void juegaHeroe(Jugador& j) {
    j.prefiere = kHeroe;
    j.prologo();

    j.fase("dia1");
    j.entra("interior_taberna");     // llegas de noche: la primera noche en la taberna
    j.habla("skilla");               // la cama, a dormir y el desayuno con Venides
    j.entra("ciudad_centro");
    j.habla("sectario_perdido");     // los sectarios de la plaza
    j.entra("interior_herreria");
    j.habla("aigren");               // los Perdidos -> convencerle
    j.entra("interior_sastreria");
    j.habla("luisarda");             // las antorchas -> la sala -> silencio
    j.entra("ciudad_oeste");
    j.habla("xila");                 // el desconocido que bebe -> Xila sobrio
    j.entra("interior_iglesia");     // la pista de Xila lleva a la iglesia
    j.entra("interior_taberna");
    j.anochece();
    j.habla("venides");              // borracho: el informe -> veinte hombres
    j.duerme();                 // a dormir: la pelea de la taberna
    require(j.s.hasFlag("bnd_dia_cerrado"));

    // GORRAN-DIA2 (22/10): el dia 2 es la montaña de Gorran y el antiguo segundo dia es ya el 3.
    j.fase("dia_gorran");
    j.drena();                       // amanece el segundo dia
    j.laEspadaDeRosita();            // el encargo, la Gola Fosca, el pedestal: Gorran, vencido y perdonado
    j.entra("interior_taberna");     // ... y a la taberna con la Furia
    j.habla("skilla");               // a dormir
    // (la Furia solo se consigue con buena suerte; con mala, Gorran gana y se cierra la cueva)
    require(j.s.hasFlag("bndg_dia_cerrado"));

    j.fase("dia2");
    j.drena();                       // amanece el tercero
    j.habla("skilla");               // la espada devuelta -> Rosita cuenta
    j.entra("interior_castillo");    // la puerta, Venides y el capitan
    j.entra("interior_castillo_p1");
    j.habla("capitan_guardia");      // Venides responde por ti
    j.habla("capitan_guardia");      // ... y la guardia queda preparada
    j.entra("ciudad_oeste");
    j.entra("tienda_abandonada");    // el disfraz
    j.entra("ciudad_surysal");
    j.habla("viejo_surysal");        // la Barriada: Zarela, los saqueadores, el ritual
    j.anochece();
    j.entra("ciudad_este");          // la fiesta de los nobles: el vigilante, la mansion
    j.entra("mansion_barrio_alto");
    j.habla("seguidor_brinda");
    j.habla("seguidor_brinda");      // los invitados: el noble que brinda
    j.entra("interior_taberna");
    j.habla("venides");              // el informe de la noche
    j.duermeDia3();                  // a dormir
    require(j.s.hasFlag("bnd2_dia_cerrado"));

    j.fase("ocaso");
    j.ocasoBase();                   // el sermon de Sandes, correr a avisar, las campanas, Venides
    j.zonasPrimero = {"edul_casco_retenido"};  // al heroe le da tiempo a liberar a Edul antes del final
    j.ocasoRecorrido();              // la salida oeste, Verina, Sandes, Edul, Las Afueras, la huida y la barca
    require(j.s.hasFlag("bnd3_fuera"));
    require(j.s.hasFlag("bnd3_epilogo"));
}

// Ruta B — el que va a lo justo: cama, desayuno, lo que oyes, a dormir.
const std::vector<std::string> kJusto = {
    "Gritar «¿Hola?»", "«¿Quién eres?»", "Callar y escuchar", "Pagar la cama", "Subir a dormir",
    "¿Qué sabes de los Perdidos?", "Hablar de lo que has averiguado", "Contarle lo que tienes",
    "Quedarte mirando",
    // tercer dia: la puerta del Barrio Alto se paga y se escucha desde el jardin (los textos, como en el JSON)
    "Acercarte al cobrador", "Pagar (20)", "Escuchar desde el jardin [Percepcion]", "Contarselo todo",
    // el Ocaso
    "Correr a avisar a Venides", "Quedarse y luchar",
};

void juegaJusto(Jugador& j) {
    j.prefiere = kJusto;
    j.prologo();

    j.fase("dia1");
    j.entra("interior_taberna");
    j.habla("skilla");
    j.anochece();
    j.habla("venides");
    j.duerme();
    require(j.s.hasFlag("bnd_dia_cerrado"));

    // El que va a lo justo: el dia de Gorran, a oscuras, y un tercero sin tocar nada.
    j.pasaElDiaDeGorran();
    j.entra("interior_castillo");
    j.entra("interior_taberna");
    j.anochece();
    j.entra("ciudad_este");
    j.entra("interior_taberna");
    j.habla("venides");
    j.duermeDia3();
    require(j.s.hasFlag("bnd2_dia_cerrado"));

    j.fase("ocaso");
    j.ocasoBase();
    j.ocasoRecorrido();              // la salida oeste y lo que va saliendo hasta la huida
    require(j.s.hasFlag("bnd3_fuera"));
    require(j.s.hasFlag("bnd3_epilogo"));
}

void cuenta(const NarrativeState& s, const char* que) {
    std::printf("    %-24s %s\n", que, s.hasFlag(que) ? "si" : "no");
}

}  // namespace

int main() {
    const Guiones g = cargaTodo();
    std::vector<std::string> vistos;  // cobertura: los beats que disparan las rutas
    std::printf("=== Los Perdidos de Boundington — campana completa ===\n");
    std::printf("    %d + %d + %d + %d + %d beats en cinco aventuras (prologo, dia 1, Gorran, dia 3, Ocaso)\n",
                static_cast<int>(g.prologo.beats.size()), static_cast<int>(g.dia1.beats.size()),
                static_cast<int>(g.gorran.beats.size()), static_cast<int>(g.dia2.beats.size()),
                static_cast<int>(g.ocaso.beats.size()));

    // ---------------------------------------------------------------
    // Partida A — el que lo hace todo bien. Pierde la primera pelea con
    // Sandes a proposito: es justo cuando llega la antigua compania de Rosita
    // (Oriol: si salvaste a Edul, Verina, Xila y Venides, peleas con Sandes y
    // Rosita confia en ti dos veces).
    // ---------------------------------------------------------------
    std::printf("\n########## PARTIDA A: el heroe que lo hace todo ##########\n");
    {
        Jugador j(g, Suerte::Buena, {"heroe_pueblo_humano", "heroe_pueblo_de_aegroum", "heroe_rol_tank"}, true);
        j.pierde.insert("bnd3_sandes_vencido");
        j.anotador = &vistos;
        juegaHeroe(j);
        const NarrativeState& s = j.s;
        for (const char* f : {"bnd_venides_reconocido", "bnd_venides_20", "bnd2_furia_alaro", "bnd2_aliado_gitanos",
                              "bnd2_rosita_semilla"}) {
            cuenta(s, f);
            require(s.hasFlag(f));
        }
        require(s.variable("confianza_rosita") >= 2);
        for (const char* f : {"bnd3_verina_con_grupo", "bnd3_edul_salvado", "bnd3_con_xila", "bnd3_compania",
                              "bnd3_final_oeste"}) {
            cuenta(s, f);
            require(s.hasFlag(f));
        }
        std::printf("  -> barca compartida, 20 hombres, espada, gitanos, Verina y Edul vivos,\n"
                    "     la compania de Rosita en Las Afueras y salida por el oeste. OK\n");
    }

    // ---------------------------------------------------------------
    // Partida B — el que va a lo justo y le sale a medias.
    // ---------------------------------------------------------------
    std::printf("\n########## PARTIDA B: el que va a lo justo ##########\n");
    {
        Jugador j(g, Suerte::Regular, {"heroe_pueblo_humano", "heroe_rol_agile"});
        j.anotador = &vistos;
        juegaJusto(j);
        const NarrativeState& s = j.s;
        for (const char* f : {"bnd_venides_10", "bnd_venides_20", "bnd2_aliado_gitanos", "bnd3_compania",
                              "bnd3_final_oeste", "bnd3_final_caido"})
            cuenta(s, f);
        require(s.hasFlag("bnd_venides_10"));        // solo diez hombres, y el tercer dia
        require(!s.hasFlag("bnd_venides_20"));
        require(!s.hasFlag("bnd2_aliado_gitanos"));  // nadie le debe nada
        require(!s.hasFlag("bnd3_compania"));
        require(!s.hasFlag("bnd3_final_oeste"));  // sale sin ayuda o cae
        std::printf("  -> 10 hombres, sin aliados, final distinto. OK\n");
    }

    // ---------------------------------------------------------------
    // Lo que NO puede cambiar: la ciudad cae siempre. Es canon
    // (EV-1981-BOUNDINGTON) y ninguna ruta debe poder evitarlo. Y toda
    // partida acaba en la barca, viva o no.
    // ---------------------------------------------------------------
    std::printf("\n########## invariante: Boundington cae en todas ##########\n");
    {
        for (Suerte suerte : {Suerte::Critica, Suerte::Buena, Suerte::Regular, Suerte::Mala}) {
            for (bool heroe : {true, false}) {
                Jugador j(g, suerte, {"heroe_pueblo_humano", heroe ? "heroe_rol_tank" : "heroe_rol_caster"});
                j.anotador = &vistos;
                if (heroe) juegaHeroe(j); else juegaJusto(j);
                require(j.s.hasFlag("bnd3_campanas"));  // la Matanza ocurre
                require(j.s.hasFlag("bnd3_epilogo"));   // y la campana cierra
                std::string fin = "?";
                for (const char* f : {"bnd3_final_oeste", "bnd3_final_solos", "bnd3_final_caido"})
                    if (j.s.hasFlag(f)) fin = f + 11;
                std::printf("  %-8s %-6s -> final %s\n", nombre(suerte), heroe ? "heroe" : "justo", fin.c_str());
            }
        }
        std::printf("  -> las 8 combinaciones: la ciudad arde (campanas) y la campana cierra. OK\n");
    }

    // ---------------------------------------------------------------
    // Una rama que una partida con la suerte uniforme no toma: GANAR los
    // dos combates del Ocaso y PERDER el de Verina. Se prueba aparte,
    // montando el estado a mano.
    // ---------------------------------------------------------------
    {
        Jugador j(g, Suerte::Buena);
        j.anotador = &vistos;
        j.fase("ocaso");
        j.prefiere = {"Al oeste"};
        j.pierde.insert("bnd3_verina_libre");
        for (const char* f : {"bnd2_dia_cerrado", "bnd_prologo_pasado", "bnd3_plaza_vista", "bnd3_avisado",
                              "bnd3_campanas", "bnd3_arenga", "bnd3_con_venides"})
            j.s.setFlag(f);
        j.drena();
        j.entra("interior_cuartel");  // b3_por_donde: la salida oeste, el paso norte o mantener la puerta
        j.ocasoRecorrido();           // la salida oeste, la barricada, Verina, Edul, el final
        require(j.s.hasFlag("bnd3_verina_muerta"));
        require(j.s.hasFlag("bnd3_fuera"));
        std::printf("\n  Verina perdida en la barricada: la partida sigue y termina. OK\n");
    }

    // ---------------------------------------------------------------
    // Cobertura de estas rutas (orientativa: la de verdad es la de
    // explorar_boundington). Lo que si se exige: los momentos clave de la
    // historia salen en alguna de ellas.
    // ---------------------------------------------------------------
    std::printf("\n########## cobertura de las rutas ##########\n");
    {
        int total = 0;
        int n = 0;
        for (const AdventureScript* a : {&g.prologo, &g.dia1, &g.gorran, &g.dia2, &g.ocaso}) {
            for (const auto& b : a->beats) {
                ++total;
                for (const auto& v : vistos)
                    if (v == b.id) { ++n; break; }
            }
        }
        std::printf("  beats disparados por las rutas: %d de %d (el resto, explorar_boundington)\n", n, total);
        for (const char* clave : {"p_vision", "b0_llegada", "b0_rosita", "b1_desayuno_s1_a", "b1_secta",
                                  "b1_aigren_perdidos", "b1_sala", "b1_xila", "b1_borracho",
                                  "b1_venides_informe_bastan", "b1_venides_informe", "cierre_creadores",
                                  "cierre_espera", "b2_capitan", "b2_gorran_victoria", "b2_zarela_pide",
                                  "b2_rosita_revela", "b2_mansion", "b2_venides_hombres", "b3_sermon",
                                  "b3_por_donde", "b3_verina_salvada", "b3_rosita_calle", "b3_edul",
                                  "b3_compania", "b3_huida_gitanos", "b3_verina_perdida", "b3_caida",
                                  "b3_epilogo", "b3_barca"}) {
            bool visto = false;
            for (const auto& v : vistos)
                if (v == clave) { visto = true; break; }
            if (!visto) std::printf("    !! sin disparar: %s\n", clave);
            require(visto);
        }
        std::printf("  -> los 30 momentos clave salen. OK\n");
    }

    std::printf("\ntodas las comprobaciones han pasado\n");
    return 0;
}
