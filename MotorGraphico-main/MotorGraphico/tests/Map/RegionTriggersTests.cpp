#include "Map/RegionPresence.h"

#include "TestHarness.h"

#include <algorithm>
#include <climits>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

// U3-02.E02..E05, E07 — Regiones: pertenencia, delta, permanencia y orden de triggers.
// Defiende: borde inclusivo en las 4 caras, orden estable, identidad compuesta
// (capa, region) y que fuera del mapa no haya nada.

using Regiones::RegionRef;
using Regiones::contains;
using Regiones::regionsAt;
using Regiones::RegionDelta;
using Regiones::regionDelta;
using Regiones::RegionPresence;
using Regiones::StayPolicy;
using Regiones::stayEvents;
using Regiones::RegionTrigger;
using Regiones::RegionPolicy;
using Regiones::StayBatch;
using Regiones::orderRegionEvents;
using Regiones::resetRegionPresence;

namespace {

ids::LayerId lyr(const std::string& v) { return ids::LayerId::parse(v).value(); }
ids::RegionId rid(const std::string& v) { return ids::RegionId::parse(v).value(); }

GridBounds caja(int x0, int x1, int y0, int y1) { return GridBounds{x0, x1, y0, y1}; }
MapRegionDocument reg(const std::string& id, GridBounds b) { return MapRegionDocument{rid(id), id, b}; }
RegionRef ref(const std::string& capa, const std::string& id) { return RegionRef{lyr(capa), rid(id)}; }

MapLayer capaRegiones(const std::string& id, int order, std::vector<MapRegionDocument> rs) {
    MapLayerMeta m;
    m.id = lyr(id);
    m.name = id;
    m.order = order;
    return MapLayer::regions(m, RegionLayerDocument(std::move(rs)));
}

MapDocument mapa(int w, int h) {
    return MapDocument(ids::MapId::parse("mapa").value(), "mapa", w, h, 64, 32);
}

MOTOR_TEST_CASE(contains_es_inclusivo_en_las_cuatro_caras) {
    const GridBounds b = caja(2, 4, 3, 5);
    MOTOR_CHECK(contains(b, GridCoord{2, 3}));
    MOTOR_CHECK(contains(b, GridCoord{4, 5}));
    MOTOR_CHECK(contains(b, GridCoord{3, 4}));
    MOTOR_CHECK(contains(b, GridCoord{2, 5}));
    MOTOR_CHECK(contains(b, GridCoord{4, 3}));
    MOTOR_CHECK(!contains(b, GridCoord{1, 4}));  // justo fuera de minX
    MOTOR_CHECK(!contains(b, GridCoord{5, 4}));  // justo fuera de maxX
    MOTOR_CHECK(!contains(b, GridCoord{3, 2}));  // justo fuera de minY
    MOTOR_CHECK(!contains(b, GridCoord{3, 6}));  // justo fuera de maxY
}

MOTOR_TEST_CASE(contains_con_bounds_vacios_o_coordenadas_negativas) {
    MOTOR_CHECK(!contains(GridBounds{}, GridCoord{0, 0}));
    MOTOR_CHECK(!contains(caja(3, 1, 0, 5), GridCoord{2, 2}));
    MOTOR_CHECK(contains(caja(-2, 0, -2, 0), GridCoord{-2, -1}));
    MOTOR_CHECK(!contains(caja(0, 3, 0, 3), GridCoord{-1, 0}));
    MOTOR_CHECK(!contains(caja(0, 3, 0, 3), GridCoord{0, -1}));
}

MOTOR_TEST_CASE(una_capa_vacia_no_devuelve_nada) {
    MOTOR_CHECK(regionsAt(RegionLayerDocument{}, lyr("r"), GridCoord{0, 0}).empty());
}

MOTOR_TEST_CASE(dos_regiones_solapadas_salen_en_orden_de_declaracion) {
    const RegionLayerDocument capa({reg("b", caja(0, 3, 0, 3)), reg("a", caja(2, 5, 2, 5))});
    const std::vector<RegionRef> r = regionsAt(capa, lyr("r"), GridCoord{3, 3});
    MOTOR_CHECK(r.size() == 2);
    MOTOR_CHECK(r[0] == ref("r", "b"));
    MOTOR_CHECK(r[1] == ref("r", "a"));
    MOTOR_CHECK(regionsAt(capa, lyr("r"), GridCoord{0, 0}).size() == 1);
    MOTOR_CHECK(regionsAt(capa, lyr("r"), GridCoord{9, 9}).empty());
}

MOTOR_TEST_CASE(capa_suelta_no_aplica_limites_de_mapa) {
    // La capa aislada no conoce el mapa: una region en negativos se encuentra.
    const RegionLayerDocument capa({reg("z", caja(-3, -1, -3, -1))});
    MOTOR_CHECK(regionsAt(capa, lyr("r"), GridCoord{-2, -2}).size() == 1);
}

MOTOR_TEST_CASE(el_mismo_id_en_dos_capas_da_dos_refs_distintas_en_orden_de_capas) {
    MapDocument m = mapa(8, 8);
    MOTOR_CHECK(m.addLayer(capaRegiones("capa_a", 0, {reg("zona", caja(1, 3, 1, 3))})).isOk());
    MOTOR_CHECK(m.addLayer(capaRegiones("capa_b", 1, {reg("zona", caja(2, 4, 2, 4))})).isOk());
    const std::vector<RegionRef> r = regionsAt(m, GridCoord{2, 2});
    MOTOR_CHECK(r.size() == 2);
    MOTOR_CHECK(r[0] == ref("capa_a", "zona"));
    MOTOR_CHECK(r[1] == ref("capa_b", "zona"));
    MOTOR_CHECK(!(r[0] == r[1]));
    MOTOR_CHECK(r[0] < r[1]);
    MOTOR_CHECK(!(r[1] < r[0]));
    const std::vector<RegionRef> soloB = regionsAt(m, GridCoord{4, 4});
    MOTOR_CHECK(soloB.size() == 1 && soloB[0] == ref("capa_b", "zona"));
}

MOTOR_TEST_CASE(refs_se_ordenan_por_capa_y_luego_region) {
    MOTOR_CHECK(ref("a", "z") < ref("b", "a"));
    MOTOR_CHECK(ref("a", "a") < ref("a", "b"));
    MOTOR_CHECK(ref("a", "a") != ref("a", "b"));
}

MOTOR_TEST_CASE(las_capas_que_no_son_de_regiones_se_ignoran) {
    MapDocument m = mapa(4, 4);
    MapLayerMeta mt;
    mt.id = lyr("suelo");
    mt.name = "suelo";
    mt.order = 0;
    MOTOR_CHECK(m.addLayer(MapLayer::tiles(mt, TileLayerDocument(4, 4, std::vector<int>(16, 1)))).isOk());
    MOTOR_CHECK(m.addLayer(capaRegiones("zonas", 1, {reg("x", caja(0, 3, 0, 3))})).isOk());
    const std::vector<RegionRef> r = regionsAt(m, GridCoord{1, 1});
    MOTOR_CHECK(r.size() == 1 && r[0] == ref("zonas", "x"));
}

MOTOR_TEST_CASE(fuera_del_mapa_no_hay_regiones_y_los_bordes_del_mapa_si) {
    // Un MapDocument valido no admite regiones fuera del mapa, asi que el
    // caso "region que reclama una casilla exterior" no es construible aqui;
    // se defiende la guarda con una region que cubre todo el mapa.
    MapDocument m = mapa(4, 3);
    MOTOR_CHECK(m.addLayer(capaRegiones("zonas", 0, {reg("todo", caja(0, 3, 0, 2))})).isOk());
    MOTOR_CHECK(regionsAt(m, GridCoord{0, 0}).size() == 1);
    MOTOR_CHECK(regionsAt(m, GridCoord{3, 2}).size() == 1);
    MOTOR_CHECK(regionsAt(m, GridCoord{4, 2}).empty());
    MOTOR_CHECK(regionsAt(m, GridCoord{3, 3}).empty());
    MOTOR_CHECK(regionsAt(m, GridCoord{-1, 0}).empty());
    MOTOR_CHECK(regionsAt(m, GridCoord{0, -1}).empty());
}

MOTOR_TEST_CASE(mapa_sin_capas_de_regiones_devuelve_vacio) {
    MOTOR_CHECK(regionsAt(mapa(4, 4), GridCoord{1, 1}).empty());
}

MOTOR_TEST_CASE(capas_insertadas_en_orden_inverso_siguen_el_orden_de_layers) {
    MapDocument m = mapa(8, 8);
    MOTOR_CHECK(m.addLayer(capaRegiones("capa_b", 5, {reg("zona", caja(0, 3, 0, 3))})).isOk());
    MOTOR_CHECK(m.addLayer(capaRegiones("capa_a", 1, {reg("zona", caja(0, 3, 0, 3))})).isOk());
    const std::vector<RegionRef> esperado = [&] {
        std::vector<RegionRef> v;
        for (const MapLayer& l : m.layers()) v.push_back(ref(l.meta().id.value(), "zona"));
        return v;
    }();
    const std::vector<RegionRef> r = regionsAt(m, GridCoord{1, 1});
    MOTOR_CHECK(r.size() == 2);
    MOTOR_CHECK(r == esperado);
    MOTOR_CHECK(r[0] == ref("capa_a", "zona"));  // order 1 antes que order 5
}

using V = std::vector<RegionRef>;

MOTOR_TEST_CASE(delta_vacio_a_una_region_es_enter) {
    const RegionDelta d = regionDelta({}, V{ref("c", "a")});
    MOTOR_CHECK(d.entered == V{ref("c", "a")});
    MOTOR_CHECK(d.stayed.empty() && d.exited.empty());
}

MOTOR_TEST_CASE(delta_misma_region_es_stay) {
    const RegionDelta d = regionDelta(V{ref("c", "a")}, V{ref("c", "a")});
    MOTOR_CHECK(d.stayed == V{ref("c", "a")});
    MOTOR_CHECK(d.entered.empty() && d.exited.empty());
}

MOTOR_TEST_CASE(delta_region_a_vacio_es_exit) {
    const RegionDelta d = regionDelta(V{ref("c", "a")}, {});
    MOTOR_CHECK(d.exited == V{ref("c", "a")});
    MOTOR_CHECK(d.entered.empty() && d.stayed.empty());
}

MOTOR_TEST_CASE(delta_mixto_ab_a_bc) {
    const RegionDelta d = regionDelta(V{ref("c", "a"), ref("c", "b")}, V{ref("c", "b"), ref("c", "c")});
    MOTOR_CHECK(d.exited == V{ref("c", "a")});
    MOTOR_CHECK(d.stayed == V{ref("c", "b")});
    MOTOR_CHECK(d.entered == V{ref("c", "c")});
}

MOTOR_TEST_CASE(delta_mismo_id_en_otra_capa_es_exit_mas_enter) {
    const RegionDelta d = regionDelta(V{ref("capa_a", "zona")}, V{ref("capa_b", "zona")});
    MOTOR_CHECK(d.exited == V{ref("capa_a", "zona")});
    MOTOR_CHECK(d.entered == V{ref("capa_b", "zona")});
    MOTOR_CHECK(d.stayed.empty());
}

MOTOR_TEST_CASE(delta_con_duplicados_en_la_entrada_no_duplica_salida) {
    const RegionDelta d = regionDelta(V{ref("c", "a"), ref("c", "a"), ref("c", "x"), ref("c", "x")},
                                      V{ref("c", "a"), ref("c", "a"), ref("c", "n"), ref("c", "n")});
    MOTOR_CHECK(d.stayed == V{ref("c", "a")});
    MOTOR_CHECK(d.entered == V{ref("c", "n")});
    MOTOR_CHECK(d.exited == V{ref("c", "x")});
}

MOTOR_TEST_CASE(delta_entrada_desordenada_da_el_mismo_conjunto_y_orden_de_now_y_before) {
    const V b1{ref("c", "a"), ref("c", "b"), ref("d", "k")};
    const V n1{ref("c", "b"), ref("c", "z"), ref("d", "m")};
    V b2 = b1, n2 = n1;
    std::reverse(b2.begin(), b2.end());
    std::reverse(n2.begin(), n2.end());
    RegionDelta d1 = regionDelta(b1, n1), d2 = regionDelta(b2, n2);
    // Orden documentado: entered/stayed siguen `now`, exited sigue `before`.
    MOTOR_CHECK(d1.entered == V({ref("c", "z"), ref("d", "m")}));
    MOTOR_CHECK(d2.entered == V({ref("d", "m"), ref("c", "z")}));
    MOTOR_CHECK(d1.exited == V({ref("c", "a"), ref("d", "k")}));
    for (V* v : {&d1.entered, &d1.stayed, &d1.exited, &d2.entered, &d2.stayed, &d2.exited})
        std::sort(v->begin(), v->end());
    MOTOR_CHECK(d1.entered == d2.entered);
    MOTOR_CHECK(d1.stayed == d2.stayed);
    MOTOR_CHECK(d1.exited == d2.exited);
}

MOTOR_TEST_CASE(delta_grupos_disjuntos) {
    const V b{ref("c", "a"), ref("c", "b"), ref("d", "a")};
    const V n{ref("c", "b"), ref("d", "a"), ref("d", "b"), ref("c", "a")};
    const RegionDelta d = regionDelta(b, n);
    V todos;
    for (const V* g : {&d.entered, &d.stayed, &d.exited}) todos.insert(todos.end(), g->begin(), g->end());
    std::sort(todos.begin(), todos.end());
    MOTOR_CHECK(std::adjacent_find(todos.begin(), todos.end()) == todos.end());
    MOTOR_CHECK(d.stayed.size() == 3 && d.entered.size() == 1 && d.exited.empty());
}

// U3-02.E04 — Permanencia sin inundar (D7): cada N ms, sin catch-up.
RegionPresence pres(std::vector<RegionRef> rs, std::vector<int> ms) {
    RegionPresence p;
    p.current = std::move(rs);
    p.stayMs = std::move(ms);
    p.known = true;
    return p;
}

MOTOR_TEST_CASE(stay_acumula_por_debajo_de_N_y_dispara_al_llegar) {
    const V a{ref("c", "a")};
    RegionPresence p = pres(a, {0});
    StayPolicy pol;  // 1000
    auto r1 = stayEvents(p, a, 400, pol);
    MOTOR_CHECK(r1.isOk() && r1.value().fired.empty());
    MOTOR_CHECK(p.stayMs == std::vector<int>({400}));
    auto r2 = stayEvents(p, a, 599, pol);
    MOTOR_CHECK(r2.isOk() && r2.value().fired.empty());
    auto r3 = stayEvents(p, a, 1, pol);  // 1000 exactos
    MOTOR_CHECK(r3.isOk() && r3.value().fired == a);
    MOTOR_CHECK(p.stayMs == std::vector<int>({0}));
    MOTOR_CHECK(p.current.size() == p.stayMs.size());
}

MOTOR_TEST_CASE(stay_dt_enorme_dispara_uno_y_no_hay_catch_up_entre_llamadas) {
    const V ab{ref("c", "a"), ref("c", "b")};
    RegionPresence p = pres(ab, {0, 0});
    auto r = stayEvents(p, ab, 10 * 1000, StayPolicy{});
    MOTOR_CHECK(r.isOk() && r.value().fired == ab);
    auto big = stayEvents(p, ab, 2000000000, StayPolicy{});  // sin desbordar
    MOTOR_CHECK(big.isOk() && big.value().fired.size() == 2);
    // El resto no se arrastra: tras 2500 ms, 1 ms mas NO dispara.
    const V a{ref("c", "a")};
    RegionPresence q = pres(a, {0});
    auto g = stayEvents(q, a, 2500, StayPolicy{});
    MOTOR_CHECK(g.isOk() && g.value().fired == a);
    auto s1 = stayEvents(q, a, 1, StayPolicy{});
    MOTOR_CHECK(s1.isOk() && s1.value().fired.empty());
    MOTOR_CHECK(q.stayMs == std::vector<int>({1}));
}

MOTOR_TEST_CASE(stay_intervalos_distintos_por_region_y_orden_de_presencia) {
    StayPolicy pol;
    pol.overrides.push_back({ref("c", "rapida"), 200});
    const V ls{ref("c", "lenta"), ref("c", "rapida")};
    RegionPresence p = pres(ls, {0, 0});
    auto r1 = stayEvents(p, ls, 300, pol);
    MOTOR_CHECK(r1.isOk() && r1.value().fired == V({ref("c", "rapida")}));
    auto r2 = stayEvents(p, ls, 700, pol);  // lenta: 300+700=1000; rapida: 0+700>=200
    MOTOR_CHECK(r2.isOk() && r2.value().fired == ls);
}

MOTOR_TEST_CASE(stay_entrar_no_cuenta_con_regionDelta_real) {
    const V a{ref("c", "a")}, ab{ref("c", "a"), ref("c", "b")};
    RegionPresence p = pres(a, {900});
    MOTOR_CHECK(regionDelta(p.current, ab).entered == V({ref("c", "b")}));
    auto r = stayEvents(p, ab, 1500, StayPolicy{});  // a: 900+1500 >= N; b entra con dt >= N
    MOTOR_CHECK(r.isOk() && r.value().fired == a);   // solo a dispara: entrar no cuenta
    MOTOR_CHECK(p.current == ab && p.stayMs == std::vector<int>({0, 0}));
    auto big = stayEvents(p, ab, 5000, StayPolicy{});
    // en la llamada siguiente b ya es stayed y si cuenta
    MOTOR_CHECK(big.isOk() && big.value().fired == ab);
    // Entrar con dt >= N tampoco dispara (dt=5000 > 1000).
    RegionPresence q = pres(a, {0});
    auto e = stayEvents(q, ab, 5000, StayPolicy{});
    MOTOR_CHECK(e.isOk() && e.value().fired == a);
    MOTOR_CHECK(q.stayMs == std::vector<int>({0, 0}));
}

MOTOR_TEST_CASE(stay_override_repetido_primera_coincidencia_gana) {
    const V a{ref("c", "a")};
    StayPolicy pol;
    pol.overrides.push_back({ref("c", "a"), 200});
    pol.overrides.push_back({ref("c", "a"), 5000});
    RegionPresence p = pres(a, {0});
    auto r = stayEvents(p, a, 300, pol);
    MOTOR_CHECK(r.isOk() && r.value().fired == a);
}

MOTOR_TEST_CASE(stay_desbordamiento_con_acumulado_positivo) {
    const V a{ref("c", "a")};
    RegionPresence p = pres(a, {500});
    auto r = stayEvents(p, a, INT_MAX, StayPolicy{});
    MOTOR_CHECK(r.isOk() && r.value().fired == a);
    MOTOR_CHECK(p.stayMs == std::vector<int>({0}));
}

MOTOR_TEST_CASE(stay_dt_cero_actualiza_pertenencia) {
    const V a{ref("c", "a")}, ab{ref("c", "a"), ref("c", "b")};
    RegionPresence p = pres(a, {400});
    auto r = stayEvents(p, ab, 0, StayPolicy{});
    MOTOR_CHECK(r.isOk() && r.value().fired.empty());
    MOTOR_CHECK(p.current == ab && p.stayMs == std::vector<int>({400, 0}));
}

// U3-02.E05 — Orden de simultaneos (D3 decidida: Exit, Enter, Stay).
using K = RegionTrigger::Kind;
std::vector<RegionTrigger> trig(std::vector<std::pair<K, RegionRef>> xs) {
    std::vector<RegionTrigger> o;
    for (auto& x : xs) o.push_back(RegionTrigger{x.first, x.second});
    return o;
}
bool same(const std::vector<RegionTrigger>& a, const std::vector<RegionTrigger>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i].kind != b[i].kind || a[i].ref != b[i].ref) return false;
    return true;
}

MOTOR_TEST_CASE(orden_exit_enter_stay_por_defecto_y_variante) {
    RegionDelta d;
    d.exited = {ref("c", "x2"), ref("c", "x1")};
    d.entered = {ref("c", "e2"), ref("c", "e1")};
    d.stayed = {ref("c", "s")};
    StayBatch sb;
    sb.fired = {ref("c", "s")};
    const auto def = orderRegionEvents(d, sb, RegionPolicy{});
    MOTOR_CHECK(same(def, trig({{K::Exit, ref("c", "x2")}, {K::Exit, ref("c", "x1")},
                                {K::Enter, ref("c", "e2")}, {K::Enter, ref("c", "e1")},
                                {K::Stay, ref("c", "s")}})));
    RegionPolicy p;
    p.exitsFirst = false;
    const auto alt = orderRegionEvents(d, sb, p);
    MOTOR_CHECK(same(alt, trig({{K::Enter, ref("c", "e2")}, {K::Enter, ref("c", "e1")},
                                {K::Exit, ref("c", "x2")}, {K::Exit, ref("c", "x1")},
                                {K::Stay, ref("c", "s")}})));
    MOTOR_CHECK(same(def, orderRegionEvents(d, sb, RegionPolicy{})));  // determinista
}

MOTOR_TEST_CASE(orden_frontera_salida_y_entrada_solapadas_con_regionDelta_real) {
    // Capas distintas, misma id; no se reordena por RegionId ni por capa.
    const V antes{ref("z", "a"), ref("m", "k")}, ahora{ref("z", "k"), ref("m", "k"), ref("z", "a")};
    const RegionDelta d = regionDelta(antes, ahora);
    MOTOR_CHECK(d.entered == V({ref("z", "k")}) && d.exited.empty());
    const V antes2{ref("z", "a"), ref("m", "k")}, ahora2{ref("q", "b"), ref("m", "k")};
    const RegionDelta d2 = regionDelta(antes2, ahora2);
    StayBatch sb;
    sb.fired = {ref("m", "k")};
    const auto o = orderRegionEvents(d2, sb, RegionPolicy{});
    MOTOR_CHECK(same(o, trig({{K::Exit, ref("z", "a")}, {K::Enter, ref("q", "b")},
                              {K::Stay, ref("m", "k")}})));
}

MOTOR_TEST_CASE(orden_stay_solo_de_fired_y_fired_incoherente_se_ignora) {
    RegionDelta d;
    d.stayed = {ref("c", "a"), ref("c", "b")};
    StayBatch sb;
    sb.fired = {ref("c", "b"), ref("c", "fantasma")};  // fantasma no esta en stayed
    const auto o = orderRegionEvents(d, sb, RegionPolicy{});
    MOTOR_CHECK(same(o, trig({{K::Stay, ref("c", "b")}})));
    MOTOR_CHECK(orderRegionEvents(RegionDelta{}, StayBatch{}, RegionPolicy{}).empty());
}

MOTOR_TEST_CASE(orden_varios_stay_siguen_el_orden_de_fired) {
    RegionDelta d;
    d.stayed = {ref("c", "a"), ref("c", "b"), ref("c", "c")};
    StayBatch sb;
    sb.fired = {ref("c", "c"), ref("c", "a")};
    const auto o = orderRegionEvents(d, sb, RegionPolicy{});
    MOTOR_CHECK(same(o, trig({{K::Stay, ref("c", "c")}, {K::Stay, ref("c", "a")}})));
}

// U3-02.E07 — Reinicio de presencia (D5): aparecer dentro dispara Enter.
MOTOR_TEST_CASE(reset_presencia_vacia_y_idempotente) {
    const RegionPresence r = resetRegionPresence();
    MOTOR_CHECK(!r.known && r.current.empty() && r.stayMs.empty());
    RegionPresence sucia = pres(V{ref("c", "a")}, {700});
    sucia = resetRegionPresence();
    const RegionPresence otra = resetRegionPresence();
    MOTOR_CHECK(!sucia.known && sucia.current == otra.current && sucia.stayMs == otra.stayMs);
    RegionPresence vacia;
    vacia = resetRegionPresence();
    MOTOR_CHECK(!vacia.known && vacia.current.empty() && vacia.stayMs.empty());
}

MOTOR_TEST_CASE(reset_cambio_de_mapa_solo_enter_sin_exit_ni_stay) {
    // Mapa A: a y b con acumulados; mapa B: la MISMA RegionRef a mas una nueva.
    const V mapaA{ref("c", "a"), ref("c", "b")}, mapaB{ref("c", "a"), ref("d", "n")};
    RegionPresence p = pres(mapaA, {900, 400});
    p = resetRegionPresence();
    const RegionDelta d = regionDelta(p.current, mapaB);
    MOTOR_CHECK(d.entered == mapaB && d.stayed.empty() && d.exited.empty());
    auto r = stayEvents(p, mapaB, 5000, StayPolicy{});
    MOTOR_CHECK(r.isOk() && r.value().fired.empty());  // entrar no cuenta, aunque dt >= N
    MOTOR_CHECK(p.known && p.current == mapaB && p.stayMs == std::vector<int>({0, 0}));
    const auto o = orderRegionEvents(d, r.value(), RegionPolicy{});
    MOTOR_CHECK(same(o, trig({{K::Enter, ref("c", "a")}, {K::Enter, ref("d", "n")}})));
}

MOTOR_TEST_CASE(reset_seguido_de_dt_cero_no_dispara_stay) {
    const V a{ref("c", "a")};
    RegionPresence p = pres(a, {999});
    p = resetRegionPresence();
    auto r = stayEvents(p, a, 0, StayPolicy{});
    MOTOR_CHECK(r.isOk() && r.value().fired.empty());
    MOTOR_CHECK(p.current == a && p.stayMs == std::vector<int>({0}));
    MOTOR_CHECK(regionDelta(resetRegionPresence().current, a).entered == a);
    // Mapa nuevo sin regiones en la casilla: nada, y tampoco Exit del viejo.
    RegionPresence q = pres(a, {10});
    q = resetRegionPresence();
    MOTOR_CHECK(regionDelta(q.current, V{}).exited.empty());
}

MOTOR_TEST_CASE(stay_region_que_sale_no_dispara) {
    const V a{ref("c", "a")}, ab{ref("c", "a"), ref("c", "b")};
    RegionPresence p = pres(ab, {900, 999});
    auto r = stayEvents(p, a, 100, StayPolicy{});  // b sale justo en el umbral
    MOTOR_CHECK(r.isOk() && r.value().fired == a);
    MOTOR_CHECK(p.current == a && p.stayMs == std::vector<int>({0}));
    RegionPresence q = pres(a, {900});
    auto s = stayEvents(q, V{}, 100, StayPolicy{});  // sale la unica region
    MOTOR_CHECK(s.isOk() && s.value().fired.empty());
    MOTOR_CHECK(q.current.empty() && q.stayMs.empty());
    // Vuelve a entrar: empieza de cero, no recupera el contador viejo.
    auto e = stayEvents(q, a, 100, StayPolicy{});
    MOTOR_CHECK(e.isOk() && e.value().fired.empty() && q.stayMs == std::vector<int>({0}));
}

MOTOR_TEST_CASE(stay_errores_y_pausa) {
    const V a{ref("c", "a")};
    RegionPresence p = pres(a, {500});
    MOTOR_CHECK(!stayEvents(p, a, -1, StayPolicy{}).isOk());
    StayPolicy cero;
    cero.intervalMs = 0;
    MOTOR_CHECK(!stayEvents(p, a, 10, cero).isOk());
    StayPolicy ov;
    ov.overrides.push_back({ref("c", "a"), -5});
    MOTOR_CHECK(!stayEvents(p, a, 10, ov).isOk());
    RegionPresence mal = pres(a, {});
    MOTOR_CHECK(!stayEvents(mal, a, 10, StayPolicy{}).isOk());
    RegionPresence neg = pres(a, {-1});
    MOTOR_CHECK(!stayEvents(neg, a, 10, StayPolicy{}).isOk());
    MOTOR_CHECK(neg.stayMs == std::vector<int>({-1}));
    MOTOR_CHECK(p.stayMs == std::vector<int>({500}));  // los errores no mutan
    auto pausa = stayEvents(p, a, 0, StayPolicy{});
    MOTOR_CHECK(pausa.isOk() && pausa.value().fired.empty());
    MOTOR_CHECK(p.stayMs == std::vector<int>({500}));
    // dt=0 con acumulado ya >= intervalo: no dispara ni cambia el estado.
    RegionPresence lleno = pres(a, {1500});
    auto z = stayEvents(lleno, a, 0, StayPolicy{});
    MOTOR_CHECK(z.isOk() && z.value().fired.empty());
    MOTOR_CHECK(lleno.stayMs == std::vector<int>({1500}));
    RegionPresence vacia;
    auto rv = stayEvents(vacia, V{}, 5000, StayPolicy{});
    MOTOR_CHECK(rv.isOk() && rv.value().fired.empty());
}

}  // namespace

int main() { return motor_test::runAllTests(); }
