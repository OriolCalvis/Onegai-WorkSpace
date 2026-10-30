// H2-06 — Lo que una carta HACE EN COMBATE, leido de su texto para la maquina.
//
// Las cartas de clase llevan, como las condiciones de conditions.json, una
// lista effects[] de {frase de la carta, mechanicHook}. El cargador los deja
// crudos (SkillDefinition/PassiveDefinition/ConditionDefinition::
// mechanicHooks); aqui se traducen a algo que el combate puede ejecutar. Es la
// pieza «la carta dice -> el motor hace» de DEC-02 («una habilidad hace
// exactamente lo que dice la carta»), para lo que piden las cuatro clases de
// CLASES-03 que Oriol pidio jugables (05/10): Bardo de las Cuatro Estaciones,
// El Apostata, Senor de las Armas y Furia de los Creadores.
//
// GRAMATICA (verbo:parametros, separados por ':'). Al JUGAR la carta:
//   dano:<dados|escudo>          cura:<dados|dano_causado>
//   ataque_arma[:x<N>|:critico|:ventaja]      ataques_arma:<N>
//   estado:<rondas|combate>:<rasgo...>        condicion:<id>[:<rondas>]
//   escudo:<por_golpe>:<maximo>               turno_extra
//   devuelve_carta:<etiqueta>                 invoca:<rondas>:<id>[,<id>...]
//   bloquea:ataque | bloquea:ataques:<rondas> dados_extra:<N>   revive:<PV>
//   defiende_aliado:<rondas>                  aliado_ataca:sin_dados
//   pendiente:<que falta>  pendiente_parcial:<que falta>  requiere_posiciones
//   coste_desgarro:<N>
//   propio:<rondas|combate>:<rasgo...>  (SP-02: quien juega la carta gana el
//                                        estado, apunte adonde apunte)
//   limpia                              (SP-02: quita los estados en contra de
//                                        los aliados a los que apunta)
// En una PASIVA:  elige_una:<op>,<op>...   si:<op>[@<rondas>]:<rasgo...>
//                 recupera_carta:<N>       siempre:<rasgo...> (SP-02: sin elegir)
//                 azar_una:<op>,<op>...    (CP-01: como elige_una, pero la
//                                          opcion la saca la suerte: el tarot)
// Un RASGO (lo que hace un estado mientras dura; tambien los de las
// condiciones del catalogo): desventaja:<todo|ataque|ataque_y_habilidad|
// defensa|salvacion_des_con>, ventaja:<ataque|defensa>, ventaja_contra:<todo|
// melee|origen>, bono_ca:<N>, penal_ca:<N>, incapacitado, critico_contra,
// dano_por_turno:<dados>, absorbe_por_turno:<dados>, bono_dano_arma:<dados>,
// bono_dano_contra:<dados>, bono_dano_melee:<dados>, refleja:fisico,
// provocado:origen, desarmado, bono_cura:<N>, bloquea:vision,
// penal_dados:<N>. Lo que el combate no simula (velocidad, vision fuera de
// combate, interacciones sociales...) se reconoce como Ignorado.
//
// GL-free y sin estado.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace RPG {
class RandomEngine;
}

namespace Game {

// "1d6", "3d6", "2": cuantos dados de cuantas caras, o un numero fijo.
struct Dados {
    int cantidad = 0;
    int caras = 6;
    int fijo = 0;
    bool vacio() const { return cantidad <= 0 && fijo <= 0; }
    int tirar(RPG::RandomEngine& rng) const;
    std::string texto() const;
};
Dados leerDados(const std::string& texto);

struct Gancho {
    enum class Tipo : std::uint8_t {
        Desconocido,
        // Al jugar la carta
        Dano, DanoEscudo, Cura, CuraDanoCausado, AtaqueArma, AtaquesArma, Estado, Condicion,
        Escudo, TurnoExtra, DevuelveCarta, Invoca, Bloquea, BloqueaTodos, DadosExtra, Revive,
        DefiendeAliado, AliadoAtaca, Pendiente, PendienteParcial, RequierePosiciones,
        CosteDesgarro, Propio, Limpia,
        // Pasivas
        EligeUna, Si, RecuperaCarta, Siempre, AzarUna,
        // Rasgos
        DesventajaTodo, DesventajaAtaque, DesventajaDefensa, DesventajaDefensaFisica,
        VentajaAtaque, VentajaDefensa, VentajaContraTodo, VentajaContraMelee,
        VentajaContraOrigen, BonoCa, Incapacitado, CriticoContra, DanoPorTurno,
        AbsorbePorTurno, BonoDanoArma, BonoDanoContra, BonoDanoMelee, Refleja, Provocado,
        Desarmado, BonoCura, PenalDados, VelocidadMitad, VelocidadCero, Silenciado, Ignorado
    };
    Tipo tipo = Tipo::Desconocido;
    Dados dados;
    int numero = 0;  // bono_ca, por golpe, veces, multiplicador, PV, dados extra, coste
    int maximo = 0;  // tope del escudo
    int rondas = 0;  // -1 = hasta el final del combate
    std::string texto;                  // id de condicion, etiqueta, opcion, variante
    std::vector<std::string> opciones;  // elige_una, invoca
    std::vector<Gancho> rasgo;          // estado:/si: llevan dentro UN rasgo
    std::string crudo;                  // el gancho tal cual (log, diagnostico)

    bool esRasgo() const;
};

Gancho leerGancho(const std::string& crudo);
std::vector<Gancho> leerGanchos(const std::vector<std::string>& crudos);

// Lo que se ve en el menu de la carta que no se puede jugar: "" si se puede.
// "[pendiente] ..." o "necesita posiciones en el combate".
std::string motivoBloqueo(const std::vector<Gancho>& ganchos);

// Si un rasgo va en contra de quien lo lleva (desventajas, incapacitado,
// penal_ca...) o a su favor (ventajas, bono_ca, bonos de dano...). Lo usan el
// icono del estado (H2-03) y «limpia» (SP-02).
bool esPerjuicio(const Gancho& rasgo);

}  // namespace Game
