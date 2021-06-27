
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
	char prevSymbol = 'a';

	while (fin.get(buff)) {
		if (buff == '"') {
			fout << '\\';
			fout << '"';
		} else if ((buff != '\n') && ((buff != ' ') && (prevSymbol != '>'))) {
			fout << buff;
		}
		prevSymbol = buff;
	}

	fin.close();
	fout.close();
}

int main() {
	const char *inputFileName = "D:\\Projects\\ESP 8266\\AmbientOrb\\HTML_Parser\\input.txt";
	const char *outputFileName = "D:\\Projects\\ESP 8266\\AmbientOrb\\HTML_Parser\\output.txt";
	HTML_Parser(inputFileName, outputFileName);
	return 0;
}
