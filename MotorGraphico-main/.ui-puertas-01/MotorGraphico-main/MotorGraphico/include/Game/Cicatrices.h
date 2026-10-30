#pragma once
// AV13-33 (Oriol, 14/10, D0015 §7) — Morir deja cicatriz: «si mueres vuelves al ultimo punto de
// guardado despues de tener una cicatriz mas».
//
// Las cicatrices viven FUERA de las partidas guardadas, en saves/<proyecto>/cicatrices.txt (una
// linea por personaje: «id<TAB>cuantas»): asi cargar la ultima partida no las borra, y no hace
// falta reescribir el fichero de la partida (que es lo que rompio «Continuar» el 14/10, PO-03).
// GL-free; se prueba en tests/Game/CicatricesTests.cpp.

#include <map>
#include <string>

namespace Game {

class Cicatrices {
public:
    // Lee el fichero (si no existe, ninguna).
    static Cicatrices carga(const std::string& ruta);
    // Escribe el fichero entero. false si no se pudo.
    bool guarda(const std::string& ruta) const;

    int de(const std::string& personaje) const;
    // Una mas; devuelve cuantas lleva ya.
    int suma(const std::string& personaje);

    // «Una cicatriz más. Ya llevas 3.» (lo que dice la pantalla de «Has caído»).
    static std::string nota(int cuantas);

private:
    std::map<std::string, int> m_porPersonaje;
};

}  // namespace Game
