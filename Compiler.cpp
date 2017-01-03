#include "Compiler.h"
#include "StringFun.h"

using namespace std;

void Compiler::convert(Program* program, compiler::CONVERSION convType) {
	if (program == nullptr)
		throw(CompilerException("NULL PROGRAM"));

	if (!program->isLabled())
		throw(CompilerException("PROGRAM NOT LABELED"));
	
	
	convert(program, convType);
}

Program Compiler::convert(Program program, compiler::CONVERSION convType) {
	if (!program.isLabled())
		throw(CompilerException("PROGRAM NOT LABELED"));
	
	convertP(program, convType);

	return program;

}



void Compiler::convertP(Program& program, compiler::CONVERSION convType) {
	//Iterate through program replacing all words in statements
	// with a single word per statement which is a NUMBER string in base hex
	for (int i = 0; i < program.getSize(); i++) {
		int hexInt = 0;
		
		//first, we will make sure that the first word is either a psudo op, or an operator (opcode).
		if (program[i][0].getWordType() != word::OPERATOR && program[i][0].getWordType() != word::PSUDO)
			throw(CompilerException("OPCODE OR PSUDO OP EXPECTED"), new Statement(program[i]), 0);

		//if the word is an operator (opcode) we will treat it according to its decode index
		if (program[i][0].getWordType() == word::OPERATOR) {
			Word word = program[i][0];
			Word hexWord = Word("", word::NUMBER, 0, number::HEX);
			int pcOffset;

			switch (word.getDec())
			{
				/*******************************************BR OPCODE (0)**********************************************/
			case(opcode::BR) :
				//test to see what type of BR we have (IE: BRn, or BRzp) simple BR will be assumed as BRnzp


				//validate the ammount of words in the statement.
				if (program[i].getWordCount() > 2)
					throw(CompilerException("UNEXPECTED WORD. OPCODE ONLY SUPPORTS TWO OPERANDS", new Statement(program[i]), 2));
				if (program[i].getWordCount() < 2)
					throw(CompilerException("NUMBER OR LABEL EXPECTED", new Statement(program[i]), 1));


				//we will see if the string is of two characters, if so, we are dealing with an assumed
				// BRnzp and will add to the hexInt appropriately
				if (word.getWord().size() == 2)
					hexInt = opcode::BR_COND[opcode::NZP];

				//if we are not dealing with an assumed BRnzp, we will evaluate the conditions manually.
				else {
					if (word.getWord().find('n') != string::npos) 
						//add the appropriate code for n
						hexInt += opcode::BR_COND[opcode::N];
					
					if (word.getWord().find('z') != string::npos)
						//add code for z
						hexInt += opcode::BR_COND[opcode::Z];

					if (word.getWord().find('p') != string::npos)
						//add code for p
						hexInt += opcode::BR_COND[opcode::P];
				}

				//NZP conditions added, we will now add the PCoffset9 after checking for bounds.

				//check to make sure that the second word in current statement is a number, and add it to the hexInt
				if (!program[i][1].isNumber())
					throw(CompilerException("UNEXPECTED WORD, NUMBER OR LABEL EXPECTED"), &program[i], 1);

				pcOffset = stringOps::getNumberFromWord(program[i][1]);
				
				if (!inBounds(pcOffset, offset::NINE))
					throw(CompilerException("NUMBER OUT OF PCOFFSET9 BOUNDS", new Statement(program[i]), 1));
				
				//if pcOffset is negative, we must concatinate the two's compliment down to nine bits.
				if (pcOffset < 0)
					pcOffset = pcOffset & offset::N_ISOLATION[offset::NINE];

				hexInt += pcOffset;

				

				//if no following words are found, we will add zero to the hexInt
				break;


				/*******************************************ADD OPCODE (1)*********************************************/
			case(opcode::ADD) :
				//add the appropriate number for the add function
				hexInt = opcode::HEX[opcode::ADD];

				//call add or and function, adding the code determined from there to the hexInt
				hexInt += addOrAnd(program[i]);
				break;

				/*******************************************LD  OPCODE(2)**********************************************/
			case(opcode::LD) :
				//set the hexInt to the opcode number
				hexInt = opcode::HEX[opcode::LD];

				//call the memoryOffset() function and add code to the hexInt
				hexInt += memoryNineOffset(program[i]);
				break;
				/******************************************ST OPCODE(3)************************************************/
			case(opcode::ST) :
				hexInt = opcode::HEX[opcode::ST];

				//call memoryNineOffset() and add to hexInt;
				hexInt += memoryNineOffset(program[i]);

				break;
				/******************************************JSR OPCODE(4)***********************************************/
			case(opcode::JSR) :
				hexInt = opcode::HEX[opcode::ST];
				int pcOffset;
				//check ammount of words and valitity of words
				if (program[i].getWordCount() > 2)
					throw(CompilerException("UNEXPECTED WORD(S), OPCODE ONLY SUPPORTS 1 OPERAND", new Statement(program[i]), 2));
				if (program[i].getWordCount() < 2)
					throw(CompilerException("LABEL OR NUMBER EXPECTED AND NOT FOUND", new Statement(program[i]), 0));


				if (!program[i][1].isNumber())
					throw(CompilerException("UNEXPECTED WORD, LABEL OR NUMBER EXPECED", new Statement(program[i]), 1));

				//get pcoffset and check bounds

				pcOffset = stringOps::getNumberFromWord(program[i][1]);

				if (!inBounds((pcOffset), offset::ELEVEN))
					throw(CompilerException("NUMBER OUTSIDE OF OFFSET11 BOUNDS", new Statement(program[i]), 1));


				//concatinate offset11 if necesary
				if (pcOffset < 0)
					pcOffset = pcOffset & offset::N_ISOLATION[offset::ELEVEN];
				//if all qualifications are met, add offset11 to hexInt
				hexInt += pcOffset;
				break;

				/****************************************AND OPCODE(5)*************************************************/
			case(opcode::AND) :
				//see add... you'll get what I'm doing.
				hexInt = opcode::HEX[opcode::AND];

				hexInt += addOrAnd(program[i]);
				break;


				/*****************************************LDR OPCODE(6)************************************************/
			case(opcode::LDR) :
				hexInt = opcode::HEX[opcode::LDR];

				hexInt += memorySixOffset(program[i]);
				break;

				/****************************************STR OPCODE(7)*************************************************/
			case(opcode::STR) :
				hexInt = opcode::HEX[opcode::STR];

				hexInt += memorySixOffset(program[i]);
				break;

				/****************************************RTI OPCODE(8)*************************************************/
			case(opcode::RTI) :
				throw(CompilerException("RTI OPCODE IS UNSUPPORTED AT THE PRESENT TIME", new Statement(program[i]),0));

				break;

				/****************************************NOT OPCODE(9)*************************************************/
			case(opcode::NOT) :
				hexInt = opcode::HEX[opcode::NOT];

				//NOT opcode is suprisingly similar to memorySixOffset opcode set. We will make sure there are no more
				// than three words in the statement and then call memorySixOffset()

				if (program[i].getWordCount() > 3)
					throw(CompilerException("UNEXPECTED WORD, OPCODE ONLY SUPPORTS 2 OPERANDS"), new Statement(program[i]), 3);

				hexInt += memorySixOffset(program[i]);

				break;

				/****************************************LDI OPCODE(A)*************************************************/
			case(opcode::LDI) :
				hexInt = opcode::HEX[opcode::LDI];

				hexInt += memoryNineOffset(program[i]);

				break;

				/*****************************************STI OPCODE(B)************************************************/
			case(opcode::STI) :
				hexInt = opcode::HEX[opcode::STI];

				hexInt += memoryNineOffset(program[i]);

				break;

				/****************************************JMP OPCODE(C)*************************************************/
			case(opcode::JMP) :
				hexInt = opcode::HEX[opcode::JMP];

				hexInt += jmpOrJsrr(program[i]);
				break;

				/**************************************ILLI OPCODE(D)**************************************************/
			case(opcode::ILLI) :
				throw(CompilerException("ILLI OPCODE NOT SUPPORTED AT THIS TIME", new Statement(program[i]), 0));
				
				break;

				/**************************************LEA OPCODE(E)***************************************************/
			case(opcode::LEA) :
				hexInt = opcode::HEX[opcode::LEA];

				hexInt += memoryNineOffset(program[i]);
				break;
				/***************************************JSRR OPCODE****************************************************/
			case(opcode::JSRR) :
				hexInt = opcode::HEX[opcode::JSRR];

				hexInt += jmpOrJsrr(program[i]);
				break;
		/*************************************************************************************************************************/
			default:

				//the other cases are completely defined by ther HEX decode index under the opcode namespace
				hexInt = opcode::HEX[word.getDec()];
				break;
			}

			replaceAllWords(hexInt, program[i], convType);

		}

/*****************************PSUDO OP EVALUATION***************************************************************************************/
		else {
			//determine psudo op type, do nothing for .EXTERNAL, but throw errors for .ORIG and .END

			/**********************************BLKW************************************************************************************/
			if (program[i][0] == ".BLKW") {
				//there can be 1-3 words in this statement. report errors if necesary
				if (program[i].getWordCount() > 3)
					throw(CompilerException("UNEXPECTED WORD, PSUDO OP ONLY SUPPORTS 2 OPERANDS", new Statement(program[i]), 3));
				if (program[i].getWordCount() == 1) {
					//effectivly do nothing but add a blank space to the program.
					replaceAllWords(hexInt, program[i], convType);
				}
				else {
					//if we have three words in the statement, the third will be a number which will be used to fill
					//a given number of statements preceeding this statement determined by the number contained in the second word of the
					//statement
					if (program[i].getWordCount() == 3)
					{
						//check third word to make sure it's a number
						if (!program[i][2].isNumber())
							throw(CompilerException("NUMBER OR LABEL EXPECTED", new Statement(program[i]), 2));

						//set fill number
						hexInt = stringOps::getNumberFromWord(program[i][2]);
					}

					if (!program[i][1].isNumber())
						throw(CompilerException("NUMBER EXPECTED", new Statement(program[i]), 1));

					int iterCnt = stringOps::getNumberFromWord(program[i][1]);

					//insert new statement(s) containing desired hexInt into the program while maintaining
					// the current statement position in the program.
					Statement* blockStatement = nullptr;
					Word* blockWord = nullptr;
					for (int j = 0; j < (iterCnt - 1); j++) {
						switch (convType) {
						case(compiler::HEX):
							blockWord = new Word(stringOps::getStringFromNum(hexInt, 16), word::NUMBER, 0, number::HEX);
							break;
						case(compiler::OBJ) :
							blockWord = new Word("", word::INTEGER, 0, -1, hexInt);
							break;
						}
						blockStatement = new Statement();
						blockStatement->addWord(blockWord);

						//accurately set memory location
						blockStatement->setMemLocation(i);

						program.insertAt(i, blockStatement);
						//we still want to point at the statement that is currently being evaluated.
						i++;
					}

					//For the last (or frindge case first) insertion, we will clear the currently evaluated
					// Statement and add the desired conversion type word
					replaceAllWords(hexInt, program[i], convType);

					//set accurate mem location for final statement
					program[i].setMemLocation(i);
				}
			}

			/***********************************FILL*******************************************************************/
			else if (program[i][0] == ".FILL") {
				//check to make sure there are only two words in the statement and report as necesary
				if (program[i].getWordCount() > 2)
					throw(CompilerException("UNEXPECTED WORD, PSUDO OP ONLY SUPPORTS ONE OPERAND", new Statement(program[i]), 2));
				if (program[i].getWordCount() < 2)
					throw(CompilerException("NUMBER OR LABEL EXPECTED", new Statement(program[i]), 0));

				//report if word is not number
				if (!program[i][1].isNumber())
					throw(CompilerException("UNEXPECTED WORD, LABEL OR NUMBER EXPECTED", new Statement(program[i]), 1));
				replaceAllWords(stringOps::getNumberFromWord(program[i][1]), program[i], convType);
			}

			/************************************STRINGZ**************************************************************/
			else if (program[i][0] == ".STRINGZ" || program[i][0] == ".STRINGz") {
				//check to make sure following word is of string type and exists. Report as necessary
				if (program[i].getWordCount() > 2)
					throw(CompilerException("UNEXPECTED WORD, PSUDO OP ONLY SUPPORTS 1 OPERAND", new Statement(program[i]), 2));
				if (program[i].getWordCount() < 2)
					throw(CompilerException("STRING EXPECTED", new Statement(program[i]), 0));
				if (!program[i][1].isString())
					throw(CompilerException("UNEXPECTED WORD, STRING EXPECTED", new Statement(program[i]), 1));


				//Add new statements into the program filled with characters from string. 

				Statement* stringStatement = nullptr;
				for (int j = 0; j < program[i][1].size(); j++) {
					stringStatement = new Statement();
					replaceAllWords(static_cast<int>(program[i][1].getWord().at(j)), *stringStatement, convType);
					
					//set memory location
					stringStatement->setMemLocation(i + program[0].getMemLocation());

					program.insertAt(i, stringStatement);
					i++;
				}

				//Terminate with null memory location
				replaceAllWords(0, program[i], convType);

				//set memory location
				program[i].setMemLocation(i);

			}
			else if (program[i][0] != ".EXTERNAL") 
				throw(CompilerException("UNEXPECTED PSUDO OP: .END or .ORIG, ONLY ONE CALL TO THESE PSUDO OPS CAN BE MADE", new Statement(program[i]), 0));
		}
	}


	//All done converting
	return;
}

bool Compiler::inBounds(int number, offset::Type offsetType) {
	using namespace offset;
	if (number <= POS_BOUNDS[offsetType] && number >= NEG_BOUDNS[offsetType])
		return true;
	else
		return false;
}

int Compiler::addOrAnd(Statement &statement) {
	int hexInt = 0;
	//first, make sure there are four words in the statement, no more, no less. Report if there are more words
	if (statement.getWordCount() > 4)
		throw(CompilerException("UNEXPECTED WORD, OPCODE ONLY SUPPORTS 3 OPERANDS", new Statement(statement), 4));
	if (statement.getWordCount() < 4)
		throw(CompilerException("WORD EXPECTED", new Statement(statement), statement.getWordCount() - 1));

	//now we will validate each operand
	if (!statement[1].isOperand())
		throw(CompilerException("REGISTER IDENTIFIER EXPECTED", new Statement(statement), 1));
	if (!statement[2].isOperand())
		throw(CompilerException("REGISTER IDENTIFIER EXPECTED", new Statement(statement), 2));
	if (!statement[3].isOperand() && !statement[3].isNumber())
		throw(CompilerException("REGISTER IDENTIFIER OR IMM5 EXPECTED", new Statement(statement), 3));

	//add appropriate register codes to hexInt
	hexInt += operand::DR_HEX[statement[1].getDec()];
	hexInt += operand::SR1_HEX[statement[2].getDec()];

	//if the third operand is a number, check its imm5 bounds, report if necesary and add to hexInt
	if (statement[3].isNumber()) {
		Word imm5 = statement[3];
		//check bounds
		if (!inBounds(stringOps::getNumberFromWord(imm5), offset::FIVE))
			throw(CompilerException("NUMBER IS NOT WITHIN IMM5 BOUNDS"), new Statement(statement), 3);

		int imm5off = stringOps::getNumberFromWord(imm5);

		if (imm5off < 0)
			imm5off = imm5off & offset::N_ISOLATION[offset::FIVE];
		//add the imm5 select code
		hexInt += operand::IMM5_SELECT;

		hexInt += imm5off;
	}
	else
		hexInt += operand::SR2_HEX[statement[3].getDec()];

	return hexInt;
}


int Compiler::memoryNineOffset(Statement &statement) {
	int hexInt = 0;

	//first, make sure there are only 3 words in the statement, and report as necesary
	if (statement.getWordCount() > 3)
		throw(CompilerException("UNEXPECTED WORD, OPCODE ONLY SUPPORTS 2 OPERANDS", new Statement(statement), 3));
	if (statement.getWordCount() < 3)
		throw(CompilerException("WORD EXPECTED", new Statement(statement), statement.getWordCount() - 1));

	//validate remaining words to assure that they are operands or numbers 
	if (!statement[1].isOperand())
		throw(CompilerException("REGISTER IDENTIFIER EXPECTED", new Statement(statement), 1));
	if (!statement[2].isNumber())
		throw(CompilerException("NUMBER EXPECTED", new Statement(statement), 2));

	//check the bounds of the number and report as necesary
	if (!inBounds(stringOps::getNumberFromWord(statement[2]), offset::NINE))
		throw(CompilerException("NUMBER IS OUTSIDE OF OFFSET9 BOUNDS"), new Statement(statement), 2);

	hexInt += operand::DR_HEX[statement[1].getDec()];
	hexInt += (offset::N_ISOLATION[offset::NINE] & stringOps::getNumberFromWord(statement[2]));

	return hexInt;
}

int Compiler::memorySixOffset(Statement &statement)
{
	int hexInt = 0;

	//first, make sure there are only 4-3 words in the statement, and report as necesary
	if (statement.getWordCount() > 4)
		throw(CompilerException("UNEXPECTED WORD, OPCODE ONLY SUPPORTS 3 OPERANDS", new Statement(statement), 4));
	if (statement.getWordCount() < 3)
		throw(CompilerException("WORD EXPECTED", new Statement(statement), statement.getWordCount() - 1));

	//validate remaining words to assure that they are operands or numbers 
	if (!statement[1].isOperand())
		throw(CompilerException("UNEXPECTED WORD, REGISTER IDENTIFIER EXPECTED", new Statement(statement), 1));
	if (!statement[2].isOperand())
		throw(CompilerException("UNEXPECTED WORD, REGISTER IDENTIFIER EXPECTED", new Statement(statement), 2));


	//assume zero if there is not a fourth word.
	if (statement.getWordCount() == 4) {
		if (!statement[3].isNumber())
			throw(CompilerException("UNEXPECTED WORD, NUMBER EXPECTED", new Statement(statement), 3));

		//check the bounds of the number and report as necesary
		if (!inBounds(stringOps::getNumberFromWord(statement[3]), offset::SIX))
			throw(CompilerException("NUMBER IS OUTSIDE OF OFFSET6 BOUNDS"), new Statement(statement), 3);

		//isolate the appropriate bitsr
		hexInt += (offset::N_ISOLATION[offset::SIX] & stringOps::getNumberFromWord(statement[3]));
	}


	hexInt += operand::DR_HEX[statement[1].getDec()];
	hexInt += operand::BaseR_HEX[statement[2].getDec()];

	return hexInt;
}

int Compiler::jmpOrJsrr(Statement &statement)
{
	int hexInt = 0;
	//check for user errors
	if (statement.getWordCount() > 2)
		throw(CompilerException("UNEXPECTED WORD(S), OPCODE ONLY SUPPORTS 1 OPERAND", new Statement(statement), 2));
	if (statement.getWordCount() < 2)
		throw(CompilerException("REGISTER IDENTIFIER EXPECTED AND NOT FOUND", new Statement(statement), 0));

	Word reg = statement[1];

	if (!reg.isOperand())
		throw(CompilerException("UNEXPECTED WORD, REGISTER IDENTIFIER EXPECTED", new Statement(statement), 1));

	hexInt = operand::BaseR_HEX[reg.getDec()];

	return hexInt;
}


void Compiler::replaceAllWords(int hexInt, Statement& statement, compiler::CONVERSION convType) {
	switch (convType)
	{
	case(compiler::HEX) :
		//Clear all words from statement and add hexnumber with no preceding character to statement.
		statement.clearWords();

		statement.addWord(new Word(stringOps::getStringFromNum(hexInt, 16), word::NUMBER, 0, number::HEX));
		break;
	case(compiler::OBJ) :
		//Clear all words from statement and add integer word to statement.
		statement.clearWords();

		statement.addWord(new Word("", word::INTEGER, 0, -1, hexInt));
		break;
	}
}