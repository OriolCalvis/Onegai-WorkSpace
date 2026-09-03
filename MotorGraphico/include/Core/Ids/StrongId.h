#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <utility>

#include "Core/Errors/Result.h"

namespace ids {

inline bool isValidValue(std::string_view value) {
    if (value.empty()) {
        return false;
    }
    for (const char character : value) {
        if (!((character >= 'a' && character <= 'z') ||
              (character >= '0' && character <= '9') || character == '_')) {
            return false;
        }
    }
    return true;
}

// Identificador semántico sin conversiones implícitas entre dominios. La
// validación centraliza el contrato [a-z0-9_]+ que también siguen nombres de
// ficheros y catálogos, y evita que un AssetId acabe en una API que pide MapId.
template <class Tag>
class StrongId {
public:
    StrongId() = default;
    static constexpr const char* kNamespace = Tag::kNamespace;

    static Result<StrongId> parse(std::string value) {
        if (!isValidValue(value)) {
            return Result<StrongId>::Error(std::string(Tag::kNamespace) +
                                           " id inválido: se espera [a-z0-9_]+");
        }
        return Result<StrongId>::Ok(StrongId(std::move(value)));
    }

    bool empty() const { return m_value.empty(); }
    const std::string& value() const { return m_value; }

    friend bool operator==(const StrongId& left, const StrongId& right) {
        return left.m_value == right.m_value;
    }
    friend bool operator!=(const StrongId& left, const StrongId& right) {
        return !(left == right);
    }
    friend bool operator<(const StrongId& left, const StrongId& right) {
        return left.m_value < right.m_value;
    }

private:
    explicit StrongId(std::string value) : m_value(std::move(value)) {}

    std::string m_value;
};

// Fuente inyectable: los tests pueden generar una secuencia fija y las
// herramientas de autoría pueden decidir su estrategia sin acoplar los
// documentos a UUID, reloj del sistema o aleatoriedad global.
class IdGenerator {
public:
    virtual ~IdGenerator() = default;
    virtual std::string next(std::string_view idNamespace) = 0;
};

template <class Id>
Result<Id> generate(IdGenerator& generator) {
    return Id::parse(generator.next(Id::kNamespace));
}

}  // namespace ids

namespace std {

template <class Tag>
struct hash<ids::StrongId<Tag>> {
    std::size_t operator()(const ids::StrongId<Tag>& id) const noexcept {
        return std::hash<std::string>{}(id.value());
    }
};

}  // namespace std
