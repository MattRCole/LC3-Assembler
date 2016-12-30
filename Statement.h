#pragma once
#include <string>
#include <exception>
#include "Word.h"
#include "Linker.h"
#include "GlobalVars.h"
#include <vector>

using namespace std;
/*Written by Matthew Cole

The statement object will recieve a string of characters, parse the operators, operands, numbers and labels from the given
string returning a valid word or throwing an interpreter error
*/



class Statement {
public:

	//construct statement by string. doubles as default constructor.
	Statement(string statement = "", int = 0, int = 0x3000);

	Statement(Statement&);

	~Statement();

	/*if the next word does not exist, eos = true, else eos = false*/
	bool eos() { return eos_; }

	/*Returns the index of the next word. first index = 0, last index = wordCount - 1*/
	int getNextWordPosition() {	return wordPosition_; }


	//returns the line number of the given statement
	int getLineNum() { return lineNumber_; }
	
	/*Changes the current position of the next word. Throws StatementOOB error if index is out of bounds*/
	void changeGetPosition(int index);

	/*Places the current position of the next word to the beginning word. eos_ is reevaluated*/
	void gbeginning() {
		wordPosition_ = 0;
		if (wordCount_ > 0)
			eos_ = false;
		else
			eos_ = true;
	}

	/*Gets the word at a given position, does not change current wordPosition
	  Throws StatementOOB error if given index is out of bounds  */
	Word& getWordAt(int index);


	/*Deletes the word held at index index. 
	  Performs bounds checking and throws StatementOOB.
	  eos_ is reevaluated*/
	void deleteWordAt(int index);

	/*Overloaded operator[], performs bounds checking and throws StatementOOB*/
	Word& operator[](int index);
	
	/*returns the current word count*/
	int getWordCount() {
		return wordCount_;
	}

	

	/*Returns true if the current Statement contains no words*/
	bool isEmpty() {
		if (wordCount_ == 0)
			return true;

		return false;
	}
	
	/*returns word::Type at given index. Throws StatementOOB*/
	word::Type getWordTypeAt(int index);

	/*get the next word in the statement, throws StatementOOB*/
	Word& getNextWord();

	/*returns the memory location of the current statement*/
	int getMemLocation() {
		return memLocation_;
	}

	void replaceWordAt(int, Word*);

	void setMemLocation(int);


	/*Will clear all words in the wordList and set eos as true*/
	void clearWords();

	/*Will add word to the end of current wordList. eos is re-evaluated*/
	void addWord(Word*);

private:

	bool eos_;
	int wordPosition_;
	int wordCount_;
	int lineNumber_;
	int memLocation_;
	vector<Word*> wordList_;


	//Member functions!

	//will determine the type and decode index for a given word
	word::Type determineType(string, int&);

	//will determine if the character is a currently accepted linker command prefix. 
	bool isLinker(char);

	/*Will accept a string and parse out words, determening word type and location in the line of the program.*/
	void interpret(string);

	/*Strings containing nested strings will need to be corrected and accurately recreated as strings. This function will
	  return an accurately parsed string with correct escape characters.*/
	void parseEscapes(string& toParse);
};

class StatementError : public exception {
public:
	StatementError(char* errorName = nullptr) : exception(errorName) {
	}
};

class StatementOOB : public exception {
public:
	StatementOOB(char* errorName = nullptr) : exception(errorName) {
	}
};