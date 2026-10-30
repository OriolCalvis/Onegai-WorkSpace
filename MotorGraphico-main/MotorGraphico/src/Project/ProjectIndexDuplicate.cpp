// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 2 paquetes del corte B:
//   C-11  (fase C, 4 j) - Un sol format de projecte
//   C-15  (fase C, 2 j) - Desar deixa el projecte coherent
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// E-11 — Nombre visible arriba, id generado abajo. Extret de
// ProjectIndex.cpp (EST-28) per reduir-lo per sota de 500 línies.

#include "Project/ProjectIndex.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "Core/Json/JsonValue.h"
#include "Project/ProjectSerializer.h"
#include "ProjectIndexInterno.h"
#include <algorithm>

namespace Editor {
// Los cuatro helpers que este fichero compartia byte a byte con su gemelo,
// en un solo sitio y con la escritura del indice atomica. Ver
// src/Project/ProjectIndexInterno.h.
using namespace ProjectIndexIO;

namespace {

// Pliega los acentos del Latin-1 en UTF-8 (dos bytes que empiezan por
// 0xC3). El mismo criterio que usa la busqueda de G-17, escrito aqui
// otra vez a proposito: motor_project no depende de motor_engine y
// crear esa flecha por seis lineas seria peor que repetirlas.
char plegarAcento(unsigned char segundo) {
    const unsigned char b = segundo | 0x20;  // a minuscula
    if (b >= 0xA0 && b <= 0xA5) return 'a';
    if (b >= 0xA8 && b <= 0xAB) return 'e';
    if (b >= 0xAC && b <= 0xAF) return 'i';
    if (b >= 0xB2 && b <= 0xB6) return 'o';
    if (b >= 0xB9 && b <= 0xBC) return 'u';
    if (b == 0xB1) return 'n';
    if (b == 0xA7) return 'c';
    return 0;
}

// Nombre de fichero con el prefijo cambiado. "es_taurengrad.json" con
// prefijos "es_" -> "ns_" da "ns_taurengrad.json". Cadena vacia si el
// fichero NO empieza por el prefijo viejo: eso significa que no es suyo.
std::string reprefijar(const std::string& fichero, const std::string& viejo,
                       const std::string& nuevo) {
    if (viejo.empty() || fichero.size() < viejo.size() ||
        fichero.compare(0, viejo.size(), viejo) != 0) {
        return std::string();
    }
    return nuevo + fichero.substr(viejo.size());
}

bool copiaFichero(const std::string& origen, const std::string& destino) {
    std::string contenido;
    if (!leeFichero(origen, contenido)) {
        return false;
    }
    std::FILE* fh = std::fopen(destino.c_str(), "wb");
    if (fh == nullptr) {
        return false;
    }
    const std::size_t escritos = contenido.empty()
        ? 0
        : std::fwrite(contenido.data(), 1, contenido.size(), fh);
    const bool ok = (escritos == contenido.size());
    std::fclose(fh);
    return ok;
}

// Carpeta que le toca a cada familia de asset dentro del proyecto.
std::string carpetaDe(const Project& p, const char* familia) {
    if (std::strcmp(familia, "levels") == 0) return p.settings.levelsDirectory;
    if (std::strcmp(familia, "maps") == 0) return p.settings.mapsDirectory;
    if (std::strcmp(familia, "adventures") == 0) return p.settings.adventuresDirectory;
    return p.settings.catalogsDirectory;
}

}  // namespace

Result<std::string> ProjectIndex::suggestId(const std::string& visibleName) {
    std::string id;
    bool ultimoGuion = true;  // true al principio: evita empezar por "_"
    for (std::size_t i = 0; i < visibleName.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(visibleName[i]);
        char letra = 0;
        if (c == 0xC3 && i + 1 < visibleName.size()) {
            letra = plegarAcento(static_cast<unsigned char>(visibleName[i + 1]));
            if (letra != 0) {
                ++i;
            }
        }
        if (letra == 0) {
            if (c >= 'A' && c <= 'Z') {
                letra = static_cast<char>(c - 'A' + 'a');
            } else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
                letra = static_cast<char>(c);
            }
        }
        if (letra == 0) {
            // Cualquier otra cosa —espacios, guiones, signos, el resto de
            // UTF-8— es un separador, y varios seguidos son uno solo.
            if (!ultimoGuion) {
                id += '_';
                ultimoGuion = true;
            }
            continue;
        }
        id += letra;
        ultimoGuion = false;
    }
    while (!id.empty() && id.back() == '_') {
        id.pop_back();
    }
    if (id.empty()) {
        return Result<std::string>::Error(
            "de '" + visibleName + "' no sale ningun identificador: hace falta al menos "
            "una letra o un numero en el nombre");
    }
    return Result<std::string>::Ok(id);
}

std::string ProjectIndex::uniqueId(const std::string& baseId,
                                   const std::vector<std::string>& taken) {
    if (std::find(taken.begin(), taken.end(), baseId) == taken.end()) {
        return baseId;
    }
    // Desde el 2: "cofre" y "cofre_2" se leen como el primero y el
    // segundo, que es como los cuenta cualquiera.
    for (int n = 2; n < 10000; ++n) {
        const std::string candidato = baseId + "_" + std::to_string(n);
        if (std::find(taken.begin(), taken.end(), candidato) == taken.end()) {
            return candidato;
        }
    }
    return baseId + "_" + std::to_string(taken.size() + 1);
}

Result<Project> ProjectIndex::createNamed(const std::string& assetsRoot,
                                          const std::string& visibleName,
                                          const std::string& prefix,
                                          const std::string& author,
                                          const std::string& epoch) {
    const Result<std::string> base = suggestId(visibleName);
    if (!base.isOk()) {
        return Result<Project>::Error(base.errorMessage());
    }
    auto indice = scan(assetsRoot);
    if (!indice.isOk()) {
        return Result<Project>::Error(indice.errorMessage());
    }
    // Ocupados: los declarados en el indice Y los manifiestos sueltos.
    // Mirar solo el indice dejaria que createNamed eligiera un id cuyo
    // fichero ya existe, y create() fallaria despues de haber elegido.
    std::vector<std::string> ocupados = indice.value().m_declaredIds;
    for (const Project& p : indice.value().m_projects) {
        ocupados.push_back(p.id);
    }
    std::string id = uniqueId(base.value(), ocupados);
    while (existe(assetsRoot + "/proyectos/" + id + ".json")) {
        ocupados.push_back(id);
        id = uniqueId(base.value(), ocupados);
    }
    return create(assetsRoot, id, visibleName, prefix, author, epoch);
}

Result<ProjectIndex::DuplicateReport> ProjectIndex::duplicate(
        const std::string& assetsRoot, const std::string& sourceId,
        const std::string& newVisibleName) {
    using R = Result<DuplicateReport>;

    auto indice = scan(assetsRoot);
    if (!indice.isOk()) {
        return R::Error(indice.errorMessage());
    }
    const Project* origen = indice.value().find(sourceId);
    if (origen == nullptr) {
        return R::Error("no hay ningun proyecto con id '" + sourceId + "'");
    }

    const Result<std::string> base = suggestId(newVisibleName);
    if (!base.isOk()) {
        return R::Error(base.errorMessage());
    }
    std::vector<std::string> ocupados = indice.value().m_declaredIds;
    for (const Project& p : indice.value().m_projects) {
        ocupados.push_back(p.id);
    }
    std::string nuevoId = uniqueId(base.value(), ocupados);
    while (existe(assetsRoot + "/proyectos/" + nuevoId + ".json")) {
        ocupados.push_back(nuevoId);
        nuevoId = uniqueId(base.value(), ocupados);
    }

    // El prefijo nuevo sale del id nuevo. Si el original no tenia
    // prefijo, la copia tampoco lo lleva y entonces no hay nada «suyo»
    // que copiar: todo queda compartido, y el informe lo dice.
    const std::string prefijoViejo = origen->prefix;
    const std::string prefijoNuevo = prefijoViejo.empty() ? std::string() : (nuevoId + "_");

    DuplicateReport informe;
    informe.project = *origen;
    informe.project.id = nuevoId;
    informe.project.name = newVisibleName;
    informe.project.prefix = prefijoNuevo;

    // --- Primero se decide TODO y se comprueba que nada exista ya. No se
    // escribe ni un byte hasta que se sabe que la operacion entera cabe.
    struct Copia { std::string origen; std::string destino; std::string nombreNuevo; };
    std::vector<Copia> copias;
    auto planear = [&](std::vector<std::string>& lista, const char* familia) -> std::string {
        const std::string carpeta = carpetaDe(*origen, familia);
        for (std::string& fichero : lista) {
            const std::string nuevoNombre = reprefijar(fichero, prefijoViejo, prefijoNuevo);
            if (nuevoNombre.empty()) {
                informe.shared.push_back(fichero);
                continue;
            }
            const std::string rutaOrigen = assetsRoot + "/" + carpeta + "/" + fichero;
            const std::string rutaDestino = assetsRoot + "/" + carpeta + "/" + nuevoNombre;
            if (!existe(rutaOrigen)) {
                return "el proyecto declara " + rutaOrigen + " y ese fichero no esta";
            }
            if (existe(rutaDestino)) {
                return "ya existe " + rutaDestino + ": duplicar no sobreescribe nada";
            }
            copias.push_back({rutaOrigen, rutaDestino, nuevoNombre});
            fichero = nuevoNombre;
        }
        return std::string();
    };

    std::string fallo = planear(informe.project.levels, "levels");
    if (fallo.empty()) fallo = planear(informe.project.maps, "maps");
    if (fallo.empty()) fallo = planear(informe.project.adventures, "adventures");
    if (fallo.empty()) fallo = planear(informe.project.catalogs, "catalogs");
    if (!fallo.empty()) {
        return R::Error(fallo);
    }
    // La entrada apunta a un fichero: si era suyo, ahora apunta a la copia.
    if (!informe.project.entry.empty()) {
        const std::string nuevaEntrada =
            reprefijar(informe.project.entry, prefijoViejo, prefijoNuevo);
        if (!nuevaEntrada.empty()) {
            informe.project.entry = nuevaEntrada;
        }
    }

    const Result<bool> valido = informe.project.validate();
    if (!valido.isOk()) {
        return R::Error("la copia no seria un proyecto valido: " + valido.errorMessage());
    }

    // --- Ahora se escribe. Si algo falla, se deshace lo escrito y el
    // indice no se toca: el criterio de cierre pide exactamente esto.
    std::vector<std::string> escritos;
    auto deshacer = [&escritos]() {
        for (const std::string& ruta : escritos) {
            std::remove(ruta.c_str());
        }
    };
    for (const Copia& c : copias) {
        if (!copiaFichero(c.origen, c.destino)) {
            deshacer();
            return R::Error("no se ha podido copiar " + c.origen + " a " + c.destino +
                            "; no se ha registrado nada");
        }
        escritos.push_back(c.destino);
        informe.copied.push_back(c.nombreNuevo);
    }

    const Result<std::string> manifiesto = ProjectSerializer::serializeProject(informe.project);
    if (!manifiesto.isOk()) {
        deshacer();
        return R::Error(manifiesto.errorMessage());
    }
    const std::string rutaManifiesto = assetsRoot + "/proyectos/" + nuevoId + ".json";
    std::FILE* fh = std::fopen(rutaManifiesto.c_str(), "wb");
    if (fh == nullptr) {
        deshacer();
        return R::Error("no se puede escribir " + rutaManifiesto + "; no se ha registrado nada");
    }
    std::fwrite(manifiesto.value().data(), 1, manifiesto.value().size(), fh);
    std::fclose(fh);
    escritos.push_back(rutaManifiesto);

    // El indice, lo ULTIMO. Mientras no se toque, el proyecto no existe
    // para nadie y cualquier fallo anterior se limpia sin dejar rastro.
    std::vector<std::string> ids = indice.value().m_declaredIds;
    if (std::find(ids.begin(), ids.end(), nuevoId) == ids.end()) {
        ids.push_back(nuevoId);
    }
    const Result<bool> escrito = escribeIndice(assetsRoot, ids);
    if (!escrito.isOk()) {
        deshacer();
        return R::Error("no se ha podido actualizar el indice; no se ha registrado nada: " +
                        escrito.errorMessage());
    }
    return R::Ok(informe);
}

}  // namespace Editor
