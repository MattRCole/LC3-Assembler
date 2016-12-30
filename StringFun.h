#pragma once
#include <string>
#include "Word.h"
#include "GlobalVars.h"


using namespace std;

/*Static container class for static functions only. Deals with conversion from string to number or vice-versa. does not throw exceptions*/
static class stringOps{
public:

	/*Accepts an int in any bounds and will return, based on the radix given, an appropriate string representing the number in the given
	radix or base. If the given number is negative, the negation symbol '-' will be included preceeding the most significant character
	or number in the string.  */
	static string getStringFromNum(int num, int radix) {
		//function variables including the ascii-offset that will give us charachters 0 through 9
		// as well as the offset that will give us characters A-Z if necesary for the radix 
		const int NUM_OFFSET = 48;
		const int CHAR_OFFSET = 55;
		bool found = false;
		bool negative = false;
		int remainder = 0;
		string numString = "";

		//this is a no exception function. If ill-used it will simply return 0
		if (radix < 1 || radix > 36)
			return "0";

		//if the number is negative, we will invert the sign and add the negation symbol
		// after the full number has been converted to string.
		if (num < 0) {
			num *= -1;
			negative = true;
		}

		//if the number given is equal to zero, we will simply return zero.
		if (num == 0) {
			return "0";
		}

		//Through integer modulous and integer division, the individual number characters will be parsed
		// and added to the string
		while (num != 0)
		{
			remainder = num % radix;
			num = num / radix;
			if (remainder < 10)
				numString = static_cast<char>(remainder + NUM_OFFSET) + numString;
			else
				numString = static_cast<char>(remainder + CHAR_OFFSET) + numString;
		}

		if (negative) {
			numString = "-" + numString;
		}

		return numString;
	}

	/*Precondition: word::Type == NUMBER
	Postcondition: a valid long integer will
	be returned. Does not throw exceptions.*/
	static long getNumberFromWord(Word word) {
		if (word == "")
			return 0;

		//get a substring w/o the prefix of the number.
		string strNum = word.getWord().substr(1);
		long number = 0x3000;

		if (strNum == "")
			return 0;

		//make sure word is number, if not, return zero. 
		if (!word.isNumber())
			return 0;

		using namespace number;

		switch (word.getDec()) {
		case(ASSUMED_DIG) :
			number = strtol(word.getWord().c_str(), nullptr, 10);
			break;
		case(HEX) :
			number = strtol(strNum.c_str(), nullptr, 16);
			break;
		case(BIN) :
			number = strtol(strNum.c_str(), nullptr, 2);
			break;
		case(DIG) :
			number = strtol(strNum.c_str(), nullptr, 10);
			break;
		default:
			//the default case will be to return zero
			number = 0;
			break;
		};

		return number;
	}
};