#include "TermManager.h"
#include <iostream>

using namespace std;

int getFinishedTerm() {
    int t;
    cout << "\nWhich term did you JUST finish?\n";
    cout << "[1] Prelims\n[2] Midterms\n[0] Previous\nChoice: ";
    cin >> t;
    return t;
}

//this function validates for numerical input between 0-100 only.
float getValidatedGrade(string prompt, float min, float max) {
    float value;

    while (true) {
        cout << prompt;

        if (cin >> value) {

            if (value >= min && value <= max) {
                return value;
            }

            cout << "Input must be between "
                << min << " and " << max << ".\n";
        }
        else {
            cout << "Invalid input. Please enter a number.\n";
            cin.clear();
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}