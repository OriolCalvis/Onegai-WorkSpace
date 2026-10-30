#include "Map/RegionPresence.h"

#include <algorithm>
#include <variant>

namespace Regiones {

bool contains(const GridBounds& bounds, GridCoord cell) {
    if (bounds.isEmpty()) return false;
    return cell.x >= bounds.minX && cell.x <= bounds.maxX && cell.y >= bounds.minY &&
           cell.y <= bounds.maxY;
}

std::vector<RegionRef> regionsAt(const RegionLayerDocument& layer, const ids::LayerId& capa,
                                 GridCoord cell) {
    std::vector<RegionRef> out;
    for (const MapRegionDocument& r : layer.regions()) {
        if (contains(r.bounds, cell)) out.push_back(RegionRef{capa, r.id});
    }
    return out;
}

std::vector<RegionRef> regionsAt(const MapDocument& map, GridCoord cell) {
    std::vector<RegionRef> out;
    if (cell.x < 0 || cell.y < 0 || cell.x >= map.width() || cell.y >= map.height()) return out;
    for (const MapLayer& layer : map.layers()) {
        if (layer.type() != MapLayerType::Region) continue;
        const auto* data = std::get_if<RegionLayerDocument>(&layer.data());
        if (data == nullptr) continue;
        const std::vector<RegionRef> here = regionsAt(*data, layer.meta().id, cell);
        out.insert(out.end(), here.begin(), here.end());
    }
    return out;
}

namespace {
bool has(const std::vector<RegionRef>& v, const RegionRef& r) {
    return std::find(v.begin(), v.end(), r) != v.end();
}
}  // namespace

RegionDelta regionDelta(const std::vector<RegionRef>& before, const std::vector<RegionRef>& now) {
    RegionDelta d;
    for (const RegionRef& r : now) {
        if (has(d.entered, r) || has(d.stayed, r)) continue;
        (has(before, r) ? d.stayed : d.entered).push_back(r);
    }
    for (const RegionRef& r : before) {
        if (has(now, r) || has(d.exited, r)) continue;
        d.exited.push_back(r);
    }
    return d;
}

namespace {
int intervalFor(const StayPolicy& p, const RegionRef& r) {
    for (const StayOverride& o : p.overrides) {
        if (o.ref == r) return o.intervalMs;
    }
    return p.intervalMs;
}
}  // namespace

Result<StayBatch> stayEvents(RegionPresence& presence, const std::vector<RegionRef>& now, int dtMs,
                             const StayPolicy& policy) {
    using R = Result<StayBatch>;
    if (dtMs < 0) return R::Error("stayEvents: dtMs negativo");
    if (policy.intervalMs <= 0) return R::Error("stayEvents: intervalMs <= 0");
    for (const StayOverride& o : policy.overrides) {
        if (o.intervalMs <= 0) return R::Error("stayEvents: override con intervalMs <= 0");
    }
    if (presence.stayMs.size() != presence.current.size()) {
        return R::Error("stayEvents: stayMs y current con distinta longitud");
    }
    for (int ms : presence.stayMs) {
        if (ms < 0) return R::Error("stayEvents: stayMs negativo");
    }
    const RegionDelta delta = regionDelta(presence.current, now);
    StayBatch batch;
    std::vector<RegionRef> cur;
    std::vector<int> ms;
    for (const RegionRef& r : now) {
        if (has(cur, r)) continue;
        int acc = 0;  // entered: empieza en 0
        if (has(delta.stayed, r)) {
            const size_t k = std::find(presence.current.begin(), presence.current.end(), r) -
                             presence.current.begin();
            acc = presence.stayMs[k];
            const int interval = intervalFor(policy, r);
            // Sin sumar (evita desbordar int): acumulado + dt >= intervalo.
            if (dtMs > 0 && (dtMs >= interval - acc || acc >= interval)) {
                batch.fired.push_back(r);
                acc = 0;
            } else if (dtMs > 0) {
                acc += dtMs;
            }
        }
        cur.push_back(r);
        ms.push_back(acc);
    }
    presence.current = std::move(cur);
    presence.stayMs = std::move(ms);
    presence.known = true;
    return R::Ok(batch);
}

std::vector<RegionTrigger> orderRegionEvents(const RegionDelta& delta, const StayBatch& stays,
                                             const RegionPolicy& policy) {
    using K = RegionTrigger::Kind;
    std::vector<RegionTrigger> out;
    auto add = [&out](K kind, const std::vector<RegionRef>& group) {
        for (const RegionRef& r : group) out.push_back(RegionTrigger{kind, r});
    };
    if (policy.exitsFirst) {
        add(K::Exit, delta.exited);
        add(K::Enter, delta.entered);
    } else {
        add(K::Enter, delta.entered);
        add(K::Exit, delta.exited);
    }
    for (const RegionRef& r : stays.fired) {
        // fired fuera de stayed: incoherente, se ignora.
        if (std::find(delta.stayed.begin(), delta.stayed.end(), r) != delta.stayed.end())
            out.push_back(RegionTrigger{K::Stay, r});
    }
    return out;
}

}  // namespace Regiones
