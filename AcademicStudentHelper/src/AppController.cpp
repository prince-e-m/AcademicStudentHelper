#include "AppController.h"
#include "StudentManager.h"
#include "Persistence.h"
#include "GradeMath.h"
#include "Settings.h"
#include <filesystem>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <functional>
#include <cctype>
#include <sstream>
#include <iomanip>

static std::string inputLine(const std::string &prompt) {
    std::string s;
    std::cout << prompt;
    std::getline(std::cin, s);
    return s;
}

// forward declare helpers used by editCourse
static void waitForEnter(const std::string &message = "Press Enter to continue...");
static float inputFloat(const std::string &prompt, float minv = 0.0f, float maxv = 100.0f);

// Generic course editor used by guest and signed-in flows
static void editCourse(Course &c, std::function<void()> clearConsole) {
    while (true) {
        clearConsole();
        std::cout << "Editing course: " << c.name << "\n";
        std::cout << "[1] Edit Grades\n";
        std::cout << "[2] Edit Weights\n";
        std::cout << "[3] View Final\n";
        std::cout << "[4] Goal Calculator (includes safety-pass requirement)\n";
        std::cout << "[0] Back\n";
        std::cout << "Choice: ";
        int sub = -1; if (!(std::cin >> sub)) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); continue; }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (sub == 0) break;
        if (sub == 1) {
            c.grade.prelim = inputFloat("Enter prelim (0-100): ", 0.0f, 100.0f);
            c.grade.midterm = inputFloat("Enter midterm (0-100): ", 0.0f, 100.0f);
            c.grade.final = inputFloat("Enter final (0-100): ", 0.0f, 100.0f);
            std::cout << "Computed final: " << std::fixed << std::setprecision(2) << c.ComputeFinal() << "\n";
            std::cout << "Weights: P:" << std::fixed << std::setprecision(2) << (c.prelimWeight*100.0f)
                      << "% M:" << (c.midtermWeight*100.0f) << "% F:" << (c.finalWeight*100.0f) << "%\n";
            waitForEnter();
        }
        else if (sub == 2) {
            std::cout << "Enter weights as percentages (they will be normalized if not summing to 100).\n";
            float wp = inputFloat("Prelim weight (0-100): ", 0.0f, 100.0f);
            float wm = inputFloat("Midterm weight (0-100): ", 0.0f, 100.0f);
            float wf = inputFloat("Final weight (0-100): ", 0.0f, 100.0f);
            float sum = wp + wm + wf;
            if (sum <= 0.0f) {
                std::cout << "Invalid weights (sum zero).\n";
                waitForEnter();
                continue;
            }
            c.prelimWeight = wp / sum;
            c.midtermWeight = wm / sum;
            c.finalWeight = wf / sum;
            std::cout << "Weights updated.\n";
            waitForEnter();
        }
        else if (sub == 3) {
            std::cout << "Computed final: " << std::fixed << std::setprecision(2) << c.ComputeFinal() << "\n";
            std::cout << "Weights: P:" << std::fixed << std::setprecision(2) << (c.prelimWeight*100.0f)
                      << "% M:" << (c.midtermWeight*100.0f) << "% F:" << (c.finalWeight*100.0f) << "%\n";
            waitForEnter();
        }
        else if (sub == 4) {
            // Goal calculator: compute required final to pass (75) and to reach user-entered goal
            const float PASS_GOAL = Settings::LoadPassThreshold();
            float reqPass = GradeMath::RequiredFinalForGoal(c, PASS_GOAL);
            float goal = inputFloat("Enter desired final course grade (0-100, 0 to cancel): ", 0.0f, 100.0f);
            if (goal <= 0.0f) {
                std::cout << "Cancelled.\n";
                waitForEnter();
            }
            else {
                float reqGoal = GradeMath::RequiredFinalForGoal(c, goal);

                // Safety-pass result
                if (reqPass == FLT_MAX) {
                    std::cout << "PASS CHECK: Cannot compute required final: final weight is zero.\n";
                }
                else if (reqPass > 100.0f) {
                    std::cout << "PASS CHECK: To reach " << PASS_GOAL << "% you would need " << std::fixed << std::setprecision(2) << reqPass << "% on the final (IMPOSSIBLE >100%).\n";
                }
                else if (reqPass <= 0.0f) {
                    std::cout << "PASS CHECK: You already pass this course (no additional final points needed to reach " << PASS_GOAL << "%).\n";
                }
                else {
                    std::cout << "PASS CHECK: To reach " << PASS_GOAL << "% you need at least " << std::fixed << std::setprecision(2) << reqPass << "% on the final.\n";
                }

                // Goal result
                if (reqGoal == FLT_MAX) {
                    std::cout << "GOAL CHECK: Cannot compute required final: final weight is zero.\n";
                }
                else if (reqGoal > 100.0f) {
                    std::cout << "GOAL CHECK: To reach " << goal << "% you would need " << std::fixed << std::setprecision(2) << reqGoal << "% on the final (IMPOSSIBLE >100%).\n";
                }
                else if (reqGoal <= 0.0f) {
                    std::cout << "GOAL CHECK: You already achieved the goal (no additional final points needed).\n";
                }
                else {
                    std::cout << "GOAL CHECK: To reach " << goal << "% you need at least " << std::fixed << std::setprecision(2) << reqGoal << "% on the final.\n";
                }

                waitForEnter();
            }
        }
    }
}

// simple ANSI colors (works on most terminals)
static const char *C_RESET = "\x1b[0m";
static const char *C_HDR = "\x1b[36m"; // cyan
static const char *C_ACCENT = "\x1b[33m"; // yellow
static const char *C_GOOD = "\x1b[32m"; // green

// pagination helpers
static int selectStudentFromList(const std::vector<Student> &list, std::function<void()> clearConsole) {
    if (list.empty()) return -1;
    const size_t PAGE_SIZE = 5;
    size_t page = 0;
    while (true) {
        clearConsole();
        size_t start = page * PAGE_SIZE;
        size_t end = std::min(start + PAGE_SIZE, list.size());
        std::cout << C_HDR << "Profiles (page " << (page+1) << "/" << ((list.size()+PAGE_SIZE-1)/PAGE_SIZE) << ")" << C_RESET << "\n";
        for (size_t i = start; i < end; ++i) {
            std::cout << (i+1) << ": " << list[i].fullName << " [" << list[i].studentNumber << "]\n";
        }
        std::cout << C_ACCENT << "Commands: N=next, P=prev, Q=cancel, or enter index" << C_RESET << "\n";
        std::cout << "Choice: ";
        std::string inp; std::getline(std::cin, inp);
        if (inp.empty()) continue;
        if ((inp.size() == 1) && (std::toupper(inp[0]) == 'N')) {
            if ((page+1)*PAGE_SIZE < list.size()) ++page;
            continue;
        }
        if ((inp.size() == 1) && (std::toupper(inp[0]) == 'P')) {
            if (page > 0) --page;
            continue;
        }
        if ((inp.size() == 1) && (std::toupper(inp[0]) == 'Q')) return -1;
        // try parse index
        std::istringstream iss(inp);
        int num; if (iss >> num) {
            int idx = num - 1; // user-facing index starts at 1
            if (idx >= 0 && static_cast<size_t>(idx) < list.size()) return idx;
        }
        std::cout << "Invalid choice.\n";
    }
}

static int selectCourseFromList(const std::vector<Course> &list, std::function<void()> clearConsole) {
    if (list.empty()) return -1;
    const size_t PAGE_SIZE = 6;
    size_t page = 0;
    while (true) {
        clearConsole();
        size_t start = page * PAGE_SIZE;
        size_t end = std::min(start + PAGE_SIZE, list.size());
        std::cout << C_HDR << "Courses (page " << (page+1) << "/" << ((list.size()+PAGE_SIZE-1)/PAGE_SIZE) << ")" << C_RESET << "\n";
        for (size_t i = start; i < end; ++i) {
            const auto &ci = list[i];
            float wp = ci.prelimWeight * 100.0f;
            float wm = ci.midtermWeight * 100.0f;
            float wf = ci.finalWeight * 100.0f;
            std::cout << (i+1) << ": " << ci.name
                      << " | final: " << std::fixed << std::setprecision(2) << ci.ComputeFinal()
                      << std::fixed << std::setprecision(2)
                      << " (weights P:" << wp << "% M:" << wm << "% F:" << wf << "% )\n";
        }
        std::cout << C_ACCENT << "Commands: N=next, P=prev, Q=cancel, or enter index" << C_RESET << "\n";
        std::cout << "Choice: ";
        std::string inp; std::getline(std::cin, inp);
        if (inp.empty()) continue;
        if ((inp.size() == 1) && (std::toupper(inp[0]) == 'N')) {
            if ((page+1)*PAGE_SIZE < list.size()) ++page;
            continue;
        }
        if ((inp.size() == 1) && (std::toupper(inp[0]) == 'P')) {
            if (page > 0) --page;
            continue;
        }
        if ((inp.size() == 1) && (std::toupper(inp[0]) == 'Q')) return -1;
        std::istringstream iss(inp);
        int num; if (iss >> num) {
            int idx = num - 1;
            if (idx >= 0 && static_cast<size_t>(idx) < list.size()) return idx;
        }
        std::cout << "Invalid choice.\n";
    }
}

static void waitForEnter(const std::string &message) {
    std::cout << message;
    std::string tmp; std::getline(std::cin, tmp);
}

static float inputFloat(const std::string &prompt, float minv, float maxv) {
    while (true) {
        std::string s = inputLine(prompt);
        try {
            size_t pos = 0;
            float v = std::stof(s, &pos);
            if (pos != s.size()) throw std::invalid_argument("extra");
            if (v < minv || v > maxv) {
                std::cout << "Value must be between " << minv << " and " << maxv << ".\n";
                continue;
            }
            return v;
        }
        catch (...) {
            std::cout << "Invalid number. Please enter a numeric value.\n";
            continue;
        }
    }
}

static bool confirmPrompt(const std::string &prompt) {
    while (true) {
        std::cout << prompt << " (Y/N): ";
        std::string a; std::getline(std::cin, a);
        if (a.empty()) continue;
        char c = std::toupper(a[0]);
        if (c == 'Y') return true;
        if (c == 'N') return false;
    }
}

AppController::AppController(const std::string &dataDir) {
    // nothing here; will create manager in Run
}

void AppController::Run() {
    StudentManager mgr("data");
    bool firstRender = true;
    auto clearConsole = [&firstRender]() {
        // confirmation buffer: wait for Enter so user can read previous messages/errors
        if (!firstRender) {
            std::cout << "\n--- Press Enter to continue ---\n";
            std::string tmp; std::getline(std::cin, tmp);
        }
        firstRender = false;
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    };

    auto printFooter = [](const std::string &context) {
        std::cout << "\n" << "--- Legend: N=next P=prev Q=cancel | Indices start at 1 | Type number and Enter to select" << "\n";
    };

    auto confirmOverwriteIfExists = [&](const std::string &studentNumber)->bool {
        if (studentNumber.empty()) return true;
        auto p = std::filesystem::path("data") / (std::string("student_") + studentNumber + ".json");
        if (std::filesystem::exists(p)) {
            return confirmPrompt("A profile with this student number exists. Overwrite?");
        }
        return true;
    };

    while (true) {
        clearConsole();
        std::cout << "\n== Academic Student Helper ==\n";
        std::cout << "Mode:\n";
        std::cout << "[1] Guest (temporary)\n";
        std::cout << "[2] Sign In (open existing profile)\n";
        std::cout << "[3] Sign Up (create new profile)\n";
        std::cout << "[4] Delete a profile\n";
        std::cout << "[9] Reset all JSON data (delete all profiles)\n";
        std::cout << "[0] Exit\n";
        std::cout << "Select mode: ";

        int mode = -1;
        if (!(std::cin >> mode)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (mode == 0) break;

        if (mode == 9) {
            if (confirmPrompt("This will delete ALL saved profiles. Continue?")) {
                // create a timestamped backup folder
                auto now = std::time(nullptr);
                std::tm tm; localtime_s(&tm, &now);
                char buf[64];
                std::strftime(buf, sizeof(buf), "data_backup_%Y%m%d_%H%M%S", &tm);
                auto backupDir = std::filesystem::path(buf);
                if (!Persistence::BackupAllStudentFiles(std::filesystem::path("data"), backupDir)) {
                    std::cout << "Warning: backup failed. Aborting delete.\n";
                }
                else {
                    if (Persistence::DeleteAllStudentFiles(std::filesystem::path("data"))) {
                        std::cout << "All profiles deleted (backup at " << backupDir.string() << ").\n";
                        auto backups = Persistence::ListBackupDirs(std::filesystem::path("."));
                        std::cout << "Total backup folders: " << backups.size() << "\n";
                    }
                else std::cout << "Failed to delete profiles.\n";
            }
            }
            continue;
        }

        if (mode == 4) {
            // Delete single profile: list profiles and allow selection
            auto ids = mgr.ListStudents();
            if (ids.empty()) { std::cout << "No students found.\n"; waitForEnter(); continue; }
            std::vector<Student> loaded;
            for (auto &id : ids) {
                Student t; if (mgr.LoadStudent(id, t)) loaded.push_back(t);
            }
            int sel = selectStudentFromList(loaded, clearConsole);
            if (sel < 0) continue;
            auto &toDelete = loaded[sel];
            std::cout << "Selected: " << toDelete.fullName << " [" << toDelete.studentNumber << "]\n";
            if (!confirmPrompt("Backup this profile before deletion?")) {
                if (confirmPrompt("Delete without backup? This is irreversible.")) {
                    if (mgr.DeleteStudent(toDelete.id)) std::cout << "Deleted.\n"; else std::cout << "Delete failed.\n";
                }
            }
            else {
                // create timestamped per-profile backup dir
                auto now = std::time(nullptr);
                std::tm tm; localtime_s(&tm, &now);
                char buf[64];
                std::strftime(buf, sizeof(buf), "data_backup_%Y%m%d_%H%M%S", &tm);
                auto backupDir = std::filesystem::path(buf);
                if (Persistence::BackupStudentFile(toDelete.id, std::filesystem::path("data"), backupDir)) {
                    if (mgr.DeleteStudent(toDelete.id)) std::cout << "Deleted (backup created at " << backupDir.string() << ").\n"; else std::cout << "Delete failed.\n";
                    // report number of backups
                    auto backups = Persistence::ListBackupDirs(std::filesystem::path("."));
                    std::cout << "Total backup folders: " << backups.size() << "\n";
                }
                else {
                    std::cout << "Backup failed. Aborting delete.\n";
                }
            }
            waitForEnter();
            continue;
        }

        if (mode == 1) {
            // Guest session: temporary student in memory
            Student s;
            s.fullName = "Guest";
            s.studentNumber = "guest";
            std::cout << "Starting guest session. Changes are not saved unless you choose Save.\n";

            // reuse interactive student editor
            while (true) {
                clearConsole();
                std::cout << "\nGuest Session - Courses: " << s.courses.size() << "\n";
                std::cout << "[1] Add Course\n";
                std::cout << "[2] List Courses\n";
                std::cout << "[3] Edit Course Grades\n";
                std::cout << "[4] Save As Profile\n";
                std::cout << "[0] End Guest Session\n";
                std::cout << "Select: ";
                int gopt = -1; if (!(std::cin >> gopt)) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); continue; }
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                if (gopt == 0) break;
                if (gopt == 1) {
                    Course c; c.name = inputLine("Course name: "); s.courses.push_back(c); std::cout << "Course added.\n";
                }
                else if (gopt == 2) {
                    // Paginate and display courses; user can view pages
                    selectCourseFromList(s.courses, clearConsole);
                }
                else if (gopt == 3) {
                    if (s.courses.empty()) { std::cout << "No courses.\n"; continue; }
                    // Paginated selection for course index
                    int selIdx = selectCourseFromList(s.courses, clearConsole);
                    if (selIdx < 0) continue;
                    size_t idx = static_cast<size_t>(selIdx);
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); if (idx >= s.courses.size()) { std::cout << "Invalid index.\n"; continue; }
                    auto &c = s.courses[idx];
                    editCourse(c, clearConsole);
                }
                else if (gopt == 4) {
                    // Save guest as real profile
                    s.fullName = inputLine("Full name for profile: ");
                    s.studentNumber = inputLine("Student number (optional): ");
                    if (mgr.CreateStudent(s, false)) {
                        if (mgr.SaveStudent(s)) std::cout << "Profile saved.\n"; else std::cout << "Save failed.\n";
                    }
                    else std::cout << "Failed to create profile.\n";
                }
            }
        }
        else if (mode == 2) {
            // Sign In - open existing; list with indexes for easy selection
            auto ids = mgr.ListStudents();
            if (ids.empty()) { std::cout << "No students found.\n"; continue; }
            std::vector<Student> loaded;
            for (auto &id : ids) {
                Student t;
                if (mgr.LoadStudent(id, t)) loaded.push_back(t);
            }
            if (loaded.empty()) { std::cout << "No valid student profiles found.\n"; continue; }
            // Paginated selection of profiles
            int sel = selectStudentFromList(loaded, clearConsole);
            if (sel < 0) continue;
            Student s = loaded[sel];
            std::cout << "Loaded: " << s.fullName << "\n";

            // same editor as above (with save)
            while (true) {
                clearConsole();
                std::cout << "\nStudent: " << s.fullName << "\n";
                std::cout << "[1] Add Course\n";
                std::cout << "[2] List Courses\n";
                std::cout << "[3] Edit Course Grades\n";
                std::cout << "[4] Save Student\n";
                std::cout << "[0] Back\n";
                std::cout << "Select: ";
                int copt = -1; if (!(std::cin >> copt)) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); continue; }
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                if (copt == 0) break;
                if (copt == 1) { Course c; c.name = inputLine("Course name: "); s.courses.push_back(c); std::cout << "Course added.\n"; }
                else if (copt == 2) { selectCourseFromList(s.courses, clearConsole); }
                else if (copt == 3) { 
                    if (s.courses.empty()) { std::cout << "No courses.\n"; waitForEnter(); continue; } 
                    int selC = selectCourseFromList(s.courses, clearConsole); 
                    if (selC < 0) continue; 
                    size_t idx = static_cast<size_t>(selC); 
                    auto &c = s.courses[idx];
                    editCourse(c, clearConsole);
                }
                else if (copt == 4) { if (mgr.SaveStudent(s)) std::cout << "Saved.\n"; else std::cout << "Save failed.\n"; }
            }
        }
        else if (mode == 3) {
            // Sign Up - create new profile
            Student s;
            s.fullName = inputLine("Full name: ");
            s.studentNumber = inputLine("Student number (optional): ");
            if (mgr.CreateStudent(s, false)) {
                if (confirmPrompt("Save new student now?")) {
                    if (mgr.SaveStudent(s)) std::cout << "Saved.\n"; else std::cout << "Failed to save.\n";
                }
                else std::cout << "Student created in memory (not saved).\n";
            }
            else std::cout << "Failed to create student.\n";
        }
    }
}
