#pragma once
#include <exception>
#include "Program.h"
#include <vector>
#include "Statement.h"
#include "Label.h"


using namespace std;



/*The labeler will define neumeric pointers to given "Master labels" based on the incremented PC.
  The labeler requires a sanatized program in order to label correctly.*/
class Labeler {
private:
	Program* program_;
	vector<Label*>*  masterLabels_;


	/*will perform the actual labeling of the member variable program_,
	  Precondition: program_ must point to a valid, steril Program object in memory, masterLabels_ must point to nullptr.
	  Postcondition: program_ will point to a valid, labeled Program object in memory, masterLabels_ will point to a valid array of Labels. 
	  Throws LabelerException with various definitions, the vast majority of which are formatted for user benifit.*/
	void label();

	
public:
	/*Standard default labeler which will not attempt to label a program until the function getLabeled(Program*) is called.
	  Note: An error will be thrown if a labeled program is called for from the labeler without first providing a sterilized
	  program. */
	Labeler();

	/*Constructor which, upon recieving a valid pointer to a program will proceed to label said program. If the program is not steril,
	  an exception will be thrown. The given program will be directly manipulated, and no copy of the given program will be made.*/
	Labeler(Program*);

	/*Constructor which, upon recieving a valid steril program, will copy by value the program and immediately attempt to produce a 
	  labeled program. If successful, the labeled program can be gotten through calling getLabeled() without parameters. The constructor
	  will pass-up any exceptions thrown by called member functions, and will throw a labeler exception if the program is not sterilized.*/
	Labeler(Program);


	/*will return a properly labeled program, will throw labeler exception if member variable program is null*/
	Program getLabeled();

	/*Will attempt to produce a labeled program from the given program*/
	Program getLabeled(Program*);



	
};


/*Meant to possibly be reported to the user, showing a line and cursor position corrisponding to the user error if
  applicable.*/
class LabelerException : public exception {
public:
	LabelerException(const char* error = nullptr, Statement* statement = nullptr, int wordLocation = -1) : exception(error) {
		statement_ = statement;
		where_ = wordLocation;
	}

	Statement* getStatement() { return statement_; };

	int getWhere() { return where_; }

private:
	Statement* statement_;
	int where_;
};