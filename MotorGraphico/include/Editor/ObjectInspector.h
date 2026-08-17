#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Level/LevelDefinition.h"

class EditorState;
class ObjectCatalog;

// ---------------------------------------------------------------------
// ObjectInspector — el panel de propiedades de un objeto YA COLOCADO,
// SIN abrir una ventana.
//
// Mismo criterio que EditorState/ProjectHub/CharacterCreationScreen: el
// estado entero (que campo esta marcado, que modo de edicion, el buffer
// de texto) vive aqui y se prueba de verdad en
// examples/demo_object_inspector.cpp; la vista (level_editor.cpp) solo
// pinta lines()/titulo()/aviso() y pasa las teclas por key().
//
// POR QUE EXISTE. Hasta aqui las propiedades por instancia (nombre
// visible, variante, salida, botin, evento...) solo se podian poner a
// mano en el JSON del nivel: el editor colocaba el objeto y habia que
// salir a editar texto. Un objeto sin sus datos de autor no es un
// objeto, es un marcador: el cofre sin botin, la puerta sin destino, el
// PNJ sin dialogo. Este panel es el sitio donde la instancia se
// personaliza sin salir del mapa (roadmap del editor: "el catalogo
// define el tipo; el mapa define la instancia").
//
// NO es dueno de nada: recibe EditorState y ObjectCatalog por referencia
// para leer y editar, igual que un HUD no posee el BattleState que
// muestra. Si el objeto inspeccionado desaparece (deshacer, borrado),
// el inspector se cierra solo en la siguiente tecla.
// ---------------------------------------------------------------------
namespace Editor {

class ObjectInspector {
public:
    // "destinos": rutas de nivel que se ofrecen como destino de salida
    // (los niveles del proyecto abierto, misma lista que la herramienta
    // de enlazar). Puede estar vacia: la salida se queda sin opciones.
    ObjectInspector(EditorState& editor, const ObjectCatalog& catalog,
                    std::vector<std::string> destinos);

    // Abre el panel sobre el objeto de la celda. Sin objeto en la celda
    // no se abre (un inspector sin inspeccionado es un panel vacio).
    void open(int x, int y);
    void close();
    bool isOpen() const { return m_open; }
    GridCoord cell() const { return m_cell; }

    // El spawn inspeccionado (nullptr si desaparecio del nivel).
    const ObjectSpawn* spawn() const;

    // La puerta unica del bucle de teclado (igual que ProjectHub::key):
    //   W/S     campo anterior / siguiente
    //   ENTER   editar el campo marcado (o confirmar la edicion)
    //   ESC     cancelar edicion / cerrar el panel
    //   Q/E     ciclar valores (escala, destino) y filtrar en el picker
    //   X       quitar la entrada de botin marcada
    //   letras  escriben en el buffer cuando hay edicion abierta
    //   BACKSPACE borra del buffer
    // Devuelve true si la tecla se consumio (mientras esta abierto, el
    // editor no debe ver NINGUNA tecla: el panel es modal).
    bool key(char k);

    // --- Lo que pinta la vista ---
    // Titulo: nombre del objeto, id y celda. Lineas: una por campo, con
    // el valor actual; la marcada lleva el cursor (durante los pickers,
    // las lineas son las del picker). Aviso: feedback de la ultima
    // accion (vacio = nada). Buffer: lo que se esta tecleando en la
    // edicion de texto en curso (vacio fuera de ella); la vista lo
    // pinta como campo activo, con el texto original detras.
    std::string titulo() const;
    const std::vector<std::string>& lines() const { return m_lineas; }
    std::size_t selected() const { return m_sel; }
    bool editando() const { return m_modo != Modo::Navegar; }
    bool editandoTexto() const { return m_modo == Modo::Texto; }
    const std::string& buffer() const { return m_buffer; }
    const std::string& aviso() const { return m_aviso; }

private:
    // Que es cada fila. El orden es fijo (ver rebuild()): los campos de
    // texto, la escala, la salida, el botin (una fila por id + la fila
    // de anadir), la tabla de botin, el evento y las propiedades.
    enum class Campo {
        NombreVisible,
        Variante,
        Escala,
        Efecto,
        Salida,
        PosicionDestino,
        LootItem,      // m_filaLootIndex dice cual
        AnadirLoot,
        LootTabla,
        Evento,
        PropiedadItem, // m_filaLootIndex dice cual (indice en properties)
        AnadirPropiedad
    };
    enum class Modo { Navegar, Texto, ElegirSalida, ElegirLoot };

    struct Fila {
        Campo campo;
        int indice = -1;   // para LootItem/PropiedadItem
    };

    void rebuild();
    void abrirTexto(const std::string& inicial);
    void confirmar();
    void commitTexto();
    std::string nombreLoot(const std::string& id) const;

    EditorState& m_editor;
    const ObjectCatalog& m_catalog;
    std::vector<std::string> m_destinos;

    bool m_open = false;
    GridCoord m_cell{0, 0};
    std::size_t m_sel = 0;
    Modo m_modo = Modo::Navegar;

    // Edicion de texto en curso: a que campo se aplica y el buffer.
    Campo m_campoTexto = Campo::NombreVisible;
    int m_indiceTexto = -1;
    std::string m_buffer;

    // Pickers (ElegirSalida / ElegirLoot): la lista visible, el filtro
    // y la marca. El picker de loot filtra el catalogo entero, como la
    // paleta del editor: con miles de ids, pasar de uno en uno no es.
    std::vector<std::string> m_opciones;   // ids/destinos ya filtrados
    std::vector<std::string> m_filtro;     // por posicion en la lista base
    std::string m_textoFiltro;
    std::size_t m_marca = 0;
    std::vector<std::string> m_baseLoot;   // ids del catalogo, sin filtro

    std::vector<Fila> m_filas;
    std::vector<std::string> m_lineas;
    std::string m_aviso;
};

}  // namespace Editor
