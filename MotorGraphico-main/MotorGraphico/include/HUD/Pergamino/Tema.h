// PERGAMINO-01 — Los tokens del tema pergamino, sin OpenGL.
//
// Son los de Claude outputs/estudio_uiux_2026-09-28/maquetas/fuente/sistema/
// tema_pergamino.css, pasados a constantes: tinta sobre papel, cuero para
// las tapas y un solo color de foco, el carmin de la cinta de marcapaginas
// (4,9:1 sobre el papel base). Quien dibuja algo del tema pide el color
// aqui; ningun componente escribe un RGB suelto (MODELO_VISUAL.md §10.1).
//
// Cabecera sola y sin estado: son valores, no una clase.
#pragma once

#include "Core/Math/Vector4.h"

namespace Pergamino {

constexpr Vector4 rgb(int r, int g, int b, float alfa = 1.0f) {
    return Vector4{static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f,
                   static_cast<float>(b) / 255.0f, alfa};
}

namespace Color {
// Tintas (contraste medido sobre el papel base #E2CD9C). TINTAS-01 (Oriol,
// 08/10: «el texto se ve demasiado clarito»): las tres, mas oscuras, con los
// mismos tres escalones; eran #2B1D12 10,5:1, #553E2B 6,4:1 y #6B5038 4,8:1.
constexpr Vector4 kTinta = rgb(0x1C, 0x12, 0x09);   // 11,8:1
constexpr Vector4 kTinta2 = rgb(0x3B, 0x2A, 0x1B);  // 8,8:1
constexpr Vector4 kTinta3 = rgb(0x4F, 0x3B, 0x28);  // 6,8:1 (apagado: sigue un escalon por debajo)
constexpr Vector4 kCarmin = rgb(0x9A, 0x2A, 0x1C);  // foco y accion principal
constexpr Vector4 kCarmin2 = rgb(0x6E, 0x1A, 0x10);
// Estados: nunca van solos (el error lleva «!» y su caja).
constexpr Vector4 kExito = rgb(0x2E, 0x63, 0x37);
constexpr Vector4 kAviso = rgb(0x7A, 0x44, 0x09);
constexpr Vector4 kError = rgb(0x8E, 0x1B, 0x12);
constexpr Vector4 kInfo = rgb(0x29, 0x50, 0x7A);
// Recursos. El mana y la moral ya no se dibujan en el HUD (decision de
// Oriol del 29/09): solo queda la vida.
constexpr Vector4 kPv = rgb(0xA8, 0x3A, 0x2A);
// Materiales.
constexpr Vector4 kPapel = rgb(0xE2, 0xCD, 0x9C);
constexpr Vector4 kPapelLuz = rgb(0xF2, 0xE4, 0xC1);
constexpr Vector4 kPapelMancha = rgb(0xC6, 0xA5, 0x6C);
constexpr Vector4 kCuero = rgb(0x4E, 0x32, 0x1D);
constexpr Vector4 kClaroCuero = rgb(0xF2, 0xE4, 0xC1);   // 9,3:1 sobre cuero
constexpr Vector4 kClaroCuero2 = rgb(0xD9, 0xC4, 0x9A);
constexpr Vector4 kLinea = rgb(0x4A, 0x34, 0x21);        // contorno de todo material
constexpr Vector4 kAdorno = rgb(0x6B, 0x50, 0x38);
// Velos y sombras.
constexpr Vector4 kVelo = rgb(24, 14, 6, 0.62f);          // mundo detras de un menu
constexpr Vector4 kSombra = rgb(26, 16, 8, 0.45f);        // sombra de hoja, papiro y retal
constexpr Vector4 kSombraCuero = rgb(10, 6, 2, 0.5f);
constexpr Vector4 kLavadoFoco = rgb(154, 42, 28, 0.10f);  // fondo de la fila con foco
constexpr Vector4 kSubrayadoFoco = rgb(154, 42, 28, 0.35f);
constexpr Vector4 kBlanco = rgb(255, 255, 255);
// El rombo de la baldosa en el mundo (P03a): sobre el suelo el carmin del
// papel se pierde; va un rojo mas vivo con contorno de tinta de cuero.
constexpr Vector4 kRombo = rgb(0xC0, 0x39, 0x2B);
constexpr Vector4 kRomboContorno = rgb(0x2A, 0x1A, 0x0E);
}  // namespace Color

namespace Medida {
constexpr float kAreaSegura = 24.0f;    // margen de la pantalla (MODELO_VISUAL.md §3.3)
constexpr float kSombraY = 4.0f;        // desplazamiento de la sombra de un material
constexpr float kFila = 44.0f;          // alto de fila de lista
constexpr float kTecla = 28.0f;         // alto de la caja de una tecla
constexpr int kEscalaPixel = 2;         // la fuente 5x7 se lee a x2 (x3 en titulos)
constexpr float kDuracionHoja = 0.36f;  // vuelta de hoja (§13.3)
constexpr float kFundidoReducido = 0.16f;
}  // namespace Medida

}  // namespace Pergamino
