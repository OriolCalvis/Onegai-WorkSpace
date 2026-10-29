// PERGAMINO-01 — El dialogo en una hoja de pergamino suelta.
//
// Maqueta P03b: hoja de 920x162 abajo y centrada, el hablante en un rotulo
// de cuero (vacio en los datos = «NARRACIÓN»), el texto en Lora 22 y un pie
// con la pagina (1/2) y la tecla: la ultima pagina dice «Cerrar», no
// «Continuar» (MODELO_VISUAL.md §5). Cada linea del dialogo empieza pagina;
// si no cabe en dos renglones, sigue en la siguiente, partida por frases
// (TUTORIAL-01) y, si una frase sola no cabe, por renglones.
//
// H2-01: si el beat pide una prueba («(Tirada de Percepcion contra el
// vacio.)»), su ultima pagina lleva encima la tira con los dados y el grado.
//
// OPCIONES-01: si el beat ofrece opciones, la ultima pagina lleva encima, a
// la izquierda, la lista (una tecla 1-4 cada una y la elegida subrayada) y el
// pie dice «W S Elegir  ENTER Decidir».
//
// Estado propio: en que pagina va el jugador y la tirada de este dialogo.
// Sin OpenGL.
#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "HUD/Pergamino/Lienzo.h"
#include "RPG/Narrative/NarrativeState.h"
#include "RPG/TiradaVisible.h"

namespace Pergamino {

class FuenteAtlas;
class Pincel;

// OPCIONES-01 — La zona de cada opcion (indice = la opcion), para el raton.
constexpr int kZonaOpcionDialogo = 1500;

class DialogoHoja {
public:
    static constexpr float kAncho = 920.0f;
    static constexpr float kAlto = 162.0f;
    static constexpr float kAnchoTexto = 840.0f;
    static constexpr int kRenglones = 2;

    // Lo que dice la sesion en este fotograma. Un dialogo distinto (otro
    // hablante u otras lineas) vuelve a la primera pagina.
    void mira(const std::string& hablante, const std::vector<std::string>& lineas,
              const FuenteAtlas& letras);
    // ENTER: pasa pagina. Devuelve false si ya estaba en la ultima (entonces
    // el que llama cierra el dialogo).
    bool avanza();
    // Fuera del dialogo se olvida: si el mismo NPC vuelve a decir lo mismo,
    // empieza otra vez por la primera pagina.
    void cierra();
    // H2-01 — La ultima tirada de la partida. Si es nueva (otro numero que la
    // ultima mirada), es la de este dialogo: el beat que la pide la tira al
    // abrirse. Va despues de mira(), porque un dialogo nuevo empieza sin
    // tirada; cerrar el dialogo tambien la quita.
    void miraTirada(const RPG::TiradaVisible& t);
    // OPCIONES-01 — Lo que se puede contestar (las de la eleccion pendiente).
    // Otras opciones vuelven a dejar elegida la primera.
    void miraOpciones(const std::vector<std::string>& opciones);
    // Las dos cosas que el dialogo mira de la aventura, cada fotograma.
    void miraNarrativa(const RPG::NarrativeState& n) {
        miraTirada(n.ultimaTirada());
        miraOpciones(n.eleccion().textos);
    }
    bool conOpciones() const { return !m_opciones.empty(); }
    const std::vector<std::string>& opciones() const { return m_opciones; }
    int opcion() const { return m_opcion; }
    // W/S: la de arriba o la de abajo, dando la vuelta. El raton: la de encima.
    void mueveOpcion(int paso);
    void senalaOpcion(int i) {
        if (i >= 0 && i < static_cast<int>(m_opciones.size())) m_opcion = i;
    }
    bool conTirada() const { return m_tirada.numero > 0; }
    const RPG::TiradaVisible& tirada() const { return m_tirada; }
    int pagina() const { return m_pagina; }
    int paginas() const { return static_cast<int>(m_paginas.size()); }
    // DIALOGO-02 — Quien habla en la pagina que se ve («» = la narracion).
    const std::string& quienHabla() const;
    // «Quien, acotacion: «texto»» -> quien = «Quien», texto = «(acotacion) texto»
    // (sin las comillas de fuera). Sin eso, la linea es del hablante del beat
    // (vacio: de la narracion). Un prefijo con un punto es narracion: no se parte.
    static std::pair<std::string, std::string> vozDe(const std::string& linea, const std::string& hablante);
    bool ultima() const { return m_pagina + 1 >= paginas(); }
    // Los renglones de la pagina en la que se va (ninguno sin dialogo).
    std::vector<std::string> renglones() const {
        return m_paginas.empty() ? std::vector<std::string>{} : m_paginas[static_cast<std::size_t>(m_pagina)];
    }

    // TIENDA-02: la ultima pagina dice lo que pasa al cerrarla («Ver la
    // tienda» si es un tendero, «Pelear» si el beat monto un combate).
    void pinta(Lienzo& l, const Pincel& p, int anchoVista, int altoVista,
               const std::string& verboFinal = "Cerrar") const;

private:
    // OPCIONES-01 — La lista de las opciones, con su borde de abajo en `abajo`.
    void pintaOpciones(Lienzo& l, const Pincel& p, float x, float abajo) const;

    std::string m_hablante;
    std::vector<std::string> m_lineas;
    std::vector<std::vector<std::string>> m_paginas;  // renglones de cada pagina
    std::vector<std::string> m_quienes;  // DIALOGO-02: quien habla en cada pagina (vacio = narracion)
    int m_pagina = 0;
    RPG::TiradaVisible m_tirada;  // la de este dialogo (numero 0 = ninguna)
    int m_tiradaVista = 0;        // numero de la ultima tirada mirada
    // Oriol (22/10): los dados ruedan un momento al salir la tira (en la ultima pagina).
    mutable bool m_tiradaRodando = false;
    mutable std::chrono::steady_clock::time_point m_tiradaDesde{};
    std::vector<std::string> m_opciones;  // OPCIONES-01: las de la eleccion pendiente
    int m_opcion = 0;                     // la elegida (subrayada)
};

}  // namespace Pergamino
