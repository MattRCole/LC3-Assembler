#include "Compiler.h"
#include "StringFun.h"

using namespace std;

void Compiler::convert(Program* program, compiler::CONVERSION convType) {
	if (program == nullptr)
		throw(CompilerException("NULL PROGRAM"));

	if (!program->isLabled())
		throw(CompilerException("PROGRAM NOT LABELED"));
	
	switch (convType) {
	case(compiler::HEX) :
		convertHex(*program);
		break;
	case(compiler::OBJ) :
		convertObj(*program);
		break;
	default:
		break;
	};
}

Program Compiler::convert(Program program, compiler::CONVERSION convType) {
	if (!program.isLabled())
		throw(CompilerException("PROGRAM NOT LABELED"));
	
	switch (convType)
	{
	case(compiler::HEX) :
		convertHex(program);
		break;
	case(compiler::OBJ) :
		convertObj(program);
		break;
	default:
		break;
	};

	return program;

}



void Compiler::convertHex(Program& program) {
	//Iterate through program replacing all words in statements
	// with a single word per statement which is a NUMBER string in base hex
	for (int i = 0; i < program.getSize(); i++) {
		
		//first, we will make sure that the first word is either a psudo op, or an operator (opcode).
		if (program[i][0].getWordType() != word::OPERATOR && program[i][0].getWordType() != word::PSUDO)
			throw(CompilerException("OPCODE OR PSUDO OP EXPECTED"), new Statement(program[i]), 0);

		//if the word is an operator (opcode) we will treat it according to its decode index
		if (program[i][0].getWordType() == word::OPERATOR) {
			Word word = program[i][0];
			Word hexWord = Word("", word::NUMBER, 0, number::HEX);
			int hexInt = 0;
			int pcOffset;

			switch (word.getDec())
			{
				/*******************************************BR OPCODE (0)**********************************************/
			case(opcode::BR) :
				//test to see what type of BR we have (IE: BRn, or BRzp) simple BR will be assumed as BRnzp

				//first, we will see if the string is of two characters, if so, we are dealing with an assumed
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
				if (program[i].getWordCount() > 1 || !program[i][2].isNumber())
					throw(CompilerException("NUMBER OR LABEL EXPECTED"), &program[i], 2);

				//If a number is present, check its bounds
				if (program[i].getWordCount() > 1) {
					pcOffset = stringOps::getNumberFromWord(program[i][2]);
					if (!inBounds(pcOffset, offset::NINE))
						throw(CompilerException("NUMBER OUT OF PCOFFSET9 BOUNDS", new Statement(program[i]), 1));
					
					hexInt += pcOffset;

					//if more than two words are in the statement, we will report up.
					if (program[i].getWordCount() > 2)
						throw(CompilerException("UNEXPECTED WORD(S)", new Statement(program[i]), 2));

				}

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

				//check ammount of words and valitity of words
				if (program[i].getWordCount() > 2)
					throw(CompilerException("UNEXPECTED WORD(S), OPCODE ONLY SUPPORTS 1 OPERAND", new Statement(program[i]), 2));
				if (program[i].getWordCount() < 2)
					throw(CompilerException("LABEL OR NUMBER EXPECTED AND NOT FOUND", new Statement(program[i]), 0));


				if (!program[i][1].isNumber())
					throw(CompilerException("UNEXPECTED WORD, LABEL OR NUMBER EXPECED", new Statement(program[i]), 1));

				if (!inBounds(stringOps::getNumberFromWord(program[i][1]), offset::ELEVEN))
					throw(CompilerException("NUMBER OUTSIDE OF OFFSET11 BOUNDS", new Statement(program[i]), 1));

				//if all qualifications are met, add offset11 to hexInt
				hexInt += stringOps::getNumberFromWord(program[i][1]);
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
				hexInt = opcode::HEX[word.getDec()];
				break;
			}
			//Clear all words from statement and add hexnumber with no preceding character to statement.
			program[i].clearWords();

			program[i].addWord(new Word(stringOps::getStringFromNum(hexInt, 16), word::NUMBER, 0, number::HEX));
		}


	}
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

		//add the imm5 select code
		hexInt += operand::IMM5_SELECT;

		hexInt += stringOps::getNumberFromWord(imm5);
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
	hexInt += stringOps::getNumberFromWord(statement[2]);

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

		hexInt += stringOps::getNumberFromWord(statement[3]);
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
