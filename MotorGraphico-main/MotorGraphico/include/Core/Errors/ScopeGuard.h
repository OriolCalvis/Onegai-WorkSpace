#pragma once

#include <type_traits>
#include <utility>

// EST-11 - El `finally` de C++, que es RAII: un destructor.
//
// Por que existe: la norma N4 del repo dice que un fallo esperable se DEVUELVE
// como Result<T> y que las excepciones no cruzan la frontera de un modulo. Pero
// un retorno temprano con Result<T> en la mano deja a medias lo que ya se habia
// hecho: el directorio creado, el recurso abierto, el modo cambiado. En otro
// lenguaje eso se escribe con `finally`; aqui se escribe con un objeto que vive
// hasta el final del ambito y limpia al destruirse, salga como salga la funcion.
//
// Contrato, leido de arriba abajo:
//   1. La accion NO debe lanzar. Se invoca desde un destructor: si lanza
//      mientras se desenvuelve otra excepcion, el proceso termina. Si la
//      limpieza puede fallar, que registre el fallo (Core/Log) y siga; una
//      limpieza que falla no arregla nada reventando encima.
//   2. No se copia: dos guardas ejecutarian la accion dos veces. Si se mueve,
//      la de origen queda desarmada y la accion ocurre exactamente una vez.
//   3. release() desarma. Es lo que se llama cuando el ambito termina bien y
//      la limpieza ya no toca (el recurso pasa a ser del llamador).
//
// Solo cabecera, a proposito: es una plantilla sobre el callable y no tiene
// estado que ocultar, igual que MemorySink en Core/Log.
template <typename Action>
class ScopeGuard {
public:
    explicit ScopeGuard(Action action) : m_action(std::move(action)) {}

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

    ScopeGuard(ScopeGuard&& other) noexcept
        : m_action(std::move(other.m_action)), m_armed(other.m_armed) {
        other.m_armed = false;
    }
    ScopeGuard& operator=(ScopeGuard&&) = delete;

    ~ScopeGuard() {
        if (m_armed) {
            m_action();
        }
    }

    void release() { m_armed = false; }
    bool armed() const { return m_armed; }

private:
    Action m_action;
    bool m_armed = true;
};

// Fabrica: deduce el tipo del callable para no tener que escribirlo. Se usa con
// llaves para que no se pueda confundir con la declaracion de una funcion.
template <typename Action>
ScopeGuard<std::decay_t<Action>> makeScopeGuard(Action&& action) {
    return ScopeGuard<std::decay_t<Action>>{std::decay_t<Action>(std::forward<Action>(action))};
}
