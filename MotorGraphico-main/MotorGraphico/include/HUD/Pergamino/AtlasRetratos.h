// UI-BG3-01 — Los retratos de las pantallas en cuadricula, en la CPU.
//
// Las rejillas de la creacion (razas y clases), la vista previa del
// personaje y la barra de combate ensenan personajes montados por capas
// (Game::MontajePersonajes). Cada uno es un frame de 64x64; aqui se guardan
// en un atlas de 1024x1024 (256 celdas) con una clave por retrato. Quien
// pinta pide la celda de una clave y, si no esta, da la imagen. El atlas
// cambia de version cuando entra un retrato nuevo y Render/PergaminoGL lo
// sube entonces, como el terreno del minimapa.
//
// Cuando se llena se vacia entero: lo que se este viendo se vuelve a pedir
// en el fotograma siguiente. Sin OpenGL.
#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>

#include "HUD/Pergamino/Lienzo.h"
#include "HUD/Pergamino/Materiales.h"

namespace Pergamino {

class AtlasRetratos {
public:
    static constexpr int kLado = 64;
    static constexpr int kColumnas = 16;
    static constexpr int kCeldas = kColumnas * kColumnas;

    // Rellena `frame` (kLado x kLado, ya creado y transparente). false = no
    // hay retrato (la celda no se gasta).
    using Fabrica = std::function<bool(Imagen& frame)>;

    // La region UV (0..1) de la celda de `clave`; si no esta, la fabrica.
    // Rect vacio (w == 0) si la fabrica no da imagen.
    Rect celda(const std::string& clave, const Fabrica& fabrica);
    bool tiene(const std::string& clave) const { return m_celdas.count(clave) != 0; }
    std::size_t cuantas() const { return m_celdas.size(); }

    const Imagen& imagen() const { return m_imagen; }
    int version() const { return m_version; }
    void vacia();
    void guardaAviso(const std::string& clave, std::string aviso) { m_avisos[clave] = std::move(aviso); }
    std::string aviso(const std::string& clave) const {
        const auto it = m_avisos.find(clave);
        return it == m_avisos.end() ? std::string{} : it->second;
    }

    // La ultima tira de frames montada (la hoja de 12 de un personaje): la
    // vista previa pide sus frames de uno en uno mientras anda, y asi se
    // monta una sola vez. nullptr si la guardada es de otra clave.
    const Imagen* tira(const std::string& clave) const {
        return clave == m_claveTira && !m_tira.vacia() ? &m_tira : nullptr;
    }
    void guardaTira(const std::string& clave, Imagen tira) {
        m_claveTira = clave;
        m_tira = std::move(tira);
    }

private:
    Rect uvDe(int celda) const;

    Imagen m_imagen;
    std::unordered_map<std::string, int> m_celdas;
    int m_version = 0;
    std::string m_claveTira;
    Imagen m_tira;
    std::unordered_map<std::string, std::string> m_avisos;  // acompana las celdas de la cache

};

// Copia el frame `frame` (0-based) de una tira horizontal de frames de
// kLado x kLado (la hoja de 12 de MontajePersonajes) en `destino`.
void copiaFrame(const std::uint8_t* tira, int anchoTira, int frame, Imagen& destino);

}  // namespace Pergamino
