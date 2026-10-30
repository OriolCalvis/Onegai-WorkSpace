#include "Editor/ExportState.h"

#include "Core/Errors/Result.h"
#include "Engine/Locale.h"
#include "Game/SaveSlotState.h"

#include <Core/Json/JsonValue.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <variant>

namespace Editor {

ExportPanelCore::ExportPanelCore() = default;
ExportPanelCore::ExportPanelCore(std::shared_ptr<Engine::Locale> locale,
                                 std::shared_ptr<Game::SaveSlots> saveSlots)
    : m_locale(std::move(locale))
    , m_saveSlots(std::move(saveSlots))
{
}

static std::string kPlatformKeyExport = "g15.export.";

auto ExportPanelCore::availablePlatforms() const -> std::vector<ExportPlatform>
{
    return { ExportPlatform::MacOS, ExportPlatform::Linux, ExportPlatform::Windows };
}

auto ExportPanelCore::platformLabel(ExportPlatform p) const -> std::string
{
    std::string key = kPlatformKeyExport;
    switch (p) {
        case ExportPlatform::MacOS: key += "macos"; break;
        case ExportPlatform::Linux: key += "linux"; break;
        case ExportPlatform::Windows: key += "windows"; break;
    }
    if (m_locale && m_locale->loaded()) {
        auto t = m_locale->text(key);
        if (!t.text.empty()) return t.text;
    }
    if (p == ExportPlatform::MacOS) return "macOS";
    if (p == ExportPlatform::Linux) return "Linux";
    return "Windows";
}

auto ExportPanelCore::platformCode(ExportPlatform p) const -> std::string
{
    switch (p) {
        case ExportPlatform::MacOS: return "macos";
        case ExportPlatform::Linux: return "linux";
        case ExportPlatform::Windows: return "windows";
    }
    return "macos";
}

auto ExportPanelCore::isRuntimeAvailable(ExportPlatform p) -> bool
{
#if defined(__APPLE__)
    if (p == ExportPlatform::MacOS) return true;
    return false;
#elif defined(__linux__)
    if (p == ExportPlatform::Linux) return true;
    return false;
#elif defined(_WIN32)
    if (p == ExportPlatform::Windows) return true;
    return false;
#else
    if (p == ExportPlatform::MacOS) return true;
    return false;
#endif
}

auto ExportPanelCore::detectBinaryForPlatform(const std::filesystem::path& repoBuildDir,
                                              ExportPlatform p) -> std::optional<std::filesystem::path>
{
    std::filesystem::path buildDir = repoBuildDir.empty() ? std::filesystem::path("build") : repoBuildDir;
    std::vector<std::filesystem::path> candidates;
    switch (p) {
        case ExportPlatform::MacOS:
            candidates.push_back(buildDir / "juego");
            candidates.push_back(buildDir / "motor_graphico");
            break;
        case ExportPlatform::Linux:
            candidates.push_back(buildDir / "juego");
            break;
        case ExportPlatform::Windows:
            candidates.push_back(buildDir / "juego.exe");
            candidates.push_back(buildDir / "Release" / "juego.exe");
            break;
    }
    for (const auto& c : candidates) {
        std::error_code ec;
        if (std::filesystem::exists(c, ec)) return c;
    }
    return std::nullopt;
}

auto ExportPanelCore::cancelInProgress() -> bool
{
    return true;
}

auto ExportPanelCore::lastReport() const -> const std::optional<ExportReport>&
{
    return m_last;
}

static bool fileIsProbablyZip(const std::filesystem::path& p)
{
    std::ifstream in(p, std::ios::binary);
    if (!in.good()) return false;
    char magic[4] = {0,0,0,0};
    in.read(magic, 4);
    return magic[0] == 'P' && magic[1] == 'K';
}

static bool fileIsProbablyDirectory(const std::filesystem::path& p)
{
    std::error_code ec;
    return std::filesystem::is_directory(p, ec);
}

static int signatureMatchesPlatform(const std::filesystem::path& bin, ExportPlatform want)
{
    (void)want;
    std::ifstream in(bin, std::ios::binary);
    if (!in.good()) return -1;
    char magic[5] = {0,0,0,0,0};
    in.read(magic, 4);
#if defined(__APPLE__)
    const bool matchMachO = (magic[0] == (char)0xCF && magic[1] == (char)0xFA && magic[2] == (char)0xED && magic[3] == (char)0xFE)
                         || (magic[0] == (char)0xFE && magic[1] == (char)0xED && magic[2] == (char)0xFA && magic[3] == (char)0xCF);
    return matchMachO ? 1 : 0;
#else
    (void)want;
    if (magic[0] == 'M' && magic[1] == 'Z') return want == ExportPlatform::Windows ? 1 : 0;
    if (magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F') return want == ExportPlatform::Linux ? 1 : 0;
    return (magic[0] == (char)0xCF && magic[1] == (char)0xFA) ? 1 : 0;
#endif
}

static std::vector<std::string> readAllLines(const std::filesystem::path& p)
{
    std::vector<std::string> lines;
    std::ifstream in(p);
    if (!in.good()) return lines;
    std::string line;
    while (std::getline(in, line)) lines.push_back(line);
    return lines;
}

static bool parseJsonExportReport(const std::filesystem::path& reportFile,
                                  ExportReport& out)
{
    std::ifstream in(reportFile);
    if (!in.good()) return false;
    std::ostringstream ss;
    ss << in.rdbuf();
    auto raw = ss.str();
    auto parsed = JsonValue::parse(raw);
    if (!parsed.isOk()) return false;
    auto& obj = parsed.value();
    if (!obj.isObject()) return false;
    if (obj.has("jugable")) {
        auto& j = obj["jugable"];
        if (j.isBool()) out.jugable = j.asBool();
    }
    if (obj.has("referencias_faltantes")) {
        auto& j = obj["referencias_faltantes"];
        if (j.isNumber()) out.missingReferences = j.asInt();
    }
    if (obj.has("ficheros_empaquetados")) {
        auto& j = obj["ficheros_empaquetados"];
        if (j.isNumber()) out.packagedFiles = j.asInt();
    }
    if (obj.has("bytes_totales")) {
        auto& j = obj["bytes_totales"];
        if (j.isNumber()) out.totalBytes = (uint64_t)j.asInt();
    }
    if (obj.has("paquete_generado")) {
        auto& j = obj["paquete_generado"];
        if (j.isString()) out.producedPackage = j.asString();
    }
    if (obj.has("errores")) {
        auto& j = obj["errores"];
        if (j.isArray()) {
            for (size_t i = 0; i < j.size(); ++i) {
                auto& item = j[i];
                if (item.isString()) out.errors.push_back(item.asString());
            }
        }
    }
    if (obj.has("avisos")) {
        auto& j = obj["avisos"];
        if (j.isArray()) {
            for (size_t i = 0; i < j.size(); ++i) {
                auto& item = j[i];
                if (item.isString()) out.warnings.push_back(item.asString());
            }
        }
    }
    return true;
}

auto ExportPanelCore::runExport(const std::filesystem::path& repoRoot,
                                const ExportSelection& sel,
                                bool silent) const -> ExportReport
{
    ExportReport report;

    if (sel.projectId.empty()) {
        report.errors.push_back(m_locale && m_locale->loaded()
            ? m_locale->text("g15.error.project_empty").text
            : "Projecte buit");
        report.success = false;
        report.summaryLine = "FAIL: projecte buit";
        m_last = report;
        return report;
    }

    if (sel.outputDir.empty()) {
        report.errors.push_back("Carpeta de sortida buida");
        report.success = false;
        report.summaryLine = "FAIL: sortida buida";
        m_last = report;
        return report;
    }

    if (!isRuntimeAvailable(sel.platform)) {
        report.errors.push_back("Runtime no disponible en aquest host");
        report.success = false;
        report.summaryLine = "SKIP: runtime no disponible";
        m_last = report;
        return report;
    }

    std::filesystem::path repo = repoRoot.empty() ? std::filesystem::current_path() : repoRoot;
    std::filesystem::path tool = repo / "tools" / "exportar.py";
    {
        std::error_code ec;
        if (!std::filesystem::exists(tool, ec)) {
            report.errors.push_back("tools/exportar.py no troba: " + tool.generic_string());
            report.success = false;
            report.summaryLine = "FAIL: no troba exportar.py";
            m_last = report;
            return report;
        }
    }

    std::filesystem::path binary;
    if (sel.binaryOverride.has_value()) {
        binary = *sel.binaryOverride;
    } else {
        auto d = detectBinaryForPlatform(repo / "build", sel.platform);
        if (!d.has_value()) {
            report.errors.push_back("No s'ha trobat cap binari per a la plataforma");
            report.success = false;
            report.summaryLine = "FAIL: binari no trobat";
            m_last = report;
            return report;
        }
        binary = *d;
    }
    {
        std::error_code ec;
        if (!std::filesystem::exists(binary, ec)) {
            report.errors.push_back("Binari inexistent: " + binary.generic_string());
            report.success = false;
            report.summaryLine = "FAIL: binari inexistent";
            m_last = report;
            return report;
        }
    }

    int sig = signatureMatchesPlatform(binary, sel.platform);
    if (sig < 0) {
        report.errors.push_back("No s'ha pogut llegir la signatura del binari");
        report.success = false;
        report.summaryLine = "FAIL: signatura binari";
        m_last = report;
        return report;
    }
    if (sig == 0) {
        report.errors.push_back("La signatura del binari no correspon a la plataforma");
        report.success = false;
        report.summaryLine = "FAIL: signatura no coincideix";
        m_last = report;
        return report;
    }

    std::filesystem::path tmpOut = sel.outputDir;
    {
        std::error_code ec;
        std::filesystem::create_directories(tmpOut, ec);
    }

    auto tmpDir = std::filesystem::temp_directory_path() / ("g15-export-" + std::to_string(getpid()));
    {
        std::error_code ec;
        std::filesystem::create_directories(tmpDir, ec);
    }
    auto reportFile = tmpDir / "report.json";
    auto stderrFile = tmpDir / "stderr.log";
    auto stdoutFile = tmpDir / "stdout.log";

    std::vector<std::string> argv;
    argv.push_back("/usr/bin/env");
    argv.push_back("python3");
    argv.push_back(tool.generic_string());
    argv.push_back(sel.projectId);
    argv.push_back("--plataforma");
    argv.push_back(platformCode(sel.platform));
    argv.push_back("--binario");
    argv.push_back(binary.generic_string());
    if (sel.includeAllAssets) argv.push_back("--tots-els-assets");
    argv.push_back("--sortida");
    argv.push_back(tmpOut.generic_string());
    if (silent) argv.push_back("--silenciós");
    argv.push_back("--report");
    argv.push_back(reportFile.generic_string());

    std::vector<const char*> cargv;
    cargv.reserve(argv.size() + 1);
    for (const auto& a : argv) cargv.push_back(a.c_str());
    cargv.push_back(nullptr);

    pid_t child = ::fork();
    if (child < 0) {
        report.errors.push_back("fork() ha fallat");
        report.success = false;
        report.summaryLine = "FAIL: fork";
        m_last = report;
        return report;
    }
    if (child == 0) {
        ::freopen(stderrFile.generic_string().c_str(), "w", stderr);
        ::freopen(stdoutFile.generic_string().c_str(), "w", stdout);
        ::chdir(repo.generic_string().c_str());
        ::execvp(cargv[0], (char* const*)cargv.data());
        std::_Exit(127);
    }
    int status = 0;
    ::waitpid(child, &status, 0);
    int exitCode = -1;
    if (WIFEXITED(status)) exitCode = WEXITSTATUS(status);
    report.exitCode = exitCode;

    std::error_code ec;
    if (std::filesystem::exists(reportFile, ec)) {
        (void)parseJsonExportReport(reportFile, report);
    }

    if (exitCode != 0) {
        report.errors.push_back("Exportar.py exit " + std::to_string(exitCode));
        for (const auto& l : readAllLines(stderrFile)) {
            if (l.size() < 4) continue;
            report.warnings.push_back(l);
        }
        report.success = false;
        if (report.summaryLine.empty()) {
            report.summaryLine = "FAIL: exit " + std::to_string(exitCode);
        }
        m_last = report;
        return report;
    }

    if (!report.producedPackage.empty() && std::filesystem::exists(report.producedPackage, ec)) {
        if (fileIsProbablyDirectory(report.producedPackage)) {
            uint64_t total = 0;
            int files = 0;
            auto it = std::filesystem::recursive_directory_iterator(report.producedPackage, ec);
            for (const auto& entry : it) {
                if (entry.is_regular_file(ec)) {
                    files++;
                    std::error_code e2;
                    auto sz = entry.file_size(e2);
                    if (!e2) total += (uint64_t)sz;
                }
            }
            report.packagedFiles = files;
            report.totalBytes = total;
        } else if (fileIsProbablyZip(report.producedPackage)) {
            report.packagedFiles = report.packagedFiles > 0 ? report.packagedFiles : 1;
            auto sz = std::filesystem::file_size(report.producedPackage, ec);
            if (!ec) report.totalBytes = (uint64_t)sz;
        }
    }

    report.success = report.jugable && report.errors.empty();
    if (report.summaryLine.empty()) {
        std::ostringstream ss;
        ss << (report.success ? "OK" : "AVIS") << ": " << report.packagedFiles << " fitxers ("
           << report.totalBytes << " bytes), refs_faltants=" << report.missingReferences
           << " jugable=" << (report.jugable ? "si" : "no");
        report.summaryLine = ss.str();
    }

    {
        std::error_code e2;
        std::filesystem::remove_all(tmpDir, e2);
    }
    m_last = report;
    return report;
}

} // namespace Editor
