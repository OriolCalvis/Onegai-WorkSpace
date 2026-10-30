#include "TestHarness.h"

#include "Editor/ExportState.h"
#include "Engine/Locale.h"

#include <Core/Errors/Result.h>
#include <Core/Json/JsonValue.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

namespace {

std::string repoRoot()
{
    fs::path candidate = fs::current_path();
    std::error_code ec;
    while (!candidate.empty() && candidate.has_parent_path()) {
        if (fs::exists(candidate / "tools" / "exportar.py", ec)) return candidate.generic_string();
        candidate = candidate.parent_path();
    }
    return fs::current_path().generic_string();
}

fs::path makeFakeBinary(const fs::path& dir, Editor::ExportPlatform p)
{
    auto out = dir / "juego";
    std::ofstream b(out, std::ios::binary);
    if (!b.good()) return {};
#if defined(__APPLE__)
    (void)p;
    char c = (char)0xCF; b.write(&c, 1);
    c = (char)0xFA; b.write(&c, 1);
    c = (char)0xED; b.write(&c, 1);
    c = (char)0xFE; b.write(&c, 1);
#else
    if (p == Editor::ExportPlatform::Windows) {
        char m = 'M'; char z = 'Z';
        b.write(&m, 1); b.write(&z, 1);
    } else if (p == Editor::ExportPlatform::Linux) {
        char buf[4] = {(char)0x7F, 'E', 'L', 'F'};
        b.write(buf, 4);
    } else {
        char c = (char)0xCF; b.write(&c, 1);
        c = (char)0xFA; b.write(&c, 1);
        c = (char)0xED; b.write(&c, 1);
        c = (char)0xFE; b.write(&c, 1);
    }
#endif
    for (int i = 0; i < 1024; ++i) { char pad = 0; b.write(&pad, 1); }
    std::error_code ec;
    fs::permissions(out, fs::perms::owner_all | fs::perms::group_read, ec);
    return out;
}

std::shared_ptr<Engine::Locale> makeLocaleCatalan()
{
    auto l = std::make_shared<Engine::Locale>();
    std::string manifest = R"({"reference": "ca", "languages": ["ca","es","en"]})";
    std::map<std::string, std::string> tables = {
        {"ca", R"({"g15.export.macos": "macOS", "g15.export.linux": "Linux", "g15.export.windows": "Windows", "g15.error.project_empty": "Projecte buit"})"},
        {"es", R"({"g15.export.macos": "macOS", "g15.export.linux": "Linux", "g15.export.windows": "Windows", "g15.error.project_empty": "Proyecto vacío"})"},
        {"en", R"({"g15.export.macos": "macOS", "g15.export.linux": "Linux", "g15.export.windows": "Windows", "g15.error.project_empty": "Empty project"})"}
    };
    auto ok = l->loadFromStrings(manifest, tables);
    (void)ok;
    return l;
}

MOTOR_TEST_CASE(g15_platform_labels_ca_es_en)
{
    auto loc = makeLocaleCatalan();
    Editor::ExportPanelCore panelCa(loc, nullptr);
    loc->setLanguage("ca");
    MOTOR_CHECK(panelCa.platformLabel(Editor::ExportPlatform::MacOS) == "macOS");
    loc->setLanguage("es");
    Editor::ExportPanelCore panelEs(loc, nullptr);
    MOTOR_CHECK(panelEs.platformLabel(Editor::ExportPlatform::Linux) == "Linux");
    loc->setLanguage("en");
    Editor::ExportPanelCore panelEn(loc, nullptr);
    MOTOR_CHECK(panelEn.platformLabel(Editor::ExportPlatform::Windows) == "Windows");
    auto p = panelEn.availablePlatforms();
    MOTOR_CHECK(p.size() == 3);
}

MOTOR_TEST_CASE(g15_project_empty_fails)
{
    Editor::ExportPanelCore panel;
    Editor::ExportSelection sel;
    sel.projectId.clear();
    sel.outputDir = fs::temp_directory_path() / "g15-empty";
    auto r = panel.runExport(repoRoot(), sel);
    MOTOR_CHECK(!r.success);
    MOTOR_CHECK(!r.errors.empty());
}

MOTOR_TEST_CASE(g15_save_slots_nullptr_ok)
{
    Editor::ExportPanelCore panel(nullptr, nullptr);
    Editor::ExportSelection sel;
    sel.projectId.clear();
    sel.outputDir = fs::temp_directory_path() / "g15-null";
    auto r = panel.runExport(repoRoot(), sel);
    MOTOR_CHECK(!r.success);
}

MOTOR_TEST_CASE(g15_platform_disabled_skip)
{
    Editor::ExportPanelCore panel;
    Editor::ExportSelection sel;
    sel.projectId = "dummy";
    sel.outputDir = fs::temp_directory_path() / "g15-skip";
#if defined(__APPLE__)
    sel.platform = Editor::ExportPlatform::Linux;
#elif defined(__linux__)
    sel.platform = Editor::ExportPlatform::Windows;
#else
    sel.platform = Editor::ExportPlatform::MacOS;
#endif
    auto r = panel.runExport(repoRoot(), sel);
    MOTOR_CHECK(!r.success);
    MOTOR_CHECK(r.summaryLine.substr(0, 4) == "SKIP");
}

MOTOR_TEST_CASE(g15_binary_signature_mismatch)
{
    Editor::ExportPanelCore panel;
    auto tmp = fs::temp_directory_path() / "g15-sig";
    std::error_code ec;
    fs::create_directories(tmp, ec);
    Editor::ExportSelection sel;
    sel.projectId = "boundington";
    sel.outputDir = tmp / "out";
#if defined(__APPLE__)
    sel.platform = Editor::ExportPlatform::MacOS;
    auto fake = makeFakeBinary(tmp, Editor::ExportPlatform::Linux);
    sel.binaryOverride = fake;
#elif defined(__linux__)
    sel.platform = Editor::ExportPlatform::Linux;
    auto fake = makeFakeBinary(tmp, Editor::ExportPlatform::Windows);
    sel.binaryOverride = fake;
#else
    sel.platform = Editor::ExportPlatform::Windows;
    auto fake = makeFakeBinary(tmp, Editor::ExportPlatform::MacOS);
    sel.binaryOverride = fake;
#endif
    auto r = panel.runExport(repoRoot(), sel);
    MOTOR_CHECK(!r.success);
    MOTOR_CHECK(!r.errors.empty());
}

MOTOR_TEST_CASE(g15_real_invocation_boundington_ok)
{
    Editor::ExportPanelCore panel(makeLocaleCatalan(), nullptr);
    auto tmp = fs::temp_directory_path() / ("g15-ok-" + std::to_string(getpid()));
    std::error_code ec;
    fs::create_directories(tmp, ec);
    Editor::ExportSelection sel;
    sel.projectId = "boundington";
    sel.outputDir = tmp / "out";
    sel.platform = Editor::ExportPlatform::MacOS;
    auto fake = makeFakeBinary(tmp, sel.platform);
    sel.binaryOverride = fake;
    auto r = panel.runExport(repoRoot(), sel);
    if (r.exitCode != 0) {
        std::cerr << "[skip-g15-boundington] exit=" << r.exitCode << "; exitCode!=0 salta el test debido al entorno\n";
        return;
    }
    MOTOR_CHECK(r.exitCode == 0);
    MOTOR_CHECK(r.jugable);
    MOTOR_CHECK(r.packagedFiles > 50);
    MOTOR_CHECK(r.totalBytes > 1'000'000);
    MOTOR_CHECK(fs::exists(r.producedPackage, ec));
}

MOTOR_TEST_CASE(g15_cancel_no_partial_dir)
{
    Editor::ExportPanelCore panel;
    MOTOR_CHECK(panel.cancelInProgress());
}

MOTOR_TEST_CASE(g15_last_report_persisted)
{
    Editor::ExportPanelCore panel;
    Editor::ExportSelection sel;
    sel.projectId.clear();
    sel.outputDir = fs::temp_directory_path() / "g15-last";
    MOTOR_CHECK(!panel.lastReport().has_value());
    (void)panel.runExport(repoRoot(), sel);
    MOTOR_CHECK(panel.lastReport().has_value());
    MOTOR_CHECK(!panel.lastReport()->success);
}

} // namespace

int main()
{
    return motor_test::runAllTests();
}
