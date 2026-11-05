// PERGAMINO-01 — ver include/HUD/Pergamino/InterfazPergamino.h.
#include "HUD/Pergamino/InterfazPergamino.h"

#include <algorithm>
#include <cmath>

#include "Core/Math/IsoMath.h"
#include "Game/BarraCombate.h"
#include "Game/BattleState.h"
#include "Game/DiarioMisiones.h"
#include "Game/EnemyBrain.h"
#include "Game/GameOverScreen.h"
#include "Game/GameSession.h"
#include "Game/InventoryScreen.h"
#include "Game/Lugares.h"  // AV15-02
#include "Game/OptionsState.h"
#include "Game/PauseMenu.h"
#include "Game/TitleMenuState.h"
#include "HUD/Pergamino/MesaCombateHud.h"
#include "HUD/Pergamino/PaginasCreacion.h"
#include "HUD/Pergamino/Papiro.h"
#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/TiendaHoja.h"
#include "Level/ObjectCatalog.h"
#include "RPG/Narrative/NarrativeState.h"
#include "Runtime/RuntimeTileMap.h"
#include "World/World.h"

namespace Pergamino {

namespace {

// Donde pisa un personaje dentro del quad de su baldosa, en fraccion del
// alto: el centro de su sombra blob (Entity::renderShadow).
constexpr float kPies = 0.9f;

const char* textoPausa(Game::PauseAction a) {
    switch (a) {
        case Game::PauseAction::Resume: return "Reanudar";
        case Game::PauseAction::Character: return "Personaje";
        case Game::PauseAction::Inventory: return "Inventario";
        case Game::PauseAction::Save: return "Guardar partida";
        case Game::PauseAction::Load: return "Cargar partida";
        case Game::PauseAction::Options: return "Opciones";
        case Game::PauseAction::Title: return "Volver al título";
        case Game::PauseAction::Quit: return "Salir del juego";
        case Game::PauseAction::None: break;
    }
    return "";
}

std::string textoOpcion(const std::string& clave) {
    if (clave == "music") return "Música";
    if (clave == "sfx") return "Efectos";
    if (clave == "ambience") return "Ambiente";
    if (clave == "language") return "Idioma";
    if (clave == "controls") return "Controles";
    if (clave == "back") return "Volver";
    return clave;
}

// Idiomas con su nombre propio (MODELO_VISUAL.md §5, selector).
std::string nombreIdioma(const std::string& valor) {
    if (valor == "CATALA") return "Català";
    if (valor == "CASTELLA") return "Español";
    if (valor == "ENGLISH") return "English";
    return valor;
}

// El paso seguido: lo que toca de la mision seguida (MAPA-02: la que elige el jugador).
void objetivoSeguido(const Game::DiarioDatos* diario, DatosExploracion& d) {
    const Game::DiarioMision* m = diario != nullptr ? Game::misionSeguida(*diario) : nullptr;
    const Game::DiarioObjetivo* o = m != nullptr ? m->siguiente() : nullptr;
    if (o != nullptr) d.mision = m->nombre, d.paso = o->texto, d.hechos = m->hechos, d.total = m->total;
}

// El objeto del mundo de un enemigo del combate (para su retrato): la
// entidad cuyo cuerpo es el combatiente de la batalla.
std::string objetoDe(const Engine::World* mundo, const ICombatant* cuerpo) {
    std::string id;
    if (mundo == nullptr || cuerpo == nullptr) return id;
    mundo->eachEntity([&](Engine::EntityHandle, const Engine::WorldEntity& w) {
        if (id.empty() && w.kind == Engine::EntityKind::Enemy && w.enemy.brain != nullptr &&
            static_cast<const ICombatant*>(w.enemy.brain.get()) == cuerpo)
            id = w.enemy.objectId;
    });
    return id;
}

}  // namespace

AvisoInteraccion avisoInteraccion(const GameSession& s) {
    AvisoInteraccion a;
    const Engine::World* mundo = s.world();
    const ObjectCatalog* catalogo = s.objectCatalog();
    if (mundo == nullptr || s.mode() != GameMode::Exploration) return a;
    const GridCoord& p = s.playerPosition();
    auto definicion = [&](const Engine::WorldEntity* e) -> const ObjectDefinition* {
        return e != nullptr && catalogo != nullptr ? catalogo->find(e->object.objectId) : nullptr;
    };
    // 1) La puerta que se pisa.
    if (const Engine::WorldEntity* aqui = mundo->get(mundo->findObjectAt(p.x, p.y))) {
        if (!aqui->object.targetLevel.empty()) {
            const ObjectDefinition* def = definicion(aqui);
            // AV15-02 / AV16-06: «Entrar en La Boutique de Madame»; una salida, «Salir a la calle».
            const Game::TextoPuerta t = Game::Lugares::delJuego().textoDePuerta(
                aqui->object.objectId, aqui->object.targetLevel,
                def != nullptr ? def->name : aqui->object.objectId);
            a.verbo = t.verbo;
            a.destino = t.destino;
            a.celdaX = p.x;
            a.celdaY = p.y;
            a.enLaCelda = true;
            return a;
        }
    }
    // 2) La propia celda y las cuatro de al lado, en el orden de interact().
    const int dx[] = {0, 0, 0, -1, 1};
    const int dy[] = {0, -1, 1, 0, 0};
    for (int i = 0; i < 5; ++i) {
        const Engine::WorldEntity* e = mundo->get(mundo->findObjectAt(p.x + dx[i], p.y + dy[i]));
        const ObjectDefinition* def = definicion(e);
        if (def == nullptr) continue;
        if (def->category == ObjectCategory::Npc) {
            a.verbo = "Hablar con";
            a.destino = def->name;
        } else if (e->object.objectId == Game::CofreKit::kObjeto) {
            a.verbo = "Abrir";  // UI-BG3-01: el cofre de El Vacio (C04a)
            a.destino = "cofre";
        } else if (def->actuacion.rivalDice > 0) {
            a.verbo = "Actuar";  // ACTUAR-01: «[E] Actuar junto a la fuente»
            a.destino = def->actuacion.lugar;
        } else if (def->business.price > 0) {
            a.verbo = "Ver";
            a.destino = def->business.businessName.empty() ? def->name : def->business.businessName;
        } else {
            continue;
        }
        a.celdaX = p.x + dx[i];
        a.celdaY = p.y + dy[i];
        return a;
    }
    return a;
}

Result<bool> InterfazPergamino::carga(const std::string& carpeta) {
    auto letras = m_letras.carga(carpeta + "/textures/letras_pergamino.json");
    if (!letras.isOk()) return letras;
    auto iconos = m_iconos.cargar(carpeta + "/textures/iconos.json");
    if (!iconos.isOk()) return iconos;
    // Sin nombres de historia la ficha sale con los ids legibles: no es fatal.
    (void)m_nombres.carga(carpeta + "/catalogs/backgrounds.json");
    return Result<bool>::Ok(true);
}

void InterfazPergamino::ponZona(const std::string& nombre, const std::string& claveNivel) {
    const Game::Lugar* lugar = Game::Lugares::delJuego().de(claveNivel);  // AV15-02: un sitio con nombre
    m_zona = lugar != nullptr ? lugar->nombre : nombre;
    if (lugar != nullptr && claveNivel != m_claveNivel) m_retales.anunciaLugar(lugar->nombre, lugar->funcion, lugar->icono());
    m_claveNivel = claveNivel;
    m_guia.ponNivel(claveNivel);  // MAPA-02: las chinchetas de este nivel
}

void InterfazPergamino::avisaPausa(bool ok, const std::string& texto, const std::string& detalle) {
    m_avisoPausa = AvisoPausa{texto, detalle, !ok};
}

const Zona* InterfazPergamino::zonaEn(float x, float y) const {
    if (const Zona* z = m_capas.alta.zonaEn(x, y)) return z;
    return m_capas.baja.zonaEn(x, y);
}

void InterfazPergamino::avanza(float segundos) {
    m_retales.avanza(segundos);
    m_cuaderno.avanza(segundos);
    m_guia.avanza(segundos);
}

void InterfazPergamino::actualiza(const EstadoJuego& e) {
    m_salidas.clear();  // AV16-06: las vuelve a poner pintaExploracion
    m_capas.baja.limpia();
    m_capas.alta.limpia(); m_capas.hoja.limpia();
    m_capas.hojaVisible = false;
    m_retratos.nuevoFotograma();  // PERF-2: el cupo de montajes de retratos, por cada actualiza
    if ((e.titulo != nullptr || e.creacion != nullptr) && cargada()) {
        m_cuadernoAbierto = false;
        m_dialogo.cierra();
        m_marca = MarcaSuelo{};
        if (e.creacion != nullptr) pintaCreacion(e);
        else pintaTitulo(e);
        return;
    }
    if (e.oculta || e.sesion == nullptr || !cargada()) {
        m_cuadernoAbierto = false;
        m_dialogo.cierra();
        m_marca = MarcaSuelo{};
        return;
    }
    const GameMode modo = e.sesion->mode();
    if (modo != GameMode::Dialogue) {
        m_dialogo.cierra();
        m_marca = MarcaSuelo{};  // la vuelve a poner pintaExploracion
    } else if (m_marca.destino != e.sesion->dialogueSpeaker()) {
        m_marca = MarcaSuelo{};  // habla otro (un beat de la historia)
    }
    const bool opciones = modo == GameMode::Pause && e.opciones != nullptr;
    if (modo != GameMode::Pause || opciones != m_opcionesAbiertas) m_avisoPausa = AvisoPausa{};
    m_opcionesAbiertas = opciones;
    const bool cuaderno = modo == GameMode::Exploration && e.cuadernoAbierto && e.cuaderno != nullptr;
    if (modo != GameMode::Exploration || cuaderno) m_guia.cierra();  // MAPA-02: el mapa grande, solo explorando
    if (e.diario != nullptr) m_guia.miraDiario(*e.diario);           // MAPA-02: para seguir desde la hoja Misiones
    // H2-01: la ultima tirada se mira en todos los modos, para que una de
    // fuera del dialogo no se pegue al siguiente que se abra.
    if (modo == GameMode::Dialogue) m_dialogo.mira(e.sesion->dialogueSpeaker(), e.sesion->dialogueLines(), m_letras);
    if (const RPG::NarrativeState* narrativa = e.sesion->narrativeState())
        m_dialogo.miraNarrativa(*narrativa);  // H2-01 (la tirada) y OPCIONES-01 (lo que se contesta)
    switch (modo) {
        case GameMode::Exploration:
            if (cuaderno) {
                pintaCuaderno(e);
            } else {
                pintaExploracion(e, false);
                if (e.cofre != nullptr) pintaCofre(e);
                else if (m_guia.abierto()) m_guia.pintaMapa(m_capas.alta, Pincel(m_letras, m_iconos), *e.sesion, e.diario, m_zona, e.anchoVista, e.altoVista);
            }
            break;
        case GameMode::Dialogue: {
            pintaExploracion(e, true);
            const Pincel p(m_letras, m_iconos);
            // TIENDA-02: al cerrar, lo que dejo montado (GameSession::closeInteraction).
            const char* verbo = e.sesion->battle() != nullptr       ? "Pelear"
                                : !e.sesion->shopOffers().empty() ? "Ver la tienda"
                                                                  : "Cerrar";
            m_dialogo.pinta(m_capas.baja, p, e.anchoVista, e.altoVista, verbo);
            break;
        }
        case GameMode::Shop:
            pintaExploracion(e, true);
            pintaTiendaAbierta(e);
            break;
        case GameMode::Business:  // AV15-05a: la hoja del negocio y su pregunta, en papiro
            pintaExploracion(e, true);
            if (e.negocio != nullptr) pintaNegocio(m_capas.alta, Pincel(m_letras, m_iconos), *e.sesion, *e.negocio, e.anchoVista, e.altoVista); break;
        case GameMode::Battle:
            pintaExploracion(e, true);
            pintaCombate(e);
            break;
        case GameMode::Pause:
            pintaPausa(e);
            break;
        case GameMode::GameOver:
            pintaDerrota(e);
            break;
    }
    m_cuadernoAbierto = cuaderno;
}

void InterfazPergamino::pintaExploracion(const EstadoJuego& e, bool compacto) {
    const Pincel p(m_letras, m_iconos);
    DatosExploracion d;
    d.compacto = compacto;
    const FuentesCuaderno fc{e.sesion, e.cuaderno, e.diario, e.codice, &m_nombres, e.cicatrices};
    const CabeceraCuaderno cab = cabeceraCuaderno(fc);
    d.nombre = cab.nombre, d.tier = cab.tier;
    d.golpe = e.golpeHoja, d.pvFantasma = e.pvFantasma, d.temblor = Vector2{e.temblorHojaX, e.temblorHojaY};  // AV14-02
    if (compacto && e.vistaBarra != nullptr && e.sesion->mode() == GameMode::Battle)
        d.insignia = e.vistaBarra->elegida;  // la estacion del Bardo (C03a)
    d.oro = e.sesion->gold();
    if (const ICombatant* cuerpo = e.sesion->playerBody()) {
        d.pv = cuerpo->health();
        d.pvMax = cuerpo->maxHealth();
    }
    if (!compacto) {
        objetivoSeguido(e.diario, d);
        m_retales.observaObjetivo(d.paso);
        // H3-06: los temporizadores de mision que se ven (el reloj lo lleva
        // la sesion y solo corre explorando: en combate y en un dialogo se
        // queda quieto, y aqui ni se pinta).
        if (e.timers != nullptr && e.sesion->narrativeState() != nullptr)
            d.relojes = Game::relojesVisibles(e.sesion->narrativeState()->timers(), *e.timers);
        const AvisoInteraccion a = e.sinAviso ? AvisoInteraccion{} : avisoInteraccion(*e.sesion);
        if (!a.destino.empty() && e.mapa != nullptr) {
            const float tw = static_cast<float>(e.mapa->getTileWidth() > 0 ? e.mapa->getTileWidth() : 64);
            const float th = static_cast<float>(e.mapa->getTileHeight() > 0 ? e.mapa->getTileHeight() : 32);
            // Los pies de quien esta en esa celda: donde apoya su sombra blob
            // (Entity::renderShadow). El aviso flota sobre su cabeza (P03a).
            const Vector2 celda = IsoMath::gridToScreen(GridCoord{a.celdaX, a.celdaY}, tw, th);
            const Vector2 pies{(celda.x + tw * 0.5f - e.camaraX) * e.zoom + static_cast<float>(e.anchoVista) * 0.5f,
                               (celda.y + th * kPies - e.camaraY) * e.zoom + static_cast<float>(e.altoVista) * 0.5f};
            d.avisoVerbo = a.verbo;
            d.avisoDestino = a.destino;
            if (!a.enLaCelda) m_marca = MarcaSuelo{true, a.celdaX, a.celdaY, tw, th, kPies, a.destino};
            d.avisoAncla = Vector2{pies.x, pies.y - 54.0f * e.zoom};
        }
        m_guia.minimapa().mira(e.mapa, m_claveNivel);
        m_guia.minimapa().rellena(d, *e.sesion, e.diario, &m_guia.chinchetasAqui());  // MAPA-01/02: objetivo y chinchetas
        d.zona = m_zona;
    }
    const std::size_t desdeHud = m_capas.baja.cuantos();  // MAPA-02: la flecha no pisa carteles ni retales
    if (!compacto) m_salidas = salidasCercanas(e);  // AV16-06: la flecha de suelo de cada salida
    if (!compacto) pintaCarteles(m_capas.baja, p, e, d.avisoDestino);  // AV15-02: el nombre sobre cada puerta
    if (!compacto) pintaBocadillosPnj(m_capas.baja, p, e, d.avisoDestino);  // PNJ-02: el oficio y las frases de los PNJ
    if (!compacto) pintaEmociones(m_capas.baja, p, e);  // C2-04d: "!" y compania de las cinematicas
    m_retales.pinta(m_capas.baja, p, d, e.anchoVista, e.altoVista);
    const GuiaMapa::Vista vista{e.anchoVista, e.altoVista, e.camaraX, e.camaraY, e.zoom, e.mapa ? e.mapa->getTileWidth() : 0,
                                e.mapa ? e.mapa->getTileHeight() : 0, e.animacionesReducidas};
    if (!compacto && !m_guia.abierto()) m_guia.pintaFlecha(m_capas.baja, p, *e.sesion, e.diario, vista, desdeHud);  // MAPA-02
}

void InterfazPergamino::pintaTiendaAbierta(const EstadoJuego& e) {
    if (e.tienda == nullptr) return;
    const Pincel p(m_letras, m_iconos);
    DatosTienda d = datosTienda(*e.tienda, *e.sesion);
    // AV15-02: la tienda dice dónde estás («Lingote de Oro · Aigren»).
    if (const Game::Lugar* lugar = Game::Lugares::delJuego().de(m_claveNivel); lugar != nullptr)
        d.tendero = d.tendero.empty() ? lugar->nombre : lugar->nombre + " · " + d.tendero;
    pintaTienda(m_capas.alta, p, d, e.anchoVista, e.altoVista);
}

void InterfazPergamino::pintaCuaderno(const EstadoJuego& e) {
    const Pincel p(m_letras, m_iconos);
    const FuentesCuaderno fc{e.sesion, e.cuaderno, e.diario, e.codice, &m_nombres, e.cicatrices};
    const PaginaCuaderno pagina = paginaActual(fc);
    const int foco = static_cast<int>(e.cuaderno->focus());
    if (!m_cuadernoAbierto) m_cuaderno.abre(foco);
    else m_cuaderno.mira(foco, e.animacionesReducidas);
    m_cuaderno.pintaHoja(m_capas.hoja, p, pagina, foco + 1);
    m_capas.hojaVisible = true;
    m_capas.rectHoja = Cuaderno::hoja(e.anchoVista, e.altoVista);
    m_cuaderno.pintaMarco(m_capas.alta, p, cabeceraCuaderno(fc), pagina, e.anchoVista, e.altoVista);
}

void InterfazPergamino::pintaPausa(const EstadoJuego& e) {
    const Pincel p(m_letras, m_iconos);
    DatosPapiro d;
    if (e.opciones != nullptr) {
        d.titulo = "OPCIONES";
        d.ancho = 560.0f;  // el pie lleva tres ayudas: ← → Ajustar, ENTER y ESC
        for (const Game::OptionsEntry& o : e.opciones->entries()) {
            EntradaPapiro fila;
            fila.texto = textoOpcion(o.key);
            if (o.key == "music" || o.key == "sfx" || o.key == "ambience") {
                fila.deslizador = std::clamp(e.opciones->volume(o.key), 0, 10);
            } else if (o.key == "language") {
                fila.valor = nombreIdioma(o.value);
            }
            fila.separadorAntes = o.key == "back";
            d.entradas.push_back(fila);
        }
        d.foco = static_cast<int>(e.opciones->selected());
        d.pie = {{{"←", "→"}, "Ajustar"}, {{"ENTER"}, "Aceptar"}, {{"ESC"}, "Volver"}};
    } else if (e.pausa != nullptr) {
        d.titulo = e.pausa->titulo(), d.subtitulo = e.pausa->subtitulo();  // AV15-05a: o la pregunta
        for (const Game::PauseEntry& entrada : e.pausa->entries()) {
            EntradaPapiro fila;
            fila.texto = textoPausa(entrada.action);
            if (fila.texto.empty()) fila.texto = entrada.label;
            fila.separadorAntes = entrada.action == Game::PauseAction::Title;
            d.entradas.push_back(fila);
        }
        d.foco = static_cast<int>(e.pausa->selected()), d.ancho = e.pausa->eligiendoPartida() ? 720.0f : d.ancho;  // D0034: fecha y lugar en la fila
        d.pie = {{{"ENTER"}, "Aceptar"}, {{"ESC"}, e.pausa->preguntando() ? "Volver" : "Reanudar"}};
    } else {
        return;
    }
    d.estado = m_avisoPausa.texto;
    d.estadoDetalle = m_avisoPausa.detalle;
    d.estadoError = m_avisoPausa.error;
    pintaPapiro(m_capas.alta, p, d, e.anchoVista, e.altoVista);
}

// UI-BG3-01 — Maqueta C01: el nombre del juego en pixel grande sobre la
// ciudad (sin velo) y el menu en un papiro estrecho debajo. «Continuar»
// apagado lleva su motivo; el pie dice que tambien vale el raton.
void InterfazPergamino::pintaTitulo(const EstadoJuego& e) {
    const Pincel p(m_letras, m_iconos);
    Lienzo& l = m_capas.alta;
    const float ancho = static_cast<float>(e.anchoVista);
    const Vector4 crema = Color::kClaroCuero;
    const Vector4 sombra = rgb(20, 12, 6, 0.75f);
    // «Los Perdidos de Boundington»: la segunda linea empieza en « de ».
    std::string nombre = e.titulo->title(), segunda;
    const std::size_t corte = nombre.find(" de ");
    if (corte != std::string::npos && corte > 0) {
        segunda = nombre.substr(corte + 1);
        nombre = nombre.substr(0, corte);
    }
    auto rotulo = [&](const std::string& t, float y, int escala) {
        const float w = p.anchoPixel(t, escala);
        const float x = std::round((ancho - w) * 0.5f);
        p.pixel(l, t, x + 4.0f, y + 4.0f, escala, sombra);
        p.pixel(l, t, x, y, escala, crema);
    };
    float y = 48.0f;
    rotulo(nombre, y, 6);
    y += 9.0f * 6.0f + 4.0f;
    if (!segunda.empty()) {
        rotulo(segunda, y, 4);
        y += 9.0f * 4.0f + 8.0f;
    }
    const std::string& sub = e.titulo->subtitle();
    if (!sub.empty() && sub.size() <= 60) {
        const float ws = static_cast<float>(m_letras.ancho(Letra::Lora18i, sub));
        p.texto(l, Letra::Lora18i, sub, std::round((ancho - ws) * 0.5f) + 2.0f, y + 2.0f, 26.0f, sombra);
        p.texto(l, Letra::Lora18i, sub, std::round((ancho - ws) * 0.5f), y, 26.0f, crema);
    }
    DatosPapiro d;
    d.ancho = 420.0f;
    d.arriba = 236.0f;
    d.velo = false;
    for (const Game::TitleMenuItem& item : e.titulo->items()) {
        EntradaPapiro fila;
        fila.texto = item.label;
        fila.apagada = !item.enabled;
        fila.motivo = item.enabled ? std::string() : item.reason;
        d.entradas.push_back(fila);
    }
    d.foco = static_cast<int>(e.titulo->selected());
    d.pie = {{{"W", "S"}, "Elegir"}, {{"ENTER"}, "Aceptar"}, {{"ESC"}, "Salir"}};
    d.notaPie = "o haz clic con el ratón";
    d.estado = m_avisoPausa.texto;
    d.estadoDetalle = m_avisoPausa.detalle;
    d.estadoError = m_avisoPausa.error;
    pintaPapiro(l, p, d, e.anchoVista, e.altoVista);
    p.texto(l, Letra::Lora16, "MotorGraphico", 24.0f, static_cast<float>(e.altoVista) - 34.0f, 22.0f, crema);
}

void InterfazPergamino::pintaCreacion(const EstadoJuego& e) {
    const Pincel p(m_letras, m_iconos);
    Retratista retratos(m_retratos, e.montaje);
    Pergamino::pintaCreacion(m_capas.alta, p, *e.creacion, retratos, e.anchoVista, e.altoVista);
}

// UI-BG3-01 — Maqueta C04b: lo que trae el cofre, guardado en la mochila, y
// las dos salidas: ponerselo (abre el cuaderno) o cerrar.
void InterfazPergamino::pintaCofre(const EstadoJuego& e) {
    const Pincel p(m_letras, m_iconos);
    DatosPapiro d;
    d.ancho = 520.0f;
    d.titulo = e.cofre->contenedor.empty() ? "COFRE" : "BOTIN";
    d.subtitulo = !e.cofre->contenedor.empty() ? e.cofre->clase : e.cofre->clase.empty() ? std::string("Lo que traías contigo")
                                         : "El kit básico de " + e.cofre->clase;
    if (m_letras.ancho(Letra::Lora18, d.subtitulo) > 440) d.subtitulo = e.cofre->contenedor.empty() ? "El kit básico de tu clase" : "Contenedor";
    // Con mas de tres piezas (el kit de +2, +2, +1, +1 lleva hasta cinco) no
    // caben dos lineas por pieza: la ranura va donde iria el bono si no lo
    // tiene, y lo de «sin requisitos» (lo es todo el kit) se calla.
    const bool compacto = e.cofre->piezas.size() > 3;
    for (const Game::CofreKit::Pieza& pieza : e.cofre->piezas) {
        EntradaPapiro fila;
        fila.texto = pieza.nombre;
        fila.valor = compacto && pieza.bono.empty() ? pieza.ranura : pieza.bono;
        if (!compacto) fila.motivo = pieza.ranura + (e.cofre->contenedor.empty() ? " · sin requisitos" : "");
        d.entradas.push_back(fila);
    }
    if (d.entradas.empty()) d.entradas.push_back(EntradaPapiro{"Está vacío", false, "", -1, true, ""});
    EntradaPapiro ponermelo{e.cofre->accion, true};
    d.entradas.push_back(ponermelo);
    d.entradas.push_back(EntradaPapiro{"Cerrar"});
    d.foco = static_cast<int>(e.cofre->piezas.size()) + e.cofre->foco;
    d.estado = e.cofre->contenedor.empty() ? "Guardado en tu mochila" : "Contenido del contenedor abierto";
    d.estadoDetalle = e.cofre->contenedor.empty() ? "Póntelo con TAB, en la hoja Inventario." : "Lo que dejes se conserva al cerrar.";
    d.pie = {{{"ENTER"}, "Aceptar"}, {{"ESC"}, "Cerrar"}};
    pintaPapiro(m_capas.alta, p, d, e.anchoVista, e.altoVista);
}

// UI-BG3-01 — Maquetas C03a-C03c: la barra de cartas abajo, la carta grande,
// el turno, el orden de turno, la placa de cada enemigo con sus estados y el
// registro. Lo que hay en la mesa lo da la batalla; el estado de la pantalla,
// la barra que lleva Application.
void InterfazPergamino::pintaCombate(const EstadoJuego& e) {
    const BattleState* batalla = e.sesion->battle();
    if (batalla == nullptr || e.vistaBarra == nullptr || e.barra == nullptr) return;
    DatosCombate d;
    d.vista = e.vistaBarra, d.barra = e.barra;
    d.orden = batalla->orden().puestos(), d.enTurno = batalla->orden().actual();  // AV13-11
    d.heroe = batalla->allies().empty() ? std::string() : batalla->allies().front().name;
    d.retratoHeroe = e.retratoHeroe;
    d.sprites = e.sprites;
    d.registro = batalla->log();
    d.aliados = aliadosEnMesa(*batalla);  // ACOMPANANTES-01
    for (std::size_t i = 0; i < batalla->enemies().size(); ++i) {
        const BattleParticipant& b = batalla->enemies()[i];
        EnemigoEnMesa en;
        en.nombre = b.name;
        if (b.combatant != nullptr) {
            en.pv = b.combatant->health();
            en.pvMax = b.combatant->maxHealth();
            en.vivo = b.combatant->isAlive();
        }
        en.objectId = objetoDe(e.sesion->world(), b.combatant);
        d.enemigos.push_back(std::move(en));
    }
    const Pincel p(m_letras, m_iconos);
    Retratista retratos(m_retratos, e.montaje);
    Pergamino::pintaCombate(m_capas.baja, p, d, retratos, e.anchoVista, e.altoVista);
}

// pintaDerrota, en InterfazDerrota.cpp (AV13-33: con la escena de noqueo antes).

}  // namespace Pergamino
