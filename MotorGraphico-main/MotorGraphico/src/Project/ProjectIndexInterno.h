// Lo que ProjectIndex.cpp y ProjectIndexDuplicate.cpp comparten y no es API:
// leer un fichero, saber si existe, escapar un id y reescribir el indice.
//
// POR QUE EXISTE. Los cuatro estaban duplicados BYTE A BYTE en los dos .cpp
// (lo midio tools/buscar_sobrante.py --repetido: cuatro grupos, 50 lineas). Los
// dos ficheros viven en motor_project, asi que aqui no vale la excusa que si
// justifica otras repeticiones del motor —la de plegarAcento, que no se comparte
// para no crear una flecha entre librerias por seis lineas—: compartir no cuesta
// nada. Y la copia no era inocua, es el mecanismo de siempre: el arreglo de la
// escritura atomica (5ecaab2) llego a SerializerHelpers y a SaveGame, y el
// indice se quedo con su fopen("wb") truncante en los dos sitios a la vez.
//
// Vive en src/ y no en include/ (patron de src/Editor/ProjectMapsDetail.h): no
// lo incluye nadie mas, y asi no suma una cabecera publica sin consumidor.
// Todo es inline porque lo incluyen dos unidades de traduccion de la misma
// libreria; con definiciones normales el enlazador veria cada simbolo dos veces.
#pragma once

#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Project/SerializerHelpers.h"

namespace Editor {
// Namespace propio y `using namespace` en cada .cpp, igual que ProjectIO en
// SerializerHelpers.h: los cuerpos que ya estaban escritos no se tocan ni una
// linea y las llamadas siguen siendo igual de cortas.
namespace ProjectIndexIO {

inline bool leeFichero(const std::string& ruta, std::string& out) {
    std::FILE* f = std::fopen(ruta.c_str(), "rb");
    if (f == nullptr) {
        return false;
    }
    std::fseek(f, 0, SEEK_END);
    const long sz = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    out.clear();
    if (sz > 0) {
        out.resize(static_cast<std::size_t>(sz));
        (void)std::fread(&out[0], 1, static_cast<std::size_t>(sz), f);
    }
    std::fclose(f);
    return true;
}

inline bool existe(const std::string& ruta) {
    std::FILE* f = std::fopen(ruta.c_str(), "rb");
    if (f == nullptr) {
        return false;
    }
    std::fclose(f);
    return true;
}

// El indice solo escribe ids previamente validados como [a-z0-9_].
inline std::string esc(const std::string& s) {
    std::string r;
    r.reserve(s.size() + 8);
    for (char c : s) {
        if (c == '"' || c == '\\') {
            r += '\\';
        }
        r += c;
    }
    return r;
}

// Reescribe assets/proyectos/index.json con esta lista de ids. Lo usan
// create(), remove() y duplicate(): el manifiesto en disco y el indice tienen
// que moverse juntos o el proyecto existe y no lo ve nadie (o al reves).
//
// ATOMICO, Y NO ES COSMETICA. Antes hacia fopen(ruta, "wb") y escribia encima:
// un corte a mitad dejaba el indice —la lista de TODOS los proyectos— vacio o a
// medias, y el motor no encontraba ningun proyecto aunque los manifiestos
// estuvieran intactos. Los manifiestos ya se guardaban con saveProjectAtomic y
// las partidas con ProjectIO::atomicWrite; el indice era el unico fichero del
// sistema de proyectos que se truncaba. Los bytes que produce son EXACTAMENTE
// los de antes, que es lo que comprueba test_project_manifests al exigir la ida
// y vuelta canonica.
inline Result<bool> escribeIndice(const std::string& assetsRoot,
                                  const std::vector<std::string>& ids) {
    std::string texto =
        "{\n \"_nota\": \"Lista de proyectos vivos. La lee Editor/ProjectIndex.\",\n"
        " \"proyectos\": [\n";
    for (std::size_t i = 0; i < ids.size(); ++i) {
        texto += "  \"" + esc(ids[i]) + "\"";
        if (i + 1 < ids.size()) {
            texto += ",";
        }
        texto += "\n";
    }
    texto += " ]\n}\n";
    return ProjectIO::atomicWrite(assetsRoot + "/proyectos/index.json", texto);
}

}  // namespace ProjectIndexIO
}  // namespace Editor
