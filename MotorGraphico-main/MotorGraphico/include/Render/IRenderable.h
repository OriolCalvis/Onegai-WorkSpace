#pragma once

class SpriteBatch;
class Texture;

// Cualquier objeto que IsometricRenderer pueda encolar y dibujar
// (entidades por ahora; en el futuro, efectos/props sueltos). Separado
// de IUpdatable: no todo lo que se dibuja necesita logica por frame (y
// viceversa, ver motor_grafico_clases.puml).
class IRenderable {
public:
    virtual ~IRenderable() = default;

    virtual void render(SpriteBatch& batch) = 0;

    // Sombra blob a los pies del objeto (Fase 11, ver BlobShadow.h).
    // Virtual con cuerpo vacio y NO pura a proposito: dibujar sombra es
    // opt-in por tipo (una entidad la quiere, un efecto de particulas
    // futuro probablemente no), y las implementaciones existentes de
    // IRenderable no deben romperse por anadir el metodo. La llama
    // IsometricRenderer DESPUES de los tiles y ANTES de render() de
    // todas las entidades: todas las sombras quedan bajo todos los
    // sprites (una sombra nunca debe pintarse encima de otra entidad).
    // shadowTexture: no propietaria, la gestiona quien la creo (ver
    // CreateBlobShadowTexture()).
    virtual void renderShadow(SpriteBatch& batch, Texture* shadowTexture) {
        (void)batch;
        (void)shadowTexture;
    }

    // Clave de ordenacion del Painter's Algorithm (ver
    // IsometricRenderer::sortQueue): a menor clave, se dibuja antes.
    virtual int getSortKey() const = 0;
};
