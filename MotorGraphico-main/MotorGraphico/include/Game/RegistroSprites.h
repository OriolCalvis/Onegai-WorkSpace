// SPRITES-01 — Que hoja dibuja a cada cosa, desde datos.
//
// Hasta ahora ApplicationNivel.cpp llevaba dos tablas escritas a mano (que
// frame de race_npc_idle.png tiene cada raza, que frame del atlas de
// Boundington tiene cada personaje de historia), los props y los pickups se
// pintaban como un tile del suelo tenido, y las hojas que ya andaban
// (razas_andando.png, personajes_historia.png, props_p0.png, pickups_p0.png)
// no las usaba nadie. Con PixelLab entrando (PERGAMINO/PIXELLAB, 01/10) hace
// falta que una hoja nueva sea una linea de datos, no una tabla en C++.
//
// assets/textures/sprites.json es el indice: lista los manifiestos de las
// hojas (los mismos que generan tools/gen_personajes.py y gen_props_p0.py) y
// las equivalencias entre ids del catalogo de objetos e ids de las hojas.
// Hay dos clases de hoja:
//   - de ACTORES: filas de 12 frames con el reparto de personaje.png
//     (arriba 1-3, derecha 4-6, abajo 7-9, izquierda 10-12); cada actor es
//     una fila. La busqueda va por id de objeto y, para los PNJ, por raza
//     («race:<raceId>»).
//   - de FRAMES: un dibujo por id (props y pickups).
// Un actor puede tener VARIANTES (PNJ-01: «variantes» en su fila del
// manifiesto): el mismo PNJ con otra cara, para que los que se colocan varias
// veces no sean clones (SpriteActor::filaPara).
// Un objeto del catalogo sin hoja propia puede caer en un alias (id -> id) o
// en un patron por prefijo o sufijo del id («puerta_» -> puerta_taberna,
// «_tendero» -> mercader); el primero que encaja manda.
//
// Sin OpenGL. El que sube las texturas y recorta las regiones es
// Render/BancoAtlas (motor_core).
#pragma once

#include <map>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"

namespace Game {

struct HojaSprites {
    std::string id;
    std::string imagen;  // ruta desde la carpeta de assets («textures/x.png»)
    int celdaAncho = 64, celdaAlto = 64;
    int columnas = 1, filas = 1;
    bool actores = false;  // filas de 12 frames (si no, un frame por id)
    // INT-03: false («sombraBlob»: false en el manifiesto) = el dibujo ya lleva
    // su sombra o no debe tenerla; el motor no le pone la mancha blob.
    bool sombraBlob = true;
};

struct SpriteActor {
    std::string hoja;
    int fila = 0;  // 1-based
    // PNJ-01: otras filas del mismo PNJ (otra cara, otra ropa del mismo
    // oficio). Un PNJ colocado varias veces en un nivel lleva una distinta en
    // cada sitio: la colocacion n (0, 1, 2...) usa filaPara(n).
    std::vector<int> variantes;
    int filaPara(int n) const {
        const int total = 1 + static_cast<int>(variantes.size());
        const int k = ((n % total) + total) % total;
        return k == 0 ? fila : variantes[static_cast<std::size_t>(k - 1)];
    }
};

struct SpriteFijo {
    std::string hoja;
    int frame = 0;  // 1-based, por filas
};

class RegistroSprites {
public:
    // Lee <carpetaAssets>/textures/sprites.json y los manifiestos que nombra.
    // Un manifiesto que falta no es fatal: se avisa en el resultado y las
    // demas hojas siguen (el juego cae en los sprites de siempre).
    Result<bool> carga(const std::string& carpetaAssets);
    bool cargado() const { return m_cargado; }
    const std::vector<std::string>& avisos() const { return m_avisos; }

    const HojaSprites* hoja(const std::string& id) const;
    // El actor con hoja propia: por su id de objeto (con sus alias) y, si no
    // la tiene y es un PNJ con raza, por la raza. nullptr = el sprite base.
    const SpriteActor* actor(const std::string& objectId, const std::string& raceId = "") const;
    // El dibujo de un prop o pickup: por id (con alias) y luego por prefijo.
    const SpriteFijo* fijo(const std::string& objectId) const;

    std::size_t hojas() const { return m_hojas.size(); }
    std::size_t actores() const { return m_actores.size(); }
    std::size_t fijos() const { return m_fijos.size(); }

private:
    Result<bool> cargaHoja(const std::string& carpetaAssets, const std::string& id,
                           const std::string& manifiesto, const std::string& prefijo);
    std::string resuelveAlias(const std::string& id) const;
    // Los ids por los que buscar: el alias (o el propio id) y luego los
    // patrones que encajen, en su orden.
    std::vector<std::string> candidatos(const std::string& objectId) const;

    struct Patron {
        std::string texto, id;
        bool sufijo = false;
    };

    std::map<std::string, HojaSprites> m_hojas;
    std::map<std::string, SpriteActor> m_actores;  // id de objeto o «race:<id>»
    std::map<std::string, SpriteFijo> m_fijos;
    std::map<std::string, std::string> m_alias;    // id del catalogo -> id de hoja
    std::vector<Patron> m_patrones;
    std::vector<std::string> m_avisos;
    bool m_cargado = false;
};

}  // namespace Game
