
//#include "stdafx.h"
#include <fstream>
#include <iostream>

using namespace std;

void HTML_Parser(const char *inputFile, const char *outputFile) {
	ifstream fin(inputFile);
	if (!fin.is_open()) {
		return;
	}

	ofstream fout(outputFile);
	if (!fout.is_open()) {
		return;
	}

	char buff;
	fout << '"';
	bool nextString = false;
	while (fin.get(buff)) {
		if (buff == '"') {
			fout << '\\';
			fout << '"';
		} else if (buff == '\n') {
			fout << '"';
			fout << buff;
			nextString = true;
		} else if ((buff != ' ') && nextString) {
			fout << '"';
			fout << buff;
			nextString = false;
		} else {
			fout << buff;
		}
	}
	fout << '"';
	fin.close();
	fout.close();
}

int main() {
	const char *inputFileName = "..\\..\\input.txt";
	const char *outputFileName = "..\\..\\output.txt";
	HTML_Parser(inputFileName, outputFileName);
	return 0;
}
