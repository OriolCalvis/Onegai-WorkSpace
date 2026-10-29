#pragma once
// AV15-02 (D0015 §14, Oriol: «los sitios tienen que llamarse por su nombre, por encima de su función»)
// — Los sitios de la ciudad por su nombre: assets/catalogs/lugares.json (ChatGPT, AV15-02b; tabla
// aprobada por Oriol, PO-39 a PO-41). Cada interior al que lleva una puerta: su nombre propio
// («La Boutique de Madame»), su función en dos o tres palabras («sastre y tela»), su emblema (un id
// de iconos.json, o vacío) y quién lo lleva.
//
// Lo usan el cartel sobre la puerta y el aviso «Entrar en …», el minimapa y el rótulo al entrar.
// GL-free; pruebas en tests/Game/LugaresTests.cpp.

#include <string>
#include <vector>

#include "Core/Errors/Result.h"

namespace Game {

struct Lugar {
    std::string nivel;    // "assets/levels/interior_sastreria.json"
    std::string nombre;   // "La Boutique de Madame"
    std::string funcion;  // "sastre y tela"
    std::string emblema;  // id de iconos.json; "" sin dibujo todavía
    std::string quien;    // "Luisarda"; "" si no consta
    // El icono que se pinta: su emblema o, mientras no lo tenga, el genérico «lugar».
    const std::string& icono() const {
        static const std::string kGenerico = "lugar";
        return emblema.empty() ? kGenerico : emblema;
    }
};

// AV16-06 — A donde lleva una SALIDA (la puerta de dentro que da a fuera): la calle, la plaza, el
// mapamundi. «salir» es lo que sigue a «Salir a» en el aviso («la calle», «la plaza»); «nombre» es
// lo que dice el cartel «Hacia …» y el mapa.
struct Calle {
    std::string nivel;
    std::string nombre;  // "el Casco Antiguo"
    std::string salir;   // "la calle"
};

// El aviso de una puerta: «Entrar en La Boutique de Madame», «Salir a la calle».
struct TextoPuerta {
    std::string verbo, destino;
};

class Lugares {
public:
    static Result<Lugares> desdeTexto(const std::string& json);
    static Result<Lugares> carga(const std::string& ruta);
    // Los del juego (assets/catalogs/lugares.json, leído la primera vez). Sin fichero, ninguno.
    static const Lugares& delJuego();

    // El sitio al que lleva `nivel` (vale una ruta con o sin carpeta delante); nullptr si no es
    // ninguno de la lista.
    const Lugar* de(const std::string& nivel) const;
    const std::vector<Lugar>& todos() const { return m_lugares; }

    // AV16-06 — Las salidas. Una puerta es SALIDA si su objeto se llama «salida_…».
    static bool esSalida(const std::string& objectId);
    const Calle* calle(const std::string& nivel) const;
    const std::vector<Calle>& calles() const { return m_calles; }
    // El nombre de lo que hay al otro lado (un interior con nombre propio o una calle/plaza); "" si no consta.
    std::string nombreDeDestino(const std::string& nivel) const;
    // El aviso de la puerta: una salida nunca dice «Entrar en Salida», dice «Salir a la calle».
    // `nombreObjeto` es el del objeto, para una puerta sin lugar con nombre.
    TextoPuerta textoDePuerta(const std::string& objectId, const std::string& nivelDestino,
                              const std::string& nombreObjeto) const;

private:
    std::vector<Lugar> m_lugares;
    std::vector<Calle> m_calles;
};

}  // namespace Game
