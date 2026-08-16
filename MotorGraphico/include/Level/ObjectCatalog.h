#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Resources/ICatalog.h"

// Catalogo generico de objetos (motor_grafico_gantt_rpg.puml, Fase 10:
// "todo es un objeto" -- desde una llave hasta un arbusto o un enemigo).
// Generaliza el "catalogo de enemigos" implicito de la Fase 6: en vez de
// que EnemySpawn::type referencie un catalogo que solo sabe de enemigos,
// ObjectSpawn::objectId (ver LevelDefinition.h) referencia entradas de
// AQUI, y la categoria de cada entrada dice que es (enemigo, item
// recogible, decorado...). GL-free como todo motor_level: datos puros,
// sin Entity/Texture/GL -- quien instancie el nivel (la futura
// Application, o un demo) decide como convertir cada ObjectDefinition en
// una entidad real (un Enemy, un sprite estatico, una entrada de
// inventario...).
//
// Datos por categoria (CombatData/PickupData): miembros planos SIEMPRE
// presentes en ObjectDefinition, no un std::variant -- solo tienen
// significado si la categoria coincide (combat solo si category==Enemy,
// pickup solo si category==Pickup) y quedan con sus defaults inertes en
// el resto de casos. Un variant seria mas "correcto" pero complica cada
// acceso (visitas/get_if) para exactamente dos categorias con datos; si
// las categorias con datos crecen, se reevalua (mismo criterio anti
// sobre-ingenieria que IHudElement sin update(), ver HudElement.h).

// Que es cada objeto del catalogo. Prop = decorado/escenario (un
// arbusto, una roca): puede bloquear el paso o ser interactuable, pero
// no tiene datos de combate ni de recogida.
// Npc = personaje con el que se habla (y que ademas puede regentar una
// tienda, ver ShopData): no combate ni se recoge, pero SI bloquea el
// paso -- se habla con el desde una celda contigua, igual que en
// cualquier RPG clasico.
enum class ObjectCategory { Prop, Enemy, Pickup, Npc };

// Datos de combate (solo con ObjectCategory::Enemy): lo que hace falta
// para armar un combatiente de la Fase 8 -- vida/mana maximos y las
// habilidades (ids contra SkillCatalog, ver Skill.h) que conoce por
// defecto. Sustituye a EnemySpawn::skillIds como fuente de "que sabe
// hacer un slime": las habilidades son del TIPO de enemigo (catalogo),
// no de cada spawn individual (nivel) -- mismo principio que EnemySpawn
// no duplicando stats por spawn (ver su comentario en LevelDefinition.h).
struct CombatData {
    int maxHealth = 10;
    int maxMana = 0;
    std::vector<std::string> skillIds;
};

// Efecto al CONSUMIR un objeto recogible (solo con ObjectCategory::
// Pickup). None = objeto de inventario sin efecto consumible (una llave:
// se tiene o no se tiene, abrir puertas con ella es logica de quien
// orqueste el nivel, no del catalogo).
enum class PickupEffect { None, Heal, RestoreMana };

struct PickupData {
    PickupEffect effect = PickupEffect::None;
    // Magnitud del efecto (HP curados / mana restaurado); ignorado con
    // PickupEffect::None. Mismo rol que Skill::power.
    int power = 0;
    // Precio base en oro. Es del TIPO de objeto, no de cada tienda
    // (mismo principio que CombatData con las habilidades): una tienda
    // puede aplicar su margen sobre este valor, pero "cuanto vale una
    // pocion" es una propiedad de la pocion. 0 = sin precio (no se
    // compra ni se vende: una llave de mision, por ejemplo).
    int price = 0;
};

// Un articulo a la venta: un id del catalogo mas su precio EN ESTA
// tienda. El precio va aqui y no solo en PickupData porque el mismo
// objeto puede costar distinto segun donde se compre (la posada del
// centro es mas cara que la del sur), que es justo lo que da vida a
// tener varias tiendas. price <= 0 significa "usa el precio base del
// catalogo".
struct ShopItem {
    std::string objectId;
    int price = 0;
};

// Datos de comercio (solo con ObjectCategory::Npc): que vende y a que
// precio recompra. Vacio = el NPC solo habla.
struct ShopData {
    std::vector<ShopItem> items;
    // Porcentaje del precio de compra que paga el tendero al recomprar
    // (50 = te da la mitad). Es lo que evita el bucle infinito de
    // comprar y vender al mismo precio para... nada, en realidad, pero
    // sobre todo es lo que hace que vender sea una decision y no un
    // deshacer gratuito.
    int buybackPercent = 50;
};

// Lineas de dialogo de un NPC (solo con ObjectCategory::Npc). Se
// muestran en orden en el cuadro de dialogo del HUD, una "pagina" por
// pulsacion. Sin ramas ni condiciones: un arbol de dialogo con estado
// es un sistema propio (ver motor_grafico_dafo.md sobre
// sobre-ingenieria), y para "el tendero te saluda" sobra con una lista.
struct DialogueData {
    std::vector<std::string> lines;
};

// Negocio en venta (el cartel plantado delante de un local). Lo lleva un
// objeto normal de categoria Prop: no hace falta una categoria nueva
// porque un cartel ES un decorado con el que se puede interactuar --
// misma razon por la que las puertas son Prop y no "Door".
//
// El alquiler NO esta aqui: es estado de PARTIDA (cambia cuando el
// jugador lo sube o lo baja), y el catalogo describe tipos, no
// situaciones. Vive en GameSession::OwnedBusiness.
struct BusinessData {
    // > 0 = este objeto es un cartel de negocio en venta. 0 = no lo es,
    // que es lo que hace que un arbusto no ofrezca comprarse.
    int price = 0;
    // Ingreso por ciclo de renta a alquiler JUSTO (100%). El ingreso
    // real sale de aqui, del alquiler que ponga el jugador y de la
    // ocupacion resultante (ver GameSession::collectRent).
    int baseIncome = 0;
    // Nombre del local para el cartel y el HUD ("Posada del viajero").
    // Si esta vacio se usa el name del propio objeto.
    std::string businessName;
};

struct ObjectDefinition {
    std::string id;    // clave estable (ver ObjectSpawn::objectId)
    std::string name;  // nombre para mostrar (HUD, log de combate)
    ObjectCategory category = ObjectCategory::Prop;

    // Frame del TextureAtlas con el que se dibuja (ver
    // TextureAtlas::defineRegion; -1 = sin sprite asignado todavia).
    // Un int y no una ruta de imagen: las entidades existentes
    // (Player/Enemy) ya se dibujan por frame de atlas, no por textura
    // suelta.
    int spriteId = -1;

    // Id opcional de la raza RPG del personaje. Solo es significativo
    // para NPCs, pero permanece como dato de autoría plano para que un
    // nivel pueda referenciarlo sin duplicar el nombre de la raza en cada
    // dialogo o depender de convenciones en el id del objeto.
    std::string raceId;

    bool blocksMovement = false;
    bool interactable = false;

    CombatData combat;      // solo significativo si category == Enemy
    PickupData pickup;      // solo significativo si category == Pickup
    DialogueData dialogue;  // solo significativo si category == Npc
    ShopData shop;          // solo significativo si category == Npc
    BusinessData business;  // price > 0 = cartel de negocio en venta
};

// Catalogo por id, mismo patron que SkillCatalog (ver Skill.h): quien
// construya el contenido lo rellena (desde JSON con loadFrom*, o a mano
// con add()) y los niveles/el combate referencian por id.
//
// : public ICatalog<ObjectDefinition> (fractura #2, ver ARCHITECTURE.md):
// has/find/size ya existian con la firma canonica; marcarlos override
// formaliza el contrato comun con ResourceManager y SkillCatalog.
class ObjectCatalog : public ICatalog<ObjectDefinition> {
public:
    void add(ObjectDefinition definition);
    bool has(const std::string& id) const override;
    // nullptr si no existe (puntero no-propietario, mismo criterio que
    // SkillCatalog::find / ResourceManager<T>::find).
    const ObjectDefinition* find(const std::string& id) const override;
    std::size_t size() const override { return m_objects.size(); }
    // Identificadores ordenados para UI de autoría (editor, selector de
    // spawns). El mapa interno sigue sin exponerse ni poder mutarse desde
    // fuera; devolver una copia evita que una vista altere el catálogo.
    std::vector<std::string> ids() const;
    // Subgrupo de autoría: evita que la paleta visual sea una lista plana
    // de cientos de entradas cuando el contenido de un proyecto crece.
    std::vector<std::string> ids(ObjectCategory category) const;

    // Anaden (merge) las entradas de un JSON a este catalogo -- no lo
    // vacian antes, para poder cargar varios archivos (objetos base +
    // objetos de un area concreta). Ok(n) = numero de entradas anadidas;
    // un id repetido SOBREESCRIBE la entrada anterior (ultimo archivo
    // gana, util para que un area redefina un objeto base) y cuenta
    // igualmente. Error si el JSON no parsea, la raiz no trae "objects"
    // como array, o alguna entrada no tiene "id"/"category" validos --
    // mismo reparto obligatorio/opcional que LevelLoader (ver su
    // comentario: los campos estructurales se rechazan, los de detalle
    // tienen default).
    Result<int> loadFromString(const std::string& jsonText);
    Result<int> loadFromFile(const std::string& path);

private:
    std::unordered_map<std::string, ObjectDefinition> m_objects;
};
