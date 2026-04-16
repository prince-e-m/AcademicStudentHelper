#include "GradesTerminal.h"
#include <iostream>

using namespace std;

int getChoice() {
	int c;
	cout << "[1] Presets\n[2] Grades\n[0] Previous\nChoice: ";
	cin >> c;
	return c;
}

int getPresetDisplay() {
	int c;
	cout << "\nWhich preset would you like to do?\n";
	cout << "[1] Add Preset\n[2] Edit Preset\n[3] Delete Preset\n[4] View All Presets\n[5] Search Preset\n[0] Previous\nChoice: ";
	cin >> c;
	return c;
}

int getRecordGrade() {
	int c;
	cout << "\nWhich grade would you like to record?\n";
	cout << "[1] Prelim\n[2] Midterm\n[3] Final\n[0] Previous\nChoice: ";
	cin >> c;
	return c;
}