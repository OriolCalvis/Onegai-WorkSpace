// FICHA-01 — Manifiesto del atlas de iconos (assets/textures/iconos.json),
// generado por tools/gen_iconos.py. Sin GL: traduce ids de concepto (un stat,
// una ranura, una condicion, un estado de mision) al frame del atlas. Quien
// pinta es un widget del HUD con la textura; quien decide que icono toca
// es esto, para que la prueba pueda comprobar el contrato sin ventana.
#pragma once

#include <map>
#include <string>

#include "Core/Errors/Result.h"

namespace Hud {

class Iconos {
public:
    Result<bool> cargar(const std::string& rutaJson);
    bool cargado() const { return !m_frames.empty(); }

    // Geometria del atlas, para definir las regiones en un TextureAtlas.
    int columnas() const { return m_columnas; }
    int filas() const { return m_filas; }
    int lado() const { return m_lado; }
    const std::string& imagen() const { return m_imagen; }
    std::size_t total() const { return m_frames.size(); }

    // Frame 1-based del icono, 0 si no existe.
    int frame(const std::string& id) const;
    int frameStat(const std::string& clave) const;      // "CON" -> frame de "con"
    int frameRanura(const std::string& ranura) const;   // "Head" -> "ranura_cabeza"
    // Condicion con icono propio, o el generico segun sea negativa.
    int frameCondicion(const std::string& id, bool negativa) const;
    int frameMision(const std::string& estado) const;   // "activa"/"hecha"/"fallida"/"objetivo"/"objetivoHecho"
    int frameCodice(const std::string& tipo) const;     // "personaje"/"lugar"/"deidad"/"lore"/"curiosidad"/"mapa"

private:
    int viaTabla(const std::map<std::string, std::string>& tabla, const std::string& clave) const;

    int m_columnas = 0, m_filas = 0, m_lado = 16;
    std::string m_imagen;
    std::map<std::string, int> m_frames;
    std::map<std::string, std::string> m_stats, m_ranuras, m_condiciones, m_misiones, m_codice;
    std::string m_perjuicio = "perjuicio", m_mejora = "mejora";
};

}  // namespace Hud
