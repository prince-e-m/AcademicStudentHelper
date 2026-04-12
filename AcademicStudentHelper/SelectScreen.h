#ifndef SELECTSCREEN_H
#define SELECTSCREEN_H

#include <iostream>
#include <string>
#include "Display.h"

using namespace std;

namespace Intro {
	inline void showSelectionScreen() {
		UI::printHeader("SELECT SCREEN");
		cout << "What do you want to calculate?\n";
		cout << "[1] TERM GRADE\n[2] FINAL COURSE GRADE\n[0] EXIT PROGRAM\nChoice: ";
	}
}