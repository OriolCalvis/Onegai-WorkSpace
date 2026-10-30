// PERGAMINO-01 — Una hoja del cuaderno del jugador, como datos.
//
// El cuaderno (TAB) tiene seis hojas: Inventario, Equipo, Ficha, Acciones,
// Misiones y Codice. Casi todas usan la plantilla T2 del modelo visual:
// lista a la izquierda (cinta carmin en la fila con foco) y detalle a la
// derecha con su accion y su motivo. La Ficha tiene su propia rejilla y
// Acciones, de momento, un estado vacio honesto.
//
// Estas estructuras no saben de GameSession ni de catalogos: las rellena
// HUD/Pergamino/PaginasJugador.cpp a partir de los modelos del juego, y las
// pinta pintaPagina() en coordenadas de la hoja (0,0 = esquina de arriba a
// la izquierda). Asi la maquetacion se prueba con datos inventados.
#pragma once

#include <array>
#include <string>
#include <vector>

#include "HUD/Pergamino/Lienzo.h"

namespace Pergamino {

class Pincel;

enum class TonoInsignia { Neutra, Exito, Aviso, Info };

struct FilaPagina {
    std::string icono;         // nombre en iconos.json; vacio = sin icono
    int escalaIcono = 2;
    std::string nombre;
    std::string valor;         // a la derecha
    bool valorPixel = true;    // cifras en pixel; si no, Lora cursiva (p. ej. «Vacío»)
    bool apagada = false;
    bool hecha = false;        // marca de cumplido
    int sangria = 0;
    // AV14-22 — En la lista del detalle: cuantas lineas puede ocupar (una
    // consecuencia necesita mas que un objetivo). El icono de la fila, si
    // lo trae, sustituye a la casilla de objetivo.
    int lineas = 2;
};

struct DatoPagina {
    std::string clave, valor;
};

struct DetallePagina {
    std::string icono;
    std::string titulo;
    std::string insignia;
    TonoInsignia tono = TonoInsignia::Neutra;
    std::string efectoIcono;
    std::string efecto;                  // «Recupera 15 PV.»
    std::vector<DatoPagina> datos;       // «Tienes 2»
    std::string bloque;                  // rotulo de bloque antes de la lista (OBJETIVO ACTUAL)
    std::vector<FilaPagina> lista;       // objetivos de una mision, etc.
    std::vector<std::string> parrafos;   // descripcion
    std::string nota;                    // cursiva al pie
    std::string boton;                   // «USAR»; vacio = sin accion
    bool botonActivo = false;
    std::string motivo;                  // por que no se puede (aviso con «!»)
    std::string estado;                  // resultado de la ultima accion
    bool estadoError = false;
};

struct StatPagina {
    std::string icono, nombre, abreviatura, detalle;
    int total = 0;
};

struct FichaPagina {
    std::vector<DatoPagina> identidad;   // Raza, Clase, Trasfondo, Deidad
    std::vector<DatoPagina> estado;      // Moral, Condiciones
    std::array<StatPagina, 4> stats;
    std::string rasgoIcono, rasgoNombre, rasgoDescripcion;
    std::vector<DatoPagina> historia;    // Vinculo, Miedo... (nombres del canon)
    std::vector<DatoPagina> reputacion;  // AV14-23: facción y cómo te ven («Te aprecian (+12)»)
};

struct PaginaCuaderno {
    enum class Tipo { Lista, Ficha, Vacia };
    Tipo tipo = Tipo::Lista;
    std::string titulo;        // en pixel entre filetes
    std::string pestana;       // etiqueta de la pestana
    std::string iconoPestana;
    std::string cabeceraIzq, cabeceraDer;  // «OBJETO» · «CANT.»
    float anchoLista = 360.0f;
    std::vector<FilaPagina> filas;
    int foco = 0;
    DetallePagina detalle;     // el de la fila con foco
    FichaPagina ficha;
    std::string vacioIcono, vacioTitulo, vacioTexto;
    std::string elegir, verbo; // pie: «W S Objeto» · «ENTER Usar»
};

// TIENDA-02 — Lo que responde al raton en una pagina: cada fila de la lista
// (indice = su posicion en `filas`) y el boton del detalle. Las zonas caen
// en coordenadas de la hoja: quien la pinta en pantalla las mueve con ella
// (Lienzo::desplaza), como la tienda.
constexpr int kZonaFilaPagina = 2;
constexpr int kZonaBotonPagina = 3;

// Pinta la hoja entera (contenido, sin el papel) en coordenadas de la hoja
// de ancho x alto. `numero`/`total` es el folio de la esquina («1/6»).
void pintaPagina(Lienzo& l, const Pincel& p, const PaginaCuaderno& pagina, float ancho,
                 float alto, int numero, int total);

}  // namespace Pergamino
