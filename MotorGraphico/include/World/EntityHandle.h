#pragma once

#include <cstdint>
#include <limits>

namespace Engine {

// Identidad efímera de una entidad viva en World. El índice permite una
// búsqueda compacta; la generación impide que un handle de una entidad
// destruida apunte por accidente a otra que reutilizó su slot.
struct EntityHandle {
    static constexpr std::uint32_t kInvalidIndex = std::numeric_limits<std::uint32_t>::max();

    std::uint32_t index = kInvalidIndex;
    std::uint32_t generation = 0;

    bool valid() const { return index != kInvalidIndex && generation != 0; }

    friend bool operator==(EntityHandle left, EntityHandle right) {
        return left.index == right.index && left.generation == right.generation;
    }
    friend bool operator!=(EntityHandle left, EntityHandle right) { return !(left == right); }
};

}  // namespace Engine
