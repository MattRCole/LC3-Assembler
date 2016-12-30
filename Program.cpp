#include "Program.h"
#include "StringFun.h"
#include <string>
using namespace std;

Program::Program(fstream& fileInput) {
	//initialize member objects and variables and call necesary functions.
	statementList_ = new vector<Statement*>();
	inputProgram(fileInput);

	steril_ = false;
	labeled_ = false;
}

Program::Program(Program & that)
{
	size_ = that.size_;
	statementList_ = new vector<Statement*>();

	for (int i = 0; i < size_; i++) {
		statementList_->push_back(new Statement(*(that.statementList_->at(i))));
	}

	steril_ = that.steril_;
	labeled_ = that.labeled_;
}

Program::Program(vector<Statement*>* statementList, int size) {
	statementList_ = statementList;

	size_ = size;

	sterilize();

	steril_ = true;

	labeled_ = false;
}

Program::Program() {
	statementList_ = nullptr;

	size_ = 0;

	steril_ = false;

	labeled_ = false;

}

Program::~Program() {
	for (int i = 0; i < size_; i++) {
		if (statementList_->at(i) != nullptr) {
			delete statementList_->at(i);
			statementList_->at(i) = nullptr;
		}
	}
	delete statementList_;

	statementList_ = nullptr;
}

Statement& Program::operator[](int index) {
	if (index >= size_)
		throw(ProgramOOB("INDEX OUT OF BOUNDS"));

	return *(statementList_->at(index));
}

Statement& Program::getStatementAt(int index){
	if (index >= size_)
		throw(ProgramOOB("INDEX OUT OF BOUNDS"));

	return *(statementList_->at(index));
}

void Program::inputProgram(fstream& fileInput) {
	size_ = 0;
	if (!fileInput.is_open())
		throw(ProgramException("UNABLE TO READ FILE"));

	statementList_->clear();

	string nextStatement = "";
	int lineNum = 0;
	//iterate through file and read in lines to new statements.
	while (!fileInput.eof()) {

		getline(fileInput, nextStatement);

		//see statement initilization for string parsing.
		statementList_->push_back(new Statement(nextStatement, lineNum));

		nextStatement = "";
		lineNum++;
		size_++;

	}

	//close the file.
	fileInput.close();
}

void Program::deleteAt(int index)
{
	if (index > size_) {
		throw(ProgramOOB("DELETE INDEX OUT OF BOUNDS"));
	}
	vector<Statement*>::iterator iter = statementList_->begin();

	//seriously. I am doing something wrong. This is ridiculous...
	for (int i = 0; i < index; i++) {
		iter++;
	}
	//there we go.
	delete statementList_->at(index);
	statementList_->erase(iter);

	size_--;
}

void Program::insertAt(int index, Statement* statement)
{
	if (index < 0)
		throw(ProgramOOB("INVALID INSERT INDEX"));

	if (statement == nullptr || statement == NULL)
		throw(ProgramException("NULL STATEMENT INSERTION ATTEMPTED"));
	try {
		vector<Statement*>::iterator iter = statementList_->begin();

		//if there's a better way to do this, I'm all ears.
		for (int i = 0; i < index; i++) {
			iter++;
		}

		statementList_->insert(iter, statement);
		
	}
	catch (bad_alloc) {
		throw(ProgramException("MEMORY EXAUSTED", statement));
	}
}

//Precondition: Program is not steril
Program* Program::getSteril() {
	if (steril_)
		throw(ProgramException("PROGRAM IS STERIL"));

	//deep copy statementList_.
	vector<Statement*>* toSterilize = new vector<Statement*>();

	for (int i = 0; i < size_; i++) {
		toSterilize->push_back(new Statement(*(statementList_->at(i))));
	}

	//construct new program to be sterilized and return it.
	return new Program(toSterilize, size_);
}

void Program::sterilize() {
	//Initialize function variables
	int programLocation = 0;
	bool found = false;
	Statement* statement = nullptr;
	Word searchFor;
	Word memoryStartNum;

	//iterate through statement list searching for the .ORIG psudo op which will give the starting location of the program
	// in memory.
	for (int i = 0; i < statementList_->size() && !found; i++) {
		statement = statementList_->at(i);
		while (!statement->eos())
		{
			searchFor = statement->getNextWord();
			if (searchFor.isPsudo() && searchFor == ".ORIG") {
				found = true;
				break;
			}
		}
	}

	if (!found)
		throw(ProgramException(".ORIG COMMAND EXPECTED AND NOT FOUND"));

	if (statement->eos())
		throw(ProgramException("MEMORY ORIGIN NUMBER EXPECTED", statement));

	memoryStartNum = statement->getNextWord();

	if (!memoryStartNum.isNumber())
		throw(ProgramException("UNEXPECTED WORD, MEMORY ORIGIN NUBMER EXPECTED", statement, statement->getNextWordPosition() - 1));
	
	//recieve number based on given number
	long memStart = stringOps::getNumberFromWord(memoryStartNum);

	//delete all blank statements (comments or empty lines) including .ORIG statement
	deleteAt(statement->getLineNum());

	vector<Statement*>::iterator iterator = statementList_->begin();
	int i = 0; //we have to do this because... I don't know how to use iterators.
	while (iterator < statementList_->end()) {
		if ((*iterator)->eos()) {
			delete statementList_->at(i);
			statementList_->erase(iterator);
			size_--;
			//iterator has possibly become invalidated and must be moved to the current location.
			iterator = statementList_->begin();
			for (int j = 0; j < i; j++) {
				iterator++;
			}
		}
		else
			//If the current statement is not empty, we will continue to the next statement.
		{
			iterator++;
			i++;
		}
	};

	//check the last line of the program and delete if empty
	if (statementList_->at(size_ - 1)->eos()) {
		deleteAt(size_ - 1);
	}

	//we must look through the given psudo ops in order to accurately define memory locations
	iterator = statementList_->begin();
	
	while (iterator < statementList_->end()) {
		statement = *iterator;
		//if possible, set memory location.
			try {
			statement->setMemLocation(memStart);
		}
			//if the memory location is out of accepted bounds, report up.
		catch (StatementOOB e) {
			throw(ProgramException(e.what(), statement));
		}

		//test all words to see if a .STRINGZ or .BLKW psudo op has been found. If so, set memory accordingly
		for (int i = 0; i < statement->getWordCount(); i++) {
			if (statement->getWordTypeAt(i) == word::PSUDO) {
				try {
					if (statement->getWordAt(i) == ".STRINGZ" || statement->getWordAt(i) == ".STRINGz") {
						if (statement->getWordTypeAt(i + 1) != word::STRING)
							throw(ProgramException("STRING EXPECTED", statement));

						//add full size of the string, the incrementation of memStart will represent the null
						// termination of the given string
						memStart += ((statement->getWordAt(i + 1)).size());
					}
					else if (statement->getWordAt(i) == ".BLKW") {
						if (statement->getWordTypeAt(i + 1) == word::NUMBER)
							throw(ProgramException("NUMBER EXPECTED", statement));
						int jump = (stringOps::getNumberFromWord(statement->getWordAt(i)));
						//jump - 1 will be added to memory due to the later incrementation of memStart.
						memStart += (jump - 1);
					}
				}
				catch (StatementOOB e) {
					throw(ProgramException("WORD EXPECTED AFTER CURRENT WORD", statement, i));
				}
			}
		}
		memStart++;
		iterator++;
	};

	//The last line of this sterilized program should be the .END command, we will check for it and delete it.
	if (statementList_->at(size_ - 1)->getNextWord().getWord() == ".END")
		deleteAt(size_ - 1);
	else
		throw(ProgramException(".END COMMAND ILL-PLACED OR NOT INCLUDED"));


}


Program& Program::operator=(Program& right) {
	//perform a deep copy of the statement list by first deleting statementList_
	for (int i = 0; i < size_; i++) {
		if (statementList_->at(i) != nullptr) {
			delete statementList_->at(i);
			statementList_->at(i) = nullptr;
		}
	}

	delete statementList_;

	statementList_ = new vector<Statement*>();

	for (int i = 0; i < right.size_; i++) {
		statementList_->push_back(new Statement(*(right.statementList_->at(i))));
	}
	//set member variables
	size_ = right.size_;
	steril_ = right.steril_;
	labeled_ = right.labeled_;
	
	//return the assigned from operand.
	return right;

}
