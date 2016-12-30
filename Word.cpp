#include "Word.h"
using namespace std;

Word::Word(string word, word::Type type, int position, int decodeIndex, int integer)
{
	word_ = word;
	type_ = type;
	position_ = position;
	decodeIndex_ = decodeIndex;
	integer_ = integer;

	switch (type) {
	case(word::OPERATOR) :
		operator_ = true;
		operand_ = false;
		label_ = false;
		void_ = false;
		number_ = false;
		psudo_ = false;
		int_ = false;
		break;
	case(word::OPERAND) :
		operand_ = true;
		operator_ = false;
		label_ = false;
		void_ = false;
		number_ = false;
		psudo_ = false;
		int_ = false;
		break;
	case(word::LABEL) :
		label_ = true;
		operand_ = false;
		operator_ = false;
		void_ = false;
		number_ = false;
		psudo_ = false;
		int_ = false;
		break;
	case(word::NUMBER) :
		number_ = true;
		operator_ = false;
		operand_ = false;
		label_ = false;
		void_ = false;
		psudo_ = false;
		int_ = false;
		break;
	case(word::PSUDO) :
		psudo_ = true;
		void_ = false;
		operator_ = false;
		operand_ = false;
		label_ = false;
		number_ = false;
		int_ = false;
		break;
	case(word::STRING) :
		string_ = true;
		void_ = false;
		operator_ = false;
		operand_ = false;
		label_ = false;
		number_ = false;
		psudo_ = false;
		int_ = false;
		break;
	case(word::INTEGER) :
		int_ = true;
		operator_ = false;
		operand_ = false;
		label_ = false;
		void_ = false;
		number_ = false;
		psudo_ = false;
		break;
	default:
		void_ = true;
		operator_ = false;
		operand_ = false;
		label_ = false;
		number_ = false;
		psudo_ = false;
		int_ = false;
		break;
	}

}


int Word::size() {
	return word_.size();
}

bool operator==(string left, Word right) {
	if (right.getWord() == left)
		return true;

	return false;
}

bool operator==(Word left, string right) {
	if (left.getWord() == right)
		return true;

	return false;
}

bool operator!=(string left, Word right) {
	if (right.getWord() != left)
		return true;

	return false;
}

bool operator!=(Word left, string right) {
	if (left.getWord() != right)
		return true;

	return false;
}