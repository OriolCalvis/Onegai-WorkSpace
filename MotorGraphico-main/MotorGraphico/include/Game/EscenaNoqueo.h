#pragma once
// AV13-33 (Oriol, 14/10, D0015 §7) — Morir: «escena de noqueo o rescate, cicatriz y moral; después la
// pantalla de Game Over y vuelves al último punto de guardado con una cicatriz más».
//
// La escena, antes de «Has caído»: el héroe cae (el dibujo se hunde y se apaga), la pantalla se
// oscurece y salen dos o tres frases, una tras otra. Si alguien que iba contigo sigue en pie (Venides,
// Xila…), es un RESCATE: te saca de la pelea. Si no, un NOQUEO. ENTER pasa a la frase siguiente.
// GL-free (el reloj y los textos); la dibuja ApplicationCinematica. Pruebas: EscenaNoqueoTests.cpp.

#include <string>
#include <vector>

namespace Game {

class EscenaNoqueo {
public:
    static constexpr float kCaida = 0.9f;   // lo que tarda en caer
    static constexpr float kLinea = 2.6f;   // lo que dura cada frase (ENTER la acorta)
    static constexpr float kFundido = 0.7f; // del velo oscuro al negro, al final

    // `rescatador`: el nombre del primero de los tuyos que sigue en pie; vacío, nadie.
    static EscenaNoqueo para(const std::string& rescatador);

    bool rescate() const { return m_rescate; }
    const std::vector<std::string>& lineas() const { return m_lineas; }

    void avanza(float dt) { m_t += dt; }
    // ENTER: a la frase siguiente (durante la caída, a la primera).
    void salta();
    bool acabada() const { return m_t >= duracion(); }
    float duracion() const;

    // 0..1: cuánto ha caído el héroe.
    float caida() const;
    // 0..1: lo oscura que está la pantalla.
    float velo() const;
    // La frase que se lee ahora (vacía durante la caída y el final) y su opacidad 0..1.
    std::string linea() const;
    float alfaLinea() const;

private:
    int indiceLinea() const;  // -1: antes de la primera; size(): ya acabadas

    bool m_rescate = false;
    std::vector<std::string> m_lineas;
    float m_t = 0.0f;
};

}  // namespace Game
