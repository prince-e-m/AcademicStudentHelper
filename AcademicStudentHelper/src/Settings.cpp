#include "Settings.h"
#include <filesystem>
#include <fstream>

namespace Settings {

static std::filesystem::path settingsPath() {
    return std::filesystem::path("data") / "settings.cfg";
}

float LoadPassThreshold() {
    try {
        auto p = settingsPath();
        if (!std::filesystem::exists(p)) return 75.0f;
        std::ifstream ifs(p);
        float v; ifs >> v; if (!ifs) return 75.0f; return v;
    } catch (...) { return 75.0f; }
}

bool SavePassThreshold(float v) {
    try {
        auto p = settingsPath();
        if (!std::filesystem::exists(p.parent_path())) std::filesystem::create_directories(p.parent_path());
        std::ofstream ofs(p); ofs << v; return !!ofs;
    } catch (...) { return false; }
}

}
