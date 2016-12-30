#include "Label.h"

using namespace std;

Label::Label(string label, int memLocation) {
	label_ = label;
	if (memLocation > 0xFDFF || memLocation < 0)
		throw(LabelException("LABEL MEMORY LOCATION OUT OF BOUNDS"));

	memLocation_ = memLocation;

}

Label::Label(Word word, int memLocation) {
	label_ = word.getWord();

	if (!word.isLabel())
		throw(LabelException("UNEXPECTED WORD, LABEL EXPECTED"));

	if (memLocation > 0xFDFF || memLocation < 0)
		throw(LabelException("LABEL MEMORY LOCATION OUT OF BOUNDS"));

	memLocation_ = memLocation;
}

Label::Label(Statement statement, int labelLocation) {
	if (!statement[labelLocation].isLabel()) {
		throw(LabelException("UNEXPECTED WORD, LABEL EXPECTED"));
	}

	if(statement.getMemLocation() > 0xFDFF || statement.getMemLocation() < 0)
		throw(LabelException("LABEL MEMORY LOCATION OUT OF BOUNDS"));

	label_ = statement[labelLocation].getWord();

	memLocation_ = statement.getMemLocation();

}


//Overloaded boolean operators.


bool operator==(Word word, Label label) {
	if (word.getWord() == label.label_)
		return true;

	return false;
}

bool operator==(Label label, Word word) {
	if (word.getWord() == label.label_)
		return true;

	return false;
}

bool operator>(Label label, int integer) {
	if (label.getMem() > integer)
		return true;

	return false;
}

bool operator>(int integer, Label label) {
	if (integer > label.getMem())
		return true;
	return false;
}

bool operator>=(Label label, int integer) {
	if (label.getMem() >= integer)
		return true;

	return false;
}

bool operator>=(int integer, Label label) {
	if (integer >= label.getMem())
		return true;
	return false;
}

bool operator<(Label label, int integer) {
	if (label.getMem() < integer)
		return true;

	return false;
}

bool operator<(int integer, Label label) {
	if (integer < label.getMem())
		return true;
	return false;
}

bool operator<=(Label label, int integer) {
	if (label.getMem() <= integer)
		return true;

	return false;
}

bool operator<=(int integer, Label label) {
	if (integer <= label.getMem())
		return true;
	return false;
}

bool operator==(Label label, int integer) {
	if (label.getMem() == integer)
		return true;

	return false;
}

bool operator==(int integer, Label label) {
	if (integer == label.getMem())
		return true;
	return false;
}
