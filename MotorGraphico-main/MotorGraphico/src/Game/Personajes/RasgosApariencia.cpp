// El valor propio no es un complemento: recupera la capa de nacimiento de la raza.
#include "Game/Personajes/RasgosApariencia.h"
#include <algorithm>
namespace Game {
namespace {
constexpr const char* kPropio = "__raza__";
bool contiene(const std::vector<std::string>& v, const std::string& s) {
    return std::find(v.begin(), v.end(), s) != v.end();
}
}
void RasgosApariencia::migrarTallaAnterior(Apariencia& a) {
    if (a.talla == "grande" && (a.raza == "minotauros_de_la_llanura" || a.raza == "minotauros_del_laberinto" || a.raza == "aeronautas_de_esmua")) a.talla = "minotauro";
}
bool RasgosApariencia::sustituido(const Apariencia& a, const std::string& ranura) {
    const auto it = a.ropa.find(ranura);
    return esRasgo(ranura) && it != a.ropa.end() && !it->second.empty();
}
bool RasgosApariencia::esRasgo(const std::string& r) { return r == "alas" || r == "cola" || r == "cuernos"; }
bool RasgosApariencia::natural(const RazaVisual* r, const std::string& s) { return r && contiene(r->capas, s); }
std::vector<EditorApariencia::Fila> RasgosApariencia::filas(const ManifiestoPersonajes& m,
                                                        const RazaVisual* r, const std::string& talla) {
    using F = EditorApariencia::Fila;
    std::vector<F> out;
    for (const auto& [fila, ranura] : {std::pair{F::Alas, "alas"}, {F::Cola, "cola"}, {F::Cuernos, "cuernos"}}) {
        const auto g = m.equipo.find(ranura);
        const bool alternativas = g != m.equipo.end() && !g->second.filas.empty() && g->second.atlas.count(talla);
        if ((!r || r->admite(ranura)) && (natural(r, ranura) || alternativas)) out.push_back(fila);
    }
    return out;
}
std::vector<std::string> RasgosApariencia::opciones(const RazaVisual* r, const std::string& s) {
    return natural(r, s) ? std::vector<std::string>{kPropio, ""} : std::vector<std::string>{""};
}
std::string RasgosApariencia::actual(const Apariencia& a, const RazaVisual* r, const std::string& s) {
    const auto it = a.ropa.find(s);
    if (it != a.ropa.end() && !it->second.empty()) return it->second;
    return natural(r, s) && !contiene(a.quitarCapas, s) ? kPropio : "";
}
void RasgosApariencia::poner(Apariencia& a, const RazaVisual* r, const std::string& s, const std::string& v) {
    auto& quitar = a.quitarCapas;
    quitar.erase(std::remove(quitar.begin(), quitar.end(), s), quitar.end());
    if (natural(r, s) && v != kPropio) quitar.push_back(s);
    if (v.empty() || v == kPropio) a.ropa.erase(s);
    else a.ropa[s] = v;
}
}
