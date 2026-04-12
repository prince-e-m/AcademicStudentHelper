#include <iostream>
#include <iomanip>
#include <limits>
#include "GradesTerminal.h"
#include "TermManager.h"
#include "GradeMath.h"
#include "Display.h"

using namespace std;

const float PRELIM_WEIGHT = 0.333f;
const float MIDTERM_WEIGHT = 0.333f;
const float FINAL_WEIGHT = 0.334f;

float getValidGoalGrade() {
    float grade;

    while (true) {
        cout << "Enter Goal Grade (0 to skip): ";

        if (cin >> grade) {
            if (grade >= 0 && grade <= 100) {
                return grade;
            }
            cout << "Grade must be between 0 and 100.\n";
        }
        else {
            cout << "Invalid input. Please enter a number.\n";
            cin.clear();
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

int main() {
    //Initial menu
    initial_menu:
    int option = 0;
    do {
        system("cls");
        UI::printHeader("TIP QC ACADEMIC SYSTEM");
        cout << "\n[1] Term Course Grade\n[2] Grade Goal Setter\n[0] Exit\nSelect an option: ";

        if (!(cin >> option)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        switch (option) {

        case 1: {
            //Grade Calculator for Term
            system("cls");
			bool running = true;
            UI::printHeader("TIP QC ACADEMIC SYSTEM");

            while (running) {
                system("cls");
                UI::printHeader("TIP QC ACADEMIC SYSTEM");
                cout << "\n";
               int choice = getChoice();
                if (choice == 0) {
                    break;
                }
                else if (choice == 1) {
                    system("cls");
                    UI::printHeader("TIP QC ACADEMIC SYSTEM");
					int path = getPresetDisplay(); //Path for displaying different grading systems.
                }
                else if (choice == 2) {
                    system("cls");
                    UI::printHeader("TIP QC ACADEMIC SYSTEM");
                    int path = getRecordGrade(); //Path for saving grades and scores.
                }
                else {
                    system("cls");
                    cout << "Invalid choice. Please select again.\n";
                }
                cout << "\nPress Enter to continue...";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
				cin.get();
                  
            }
            break;
        }

        case 2: {
            cout << "Invalid option selected. Please try again.\n";
            //Final Course Grade Dashboard
			system("cls");
            bool running = true;

            UI::printHeader("TIP QC ACADEMIC SYSTEM");

            while (running) {

                int term = getFinishedTerm();

                if (term == 0) {
                    break;
                }

                if (term < 0 || term > 2) {
                    cout << "Invalid term selection.\n";
                    continue;
                }

                float p = 0.0f;
                float m = 0.0f;
                float target = 0.0f;

                if (term == 1) {
                    p = getValidatedGrade("Enter Prelim Grade: ", 0, 100);
                }
                else if (term == 2) {
                    p = getValidatedGrade("Enter Prelim Grade: ", 0, 100);
                    m = getValidatedGrade("Enter Midterm Grade: ", 0, 100);
                }

                target = getValidatedGrade("Enter Goal Grade (0 to skip): ", 0, 100);

                GradeReport report = calculateAcademicStatus(p, m, target);

                UI::printHeader("ACADEMIC DASHBOARD");

                cout << fixed << setprecision(2);

                cout << "[1] PREDICTION\n";
                cout << "    Projected Final Grade: " << report.projectedGrade << "%\n\n";

                if (target > 0) {
                    cout << "[2] GRADE GOAL MAKER (Target: " << target << "%)\n";

                    if (term == 1) {
                        cout << "    Required average for Midterms & Finals : " << report.neededForGoalMidterms << "%\n\n";
                    }
                    else {
                        cout << "    Required in Finals: " << report.neededForGoalFinals << "%\n\n";
                    }

                }

                cout << "[3] MINIMUM TO PASS (75%)\n";

                if (term == 1) {

                    float requiredAverage =
                        (75.0f - (p * PRELIM_WEIGHT)) /
                        (MIDTERM_WEIGHT + FINAL_WEIGHT);

                    if (requiredAverage > 100) {
                        cout << "    Impossible to pass this term.\n";
                    }
                    else {
                        cout << "    Required Avg for Midterms & Finals: "
                            << requiredAverage << "%\n";
                    }
                }
                else {

                    if (report.neededToPass > 100) {
                        cout << "    Impossible to pass this term.\n";
                    }
                    else {
                        cout << "    Required in Finals: "
                            << report.neededToPass << "%\n";
                    }
                }

                UI::printSubDivider();

                cout << "[4] SAFETY NET\n";

                const float FAILSAFE_FINAL = 60.0f;

                if (term == 1) {

                    cout << "    To survive a 60.00 in Finals, you need\n";
                    cout << "    a Midterm grade of: "
                        << report.safetyNetMidterm << "%\n";
                }
                else {

                    if (report.neededToPass <= FAILSAFE_FINAL) {
                        cout << "    SAFE: You pass even with a 60.00 in Finals.\n";
                    }
                    else {
                        cout << "    DANGER: A 60.00 in Finals will result in failure.\n";
                    }
                }

                UI::printInstanceDivider();

                char again;

                cout << "\nRun another calculation? (Y/N): ";

                while (true) {

                    if (cin >> again) {

                        again = toupper(again);

                        if (again == 'Y' || again == 'N') break;
                    }

                    cout << "Please enter Y or N: ";

                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }

                if (again == 'N') {
                    goto initial_menu;
                }
            }
            break;
        }
        case 0:
            cout << "Exiting program. Good luck!\n";
            break;

        default:
            cout << "\n[" << option << "] is not a valid choice. Try again.\n";
            break;
        }

        if (option != 0) {
            cout << "\nPress Enter to return to Menu...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.get();
        }

    } while (option != 0);

            return 0;
        }