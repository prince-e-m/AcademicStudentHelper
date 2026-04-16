#pragma once

#include <string>
#include <vector>
#include "../third_party/json.hpp"

using nlohmann::json;

struct GradeEntry {
    float prelim = 0.0f;
    float midterm = 0.0f;
    float final = 0.0f;
};

struct Course {
    std::string id;
    std::string name;
    float prelimWeight = 0.333f;
    float midtermWeight = 0.333f;
    float finalWeight = 0.334f;
    GradeEntry grade;

    float ComputeFinal() const {
        return grade.prelim * prelimWeight + grade.midterm * midtermWeight + grade.final * finalWeight;
    }
};

struct ChangeRecord {
    std::string timestamp;
    std::string action;
    std::string details;
};

struct Student {
    std::string id;
    std::string fullName;
    std::string studentNumber;
    std::vector<Course> courses;
    std::vector<ChangeRecord> history;
};

// Use nlohmann::adl_serializer specializations to avoid free-function conflicts
namespace nlohmann {

template<>
struct adl_serializer<GradeEntry> {
    static void to_json(json &j, const GradeEntry &g) {
        j = json{{"prelim", g.prelim}, {"midterm", g.midterm}, {"final", g.final}};
    }
    static void from_json(const json &j, GradeEntry &g) {
        if (j.contains("prelim")) j.at("prelim").get_to(g.prelim);
        if (j.contains("midterm")) j.at("midterm").get_to(g.midterm);
        if (j.contains("final")) j.at("final").get_to(g.final);
    }
};

template<>
struct adl_serializer<Course> {
    static void to_json(json &j, const Course &c) {
        j = json{
            {"id", c.id},
            {"name", c.name},
            {"weights", {{"prelim", c.prelimWeight}, {"midterm", c.midtermWeight}, {"final", c.finalWeight}}},
            {"grade", c.grade}
        };
    }
    static void from_json(const json &j, Course &c) {
        if (j.contains("id")) j.at("id").get_to(c.id);
        if (j.contains("name")) j.at("name").get_to(c.name);
        if (j.contains("weights")) {
            const auto &w = j.at("weights");
            if (w.contains("prelim")) w.at("prelim").get_to(c.prelimWeight);
            if (w.contains("midterm")) w.at("midterm").get_to(c.midtermWeight);
            if (w.contains("final")) w.at("final").get_to(c.finalWeight);
        }
        if (j.contains("grade")) j.at("grade").get_to(c.grade);
    }
};

template<>
struct adl_serializer<ChangeRecord> {
    static void to_json(json &j, const ChangeRecord &r) {
        j = json{{"timestamp", r.timestamp}, {"action", r.action}, {"details", r.details}};
    }
    static void from_json(const json &j, ChangeRecord &r) {
        if (j.contains("timestamp")) j.at("timestamp").get_to(r.timestamp);
        if (j.contains("action")) j.at("action").get_to(r.action);
        if (j.contains("details")) j.at("details").get_to(r.details);
    }
};

template<>
struct adl_serializer<Student> {
    static void to_json(json &j, const Student &s) {
        j = json{
            {"id", s.id},
            {"fullName", s.fullName},
            {"studentNumber", s.studentNumber},
            {"courses", s.courses},
            {"history", s.history}
        };
    }
    static void from_json(const json &j, Student &s) {
        if (j.contains("id")) j.at("id").get_to(s.id);
        if (j.contains("fullName")) j.at("fullName").get_to(s.fullName);
        if (j.contains("studentNumber")) j.at("studentNumber").get_to(s.studentNumber);
        if (j.contains("courses")) j.at("courses").get_to(s.courses);
        if (j.contains("history")) j.at("history").get_to(s.history);
    }
};

} // namespace nlohmann
