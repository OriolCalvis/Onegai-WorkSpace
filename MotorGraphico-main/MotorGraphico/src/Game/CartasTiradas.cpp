// H2-06 — Las tiradas de la mesa, los golpes, el dano y las reacciones, y el
// turno de los enemigos contra todo eso. Toda tirada pasa por SkillExecutor
// (Ed. 3, enfrentada) con las ventajas, desventajas y dados de mas que dicen
// los estados de los dos lados.
#include "CartasEnCombateInterno.h"

#include <algorithm>
#include <cstdio>
#include <utility>

#include "Game/FrasesDeCombate.h"  // EH-16
#include "Game/Skill.h"
#include "RPG/CharacterSheet.h"
#include "RPG/ConditionEngine.h"
#include "RPG/Definitions/SkillDefinition.h"
#include "RPG/EnhanceHooks.h"

namespace Game {

using T = Gancho::Tipo;
using namespace cartas;

namespace {

bool hay(const std::vector<RasgoActivo>& rasgos, T tipo) {
    return std::any_of(rasgos.begin(), rasgos.end(), [tipo](const RasgoActivo& r) { return r.g.tipo == tipo; });
}

int suma(const std::vector<RasgoActivo>& rasgos, T tipo) {
    int total = 0;
    for (const RasgoActivo& r : rasgos) {
        if (r.g.tipo == tipo) total += r.g.numero;
    }
    return total;
}

// ¿Lo puso este? (rasgos con origen: marca, provocacion)
bool puestoPor(const RasgoActivo& r, bool aliado, std::size_t i) {
    return r.conOrigen && r.origenAliado == aliado && r.origen == i;
}

// Ventaja y desventaja a la vez se anulan.
RPG::DiceMod mod(bool ventaja, bool desventaja) {
    if (ventaja == desventaja) return RPG::DiceMod::NORMAL;
    return ventaja ? RPG::DiceMod::ADVANTAGE : RPG::DiceMod::DISADVANTAGE;
}

// H2-01 — Lo que se enseña de la carta: su nombre (el ataque de siempre no
// lo trae).
std::string nombreDeTirada(const RPG::SkillDefinition& def) {
    if (def.id == "atac_basic") return "Ataque";
    return def.name.empty() ? def.id : def.name;
}

std::string exitos(float n) {
    char buf[16];
    std::snprintf(buf, sizeof buf, n == static_cast<int>(n) ? "%.0f" : "%.1f", static_cast<double>(n));
    std::string s = buf;
    std::replace(s.begin(), s.end(), '.', ',');
    return s;
}

}  // namespace

ICombatant* CartasEnCombate::combatiente(const ContextoBatalla& c, Ref quien) const {
    const auto& lado = quien.lado == Lado::Aliado ? c.aliados : c.enemigos;
    return quien.i < lado.size() ? lado[quien.i].combatant : nullptr;
}

const std::string& CartasEnCombate::nombre(const ContextoBatalla& c, Ref quien) const {
    static const std::string nadie = "?";
    const auto& lado = quien.lado == Lado::Aliado ? c.aliados : c.enemigos;
    return quien.i < lado.size() ? lado[quien.i].name : nadie;
}

bool CartasEnCombate::vivo(const ContextoBatalla& c, Ref quien) const {
    const ICombatant* x = combatiente(c, quien);
    return x != nullptr && x->isAlive();
}

SituacionCombate& CartasEnCombate::situ(Ref quien) {
    auto& lado = quien.lado == Lado::Aliado ? m_aliados : m_enemigos;
    if (lado.size() <= quien.i) lado.resize(quien.i + 1);
    return lado[quien.i];
}

int CartasEnCombate::primerVivo(const ContextoBatalla& c, Lado lado) const {
    const std::size_t n = lado == Lado::Aliado ? c.aliados.size() : c.enemigos.size();
    for (std::size_t i = 0; i < n; ++i) {
        if (vivo(c, {lado, i})) return static_cast<int>(i);
    }
    return -1;
}

// Todo lo que pesa ahora sobre "quien": sus estados de la mesa, las
// condiciones de su ficha (si tiene) y, si es el heroe, su pasiva elegida.
std::vector<RasgoActivo> CartasEnCombate::rasgosDe(const ContextoBatalla& c, Ref quien) const {
    std::vector<RasgoActivo> rasgos;
    const auto& lado = quien.lado == Lado::Aliado ? m_aliados : m_enemigos;
    if (quien.i < lado.size()) {
        for (const EstadoCarta& e : lado[quien.i].estados) {
            for (const Gancho& g : e.rasgos) rasgos.push_back({g, true, e.origenAliado, e.origen});
        }
    }
    if (quien.lado == Lado::Aliado && quien.i == 0) rasgosDePasiva(rasgos);
    if (const ICombatant* x = combatiente(c, quien)) {
        if (const RPG::CharacterSheet* ficha = x->character_sheet()) {
            for (const auto& activa : ficha->conditions) {
                for (const Gancho& g : m_catalogo.ganchosDeCondicion(activa.conditionId)) {
                    if (g.esRasgo() && g.tipo != T::Ignorado) rasgos.push_back({g, false, true, 0});
                }
            }
        }
    }
    return rasgos;
}

bool CartasEnCombate::tiene(const ContextoBatalla& c, Ref quien, T tipo) const {
    return hay(rasgosDe(c, quien), tipo);
}

void CartasEnCombate::gastarPreparada(ContextoBatalla& c) {
    if (m_turno.preparada < 0) return;
    CartaEnMano& cm = m_mano[static_cast<std::size_t>(m_turno.preparada)];
    cm.gastada = true;
    cm.ordenGasto = ++m_turno.gastos;
    m_turno.preparada = -1;
    m_turno.reaccionUsada = true;
    (void)c;
}

// Egida Divina: «+2d6 a la tirada de un aliado». Salta en la proxima tirada
// del aliado protegido, ataque o defensa.
int CartasEnCombate::dadosExtraDe(ContextoBatalla& c, Ref quien) {
    if (m_turno.preparada < 0 || m_turno.reaccionUsada || quien.lado != Lado::Aliado ||
        quien.i != m_turno.preparadaPara) {
        return 0;
    }
    const CatalogoCartas::Carta& carta = *m_mano[static_cast<std::size_t>(m_turno.preparada)].carta;
    for (const Gancho& g : carta.ganchos) {
        if (g.tipo != T::DadosExtra) continue;
        c.log.push_back(carta.def->name + ": +" + std::to_string(g.numero) + " dados para " + nombre(c, quien) + ".");
        gastarPreparada(c);
        return g.numero;
    }
    return 0;
}

Tirada CartasEnCombate::tirar(ContextoBatalla& c, Ref atacante, Ref defensor,
                              const RPG::SkillDefinition& def, bool fisico, bool ventaja) {
    const ICombatant& atk = *combatiente(c, atacante);
    const ICombatant& dfn = *combatiente(c, defensor);
    const std::vector<RasgoActivo> ra = rasgosDe(c, atacante);
    const std::vector<RasgoActivo> rd = rasgosDe(c, defensor);
    bool contraOrigen = false;
    for (const RasgoActivo& r : rd) {
        contraOrigen |= r.g.tipo == T::VentajaContraOrigen &&
                        puestoPor(r, atacante.lado == Lado::Aliado, atacante.i);
    }

    RPG::Stat stat = RPG::Stat::DES;
    switch (def.save_attribute) {
        case RPG::SaveAttribute::CON: stat = RPG::Stat::CON; break;
        case RPG::SaveAttribute::INT: stat = RPG::Stat::INT; break;
        case RPG::SaveAttribute::CAR: stat = RPG::Stat::CAR; break;
        default: stat = dfn.stat(RPG::Stat::CON) > dfn.stat(RPG::Stat::DES) ? RPG::Stat::CON : RPG::Stat::DES;
    }
    const bool defensaFisica = stat == RPG::Stat::CON || stat == RPG::Stat::DES;

    RPG::ExecutionContext ctx;
    ctx.skill = &def;
    ctx.rng = &c.rng;
    ctx.override_base_dice =
        std::max(0, 1 + atk.stat(def.casting_stat) + dadosExtraDe(c, atacante) - suma(ra, T::PenalDados));
    ctx.force_dice_mod = mod(ventaja || hay(ra, T::VentajaAtaque) || hay(rd, T::VentajaContraTodo) ||
                                 (fisico && hay(rd, T::VentajaContraMelee)) || contraOrigen,
                             hay(ra, T::DesventajaTodo) || hay(ra, T::DesventajaAtaque));
    ctx.override_target_dice = std::max(0, dfn.stat(stat) + (fisico ? suma(rd, T::BonoCa) : 0) +
                                               dadosExtraDe(c, defensor) - suma(rd, T::PenalDados));
    ctx.defender_dice_mod =
        mod(hay(rd, T::VentajaDefensa), hay(rd, T::DesventajaTodo) || hay(rd, T::DesventajaDefensa) ||
                                            (defensaFisica && hay(rd, T::DesventajaDefensaFisica)));
    const RPG::SkillExecutor::Out out = RPG::SkillExecutor::execute(ctx);
    Tirada t;
    t.grado = out.final_degree;
    t.exitos.atacante = out.effective_successes;
    t.exitos.defensor = out.defender_pool.successes;
    t.exitos.pifiaAtacante = out.pool.botch;
    t.exitos.pifiaDefensor = out.defender_pool.botch;
    t.exitos.todosSeis = out.pool.critical;
    if (t.grado >= RPG::Degree::SUCCESS && hay(rd, T::CriticoContra)) {
        t.grado = RPG::Degree::CRITICAL;
        t.exitos.todosSeis = true;  // «los golpes contra el son criticos»: +1d6
    }
    // DANO-02: el empate ya no hace la mitad, no entra.
    t.resumen = frases::enfrentada(t.grado, out.effective_successes, out.defender_pool.successes, out.pool.botch);  // EH-16
    // H2-01 — La foto para el HUD: los dados de los dos y el grado final.
    RPG::TiradaVisible vista;  // numero y accion los pone el historial
    vista.que = nombreDeTirada(def);
    vista.quien = nombre(c, atacante);
    vista.contra = nombre(c, defensor);
    vista.delJugador = atacante.lado == Lado::Aliado;
    vista.tirada = out.pool;
    vista.bono = out.effective_successes - out.pool.successes;
    vista.rival = out.defender_pool;
    vista.enfrentada = out.opposed;
    vista.grado = t.grado;
    m_tiradas.apunta(std::move(vista));
    return t;
}

Tirada CartasEnCombate::tirarSinRival(ContextoBatalla& c, Ref quien, const RPG::SkillDefinition& def) {
    const std::vector<RasgoActivo> r = rasgosDe(c, quien);
    RPG::ExecutionContext ctx;
    ctx.skill = &def;
    ctx.rng = &c.rng;
    ctx.override_base_dice = std::max(0, 1 + combatiente(c, quien)->stat(def.casting_stat) +
                                             dadosExtraDe(c, quien) - suma(r, T::PenalDados));
    ctx.force_dice_mod = mod(hay(r, T::VentajaAtaque), hay(r, T::DesventajaTodo) || hay(r, T::DesventajaAtaque));
    const RPG::SkillExecutor::Out out = RPG::SkillExecutor::execute(ctx);
    Tirada t;
    t.grado = out.final_degree;
    // Sin rival nadie se defiende: cuenta como si el defensor pifiara (todos
    // los exitos son dados).
    t.exitos.atacante = out.effective_successes;
    t.exitos.pifiaAtacante = out.pool.botch;
    t.exitos.pifiaDefensor = true;
    t.exitos.todosSeis = out.pool.critical;
    t.resumen = frases::sinRival(t.grado, out.effective_successes, out.pool.botch);
    RPG::TiradaVisible vista;  // H2-01, sin rival
    vista.que = nombreDeTirada(def);
    vista.quien = nombre(c, quien);
    vista.delJugador = quien.lado == Lado::Aliado;
    vista.tirada = out.pool;
    vista.bono = out.effective_successes - out.pool.successes;
    vista.grado = t.grado;
    m_tiradas.apunta(std::move(vista));
    return t;
}

// Golpe de arma (Atacar, las cartas de ataque y las invocaciones). Devuelve
// la vida que de verdad quita (Furia: Fe se cura eso).
int CartasEnCombate::golpeDeArma(ContextoBatalla& c, Ref atacante, Ref objetivo, const Gancho* variante) {
    if (!vivo(c, atacante) || !vivo(c, objetivo)) return 0;
    if (situ(objetivo).protector >= 0) objetivo = {Lado::Aliado, static_cast<std::size_t>(situ(objetivo).protector)};
    if (tiene(c, objetivo, T::BloqueaTodos)) {
        c.log.push_back(nombre(c, objetivo) + " contrarresta el golpe de " + nombre(c, atacante) + ".");
        return 0;
    }
    static const RPG::SkillDefinition basico = definicionAtaqueBasico();
    const std::string modo = variante != nullptr ? variante->texto : std::string();
    Tirada t;
    if (modo == "critico" || modo == "exito") {
        // «Tu aliado no tira dados» / «tu ataque es critico»: el objetivo no
        // se defiende y el atacante tira solo; critico, +1d6.
        t = tirarSinRival(c, atacante, basico);
        t.exitos.pifiaAtacante = false;
        if (modo == "critico") {
            t.grado = RPG::Degree::CRITICAL;
            t.exitos.todosSeis = true;
        } else if (t.grado < RPG::Degree::SUCCESS) {
            t.grado = RPG::Degree::SUCCESS;
        }
        t.resumen = frases::sinDefensa(t.grado);
    } else {
        t = tirar(c, atacante, objetivo, basico, true, modo == "ventaja");
    }
    const int dados = dadosDeGolpe(t);
    Dados d6;
    d6.cantidad = dados;
    int dano = d6.tirar(c.rng);
    const bool roce = dados == 0 && RPG::dano::roza(t.exitos);  // D0014: el empate roza
    if (roce) dano = RPG::dano::kRoce;
    if (dano > 0 && !roce) {
        for (const RasgoActivo& r : rasgosDe(c, atacante)) {
            if (r.g.tipo == T::BonoDanoArma || r.g.tipo == T::BonoDanoMelee) dano += r.g.dados.tirar(c.rng);
        }
        dano += bonoDanoContra(c, atacante, objetivo);
        if (variante != nullptr && variante->numero > 1) dano *= variante->numero;
    }
    c.log.push_back(nombre(c, atacante) + " ataca a " + nombre(c, objetivo) + ": " + t.resumen + ". " +
                    frases::dano(dano, dados > 0 ? std::to_string(dados) + "d6" : roce ? "solo roza" : "") + ".");
    const int perdido = aplicarDano(c, atacante, objetivo, dano, true);
    contraPorPifia(c, t.exitos.pifiaAtacante, atacante, objetivo);  // AV13-30
    SituacionCombate& yo = situ(atacante);
    if (dano > 0 && yo.escudoPorGolpe > 0 && yo.escudo < yo.escudoMax) {
        yo.escudo = std::min(yo.escudoMax, yo.escudo + yo.escudoPorGolpe);
        c.log.push_back("Escudo de " + nombre(c, atacante) + ": " + std::to_string(yo.escudo) + ".");
    }
    return perdido;
}

// «+1d6 de dano» de quien te marco; sin origen (una condicion del catalogo),
// de cualquiera. Vale para los golpes y para las cartas que hacen dano.
int CartasEnCombate::bonoDanoContra(ContextoBatalla& c, Ref atacante, Ref objetivo) const {
    int dano = 0;
    for (const RasgoActivo& r : rasgosDe(c, objetivo)) {
        if (r.g.tipo == T::BonoDanoContra &&
            (!r.conOrigen || puestoPor(r, atacante.lado == Lado::Aliado, atacante.i))) {
            dano += r.g.dados.tirar(c.rng);
        }
    }
    return dano;
}

int CartasEnCombate::aplicarDano(ContextoBatalla& c, Ref fuente, Ref objetivo, int cantidad, bool fisico) {
    if (cantidad <= 0 || !vivo(c, objetivo)) return 0;
    if (fisico && tiene(c, objetivo, T::Refleja) && vivo(c, fuente)) {
        combatiente(c, fuente)->takeDamage(cantidad);
        c.log.push_back(nombre(c, objetivo) + " refleja el golpe: " + nombre(c, fuente) + " recibe " +
                        std::to_string(cantidad) + ".");
        trasDano(c, fuente);
        return 0;
    }
    SituacionCombate& s = situ(objetivo);
    const int absorbido = std::min(s.escudo, cantidad);
    if (absorbido > 0) {
        s.escudo -= absorbido;
        cantidad -= absorbido;
        c.log.push_back("El escudo de " + nombre(c, objetivo) + " absorbe " + std::to_string(absorbido) + ".");
    }
    ICombatant& x = *combatiente(c, objetivo);
    const int antes = x.health();
    x.takeDamage(cantidad);
    const int perdido = antes - x.health();
    if (perdido > 0) apuntaAgresor(c, fuente, objetivo);
    if (perdido > 0 && c.condiciones != nullptr && x.character_sheet() != nullptr) {
        RPG::ConditionEngine::on_damage_received(*x.character_sheet(), *c.condiciones, perdido);
    }
    trasDano(c, objetivo);
    return perdido;
}

// Despues de un golpe: Ultimo Aliento si cae un aliado, y si cae el heroe sus
// invocaciones se desvanecen y sus acompañantes se retiran (la batalla la
// pierde el heroe).
void CartasEnCombate::trasDano(ContextoBatalla& c, Ref objetivo) {
    if (objetivo.lado != Lado::Aliado || vivo(c, objetivo)) return;
    if (m_turno.preparada >= 0 && !m_turno.reaccionUsada && objetivo.i == m_turno.preparadaPara) {
        const CatalogoCartas::Carta& carta = *m_mano[static_cast<std::size_t>(m_turno.preparada)].carta;
        for (const Gancho& g : carta.ganchos) {
            if (g.tipo != T::Revive) continue;
            combatiente(c, objetivo)->heal(g.numero);
            c.log.push_back(carta.def->name + ": " + nombre(c, objetivo) + " vuelve con " +
                            std::to_string(g.numero) + " PV.");
            gastarPreparada(c);
            return;
        }
    }
    if (objetivo.i == 0) {
        for (const auto& inv : m_invocaciones) inv->cuerpo->desvanecer();
    }
    caeAliado(c, objetivo.i);  // ACOMPANANTES-01: se retira, o se retiran con el heroe
}

// Las reacciones que saltan ANTES de que un enemigo tire: bloquear el golpe
// (y cegar al atacante), contrarrestarlo todo durante el turno, o ponerse
// delante de un aliado. true = el golpe no llega.
bool CartasEnCombate::reaccionAntesDelGolpe(ContextoBatalla& c, Ref atacante, Ref& objetivo, bool fisico, bool& atraviesa) {
    if (m_turno.preparada < 0 || m_turno.reaccionUsada) return false;
    const CatalogoCartas::Carta& carta = *m_mano[static_cast<std::size_t>(m_turno.preparada)].carta;
    const std::string& carta_nombre = carta.def->name;
    for (const Gancho& g : carta.ganchos) {
        if ((g.tipo == T::Bloquea || g.tipo == T::BloqueaTodos) && objetivo.i == 0) {
            if (fisico && (atraviesa = bloqueoAtravesado(c, atacante, objetivo, carta_nombre))) return gastarPreparada(c), false;
            c.log.push_back(carta_nombre + ": " + nombre(c, objetivo) + " bloquea el ataque de " +
                            nombre(c, atacante) + ".");
            for (const Gancho& otro : carta.ganchos) {
                if (otro.tipo == T::Condicion) {
                    EstadoCarta e = estadoDeCondicion(otro.texto, otro.rondas);
                    e.origen = 0;
                    ponerEstado(c, atacante, e);
                } else if (otro.tipo == T::Limpia) {
                    limpiar(c, objetivo, carta_nombre);  // SP-02: al saltar
                }
            }
            ponerPropios(c, *carta.def, carta.ganchos);  // SP-02: lo que gana al bloquear
            if (g.tipo == T::BloqueaTodos) {
                EstadoCarta e;
                e.id = carta.def->id;
                e.nombre = carta_nombre;
                e.rondas = g.rondas;
                Gancho rasgo;
                rasgo.tipo = T::BloqueaTodos;
                e.rasgos.push_back(rasgo);
                ponerEstado(c, objetivo, e);
            }
            gastarPreparada(c);
            return true;
        }
        if (g.tipo == T::DefiendeAliado && objetivo.i != 0 && objetivo.lado == Lado::Aliado &&
            vivo(c, {Lado::Aliado, 0})) {
            c.log.push_back(carta_nombre + ": " + nombre(c, {Lado::Aliado, 0}) + " se interpone por " +
                            nombre(c, objetivo) + ".");
            situ(objetivo).protector = 0;
            objetivo = {Lado::Aliado, 0};
            gastarPreparada(c);
            return false;
        }
    }
    return false;
}

void CartasEnCombate::turnoInvocaciones(ContextoBatalla& c) {
    for (const auto& inv : m_invocaciones) {
        const Ref yo{Lado::Aliado, inv->aliado};
        if (!vivo(c, yo) || tiene(c, yo, T::Incapacitado)) continue;
        const int e = primerVivo(c, Lado::Enemigo);
        if (e < 0) return;
        golpeDeArma(c, yo, {Lado::Enemigo, static_cast<std::size_t>(e)}, nullptr);
    }
}

bool CartasEnCombate::resolverEnemigo(ContextoBatalla& c, std::size_t enemigo) {
    asegurarTamanos(c);
    m_tiradas.empiezaAccion();  // lo que tire este enemigo sale junto
    const Ref yo{Lado::Enemigo, enemigo};
    BattleParticipant& p = c.enemigos[enemigo];
    const std::vector<RasgoActivo> mios = rasgosDe(c, yo);
    if (hay(mios, T::Incapacitado)) {
        c.log.push_back(p.name + " no puede actuar.");
        return true;
    }
    int destino = objetivoDeEnemigo(c, enemigo);  // quien le hizo daño, o el heroe
    for (const RasgoActivo& r : mios) {
        if (r.g.tipo == T::Provocado && r.conOrigen && r.origenAliado && vivo(c, {Lado::Aliado, r.origen}) &&
            (!c.llega || c.llega(enemigo, r.origen))) {
            destino = static_cast<int>(r.origen);  // Ira Imparable: te ataca a ti
        }
    }
    if (destino < 0) return true;
    Ref objetivo{Lado::Aliado, static_cast<std::size_t>(destino)};

    // La IA de siempre (BattleState::resolveEnemyTurn): la primera habilidad
    // conocida que pueda pagar; si no, golpe de arma.
    const Skill* elegida = nullptr;
    const bool silenciado = hay(mios, T::Silenciado);  // AV16-14: solo golpe de arma y lo fisico
    if (!silenciado && p.skills != nullptr && c.habilidadesLegado != nullptr) {
        for (const std::string& id : p.skills->knownSkillIds()) {
            const Skill* s = c.habilidadesLegado->find(id);
            if (s != nullptr && p.skills->canUse(*s)) {
                elegida = s;
                break;
            }
        }
    }
    if (elegida != nullptr && (elegida->target == SkillTarget::Self || elegida->target == SkillTarget::SingleAlly ||
                               elegida->target == SkillTarget::AllAllies)) {
        p.skills->spend(*elegida);
        const SkillApplyResult r = ApplySkillEffect(*elegida, *p.combatant, *p.combatant, c.rng);
        c.log.push_back(p.name + " usa " + elegida->name + " (+" + std::to_string(r.hpChange) + " PV).");
        return true;
    }
    if (elegida == nullptr && hay(mios, T::Desarmado)) {
        c.log.push_back(p.name + " esta desarmado y no puede atacar.");
        return true;
    }
    if (situ(objetivo).protector >= 0) objetivo = {Lado::Aliado, static_cast<std::size_t>(situ(objetivo).protector)};
    bool atraviesa = false;  // AV13-30: el critico atraviesa el bloqueo
    if (reaccionAntesDelGolpe(c, yo, objetivo, elegida == nullptr, atraviesa)) return true;
    if (elegida == nullptr) {
        golpeDeArma(c, yo, objetivo, atraviesa ? &ganchoCritico() : nullptr);
        return true;
    }
    if (tiene(c, objetivo, T::BloqueaTodos)) {
        c.log.push_back(nombre(c, objetivo) + " contrarresta " + elegida->name + ".");
        return true;
    }
    p.skills->spend(*elegida);
    const RPG::SkillDefinition def = definicionDeHabilidad(*elegida);
    const Tirada t = tirar(c, yo, objetivo, def, true, false);
    // DANO-02: una habilidad de monstruo pega como un golpe y suma un d6 por
    // cada 10 de poder (la de 10, +1d6; la de 20, +2d6), solo si gana.
    Dados propios;
    propios.cantidad = (std::max(0, elegida->power) + 5) / 10;
    std::string como;
    const int dano = danoDeCarta(t, propios, def.save_attribute != RPG::SaveAttribute::NONE, c.rng, como);
    c.log.push_back(p.name + " usa " + elegida->name + " sobre " + nombre(c, objetivo) + ": " + t.resumen + ". " +
                    frases::dano(dano, como) + ".");
    aplicarDano(c, yo, objetivo, dano, true);
    if (!def.apply_condition_id.empty() && t.grado >= def.apply_condition_min_degree && vivo(c, objetivo)) {
        RPG::CharacterSheet* ficha = combatiente(c, objetivo)->character_sheet();
        const RPG::ConditionDefinition* cond = m_catalogo.condicion(def.apply_condition_id);
        if (ficha != nullptr && cond != nullptr && c.condiciones != nullptr) {
            // Con ficha, la condicion va a la ficha como siempre (D-09): dura
            // mas alla del combate si la carta lo dice.
            RPG::ConditionEngine::apply(*ficha, *cond, *c.condiciones);
            c.log.push_back(p.name + " aplica " + cond->name + " a " + nombre(c, objetivo) + ".");
        } else {
            EstadoCarta e = estadoDeCondicion(def.apply_condition_id, def.condition_duration_rounds);
            e.origenAliado = false;
            e.origen = enemigo;
            ponerEstado(c, objetivo, e);
        }
    }
    return true;
}

}  // namespace Game
