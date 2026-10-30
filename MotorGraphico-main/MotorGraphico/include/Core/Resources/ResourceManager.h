#pragma once

#include <cstddef>
#include <exception>
#include <memory>
#include <string>
#include <unordered_map>

#include "Core/Errors/Result.h"
#include "Core/Resources/ICatalog.h"

// ResourceManager<T> es la base generica para cualquier gestor de recursos
// (texturas, shaders, futuros: sonidos, fuentes...). Resuelve dos problemas
// de una vez: (1) evita duplicar el codigo de cache/propiedad para cada
// tipo de recurso, y (2) normaliza los fallos de carga a Result<T*>, sin
// importar si la implementacion concreta de loadFromDisk() lanza
// excepciones (ResourceLoadException, ShaderCompileException...) o
// simplemente devuelve nullptr.
//
// Las subclases SOLO implementan loadFromDisk(); toda la gestion de mapa,
// cache y ownership (unique_ptr) vive aqui una unica vez.
//
// : public ICatalog<T> (fractura #2 del analisis de coherencia, ver
// ARCHITECTURE.md): unifica los catalogos por id del motor (ResourceManager,
// SkillCatalog, ObjectCatalog) bajo un mismo contrato has/find/size. Los
// nombres viejos contains/get/count se mantienen como alias por
// compatibilidad hacia atras (delegan en has/find/size).
template <typename T>
class ResourceManager : public ICatalog<T> {
public:
    virtual ~ResourceManager() = default;

    // Carga (o recupera de cache si "id" ya existe) un recurso.
    // Nunca lanza: cualquier fallo de loadFromDisk() (excepcion o nullptr)
    // se convierte en Result<T*>::Error con un mensaje descriptivo.
    Result<T*> load(const std::string& id, const std::string& path) {
        auto existing = m_resources.find(id);
        if (existing != m_resources.end()) {
            return Result<T*>::Ok(existing->second.get());
        }

        std::unique_ptr<T> resource;
        try {
            resource = loadFromDisk(path);
        } catch (const std::exception& e) {
            return Result<T*>::Error("Excepcion durante loadFromDisk('" + path + "'): " + e.what());
        }

        if (!resource) {
            return Result<T*>::Error("loadFromDisk('" + path + "') devolvio nullptr");
        }

        T* rawPtr = resource.get();
        m_resources.emplace(id, std::move(resource));
        // rawPtr sigue vivo: apunta al T* en el heap que ahora posee
        // m_resources, no al unique_ptr local movido. cppcheck no
        // distingue esto y lo marca como dangling; falso positivo.
        // cppcheck-suppress returnDanglingLifetime
        return Result<T*>::Ok(rawPtr);
    }

    // --- ICatalog<T> (nomenclatura canonica has/find/size) ---
    bool has(const std::string& id) const override {
        return m_resources.find(id) != m_resources.end();
    }
    const T* find(const std::string& id) const override {
        auto it = m_resources.find(id);
        return (it != m_resources.end()) ? it->second.get() : nullptr;
    }
    std::size_t size() const override { return m_resources.size(); }

    // --- Alias historicos (estilo Java, previos a ICatalog<T>). Delegan
    // en los canonigos para que no haya dos fuentes de verdad. Se mantienen
    // por compatibilidad hacia atras: callers existentes usan get/contains/
    // count y seguir funcionando sin tocarlos. ---
    T* get(const std::string& id) const { return const_cast<T*>(find(id)); }
    bool contains(const std::string& id) const { return has(id); }
    std::size_t count() const { return size(); }

    void unload(const std::string& id) { m_resources.erase(id); }
    void clear() { m_resources.clear(); }

protected:
    // Unico metodo que cada gestor concreto debe implementar. Puede lanzar
    // excepciones de dominio (ResourceLoadException, ShaderCompileException)
    // con detalle rico del fallo: load() las capturara y las normalizara.
    virtual std::unique_ptr<T> loadFromDisk(const std::string& path) = 0;

    std::unordered_map<std::string, std::unique_ptr<T>> m_resources;
};
