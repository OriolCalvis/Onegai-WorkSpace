// AV16-04 — La regla de capas del dibujo (Oriol, 15/10: «las alfombras se pintan
// encima del jugador; Sho-noco queda detras de su barca»).
//
// Antes todo lo que se dibujaba en una celda se ordenaba igual, por la diagonal
// x+y: una alfombra, un felpudo o el rombo de una salida (que son del SUELO) se
// pintaban encima de quien estuviera de pie en la celda de al lado, y dos cosas
// en la misma celda (la barca y su barquero) quedaban en un orden cualquiera.
//
// La regla, de abajo arriba:
//   1. Suelo    las baldosas (no son entidades: van antes que todo).
//   2. Plano    alfombras, felpudos, manchas, sombras, rombos de salida, carteles
//               de suelo: SIEMPRE debajo de cualquiera que este de pie, aunque
//               ese este en una celda «detras». Entre si, por profundidad.
//   3. DePie    personas, muebles, arboles: por profundidad isometrica.
//   4. Encima   tejados, copas, carteles altos: siempre sobre los de pie.
//
// Quien va DENTRO de otra cosa (el barquero en la barca) no es una capa nueva:
// es «de pie» con medio paso de profundidad mas que lo que lo lleva, para que
// se dibuje encima aunque esten en la misma celda (Entity::setSortBias).
//
// GL-free y solo cabecera a proposito: la usan Entity (que varios ejecutables
// compilan suelta), la carga de niveles y las pruebas.
#pragma once

#include <string>

enum class CapaDeDibujo { Plano, DePie, Encima };

namespace CapasDeDibujo {

// Cuanto vale un paso de profundidad en la clave de orden (el de Entity.cpp:
// la profundidad va en los bits altos y x desempata).
constexpr int kPasoDeProfundidad = 1 << 16;
// Los planos quedan por debajo de toda clave de pie y los de encima por encima.
// Las claves de pie no pasan de unos 5e8 (mapas de hasta 256x256).
constexpr int kDesplazamientoPlano = -(1 << 30);
constexpr int kDesplazamientoEncima = (1 << 30);

// La clave final de una entidad: la de pie (profundidad y desempate), con el sesgo
// propio, movida a su capa.
constexpr int clave(int claveDePie, CapaDeDibujo capa, int sesgo = 0) {
    return claveDePie + sesgo +
           (capa == CapaDeDibujo::Plano ? kDesplazamientoPlano
                                        : capa == CapaDeDibujo::Encima ? kDesplazamientoEncima : 0);
}

// «plano», «de_pie» o «encima» (como se escribe en un objeto: "capa": "plano").
// Vacio o desconocido = de pie. `conocida` dice si el texto era valido.
inline CapaDeDibujo desdeTexto(const std::string& texto, bool* conocida = nullptr) {
    if (conocida != nullptr) *conocida = true;
    if (texto == "plano") return CapaDeDibujo::Plano;
    if (texto == "encima") return CapaDeDibujo::Encima;
    if (texto.empty() || texto == "de_pie") return CapaDeDibujo::DePie;
    if (conocida != nullptr) *conocida = false;
    return CapaDeDibujo::DePie;
}
inline const char* nombre(CapaDeDibujo capa) {
    switch (capa) {
        case CapaDeDibujo::Plano: return "plano";
        case CapaDeDibujo::Encima: return "encima";
        case CapaDeDibujo::DePie: break;
    }
    return "de_pie";
}

// La capa de un objeto por su id cuando el objeto no la dice (las alfombras y
// salidas del kit no llevan el campo): los felpudos, alfombras y tapetes, las
// manchas y charcos, los rombos de salida, los carteles de suelo y los estrados
// (C2-01: el orador se dibuja encima del estrado, no detras) son planos;
// los tejados y las copas, encima; todo lo demas, de pie. Ignora el sufijo de
// orientacion («_o1»..«_o3»).
inline CapaDeDibujo capaDeObjeto(const std::string& objectId) {
    const auto empieza = [&](const char* prefijo) { return objectId.rfind(prefijo, 0) == 0; };
    for (const char* prefijo : {"felpudo", "alfombra", "tapete", "catifa", "mancha_", "charco", "cartel_suelo",
                                "salida_", "marca_suelo", "sombra_", "estrado"})
        if (empieza(prefijo)) return CapaDeDibujo::Plano;
    for (const char* prefijo : {"tejado_", "copa_"})
        if (empieza(prefijo)) return CapaDeDibujo::Encima;
    return CapaDeDibujo::DePie;
}

}  // namespace CapasDeDibujo
