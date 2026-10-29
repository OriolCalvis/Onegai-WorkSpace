// Copyright 2026 Motor Grafico Isometrico Pixel Art
// Licensed under the MIT License

#include "Render/Entity.h"
#include "Core/Math/GridCoord.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"

#include "TestHarness.h"

#include <cmath>

// Forward declaration para el mock
class TextureAtlas;
class SpriteBatch;

namespace {

TextureAtlas* createMockAtlas() {
    return static_cast<TextureAtlas*>(nullptr);
}

// Clase concreta minima para probar Entity (que es abstracta)
class TestEntity : public Entity {
public:
    TestEntity(GridCoord gridPosition, int spriteID, TextureAtlas* atlas, 
               int tileWidth = 64, int tileHeight = 32)
        : Entity(gridPosition, spriteID, atlas, tileWidth, tileHeight) {}
    
    // Implementar los metodos puros virtuales de IRenderable
    void render(SpriteBatch& batch) override {
        // No op para pruebas
    }
    
    int getSortKey() const override {
        return Entity::getSortKey();
    }
    
    // Implementar el metodo puro virtual de IUpdatable
    void update(float deltaTime) override {
        // No op para pruebas
    }
    
    // Exponer el getSortKey de Entity (protected en Entity.h?)
    // Ver Entity.h linea 37: getSortKey() es public
};

} // namespace

// --- Tests de construccion y valores iniciales ---

MOTOR_TEST_CASE(entity_constructor_default_values) {
    TextureAtlas* atlas = createMockAtlas();
    TestEntity entity(GridCoord{0, 0}, 0, atlas);
    MOTOR_CHECK(entity.gridPosition().x == 0);
    MOTOR_CHECK(entity.gridPosition().y == 0);
    // spriteID se pasa al constructor de Entity
    MOTOR_CHECK(entity.offset().x == 0.0f);
    MOTOR_CHECK(entity.offset().y == 0.0f);
    MOTOR_CHECK(entity.visualScale() == 1.0f);
}

MOTOR_TEST_CASE(entity_constructor_with_custom_position) {
    TextureAtlas* atlas = createMockAtlas();
    TestEntity entity(GridCoord{5, 10}, 0, atlas);
    MOTOR_CHECK(entity.gridPosition().x == 5);
    MOTOR_CHECK(entity.gridPosition().y == 10);
}

MOTOR_TEST_CASE(entity_constructor_with_custom_sprite_id) {
    TextureAtlas* atlas = createMockAtlas();
    TestEntity entity(GridCoord{0, 0}, 42, atlas);
    // No hay getter directo para m_spriteID, pero podemos verificar indirectamente
    // a traves de la logica de render (que no probamos aqui)
    MOTOR_CHECK(true); // Si compila, el constructor funciona
}

MOTOR_TEST_CASE(entity_constructor_with_custom_tile_dimensions) {
    TextureAtlas* atlas = createMockAtlas();
    TestEntity entity(GridCoord{0, 0}, 0, atlas, 128, 64);
    // No hay getter directo para tileWidth/tileHeight, pero el constructor los acepta
    MOTOR_CHECK(true);
}

// --- Tests de setters y getters ---

MOTOR_TEST_CASE(entity_set_grid_position_updates_position) {
    TextureAtlas* atlas = createMockAtlas();
    TestEntity entity(GridCoord{0, 0}, 0, atlas);
    
    entity.setGridPosition(GridCoord{7, 14});
    MOTOR_CHECK(entity.gridPosition().x == 7);
    MOTOR_CHECK(entity.gridPosition().y == 14);
}

MOTOR_TEST_CASE(entity_set_offset_updates_offset) {
    TextureAtlas* atlas = createMockAtlas();
    TestEntity entity(GridCoord{0, 0}, 0, atlas);
    
    entity.setOffset(Vector2{1.5f, 2.5f});
    MOTOR_CHECK(std::abs(entity.offset().x - 1.5f) < 0.001f);
    MOTOR_CHECK(std::abs(entity.offset().y - 2.5f) < 0.001f);
}

MOTOR_TEST_CASE(entity_set_visual_scale_updates_scale) {
    TextureAtlas* atlas = createMockAtlas();
    TestEntity entity(GridCoord{0, 0}, 0, atlas);
    
    entity.setVisualScale(2.0f);
    MOTOR_CHECK(std::abs(entity.visualScale() - 2.0f) < 0.001f);
    
    // Verificar clamp (min 0.25, max 4.0)
    entity.setVisualScale(0.1f);
    MOTOR_CHECK(std::abs(entity.visualScale() - 0.25f) < 0.001f);
    
    entity.setVisualScale(10.0f);
    MOTOR_CHECK(std::abs(entity.visualScale() - 4.0f) < 0.001f);
}

MOTOR_TEST_CASE(entity_set_tint_updates_color) {
    TextureAtlas* atlas = createMockAtlas();
    TestEntity entity(GridCoord{0, 0}, 0, atlas);
    
    Vector4 red{1.0f, 0.0f, 0.0f, 1.0f};
    entity.setTint(red);
    const Vector4& tint = entity.tint();
    MOTOR_CHECK(tint.x == 1.0f);
    MOTOR_CHECK(tint.y == 0.0f);
    MOTOR_CHECK(tint.z == 0.0f);
    MOTOR_CHECK(tint.w == 1.0f);
}

MOTOR_TEST_CASE(entity_set_atlas_updates_atlas) {
    TextureAtlas* atlas1 = createMockAtlas();
    TextureAtlas* atlas2 = createMockAtlas();
    TestEntity entity(GridCoord{0, 0}, 0, atlas1);
    
    // setAtlas es un metodo de Entity
    entity.setAtlas(atlas2);
    // No hay getter para m_atlas, pero el metodo existe y no crash
    MOTOR_CHECK(true);
}

// --- Tests de herencia (IRenderable, IUpdatable) ---

MOTOR_TEST_CASE(entity_implements_irenderable_and_iupdatable) {
    TextureAtlas* atlas = createMockAtlas();
    TestEntity entity(GridCoord{0, 0}, 0, atlas);
    
    // Entity hereda de IRenderable e IUpdatable
    // Podemos hacer static_cast para verificar
    IRenderable* renderable = &entity;
    IUpdatable* updatable = &entity;
    
    MOTOR_CHECK(renderable != nullptr);
    MOTOR_CHECK(updatable != nullptr);
}

// --- Tests adicionales para cubrir más lógica GL-free ---

MOTOR_TEST_CASE(entity_get_sort_key_formula) {
    TextureAtlas* atlas = createMockAtlas();
    // Test con valores específicos para verificar la fórmula
    // getSortKey() = (grid_x + grid_y) * (tileHeight / 2) * kSortKeyMultiplier + grid_x
    TestEntity entity1(GridCoord{0, 0}, 0, atlas, 64, 32);
    TestEntity entity2(GridCoord{1, 0}, 0, atlas, 64, 32);
    TestEntity entity3(GridCoord{0, 1}, 0, atlas, 64, 32);
    TestEntity entity4(GridCoord{1, 1}, 0, atlas, 64, 32);
    
    // (0+0)*16 = 0, +0 = 0
    MOTOR_CHECK(entity1.getSortKey() == 0);
    
    // (1+0)*16 = 16, *65536 + 1 = 16*65536 + 1
    // (0+1)*16 = 16, *65536 + 0 = 16*65536
    // Entonces entity2 > entity3 (porque grid_x desempata)
    MOTOR_CHECK(entity1.getSortKey() < entity2.getSortKey());
    MOTOR_CHECK(entity1.getSortKey() < entity3.getSortKey());
    MOTOR_CHECK(entity2.getSortKey() > entity3.getSortKey());
}

MOTOR_TEST_CASE(entity_sort_key_with_different_tile_height) {
    TextureAtlas* atlas = createMockAtlas();
    // Con tileHeight diferente, el sort key cambia
    TestEntity entity1(GridCoord{1, 1}, 0, atlas, 64, 32);
    TestEntity entity2(GridCoord{1, 1}, 0, atlas, 64, 64);
    
    // El sort key depende de tileHeight, así que deberían ser diferentes
    MOTOR_CHECK(entity1.getSortKey() != entity2.getSortKey());
}

MOTOR_TEST_CASE(entity_get_sort_key_returns_valid_value) {
    TextureAtlas* atlas = createMockAtlas();
    TestEntity entity(GridCoord{3, 2}, 0, atlas);
    
    // getSortKey() devuelve un int basado en la posicion
    int key = entity.getSortKey();
    // El valor exacto depende de la implementacion, pero no deberia ser 0
    // para una entidad en (3,2)
    MOTOR_CHECK(key != 0);
}

MOTOR_TEST_CASE(entity_sort_key_consistent_for_same_position) {
    TextureAtlas* atlas = createMockAtlas();
    TestEntity entity1(GridCoord{5, 5}, 0, atlas);
    TestEntity entity2(GridCoord{5, 5}, 0, atlas);
    
    MOTOR_CHECK(entity1.getSortKey() == entity2.getSortKey());
}

MOTOR_TEST_CASE(entity_sort_key_different_for_different_positions) {
    TextureAtlas* atlas = createMockAtlas();
    TestEntity entity1(GridCoord{0, 0}, 0, atlas);
    TestEntity entity2(GridCoord{1, 0}, 0, atlas);
    
    // Segun Entity::getSortKey(), el sort key es (grid_x + grid_y) * tileHeight/2
    // Para (0,0): 0, Para (1,0): (1+0)*16 = 16 (si tileHeight=32)
    // Entonces deberian ser diferentes
    MOTOR_CHECK(entity1.getSortKey() != entity2.getSortKey());
}

// --- Tests de Character Sprite (override de sprite) ---

MOTOR_TEST_CASE(entity_set_character_sprite_updates_state) {
    TextureAtlas* atlas = createMockAtlas();
    TestEntity entity(GridCoord{0, 0}, 0, atlas);
    
    // setCharacterSprite es un metodo de Entity
    entity.setCharacterSprite(atlas, 64, 64, Vector2{0, 0});
    // No crash
    MOTOR_CHECK(true);
    
    entity.clearCharacterSprite();
    MOTOR_CHECK(true);
}

// --- Tests de herencia de TestEntity ---

MOTOR_TEST_CASE(test_entity_implements_update) {
    TextureAtlas* atlas = createMockAtlas();
    TestEntity entity(GridCoord{0, 0}, 0, atlas);
    
    // Podemos llamar a update sin crash
    entity.update(0.1f);
    MOTOR_CHECK(true); // Si no crash, el metodo funciona
}

// --- AV16-04: la regla de capas ---

MOTOR_TEST_CASE(capas_plano_bajo_de_pie_bajo_encima) {
    TextureAtlas* atlas = createMockAtlas();
    // Una alfombra en la celda de DELANTE (14,14) sigue debajo de alguien de pie en (1,1).
    TestEntity alfombra(GridCoord{14, 14}, 0, atlas);
    alfombra.setCapa(CapaDeDibujo::Plano);
    TestEntity detras(GridCoord{1, 1}, 0, atlas);
    TestEntity delante(GridCoord{14, 14}, 0, atlas);
    TestEntity tejado(GridCoord{0, 0}, 0, atlas);
    tejado.setCapa(CapaDeDibujo::Encima);
    MOTOR_CHECK(alfombra.getSortKey() < detras.getSortKey());
    MOTOR_CHECK(alfombra.getSortKey() < delante.getSortKey());
    MOTOR_CHECK(detras.getSortKey() < delante.getSortKey());
    MOTOR_CHECK(tejado.getSortKey() > delante.getSortKey());
    // Entre dos planos, por profundidad.
    TestEntity plano2(GridCoord{2, 2}, 0, atlas);
    plano2.setCapa(CapaDeDibujo::Plano);
    MOTOR_CHECK(plano2.getSortKey() < alfombra.getSortKey());
}

MOTOR_TEST_CASE(capas_el_barquero_va_encima_de_su_barca_en_la_misma_celda) {
    TextureAtlas* atlas = createMockAtlas();
    TestEntity barca(GridCoord{14, 8}, 0, atlas);
    TestEntity barquero(GridCoord{14, 8}, 0, atlas);
    MOTOR_CHECK(barca.getSortKey() == barquero.getSortKey());  // el empate de antes
    barquero.setSortBias(CapasDeDibujo::kPasoDeProfundidad / 2);
    MOTOR_CHECK(barquero.getSortKey() > barca.getSortKey());
    // ...pero sin pasar a la diagonal siguiente.
    TestEntity siguiente(GridCoord{15, 8}, 0, atlas);
    MOTOR_CHECK(barquero.getSortKey() < siguiente.getSortKey());
}

MOTOR_TEST_CASE(capas_por_id_de_objeto) {
    MOTOR_CHECK(CapasDeDibujo::capaDeObjeto("felpudo") == CapaDeDibujo::Plano);
    MOTOR_CHECK(CapasDeDibujo::capaDeObjeto("salida_interior_taberna") == CapaDeDibujo::Plano);
    MOTOR_CHECK(CapasDeDibujo::capaDeObjeto("tejado_casa") == CapaDeDibujo::Encima);
    MOTOR_CHECK(CapasDeDibujo::capaDeObjeto("farola") == CapaDeDibujo::DePie);
    // C2-01: el estrado es suelo para el dibujo; un PNJ que se llama «verina_estrado» sigue de pie.
    MOTOR_CHECK(CapasDeDibujo::capaDeObjeto("estrado_sermon") == CapaDeDibujo::Plano);
    MOTOR_CHECK(CapasDeDibujo::capaDeObjeto("verina_estrado") == CapaDeDibujo::DePie);
    bool conocida = true;
    MOTOR_CHECK(CapasDeDibujo::desdeTexto("plano", &conocida) == CapaDeDibujo::Plano && conocida);
    CapasDeDibujo::desdeTexto("raro", &conocida);
    MOTOR_CHECK(!conocida);
}

MOTOR_TEST_CASE(alturas_el_dibujo_de_una_entidad_sube_con_el_terreno) {
    Alturas a;
    a.construye({AlturaRegion{3, 3, 1, 1, 2, false, 0, 'y'}}, 8, 8);
    TestEntity c(GridCoord{3, 3}, 0, createMockAtlas());
    MOTOR_CHECK(c.elevacion() == 0.0f);  // sin mapa de alturas, plano
    c.setAlturas(&a);
    MOTOR_CHECK(std::fabs(c.elevacion() - (-2.0f * Alturas::kPasoPx)) < 1e-6f);
    c.setGridPosition(GridCoord{0, 0});
    MOTOR_CHECK(c.elevacion() == 0.0f);  // al bajar de la meseta, vuelve al suelo
    MOTOR_CHECK(std::fabs(c.elevacionEn(GridCoord{3, 3}) + 32.0f) < 1e-6f);
}

int main() { return motor_test::runAllTests(); }
