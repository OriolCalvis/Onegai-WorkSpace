// EST-21 (relevo A) — El nucleo del editor de niveles, sin ventana.
//
// Hasta hoy esto no se podia probar: vivia dentro de las 2777 lineas de
// editor/src/level_editor.cpp, junto al bucle de GL, y cualquier
// comprobacion habria necesitado abrir una ventana. Son funciones puras
// -- texto entra, texto sale -- y por eso el primer trozo que sale del
// fichero grande es este.
//
// QUE VIGILA CADA BLOQUE:
//   · que los nombres que ve el autor no sean rutas ni numeros
//   · que el switch de herramientas siga cubriendo TODAS las del enum
//   · que la linea de comandos no se coma el modo humo de CI
//   · que un tile sin semantica declarada caiga en un nombre de reserva
//     y no en un hueco

#include "LevelEditorPaleta.h"
#include "LevelEditorOpciones.h"
#include "LevelEditorEditState.h"

#include "TestHarness.h"

#include <string>
#include <vector>

namespace {

using namespace EditorNiveles;

// argv de mentira. parseArgs no toca disco ni GL, asi que basta con esto.
struct Linea {
    std::vector<std::string> texto;
    std::vector<char*> punteros;
    explicit Linea(std::initializer_list<const char*> args) {
        texto.emplace_back("level_editor");
        for (const char* a : args) texto.emplace_back(a);
        for (auto& s : texto) punteros.push_back(s.data());
    }
    int argc() const { return static_cast<int>(punteros.size()); }
    char** argv() { return punteros.data(); }
};

// ---------------------------------------------------------------- nombres

MOTOR_TEST_CASE(el_autor_ve_nombres_no_rutas) {
    MOTOR_CHECK(nombreEscenario("assets/levels/interior_taberna.json") == "interior taberna");
    MOTOR_CHECK(nombreEscenario("plaza_mayor.json") == "plaza mayor");
    // Sin extension ni carpeta sigue siendo legible, no vacio.
    MOTOR_CHECK(nombreEscenario("entrada") == "entrada");
}

MOTOR_TEST_CASE(la_barra_de_estado_va_en_caja_alta) {
    // BitmapFont solo tiene caja alta: si esto deja de cumplirse, la barra
    // se ve con huecos en vez de letras.
    MOTOR_CHECK(aMayus("pintar tile") == "PINTAR TILE");
    MOTOR_CHECK(aMayus("") == "");
}

MOTOR_TEST_CASE(toda_herramienta_tiene_nombre) {
    // El switch de toolName no tiene default que tape un enum nuevo: si
    // alguien anade una herramienta y se olvida del nombre, aqui sale "?"
    // en vez de descubrirlo con la barra de estado puesta.
    const EditorTool todas[] = {
        EditorTool::PaintTile, EditorTool::EraseTile, EditorTool::PlaceObject,
        EditorTool::RemoveObject, EditorTool::SetPlayerStart, EditorTool::FillTiles,
        EditorTool::LinkLevel};
    for (EditorTool t : todas) {
        const std::string nombre = toolName(t);
        MOTOR_CHECK(!nombre.empty());
        MOTOR_CHECK(nombre != "?");
    }
}

MOTOR_TEST_CASE(cada_categoria_tiene_nombre) {
    const ObjectCategory todas[] = {ObjectCategory::Npc, ObjectCategory::Pickup,
                                    ObjectCategory::Enemy, ObjectCategory::Prop};
    for (ObjectCategory c : todas) {
        MOTOR_CHECK(std::string(nombreCategoria(c)) != "?");
    }
}

// ---------------------------------------------------------------- tiles

MOTOR_TEST_CASE(un_tile_sin_nombre_cae_en_uno_de_reserva) {
    // Ensenar "gid 4" al autor no es un lenguaje de autoria.
    const auto info = semanticasTile(4, "", false);
    MOTOR_CHECK(!info.name.empty());
    MOTOR_CHECK(info.name.find('4') == std::string::npos);
    // Y un gid fuera de la tabla tampoco deja un hueco.
    const auto lejano = semanticasTile(9999, "", false);
    MOTOR_CHECK(lejano.name.find("9999") != std::string::npos);
}

MOTOR_TEST_CASE(la_semantica_del_tile_sale_de_su_nombre) {
    const auto nieve = semanticasTile(1, "nieve_pisada", false);
    MOTOR_CHECK(nieve.affinity == "FRIO / NIEVE");
    MOTOR_CHECK(!nieve.requirement.empty());

    const auto lava = semanticasTile(2, "suelo_lava", true);
    MOTOR_CHECK(lava.affinity == "VOLCANICO / FUEGO");
    MOTOR_CHECK(lava.collision);

    // Lo que no encaja en ninguna familia no inventa efecto.
    const auto neutro = semanticasTile(3, "algo_raro", false);
    MOTOR_CHECK(neutro.affinity == "CONSTRUCCION");
    MOTOR_CHECK(neutro.effect == "NEUTRO");
}

MOTOR_TEST_CASE(un_fichero_que_no_esta_no_revienta) {
    MOTOR_CHECK(textoDeArchivo("no/existe/este/fichero.json").empty());
    MOTOR_CHECK(cargarSemanticaTiles("no/existe/tileset.tsx").empty());
}

// ---------------------------------------------------------------- filtros

MOTOR_TEST_CASE(el_filtro_de_familia_mira_id_raza_y_nombre) {
    ObjectDefinition elfo;
    elfo.id = "npc_guardia";
    elfo.raceId = "elfos_del_bosque";
    elfo.name = "Guardia";
    elfo.category = ObjectCategory::Npc;

    // La raza cuenta aunque el id no la mencione: si solo se mirase el id,
    // este PNJ no aparecia en su familia y el autor lo daba por perdido.
    MOTOR_CHECK(filtroAfinidadObjeto("PNJ · ELFOS / DROWS")(elfo));
    MOTOR_CHECK(!filtroAfinidadObjeto("PNJ · ORCOS / GOBLINS")(elfo));
    // "Otras razas" recoge a cualquier PNJ: nadie se queda sin familia.
    MOTOR_CHECK(filtroAfinidadObjeto("PNJ · OTRAS RAZAS")(elfo));

    ObjectDefinition espada;
    espada.id = "arma_espada_corta";
    espada.category = ObjectCategory::Pickup;
    MOTOR_CHECK(filtroAfinidadObjeto("RECOGIBLES · EQUIPO")(espada));
    // Y no se cuela entre los consumibles: son listas complementarias.
    MOTOR_CHECK(!filtroAfinidadObjeto("RECOGIBLES · CONSUMIBLES")(espada));

    // Una familia desconocida no esconde nada: deja pasar todo.
    MOTOR_CHECK(filtroAfinidadObjeto("FAMILIA QUE NO EXISTE")(espada));
}

MOTOR_TEST_CASE(contiene_cualquiera_busca_subcadenas_exactas) {
    MOTOR_CHECK(contieneCualquiera("npc_tabernero", {"tabern"}));
    MOTOR_CHECK(contieneCualquiera("npc_tabernero", {"herrero", "npc_"}));
    MOTOR_CHECK(!contieneCualquiera("npc_tabernero", {"herrero", "mago"}));
    MOTOR_CHECK(!contieneCualquiera("", {"algo"}));
    // Busca subcadenas EXACTAS, sin declinar ni ignorar mayusculas:
    // "taberna" no encuentra "tabernero" (sobra la 'a'), y por eso los
    // tokens de filtroAfinidadObjeto estan escritos en raiz ("tabern",
    // "enan", "aran"). Quien anada un token largo se quedara sin
    // resultados y no sabra por que; esta linea lo deja dicho.
    MOTOR_CHECK(!contieneCualquiera("npc_tabernero", {"taberna"}));
    MOTOR_CHECK(!contieneCualquiera("npc_tabernero", {"TABERN"}));
}

MOTOR_TEST_CASE(un_objeto_que_no_esta_en_el_catalogo_se_dice) {
    ObjectCatalog vacio;
    // "SIN DEFINIR" en la paleta es una pista; un nombre vacio es un bug
    // que el autor descubre al colocar el objeto.
    MOTOR_CHECK(nombreDePaleta(vacio, "lo_que_sea") == "SIN DEFINIR");
}

// ---------------------------------------------------------------- opciones

MOTOR_TEST_CASE(el_argumento_numerico_suelto_sigue_siendo_el_modo_humo) {
    // CI depende de esta convencion: "./level_editor 3" corre 3 frames y
    // sale. Si una opcion nueva se la come, CI deja de comprobar el editor.
    Linea humo{"3"};
    const auto opts = parseArgs(humo.argc(), humo.argv());
    MOTOR_CHECK(opts.maxFrames == 3);
}

MOTOR_TEST_CASE(la_linea_de_comandos_entiende_lo_que_dice_la_ayuda) {
    Linea l{"--size", "64x48", "--load", "assets/levels/x.json"};
    const auto opts = parseArgs(l.argc(), l.argv());
    MOTOR_CHECK(opts.mapWidth == 64);
    MOTOR_CHECK(opts.mapHeight == 48);
    MOTOR_CHECK(opts.loadLevelPath == "assets/levels/x.json");
    MOTOR_CHECK(opts.maxFrames == -1);
}

MOTOR_TEST_CASE(un_tamano_absurdo_no_cambia_el_de_por_defecto) {
    const int anchoPorDefecto = EditorOptions{}.mapWidth;
    Linea l{"--size", "64"};
    const auto opts = parseArgs(l.argc(), l.argv());
    MOTOR_CHECK(opts.mapWidth == anchoPorDefecto);

    int w = 0, h = 0;
    MOTOR_CHECK(!parseSize("64", w, h));
    MOTOR_CHECK(!parseSize("x48", w, h));
    MOTOR_CHECK(!parseSize("0x48", w, h));
    MOTOR_CHECK(parseSize("64x48", w, h));
    MOTOR_CHECK(w == 64 && h == 48);
}

// ------------------------------------------------ estado de la fase legacy

MOTOR_TEST_CASE(la_salida_del_editor_pide_confirmacion_solo_con_cambios) {
    LevelEditorEditState limpio;
    MOTOR_CHECK(limpio.requestReturnToHub(false) == ReturnToHubRequest::ReturnToHub);
    MOTOR_CHECK(limpio.exit() == EditPhaseExit::ReturnToHub);

    LevelEditorEditState modificado;
    MOTOR_CHECK(modificado.requestReturnToHub(true) == ReturnToHubRequest::ConfirmDiscard);
    MOTOR_CHECK(modificado.returnConfirmationPending());
    MOTOR_CHECK(modificado.exit() == EditPhaseExit::Continue);
    modificado.cancelReturnConfirmation();
    MOTOR_CHECK(!modificado.returnConfirmationPending());
    MOTOR_CHECK(modificado.requestReturnToHub(true) == ReturnToHubRequest::ConfirmDiscard);
    MOTOR_CHECK(modificado.requestReturnToHub(true) == ReturnToHubRequest::ReturnToHub);
    MOTOR_CHECK(!modificado.returnConfirmationPending());
    MOTOR_CHECK(modificado.exit() == EditPhaseExit::ReturnToHub);

    LevelEditorEditState cambioDeNivel;
    cambioDeNivel.requestLevelChange();
    MOTOR_CHECK(cambioDeNivel.exit() == EditPhaseExit::ChangeLevel);
}

MOTOR_TEST_CASE(el_estado_temporal_no_duplica_la_seleccion_del_documento) {
    LevelEditorEditState state;
    MOTOR_CHECK(!state.selectionPending());
    state.beginSelection(GridCoord{3, 7});
    MOTOR_CHECK(state.selectionPending());
    MOTOR_CHECK(state.selectionFirstCorner().x == 3);
    MOTOR_CHECK(state.selectionFirstCorner().y == 7);
    state.finishSelection();
    MOTOR_CHECK(!state.selectionPending());
}

MOTOR_TEST_CASE(busqueda_y_navegacion_de_paleta_son_estado_de_vista) {
    LevelEditorEditState state;
    state.toggleSearch();
    MOTOR_CHECK(state.searchOpen());
    state.closeSearch();
    MOTOR_CHECK(!state.searchOpen());

    state.setObjectPalettePage(ObjectPalettePage::Subgroups);
    state.setObjectRootGroup(3);
    state.setObjectGroup(8);
    MOTOR_CHECK(state.objectPalettePage() == ObjectPalettePage::Subgroups);
    MOTOR_CHECK(state.objectRootGroup() == 3);
    MOTOR_CHECK(state.objectGroup() == 8);
    state.setObjectPalettePage(ObjectPalettePage::Entries);
    state.moveObjectPalettePageBack();
    MOTOR_CHECK(state.objectPalettePage() == ObjectPalettePage::Subgroups);
    state.moveObjectPalettePageBack();
    MOTOR_CHECK(state.objectPalettePage() == ObjectPalettePage::Root);
}

MOTOR_TEST_CASE(el_feedback_temporal_no_puede_volverse_negativo) {
    LevelEditorEditState state;
    state.showStatusFor(-4);
    MOTOR_CHECK(!state.advanceStatusFrame());
    state.showStatusFor(2);
    MOTOR_CHECK(!state.advanceStatusFrame());
    MOTOR_CHECK(state.advanceStatusFrame());
    MOTOR_CHECK(!state.advanceStatusFrame());

    state.triggerSaveFlash();
    state.tick(0.2f);
    MOTOR_CHECK(state.animationTime() > 0.19f && state.animationTime() < 0.21f);
    MOTOR_CHECK(state.saveFlashAlpha() > 0.49f && state.saveFlashAlpha() < 0.51f);
    state.tick(1.0f);
    MOTOR_CHECK(state.saveFlashAlpha() == 0.0f);
    const float elapsedBeforeNegativeTick = state.animationTime();
    state.tick(-1.0f);
    MOTOR_CHECK(state.animationTime() == elapsedBeforeNegativeTick);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
