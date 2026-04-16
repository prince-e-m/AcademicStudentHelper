#include "StudentManager.h"
#include "Persistence.h"
#include <algorithm>
#include <chrono>
#include <random>
#include <sstream>

StudentManager::StudentManager(const std::string &dataDir)
    : m_dataDir(dataDir)
{
}

std::string StudentManager::GenerateId() {
    // Generate a human-friendly incremental ID like S0001 based on existing files
    try {
        auto ids = Persistence::ListStudentIds(m_dataDir);
        // find max numeric suffix if any
        int maxIndex = 0;
        for (const auto &id : ids) {
            // id may be like S0001 or arbitrary; try parse trailing digits
            size_t pos = id.find_first_of("0123456789");
            if (pos == std::string::npos) continue;
            std::string digits = id.substr(pos);
            try {
                int v = std::stoi(digits);
                if (v > maxIndex) maxIndex = v;
            } catch (...) {}
        }
        int next = maxIndex + 1;
        std::ostringstream ss;
        ss << 'S' << std::setw(4) << std::setfill('0') << next;
        return ss.str();
    }
    catch (...) {
        // fallback to timestamp-based id
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        std::ostringstream ss; ss << "S" << now;
        return ss.str();
    }
}

bool StudentManager::CreateStudent(Student &s, bool saveNow) {
    if (s.id.empty()) s.id = GenerateId();
    // ensure studentNumber is set to a readable value if empty
    if (s.studentNumber.empty()) s.studentNumber = s.id;
    // add history record
    ChangeRecord rec;
    rec.timestamp = std::to_string(std::time(nullptr));
    rec.action = "create";
    rec.details = "Created student";
    s.history.push_back(rec);
    if (saveNow) return SaveStudent(s);
    return true;
}

std::vector<std::string> StudentManager::ListStudents() {
    return Persistence::ListStudentIds(m_dataDir);
}

bool StudentManager::LoadStudent(const std::string &id, Student &out) {
    return Persistence::LoadStudent(id, out, m_dataDir);
}

bool StudentManager::SaveStudent(const Student &s) {
    return Persistence::SaveStudent(s, m_dataDir);
}

bool StudentManager::DeleteStudent(const std::string &id) {
    return Persistence::DeleteStudentFile(id, m_dataDir);
}
