// E-DISPARADORES-1 (D0026.6 y .7) — El ambito de un nivel: decide el radio de los disparadores de cinematica.
//
//   interior_pequeno  se dispara al entrar (sin vision: los taburetes de la taberna taparian al jugador)
//   interior_grande   radio 5
//   exterior          radio 10
//
// El nivel puede declararlo ("ambito" en su JSON). Si falta, se deduce por el nombre del fichero (interior,
// mansion, casa, tienda, dependencias, mina, cueva, galeria, camara, grieta, santuario, mazmorra, antecova y las
// plantas de las aldeas «aldea_*_pN» son interiores) y, en un interior, por el tamano del mapa: MENOS de 200
// casillas = pequeno (la taberna, 180). Cualquier otro nombre es exterior.
#pragma once

#include <climits>
#include <string>

enum class AmbitoNivel { Deducido, InteriorPequeno, InteriorGrande, Exterior };

constexpr int kCasillasInteriorPequeno = 200;  // D0026.7: por debajo, pequeno
constexpr int kRadioInteriorGrande = 5;
constexpr int kRadioExterior = 10;
constexpr int kRadioAlEntrar = INT_MAX;  // interior pequeno: en cuanto se esta en el nivel

inline const char* nombreDeAmbito(AmbitoNivel a) {
    switch (a) {
        case AmbitoNivel::InteriorPequeno: return "interior_pequeno";
        case AmbitoNivel::InteriorGrande: return "interior_grande";
        case AmbitoNivel::Exterior: return "exterior";
        default: return "deducido";
    }
}

// false si el texto no es un ambito.
inline bool ambitoDeTexto(const std::string& texto, AmbitoNivel& fuera) {
    for (AmbitoNivel a : {AmbitoNivel::InteriorPequeno, AmbitoNivel::InteriorGrande, AmbitoNivel::Exterior})
        if (texto == nombreDeAmbito(a)) {
            fuera = a;
            return true;
        }
    return false;
}

// ¿Por su nombre (sin ruta ni «.json») es un interior?
inline bool nombreDeInterior(const std::string& nivel) {
    std::string n = nivel.substr(nivel.find_last_of('/') == std::string::npos ? 0 : nivel.find_last_of('/') + 1);
    if (n.size() > 5 && n.compare(n.size() - 5, 5, ".json") == 0) n.resize(n.size() - 5);
    for (const char* prefijo : {"interior_", "mansion_", "casa_", "tienda_", "dependencias_", "mina", "cueva", "galeria",
                                "camara_", "grieta", "santuario", "mazmorra", "antecova"})
        if (n.rfind(prefijo, 0) == 0) return true;
    if (n.rfind("aldea_", 0) == 0) {  // aldea_<sitio>_p<planta>: el interior de una casa
        const std::size_t p = n.rfind("_p");
        if (p != std::string::npos && p + 2 < n.size() && n.find_first_not_of("0123456789", p + 2) == std::string::npos)
            return true;
    }
    return false;
}

// El ambito de verdad: el declarado si lo hay; si no, el deducido del nombre y del tamano (ancho x alto).
inline AmbitoNivel ambitoDelNivel(AmbitoNivel declarado, const std::string& nivel, int ancho, int alto) {
    if (declarado != AmbitoNivel::Deducido) return declarado;
    if (!nombreDeInterior(nivel)) return AmbitoNivel::Exterior;
    return ancho * alto < kCasillasInteriorPequeno ? AmbitoNivel::InteriorPequeno : AmbitoNivel::InteriorGrande;
}

// El radio de Chebyshev de un ambito.
inline int radioDelAmbito(AmbitoNivel a) {
    return a == AmbitoNivel::InteriorPequeno ? kRadioAlEntrar : a == AmbitoNivel::InteriorGrande ? kRadioInteriorGrande : kRadioExterior;
}
