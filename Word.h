#pragma once
#include <string>

using namespace std;
/* Written by Matthew Cole

  The word object will define if the contained string is an operation, operand or label.
  It is an object because it will not allow itself to be both an operator and operand ect.
  
  */

namespace word {
	enum Type { VOID, OPERATOR, OPERAND, LABEL, NUMBER, PSUDO, STRING };
}

class Word{
private:
	string word_;
	
	bool operator_;
	bool operand_;
	bool label_;
	bool void_;
	bool number_;
	bool psudo_;
	bool string_;

	word::Type type_;

	int position_;
	int decodeIndex_;
public:
	
	friend
		bool operator==(string left, Word right);
	friend
		bool operator==(Word left, string right);
	friend
		bool operator!=(string left, Word right);
	friend
		bool operator!=(Word left, string right);
	
	//Default constructor.
	Word(string word = "", word::Type type = word::VOID, int position = 0, int decodeIndex = -1);

	word::Type getWordType() { return type_; }

	string getWord() { return word_; }

	bool isOperand() { return operand_; }

	bool isOperator() {	return operator_; }

	bool isLabel() { return label_; }

	bool isVoid() { return void_; }

	bool isNumber() { return number_; }

	bool isPsudo() { return psudo_; }

	bool isString() { return string_; };

	void setDec(int decodeIndex) { decodeIndex_ = decodeIndex; };

	int getDec() { return decodeIndex_; }

	int getPosition() { return position_; };

	int size();


	/*Overloaded operators */


	//acts as a string::at(int index) would act.
	char at(int index) { return word_.at(index); }

	char operator[](int index) { return word_[index]; }
};