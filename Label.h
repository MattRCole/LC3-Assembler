#pragma once

#include <string>
#include <vector>
#include "Statement.h"
#include <exception>

using namespace std;

class Label {
public:
	Label(string = "", int = 0);
	Label(Word, int = 0);
	Label(Statement, int = 0);

	int getMem() { return memLocation_; }

	string getLabel() { return label_; }

	friend
		bool operator==(Word, Label);

	friend
		bool operator==(Label, Word);


private:
	int memLocation_;
	string label_;
};

class LabelException : public exception {
public:
	LabelException(const char* error = nullptr) :exception(error) {

	}
};
