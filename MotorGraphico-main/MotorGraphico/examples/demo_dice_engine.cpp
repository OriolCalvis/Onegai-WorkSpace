// Test GL-free del motor de tiradas Nd6 (RPG/DicePoolEngine.h) y de la
// RandomEngine inyectable. Sin ventana, sin contexto GL, sin assert():
// require() de examples/Check.h (ver ARCHITECTURE.md, regla 5).
//
// Complementa a demo_nd6_distribution, que NO es un test: imprime
// porcentajes sobre 20.000 tiradas y siempre devuelve 0. Sirve para mirar
// si la curva tiene buena pinta, pero no puede fallar, asi que no protege
// de nada. Aqui las tiradas son GUIONIZADAS (ScriptedRng): se fija dado a
// dado lo que sale y se comprueba el resultado exacto.
//
// Ed. 3 (bitacora 2.17): toda resolucion es enfrentada y la pifia
// exige ningun 5/6 y al menos dos 1es. Estos tests fijan ese contrato.

#include <cstdio>
#include <vector>

#include "Core/Check.h"
#include "ScriptedRng.h"
#include "RPG/DicePoolEngine.h"

using namespace RPG;

namespace {

const char* nombre(Degree d) {
    switch (d) {
        case Degree::BOTCH:    return "BOTCH";
        case Degree::PARTIAL:  return "PARTIAL";
        case Degree::SUCCESS:  return "SUCCESS";
        case Degree::CRITICAL: return "CRITICAL";
    }
    return "?";
}

// Los exitos son multiplos de 0.5 representados en float: comparar con ==
// funciona (0.5 y 1.0 son exactos en binario), pero se compara con
// tolerancia para que el test no sea fragil si algun dia se acumula de
// otra forma.
bool casi(float a, float b) {
    const float d = a - b;
    return d < 0.001f && d > -0.001f;
}

// ---------------------------------------------------------------
// 1. Conteo del pool: 6 = 1 exito, 5 = medio, 1-4 = nada.
// ---------------------------------------------------------------
void testConteoBase() {
    ScriptedRng rng({6, 5, 4, 3, 2, 1});
    PoolResult p = DicePoolEngine::roll_pool(6, DiceMod::NORMAL, rng);

    require(p.rolls.size() == 6);
    require(casi(p.successes, 1.5f));   // un 6 (1.0) + un 5 (0.5)
    require(p.critical == false);
    require(p.botch == false);
    require(rng.consumed() == 6);       // tira N dados, ni uno mas

    // Se guardan los valores reales, en orden: el log de combate y el HUD
    // los enseñan, y una tirada que no se puede ver no se puede discutir.
    require(p.rolls[0] == 6 && p.rolls[5] == 1);
}

// ---------------------------------------------------------------
// 2. Ventaja y desventaja.
// ---------------------------------------------------------------
void testVentajaYDesventaja() {
    // Con NORMAL el 4 no vale nada; con ADVANTAGE vale medio.
    {
        ScriptedRng a({4, 4, 4});
        require(casi(DicePoolEngine::roll_pool(3, DiceMod::NORMAL, a).successes, 0.0f));
        ScriptedRng b({4, 4, 4});
        require(casi(DicePoolEngine::roll_pool(3, DiceMod::ADVANTAGE, b).successes, 1.5f));
    }
    // Con DISADVANTAGE el 5 deja de valer medio; el 6 sigue valiendo 1.
    {
        ScriptedRng a({5, 5, 5});
        require(casi(DicePoolEngine::roll_pool(3, DiceMod::NORMAL, a).successes, 1.5f));
        ScriptedRng b({5, 5, 5});
        PoolResult p = DicePoolEngine::roll_pool(3, DiceMod::DISADVANTAGE, b);
        require(casi(p.successes, 0.0f));
        require(p.botch == false);      // Ed. 3: con 5s no hay pifia, aunque
                                        // los exitos sean 0 (ver testPifiaEd3)
        ScriptedRng c({6, 5, 5});
        require(casi(DicePoolEngine::roll_pool(3, DiceMod::DISADVANTAGE, c).successes, 1.0f));
    }
}

// ---------------------------------------------------------------
// 3. Critico = al menos DOS 6 (D0013, Oriol 08/10/2026). Con un dado, nunca.
//
// No basta con "muchos exitos": con ventaja un pool con un solo 6 puede
// llegar a la misma cifra sumando medios, y eso NO es un critico.
// ---------------------------------------------------------------
void testCritico() {
    {
        ScriptedRng rng({6, 6, 6});
        PoolResult p = DicePoolEngine::roll_pool(3, DiceMod::NORMAL, rng);
        require(casi(p.successes, 3.0f));
        require(p.critical == true);
    }
    {
        // Falso positivo que hay que evitar: con ventaja, 4+4+5+6 da
        // 2.5 exitos con un solo 6. No es critico.
        ScriptedRng rng({4, 4, 5, 6});
        PoolResult p = DicePoolEngine::roll_pool(4, DiceMod::ADVANTAGE, rng);
        require(casi(p.successes, 2.5f));
        require(p.critical == false);   // un solo 6
    }
    {
        // Y dos 6 entre cuatro dados SI lo son (D0013), aunque los otros
        // dos no sean 6.
        ScriptedRng rng({4, 4, 6, 6});
        PoolResult p = DicePoolEngine::roll_pool(4, DiceMod::ADVANTAGE, rng);
        require(casi(p.successes, 3.0f));
        require(p.critical == true);
    }
}

// ---------------------------------------------------------------
// 4. Pool vacio: quedarse sin dados es un resultado legitimo del juego
//    (condiciones que restan dados). Ed. 3: NO es pifia (la pifia
//    exige dados: ningun 5/6 y minimo dos 1es) — es una tirada que no
//    suma nada y perdera la enfrentada por comparacion.
// ---------------------------------------------------------------
void testSinDados() {
    ScriptedRng rng({6, 6, 6});
    PoolResult p = DicePoolEngine::roll_pool(0, DiceMod::NORMAL, rng);
    require(p.rolls.empty());
    require(casi(p.successes, 0.0f));
    require(p.botch == false);
    require(rng.consumed() == 0);       // no toca el RNG

    // Negativo se trata igual que cero, no revienta ni tira dados.
    ScriptedRng rng2({6});
    PoolResult n = DicePoolEngine::roll_pool(-5, DiceMod::NORMAL, rng2);
    require(n.botch == false);
    require(rng2.consumed() == 0);
}

// ---------------------------------------------------------------
// 5. Techo de 24 dados: sin el, un pool inflado por bonos encadenados
//    haria una tirada arbitrariamente larga.
// ---------------------------------------------------------------
void testTecho() {
    ScriptedRng rng({1});
    PoolResult p = DicePoolEngine::roll_pool(1000, DiceMod::NORMAL, rng);
    require(p.rolls.size() == 24);
    require(rng.consumed() == 24);
}

// ---------------------------------------------------------------
// Pools numericos para comprobar la comparacion de exitos, sin pifia
// ni todo-seises. Los grados reales se prueban debajo con caras de dados.
PoolResult pool(float exitos) {
    PoolResult p;
    p.successes = exitos;
    return p;
}

// ---------------------------------------------------------------
// 8. Tirada enfrentada. El empate se marca aparte, no se resuelve aqui.
// ---------------------------------------------------------------
void testEnfrentada() {
    OpposedOutcome gana = DicePoolEngine::resolve_opposed(pool(2.0f), pool(1.5f));
    require(gana.attacker_wins == true && gana.tie == false);

    OpposedOutcome pierde = DicePoolEngine::resolve_opposed(pool(1.0f), pool(2.5f));
    require(pierde.attacker_wins == false && pierde.tie == false);

    OpposedOutcome empate = DicePoolEngine::resolve_opposed(pool(1.5f), pool(1.5f));
    require(empate.attacker_wins == false && empate.tie == true);
    require(casi(empate.attacker_successes, 1.5f));
}

// ---------------------------------------------------------------
// 8b. Pifia Ed. 3 (bitacora 2.17): ningun 5 ni 6, y minimo dos 1es.
//     Ya no es «0 exitos».
// ---------------------------------------------------------------
void testPifiaEd3() {
    // Dos 1es sin ningun 5/6: pifia, aunque haya otros dados.
    { ScriptedRng r({1, 1});       require(DicePoolEngine::roll_pool(2, DiceMod::NORMAL, r).botch == true); }
    { ScriptedRng r({1, 1, 3});    require(DicePoolEngine::roll_pool(3, DiceMod::NORMAL, r).botch == true); }
    // 0 exitos pero un solo 1: no es pifia.
    { ScriptedRng r({1, 2, 3, 4}); require(DicePoolEngine::roll_pool(4, DiceMod::NORMAL, r).botch == false); }
    { ScriptedRng r({1});          require(DicePoolEngine::roll_pool(1, DiceMod::NORMAL, r).botch == false); }
    // Un 5 o un 6 desactivan la pifia aunque haya dos 1es.
    { ScriptedRng r({1, 1, 5});    require(DicePoolEngine::roll_pool(3, DiceMod::NORMAL, r).botch == false); }
    { ScriptedRng r({1, 1, 6});    require(DicePoolEngine::roll_pool(3, DiceMod::NORMAL, r).botch == false); }
}

// Tira los dados guionizados y devuelve el pool (azucar para Ed. 3).
PoolResult tira(std::initializer_list<int> caras) {
    ScriptedRng r(caras);
    return DicePoolEngine::roll_pool(static_cast<int>(caras.size()), DiceMod::NORMAL, r);
}

// ---------------------------------------------------------------
// 8c. Enfrentada Ed. 3: el contrato completo de opposed_degree.
// ---------------------------------------------------------------
void testEnfrentadaEd3() {
    // Gana por 0,5 -> SUCCESS («si gana por 0,5 tambien SUCCES»).
    require(DicePoolEngine::opposed_degree(tira({6, 5}), tira({6})) == Degree::SUCCESS);
    // Gana por mas margen -> SUCCESS igualmente.
    require(DicePoolEngine::opposed_degree(tira({6, 5, 5}), tira({6})) == Degree::SUCCESS);
    // Gana con todos los dados a 6 -> CRITICAL.
    require(DicePoolEngine::opposed_degree(tira({6, 6}), tira({6, 5})) == Degree::CRITICAL);
    // Empate -> PARTIAL: mitad de dano, sin debuff.
    require(DicePoolEngine::opposed_degree(tira({6, 5}), tira({5, 5, 5})) == Degree::PARTIAL);
    // Defensor gana (supera el hechizo) -> BOTCH: se salva del todo.
    require(DicePoolEngine::opposed_degree(tira({6}), tira({6, 6})) == Degree::BOTCH);
    // Pifia propia -> BOTCH aunque la comparacion estuviera ganada.
    require(DicePoolEngine::opposed_degree(tira({1, 1, 2}), tira({2, 3})) == Degree::BOTCH);
    // Pifia del defensor -> CRITICAL del atacante (confirmado por Oriol,
    // bitacora 2.17).
    require(DicePoolEngine::opposed_degree(tira({2, 3}), tira({1, 1})) == Degree::CRITICAL);
    // Las dos pifias: prevalece la propia.
    require(DicePoolEngine::opposed_degree(tira({1, 1}), tira({1, 1})) == Degree::BOTCH);
    // Dos pools sin exitos ni pifia empatan; cero dados tampoco inventa pifia.
    require(DicePoolEngine::opposed_degree(tira({1}), tira({2})) == Degree::PARTIAL);
    require(DicePoolEngine::opposed_degree(tira({}), tira({})) == Degree::PARTIAL);
    require(DicePoolEngine::opposed_degree(tira({}), tira({5})) == Degree::BOTCH);
    // Todo-seises no convierte una derrota o un empate en victoria.
    require(DicePoolEngine::opposed_degree(tira({6}), tira({6})) == Degree::PARTIAL);
    require(DicePoolEngine::opposed_degree(tira({6}), tira({6, 6})) == Degree::BOTCH);
}

// ---------------------------------------------------------------
// 9. Reproducibilidad: misma semilla, misma partida. Es la razon de que
//    RandomEngine se inyecte en vez de ser un singleton global.
// ---------------------------------------------------------------
void testReproducible() {
    Xoroshiro128p a(12345), b(12345), c(999);
    require(a.seed() == 12345);

    std::vector<int> sa, sb, sc;
    for (int i = 0; i < 200; ++i) { sa.push_back(a.roll_d6()); sb.push_back(b.roll_d6()); sc.push_back(c.roll_d6()); }
    require(sa == sb);          // misma semilla -> tirada identica
    require(sa != sc);          // otra semilla -> otra tirada

    // Y siguen siendo dados de seis caras, con las seis caras presentes.
    int vistos[7] = {0};
    for (int v : sa) { require(v >= 1 && v <= 6); vistos[v]++; }
    for (int cara = 1; cara <= 6; ++cara) require(vistos[cara] > 0);
}

}  // namespace

int main() {
    testConteoBase();
    testVentajaYDesventaja();
    testCritico();
    testSinDados();
    testTecho();
    testEnfrentada();
    testPifiaEd3();
    testEnfrentadaEd3();
    testReproducible();

    // Tabla de enfrentadas sin pifias ni todo-seises: empate = PARTIAL,
    // ganar = SUCCESS. Un margen grande por si solo no da CRITICAL.
    std::printf("grados por exitos (filas = atacante, columnas = defensor)\n");
    std::printf("       def 0.0    def 0.5    def 1.0    def 2.0    def 3.0\n");
    for (float e = 0.0f; e <= 3.0f; e += 0.5f) {
        std::printf("  %.1f ", e);
        for (float defender : {0.0f, 0.5f, 1.0f, 2.0f, 3.0f}) {
            std::printf("%9s ", nombre(DicePoolEngine::opposed_degree(pool(e), pool(defender))));
        }
        std::printf("\n");
    }

    std::printf("\ndemo_dice_engine: todas las comprobaciones han pasado\n");
    return 0;
}
