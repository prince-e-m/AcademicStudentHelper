#include <iostream>
#include "src/AppController.h"

int main() {
    try {
        AppController app("data");
        app.Run();
    }
    catch (const std::exception &ex) {
        std::cerr << "Fatal error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}