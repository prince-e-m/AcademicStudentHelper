#include "Persistence.h"
#include "../third_party/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

static std::filesystem::path studentPath(const std::filesystem::path &dataDir, const std::string &id) {
    return dataDir / ("student_" + id + ".json");
}

namespace Persistence {

bool SaveStudent(const Student &s, const std::filesystem::path &dataDir) {
    try {
        if (!std::filesystem::exists(dataDir)) std::filesystem::create_directories(dataDir);
        json j = s;
        std::ofstream ofs(studentPath(dataDir, s.id));
        if (!ofs) return false;
        ofs << j.dump(4);
        return true;
    }
    catch (const std::exception &ex) {
        std::cerr << "SaveStudent error: " << ex.what() << std::endl;
        return false;
    }
}

bool LoadStudent(const std::string &id, Student &out, const std::filesystem::path &dataDir) {
    try {
        auto p = studentPath(dataDir, id);
        if (!std::filesystem::exists(p)) return false;
        std::ifstream ifs(p);
        if (!ifs) return false;
        json j; ifs >> j;
        out = j.get<Student>();
        return true;
    }
    catch (const std::exception &ex) {
        std::cerr << "LoadStudent error: " << ex.what() << std::endl;
        return false;
    }
}

std::vector<std::string> ListStudentIds(const std::filesystem::path &dataDir) {
    std::vector<std::string> ids;
    if (!std::filesystem::exists(dataDir)) return ids;
    for (auto &e : std::filesystem::directory_iterator(dataDir)) {
        if (!e.is_regular_file()) continue;
        auto name = e.path().filename().string();
        if (name.rfind("student_", 0) == 0 && name.size() > 8) {
            // student_<id>.json
            auto id = name.substr(8);
            if (id.size() > 5 && id.substr(id.size()-5) == ".json") id = id.substr(0, id.size()-5);
            ids.push_back(id);
        }
    }
    return ids;
}

bool DeleteStudentFile(const std::string &id, const std::filesystem::path &dataDir) {
    try {
        auto p = studentPath(dataDir, id);
        if (std::filesystem::exists(p)) return std::filesystem::remove(p);
        return false;
    }
    catch (...) { return false; }
}

bool DeleteAllStudentFiles(const std::filesystem::path &dataDir) {
    try {
        if (!std::filesystem::exists(dataDir)) return true;
        bool ok = true;
        for (auto &e : std::filesystem::directory_iterator(dataDir)) {
            if (!e.is_regular_file()) continue;
            auto name = e.path().filename().string();
            if (name.rfind("student_", 0) == 0 && name.size() > 8) {
                ok = std::filesystem::remove(e.path()) && ok;
            }
        }
        return ok;
    }
    catch (...) { return false; }
}

} // namespace Persistence
