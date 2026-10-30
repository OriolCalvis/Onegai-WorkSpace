// H1-02 (Gantt Fase H) — rescate de G-14: ShopEngine/ShopItem/
// ShopTransaction vivían solo dentro de tests/Game/G14HudTimersAndCommerceTests.cpp.
// Aquí es producción: Application lo enganxa a la pantalla de comercio (H3-04)
// y al motor de moraleja (DEC-05/REV-02/REV-03).
//
// VER: planificacion/JUGABILIDAD_FASE_H.md — H1-02.
#pragma once

#include <string>
#include <vector>

namespace Game {

// Un ítem en la vitrina de una tienda (DEC-05: catálogos de tienda
// aislados por proyecto; los precios vienen del EquipmentCatalog real).
struct ShopItem {
    std::string id;
    std::string name;
    int price;
    int stock;
    float moralImpact;  // 0.0..0.1 por compra
    bool vendible;
    int requiredLevel;  // requisito de nivel del comprador
};

// Resultado atómico de una transacción (compra/venta).
// ok=false implica que NO se modificó nada: ni oro, ni stock, ni moral.
struct ShopTransaction {
    bool ok;
    std::string message;    // motivo textual, no vacío cuando !ok
    int goldAfter;          // oro del jugador tras la operación
    int moralDelta;         // ±1
    int itemCountDelta;     // +1 comprado, -1 vendido
};

// Motor de tienda GL-free: compra/venta con stock, precios y moral.
//
// Reglas que defiende (DEC-05 REV-03, G-14 closure criteria):
//  - Todo o nada: si falla, no gasta ni cambia nada.
//  - Precio correcto: compra resta price*qty; venta devuelve price/2.
//  - No doble-acción: cada buy() descuenta exactamente una vez.
//  - Moral: compra de armas caras (>=0.05 impact) sube moral en 0.05 por
//    unidad, clamped a 1.0.
//  - Vendible: items con price<=0 o vendible==false no se venden.
class ShopEngine {
public:
    explicit ShopEngine(const std::vector<ShopItem>& items, int startGold,
                        float moralBase = 0.5f);

    int gold() const;
    float moral() const;
    const std::vector<ShopItem>& items() const;

    ShopItem* find(const std::string& id);
    const ShopItem* find(const std::string& id) const;

    // Compra `qty` del item `id`. buyerLevel impide comprar si el
    // requisito de nivel no se cumple. Atómico: si falla, nada cambia.
    ShopTransaction buy(const std::string& id, int qty = 1, int buyerLevel = 1);

    // Vende `qty` del item `id`. 50% del price como refund. Atómico.
    ShopTransaction sell(const std::string& id, int qty = 1);

private:
    std::vector<ShopItem> m_items;
    int m_gold;
    float m_moral;
};

}  // namespace Game
