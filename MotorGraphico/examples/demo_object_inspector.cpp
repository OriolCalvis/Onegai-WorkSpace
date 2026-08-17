// El panel de propiedades de un objeto colocado (Editor::ObjectInspector),
// probado SIN ventana: mismo criterio que demo_editor_state y
// demo_proyectos. El estado del panel (campo marcado, modo de edicion,
// buffer, pickers) es GL-free a proposito: level_editor.cpp solo pinta
// lines()/titulo()/aviso() y pasa teclas, asi que TODO lo que hace el
// panel se verifica aqui de verdad, no "se compila y ya".
//
// Que se prueba:
//   1. Apertura: solo sobre celdas con objeto; titulo con nombre y celda.
//   2. Edicion de texto (nombre visible, variante, efecto, tabla de
//      botin, evento): buffer, ENTER confirma, ESC cancela.
//   3. Escala: ENTER cicla los cinco tamaños, igual que el atajo H.
//   4. Salidas: picker de destinos, "(sin salida)" desconecta.
//   5. Botin: picker con filtro por texto, duplicados rechazados, X quita.
//   6. Propiedades libres clave=valor: anadir y quitar.
//   7. El objeto desaparece con el panel abierto: el panel se cierra solo.
//   8. Round-trip: exportLevelJson -> LevelLoader conserva los campos
//      nuevos (loot, lootTable, eventId, displayName, targetLevel).
#include "Editor/EditorState.h"
#include "Editor/ObjectInspector.h"
#include "Level/LevelLoader.h"
#include "Level/ObjectCatalog.h"

#include "Check.h"

#include <cstdio>
#include <string>
#include <vector>

namespace {

ObjectDefinition def(const std::string& id, const std::string& name, ObjectCategory cat) {
    ObjectDefinition d;
    d.id = id;
    d.name = name;
    d.category = cat;
    return d;
}

void teclear(Editor::ObjectInspector& inspector, const std::string& texto) {
    for (char c : texto) {
        inspector.key(c);
    }
}

}  // namespace

int main() {
    std::printf("demo_object_inspector — el panel de propiedades, sin ventana\n\n");

    ObjectCatalog catalog;
    catalog.add(def("cofre", "Cofre de madera", ObjectCategory::Prop));
    catalog.add(def("puerta_posada", "Puerta de la posada", ObjectCategory::Prop));
    catalog.add(def("pocion", "Pocion", ObjectCategory::Pickup));
    catalog.add(def("pocion_mana", "Pocion de mana", ObjectCategory::Pickup));
    catalog.add(def("llave_oxidada", "Llave oxidada", ObjectCategory::Pickup));
    catalog.add(def("tabernero", "Tabernero", ObjectCategory::Npc));
    require(catalog.size() == 6);

    EditorState editor(8, 8);
    editor.placeObject(3, 2, "cofre");
    editor.placeObject(5, 4, "puerta_posada");
    editor.placeObject(6, 6, "tabernero");

    Editor::ObjectInspector inspector(editor, catalog,
                                      {"assets/levels/nivel_a.json",
                                       "assets/levels/nivel_b.json"});

    // ------------------------------------------------------------------
    std::printf("[apertura]\n");
    inspector.open(0, 0);
    require(!inspector.isOpen());   // celda vacia: nada que inspeccionar
    inspector.open(3, 2);
    require(inspector.isOpen());
    require(inspector.cell().x == 3 && inspector.cell().y == 2);
    require(inspector.titulo().find("Cofre de madera") != std::string::npos);
    require(inspector.titulo().find("(3,2)") != std::string::npos);
    require(!inspector.lines().empty());
    require(inspector.selected() == 0);
    std::printf("    solo se abre sobre objetos; titulo con nombre y celda. OK\n");

    // ------------------------------------------------------------------
    std::printf("[nombre visible]\n");
    require(inspector.key('\n'));            // ENTER edita el campo 0
    require(inspector.editando());
    teclear(inspector, "Cofre del atrio");
    require(inspector.key('\n'));            // confirma
    require(!inspector.editando());
    require(editor.objectAt(3, 2)->displayName == "Cofre del atrio");
    // ESC cancela: el buffer no llega al spawn.
    inspector.key('\n');
    teclear(inspector, "Texto descartado");
    require(inspector.key(27));
    require(editor.objectAt(3, 2)->displayName == "Cofre del atrio");
    std::printf("    ENTER confirma, ESC cancela. OK\n");

    // ------------------------------------------------------------------
    std::printf("[escala]\n");
    inspector.key('s');   // campo 1: variante
    inspector.key('s');   // campo 2: escala
    require(editor.objectAt(3, 2)->scale == 1.0f);
    inspector.key('\n');
    require(editor.objectAt(3, 2)->scale == 1.5f);
    inspector.key('\n');
    require(editor.objectAt(3, 2)->scale == 2.0f);
    std::printf("    ENTER cicla 100%% -> 150%% -> 200%%. OK\n");

    // ------------------------------------------------------------------
    std::printf("[variante, efecto, tabla de botin, evento]\n");
    inspector.key('w');   // vuelve a variante
    inspector.key('\n');
    teclear(inspector, "mision");
    inspector.key('\n');
    require(editor.objectAt(3, 2)->variant == "mision");
    inspector.key('s');   // escala
    inspector.key('s');   // efecto
    inspector.key('\n');
    teclear(inspector, "curar");
    inspector.key('\n');
    require(editor.objectAt(3, 2)->effectOverride == "curar");
    // Tabla de botin y evento: navegar hasta sus filas (tras escala y
    // efecto vienen salida, llegada y el botin, que aun esta vacio).
    inspector.key('s');   // salida
    inspector.key('s');   // llegada
    inspector.key('s');   // + ANADIR BOTIN (sin botin, es la siguiente)
    inspector.key('s');   // tabla de botin
    inspector.key('\n');
    teclear(inspector, "botin_cofre_atrio");
    inspector.key('\n');
    require(editor.objectAt(3, 2)->lootTable == "botin_cofre_atrio");
    inspector.key('s');   // evento/mision
    inspector.key('\n');
    teclear(inspector, "ev_cofre_del_atrio");
    inspector.key('\n');
    require(editor.objectAt(3, 2)->eventId == "ev_cofre_del_atrio");
    std::printf("    variante/efecto/tabla/evento editados por teclado. OK\n");

    // ------------------------------------------------------------------
    std::printf("[botin con filtro]\n");
    // Vuelve a "+ ANADIR BOTIN": esta dos filas arriba de la marca actual.
    inspector.key('w');   // tabla de botin
    inspector.key('w');   // + ANADIR BOTIN
    inspector.key('\n');
    teclear(inspector, "poc");
    inspector.key('\n');   // primera coincidencia del filtro
    {
        const ObjectSpawn* cofre = editor.objectAt(3, 2);
        require(cofre->loot.size() == 1);
        require(cofre->loot[0] == "pocion" || cofre->loot[0] == "pocion_mana");
        std::printf("    filtro 'poc' anade la primera coincidencia (%s). OK\n",
                    cofre->loot[0].c_str());
    }
    // Tras anadir, la marca cae sobre la entrada nueva; "+ ANADIR
    // BOTIN" queda justo debajo.
    inspector.key('s');
    inspector.key('\n');
    teclear(inspector, "llave");
    inspector.key('\n');
    require(editor.objectAt(3, 2)->loot.size() == 2);
    require(editor.objectAt(3, 2)->loot[1] == "llave_oxidada");
    // X sobre la entrada marcada la quita (con deshacer disponible).
    inspector.key('w');   // sobre la entrada llave_oxidada
    require(inspector.key('x'));
    require(editor.objectAt(3, 2)->loot.size() == 1);
    std::printf("    anadir con filtro, quitar con X. OK\n");

    // ------------------------------------------------------------------
    std::printf("[salidas]\n");
    inspector.close();
    inspector.open(5, 4);   // la puerta
    require(inspector.isOpen());
    inspector.key('s');   // variante
    inspector.key('s');   // escala
    inspector.key('s');   // efecto
    inspector.key('s');   // salida
    inspector.key('\n');
    // Picker: marca en "(sin salida)"; un 's' lleva al primer destino.
    inspector.key('s');
    inspector.key('\n');
    require(editor.objectAt(5, 4)->targetLevel == "assets/levels/nivel_a.json");
    // Posicion de llegada escrita como x,y.
    inspector.key('s');   // llegada
    inspector.key('\n');
    teclear(inspector, "7,1");
    inspector.key('\n');
    require(editor.objectAt(5, 4)->hasTargetPosition);
    require(editor.objectAt(5, 4)->targetPosition.x == 7);
    require(editor.objectAt(5, 4)->targetPosition.y == 1);
    // "(sin salida)" desconecta la puerta. El picker abre marcado en el
    // destino actual; un 'w' dentro del picker lleva a "(sin salida)".
    inspector.key('w');   // salida otra vez
    inspector.key('\n');
    inspector.key('w');
    inspector.key('\n');
    require(editor.objectAt(5, 4)->targetLevel.empty());
    require(!editor.objectAt(5, 4)->hasTargetPosition);
    std::printf("    elegir destino, llegada x,y, desconectar. OK\n");

    // ------------------------------------------------------------------
    std::printf("[propiedades libres]\n");
    inspector.close();
    inspector.open(6, 6);   // el tabernero
    // Baja hasta "+ ANADIR PROPIEDAD": sin botin ni propiedades, la
    // lista tiene 10 filas y la de anadir es la ultima (indice 9).
    for (int i = 0; i < 9; ++i) {
        inspector.key('s');
    }
    inspector.key('\n');
    teclear(inspector, "saludo=bienvenido a la posada");
    inspector.key('\n');
    require(editor.objectAt(6, 6)->properties.count("saludo") == 1);
    require(editor.objectAt(6, 6)->properties.at("saludo") == "bienvenido a la posada");
    std::printf("    clave=valor anadida. OK\n");

    // ------------------------------------------------------------------
    std::printf("[objeto borrado con el panel abierto]\n");
    editor.removeObjectAt(6, 6);
    require(inspector.key('w'));   // cualquier tecla lo cierra
    require(!inspector.isOpen());
    std::printf("    el panel se cierra solo al desaparecer el objeto. OK\n");

    // ------------------------------------------------------------------
    std::printf("[deshacer]\n");
    const std::string antes = editor.objectAt(3, 2)->displayName;
    inspector.open(3, 2);
    inspector.key('\n');
    // La edicion EMPIEZA en el valor actual y anade al final: para
    // reemplazar se borra primero (BACKSPACE), igual que en un campo de
    // texto de verdad.
    for (std::size_t i = 0; i < antes.size(); ++i) {
        inspector.key('\b');
    }
    teclear(inspector, "Otro nombre");
    inspector.key('\n');
    require(editor.objectAt(3, 2)->displayName == "Otro nombre");
    require(editor.undo());
    require(editor.objectAt(3, 2)->displayName == antes);
    std::printf("    cada edicion del panel es un paso de deshacer. OK\n");

    // ------------------------------------------------------------------
    std::printf("[round-trip export -> LevelLoader]\n");
    const std::string json = editor.exportLevelJson("Nivel del inspector",
                                                    "assets/maps/prueba.tmx");
    auto cargado = LevelLoader::loadFromString(json);
    require(cargado.isOk());
    const ObjectSpawn* cofre = nullptr;
    const ObjectSpawn* puerta = nullptr;
    for (const ObjectSpawn& spawn : cargado.value().objects) {
        if (spawn.objectId == "cofre") cofre = &spawn;
        if (spawn.objectId == "puerta_posada") puerta = &spawn;
    }
    require(cofre != nullptr && puerta != nullptr);
    require(cofre->displayName == "Cofre del atrio");
    require(cofre->variant == "mision");
    require(cofre->effectOverride == "curar");
    require(cofre->scale == 2.0f);
    require(cofre->lootTable == "botin_cofre_atrio");
    require(cofre->eventId == "ev_cofre_del_atrio");
    require(cofre->loot.size() == 1);   // la llave se quito con X
    require(puerta->targetLevel.empty());   // desconectada al final
    std::printf("    los campos nuevos sobreviven al guardado y la carga. OK\n");

    std::printf("\ntodas las comprobaciones han pasado\n");
    return 0;
}
