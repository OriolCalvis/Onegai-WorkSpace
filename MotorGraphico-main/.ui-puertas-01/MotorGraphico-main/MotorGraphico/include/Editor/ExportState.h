#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Engine {
class Locale;
}

namespace Game {
class SaveSlots;
}

namespace Editor {

enum class ExportPlatform {
    MacOS,
    Linux,
    Windows,
};

struct ExportSelection {
    std::string projectId;
    ExportPlatform platform = ExportPlatform::MacOS;
    std::filesystem::path outputDir;
    std::optional<std::filesystem::path> binaryOverride;
    bool includeAllAssets = false;
};

struct ExportReport {
    bool success = false;
    bool jugable = false;
    int exitCode = -1;
    std::filesystem::path producedPackage;
    std::string summaryLine;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    int missingReferences = 0;
    int packagedFiles = 0;
    uint64_t totalBytes = 0;
};

class ExportPanelCore {
public:
    ExportPanelCore();
    explicit ExportPanelCore(std::shared_ptr<Engine::Locale> locale,
                             std::shared_ptr<Game::SaveSlots> saveSlots = nullptr);

    auto availablePlatforms() const -> std::vector<ExportPlatform>;
    auto platformLabel(ExportPlatform p) const -> std::string;
    auto platformCode(ExportPlatform p) const -> std::string;

    auto runExport(const std::filesystem::path& repoRoot,
                   const ExportSelection& sel,
                   bool silent = true) const -> ExportReport;

    auto cancelInProgress() -> bool;
    auto lastReport() const -> const std::optional<ExportReport>&;

    static auto isRuntimeAvailable(ExportPlatform p) -> bool;
    static auto detectBinaryForPlatform(const std::filesystem::path& repoBuildDir,
                                        ExportPlatform p) -> std::optional<std::filesystem::path>;

private:
    std::shared_ptr<Engine::Locale> m_locale;
    std::shared_ptr<Game::SaveSlots> m_saveSlots;
    mutable std::optional<ExportReport> m_last;
};

} // namespace Editor
