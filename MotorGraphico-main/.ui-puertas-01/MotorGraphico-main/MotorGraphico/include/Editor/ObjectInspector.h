// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 2 paquetes del corte B:
//   E-12  (fase E, 4 j) - Propietats de mapa, capes i instàncies
//   E-14  (fase E, 3 j) - Selectors de dades i referències
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Editor/ObjectInspectorData.h"
#include "Level/LevelDefinition.h"

class EditorState;
class ObjectCatalog;

// ---------------------------------------------------------------------
// ObjectInspector — el panel de propiedades de un objeto YA COLOCADO,
// SIN abrir una ventana.
//
// Mismo criterio que EditorState/ProjectHub/CharacterCreationScreen: el
// estado de sesión (que campo está marcado, qué modo de edición y el buffer
// de texto) vive en ObjectInspectorData y se prueba de verdad en
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

class ObjectInspectorInput;

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
    bool isOpen() const { return m_data.open; }
    GridCoord cell() const { return m_data.cell; }

    // El spawn inspeccionado (nullptr si desaparecio del nivel).
    const ObjectSpawn* spawn() const;

    // La puerta unica del bucle de teclado (igual que ProjectHub::key):
    //   W/S     campo anterior / siguiente, o mover el selector de un picker
    //   ENTER   editar el campo marcado (o confirmar la edicion)
    //   ESC     cancelar edicion / cerrar el panel
    //   X       quitar la entrada de botin marcada
    //   letras  escriben en el buffer o filtran el picker de botin
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
    const std::vector<std::string>& lines() const { return m_data.lines; }
    std::size_t selected() const { return m_data.selected; }
    bool editando() const { return m_data.mode != ObjectInspectorMode::Navegar; }
    bool editandoTexto() const { return m_data.mode == ObjectInspectorMode::Texto; }
    const std::string& buffer() const { return m_data.buffer; }
    const std::string& aviso() const { return m_data.notice; }

private:
    using Campo = ObjectInspectorField;
    using Modo = ObjectInspectorMode;
    using Fila = ObjectInspectorRow;

    void rebuild();
    void abrirTexto(const std::string& inicial);
    void commitTexto();
    std::string nombreLoot(const std::string& id) const;

    EditorState& m_editor;           // colaborador: documento editable, no propietario.
    const ObjectCatalog& m_catalog;  // colaborador: nombres y opciones de objetos, no propietario.
    ObjectInspectorData m_data;      // estado efímero de la sesión del panel.

    friend class ObjectInspectorInput;
};

}  // namespace Editor
