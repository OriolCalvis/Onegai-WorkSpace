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
// Este fichero nacio cazando un fallo concreto: contra CD 0.0 -- que es
// la dificultad de la defensa 10, o sea el enemigo base de Tier I --
// resolve_against_cd devolvia SUCCESS con CERO exitos, porque comprobaba
// "exitos >= cd" antes que la pifia y 0 >= 0 es cierto. Un jugador que
// sacaba 1 en 1d6 acertaba igual. Ver el bloque "grados contra CD 0".

#include <cstdio>
#include <vector>

#include "Check.h"
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
        require(p.botch == true);       // tres cincos y aun asi pifia
        ScriptedRng c({6, 5, 5});
        require(casi(DicePoolEngine::roll_pool(3, DiceMod::DISADVANTAGE, c).successes, 1.0f));
    }
}

// ---------------------------------------------------------------
// 3. Critico = TODOS los dados sacan 6.
//
// No basta con "muchos exitos": con ventaja un pool sin ningun 6 puede
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
        // 6 exitos por acumulacion (seis 6... no: seis dados a 6 SI es
        // critico). Caso real de falso positivo: con ventaja, 4+4+6+6
        // da 3.0 exitos con solo dos seises.
        ScriptedRng rng({4, 4, 6, 6});
        PoolResult p = DicePoolEngine::roll_pool(4, DiceMod::ADVANTAGE, rng);
        require(casi(p.successes, 3.0f));
        require(p.critical == false);   // hay dos dados que no son 6
    }
}

// ---------------------------------------------------------------
// 4. Pool vacio: quedarse sin dados es un resultado legitimo del juego
//    (condiciones que restan dados), y debe ser pifia, no exito.
// ---------------------------------------------------------------
void testSinDados() {
    ScriptedRng rng({6, 6, 6});
    PoolResult p = DicePoolEngine::roll_pool(0, DiceMod::NORMAL, rng);
    require(p.rolls.empty());
    require(casi(p.successes, 0.0f));
    require(p.botch == true);
    require(rng.consumed() == 0);       // no toca el RNG

    // Negativo se trata igual que cero, no revienta ni tira dados.
    ScriptedRng rng2({6});
    PoolResult n = DicePoolEngine::roll_pool(-5, DiceMod::NORMAL, rng2);
    require(n.botch == true);
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
// 6. Defensa entera -> CD. Cada punto sobre 10 son +0.5, con suelo en 0
//    y techo en CD_MAX (si no, una defensa alta seria inalcanzable).
// ---------------------------------------------------------------
void testDefensaACd() {
    require(casi(DicePoolEngine::defense_to_cd(10), 0.0f));
    require(casi(DicePoolEngine::defense_to_cd(11), 0.5f));
    require(casi(DicePoolEngine::defense_to_cd(12), 1.0f));
    require(casi(DicePoolEngine::defense_to_cd(15), 2.5f));
    require(casi(DicePoolEngine::defense_to_cd(16), 3.0f));
    require(casi(DicePoolEngine::defense_to_cd(99), DicePoolEngine::CD_MAX));  // techo
    require(casi(DicePoolEngine::defense_to_cd(3), 0.0f));                     // suelo
}

// ---------------------------------------------------------------
// 7. Tabla de grados. El caso que dio origen a este fichero.
//
// CD 0.0 no es un caso raro: es exactamente lo que sale de la defensa 10,
// o sea un objetivo sin armadura y con DES 0 -- el enemigo tipico de
// Tier I -- y tambien el def_int por defecto de SkillExecutor cuando no
// hay objetivo. Si con 0 exitos ahi el grado sale SUCCESS, en la practica
// TODO ataque acierta en el tramo inicial del juego y la tirada no
// decide nada.
// ---------------------------------------------------------------
PoolResult pool(float exitos, bool critico = false) {
    PoolResult p;
    p.successes = exitos;
    p.critical = critico;
    p.botch = (exitos <= 0.0f);
    return p;
}

void testGrados() {
    // --- CD 0.0 (defensa 10) ---
    require(DicePoolEngine::resolve_against_cd(pool(0.0f), 0.0f).degree == Degree::BOTCH);
    require(DicePoolEngine::resolve_against_cd(pool(0.5f), 0.0f).degree == Degree::SUCCESS);
    require(DicePoolEngine::resolve_against_cd(pool(1.0f), 0.0f).degree == Degree::CRITICAL);

    // --- CD 1.0 (defensa 12) ---
    require(DicePoolEngine::resolve_against_cd(pool(0.0f), 1.0f).degree == Degree::BOTCH);
    require(DicePoolEngine::resolve_against_cd(pool(0.5f), 1.0f).degree == Degree::PARTIAL);
    require(DicePoolEngine::resolve_against_cd(pool(1.0f), 1.0f).degree == Degree::SUCCESS);
    require(DicePoolEngine::resolve_against_cd(pool(1.5f), 1.0f).degree == Degree::SUCCESS);
    require(DicePoolEngine::resolve_against_cd(pool(2.0f), 1.0f).degree == Degree::CRITICAL);

    // Todos 6 es critico aunque no llegue a cd+1.
    require(DicePoolEngine::resolve_against_cd(pool(1.0f, true), 3.0f).degree == Degree::CRITICAL);

    // La CD se recorta al rango [0, CD_MAX] antes de comparar, y se
    // devuelve recortada: quien lo lea (log, HUD) ve la dificultad real.
    CheckOutcome alto = DicePoolEngine::resolve_against_cd(pool(3.0f), 99.0f);
    require(casi(alto.cd, DicePoolEngine::CD_MAX));
    require(alto.degree == Degree::SUCCESS);
    CheckOutcome bajo = DicePoolEngine::resolve_against_cd(pool(0.5f), -7.0f);
    require(casi(bajo.cd, 0.0f));

    // El pool sin dados, extremo a extremo: pifia, no exito gratis.
    ScriptedRng rng({6});
    PoolResult vacio = DicePoolEngine::roll_pool(0, DiceMod::NORMAL, rng);
    require(DicePoolEngine::resolve_against_cd(vacio, 0.0f).degree == Degree::BOTCH);

    // Y por la puerta de la defensa entera, que es la que usa el combate.
    require(DicePoolEngine::resolve_against_defense(pool(0.0f), 10).degree == Degree::BOTCH);
    require(DicePoolEngine::resolve_against_defense(pool(1.0f), 12).degree == Degree::SUCCESS);
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
    testDefensaACd();
    testGrados();
    testEnfrentada();
    testReproducible();

    // Tabla de referencia, util al tocar las reglas: se ve de un vistazo
    // que la fila de 0 exitos es pifia en TODAS las dificultades.
    std::printf("grados por exitos y CD (filas = exitos, columnas = CD)\n");
    std::printf("        CD 0.0    CD 0.5    CD 1.0    CD 2.0    CD 3.0\n");
    for (float e = 0.0f; e <= 3.0f; e += 0.5f) {
        std::printf("  %.1f ", e);
        for (float cd : {0.0f, 0.5f, 1.0f, 2.0f, 3.0f}) {
            std::printf("%9s ", nombre(DicePoolEngine::resolve_against_cd(pool(e), cd).degree));
        }
        std::printf("\n");
    }

    std::printf("\ndemo_dice_engine: todas las comprobaciones han pasado\n");
    return 0;
}
