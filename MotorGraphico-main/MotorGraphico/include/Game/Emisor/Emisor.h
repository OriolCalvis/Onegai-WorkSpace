// U4-07.E05 — Un emisor con estado, el presupuesto global de particulas y `avanzarEmisor`. GL-free.
//
// Orden dentro de un paso (ver tambien el contrato U4-07 seccion E05 y 4):
//   dt valido -> recorte a kMaxPasoEmision -> dtEmision = min(dt, duracion - tiempo) -> tiempo += dt ->
//   envejecer, recolocar (forma cerrada) y compactar (LIBERA presupuesto) ->
//   si cortar y tiempo >= duracion: vaciar (libera) y no nacer nada; si no, nacer `emisiones(dtEmision,
//   tasa*escalaTasa)` en orden. Cada nacimiento reserva presupuesto ANTES de sortear (rechazo = drop-new).
// El presupuesto es DERIVADO: `recuenta` lo reconstruye desde los buffers (fuente de verdad, H2).
// `Particula::k` es uint16: el contador de nacimientos del emisor es de 64 bits y k = contador mod 65536
// (modulo explicito; k solo es la FASE del balanceo sin(3t+k) / sin(5t+k), la discontinuidad al dar la vuelta
// afecta a una particula cada 65536 y es determinista).
#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/Vector2.h"
#include "Game/Emisor/BufferParticulas.h"
#include "Game/Emisor/DefinicionEmisor.h"
#include "Game/Emisor/EmisionConResiduo.h"
#include "Game/Emisor/Particula.h"

namespace Game::Emisor {

// Modulo EXPLICITO (no depende de que el cast a uint16 ya trunque): k = nacidas mod kModuloK.
constexpr std::uint64_t kModuloK = 65536;

struct GrupoDeDibujo {          // DUENO: Emisor.h. El espacio al que pertenecen las coordenadas del emisor
    Vector2 esquina{0.0f, 0.0f};   // px de mundo de la esquina del dibujo
    Vector2 tam{64.0f, 64.0f};     // px de mundo del dibujo; 64x64 = 1 px de frame por px de mundo
};

struct PasoParticulas;
struct PresupuestoParticulas;

// Movible y asignable por movimiento (poda con remove_if+erase); NO copiable (el generador es unico).
class Emisor {
public:
    explicit Emisor(const DefinicionEmisor& d, Vector2 origen = {}, GrupoDeDibujo grupo = {});   // tipo = tipoDeParticula(familia)
    Emisor(const DefinicionEmisor& d, const TipoDeParticula& tipo, Vector2 origen, GrupoDeDibujo grupo);
    Emisor(Emisor&&) noexcept = default;
    Emisor& operator=(Emisor&&) noexcept = default;
    Emisor(const Emisor&) = delete;
    Emisor& operator=(const Emisor&) = delete;

    const DefinicionEmisor& definicion() const { return m_def; }
    const TipoDeParticula& tipo() const { return m_tipo; }
    const BufferParticulas& buffer() const { return m_buffer; }
    double tiempo() const { return m_tiempo; }
    std::uint64_t nacidas() const { return m_nacidas; }
    // Emision terminada (duracion > 0 y alcanzada) y buffer vacio: se puede podar.
    bool acabado() const;

    Vector2 origen;            // px del espacio del grupo
    GrupoDeDibujo grupo;

private:
    friend Result<PasoParticulas> avanzarEmisor(Emisor& e, double dt, double escalaTasa, PresupuestoParticulas& p);
    DefinicionEmisor m_def;
    TipoDeParticula m_tipo;
    std::unique_ptr<RPG::Xoroshiro128p> m_azar;
    EstadoEmision m_estado;
    BufferParticulas m_buffer;
    double m_tiempo = 0.0;
    std::uint64_t m_nacidas = 0;
};

struct PresupuestoParticulas {    // UNICO por adaptador; lo comparten todos sus emisores
    std::size_t maximo = static_cast<std::size_t>(kMaxParticulasGlobal);
    std::size_t vivas = 0;
    bool reserva();                                   // vivas<maximo ? ++vivas,true : false
    void libera(std::size_t n);                       // sin bajar de 0
    void recuenta(const std::vector<Emisor>& e);      // vivas = suma de buffer.vivas(): la FUENTE DE VERDAD
};

struct PasoParticulas {
    int nacidas = 0;
    int caducadas = 0;      // las que salen del buffer en el paso: por edad o vaciadas por el corte
    int rechazadas = 0;     // nacimientos sin hueco (buffer lleno o presupuesto agotado)
    bool recortado = false; // dt > kMaxPasoEmision: se uso kMaxPasoEmision
};

// dt en s, escalaTasa >= 0 (D-O3: 0,25 con movimiento reducido). Error (emisor intacto): dt o escalaTasa
// NaN/inf/<0, o tasa*escalaTasa > kMaxTasa. dt == 0: sin cambios.
Result<PasoParticulas> avanzarEmisor(Emisor& e, double dt, double escalaTasa, PresupuestoParticulas& p);

// Poda de los emisores `acabado()` (erase estable) y recuenta. Devuelve cuantos retiro.
std::size_t podaAcabados(std::vector<Emisor>& emisores, PresupuestoParticulas& p);
// Olvida todos los emisores (cambio de mapa, cierre) y recuenta: el presupuesto queda en 0.
void olvida(std::vector<Emisor>& emisores, PresupuestoParticulas& p);

}  // namespace Game::Emisor
