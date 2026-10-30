#include "Game/ShopEngine.h"

#include <algorithm>

namespace Game {

ShopEngine::ShopEngine(const std::vector<ShopItem>& items, int startGold, float moralBase)
    : m_items(items), m_gold(startGold), m_moral(moralBase) {}

int ShopEngine::gold() const { return m_gold; }
float ShopEngine::moral() const { return m_moral; }
const std::vector<ShopItem>& ShopEngine::items() const { return m_items; }

ShopItem* ShopEngine::find(const std::string& id) {
    for (auto& it : m_items) if (it.id == id) return &it;
    return nullptr;
}

const ShopItem* ShopEngine::find(const std::string& id) const {
    for (const auto& it : m_items) if (it.id == id) return &it;
    return nullptr;
}

ShopTransaction ShopEngine::buy(const std::string& id, int qty, int buyerLevel) {
    if (qty <= 0) return ShopTransaction{false, "Cantidad invalida", m_gold, 0, 0};
    const ShopItem* it = find(id);
    if (!it) return ShopTransaction{false, "Item no existe", m_gold, 0, 0};
    if (!it->vendible) return ShopTransaction{false, "Item no vendible", m_gold, 0, 0};
    if (it->stock < qty) return ShopTransaction{false, "Stock insuficiente", m_gold, 0, 0};
    if (buyerLevel < it->requiredLevel)
        return ShopTransaction{false, "Nivel insuficiente", m_gold, 0, 0};
    const int total = it->price * qty;
    if (m_gold < total) return ShopTransaction{false, "Oro insuficiente", m_gold, 0, 0};
    // Actualizar — NO doble resta (cálculo antes de aplicar, como pide REV-03).
    m_gold -= total;
    for (auto& i : m_items) if (i.id == id) { i.stock -= qty; break; }
    const int moralGain = (it->moralImpact >= 0.05f) ? 1 : 0;
    if (moralGain) m_moral = std::min(1.0f, m_moral + 0.05f * qty);
    return ShopTransaction{true, "Compra exitosa", m_gold, moralGain, qty};
}

ShopTransaction ShopEngine::sell(const std::string& id, int qty) {
    if (qty <= 0) return ShopTransaction{false, "Cantidad invalida", m_gold, 0, 0};
    ShopItem* it = find(id);
    if (!it) return ShopTransaction{false, "Item no existe", m_gold, 0, 0};
    if (!it->vendible || it->price <= 0)
        return ShopTransaction{false, "Item no se vende", m_gold, 0, 0};
    const int refund = (it->price / 2) * qty;
    m_gold += refund;
    for (auto& i : m_items) if (i.id == id) { i.stock += qty; break; }
    return ShopTransaction{true, "Venta exitosa", m_gold, 0, -qty};
}

}  // namespace Game
