#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "Models.h"

namespace Persistence {
    bool SaveStudent(const Student &s, const std::filesystem::path &dataDir);
    bool LoadStudent(const std::string &id, Student &out, const std::filesystem::path &dataDir);
    std::vector<std::string> ListStudentIds(const std::filesystem::path &dataDir);
    bool DeleteStudentFile(const std::string &id, const std::filesystem::path &dataDir);
    bool DeleteAllStudentFiles(const std::filesystem::path &dataDir);
}
