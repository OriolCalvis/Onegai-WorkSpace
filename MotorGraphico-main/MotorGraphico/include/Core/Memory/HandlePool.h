#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

// Pool de objetos con handles generacionales.
//
// Pensado para datos dinámicos de alta frecuencia (entidades de mundo,
// proyectiles, efectos o comandos), donde un vector de unique_ptr genera
// muchas reservas y un puntero crudo puede sobrevivir por error a un erase.
// El cliente conserva Handle, nunca un índice o puntero como identidad. Al
// borrar un objeto aumenta su generación: cualquier handle antiguo deja de
// resolver, incluso si su slot se reutiliza para otro objeto.
//
// No es un sustituto automático de std::vector: úsalo solo cuando se cree y
// destruya un conjunto variable de objetos durante la partida. Para datos
// estables y recorridos lineales, vector<T> sigue siendo la opción más
// rápida y simple.
template <typename T>
class HandlePool {
public:
    struct Handle {
        std::uint32_t index = kInvalidIndex;
        std::uint32_t generation = 0;

        bool operator==(const Handle& other) const {
            return index == other.index && generation == other.generation;
        }
        bool operator!=(const Handle& other) const { return !(*this == other); }
    };

    static constexpr std::uint32_t kInvalidIndex = std::numeric_limits<std::uint32_t>::max();

    HandlePool() = default;
    explicit HandlePool(std::size_t capacity) { reserve(capacity); }

    HandlePool(const HandlePool&) = delete;
    HandlePool& operator=(const HandlePool&) = delete;
    HandlePool(HandlePool&&) noexcept = default;
    HandlePool& operator=(HandlePool&&) noexcept = default;

    void reserve(std::size_t capacity) { m_slots.reserve(capacity); }
    std::size_t size() const { return m_size; }
    std::size_t capacity() const { return m_slots.capacity(); }
    bool empty() const { return m_size == 0; }

    template <typename... Args>
    Handle emplace(Args&&... args) {
        if (m_freeHead != kInvalidIndex) {
            const std::uint32_t index = m_freeHead;
            Slot& slot = m_slots[index];
            m_freeHead = slot.nextFree;
            slot.value.emplace(std::forward<Args>(args)...);
            ++m_size;
            return Handle{index, slot.generation};
        }

        // Se limita a uint32_t porque Handle se puede guardar, serializar y
        // pasar por el motor sin tamaños dependientes de plataforma.
        if (m_slots.size() >= static_cast<std::size_t>(kInvalidIndex)) {
            return Handle{};
        }
        const std::uint32_t index = static_cast<std::uint32_t>(m_slots.size());
        Slot slot;
        slot.value.emplace(std::forward<Args>(args)...);
        m_slots.push_back(std::move(slot));
        ++m_size;
        return Handle{index, m_slots.back().generation};
    }

    T* get(Handle handle) {
        Slot* slot = findSlot(handle);
        return slot != nullptr ? &*slot->value : nullptr;
    }

    const T* get(Handle handle) const {
        const Slot* slot = findSlot(handle);
        return slot != nullptr ? &*slot->value : nullptr;
    }

    bool contains(Handle handle) const { return findSlot(handle) != nullptr; }

    bool erase(Handle handle) {
        Slot* slot = findSlot(handle);
        if (slot == nullptr) {
            return false;
        }
        slot->value.reset();
        ++slot->generation;
        // La generación 0 se reserva para el Handle por defecto inválido.
        if (slot->generation == 0) {
            ++slot->generation;
        }
        slot->nextFree = m_freeHead;
        m_freeHead = handle.index;
        --m_size;
        return true;
    }

    void clear() {
        for (std::size_t index = 0; index < m_slots.size(); ++index) {
            Slot& slot = m_slots[index];
            if (slot.value.has_value()) {
                slot.value.reset();
                ++slot.generation;
                if (slot.generation == 0) {
                    ++slot.generation;
                }
            }
            slot.nextFree = index == 0 ? kInvalidIndex : static_cast<std::uint32_t>(index - 1);
        }
        m_freeHead =
            m_slots.empty() ? kInvalidIndex : static_cast<std::uint32_t>(m_slots.size() - 1);
        m_size = 0;
    }

private:
    struct Slot {
        std::optional<T> value;
        std::uint32_t generation = 1;
        std::uint32_t nextFree = kInvalidIndex;
    };

    Slot* findSlot(Handle handle) {
        if (handle.index >= m_slots.size()) {
            return nullptr;
        }
        Slot& slot = m_slots[handle.index];
        return slot.value.has_value() && slot.generation == handle.generation ? &slot : nullptr;
    }

    const Slot* findSlot(Handle handle) const {
        if (handle.index >= m_slots.size()) {
            return nullptr;
        }
        const Slot& slot = m_slots[handle.index];
        return slot.value.has_value() && slot.generation == handle.generation ? &slot : nullptr;
    }

    std::vector<Slot> m_slots;
    std::uint32_t m_freeHead = kInvalidIndex;
    std::size_t m_size = 0;
};
