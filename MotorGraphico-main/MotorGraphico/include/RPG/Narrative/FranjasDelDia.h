#pragma once

// FASE1-BND-01 — Las franjas del día (fase 1 de «Los Perdidos de Boundington
// — revisión para jugarla entera»).
//
// En la Campaña 01 de Oriol las cosas pasan a una hora: Xila está sobrio «por
// la mañana», en Pico Dragón hay «un señor por las tardes», la pelea de
// taberna y el carterista son de noche. El tiempo de la historia son dos
// variables narrativas (VARIABLES-01): "dia" (1, 2, 3...) y "franja" (0 =
// mañana, 1 = tarde, 2 = noche). Un beat las pide como cualquier variable
// ({"franja": 0} es «por la mañana»; {"franja": {"max": 1}}, «de día») y las
// pasa con el efecto pasarFranja. No es el reloj de los temporizadores (esos
// cuentan segundos de exploración): esto avanza cuando la historia lo dice.
//
// GL-free y sin estado propio: solo cuentas y textos.

#include <string>

namespace RPG {

class NarrativeState;

namespace franjas {

inline constexpr const char* kVariableDia = "dia";
inline constexpr const char* kVariableFranja = "franja";
inline constexpr int kFranjasPorDia = 3;  // mañana, tarde, noche

// El momento de la historia. dia 0 = la aventura todavía no lleva la cuenta.
struct Momento {
    int dia = 0;
    int franja = 0;
};

// "manana" -> 0, "tarde" -> 1, "noche" -> 2 (como se escriben en el JSON, sin
// tildes, igual que los ids). Cualquier otra cosa, -1.
int indice(const std::string& nombre);
// 0 -> "manana"... (para escribir el JSON); fuera de rango, "".
const char* clave(int franja);
// Lo que se lee: 0 -> "Mañana", 1 -> "Tarde", 2 -> "Noche".
const char* nombre(int franja);

// El momento de una partida (las dos variables).
Momento momentoDe(const NarrativeState& estado);

// pasos franjas más tarde: después de la noche, la mañana del día siguiente.
// Si la cuenta no había empezado (dia 0), empieza en el día 1.
Momento pasa(Momento m, int pasos);
// La siguiente vez que sea esa franja (0 pasos si ya lo es): dormir de noche
// es pasar a la mañana del día siguiente.
Momento hasta(Momento m, int franja);

// «Día 2 · Tarde» para el HUD; "" si la cuenta no ha empezado.
std::string rotulo(Momento m);
// La línea del registro al cambiar: «Amanece el día 2.», «Llega la tarde.»,
// «Anochece.»; "" si no ha cambiado nada.
std::string lineaRegistro(Momento antes, Momento despues);

}  // namespace franjas
}  // namespace RPG
