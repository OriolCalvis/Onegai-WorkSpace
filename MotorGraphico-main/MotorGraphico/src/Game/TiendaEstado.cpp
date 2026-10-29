// TIENDA-02 — ver include/Game/TiendaEstado.h.
#include "Game/TiendaEstado.h"

#include <algorithm>
#include <map>

#include "Game/GameSession.h"
#include "Level/ObjectCatalog.h"

namespace Game {

namespace {

int curaDe(const GameSession& s, const std::string& objectId) {
    const ObjectCatalog* catalogo = s.objectCatalog();
    const ObjectDefinition* def = catalogo != nullptr ? catalogo->find(objectId) : nullptr;
    if (def == nullptr || def->pickup.effect != PickupEffect::Heal) return 0;
    return std::max(0, def->pickup.power);
}

int cuantosLleva(const GameSession& s, const std::string& objectId) {
    const std::vector<std::string>* mochila = s.inventory();
    return mochila == nullptr ? 0 : static_cast<int>(std::count(mochila->begin(), mochila->end(), objectId));
}

FilaTienda fila(const GameSession& s, const std::string& objectId) {
    FilaTienda f;
    f.objectId = objectId;
    f.categoria = clasificaObjeto(s, objectId);
    f.descripcion = describeObjeto(s, objectId, f.categoria);
    f.tienes = cuantosLleva(s, objectId);
    f.curaPv = curaDe(s, objectId);
    return f;
}

}  // namespace

void TiendaEstado::sigue(bool abierta) {
    if (abierta && !m_abierta) {
        m_pestana = PestanaTienda::Comprar;
        m_foco = {};
        m_estado.clear();
        m_error = false;
    }
    m_abierta = abierta;
}

void TiendaEstado::cambiaPestana(int delta) {
    if (delta < 0) ponPestana(PestanaTienda::Comprar);
    else if (delta > 0) ponPestana(PestanaTienda::Vender);
}

void TiendaEstado::ponPestana(PestanaTienda pestana) {
    if (pestana == m_pestana) return;
    m_pestana = pestana;
    // El resultado es de la pestaña donde se hizo: al cambiar, se va.
    m_estado.clear();
    m_error = false;
}

void TiendaEstado::mueve(int delta, std::size_t filas) {
    if (filas == 0) return;
    const long actual = static_cast<long>(foco(filas));
    const long nuevo = std::clamp(actual + delta, 0L, static_cast<long>(filas) - 1);
    m_foco[static_cast<std::size_t>(m_pestana)] = static_cast<std::size_t>(nuevo);
}

void TiendaEstado::ponFoco(std::size_t fila, std::size_t filas) {
    if (filas == 0) return;
    m_foco[static_cast<std::size_t>(m_pestana)] = std::min(fila, filas - 1);
}

std::size_t TiendaEstado::foco(std::size_t filas) const {
    const std::size_t f = m_foco[static_cast<std::size_t>(m_pestana)];
    return filas == 0 ? 0 : std::min(f, filas - 1);
}

std::vector<FilaTienda> TiendaEstado::filas(const GameSession& s, PestanaTienda pestana) {
    std::vector<FilaTienda> out;
    if (pestana == PestanaTienda::Comprar) {
        const std::vector<ShopOffer>& ofertas = s.shopOffers();
        for (std::size_t i = 0; i < ofertas.size(); ++i) {
            FilaTienda f = fila(s, ofertas[i].objectId);
            f.nombre = ofertas[i].name;
            f.precio = ofertas[i].price;
            f.indice = i;
            out.push_back(std::move(f));
        }
        return out;
    }
    const std::vector<std::string>* mochila = s.inventory();
    if (mochila == nullptr) return out;
    std::map<std::string, bool> vistos;
    for (std::size_t i = 0; i < mochila->size(); ++i) {
        const std::string& id = (*mochila)[i];
        if (vistos[id]) continue;
        vistos[id] = true;
        const int paga = s.sellPriceOf(id);
        if (paga <= 0) continue;  // sin precio no se vende: una llave de mision
        FilaTienda f = fila(s, id);
        f.nombre = nombreObjeto(s, id);
        f.precio = paga;
        f.indice = i;
        out.push_back(std::move(f));
    }
    return out;
}

void TiendaEstado::confirma(GameSession& s) {
    const std::vector<FilaTienda> lista = filas(s, m_pestana);
    if (lista.empty()) return;
    const FilaTienda& f = lista[foco(lista.size())];
    if (m_pestana == PestanaTienda::Comprar) {
        const int oro = s.gold();
        if (oro < f.precio) {
            m_estado = "Te faltan " + std::to_string(f.precio - oro) + " de oro.";
            m_error = true;
            return;
        }
        if (!s.buy(f.indice)) {
            m_estado = "No se ha podido comprar.";
            m_error = true;
            return;
        }
        m_estado = "Comprado: " + f.nombre + " · –" + std::to_string(f.precio) + " de oro · ahora tienes " +
                   std::to_string(cuantosLleva(s, f.objectId));
        m_error = false;
        return;
    }
    if (!s.sell(f.indice)) {
        m_estado = "No se ha podido vender.";
        m_error = true;
        return;
    }
    m_estado = "Vendido: " + f.nombre + " · +" + std::to_string(f.precio) + " de oro · ahora tienes " +
               std::to_string(cuantosLleva(s, f.objectId));
    m_error = false;
}

}  // namespace Game
