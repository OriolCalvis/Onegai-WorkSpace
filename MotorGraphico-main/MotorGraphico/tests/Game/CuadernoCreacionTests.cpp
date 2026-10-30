// UI-BG3-01 — La creacion en siete hojas (Game/CuadernoCreacion.h), sin
// ventana: navegar con teclado y con raton, elegir en las rejillas, la
// historia en una matriz, el nombre escrito y la ficha final sin equipo.
#include "TestHarness.h"

#include <stdexcept>
#include <string>

#include "Game/CuadernoCreacion.h"
#include "Game/Personajes/KitBasico.h"
#include "Game/Personajes/ManifiestoPersonajes.h"
#include "RPG/CharacterCreation.h"
#include "RPG/TierRules.h"

namespace {

using Game::CuadernoCreacion;
using Hoja = CuadernoCreacion::Hoja;
using Zona = CuadernoCreacion::Zona;

const Game::ManifiestoPersonajes& manifiesto() {
    static const Game::ManifiestoPersonajes m = [] {
        auto r = Game::ManifiestoPersonajes::cargar("assets/textures");
        if (!r.isOk()) throw std::runtime_error(r.errorMessage());
        return r.value();
    }();
    return m;
}

const RPG::CharacterCreation& reglas() {
    static const RPG::CharacterCreation c = [] {
        auto r = RPG::CharacterCreation::load("assets/catalogs");
        if (!r.isOk()) throw std::runtime_error(r.errorMessage());
        return std::move(r.value());
    }();
    return c;
}

const RPG::Catalogs::EquipmentCatalog& equipo() {
    static const RPG::Catalogs::EquipmentCatalog cat = [] {
        RPG::Catalogs::EquipmentCatalog c;
        auto r = c.loadFromFile("assets/catalogs/equipment.json");
        if (!r.isOk()) throw std::runtime_error(r.errorMessage());
        return c;
    }();
    return cat;
}

const RPG::TierRules& tiers() {
    static const RPG::TierRules t = [] {
        RPG::TierRules r;
        (void)r.loadFromFile("assets/rules/tier_rules.json");
        return r;
    }();
    return t;
}

const Game::KitBasico::Reglas& reglasKit() {
    static const Game::KitBasico::Reglas r = Game::KitBasico::cargarReglas().value();
    return r;
}

const std::map<std::string, CuadernoCreacion::Rasgo>& rasgos() {
    static const auto r = CuadernoCreacion::leeRasgos("assets/catalogs/races.json");
    return r;
}

CuadernoCreacion::Datos datos() {
    CuadernoCreacion::Datos d;
    d.reglas = &reglas();
    d.tiers = &tiers();
    d.personajes = &manifiesto();
    d.equipo = &equipo();
    d.kit = &reglasKit();
    d.rasgos = &rasgos();
    return d;
}

int posicion(const CuadernoCreacion& c, Hoja h, const std::string& id) {
    const auto lista = c.ids(h);
    for (std::size_t i = 0; i < lista.size(); ++i)
        if (lista[i] == id) return static_cast<int>(i);
    return -1;
}

// Rellena todo hasta la hoja Nombre (con raton, como haria el jugador).
void hastaElNombre(CuadernoCreacion& c) {
    c.pulsa(Zona::Tesela, posicion(c, Hoja::Raza, "alquimistas_de_aegroum"));
    c.pulsa(Zona::Pestana, static_cast<int>(Hoja::Clase));
    c.pulsa(Zona::Tesela, posicion(c, Hoja::Clase, "bardo_de_las_cuatro_estaciones"));
    c.pulsa(Zona::Pestana, static_cast<int>(Hoja::Origen));
    c.pulsa(Zona::Tesela, posicion(c, Hoja::Origen, "mes_de_la_cosecha"));
    c.pulsa(Zona::Pestana, static_cast<int>(Hoja::Historia));
    for (int fila = 0; fila < CuadernoCreacion::kPreguntas; ++fila) c.pulsa(Zona::Tarjeta, fila * 5 + fila % 5);
    c.pulsa(Zona::Pestana, static_cast<int>(Hoja::Deidad));
    c.pulsa(Zona::Tesela, 0);  // Ninguna
    c.pulsa(Zona::Pestana, static_cast<int>(Hoja::Nombre));
}

MOTOR_TEST_CASE(empieza_en_raza_con_las_69_y_sin_nada_elegido) {
    CuadernoCreacion c(datos());
    MOTOR_CHECK(c.hoja() == Hoja::Raza);
    MOTOR_CHECK(c.ids().size() == 69);  // 72 del catalogo menos los dos yokai y los Espectros (AV16-01); 27 del 2000 b.f.
    for (const std::string& id : c.ids()) MOTOR_CHECK(id != "espectros" && id.rfind("yokai", 0) != 0);
    CuadernoCreacion busca(datos());
    busca.pulsa(Zona::Buscar, 0);
    busca.escribe("espectro");  // ni con la busqueda
    MOTOR_CHECK(busca.ids().empty());
    MOTOR_CHECK(c.foco() == 0);
    MOTOR_CHECK(!c.hecha(Hoja::Raza));
    c.continuar();  // sin raza no se sigue, y se dice por que
    MOTOR_CHECK(c.hoja() == Hoja::Raza);
    MOTOR_CHECK(!c.aviso().empty());
    // Las rejillas van por orden alfabetico, sin que las tildes manden.
    const auto ids = c.ids();
    MOTOR_CHECK(ids.front() == "aarakocra_de_las_montanas");
}

MOTOR_TEST_CASE(pasar_por_encima_senala_y_el_clic_elige) {
    CuadernoCreacion c(datos());
    const int alquimistas = posicion(c, Hoja::Raza, "alquimistas_de_aegroum");
    MOTOR_CHECK(alquimistas >= 0);
    c.senala(Zona::Tesela, alquimistas);
    MOTOR_CHECK(c.foco() == alquimistas);
    MOTOR_CHECK(c.senalado() == "alquimistas_de_aegroum");
    MOTOR_CHECK(c.eleccion().raceId.empty());  // senalar no elige
    // La vista ensena lo senalado; AV16-02: sin clase elegida no se destaca ningun atributo.
    const auto v = c.vista();
    MOTOR_CHECK(v.apariencia.raza == "alquimistas_de_aegroum");
    for (bool p : v.principales) MOTOR_CHECK(!p);
    c.pulsa(Zona::Tesela, alquimistas);
    MOTOR_CHECK(c.eleccion().raceId == "alquimistas_de_aegroum");
    MOTOR_CHECK(c.editor() != nullptr && c.editor()->apariencia().raza == "alquimistas_de_aegroum");
    MOTOR_CHECK(c.hecha(Hoja::Raza));
}

MOTOR_TEST_CASE(teclado_wasd_mueve_en_la_rejilla_y_enter_dos_veces_elige_y_sigue) {
    CuadernoCreacion c(datos());
    c.mueve(1, 0);
    c.mueve(0, 1);
    MOTOR_CHECK(c.foco() == 6);  // segunda fila, segunda columna
    c.acepta();
    MOTOR_CHECK(c.eleccion().raceId == c.ids()[6]);
    MOTOR_CHECK(c.hoja() == Hoja::Raza);
    c.acepta();  // ENTER sobre lo ya elegido: a la hoja siguiente
    MOTOR_CHECK(c.hoja() == Hoja::Clase);  // DIALOGOS-01: el aspecto va despues de la clase
    // Bajar mas alla de las filas visibles mueve la rejilla.
    CuadernoCreacion d(datos());
    for (int i = 0; i < 5; ++i) d.mueve(0, 1);
    MOTOR_CHECK(d.foco() == 25);
    MOTOR_CHECK(d.primeraFila() == 2);  // 4 filas visibles: se ven la 2..5
    d.rueda(5);
    MOTOR_CHECK(d.primeraFila() == 0);
}

MOTOR_TEST_CASE(q_e_y_las_pestanas_saltan_de_hoja_y_esc_vuelve_o_cancela) {
    CuadernoCreacion c(datos());
    c.anterior();
    MOTOR_CHECK(c.hoja() == Hoja::Raza);  // no hay hoja antes de Raza
    c.siguiente();
    c.siguiente();
    MOTOR_CHECK(c.hoja() == Hoja::Aspecto);  // DIALOGOS-01: Raza, Clase, Aspecto
    c.pulsa(Zona::Pestana, static_cast<int>(Hoja::Nombre));
    MOTOR_CHECK(c.hoja() == Hoja::Nombre);
    c.siguiente();
    MOTOR_CHECK(c.hoja() == Hoja::Nombre);
    c.atras();
    MOTOR_CHECK(c.hoja() == Hoja::Deidad);
    c.irA(Hoja::Raza);
    c.atras();
    MOTOR_CHECK(c.estado() == CuadernoCreacion::Estado::Cancelado);
}

MOTOR_TEST_CASE(la_busqueda_filtra_razas_sin_tildes_y_las_letras_no_mueven) {
    CuadernoCreacion c(datos());
    MOTOR_CHECK(!c.escribiendo());
    c.pulsa(Zona::Buscar, 0);
    MOTOR_CHECK(c.escribiendo());
    c.escribe("DRACON");
    const auto lista = c.ids();
    MOTOR_CHECK(lista.size() >= 2);  // Draconidos de Fuego y de Hielo (con tilde en el nombre)
    for (const std::string& id : lista) MOTOR_CHECK(id.find("dracon") != std::string::npos);
    c.borra();
    MOTOR_CHECK(c.busqueda() == "DRACO");
    c.atras();  // ESC sale del campo sin cancelar la creacion
    MOTOR_CHECK(!c.escribiendo());
    MOTOR_CHECK(c.estado() == CuadernoCreacion::Estado::Creando);
    MOTOR_CHECK(c.busqueda() == "DRACO");
}

MOTOR_TEST_CASE(el_filtro_de_papel_deja_solo_esa_clase_y_el_kit_se_ve) {
    CuadernoCreacion c(datos());
    c.irA(Hoja::Clase);
    std::size_t jugables = 0;  // DIALOGOS-01: solo las que se juegan (70)
    reglas().classes().forEach([&](const RPG::ClassDefinition& k) { jugables += k.playable ? 1 : 0; });
    MOTOR_CHECK(jugables == 70);
    MOTOR_CHECK(c.ids().size() == jugables);
    int lanzador = -1;
    for (std::size_t i = 0; i < CuadernoCreacion::roles().size(); ++i)
        if (CuadernoCreacion::roles()[i].first == "caster") lanzador = static_cast<int>(i);
    c.pulsa(Zona::Rol, lanzador);
    MOTOR_CHECK(c.rol() == "caster");
    // Las del catalogo con ese rol (16 antes de SP-02; el Mago suma seis especialistas que lanzan)
    std::size_t lanzadoras = 0;
    reglas().classes().forEach([&](const RPG::ClassDefinition& k) { lanzadoras += k.playable && k.role == "caster" ? 1 : 0; });
    MOTOR_CHECK(lanzadoras >= 10);  // DIALOGOS-01: de las 70 que se juegan
    MOTOR_CHECK(c.ids().size() == lanzadoras);
    for (const std::string& id : c.ids()) MOTOR_CHECK(reglas().classes().find(id)->role == "caster");
    c.pulsa(Zona::Rol, 0);  // Todas
    const int bardo = posicion(c, Hoja::Clase, "bardo_de_las_cuatro_estaciones");
    c.senala(Zona::Tesela, bardo);
    // Con el kit puesto: +2 CAR y +2 DES (principal y secundario), +1 en
    // los otros dos, y las piezas que se ven.
    const auto v = c.vista();
    MOTOR_CHECK(v.conKit);
    MOTOR_CHECK(v.kit[0] == 1 && v.kit[1] == 2 && v.kit[2] == 1 && v.kit[3] == 2);  // el dato sigue; la pantalla ya no lo ensena
    // AV16-02: se destacan los atributos principales de la clase (bardo: CAR principal y DES secundario).
    MOTOR_CHECK(!v.principales[0] && v.principales[1] && !v.principales[2] && v.principales[3]);
    MOTOR_CHECK(!v.equipo.empty());
    MOTOR_CHECK(c.kit("bardo_de_las_cuatro_estaciones").piezas.size() == 5);
}

MOTOR_TEST_CASE(sin_origen_no_hay_historia_y_cambiarlo_la_vacia) {
    CuadernoCreacion c(datos());
    c.irA(Hoja::Historia);
    MOTOR_CHECK(c.opciones(0) == nullptr);
    c.continuar();
    MOTOR_CHECK(c.aviso().find("origen") != std::string::npos);
    c.irA(Hoja::Origen);
    c.pulsa(Zona::Tesela, posicion(c, Hoja::Origen, "mes_de_la_cosecha"));
    c.irA(Hoja::Historia);
    MOTOR_CHECK(c.opciones(0) != nullptr && c.opciones(0)->size() == 5);
    // Con teclado: ENTER elige la tarjeta y baja a la pregunta siguiente.
    for (int fila = 0; fila < CuadernoCreacion::kPreguntas; ++fila) c.acepta();
    MOTOR_CHECK(c.hecha(Hoja::Historia));
    MOTOR_CHECK(!c.eleccion().virtueId.empty());
    c.irA(Hoja::Origen);
    const auto lista = c.ids();
    c.pulsa(Zona::Tesela, lista.front() == "mes_de_la_cosecha" ? 1 : 0);
    MOTOR_CHECK(!c.hecha(Hoja::Historia));
    MOTOR_CHECK(c.eleccion().bondId.empty());
}

// RAZAS-02 — Oriol (08/10): «hay que quitar todo lo relacionado con D&D». Los
// rasgos de los pueblos salen de sus documentos de razas, escritos como cartas:
// sin «Rasgo de X: Competencia en...», sin alineamientos, sin el tamaño de D&D
// (queda la altura) y sin stats que el juego no tiene. Los Poseídos se llaman
// como en su documento.
MOTOR_TEST_CASE(los_pueblos_ya_no_traen_la_plantilla_de_dnd) {
    int vistas = 0;
    reglas().races().forEach([&](const RPG::RaceDefinition& r) {
        ++vistas;
        for (const std::string& tag : r.narrativeTags)
            for (const char* alineamiento : {"Bueno", "Malo", "Legal", "Caótico", "Neutral"})
                MOTOR_CHECK(tag.find(alineamiento) == std::string::npos);
        MOTOR_CHECK(r.description.find("Tamaño:") == std::string::npos);
        const auto it = rasgos().find(r.id);
        MOTOR_CHECK(it != rasgos().end());
        if (it == rasgos().end()) return;
        MOTOR_CHECK(!it->second.nombre.empty() && it->second.nombre.rfind("Rasgo de", 0) != 0);
        for (const char* dnd : {"Competencia", "Sabiduría", "Fuerza", "salvación", "1d4", "metros", "truco"})
            MOTOR_CHECK(it->second.texto.find(dnd) == std::string::npos);
    });
    MOTOR_CHECK(vistas == 72);  // DIALOGOS-01: + el drow de las sombras y la saga de los vientos; + 27 del 2000 b.f.
    const RPG::RaceDefinition* sombra = reglas().races().find("tieflings_de_la_sombra");
    MOTOR_CHECK(sombra != nullptr && sombra->name == "Poseídos de la Sombra");
    MOTOR_CHECK(rasgos().at("minotauros_de_la_llanura").nombre == "Carga feroz");
    MOTOR_CHECK(rasgos().at("enanos_forjadores").texto.find("veneno") != std::string::npos);  // el de todo Udrax
}

MOTOR_TEST_CASE(deidad_ninguna_es_una_eleccion_valida) {
    CuadernoCreacion c(datos());
    c.irA(Hoja::Deidad);
    MOTOR_CHECK(c.ids().front().empty());
    // DIOSES-01: solo los Creadores (las deidades locales no se ofrecen).
    std::size_t creadores = 0;
    reglas().deities().forEach([&](const RPG::DeityDefinition& d) { creadores += d.esCreador() ? 1 : 0; });
    MOTOR_CHECK(creadores == 13);
    MOTOR_CHECK(c.ids().size() == creadores + 1);
    MOTOR_CHECK(!c.hecha(Hoja::Deidad));
    c.acepta();  // el foco empieza en Ninguna
    MOTOR_CHECK(c.hecha(Hoja::Deidad));
    MOTOR_CHECK(c.eleccion().deityId.empty());
}

MOTOR_TEST_CASE(el_nombre_se_escribe_borra_y_se_sugiere) {
    CuadernoCreacion c(datos());
    c.irA(Hoja::Nombre);
    MOTOR_CHECK(c.escribiendo());
    c.escribe("Nerea");
    c.escribe("\n");  // el control no entra
    MOTOR_CHECK(c.eleccion().displayName == "Nerea");
    c.borra();
    MOTOR_CHECK(c.eleccion().displayName == "Nere");
    c.escribe("ñ");
    c.borra();  // borra la letra entera (dos bytes)
    MOTOR_CHECK(c.eleccion().displayName == "Nere");
    c.pulsa(Zona::Sugerencia, 2);
    MOTOR_CHECK(c.eleccion().displayName == CuadernoCreacion::sugerencias()[2]);
    c.mueve(1, 0);
    MOTOR_CHECK(c.eleccion().displayName == CuadernoCreacion::sugerencias()[3]);
    for (int i = 0; i < 40; ++i) c.escribe("x");
    MOTOR_CHECK(c.eleccion().displayName.size() <= CuadernoCreacion::kNombreMax);
}

MOTOR_TEST_CASE(sin_todo_elegido_empezar_dice_lo_que_falta) {
    CuadernoCreacion c(datos());
    c.irA(Hoja::Nombre);
    c.continuar();
    MOTOR_CHECK(c.estado() == CuadernoCreacion::Estado::Creando);
    MOTOR_CHECK(c.aviso() == "Elige tu raza.");
    MOTOR_CHECK(c.problemas().size() == 4);  // raza, clase, origen, nombre
}

MOTOR_TEST_CASE(con_todo_elegido_empieza_y_la_ficha_sale_sin_equipo) {
    CuadernoCreacion c(datos());
    hastaElNombre(c);
    c.escribe("Nerea");
    MOTOR_CHECK(c.problemas().empty());
    for (int h = 0; h < CuadernoCreacion::kHojas; ++h) {
        if (h == static_cast<int>(Hoja::Aspecto)) continue;  // no se visito
        MOTOR_CHECK(c.hecha(static_cast<Hoja>(h)));
    }
    c.pulsa(Zona::Continuar, 0);
    MOTOR_CHECK(c.estado() == CuadernoCreacion::Estado::Terminado);
    const auto ficha = c.finalizar();
    MOTOR_CHECK(ficha.isOk());
    if (!ficha.isOk()) return;
    const RPG::CharacterSheet& f = ficha.value();
    MOTOR_CHECK(f.displayName == "Nerea");
    MOTOR_CHECK(f.classId == "bardo_de_las_cuatro_estaciones");
    MOTOR_CHECK(f.inventoryEquipment.empty());  // el kit lo da el cofre
    for (const std::string& puesto : f.equippedIds) MOTOR_CHECK(puesto.empty());
    MOTOR_CHECK(f.healthCap() > 0);
}

MOTOR_TEST_CASE(aspecto_se_cambia_con_las_flechas_de_su_fila) {
    CuadernoCreacion c(datos());
    c.pulsa(Zona::Tesela, posicion(c, Hoja::Raza, "alquimistas_de_aegroum"));
    c.irA(Hoja::Clase);  // DIALOGOS-01: el aspecto, con la clase ya elegida
    c.pulsa(Zona::Tesela, posicion(c, Hoja::Clase, "alquimista"));
    MOTOR_CHECK(c.eleccion().classId == "alquimista");
    c.irA(Hoja::Aspecto);
    MOTOR_CHECK(c.editor() != nullptr);
    const std::string antes = c.editor()->apariencia().clave();
    int ojos = -1;
    for (std::size_t i = 0; i < c.editor()->filas().size(); ++i)
        // CAR-2: los alquimistas no tienen capa de ojos; la fila que se prueba es el peinado.
        if (c.editor()->filas()[i] == Game::EditorApariencia::Fila::Peinado) ojos = static_cast<int>(i);
    MOTOR_CHECK(ojos >= 0);
    c.pulsa(Zona::Mas, ojos);
    MOTOR_CHECK(c.foco() == ojos);
    MOTOR_CHECK(c.editor()->apariencia().clave() != antes);
    MOTOR_CHECK(c.hecha(Hoja::Aspecto));
    // El rasgo de la raza llega desde races.json.
    MOTOR_CHECK(!rasgos().at("alquimistas_de_aegroum").texto.empty());
}

// RAZAS-01 — La CULTURA de la hoja Raza, sin la ficha de la plantilla vieja
// (Edad, Tamaño y la frase de la religion): lo de antes y lo de despues. Una
// descripcion sin ella, tal cual.
MOTOR_TEST_CASE(cuaderno_la_cultura_de_un_pueblo_sin_la_ficha_vieja) {
    MOTOR_CHECK(CuadernoCreacion::culturaDe(
                    "Resistentes al frío extremo. Edad: Alcanzan la madurez a los 15 años. Tamaño: Mediano "
                    "(entre 1,50 m y 1,90 m) Religión/cultura: Chronos y Eros. Usan los vientos gélidos.") ==
                "Resistentes al frío extremo. Usan los vientos gélidos.");
    MOTOR_CHECK(CuadernoCreacion::culturaDe("Fuertes. Edad: 15. Religión/cultura: Se alinean con Egos y Vida.") ==
                "Fuertes.");
    MOTOR_CHECK(CuadernoCreacion::culturaDe("Los humanos de las Marcas.") == "Los humanos de las Marcas.");
}

}  // namespace

int main() { return motor_test::runAllTests(); }
