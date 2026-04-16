#pragma once

#include <string>
#include <vector>
#include "Models.h"

class StudentManager {
public:
    StudentManager(const std::string &dataDir);

    // Create student (assigns id if empty) and optionally save
    bool CreateStudent(Student &s, bool saveNow = true);
    std::vector<std::string> ListStudents();
    bool LoadStudent(const std::string &id, Student &out);
    bool SaveStudent(const Student &s);
    bool DeleteStudent(const std::string &id);
    std::string GenerateId();

private:
    std::filesystem::path m_dataDir;
};
