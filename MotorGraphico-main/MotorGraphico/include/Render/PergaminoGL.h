// PERGAMINO-01 — El unico que dibuja el tema pergamino.
//
// Pergamino::InterfazPergamino describe cada fotograma en lienzos de trazos
// (sin GL). Esto los convierte en texturas y quads de la SpriteBatch del HUD:
//   - fabrica bajo demanda la textura de cada material pedido (y suelta las
//     que llevan un rato sin usarse);
//   - sube el terreno del minimapa cuando cambia de nivel;
//   - pinta la hoja viva del cuaderno en una textura de render y guarda la
//     foto de la anterior cuando empieza una vuelta (pasar hoja);
//   - registra dos capas en el HudManager: la baja (HUD de exploracion y
//     dialogo) antes de los widgets de siempre y la alta (cuaderno y
//     papiros, con su velo) la ultima;
//   - y dos piezas del mundo: el rombo carmin en la baldosa de quien atiende
//     a la E, en el pase de sombras del IsometricRenderer (bajo los
//     sprites, como en P03a; en el HUD le cruzaria las piernas al NPC), y
//     (PART-02d) las cartas jugadas en el combate, encima de todo.
//
// Colaborador de Application: lo crea init() con la ventana ya viva y lo
// suelta shutdown() antes que la ventana (C-05).
#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "HUD/Pergamino/BancoMateriales.h"
#include "HUD/Pergamino/GolpesEnPantalla.h"
#include "HUD/Pergamino/InterfazPergamino.h"
#include "HUD/Pergamino/SueloCombate.h"
#include "Render/EfectosDeCartas.h"
#include "Render/HudElement.h"
#include "Render/IRenderable.h"

class Shader;
class SpriteBatch;
class Texture;
class TextureManager;

namespace Render {

class PergaminoGL {
public:
    PergaminoGL();
    ~PergaminoGL();
    PergaminoGL(const PergaminoGL&) = delete;
    PergaminoGL& operator=(const PergaminoGL&) = delete;

    // Letras (PNG + JSON), iconos del juego, la textura blanca del HUD y las
    // pieles de los materiales (assets/textures/pergamino/materiales.json,
    // opcional: sin ellas todo es procedimental). Si fallan las letras, el
    // juego sigue con el HUD de siempre.
    Result<bool> carga(TextureManager& texturas, Texture* blanca, Texture* iconos,
                       const std::string& carpetaAssets);
    bool listo() const;

    Pergamino::InterfazPergamino& interfaz() { return m_interfaz; }

    // Antes del pase del HUD: texturas pendientes y la hoja viva del cuaderno
    // pintada en su textura de render.
    void prepara(SpriteBatch& batch, Shader& shader, int anchoVista, int altoVista);

    IHudElement* capaBaja() { return m_capaBaja.get(); }
    IHudElement* capaAlta() { return m_capaAlta.get(); }
    // Va en la cola del IsometricRenderer (y sale de ella antes de que esto
    // muera).
    IRenderable* marcaSuelo() { return m_marca.get(); }
    // PART-02d: las cartas jugadas, a la vista (encima de todo).
    EfectosDeCartas& efectosDeCartas() { return *m_efectos; }
    // COMBATE-01: los numeros de los golpes y el destello rojo (encima de la capa baja).
    Pergamino::GolpesEnPantalla& golpes() { return m_golpes; }
    // AV13-10: el borde del tablero del combate (en el pase de sombras, con el rombo).
    Pergamino::SueloCombate& suelo() { return m_suelo; }
    // Lo que va en la cola del IsometricRenderer: el rombo y las cartas.
    std::vector<IRenderable*> delMundo() { return {m_marca.get(), m_efectos.get()}; }

    // Vuelca un lienzo en la SpriteBatch (lo usan las capas).
    void dibuja(SpriteBatch& batch, const Pergamino::Lienzo& lienzo) const;
    // El rombo del suelo, en coordenadas del mundo (lo usa marcaSuelo()).
    void dibujaMarca(SpriteBatch& batch) const;
    void dibujaSuelo(SpriteBatch& batch) const;  // AV13-10

private:
    struct Recursos;
    std::unique_ptr<Recursos> m_recursos;  // texturas y hojas de render (GL)
    Pergamino::InterfazPergamino m_interfaz;
    Pergamino::BancoMateriales m_banco;  // pieles 9-slice o procedimental
    std::unique_ptr<IHudElement> m_capaBaja;
    std::unique_ptr<IHudElement> m_capaAlta;
    std::unique_ptr<IRenderable> m_marca;
    std::unique_ptr<EfectosDeCartas> m_efectos;  // PART-02d
    Pergamino::GolpesEnPantalla m_golpes;        // COMBATE-01
    Pergamino::SueloCombate m_suelo;             // AV13-10
};

}  // namespace Render
