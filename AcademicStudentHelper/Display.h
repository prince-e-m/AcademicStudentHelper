#ifndef DISPLAY_H
#define DISPLAY_H

#include <iostream>  // This should be at the TOP
#include <string>

using namespace std;

namespace UI {
    inline void printDivider() {
        cout << "==============================================\n";
    }

    inline void printSubDivider() {
        cout << "----------------------------------------------\n";
    }

    inline void printHeader(string title) {
        printDivider();
        cout << "      " << title << "      \n";
        printDivider();
    }

    inline void printInstanceDivider() {
        cout << "\n\n";
        cout << "##############################################\n";
        cout << "#           END OF CALCULATION               #\n";
        cout << "##############################################\n";
        cout << "\n";
    }
}

#endif
