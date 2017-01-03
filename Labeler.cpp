#include "Labeler.h"
#include "StringFun.h"
using namespace std;


Labeler::Labeler() {
}




Program Labeler::getLabeled(Program program) {
	label(program);
	return program;
}

Program Labeler::getLabeled(Program* program) {
	if (program == nullptr)
		throw(LabelerException("NULL PROGRAM"));

	
	label(*program);

	//return labeled program.
	return *program;


}

void Labeler::label(Program& program) {
	Word word;
	vector<Label*>* masterLabels = new vector<Label*>;

	//define master labels and then delete master labels from program statements.
	for (int i = 0; i < program.getSize(); i++) {
		if (program[i][0].isLabel()) {
			masterLabels->push_back(new Label(program[i]));
			program.getStatementAt(i).deleteWordAt(0);
		}
	}

	/*Find and replace pointer labes with appropriate numbers which will point to the 
	  corrisponding master label memory location of the given pointer label */

	//iterate through entire program
	for (int i = 0; i < program.getSize(); i++) {

		//iterating through each statement
		for (int j = 0; j < program[i].getWordCount(); j++) {
			
			//if a label is found, we will determine the offset based on the opcode or psudo op given in the statement, perform bounds checking,
			//report up if a programmer error has been made, evaluate pointers and replace pointer labels with hex-based numbers.
			if (program[i][j].isLabel()) {

				//reset word get position to beginning of statement;
				program[i].gbeginning();
				
				//reset the tester word to a VOID empty word.
				word = Word();
				
				//search through statement for opcode or psudo-op
				while (!word.isPsudo() && !word.isOperator() && !program[i].eos())
				{
					word = program[i].getNextWord();
				}

				//if an operator or psudo op was not found, report user error and pertinant information
				if (!word.isOperator() && !word.isPsudo())
					throw(LabelerException("PSUDO OP OR OPCODE EXPECTED, LABEL CANNOT BE CORRECTLY EVALUATED", new Statement(program[i])));


				//for this part we will need static variables defined in the Statement.h file
				using namespace opcode;
				
				//define variables used to compute pointers to labels
				int pointer = 0;
				int location = 0;
				offset::Type offsetType = offset::ERROR;
				
				
				//if an opecode (or operator) is found, the offset will be determined by the opcode in the statement.
				// word.getDec() is used to get the decode index which corrisponds to a user friendly enum definition
				// of currently accepted op-codes.
				if (word.isOperator()) {
					switch (word.getDec()) {
					case(BR) :
						offsetType = offset::NINE;
						break;
					case(ST) :
						offsetType = offset::SIX;
						break;
					case(JSR) :
						offsetType = offset::ELEVEN;
						break;
					case(LDR) :
						offsetType = offset::SIX;
						break;
					case(STR) :
						offsetType = offset::SIX;
						break;
					case(LDI) :
						offsetType = offset::NINE;
						break;
					case(STI) :
						offsetType = offset::NINE;
						break;
					case(LEA) :
						offsetType = offset::NINE;
						break;
					default:
						offsetType = offset::ERROR;
						break;
					}

					//the statement did contain an op-code, but the operator does not support labels, report necesary information up.
					if (offsetType == offset::ERROR)
						throw(LabelerException("OPCODE CANNOT SUPPORT LABEL POINTER", new Statement(program[i]), program.getStatementAt(i).getNextWordPosition() - 1));


					//search for corrisponding masterlabel and, if found, compute pointer to label based off of the memory
					// location of the current statement + 1 (the plus one representing the incremented PC)
					for (int k = 0; k < masterLabels->size(); k++) {
						if (program[i][j] == *(masterLabels->at(k))) {
							location = masterLabels->at(k)->getMem();
							//pointer is equal to the location of the master label minus the incremented PC counter of the current statement;
							pointer = location - (program[i].getMemLocation() + 1);
							break;
						}
					}

					//if the location has not been changed, we have a label pointing to an undefined master label.
					// Report necesary information up.
					if (location == 0)
						throw(LabelerException("UNDEFINED LABEL", new Statement(program[i]), j));

					//Perform bounds checking based on the offset of the given operator. If an error is detected,
					// report up with necesary information
					if (!(pointer >= offset::NEG_BOUDNS[offsetType] && pointer <= offset::POS_BOUNDS[offsetType])) {
						switch (offsetType) {
						case(offset::SIX) :
							throw(LabelerException("LABEL POINTER IS OUT OF OFFSET6 BOUNDS"), new Statement(program[i]), j);
							break;
						case(offset::NINE) :
							throw(LabelerException("LABEL POINTER IS OUT OF OFFSET9 BOUNDS"), new Statement(program[i]), j);
							break;
						case(offset::ELEVEN) :
							throw(LabelerException("LABEL POINTER IS OUT OF OFFSET11 BOUNDS"), new Statement(program[i]), j);
							break;
						}
					}

					//replace the pointer label with an appropriate hexadecimal number bearing the hex prefix 'x'.
					program.getStatementAt(i).replaceWordAt(j, 
						new Word(
							"x" + stringOps::getStringFromNum(pointer, 16), 
							word::NUMBER, program[i][j].getPosition(), 
							static_cast<int>(number::HEX)
							)
						);
				}
				//We have a psudo op. If the psudo op is not the .EXTERNAL op, 
				// We will assume that any bounds are acceptible and simply place the un-incremented memory location of the
				// specified master label in the place of the pointer label.
				// If the .EXTERNAL op is found, the label will be left and evaluated at the linking phase.
				else {
					if (word != string(".EXTERNAL")) {
						bool found = false;
						for (int k = 0; k < masterLabels->size() && !found; k++) {
							if (program[i][j] == *(masterLabels->at(k))) {
								//master label found, word may be replaced.
								program.getStatementAt(i).replaceWordAt(j,
									new Word(
										"x" + stringOps::getStringFromNum(masterLabels->at(k)->getMem(), 16),
										word::NUMBER,
										program[i][j].getPosition(),
										static_cast<int>(number::HEX)
										)
									);
								found = true;
							}
						}

						//finally, if the code is not found, we will throw a labeler exception with necesary information to debug.
						if (!found)
							throw(LabelerException("UNDEFINED LABEL", new Statement(program[i]), j));
					}
				}
			}
		}
	}

	//if we have made it to this point, the program has been labeled and should be marked as such.
	program.setLabeled();
}