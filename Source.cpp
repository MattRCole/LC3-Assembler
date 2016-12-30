#include "Word.h"
#include "StringFun.h"
#include "Program.h"
#include "Labeler.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main() {
	fstream file;

	file.open("final.asm", fstream::in);

	if (file.fail()) {
		cout << "Error opening file";
		return 1;
	}


	Program* program = new Program(file);

	Program* steril = program->getSteril();

	Program& prog = *program;
	file.open("output.txt", fstream::out | fstream::trunc);

	if (file.fail()) {
		cout << "failed opening output.";
		return 0;
	}

	for (int i = 0; i < program->getSize(); i++) {
		for (int j = 0; j < prog[i].getWordCount(); j++) {
			file << prog[i][j].getWord() << " ";
		}
		file << endl;
	}

	file << endl << endl;

	delete program;



	Labeler* labeler = new Labeler(*steril);

	Program labeled = labeler->getLabeled();

	for (int i = 0; i < labeled.getSize(); i++) {
		file << stringOps::getStringFromNum(labeled[i].getMemLocation(), 16) << "\t";
		for (int j = 0; j < labeled[i].getWordCount(); j++) {
			file << labeled[i][j].getWord() << " ";
		}
		file << endl;
	}


	file.close();

	file.open("comp_test", fstream::out | fstream::binary);

	if (file.fail())
		cout << "The .obj test file has failed to open.\n";

	int* hexInt = new int;

	*hexInt = 0x3000;

	file.write((char*)hexInt, 2);

	*hexInt = 0x0001;

	file.write((char*)hexInt, 2);

	file.close();

	return 0;
}