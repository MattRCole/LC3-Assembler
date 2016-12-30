#pragma once
#include "Statement.h"
#include <fstream>
#include <vector>
#include <exception>
using namespace std;

class Program {
public:
	//will read the given contents of a file and initialize an 
	//unsterilized list of statements with appropriate words
	//Throws ProgramException if ifstream.open(); has not been called
	Program(fstream& fileInput);


	/*Copy constructor which will perform a deep copy of Program*/
	Program(Program&);

private:

	/*Private constructor which will construct a program from a vector of statement lists.
	  size_ is also passed as a parameter.*/
	Program(vector<Statement*>* statementList, int);
	
	

public:
	//default constructor
	Program();

	~Program();

	/*overloaded operator which returns a reference of the desired statement. performs
	  bounds checking and throws ProgramOOB exception*/
	Statement& operator[](int index);

	/*returns a reference of the desired statement. performs
	  bounds checking and throws ProgramOOB exception*/ 
	Statement& getStatementAt(int index);

	
	/*returns a steril program of this with accurately declared memory locations
	  Throws ProgramException meant for user debugging.*/
	Program* getSteril();

	/*Overloaded assignment operator performing a deep copy of a program*/
	Program& operator=(Program& right);
	

	int getSize() { return size_; }

	bool isSteril() { return steril_; }

	bool isLabled() { return labeled_; }

	void setLabeled() { labeled_ = true; }

	//will clear the current program and create a new program based
	//on given input
	void inputProgram(fstream& fileInput);

	void deleteAt(int index);

	void insertAt(int index, Statement* statement);

private:

	vector<Statement*>* statementList_;

	int size_;

	bool steril_;

	bool labeled_;

	void sterilize();


	


};

class ProgramException : public exception {
public:
	ProgramException(const char* error = nullptr, Statement* errorLocation = nullptr, int wordLocation = -1) :exception(error) {
		errorLocation_ = errorLocation;
		wordLocation_ = wordLocation;
	}

	Statement* getErrorLocation() { return errorLocation_; }
	
	
	~ProgramException() {
		if (errorLocation_ != nullptr)
			delete errorLocation_;
	}

private:
	Statement* errorLocation_;
	int wordLocation_;
};

class ProgramOOB : public exception {
public:
	ProgramOOB(const char* error = nullptr) : exception(error){ }
};