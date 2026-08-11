#pragma once

#include <cstddef>
#include <string>

// Contrato comun de los catalogos por id del motor (fractura #2 del analisis
// de coherencia, ver ARCHITECTURE.md). Unifica tres catalogos que hacia lo
// mismo sin relacionarse: ResourceManager<T> (recursos GL), SkillCatalog
// (habilidades) y ObjectCatalog (objetos del mundo). Los tres mapean un id
// (string) a una definicion/recurso y exponen las mismas tres operaciones:
//
//   - has(id):   existe esa entrada?
//   - find(id):  puntero NO propietario a la entrada, o nullptr si no existe
//   - size():    cuantas entradas hay
//
// ICatalog<T> solo formaliza eso como interfaz; no impone como se carga ni
// quien posee los T (ResourceManager los posee con unique_ptr y los carga
// con loadFromDisk; SkillCatalog/ObjectCatalog los copian con add() desde un
// JSON). Igual que IRenderable/IUpdatable/ICombatant solo formalizan
// contratos que las clases ya cumplian por separado.
//
// Nomenclatura canonica: has/find/size (la usan ya SkillCatalog y
// ObjectCatalog, y es la idiomatica de C++: std::map::find, std::size).
// ResourceManager<T> historico usa contains/get/count (estilo Java): sus
// implementaciones de ICatalog<T> (has/find/size) delegan en los viejos, y
// los viejos se mantienen por compatibilidad hacia atras (marcados
// "alias de has/find/size" en su comentario).
template <typename T>
class ICatalog {
public:
    virtual ~ICatalog() = default;

    virtual bool has(const std::string& id) const = 0;
    // Puntero no propietario: el catalogo sigue siendo dueno del T. nullptr
    // = "no existe" (mismo criterio permisivo que TextureManager::get():
    // un id que falta no es un bug del llamador, se comprueba con has/find).
    virtual const T* find(const std::string& id) const = 0;
    virtual std::size_t size() const = 0;
};
