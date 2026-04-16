#pragma once

#include <string>

class AppController {
public:
    explicit AppController(const std::string &dataDir);
    void Run();
};
