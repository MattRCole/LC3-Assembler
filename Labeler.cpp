#include "Labeler.h"
#include "StringFun.h"
using namespace std;


Labeler::Labeler() {
	program_ = nullptr;
	masterLabels_ = nullptr;
}

Labeler::Labeler(Program* program) {
	program_ = program;
	masterLabels_ = nullptr;
	label();
}

Labeler::Labeler(Program program) {
	program_ = new Program(program);
	masterLabels_ = nullptr;
	label();
}

Program Labeler::getLabeled() {
	if (program_ == nullptr)
		throw(LabelerException("NULL PROGRAM"));

	return *program_;
}

Program Labeler::getLabeled(Program* program) {
	if (program == nullptr)
		throw(LabelerException("NULL PROGRAM"));

	//clear out previous program_ and maserLabels_ objects if necesary.
	if (program_ != nullptr)
		delete program_;

	program_ = new Program();

	if (masterLabels_ != nullptr) {
		for (int i = 0; i < masterLabels_->size(); i++) {
			if (masterLabels_->at(i) != nullptr)
				delete masterLabels_->at(i);
		}
		delete masterLabels_;
		masterLabels_ = nullptr;
	}
	//copy given program
	*program_ = *program;

	//label given program
	label();

	//return labeled program.
	return *program_;


}

void Labeler::label() {
	Word word;
	if (program_ == nullptr)
		throw(LabelerException("NULL PROGRAM"));
	
	masterLabels_ = new vector<Label*>;

	//define master labels and then delete master labels from program statements.
	for (int i = 0; i < program_->getSize(); i++) {
		if ((*program_)[i][0].isLabel()) {
			masterLabels_->push_back(new Label((*program_)[i]));
			program_->getStatementAt(i).deleteWordAt(0);
		}
	}

	/*Find and replace pointer labes with appropriate numbers which will point to the 
	  corrisponding master label memory location of the given pointer label */

	//iterate through entire program
	for (int i = 0; i < program_->getSize(); i++) {

		//iterating through each statement
		for (int j = 0; j < (*program_)[i].getWordCount(); j++) {
			
			//if a label is found, we will determine the offset based on the opcode or psudo op given in the statement, perform bounds checking,
			//report up if a programmer error has been made, evaluate pointers and replace pointer labels with hex-based numbers.
			if ((*program_)[i][j].isLabel()) {

				//reset word get position to beginning of statement;
				(*program_)[i].gbeginning();
				
				//reset the tester word to a VOID empty word.
				word = Word();
				
				//search through statement for opcode or psudo-op
				while (!word.isPsudo() && !word.isOperator() && !(*program_)[i].eos())
				{
					word = (*program_)[i].getNextWord();
				}

				//if an operator or psudo op was not found, report user error and pertinant information
				if (!word.isOperator() && !word.isPsudo())
					throw(LabelerException("PSUDO OP OR OPCODE EXPECTED, LABEL CANNOT BE CORRECTLY EVALUATED", &program_->getStatementAt(i)));


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
						throw(LabelerException("OPCODE CANNOT SUPPORT LABEL POINTER", &program_->getStatementAt(i), program_->getStatementAt(i).getNextWordPosition() - 1));


					//search for corrisponding masterlabel and, if found, compute pointer to label based off of the memory
					// location of the current statement + 1 (the plus one representing the incremented PC)
					for (int k = 0; k < masterLabels_->size(); k++) {
						if (program_[0][i][j] == *(masterLabels_->at(k))) {
							location = masterLabels_->at(k)->getMem();
							//pointer is equal to the location of the master label minus the incremented PC counter of the current statement;
							pointer = location - (program_[0][i].getMemLocation() + 1);
							break;
						}
					}

					//if the location has not been changed, we have a label pointing to an undefined master label.
					// Report necesary information up.
					if (location == 0)
						throw(LabelerException("UNDEFINED LABEL", &program_->getStatementAt(i), j));

					//Perform bounds checking based on the offset of the given operator. If an error is detected,
					// report up with necesary information
					switch (offsetType) {
					case(offset::SIX) :
						if (!(pointer >= -32 && pointer <= 31))
							throw(LabelerException("LABEL POINTER IS OUT OF OFFSET6 BOUNDS"), &program_[0][i], j);
						break;
					case(offset::NINE):
						if (!(pointer >= -256 && pointer <= 255))
							throw(LabelerException("LABEL POINTER IS OUT OF OFFSET9 BOUNDS"), &program_[0][i], j);
						break;
					case(offset::ELEVEN):
						if (!(pointer >= -1024 && pointer <= 1023))
							throw(LabelerException("LABEL POINTER IS OUT OF OFFSET11 BOUNDS"), &program_[0][i], j);
						break;
					}

					//replace the pointer label with an appropriate hexadecimal number bearing the hex prefix 'x'.
					program_->getStatementAt(i).replaceWordAt(j, 
						new Word(
							"x" + stringOps::getStringFromNum(pointer, 16), 
							word::NUMBER, program_[0][i][j].getPosition(), 
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
						for (int k = 0; k < masterLabels_->size() && !found; k++) {
							if (program_[0][i][j] == *(masterLabels_->at(k))) {
								//master label found, word may be replaced.
								program_->getStatementAt(i).replaceWordAt(j,
									new Word(
										"x" + stringOps::getStringFromNum(masterLabels_->at(k)->getMem(), 16),
										word::NUMBER,
										program_[0][i][j].getPosition(),
										static_cast<int>(number::HEX)
										)
									);
								found = true;
							}
						}

						//finally, if the code is not found, we will throw a labeler exception with necesary information to debug.
						if (!found)
							throw(LabelerException("UNDEFINED LABEL", &program_->getStatementAt(i), j));
					}
				}
			}
		}
	}

	//if we have made it to this point, the program has been labeled and should be marked as such.
	program_->setLabeled();
}