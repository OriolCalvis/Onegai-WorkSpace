// U4-07.E08 — limite y repetibilidad del sistema de particulas (gl-free, sin glad). Solo mide: sin codigo de producto.
// Los tiempos de CPU (std::chrono) se IMPRIMEN como dato informativo; ninguna asercion depende del reloj y no se
// promete cifra de FPS. La medida real de dibujo (SpriteBatch::submit) queda como observacion manual en GL.
#include "Game/Emisor/PasosDePolvo.h"
#include "Game/Emisor/ProyeccionParticulas.h"

#include "TestHarness.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

using namespace Game::Emisor;
using Game::Destello;
using Game::FormaDeParticula;

namespace {

constexpr double kTick = 1.0 / 60.0;
using Reloj = std::chrono::steady_clock;

double msDesde(Reloj::time_point t0) { return std::chrono::duration<double, std::milli>(Reloj::now() - t0).count(); }

// ---- hash FNV-1a 64 de todo el estado relevante ----
struct Hash {
    std::uint64_t h = 1469598103934665603ull;
    void bytes(const void* p, std::size_t n) {
        const auto* b = static_cast<const unsigned char*>(p);
        for (std::size_t i = 0; i < n; ++i) { h ^= b[i]; h *= 1099511628211ull; }
    }
    template <class T> void val(const T& v) { bytes(&v, sizeof(T)); }
};

// Hash de TODO lo que la API publica expone de un emisor: definicion completa (familia incluida), tipo de particula,
// origen, grupo de dibujo, tiempo, nacidas, vivas y las particulas (44 bytes sin relleno). Los campos se mezclan uno a uno
// (nunca la estructura entera: tiene relleno). NO se ve el estado del generador (RNG) ni el residuo de emision: son
// privados y no hay accesor. Ambos son funcion determinista de lo que SI se hashea (RNG: semilla + 9 sorteos por
// particula nacida; residuo: tasa, escala y los dt aplicados), de modo que dos emisores con la misma historia los
// tienen iguales. Lo que NO queda probado es que dos residuos distintos se distingan: si una divergencia de residuo/RNG
// no cambia ni los recuentos ni las particulas, ningun hash publico la ve. Por eso `corre` suma una traza (hash de cada
// tick) de un tramo largo posterior, que SI detecta cualquier divergencia en cuanto cambia un nacimiento o una particula.
void hashDeEmisor(Hash& h, const Emisor& e) {
    const DefinicionEmisor& d = e.definicion();
    h.val(d.familia.size());
    h.bytes(d.familia.data(), d.familia.size());
    h.val(d.tasa); h.val(d.duracion); h.val(d.cortar);
    h.val(d.vidaMin); h.val(d.vidaMax);
    h.val(d.velocidadMin); h.val(d.velocidadMax);
    h.val(d.anguloCentro); h.val(d.abanico);
    h.val(d.maximo); h.val(d.semilla);
    h.val(d.opacidad); h.val(d.contraste);
    h.bytes(d.colorBase.data(), d.colorBase.size());
    const Game::TipoDeParticula& t = e.tipo();
    h.val(t.vy); h.val(t.vx); h.val(t.vida);
    h.val(static_cast<int>(t.forma)); h.val(t.suma); h.val(static_cast<int>(t.movimiento));
    h.val(t.colores.size());
    for (const auto& c : t.colores) h.bytes(c.data(), c.size());
    h.val(t.densidad); h.val(static_cast<int>(t.otraForma)); h.val(t.conOtraForma); h.val(t.gira);
    h.val(e.origen.x); h.val(e.origen.y);
    h.val(e.grupo.esquina.x); h.val(e.grupo.esquina.y); h.val(e.grupo.tam.x); h.val(e.grupo.tam.y);
    h.val(e.tiempo());
    h.val(e.nacidas());
    const std::size_t n = e.buffer().vivas();
    h.val(n);
    for (const Particula& q : e.buffer().datos()) h.bytes(&q, sizeof(Particula));
}

std::uint64_t hashDeUno(const Emisor& e) {
    Hash h;
    hashDeEmisor(h, e);
    return h.h;
}

std::uint64_t hashDe(const std::vector<Emisor>& v, const PresupuestoParticulas& p) {
    Hash h;
    h.val(v.size());
    for (const Emisor& e : v) hashDeEmisor(h, e);
    h.val(p.vivas);
    h.val(p.maximo);
    return h.h;
}

DefinicionEmisor defCarga(std::uint64_t semilla, const char* familia = "veneno") {
    DefinicionEmisor d;
    d.familia = familia;
    d.tasa = kMaxTasa;                     // 500 part/s
    d.duracion = 0.0;                      // continuo
    d.vidaMin = 0.8; d.vidaMax = 1.6;
    d.velocidadMin = 3.0; d.velocidadMax = 8.0;
    d.anguloCentro = -90.0; d.abanico = 140.0;
    d.maximo = kMaxPorEmisor;              // 256
    d.semilla = semilla;
    return d;
}

// Recuento INDEPENDIENTE de quadsDeDestellos: replica las tres pasadas de pintaDestellos (PintaDestellos.cpp):
// contorno de todo lo visible, luego cuerpo de lo que tapa y cuerpo de lo que suma; todo con alfa > 0.
std::size_t quadsComoPintaDestellos(const std::vector<Destello>& ds) {
    std::size_t n = 0;
    for (const Destello& d : ds)
        if (d.alfa > 0.0f) n += Game::contornoDeForma(d.forma).size();
    for (const bool suma : {false, true})
        for (const Destello& d : ds) {
            if (d.suma != suma || d.alfa <= 0.0f) continue;
            n += Game::pixelesDeForma(d.forma).size();
        }
    return n;
}

struct Medida {
    std::uint64_t hash = 0, hashTras = 0, traza = 0;
    std::size_t maxVivasEmisor = 0, maxVivasGlobal = 0, maxQuads = 0, maxDescartados = 0, maxDestellos = 0;
    long rechazadas = 0, nacidas = 0;
    bool desacuerdoQuads = false, sumaIncoherente = false, excedeQuads = false;
    std::vector<double> msAvance, msProyeccion;
};

// 64 emisores de 256 a tasa maxima, `pasos` ticks de 1/60. Proyecta cada tick (como el render) con el tope.
Medida corre(std::uint64_t base, int pasos, int emisores = kMaxEmisores, bool mide = false) {
    std::vector<Emisor> v;
    v.reserve(static_cast<std::size_t>(emisores));
    for (int i = 0; i < emisores; ++i) v.emplace_back(defCarga(base + static_cast<std::uint64_t>(i)));
    PresupuestoParticulas p;   // maximo = kMaxParticulasGlobal
    Medida m;
    std::vector<Destello> out;
    out.reserve(40000);
    const EstiloEmisor estilo;
    auto tick = [&](bool proyecta) {
        const auto t0 = Reloj::now();
        p.recuenta(v);
        for (Emisor& e : v) {
            auto r = avanzarEmisor(e, kTick, 1.0, p);
            MOTOR_CHECK(r.isOk());
            m.rechazadas += r.value().rechazadas;
            m.nacidas += r.value().nacidas;
        }
        if (mide) m.msAvance.push_back(msDesde(t0));
        std::size_t suma = 0;
        for (const Emisor& e : v) {
            m.maxVivasEmisor = std::max(m.maxVivasEmisor, e.buffer().vivas());
            suma += e.buffer().vivas();
        }
        m.maxVivasGlobal = std::max(m.maxVivasGlobal, p.vivas);
        if (suma != p.vivas) m.sumaIncoherente = true;
        if (!proyecta) return;
        const auto t1 = Reloj::now();
        out.clear();
        std::size_t quads = 0, descartados = 0;
        for (const Emisor& e : v) {
            const std::size_t resto = static_cast<std::size_t>(kMaxQuadsPorFotograma) - quads;
            const ResumenProyeccion r = proyectarParticulas(e, estilo, resto, out);
            quads += r.quads;
            descartados += r.descartados;
        }
        if (mide) m.msProyeccion.push_back(msDesde(t1));
        if (quadsDeDestellos(out) != quadsComoPintaDestellos(out) || quadsDeDestellos(out) != quads)
            m.desacuerdoQuads = true;
        if (quads > static_cast<std::size_t>(kMaxQuadsPorFotograma)) m.excedeQuads = true;
        m.maxQuads = std::max(m.maxQuads, quads);
        m.maxDescartados = std::max(m.maxDescartados, descartados);
        m.maxDestellos = std::max(m.maxDestellos, out.size());
    };
    for (int i = 0; i < pasos; ++i) tick(true);
    m.hash = hashDe(v, p);
    // Sigue avanzando 120 ticks (2 s) y encadena el hash de CADA tick: una divergencia del residuo o del RNG privados
    // aparece en cuanto cambia un nacimiento o una particula. No prueba que todo residuo distinto sea visible.
    // Las cifras de carga (nacidas, rechazadas, maximos) son las de los `pasos` ticks: el tramo posterior no las toca.
    Medida res = m;
    Hash traza;
    for (int i = 0; i < 120; ++i) {
        tick(false);
        traza.val(hashDe(v, p));
    }
    res.sumaIncoherente = m.sumaIncoherente;
    res.traza = traza.h;
    res.hashTras = hashDe(v, p);
    return res;
}

double mediana(std::vector<double> v) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    return v[v.size() / 2];
}
double maximo(const std::vector<double>& v) { return v.empty() ? 0.0 : *std::max_element(v.begin(), v.end()); }

MOTOR_TEST_CASE(dos_runs_misma_entrada_mismo_hash_y_otra_semilla_distinto) {
    const Medida a = corre(1000, 600);
    std::vector<int> basura(100003, 7);   // mueve el monton entre runs: nada puede depender de direcciones
    const Medida b = corre(1000, 600);
    MOTOR_CHECK(a.hash == b.hash && a.hashTras == b.hashTras && a.traza == b.traza);
    MOTOR_CHECK(a.nacidas == b.nacidas && a.rechazadas == b.rechazadas);
    const Medida c = corre(2000, 600);
    MOTOR_CHECK(c.hash != a.hash && c.hashTras != a.hashTras && c.traza != a.traza);
    MOTOR_CHECK(basura[5] == 7);
    std::printf("[E08] hash 600 pasos: 0x%016llx (semilla 1000), tras 120 mas: 0x%016llx, traza de esos 120: 0x%016llx\n",
                static_cast<unsigned long long>(a.hash), static_cast<unsigned long long>(a.hashTras),
                static_cast<unsigned long long>(a.traza));
}

MOTOR_TEST_CASE(emisores_independientes_el_rng_no_se_comparte) {
    // Mismos datos y semilla, avanzados en paralelo con otro emisor de semilla distinta en medio: iguales bit a bit.
    Emisor a(defCarga(5)), b(defCarga(5)), c(defCarga(6));
    PresupuestoParticulas pa, pb, pc;
    for (int i = 0; i < 120; ++i) {
        MOTOR_CHECK(avanzarEmisor(a, kTick, 1.0, pa).isOk());
        MOTOR_CHECK(avanzarEmisor(c, kTick, 1.0, pc).isOk());
        if (i % 3 == 0) MOTOR_CHECK(avanzarEmisor(c, kTick, 1.0, pc).isOk());
        MOTOR_CHECK(avanzarEmisor(b, kTick, 1.0, pb).isOk());
    }
    MOTOR_CHECK(a.buffer().vivas() == b.buffer().vivas() && a.buffer().vivas() > 0);
    MOTOR_CHECK(std::memcmp(a.buffer().datos().data(), b.buffer().datos().data(),
                            a.buffer().vivas() * sizeof(Particula)) == 0);
    MOTOR_CHECK(c.buffer().vivas() != a.buffer().vivas() ||
                std::memcmp(a.buffer().datos().data(), c.buffer().datos().data(),
                            a.buffer().vivas() * sizeof(Particula)) != 0);
}

MOTOR_TEST_CASE(carga_64_emisores_respeta_aforo_presupuesto_y_quads) {
    const Medida m = corre(1000, 600, kMaxEmisores, true);
    MOTOR_CHECK(m.maxVivasEmisor <= static_cast<std::size_t>(kMaxPorEmisor));
    MOTOR_CHECK(m.maxVivasGlobal <= static_cast<std::size_t>(kMaxParticulasGlobal));
    MOTOR_CHECK(m.maxVivasGlobal > static_cast<std::size_t>(kMaxParticulasGlobal) / 2);   // la prueba SI carga
    MOTOR_CHECK(!m.sumaIncoherente);
    MOTOR_CHECK(m.rechazadas > 0);
    MOTOR_CHECK(!m.excedeQuads && m.maxQuads <= static_cast<std::size_t>(kMaxQuadsPorFotograma));
    MOTOR_CHECK(m.maxDescartados > 0);                 // el tope de quads SI actua con esta carga
    MOTOR_CHECK(!m.desacuerdoQuads);                   // quadsDeDestellos == recuento independiente == resumen
    std::printf("[E08] carga 64x256 veneno 10 s: vivas/emisor max %zu, vivas global max %zu, nacidas %ld, rechazadas %ld\n",
                m.maxVivasEmisor, m.maxVivasGlobal, m.nacidas, m.rechazadas);
    std::printf("[E08] carga: destellos/fotograma max %zu, quads/fotograma max %zu (tope %d), descartados max %zu\n",
                m.maxDestellos, m.maxQuads, kMaxQuadsPorFotograma, m.maxDescartados);
    std::printf("[E08] CPU Debug por tick (informativo): avance mediana %.3f ms max %.3f ms; proyeccion mediana %.3f ms max %.3f ms\n",
                mediana(m.msAvance), maximo(m.msAvance), mediana(m.msProyeccion), maximo(m.msProyeccion));
}

MOTOR_TEST_CASE(aforo_de_un_emisor_con_pocos_emisores_llega_a_256) {
    // Con 4 emisores el presupuesto global (2048) no es el limite: lo es el aforo de cada uno (256).
    const Medida m = corre(1000, 300, 4);
    MOTOR_CHECK(m.maxVivasEmisor == static_cast<std::size_t>(kMaxPorEmisor));
    MOTOR_CHECK(m.maxVivasGlobal <= 4u * static_cast<std::size_t>(kMaxPorEmisor) && m.rechazadas > 0);
    MOTOR_CHECK(!m.excedeQuads && !m.desacuerdoQuads && !m.sumaIncoherente);
    std::printf("[E08] 4 emisores veneno: vivas/emisor max %zu, global max %zu, rechazadas %ld, quads max %zu, descartados max %zu\n",
                m.maxVivasEmisor, m.maxVivasGlobal, m.rechazadas, m.maxQuads, m.maxDescartados);
}

MOTOR_TEST_CASE(caso_peor_calavera_quads_contra_recuento_independiente) {
    // Ninguna forma de ninguna familia cuesta mas de 34 quads: la calavera es el caso peor.
    std::size_t peor = 0;
    for (const std::string& f : Game::familiasDeParticulas()) {
        const Game::TipoDeParticula t = Game::tipoDeParticula(f);
        for (const FormaDeParticula fo : {t.forma, t.otraForma})
            peor = std::max(peor, Game::pixelesDeForma(fo).size() + Game::contornoDeForma(fo).size());
    }
    MOTOR_CHECK(peor == 34);
    MOTOR_CHECK(Game::pixelesDeForma(FormaDeParticula::Calavera).size() + Game::contornoDeForma(FormaDeParticula::Calavera).size() == 34);
    std::vector<Destello> d;
    for (int i = 0; i < 240; ++i) d.push_back({0.0f, 0.0f, 1.0f, FormaDeParticula::Calavera, i % 2 == 0, {200, 255, 100}});
    MOTOR_CHECK(quadsDeDestellos(d) == 240u * 34u && quadsDeDestellos(d) == quadsComoPintaDestellos(d));
    MOTOR_CHECK(quadsDeDestellos(d) <= static_cast<std::size_t>(kMaxQuadsPorFotograma));
    d.push_back({0.0f, 0.0f, 1.0f, FormaDeParticula::Calavera, true, {1, 2, 3}});
    MOTOR_CHECK(quadsDeDestellos(d) == 241u * 34u && quadsDeDestellos(d) > static_cast<std::size_t>(kMaxQuadsPorFotograma));
    d.push_back({0.0f, 0.0f, 0.0f, FormaDeParticula::Calavera, true, {1, 2, 3}});   // alfa 0: coste 0
    MOTOR_CHECK(quadsDeDestellos(d) == 241u * 34u && quadsComoPintaDestellos(d) == 241u * 34u);
    // Con emisores "veneno" y el tope de la proyeccion: un solo emisor lleno (256) cabe o se recorta, nunca excede.
    std::vector<Emisor> v;
    for (int i = 0; i < 2; ++i) v.emplace_back(defCarga(40 + static_cast<std::uint64_t>(i)));
    PresupuestoParticulas p;
    for (int i = 0; i < 120; ++i)
        for (Emisor& e : v) MOTOR_CHECK(avanzarEmisor(e, kTick, 1.0, p).isOk());
    std::vector<Destello> out;
    std::size_t quads = 0;
    for (const Emisor& e : v) quads += proyectarParticulas(e, EstiloEmisor{}, kMaxQuadsPorFotograma - quads, out).quads;
    MOTOR_CHECK(quads <= static_cast<std::size_t>(kMaxQuadsPorFotograma) && quads == quadsDeDestellos(out));
    MOTOR_CHECK(quadsDeDestellos(out) == quadsComoPintaDestellos(out));
}

// ---- Hallazgo 1: el hash ve origen, grupo, definicion y tipo ----
DefinicionEmisor defLarga(std::uint64_t semilla, int maximo, const char* familia) {
    DefinicionEmisor d = defCarga(semilla, familia);
    d.vidaMin = 10.0; d.vidaMax = 10.0;    // kMaxVida: nada caduca en las pruebas de frontera
    d.maximo = maximo;
    return d;
}

MOTOR_TEST_CASE(el_hash_cubre_origen_grupo_definicion_y_tipo) {
    // Emisor avanzado 60 ticks; las copias no existen (Emisor no es copiable), asi que se reconstruye con la misma
    // historia y se muta SOLO un campo publico; el hash tiene que cambiar y, sin mutar, ser igual.
    auto hecho = [](auto muta) {
        Emisor e(defCarga(77), Vector2{3.0f, 4.0f}, GrupoDeDibujo{{10.0f, 20.0f}, {64.0f, 64.0f}});
        PresupuestoParticulas p;
        for (int i = 0; i < 60; ++i) MOTOR_CHECK(avanzarEmisor(e, kTick, 1.0, p).isOk());
        muta(e);
        return hashDeUno(e);
    };
    const std::uint64_t base = hecho([](Emisor&) {});
    MOTOR_CHECK(base == hecho([](Emisor&) {}));
    MOTOR_CHECK(base != hecho([](Emisor& e) { e.origen.x += 100.0f; }));
    MOTOR_CHECK(base != hecho([](Emisor& e) { e.origen.y += 100.0f; }));
    MOTOR_CHECK(base != hecho([](Emisor& e) { e.grupo.esquina.x += 1.0f; }));
    MOTOR_CHECK(base != hecho([](Emisor& e) { e.grupo.esquina.y += 1.0f; }));
    MOTOR_CHECK(base != hecho([](Emisor& e) { e.grupo.tam.x = 32.0f; }));
    MOTOR_CHECK(base != hecho([](Emisor& e) { e.grupo.tam.y = 32.0f; }));
    // Definicion y tipo: emisores recien hechos (sin avanzar) que solo difieren en un campo que no toca la simulacion.
    auto fresco = [](auto muta) {
        DefinicionEmisor d = defCarga(5);
        Game::TipoDeParticula t = Game::tipoDeParticula(d.familia);
        muta(d, t);
        return hashDeUno(Emisor(d, t, Vector2{}, GrupoDeDibujo{}));
    };
    const std::uint64_t f0 = fresco([](DefinicionEmisor&, Game::TipoDeParticula&) {});
    MOTOR_CHECK(f0 == fresco([](DefinicionEmisor&, Game::TipoDeParticula&) {}));
    MOTOR_CHECK(f0 != fresco([](DefinicionEmisor& d, Game::TipoDeParticula&) { d.familia = "luz"; }));
    MOTOR_CHECK(f0 != fresco([](DefinicionEmisor& d, Game::TipoDeParticula&) { d.semilla += 1; }));
    MOTOR_CHECK(f0 != fresco([](DefinicionEmisor& d, Game::TipoDeParticula&) { d.opacidad = 0.5f; }));
    MOTOR_CHECK(f0 != fresco([](DefinicionEmisor& d, Game::TipoDeParticula&) { d.colorBase[0] ^= 1; }));
    MOTOR_CHECK(f0 != fresco([](DefinicionEmisor& d, Game::TipoDeParticula&) { d.cortar = true; }));
    MOTOR_CHECK(f0 != fresco([](DefinicionEmisor&, Game::TipoDeParticula& t) { t.forma = FormaDeParticula::Punto; }));
    MOTOR_CHECK(f0 != fresco([](DefinicionEmisor&, Game::TipoDeParticula& t) { t.colores.pop_back(); }));
    MOTOR_CHECK(f0 != fresco([](DefinicionEmisor&, Game::TipoDeParticula& t) { t.gira = !t.gira; }));
}

// ---- Hallazgo 2: frontera exacta del presupuesto global ----
// Llena un emisor (sin caducidades: vida 10 s, el emisor tarda ~0,5 s) hasta su aforo y devuelve las caducadas.
int rellena(Emisor& e, PresupuestoParticulas& p) {
    int caducadas = 0;
    for (int i = 0; i < 400 && e.buffer().vivas() < static_cast<std::size_t>(e.definicion().maximo); ++i) {
        auto r = avanzarEmisor(e, kTick, 1.0, p);
        MOTOR_CHECK(r.isOk());
        caducadas += r.value().caducadas;
    }
    return caducadas;
}

MOTOR_TEST_CASE(presupuesto_global_el_nacimiento_2048_entra_y_el_2049_se_rechaza) {
    PresupuestoParticulas p;
    MOTOR_CHECK(p.maximo == 2048u && p.vivas == 0u);
    std::vector<Emisor> v;
    v.reserve(10);
    int caducadas = 0;
    for (int i = 0; i < 7; ++i) {                                  // 7 x 256 = 1792
        v.emplace_back(defLarga(100 + static_cast<std::uint64_t>(i), 256, "veneno"));
        caducadas += rellena(v.back(), p);
    }
    MOTOR_CHECK(p.vivas == 1792u);
    v.emplace_back(defLarga(107, 255, "veneno"));                  // + 255 = 2047: falta UNA para el tope
    caducadas += rellena(v.back(), p);
    MOTOR_CHECK(p.vivas == 2047u && v.back().buffer().vivas() == 255u);
    // Nacimiento 2048: un unico nacimiento (tasa 500 x 0,002 s = 1) en un emisor nuevo con hueco propio: entra.
    v.emplace_back(defLarga(108, 256, "veneno"));
    auto r2048 = avanzarEmisor(v.back(), 0.002, 1.0, p);
    MOTOR_CHECK(r2048.isOk());
    MOTOR_CHECK(r2048.value().nacidas == 1 && r2048.value().rechazadas == 0);
    MOTOR_CHECK(p.vivas == 2048u && v.back().buffer().vivas() == 1u);
    // Nacimiento 2049: mismo paso en otro emisor nuevo: se rechaza, sin hueco aunque su buffer este vacio.
    v.emplace_back(defLarga(109, 256, "veneno"));
    auto r2049 = avanzarEmisor(v.back(), 0.002, 1.0, p);
    MOTOR_CHECK(r2049.isOk());
    MOTOR_CHECK(r2049.value().nacidas == 0 && r2049.value().rechazadas == 1);
    MOTOR_CHECK(p.vivas == 2048u && v.back().buffer().vivas() == 0u && v.back().nacidas() == 0u);
    MOTOR_CHECK(!p.reserva() && p.vivas == 2048u);
    // Sin caducidades intermedias y con la cuenta cuadrada contra los buffers.
    MOTOR_CHECK(caducadas == 0);
    std::size_t suma = 0;
    for (const Emisor& e : v) suma += e.buffer().vivas();
    MOTOR_CHECK(suma == 2048u);
    PresupuestoParticulas q;
    q.recuenta(v);
    MOTOR_CHECK(q.vivas == 2048u);
    std::printf("[E08] frontera presupuesto: nacimiento 2048 admitido, 2049 rechazado (sin caducidades)\n");
}

// ---- Hallazgo 3: frontera exacta de quads de la proyeccion ----
Emisor llenoDe(const char* familia, int n, std::uint64_t semilla) {
    Emisor e(defLarga(semilla, n, familia));
    PresupuestoParticulas p;
    MOTOR_CHECK(rellena(e, p) == 0);
    MOTOR_CHECK(e.buffer().vivas() == static_cast<std::size_t>(n));
    return e;
}

EstiloEmisor estiloPleno() {
    EstiloEmisor s;
    s.entrada = 0.0f; s.salida = 0.0f;   // alfa > 0 desde el primer tick: toda particula viva cuesta sus quads
    return s;
}

MOTOR_TEST_CASE(proyeccion_exacta_8192_quads_se_admite_entera_y_la_siguiente_unidad_se_descarta) {
    const EstiloEmisor est = estiloPleno();
    constexpr std::size_t kTope = static_cast<std::size_t>(kMaxQuadsPorFotograma);
    // 240 calaveras (34 quads) + 32 puntos (1 quad) = 8192 exactos; un punto mas ya no cabe.
    const Emisor calaveras = llenoDe("veneno", 240, 1);
    const Emisor puntos = llenoDe("brasas", 32, 2);
    const Emisor uno = llenoDe("brasas", 1, 3);
    for (const bool puntosPrimero : {false, true}) {
        std::vector<Destello> out;
        std::size_t quads = 0, descartados = 0;
        auto suma = [&](const Emisor& e) {
            const ResumenProyeccion r = proyectarParticulas(e, est, kTope - quads, out);
            quads += r.quads;
            descartados += r.descartados;
            return r;
        };
        if (puntosPrimero) { MOTOR_CHECK(suma(puntos).quads == 32u); MOTOR_CHECK(suma(calaveras).quads == 240u * 34u); }
        else               { MOTOR_CHECK(suma(calaveras).quads == 240u * 34u); MOTOR_CHECK(suma(puntos).quads == 32u); }
        MOTOR_CHECK(quads == kTope && descartados == 0u);                     // 8192 exactos, todo admitido
        MOTOR_CHECK(out.size() == 240u + 32u);
        MOTOR_CHECK(quadsDeDestellos(out) == kTope && quadsComoPintaDestellos(out) == kTope);
        const ResumenProyeccion r = suma(uno);                                // la unidad 8193: se descarta
        MOTOR_CHECK(r.quads == 0u && r.destellos == 0u && r.descartados == 1u);
        MOTOR_CHECK(quads == kTope && out.size() == 272u);
    }
    // Frontera dentro de un solo emisor: con quadsMax == 240*34 caben las 240; con 240*34 - 1 caben 239 y se descarta 1.
    std::vector<Destello> out;
    ResumenProyeccion r = proyectarParticulas(calaveras, est, 240u * 34u, out);
    MOTOR_CHECK(r.destellos == 240u && r.quads == 240u * 34u && r.descartados == 0u);
    out.clear();
    r = proyectarParticulas(calaveras, est, 240u * 34u - 1u, out);
    MOTOR_CHECK(r.destellos == 239u && r.quads == 239u * 34u && r.descartados == 1u);
    out.clear();
    // 241 calaveras contra el tope de 8192: la 241.a (34) no cabe en los 32 que sobran.
    const Emisor c241 = llenoDe("veneno", 241, 4);
    r = proyectarParticulas(c241, est, kTope, out);
    MOTOR_CHECK(r.destellos == 240u && r.quads == 8160u && r.descartados == 1u);
    out.clear();
    r = proyectarParticulas(calaveras, est, 0u, out);                          // tope 0: todo descartado
    MOTOR_CHECK(r.destellos == 0u && r.quads == 0u && r.descartados == 240u && out.empty());
    std::printf("[E08] frontera quads: 240 calaveras + 32 puntos = 8192 admitidos enteros; el 8193 se descarta\n");
}

// ---- Hallazgo 5: recortar la proyeccion no altera la simulacion ----
MOTOR_TEST_CASE(proyectar_con_distinto_presupuesto_no_cambia_la_simulacion) {
    PresupuestoParticulas p0, p1, p2;
    Emisor e0(defCarga(31)), e1(defCarga(31)), e2(defCarga(31));   // idénticos: veneno, 256, Sube (usa k)
    const EstiloEmisor est = estiloPleno();
    std::vector<Destello> o0, o1, o2;
    std::size_t desc0 = 0, desc1 = 0, desc2 = 0;
    auto tick = [&](bool proyecta) {
        MOTOR_CHECK(avanzarEmisor(e0, kTick, 1.0, p0).isOk());
        MOTOR_CHECK(avanzarEmisor(e1, kTick, 1.0, p1).isOk());
        MOTOR_CHECK(avanzarEmisor(e2, kTick, 1.0, p2).isOk());
        if (proyecta) {
            o0.clear(); o1.clear(); o2.clear();
            desc0 += proyectarParticulas(e0, est, 0u, o0).descartados;                                   // todo fuera
            desc1 += proyectarParticulas(e1, est, static_cast<std::size_t>(kMaxQuadsPorFotograma), o1).descartados;
            desc2 += proyectarParticulas(e2, est, static_cast<std::size_t>(1) << 40, o2).descartados;    // sobra
        }
        const std::uint64_t h = hashDeUno(e2);
        MOTOR_CHECK(hashDeUno(e0) == h && hashDeUno(e1) == h);
        MOTOR_CHECK(p0.vivas == p2.vivas && p1.vivas == p2.vivas);
    };
    for (int i = 0; i < 240; ++i) tick(true);            // 4 s con proyeccion recortada cada tick
    MOTOR_CHECK(e2.buffer().vivas() == static_cast<std::size_t>(kMaxPorEmisor));
    MOTOR_CHECK(desc0 > 0u && desc1 > 0u && desc2 == 0u);   // el recorte SI actuo (0 y 8192), y con holgura no
    MOTOR_CHECK(o0.empty() && !o2.empty());
    for (int i = 0; i < 120; ++i) tick(false);           // y la evolucion posterior sigue siendo identica
    MOTOR_CHECK(e0.nacidas() == e2.nacidas() && e1.nacidas() == e2.nacidas());
}

// Camina sin parar: un paso cada 0,16 s, con entradas y salidas de combate (D-O7) y con/sin movimiento reducido.
struct Camino {
    std::uint64_t hash = 0;
    std::size_t maxEmisores = 0, maxVivas = 0, maxQuads = 0;
    std::uint64_t pasosEnCombate = 0;
    bool incoherente = false, polvoEnCombate = false;
    std::vector<double> ms;
};

Camino camina(std::uint32_t llavor, bool reducido, double segundos) {
    PasosDePolvo p;
    p.monta(llavor, 64, 32, nullptr);
    Camino c;
    int x = 0, dir = 1;
    double acumulado = 0.0;
    const int ticks = static_cast<int>(segundos / kTick);
    std::vector<Destello> out;
    for (int t = 0; t < ticks; ++t) {
        const double seg = t * kTick;
        const bool combate = (seg >= 8.0 && seg < 12.0) || (seg >= 20.0 && seg < 21.0);
        acumulado += kTick;
        if (acumulado >= 0.16) {
            acumulado -= 0.16;
            if (combate) x += 4 * dir; else x += dir;   // en combate el heroe se mueve varias casillas por tick
            if (x > 40 || x < 0) { dir = -dir; x += 2 * dir; }
        }
        const std::uint64_t antes = p.pasos();
        const auto t0 = Reloj::now();
        p.avanza(kTick, GridCoord{x, 3}, reducido, combate);
        c.ms.push_back(msDesde(t0));
        if (combate && p.pasos() != antes) c.polvoEnCombate = true;
        std::size_t suma = 0;
        for (const Emisor& e : p.emisores()) suma += e.buffer().vivas();
        if (suma != p.presupuesto().vivas) c.incoherente = true;
        c.maxEmisores = std::max(c.maxEmisores, p.emisores().size());
        c.maxVivas = std::max(c.maxVivas, p.presupuesto().vivas);
        out.clear();
        std::size_t quads = 0;
        for (const Emisor& e : p.emisores())
            quads += proyectarParticulas(e, estiloDePolvo(), kMaxQuadsPorFotograma - quads, out).quads;
        if (quadsDeDestellos(out) != quadsComoPintaDestellos(out)) c.incoherente = true;
        c.maxQuads = std::max(c.maxQuads, quads);
    }
    c.pasosEnCombate = p.pasos();
    c.hash = hashDe(p.emisores(), p.presupuesto());
    return c;
}

MOTOR_TEST_CASE(caminar_sin_parar_mantiene_los_topes_del_polvo) {
    // Complementa los 10 s/20 pasos de test_polvo_pasos con 30 s, combate (D-O7) y proyeccion de quads.
    for (const bool reducido : {false, true}) {
        const Camino a = camina(7, reducido, 30.0);
        const Camino b = camina(7, reducido, 30.0);
        MOTOR_CHECK(a.hash == b.hash);                       // repetible
        MOTOR_CHECK(a.maxEmisores <= static_cast<std::size_t>(kMaxEmisoresPolvo));
        MOTOR_CHECK(a.maxVivas <= static_cast<std::size_t>(kPresupuestoPolvo));
        MOTOR_CHECK(!a.incoherente && !a.polvoEnCombate);    // sin polvo ni pasos nuevos en combate
        MOTOR_CHECK(a.maxQuads <= static_cast<std::size_t>(kPresupuestoPolvo) * 4);   // Voluta = 4 quads
        MOTOR_CHECK(a.maxEmisores > 0u && a.maxVivas > 0u && a.maxQuads > 0u);        // el recorrido SI levanta polvo
        // Caracterizacion del recorrido determinista (semilla 7, 30 s): valores medidos, no topes. Si cambia el
        // polvo (definicionDePolvo, estiloDePolvo, semillas) cambian a proposito y hay que releerlos.
        MOTOR_CHECK(a.maxEmisores == 7u);
        MOTOR_CHECK(a.maxVivas == (reducido ? 6u : 31u));
        MOTOR_CHECK(a.maxQuads == (reducido ? 24u : 124u));
        std::printf("[E08] polvo 30 s paso/0,16 s %s: emisores max %zu (tope %d), vivas max %zu (tope %d), quads max %zu, "
                    "CPU Debug avanza mediana %.4f ms max %.4f ms\n",
                    reducido ? "reducido" : "normal", a.maxEmisores, kMaxEmisoresPolvo, a.maxVivas, kPresupuestoPolvo,
                    a.maxQuads, mediana(a.ms), maximo(a.ms));
    }
    // 17.o paso seguido sin esperar: no pasa de 16 nubes (kMaxEmisoresPolvo)
    PasosDePolvo p;
    p.monta(3, 64, 32, nullptr);
    p.avanza(kTick, GridCoord{0, 0}, false);
    for (int i = 1; i <= 40; ++i) p.avanza(kTick, GridCoord{i, 0}, false);   // un paso por tick: 40 nubes pedidas
    MOTOR_CHECK(p.emisores().size() <= static_cast<std::size_t>(kMaxEmisoresPolvo));
    MOTOR_CHECK(p.presupuesto().vivas <= static_cast<std::size_t>(kPresupuestoPolvo));
    MOTOR_CHECK(p.pasos() == 40u);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
