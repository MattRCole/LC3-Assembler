#include "Statement.h"
using namespace std;



/*Will populate self with apropriate words. With the exceptions of LABELS, LINKER and PSUDO words, a decode
index will be supplied to corrispond to the global indecies provided in the Statement.h file*/
Statement::Statement(string statement, int lineNumber, int memLocation)
{

	//initialize member variables
	lineNumber_ = lineNumber;
	memLocation_ = memLocation;
	wordCount_ = 0;
	eos_ = true;
	wordPosition_ = -1;
	
	interpret(statement);

	if (wordCount_ > 0) {
		eos_ = false;
		wordPosition_ = 0;
	}
}

Statement::Statement(Statement &statement)
{
	eos_ = statement.eos_;
	memLocation_ = statement.memLocation_;
	lineNumber_ = statement.lineNumber_;
	wordCount_ = statement.wordCount_;
	wordPosition_ = statement.wordPosition_;


	for (int i = 0; i < statement.wordCount_; i++) {
		wordList_.push_back(new Word(*(statement.wordList_.at(i))));
	}

}

Statement::~Statement()
{
	for (vector<Word*>::iterator iter = wordList_.begin(); iter < wordList_.end(); iter++) {
		if (*iter != nullptr) {
			delete *iter;
			*iter = nullptr;
		}
	}
}

void Statement::changeGetPosition(int index) {
	//check the bounds of the current index
	if (index > wordCount_ || index < 0)
		throw (StatementOOB("NEW GET INDEX OUT OF BOUNDS"));

	//change member variable
	wordPosition_ = index;

	if (wordPosition_ == wordCount_) {
		eos_ = true;
	}
	else {
		eos_ = false;
	}

	return;
}

Word& Statement::getWordAt(int index) {
	//check the bounds of the current index
	if (index > wordCount_ || index < 0)
		throw (StatementOOB("GET WORD INDEX OUT OF BOUNDS"));

	return *wordList_[index];
}

void Statement::deleteWordAt(int index)
{
	if (index < 0 || index >= wordCount_)
		throw(StatementOOB("DELETE WORD INDEX OUT OF BOUNDS"));

	vector<Word*>::iterator iter = wordList_.begin();

	for (int i = 0; i < index; i++)
		iter++;

	delete wordList_.at(index);

	wordList_.at(index) = nullptr;

	wordList_.erase(iter);

	wordCount_--;

	if (wordPosition_ >= wordCount_)
		eos_ = true;
}

Word& Statement::operator[](int index) {
	//check the bounds of the current index
	if (index > wordCount_ || index < 0)
		throw (StatementOOB("GET WORD INDEX OUT OF BOUNDS"));

	return *wordList_[index];
}

word::Type Statement::getWordTypeAt(int index) {
	//check the bounds of the current index
	if (index > wordCount_ || index < 0)
		throw (StatementOOB("GET WORD INDEX OUT OF BOUNDS"));

	word::Type type = wordList_[index]->getWordType();

	return type;
}

Word& Statement::getNextWord() {
	if (eos_ == true)
		throw(StatementOOB("WORD EXPECTED"));

	Word& toReturn = *wordList_[wordPosition_];
	
	wordPosition_++;
	
	if (wordPosition_ == wordCount_)
		eos_ = true;

	return toReturn;
}


void Statement::replaceWordAt(int index, Word* newWord) {
	if (index >= wordCount_)
		throw(StatementOOB("REPLACE INDEX OUT OF BOUNDS"));


	Word*& oldWord = wordList_.at(index);

	delete oldWord;

	oldWord = newWord;


	return;
}

void Statement::setMemLocation(int memLocation) {
	if (memLocation > 0xFDFF || memLocation < 0 )
		throw(StatementOOB("OUT OF ACCEPTED MEMORY BOUDNS"));

	memLocation_ = memLocation;
}

void Statement::clearWords()
{
	wordCount_ = 0;
	eos_ = true;
	wordPosition_ = -1;
	
	for (int i = 0; i < wordList_.size(); i++)
		if (wordList_.at(i) != nullptr)
			delete wordList_.at(i);

	wordList_.clear();
}

void Statement::addWord(Word *word)
{
	wordList_.push_back(word);

	wordCount_++;
	if (wordPosition_ < wordCount_)
		eos_ = false;
}

word::Type Statement::determineType(string word, int& index) {
	//Define function variables
	bool found = false;
	word::Type type = word::VOID;

	//the basic formula for determining the type is to iterate through the currently
	// accepted operators, operands, number prefixes and determine type based on if 
	// a match is found.
	for (int i = 0; i < opcode::COUNT && !found; i++) {
		//special case for BR, because BR can be followed by any combination of 'n', 'z', and or 'p', we will
		// search soully for the string "BR"
		if (i == static_cast<int>(opcode::BR) && word.size() > 1 && word.substr(0,2) == "BR") {
			type = word::OPERATOR;
			found = true;
			index = i;
		}
		else if (opcode::STRING[i] == word) {
			type = word::OPERATOR;
			found = true;
			index = i;
		}
	}
	for (int i = 0; i < operand::COUNT && !found; i++) {
		if (operand::STRING[i] == word) {
			type = word::OPERAND;
			found = true;
			index = i;
		}
	}
	//Special case: Assumed digits, we will check the first charcter to see if it 
	// a digit. If so, it will be assumed as a base 10 number.
	if (word.at(0) >= '0' && word.at(0) <= '9') {
		type = word::NUMBER;
		found = true;
		index = 0;
	}
	for (int i = 1; i < number::COUNT && !found; i++) {
		if (number::STRING[i] == word.at(0)) {
			type = word::NUMBER;
			found = true;
			index = i;
		}
	}
	//special case: Psudo ops, since these are easy to determine, we will simply check the first char of word.
	if (!found && word.at(0) == '.') {
		type = word::PSUDO;
		found = true;
		index = -1;
	}
	//we will assume that the word is a label if none of the previous conditions are met.
	if (!found && word != "") {
		type = word::LABEL;
		found = true;
		index = -1;
	}
	else if(!found) {
		type = word::VOID;
		found = true;
		index = -1;
	}

	return type;

}

bool Statement::isLinker(char prefix) {
	for (int i = 0; i < LINKER_PREFIX_CNT; i++) {
		if (LINKER_PREFIX[i] == prefix)
			return true;
	}
	return false;
}

void Statement::interpret(string statement) {
	//Define function variables
	bool eos = false;
	bool getString = false;
	int position = 0;
	int wordPosition = 0;
	int decodeIndex = -1;
	int checkEscape = -2;
	word::Type type = word::VOID;
	string nextWord = "";

	if (statement.length() <= 0) {
		eos = true;
	}


	//DEBUGGING PURPOSES
	char nextChar = '\0';


	while (!eos && position < statement.length()) {
		//for ease of debugging
		nextChar = statement.at(position);
		
		//check to see if we are currently getting a string
		if (getString) {
			if (nextChar == '\"') {
				checkEscape = nextWord.size() - 2;
				//We must check one of two cases: If the current '\"' char is meant to be included in the string.
				// Two special cases will be tested: we are ending the string if: the preceding character was not the escape character, or if the preceding two characters were escape characters.
				if (
					(checkEscape > -2 && nextWord.at(nextWord.size() - 1) != '\\') 
					||
					(checkEscape > -1 && nextWord.at(nextWord.size() - 1) == '\\' && nextWord.at(nextWord.size() - 2) == '\\')
					) {
					parseEscapes(nextWord);
					type = word::STRING;
					wordList_.push_back(new Word(nextWord, type, position - nextWord.length()));
					wordCount_++;
					nextWord = "";
					getString = false;
				}

			}
			else {
				nextWord = nextWord + nextChar;
			}
			position++;
		}
		//check to see if we are getting a string value
		else if (nextChar == '\"') {
			getString = true;
			//perform the usual word check. If the word is not blank, it will be inserted into the word list.
			if (nextWord != "") {
				//determine word type
				type = determineType(nextWord, decodeIndex);
				//push new word onto the wordList_ vector.
				wordList_.push_back(new Word(nextWord, type, position - nextWord.length(), decodeIndex));
				wordCount_++;
				nextWord = "";
				decodeIndex = -1;
			}
			position++;
		} 
		//any valid "space" characters will be ignored
		else if (nextChar == ' ' || nextChar == ',' || nextChar == '\t') {
			//determine if a word needs to be added to wordList_
			if (nextWord != "") {
				type = determineType(nextWord, decodeIndex);
				wordList_.push_back(new Word(nextWord, type, position - nextWord.length(), decodeIndex));
				wordCount_++;
				nextWord = "";
				decodeIndex = -1;
			}
			position++;
		}
		//if a comment is being made, it will be ignored by the statement object.
		else if (nextChar == ';') {
			eos = true;
			if (nextWord != "") {
				type = determineType(nextWord, decodeIndex);
				wordList_.push_back(new Word(nextWord, type, position - nextWord.length(), decodeIndex));
				wordCount_++;
				nextWord = "";
				decodeIndex = -1;
			}
		}
		//iterate through accepted Linker Prefixes and terminate statement if necesary
		else if (isLinker(nextChar)) {
			eos = true;
			if (nextWord != "") {
				type = determineType(nextWord, decodeIndex);
				wordList_.push_back(new Word(nextWord, type, position - nextWord.length(), decodeIndex));
				wordCount_++;
				nextWord = "";
				decodeIndex = -1;
			}
		}
		else {
			nextWord = nextWord + nextChar;
			position++;
		}
	}

	//clear next word and add to wordList_ if applicable
	if (nextWord != "") {
		type = determineType(nextWord, decodeIndex);
		wordList_.push_back(new Word(nextWord, type, position - nextWord.length(), decodeIndex));
		wordCount_++;
	}

}

void Statement::parseEscapes(string& toParse) {
	int nextEscape = -1;
	string subLeft = "";
	string subRight = "";


	//broken for now. Look up things on the interwebs. Desired results not being achieved.
	while ((nextEscape = toParse.find('\\', nextEscape + 1)) >= 0) {
		if (toParse.at(nextEscape + 1) == 't') {
			if (nextEscape > 0)
				subLeft = toParse.substr(0, nextEscape);
			if (nextEscape < (toParse.size() - 1))
				subRight = toParse.substr(nextEscape + 2, toParse.size() - 1);

			toParse = subLeft + "\t" + subRight;

			subLeft = "";
			subRight = "";
		}
		else if (toParse.at(nextEscape + 1) == '\'') {
			if (nextEscape > 0)
				subLeft = toParse.substr(0, nextEscape);
			if (nextEscape < (toParse.size() - 1))
				subRight = toParse.substr(nextEscape + 2, toParse.size() - 1);

			toParse = subLeft + "\'" + subRight;

			subLeft = "";
			subRight = "";
		}
		else if (toParse.at(nextEscape + 1) == '\"') {
			if (nextEscape > 0)
				subLeft = toParse.substr(0, nextEscape);
			if (nextEscape < (toParse.size() - 1))
				subRight = toParse.substr(nextEscape + 2, toParse.size() - 1);

			toParse = subLeft + "\"" + subRight;

			subLeft = "";
			subRight = "";
		}
		else if (toParse.at(nextEscape + 1) == '\\') {
			if (nextEscape > 0)
				subLeft = toParse.substr(0, nextEscape);
			if (nextEscape < (toParse.size() - 1))
				subRight = toParse.substr(nextEscape + 2, toParse.size() - 1);

			toParse = subLeft + "\\" + subRight;

			subLeft = "";
			subRight = "";
		}
		else if (toParse.at(nextEscape + 1) == 'n') {
			if (nextEscape > 0)
				subLeft = toParse.substr(0, nextEscape);
			if (nextEscape < (toParse.size() - 1))
				subRight = toParse.substr(nextEscape + 2, toParse.size() - 1);

			toParse = subLeft + "\n" + subRight;

			subLeft = "";
			subRight = "";
		}
	} 
}