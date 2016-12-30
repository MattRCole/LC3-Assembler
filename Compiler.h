#pragma once
#include <exception>
#include "GlobalVars.h"
#include "StringFun.h"
#include "Program.h"

using namespace std;

/*Will take a labeled program and convert it to a hex program. All words in all statement excluding any 
  .EXTERNAL psudo-ops and labels will be changed to an appropriate number or string*/
static class Compiler {
public:

	

	/*Will convert the reference program. Throws CompilerException if reference program is not labeled,
	  or is null. All other exceptions will be passed up. Calls private function convert()*/
	void convert(Program*, compiler::CONVERSION = compiler::HEX);

	/*Will convert the program, passes program by value, not by reference. Throws CompilerException
	  if program is not labeled.*/
	Program convert(Program, compiler::CONVERSION = compiler::HEX);

	

private:

	/*Will actually carry out the hex conversion of the program.
	  Throws many forms of CompilerException, all ment for user
	  benifit.*/
	void convertP(Program&, compiler::CONVERSION);


	bool inBounds(int number, offset::Type offsetType);


	/*Computes hexInt for ADD or AND opcodes*/
	int addOrAnd(Statement&);

	/*Computes hexInt for LD, LDI, LEA, ST, and STI opcodes*/
	int memoryNineOffset(Statement&);

	/*Computes hexInt for LDR and STR opcodes*/
	int memorySixOffset(Statement&);


	/*Computes hexInt for JMP and JSRR opcodes*/
	int jmpOrJsrr(Statement&);

	/*Will enter one word into the statement based on the conversion type specified*/
	void replaceAllWords(int, Statement&, compiler::CONVERSION);
};

class CompilerException : public exception {
public:
	CompilerException(const char* error = nullptr, Statement* statement = nullptr, int wher = -1) : exception(error) {
		statement_ = statement;
		where_ = wher;
	}

	Statement* getStatement() { return statement_; }

	int getWhere() { return where_; }

private:
	Statement* statement_;
	int where_;
};