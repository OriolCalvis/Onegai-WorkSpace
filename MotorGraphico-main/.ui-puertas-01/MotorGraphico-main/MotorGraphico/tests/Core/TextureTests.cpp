#include <glad/glad.h>

#include "Core/Resources/Texture.h"

#include "TestHarness.h"

#include <utility>
#include <vector>

namespace {

// C-09 — Doble GL falso: el test no abre contexto; redirige el puntero de
// glad para glDeleteTextures y registra las liberaciones. Es lo que permite
// verificar la propiedad del recurso (quien borra que, y cuantas veces).
std::vector<unsigned int> deletedNames;

void APIENTRY fakeDeleteTextures(GLsizei n, const GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        deletedNames.push_back(names[i]);
    }
}

MOTOR_TEST_CASE(move_assignment_releases_the_previous_resource) {
    glad_glDeleteTextures = &fakeDeleteTextures;
    deletedNames.clear();
    {
        Texture destination(5, 4, 4);
        Texture source(9, 8, 8);

        destination = std::move(source);

        MOTOR_CHECK(deletedNames.size() == 1);
        MOTOR_CHECK(deletedNames[0] == 5);
        MOTOR_CHECK(destination.getWidth() == 8);
        MOTOR_CHECK(destination.getHeight() == 8);
    }
    // Al salir del ambito el destructor libera el 9; el movido-desde (id 0), nada.
    MOTOR_CHECK(deletedNames.size() == 2);
    MOTOR_CHECK(deletedNames[1] == 9);
}

MOTOR_TEST_CASE(move_constructor_transfers_ownership_without_duplicating) {
    glad_glDeleteTextures = &fakeDeleteTextures;
    deletedNames.clear();
    {
        Texture source(7, 2, 2);
        Texture destination(std::move(source));
        MOTOR_CHECK(destination.getWidth() == 2);
    }
    MOTOR_CHECK(deletedNames.size() == 1);
    MOTOR_CHECK(deletedNames[0] == 7);
}

MOTOR_TEST_CASE(assigning_into_an_empty_texture_deletes_nothing) {
    glad_glDeleteTextures = &fakeDeleteTextures;
    deletedNames.clear();
    {
        Texture destination(0, 0, 0);
        Texture source(9, 8, 8);
        destination = std::move(source);
        MOTOR_CHECK(deletedNames.empty());
    }
    MOTOR_CHECK(deletedNames.size() == 1);
    MOTOR_CHECK(deletedNames[0] == 9);
}

MOTOR_TEST_CASE(destructor_releases_the_name_exactly_once) {
    glad_glDeleteTextures = &fakeDeleteTextures;
    deletedNames.clear();
    { Texture texture(3, 1, 1); }
    MOTOR_CHECK(deletedNames.size() == 1);
    MOTOR_CHECK(deletedNames[0] == 3);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
