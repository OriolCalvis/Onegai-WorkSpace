// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   G-17  (fase G, 7 j) - Idioma seleccionable de la interfície
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <map>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"

// G-17 — Idioma seleccionable de la interfaz. NUCLEO, SIN PANTALLA.
//
// QUE RESUELVE. DEC-14: la interfaz tiene que poder estar en ingles,
// catalan o castellano, y admitir mas idiomas despues. Hoy los textos son
// literales dentro del codigo, y ni siquiera en un idioma: el log de
// combate de src/Game/BattleState.cpp dice «huye del combate» y «de daño»
// en castellano, y tres lineas mas abajo «-> efecte rebutjat» en catalan,
// con las etiquetas de grado «Fracàs / Parcial / Èxit / Crític». No es un
// arreglo de etiquetas: es que no hay ningun sitio donde poner el texto.
//
// EL PROBLEMA NO ES LA TABLA, ES LA CONCATENACION. Cualquiera puede hacer
// un mapa de clave a texto. Lo que hace intraducible el motor de hoy es
// que las frases se construyen sumando trozos:
//
//     actor.name + " ataca a " + target.name + " (" + grado + ": " + n + " de daño)."
//
// Ese orden es castellano. En ingles el complemento va donde va, y en
// catalan la preposicion cambia. Una tabla de trozos sueltos no se puede
// traducir: hay que guardar la FRASE ENTERA con huecos, y que cada idioma
// decida donde caen. Por eso el texto se pide con format() y parametros
// con nombre, no con orden.
//
//     format("combate.ataca", {{"actor", ...}, {"objetivo", ...},
//                              {"grado", ...}, {"dano", ...}})
//
// LO QUE SE COMPRUEBA SOLO. Tres cosas que en cualquier proyecto
// traducido se rompen calladamente:
//
//   · Una clave que falta en un idioma. Cae al idioma de referencia, pero
//     el resultado lo DICE (fromFallback) en vez de enseñar ingles en
//     medio de una pantalla catalana como si nada.
//   · Un hueco que se llama distinto en un idioma que en otro —{dano} en
//     castellano y {damage} en ingles— . checkPlaceholders() lo encuentra
//     sin ejecutar nada: es el fallo clasico y el mas dificil de ver.
//   · Un parametro que se pasa y no se usa, o que se usa y no se pasa. El
//     texto sale con el hueco a la vista y el resultado lo dice, en vez de
//     dejar un blanco donde iba el numero.
//
// DEC-15 — el texto visible lleva ortografia correcta, con acentos y ñ;
// los identificadores internos no. isValidKey() lo hace comprobable: una
// clave es [a-z0-9_.], y cargar una tabla con «grado.èxit» falla. La
// clave es el contrato entre el codigo y las tres tablas; si lleva
// acentos, el dia que alguien la escriba mal el texto desaparece sin
// error.
//
// PENDIENTE DE ORIOL (DEC-14), Y POR QUE NO BLOQUEA. Quedan dos
// preguntas: cual es el idioma de REFERENCIA, y si los grados de combate
// conservan el nombre propio ONEGAI en los tres idiomas o se traducen.
// Ninguna de las dos se decide en codigo: la referencia la dice el
// manifiesto de assets/i18n/, y los grados son cuatro entradas de tabla.
// Contestarlas es cambiar datos, no recompilar.
//
// LO QUE NO ESTA AQUI. El selector vive en Opciones (G-10) y la
// comprobacion de que la fuente tiene glifos para «à» y «ñ» necesita el
// render (G-14/E-10). Esto es la mitad sin pantalla: la tabla, las
// comprobaciones y el cambio en caliente.

namespace Engine {

// Resultado de pedir un texto. Nunca es solo una cadena: el que pinta
// necesita saber si lo que le dan es lo que pidio.
struct LocalizedText {
    std::string text;
    // La clave no estaba en el idioma activo y se ha servido la del
    // idioma de referencia.
    bool fromFallback = false;
    // No estaba en ninguno: `text` lleva la propia clave entre corchetes,
    // que es feo a proposito. Un texto que falta tiene que verse.
    bool missing = false;
    // Huecos de la plantilla para los que no se ha pasado valor. El hueco
    // se queda a la vista en `text`.
    std::vector<std::string> missingArgs;
    // Valores pasados que la plantilla no usa. Casi siempre es un hueco
    // renombrado en un idioma y no en otro.
    std::vector<std::string> unusedArgs;

    bool ok() const { return !missing && missingArgs.empty() && unusedArgs.empty(); }
};

// Una clave con plantillas que no encajan entre idiomas.
struct PlaceholderMismatch {
    std::string key;
    std::string language;
    // Huecos que tiene la referencia y no este idioma, y al reves.
    std::vector<std::string> missing;
    std::vector<std::string> extra;
};

class Locale {
public:
    // Nombre del manifiesto dentro de la carpeta de idiomas.
    static constexpr const char* kManifestFile = "manifiesto.json";

    Locale() = default;

    // Carga una carpeta tipo assets/i18n/: el manifiesto y un .json por
    // idioma. Todo o nada — si una tabla esta mal, no se carga ninguna:
    // media carga deja la interfaz en dos idiomas a la vez.
    Result<bool> loadFromDirectory(const std::string& directory);

    // La misma carga con las tablas ya en memoria. Es lo que usan las
    // pruebas y lo que usara el editor para previsualizar un idioma que
    // el autor esta escribiendo y aun no ha guardado.
    Result<bool> loadFromStrings(const std::string& manifestJson,
                                 const std::map<std::string, std::string>& tablesJson);

    bool loaded() const { return !m_reference.empty(); }
    const std::string& referenceLanguage() const { return m_reference; }
    const std::vector<std::string>& languages() const { return m_languages; }
    const std::string& language() const { return m_active; }

    // En caliente: no reinicia nada ni recarga los ficheros. Error si el
    // idioma no esta en el manifiesto.
    Result<bool> setLanguage(const std::string& code);

    LocalizedText text(const std::string& key) const;
    LocalizedText format(const std::string& key,
                         const std::map<std::string, std::string>& args) const;

    // Claves de la referencia que faltan en `code`. Vacio = ese idioma
    // esta completo.
    std::vector<std::string> missingKeys(const std::string& code) const;
    // Claves que tiene `code` y no tiene la referencia. Normalmente una
    // clave renombrada de la que alguien se olvido en un idioma.
    std::vector<std::string> orphanKeys(const std::string& code) const;
    // Claves cuyos huecos no coinciden con los de la referencia.
    std::vector<PlaceholderMismatch> checkPlaceholders() const;

    // DEC-15 — identificadores internos sin acentos ni mayusculas:
    // [a-z0-9_.], sin punto al principio ni al final ni dos seguidos.
    static bool isValidKey(const std::string& key);
    // Huecos {asi} de una plantilla, en orden y sin repetir. Publica
    // porque el editor de textos de G-10 tiene que enseñarlos al que
    // traduce: traducir a ciegas es como se pierde un {dano}.
    static std::vector<std::string> placeholdersOf(const std::string& tmpl);

private:
    const std::map<std::string, std::string>* table(const std::string& code) const;
    Result<bool> parseTable(const std::string& code, const std::string& json,
                            std::map<std::string, std::string>& out) const;

    std::string m_reference;
    std::string m_active;
    std::vector<std::string> m_languages;
    std::map<std::string, std::map<std::string, std::string>> m_tables;
};

}  // namespace Engine
