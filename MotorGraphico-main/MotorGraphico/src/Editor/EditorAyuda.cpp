// EDITOR-UX-01 — ver include/Editor/EditorAyuda.h.
#include "Editor/EditorAyuda.h"

#include <map>

namespace Editor {

namespace {

struct FilaAccion {
    EditorAction id;
    const char* clave;
    AyudaAccion ayuda;
};

const FilaAccion kAcciones[] = {
    {EditorAction::Save, "save", {"guardar", "Guarda el mapa en su fichero JSON. Si no hay cambios, no hace nada."}},
    {EditorAction::Close, "close", {"cerrar", "Cierra el mapa abierto. Con cambios sin guardar pregunta antes."}},
    {EditorAction::Quit, "quit", {"salir", "Cierra el editor. Con cambios sin guardar pregunta antes."}},
    {EditorAction::Undo, "undo", {"deshacer", "Deshace el ultimo cambio del mapa (baldosas, objetos, capas)."}},
    {EditorAction::Redo, "redo", {"rehacer", "Vuelve a aplicar el ultimo cambio deshecho."}},
    {EditorAction::Paint, "paint", {"pintar", "Pinta la baldosa elegida en la capa activa. Clic derecho la borra."}},
    {EditorAction::Erase, "erase", {"borrar", "Vacia la celda en la capa activa (deja la baldosa 0)."}},
    {EditorAction::Fill, "fill", {"rellenar", "Cambia toda la zona conectada de la misma baldosa, en un solo paso de deshacer."}},
    {EditorAction::PlayerStart, "playerstart", {"inicio", "Marca la celda donde aparece el jugador al entrar en el mapa (circulo verde)."}},
    {EditorAction::Objects, "objects", {"objetos", "Coloca el objeto elegido del catalogo. Una celda ocupada no se sustituye: borra antes."}},
    {EditorAction::RemoveObject, "removeobject", {"borrar_objeto", "Quita el objeto de la celda (los puntos amarillos del mapa)."}},
    {EditorAction::Center, "center", {"centrar", "Vuelve a centrar la camara en el mapa y deja el zoom a 1."}},
    {EditorAction::Help, "help", {"ayuda", "Muestra u oculta la ventana con todos los atajos y el uso del raton."}},
    {EditorAction::Layers, "layers", {"capas", "Enfoca el panel de capas: crear, renombrar, ocultar, bloquear o borrar capas de baldosas."}},
};

const std::map<std::string, const char*>& controles() {
    static const std::map<std::string, const char*> tabla = {
        {"proyecto.buscar", "Filtra los mapas de todos los proyectos por su nombre o por el de su fichero."},
        {"proyecto.mapa", "Abre este mapa (con cambios sin guardar, pregunta antes). Clic derecho: duplicar, renombrar, empezar aquí o quitar."},
        // H4-02 / E-11 — gestion de proyectos y mapas desde el panel Proyecto.
        {"proyecto.nuevo_proyecto", "Crea un proyecto vacío: escribes su nombre y el id sale solo. Después pide su primer mapa."},
        {"proyecto.nuevo_mapa", "Crea un mapa en este proyecto: nombre, ancho, alto y tileset. Se guarda y se abre."},
        {"proyecto.nombre", "Nombre que ve el jugador. El fichero sale de él (sin acentos ni espacios) y no cambia al renombrar."},
        {"proyecto.titulo", "Título que ve el jugador en la pantalla de inicio. Vacío: se usa el nombre del proyecto."},
        {"proyecto.tamano", "Ancho y alto del mapa en celdas, de 3 a 256. El suelo sale de la primera baldosa que no es muro."},
        {"proyecto.tileset", "Baldosas del mapa nuevo: se copian de un mapa que ya existe, con sus muros."},
        {"proyecto.abierto.duplicar", "Copia el mapa abierto con otro nombre y otros ficheros; el original no cambia."},
        {"proyecto.abierto.renombrar", "Cambia el nombre que ve el jugador; el fichero se queda igual."},
        {"proyecto.abierto.inicio", "El juego empezará en este mapa (la entrada del proyecto)."},
        {"proyecto.abierto.quitar", "Saca el mapa del proyecto. Sus ficheros se quedan en disco; el de inicio no se puede quitar."},
        {"herramientas.objeto.buscar", "Filtra el catalogo de objetos del proyecto por nombre."},
        {"herramientas.objeto", "Objeto que colocara la herramienta Objetos. Pasa el raton para ver su id."},
        {"herramientas.baldosa", "Baldosa que pintan Pintar y Rellenar. El numero es su gid en el tileset."},
        {"herramientas.uso", "Que hace cada boton del raton con la herramienta activa."},
        {"mapa.vista", "Clic izquierdo aplica la herramienta; derecho borra; central mueve la vista; rueda hace zoom."},
        {"inspector.nombre", "Nombre del mapa tal como lo lee el juego (level.json)."},
        {"inspector.tamano", "Ancho y alto del mapa en celdas; lo fija el TMX importado."},
        {"inspector.objetos", "Objetos colocados en el mapa (puntos amarillos)."},
        {"inspector.inicio", "Celda de aparicion del jugador (circulo verde). Se cambia con la herramienta Inicio."},
        {"inspector.capas", "Capas de baldosas del mapa, en orden de dibujo."},
        {"inspector.seleccion", "Ultima celda en la que hiciste clic (circulo azul): su baldosa y su objeto."},
        {"inspector.guardado", "Si el mapa tiene cambios que no estan en disco."},
        {"inspector.estado", "Resultado de la ultima accion: 'Listo' o el motivo del error."},
        {"capas.lista", "Capa activa: en ella pintan Pintar, Borrar y Rellenar. Clic para cambiar."},
        {"capas.nueva", "Anade una capa de baldosas vacia encima y la activa."},
        {"capas.nombre", "Nombre de la capa; Enter o Renombrar lo aplica."},
        {"capas.renombrar", "Aplica el nombre escrito arriba."},
        {"capas.visible", "Oculta o muestra la capa en el editor y en el juego."},
        {"capas.bloqueada", "Una capa bloqueada no se pinta ni se borra: protege lo terminado."},
        {"capas.borrar", "Borra la capa y sus baldosas (se puede deshacer). Debe quedar al menos una."},
    };
    return tabla;
}

}  // namespace

const AyudaAccion& ayudaDeAccion(EditorAction id) {
    for (const FilaAccion& fila : kAcciones) {
        if (fila.id == id) return fila.ayuda;
    }
    static const AyudaAccion vacia{"", ""};
    return vacia;
}

const char* claveDeAccion(EditorAction id) {
    for (const FilaAccion& fila : kAcciones) {
        if (fila.id == id) return fila.clave;
    }
    return "";
}

const char* ayudaDeControl(const std::string& id) {
    const auto& tabla = controles();
    const auto it = tabla.find(id);
    return it == tabla.end() ? "" : it->second;
}

const std::vector<std::string>& controlesConAyuda() {
    static const std::vector<std::string> ids = [] {
        std::vector<std::string> out;
        for (const auto& [id, texto] : controles()) out.push_back(id);
        return out;
    }();
    return ids;
}

std::string usoDelRaton(EditorAction tool) {
    switch (tool) {
        case EditorAction::Objects: return "Clic: colocar objeto. Derecho: borrar objeto. Central: mover vista. Rueda: zoom.";
        case EditorAction::RemoveObject: return "Clic: borrar objeto. Central: mover vista. Rueda: zoom.";
        case EditorAction::PlayerStart: return "Clic: fijar el inicio del jugador. Central: mover vista. Rueda: zoom.";
        case EditorAction::Fill: return "Clic: rellenar la zona. Derecho: borrar baldosa. Central: mover vista. Rueda: zoom.";
        case EditorAction::Erase: return "Clic: vaciar la celda. Central: mover vista. Rueda: zoom.";
        default: return "Clic: pintar. Derecho: borrar baldosa. Central: mover vista. Rueda: zoom.";
    }
}

}  // namespace Editor
