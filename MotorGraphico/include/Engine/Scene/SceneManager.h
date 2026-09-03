#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "Core/Errors/Result.h"
#include "Engine/Scene/Scene.h"

namespace Engine {

class SceneManager {
public:
    enum class TransitionKind { Replace, Push, Pop };

    SceneManager() = default;
    ~SceneManager();

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    // Inicializa la raíz de la pila. No existe una escena activa hasta que
    // este método carga y entra correctamente.
    Result<bool> start(std::unique_ptr<Scene> initialScene);
    void shutdown();

    // Las peticiones no modifican la escena durante un callback. Solo se
    // admite una por frame: ello hace el orden observable y evita que un
    // overlay creado por otro overlay ejecute dos ciclos en el mismo frame.
    Result<bool> requestReplace(std::unique_ptr<Scene> scene);
    Result<bool> requestPush(std::unique_ptr<Scene> scene);
    Result<bool> requestPop();
    bool hasPendingTransition() const;

    // Se llama al final del frame, nunca desde update(). Un error de carga
    // descarta la solicitud y mantiene intacta la escena anterior.
    Result<bool> applyPendingTransition();

    Scene* activeScene();
    const Scene* activeScene() const;
    ids::MapId activeMapId() const;
    std::size_t stackDepth() const { return m_stack.size(); }

    void fixedUpdate(float fixedDeltaSeconds);
    void update(float deltaSeconds);
    void render(float interpolationAlpha);

private:
    struct PendingTransition {
        TransitionKind kind = TransitionKind::Replace;
        std::unique_ptr<Scene> incoming;
    };

    Result<bool> request(TransitionKind kind, std::unique_ptr<Scene> incoming);

    std::vector<std::unique_ptr<Scene>> m_stack;
    std::unique_ptr<PendingTransition> m_pending;
};

}  // namespace Engine
