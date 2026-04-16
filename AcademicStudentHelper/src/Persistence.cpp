#include "Persistence.h"
#include "json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

static std::filesystem::path studentPath(const std::filesystem::path &dataDir, const std::string &id) {
    return dataDir / ("student_" + id + ".json");
}

std::vector<std::string> Persistence::ListBackupDirs(const std::filesystem::path &backupRoot) {
    std::vector<std::string> res;
    try {
        if (!std::filesystem::exists(backupRoot)) return res;
        for (auto &e : std::filesystem::directory_iterator(backupRoot)) {
            if (e.is_directory()) res.push_back(e.path().filename().string());
        }
    }
    catch (...) {}
    return res;
}

bool Persistence::RestoreBackup(const std::filesystem::path &backupDir, const std::filesystem::path &dataDir) {
    try {
        if (!std::filesystem::exists(backupDir)) return false;
        if (!std::filesystem::exists(dataDir)) std::filesystem::create_directories(dataDir);
        for (auto &e : std::filesystem::directory_iterator(backupDir)) {
            if (!e.is_regular_file()) continue;
            auto dest = dataDir / e.path().filename();
            std::filesystem::copy_file(e.path(), dest, std::filesystem::copy_options::overwrite_existing);
        }
        return true;
    }
    catch (...) { return false; }
}

bool Persistence::RestoreStudentFromBackup(const std::string &id, const std::filesystem::path &backupDir, const std::filesystem::path &dataDir) {
    try {
        auto filename = std::string("student_") + id + ".json";
        auto p = backupDir / filename;
        if (!std::filesystem::exists(p)) return false;
        if (!std::filesystem::exists(dataDir)) std::filesystem::create_directories(dataDir);
        auto dest = dataDir / p.filename();
        std::filesystem::copy_file(p, dest, std::filesystem::copy_options::overwrite_existing);
        return true;
    }
    catch (...) { return false; }
}

namespace Persistence {

bool BackupStudentFile(const std::string &id, const std::filesystem::path &dataDir, const std::filesystem::path &backupDir) {
    try {
        auto p = studentPath(dataDir, id);
        if (!std::filesystem::exists(p)) return false;
        if (!std::filesystem::exists(backupDir)) std::filesystem::create_directories(backupDir);
        auto dest = backupDir / p.filename();
        std::filesystem::copy_file(p, dest, std::filesystem::copy_options::overwrite_existing);
        // validate copied JSON by trying to load it from backup location
        Student tmp;
        if (!LoadStudent(id, tmp, backupDir)) {
            std::filesystem::remove(dest);
            return false;
        }
        return true;
    }
    catch (...) { return false; }
}

bool BackupAllStudentFiles(const std::filesystem::path &dataDir, const std::filesystem::path &backupDir) {
    try {
        if (!std::filesystem::exists(dataDir)) return false;
        if (!std::filesystem::exists(backupDir)) std::filesystem::create_directories(backupDir);
        for (auto &e : std::filesystem::directory_iterator(dataDir)) {
            if (!e.is_regular_file()) continue;
            auto name = e.path().filename().string();
            if (name.rfind("student_", 0) == 0) {
                auto dest = backupDir / e.path().filename();
                std::filesystem::copy_file(e.path(), dest, std::filesystem::copy_options::overwrite_existing);
                // validate copied file
                // extract id from filename
                std::string id = name.substr(8);
                if (id.size() > 5 && id.substr(id.size()-5) == ".json") id = id.substr(0, id.size()-5);
                Student tmp; if (!LoadStudent(id, tmp, backupDir)) {
                    // remove the copied file and continue
                    std::filesystem::remove(dest);
                }
            }
        }
        return true;
    }
    catch (...) { return false; }
}

bool SaveStudent(const Student &s, const std::filesystem::path &dataDir) {
    try {
        if (!std::filesystem::exists(dataDir)) std::filesystem::create_directories(dataDir);
        // Use studentNumber as filename identifier
        if (s.studentNumber.empty()) return false;
        json j = s;
        auto target = studentPath(dataDir, s.studentNumber);
        auto tmp = target;
        tmp += ".tmp";
        // write to temp file first
        std::ofstream ofs(tmp, std::ios::binary);
        if (!ofs) return false;
        ofs << j.dump(4);
        ofs.close();
        // validate written JSON by parsing
        try {
            std::ifstream ifs(tmp);
            json jj; ifs >> jj;
            jj.get<Student>();
        } catch (...) {
            std::filesystem::remove(tmp);
            return false;
        }
        // atomic replace
        std::error_code ec;
        std::filesystem::rename(tmp, target, ec);
        if (ec) {
            // fallback to copy+remove
            std::filesystem::copy_file(tmp, target, std::filesystem::copy_options::overwrite_existing);
            std::filesystem::remove(tmp);
        }
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
