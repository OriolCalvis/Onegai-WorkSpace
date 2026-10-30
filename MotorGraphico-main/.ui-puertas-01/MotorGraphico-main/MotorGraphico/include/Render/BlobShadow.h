#pragma once

#include <vector>

class Texture;

// Sombras "blob" bajo entidades (motor_grafico_gantt_rpg.puml, Fase 11):
// la sombra clasica de los juegos isometricos 2D -- una elipse oscura
// difusa a los pies del sprite, sin geometria de oclusion real (una
// sombra proyectada de verdad necesitaria conocer la forma del sprite y
// la direccion de la luz; el blob da el 90% de la sensacion de "pegado
// al suelo" por una fraccion del coste, y es lo que usan los isometricos
// clasicos de esta estetica).
//
// La textura es PROCEDURAL (mismo criterio que BitmapFont::generateAtlas
// y makeProceduralLightmap: sin red en este entorno de trabajo para
// vendorizar un PNG, y un degradado radial no lo necesita): un circulo
// BLANCO con alpha que cae de opaco (centro) a transparente (borde). El
// color lo pone el TINT al dibujarla (negro semitransparente, ver
// Entity::renderShadow): asi la misma textura sirve para sombras de
// cualquier oscurecimiento sin regenerarla, igual que la textura blanca
// 1x1 de los widgets de HUD sirve para cualquier color.

// Rellena "pixels" (RGBA8, size*size) con el degradado radial: RGB
// blanco, alpha = attenuacion suavizada (t*t: mas denso en el centro,
// cola suave en el borde -- una caida lineal se ve como un disco duro).
// Pura, sin GL: testeable de verdad (ver demo_dynamic_lights.cpp).
// pixels se redimensiona a size*size*4; size <= 0 lo deja vacio.
void FillBlobShadowPixels(std::vector<unsigned char>& pixels, int size);

// Crea la textura GL (GL_LINEAR: la sombra se escala al tamano de cada
// entidad, difusa a proposito). El llamador es propietario (delete).
// Necesita contexto GL real.
Texture* CreateBlobShadowTexture(int size);
