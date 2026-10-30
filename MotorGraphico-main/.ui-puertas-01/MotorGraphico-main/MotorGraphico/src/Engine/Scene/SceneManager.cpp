#include "Engine/Scene/SceneManager.h"

#include <utility>

namespace Engine {

SceneManager::~SceneManager() { shutdown(); }

Result<bool> SceneManager::start(std::unique_ptr<Scene> initialScene) {
    if (initialScene == nullptr) {
        return Result<bool>::Error("SceneManager necesita una escena inicial");
    }
    if (!m_stack.empty()) {
        return Result<bool>::Error("SceneManager ya tiene una escena inicial");
    }
    if (m_pending != nullptr) {
        return Result<bool>::Error("SceneManager tiene una transición pendiente");
    }

    Result<bool> loaded = initialScene->load();
    if (!loaded.isOk()) {
        return Result<bool>::Error("No se pudo cargar escena inicial '" +
                                   std::string(initialScene->name()) + "': " +
                                   loaded.errorMessage());
    }
    if (!loaded.value()) {
        return Result<bool>::Error("La escena inicial '" + std::string(initialScene->name()) +
                                   "' no quedó lista");
    }

    m_stack.push_back(std::move(initialScene));
    m_stack.back()->onEnter();
    return Result<bool>::Ok(true);
}

void SceneManager::shutdown() {
    m_pending.reset();
    while (!m_stack.empty()) {
        m_stack.back()->onExit();
        m_stack.pop_back();
    }
}

Result<bool> SceneManager::requestReplace(std::unique_ptr<Scene> scene) {
    return request(TransitionKind::Replace, std::move(scene));
}

Result<bool> SceneManager::requestPush(std::unique_ptr<Scene> scene) {
    return request(TransitionKind::Push, std::move(scene));
}

Result<bool> SceneManager::requestPop() { return request(TransitionKind::Pop, nullptr); }

bool SceneManager::hasPendingTransition() const { return m_pending != nullptr; }

Result<bool> SceneManager::request(TransitionKind kind, std::unique_ptr<Scene> incoming) {
    if (m_stack.empty()) {
        return Result<bool>::Error("SceneManager no tiene una escena activa");
    }
    if (m_pending != nullptr) {
        return Result<bool>::Error("SceneManager solo admite una transición pendiente");
    }
    if (kind == TransitionKind::Pop) {
        if (m_stack.size() == 1) {
            return Result<bool>::Error("No se puede retirar la escena raíz");
        }
    } else if (incoming == nullptr) {
        return Result<bool>::Error("La transición necesita una escena entrante");
    }

    auto pending = std::make_unique<PendingTransition>();
    pending->kind = kind;
    pending->incoming = std::move(incoming);
    m_pending = std::move(pending);
    return Result<bool>::Ok(true);
}

Result<bool> SceneManager::applyPendingTransition() {
    if (m_pending == nullptr) {
        return Result<bool>::Ok(false);
    }

    // Consumir antes de invocar hooks evita reentradas accidentales. Si load
    // falla, la petición se descarta y el llamador puede decidir qué mostrar.
    std::unique_ptr<PendingTransition> pending = std::move(m_pending);
    if (pending->kind == TransitionKind::Pop) {
        m_stack.back()->onExit();
        m_stack.pop_back();
        m_stack.back()->onResume();
        return Result<bool>::Ok(true);
    }

    Result<bool> loaded = pending->incoming->load();
    if (!loaded.isOk()) {
        return Result<bool>::Error("No se pudo cargar escena '" +
                                   std::string(pending->incoming->name()) + "': " +
                                   loaded.errorMessage());
    }
    if (!loaded.value()) {
        return Result<bool>::Error("La escena '" + std::string(pending->incoming->name()) +
                                   "' no quedó lista");
    }

    if (pending->kind == TransitionKind::Push) {
        m_stack.back()->onPause();
        m_stack.push_back(std::move(pending->incoming));
        m_stack.back()->onEnter();
        return Result<bool>::Ok(true);
    }

    m_stack.back()->onExit();
    m_stack.back() = std::move(pending->incoming);
    m_stack.back()->onEnter();
    return Result<bool>::Ok(true);
}

Scene* SceneManager::activeScene() { return m_stack.empty() ? nullptr : m_stack.back().get(); }

const Scene* SceneManager::activeScene() const {
    return m_stack.empty() ? nullptr : m_stack.back().get();
}

ids::MapId SceneManager::activeMapId() const {
    const Scene* scene = activeScene();
    return scene != nullptr ? scene->mapId() : ids::MapId{};
}

void SceneManager::fixedUpdate(float fixedDeltaSeconds) {
    if (Scene* scene = activeScene()) {
        scene->fixedUpdate(fixedDeltaSeconds);
    }
}

void SceneManager::update(float deltaSeconds) {
    if (Scene* scene = activeScene()) {
        scene->update(deltaSeconds);
    }
}

void SceneManager::render(float interpolationAlpha) {
    if (Scene* scene = activeScene()) {
        scene->render(interpolationAlpha);
    }
}

}  // namespace Engine
