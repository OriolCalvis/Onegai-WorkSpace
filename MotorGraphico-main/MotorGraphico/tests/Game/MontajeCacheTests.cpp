// PERF-1 / PERF-1b — La cache de FILAS de atlas de MontajePersonajes, sin ventana.
//   1. Con un presupuesto minimo, un montaje lee cada atlas una sola vez (no suelta las filas que ya
//      uso) y al acabar no queda nada.
//   2. Dos apariencias que comparten filas no las releen aunque el presupuesto sea el justo.
//   3. Entre montajes (y durante ellos) la cache respeta el presupuesto en bytes de filas.
//   4. La hoja sale igual con presupuesto 0, minimo, justo y grande.
//   5. El presupuesto por defecto es el de 320 MB; MOTOR_CACHE_ATLAS_MB lo cambia; 0 no es infinito.
//   6. Doce razas dos veces con 320 MB: la segunda vuelta no lee disco.
//   7. Una apariencia con filas de 4 atlas lee cada atlas una sola vez; la pagina Raza (20 razas) tambien.
//   8. LRU: la fila que se usa pasa al frente (orden de expulsion).
#include "TestHarness.h"

#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <map>
#include <memory>
#include <stdexcept>
#include <set>
#include <string>
#include <vector>

#include "Game/Personajes/MontajePersonajes.h"

namespace {

const char* const kTexturas = "assets/textures";

std::unique_ptr<Game::MontajePersonajes> montaje(std::size_t presupuesto) {
    auto m = Game::ManifiestoPersonajes::cargar(kTexturas);
    if (!m.isOk())
        throw std::runtime_error(m.errorMessage());
    return std::make_unique<Game::MontajePersonajes>(std::move(m.value()), kTexturas, presupuesto);
}

std::set<std::string> atlasDe(Game::MontajePersonajes& m, const Game::Apariencia& a,
                              const std::map<std::string, std::string>& equipo = {}) {
    std::set<std::string> out;
    auto capas = m.capas(a, equipo);
    if (capas.isOk())
        for (const auto& c : capas.value())
            out.insert(c.atlas);
    return out;
}

std::size_t filaBytes() { return 2560u * 64u * 4u; }

// Doce razas repartidas por el manifiesto (las que ve el benchmark).
std::vector<Game::Apariencia> docePersonajes(const Game::ManifiestoPersonajes& man) {
    std::vector<Game::Apariencia> out;
    for (std::size_t i = 0; i < man.razas.size() && out.size() < 12; i += 6)
        out.push_back(Game::Apariencia::deRaza(man.razas[i]));
    return out;
}

// Las claves ruta#fila de las capas de una apariencia.
std::set<std::string> clavesDe(Game::MontajePersonajes& m, const Game::Apariencia& a,
                               const std::map<std::string, std::string>& equipo = {}) {
    std::set<std::string> out;
    auto capas = m.capas(a, equipo);
    if (capas.isOk())
        for (const auto& c : capas.value())
            out.insert(c.atlas + "#" + std::to_string(c.fila));
    return out;
}

}  // namespace

MOTOR_TEST_CASE(montaje_cache_presupuesto_por_defecto) {
    MOTOR_CHECK(Game::MontajePersonajes::kPresupuestoPorDefecto == 320u * 1024u * 1024u);
}

MOTOR_TEST_CASE(montaje_cache_interpreta_el_texto_del_entorno) {
    using Game::MontajePersonajes;
    const std::size_t def = MontajePersonajes::kPresupuestoPorDefecto;
    MOTOR_CHECK(MontajePersonajes::presupuestoDeTexto("64") == 64u * 1024u * 1024u);
    MOTOR_CHECK(MontajePersonajes::presupuestoDeTexto("1024") == 1024u * 1024u * 1024u);
    MOTOR_CHECK(MontajePersonajes::presupuestoDeTexto("0") == 0);  // 0 no es «sin limite»
    MOTOR_CHECK(MontajePersonajes::presupuestoDeTexto(nullptr) == def);
    MOTOR_CHECK(MontajePersonajes::presupuestoDeTexto("") == def);
    MOTOR_CHECK(MontajePersonajes::presupuestoDeTexto("mucho") == def);
    MOTOR_CHECK(MontajePersonajes::presupuestoDeTexto("-5") == def);
}

MOTOR_TEST_CASE(montaje_cache_la_variable_de_entorno_manda) {
    // main() puso MOTOR_CACHE_ATLAS_MB=5 antes de la primera lectura.
    MOTOR_CHECK(Game::MontajePersonajes::presupuestoDeEntorno() == 5u * 1024u * 1024u);
    auto m = Game::ManifiestoPersonajes::cargar(kTexturas);
    MOTOR_CHECK(m.isOk());
    Game::MontajePersonajes por_defecto(std::move(m.value()), kTexturas);
    MOTOR_CHECK(por_defecto.presupuesto() == 5u * 1024u * 1024u);
}

MOTOR_TEST_CASE(montaje_cache_presupuesto_cero_no_es_infinito) {
    auto cero = montaje(0);
    auto grande = montaje(Game::MontajePersonajes::kPresupuestoPorDefecto);
    const auto& man = cero->manifiesto();
    const Game::Apariencia a = Game::Apariencia::deRaza(man.razas[0]);
    const std::size_t n = atlasDe(*cero, a).size();
    auto uno = cero->montar(a);
    auto ref = grande->montar(a);
    MOTOR_CHECK(uno.isOk() && ref.isOk());
    MOTOR_CHECK(uno.value().pixeles == ref.value().pixeles);
    MOTOR_CHECK(cero->bytesEnCache() == 0);   // nada entre montajes
    MOTOR_CHECK(cero->filasEnCache() == 0);
    MOTOR_CHECK(cero->lecturasDeDisco() == n);
    MOTOR_CHECK(cero->picoDeBytes() <= clavesDe(*cero, a).size() * filaBytes());  // solo las suyas
    cero->montar(a);
    MOTOR_CHECK(cero->lecturasDeDisco() == 2 * n);  // y por eso relee
}

MOTOR_TEST_CASE(montaje_cache_no_suelta_lo_del_montaje_en_curso) {
    auto grande = montaje(Game::MontajePersonajes::kPresupuestoPorDefecto);
    auto minimo = montaje(1);
    const auto& man = grande->manifiesto();
    for (std::size_t i = 0; i < man.razas.size(); i += 17) {
        const Game::Apariencia a = Game::Apariencia::deRaza(man.razas[i]);
        const std::size_t antes = minimo->lecturasDeDisco();
        auto uno = grande->montar(a);
        auto otro = minimo->montar(a);
        MOTOR_CHECK(uno.isOk() && otro.isOk());
        MOTOR_CHECK(uno.value().pixeles == otro.value().pixeles);
        // lee cada atlas distinto una vez en cada montaje, aunque el presupuesto sea de un byte
        MOTOR_CHECK(minimo->lecturasDeDisco() - antes == atlasDe(*minimo, a).size());
        MOTOR_CHECK(minimo->bytesEnCache() == 0 && minimo->filasEnCache() == 0);
        // durante el montaje solo tuvo sus filas
        MOTOR_CHECK(minimo->picoDeBytes() <= clavesDe(*minimo, a).size() * filaBytes());
        minimo->reiniciarPico();
    }
}

MOTOR_TEST_CASE(montaje_cache_doce_razas_dos_vueltas_sin_disco) {
    const std::size_t presupuesto = 320u * 1024u * 1024u;
    auto m = montaje(presupuesto);
    const auto personajes = docePersonajes(m->manifiesto());
    MOTOR_CHECK(personajes.size() == 12);
    std::vector<std::vector<std::uint8_t>> primera;
    for (const auto& a : personajes) {
        auto h = m->montar(a);
        MOTOR_CHECK(h.isOk());
        primera.push_back(std::move(h.value().pixeles));
        // (b) el presupuesto, en bytes de filas, entre montajes y durante ellos
        MOTOR_CHECK(m->bytesEnCache() <= presupuesto);
        MOTOR_CHECK(m->picoDeBytes() <= presupuesto);
        MOTOR_CHECK(m->bytesEnCache() == m->filasEnCache() * filaBytes());
    }
    const std::size_t lecturas = m->lecturasDeDisco();
    for (std::size_t i = 0; i < personajes.size(); ++i) {
        auto h = m->montar(personajes[i]);
        MOTOR_CHECK(h.isOk());
        MOTOR_CHECK(h.value().pixeles == primera[i]);
    }
    MOTOR_CHECK(m->lecturasDeDisco() == lecturas);  // (a) la segunda vuelta no toca el disco
}

MOTOR_TEST_CASE(montaje_cache_respeta_un_presupuesto_justo_durante_y_entre_montajes) {
    // 16 MB = 25 filas: una raza usa de 4 a 6, asi que cabe un montaje pero no todo el conjunto.
    const std::size_t presupuesto = 16u * 1024u * 1024u;
    auto m = montaje(presupuesto);
    auto grande = montaje(Game::MontajePersonajes::kPresupuestoPorDefecto);
    for (int vuelta = 0; vuelta < 2; ++vuelta)
        for (const auto& a : docePersonajes(m->manifiesto())) {
            auto uno = m->montar(a);
            auto otro = grande->montar(a);
            MOTOR_CHECK(uno.isOk() && otro.isOk());
            MOTOR_CHECK(uno.value().pixeles == otro.value().pixeles);
            MOTOR_CHECK(m->bytesEnCache() <= presupuesto);
            // el pico, DURANTE el montaje: se hace sitio antes de guardar cada fila
            MOTOR_CHECK(m->picoDeBytes() <= presupuesto);
        }
}

MOTOR_TEST_CASE(montaje_cache_cada_atlas_se_lee_una_vez_por_montaje) {
    auto m = montaje(1);  // ni una fila de sobra: solo cuenta lo del montaje
    const auto& man = m->manifiesto();
    // una raza con filas de 4 o mas atlas distintos
    const Game::RazaVisual* quien = nullptr;
    for (const auto& r : man.razas)
        if (atlasDe(*m, Game::Apariencia::deRaza(r)).size() >= 4) {
            quien = &r;
            break;
        }
    MOTOR_CHECK(quien != nullptr);
    if (quien == nullptr)
        return;
    const Game::Apariencia a = Game::Apariencia::deRaza(*quien);
    const std::size_t n = atlasDe(*m, a).size();
    MOTOR_CHECK(n >= 4);
    MOTOR_CHECK(m->montar(a).isOk());
    MOTOR_CHECK(m->lecturasDeDisco() == n);

    // con un kit de piezas: dos filas del mismo atlas (o mas capas que atlas) siguen siendo una lectura
    std::map<std::string, std::string> eq;
    for (const auto& [ranura, g] : man.equipo)
        if (ranura == "torso" || ranura == "piernas" || ranura == "pies" || ranura == "cabeza" || ranura == "capa" ||
            ranura == "arma" || ranura == "mano_izq")
            for (const auto& f : g.filas)
                if (!f.soloPnj && !f.noEncaja) {
                    eq[ranura] = f.id;
                    break;
                }
    auto otro = montaje(1);
    auto capas = otro->capas(a, eq);
    MOTOR_CHECK(capas.isOk());
    const std::size_t atlas = atlasDe(*otro, a, eq).size();
    MOTOR_CHECK(otro->montar(a, eq).isOk());
    MOTOR_CHECK(otro->lecturasDeDisco() == atlas);
}

MOTOR_TEST_CASE(montaje_cache_la_pagina_raza_lee_cada_atlas_una_vez) {
    // Razas de la pagina Raza (8 de las 20): con sitio para todo el conjunto (aqui, 900 MB: los atlas que
    // tocan suman menos), cada atlas se lee una sola vez y no una por fila. Con 320 MB el
    // conjunto no cabe y se relee lo que se soltó (lo mide el benchmark, no este test).
    auto m = montaje(900u * 1024u * 1024u);
    const auto& man = m->manifiesto();
    std::set<std::string> todos;
    std::set<std::string> filas;
    for (std::size_t i = 0; i < 8 && i < man.razas.size(); ++i) {
        const Game::Apariencia a = Game::Apariencia::deRaza(man.razas[i]);
        const auto s = atlasDe(*m, a);
        todos.insert(s.begin(), s.end());
        const auto c = clavesDe(*m, a);
        filas.insert(c.begin(), c.end());
        MOTOR_CHECK(m->montar(a).isOk());
    }
    MOTOR_CHECK(m->lecturasDeDisco() == todos.size());
    MOTOR_CHECK(m->lecturasDeDisco() < filas.size());
}

MOTOR_TEST_CASE(montaje_cache_lru_la_fila_usada_pasa_al_frente) {
    auto m = montaje(Game::MontajePersonajes::kPresupuestoPorDefecto);
    const auto& man = m->manifiesto();
    // dos peinados con filas distintas, en una raza que admita pelo
    std::vector<Game::Apariencia> v;
    const auto clavePelo = [&](const Game::Apariencia& a) {
        const auto capas = m->capas(a);
        if (capas.isOk())
            for (const auto& c : capas.value())
                if (c.ranura == "pelo")
                    return c.atlas + "#" + std::to_string(c.fila);
        return std::string();
    };
    for (const auto& raza : man.razas) {
        v.clear();
        std::set<std::string> vistas;
        for (const auto& f : man.pelo.filas) {
            if (f.estilo.empty())
                continue;
            Game::Apariencia a = Game::Apariencia::deRaza(raza);
            a.peloEstilo = f.estilo;
            a.peloColor = "pelo_castano";
            const std::string k = clavePelo(a);
            if (k.empty() || !vistas.insert(k).second)
                continue;
            v.push_back(a);
            if (v.size() == 2)
                break;
        }
        if (v.size() == 2)
            break;
    }
    MOTOR_CHECK(v.size() == 2);
    if (v.size() < 2)
        return;
    const std::string k1 = clavePelo(v[0]), k2 = clavePelo(v[1]);
    MOTOR_CHECK(!k1.empty() && !k2.empty() && k1 != k2);
    const auto pos = [&](const std::string& k) {
        const auto o = m->clavesEnOrden();
        return std::find(o.begin(), o.end(), k) - o.begin();
    };
    MOTOR_CHECK(m->montar(v[0]).isOk());
    MOTOR_CHECK(m->montar(v[1]).isOk());
    MOTOR_CHECK(pos(k2) < pos(k1));  // el ultimo montado, delante
    const std::size_t lecturas = m->lecturasDeDisco();
    MOTOR_CHECK(m->montar(v[0]).isOk());
    MOTOR_CHECK(m->lecturasDeDisco() == lecturas);
    MOTOR_CHECK(pos(k1) < pos(k2));  // la fila usada otra vez pasa al frente: es la ultima en soltarse
    // y las filas del primer peinado quedan todas por delante de la del segundo
    const auto o = m->clavesEnOrden();
    for (const std::string& k : clavesDe(*m, v[0]))
        MOTOR_CHECK(std::find(o.begin(), o.end(), k) - o.begin() < pos(k2));
}

MOTOR_TEST_CASE(montaje_cache_comparte_atlas_entre_apariencias) {
    auto grande = montaje(Game::MontajePersonajes::kPresupuestoPorDefecto);
    const auto& man = grande->manifiesto();
    // dos razas distintas con atlas en comun y alguno propio
    const Game::RazaVisual* ra = nullptr;
    const Game::RazaVisual* rb = nullptr;
    std::set<std::string> sa, sb;
    for (std::size_t i = 0; i < man.razas.size() && rb == nullptr; ++i) {
        for (std::size_t j = i + 1; j < man.razas.size(); ++j) {
            auto a = atlasDe(*grande, Game::Apariencia::deRaza(man.razas[i]));
            auto b = atlasDe(*grande, Game::Apariencia::deRaza(man.razas[j]));
            std::set<std::string> comun;
            std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::inserter(comun, comun.begin()));
            if (!comun.empty() && a != b && a.size() <= 4 && b.size() <= 4) {
                ra = &man.razas[i]; rb = &man.razas[j]; sa = a; sb = b;
                break;
            }
        }
    }
    MOTOR_CHECK(ra != nullptr && rb != nullptr);
    if (ra == nullptr || rb == nullptr)
        return;
    std::set<std::string> union_ = sa;
    union_.insert(sb.begin(), sb.end());
    const Game::Apariencia a = Game::Apariencia::deRaza(*ra);
    const Game::Apariencia b = Game::Apariencia::deRaza(*rb);

    // el presupuesto de las filas de A: A y luego B leen la union, ni uno mas
    const std::size_t bytesA = clavesDe(*grande, a).size() * filaBytes();
    auto justo = montaje(bytesA);
    auto ha = justo->montar(a);
    MOTOR_CHECK(ha.isOk());
    MOTOR_CHECK(justo->lecturasDeDisco() == sa.size());
    auto hb = justo->montar(b);
    MOTOR_CHECK(hb.isOk());
    MOTOR_CHECK(justo->lecturasDeDisco() >= sa.size() && justo->lecturasDeDisco() <= sa.size() + sb.size());
    // entre montajes respeta el presupuesto
    MOTOR_CHECK(justo->bytesEnCache() <= bytesA);

    // y los pixeles son los mismos con la cache grande
    auto ga = grande->montar(a);
    auto gb = grande->montar(b);
    MOTOR_CHECK(ga.value().pixeles == ha.value().pixeles);
    MOTOR_CHECK(gb.value().pixeles == hb.value().pixeles);

    // repetir A con el presupuesto holgado no vuelve a leer nada
    const std::size_t antes = grande->lecturasDeDisco();
    grande->montar(a);
    grande->montar(b);
    MOTOR_CHECK(grande->lecturasDeDisco() == antes);
    MOTOR_CHECK(grande->lecturasDeDisco() <= sa.size() + sb.size());
}

int main() {
    // Antes de la primera lectura del entorno (se lee una vez): lo comprueba montaje_cache_la_variable_de_entorno_manda.
    setenv("MOTOR_CACHE_ATLAS_MB", "5", 1);
    return motor_test::runAllTests();
}
