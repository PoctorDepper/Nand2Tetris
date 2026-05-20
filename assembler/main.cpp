#include <bitset>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>

constexpr unsigned short JUMP_GREATER = 0b0000000000000001;
constexpr unsigned short JUMP_EQUAL = 0b0000000000000010;
constexpr unsigned short JUMP_LESS = 0b0000000000000100;
constexpr unsigned short STORE_MEMORY = 0b0000000000001000;
constexpr unsigned short STORE_DATA = 0b0000000000010000;
constexpr unsigned short STORE_ADDRESS = 0b0000000000100000;
constexpr unsigned short NEGATE_OUTPUT = 0b0000000001000000;
constexpr unsigned short FUNCTION = 0b0000000010000000;
constexpr unsigned short NEGATE_Y = 0b0000000100000000;
constexpr unsigned short ZERO_Y = 0b0000001000000000;
constexpr unsigned short NEGATE_X = 0b0000010000000000;
constexpr unsigned short ZERO_X = 0b0000100000000000;
constexpr unsigned short SWAP_Y = 0b0001000000000000;
constexpr unsigned short C_INSTRUCTION = 0b1110000000000000;    // We also use this as an error code
constexpr unsigned short A_INSTRUCTION_MASK = 0b0111111111111111;
constexpr std::string OPERATIONS = "-+!&|";
constexpr std::string OPERANDS = "01ADM";
constexpr std::string LOCATIONS = "ADM";


// A split C instruction for computation
struct Instruction
{
	std::string assignment;
	char operation = '\0';
	std::string operands;
	std::string jump;
};

// Start a symbol table with the predefined symbols of the Hack assembler language
std::unordered_map<std::string, unsigned short> get_symbol_table()
{
	std::unordered_map<std::string, unsigned short> table;

    for (int i = 0; i < 16; i++)
    {
        table["R" + std::to_string(i)] = i;
    }

    table["SCREEN"] = 16384;
    table["KBD"] = 24576;
    table["SP"] = 0;
    table["LCL"] = 1;
    table["ARG"] = 2;
    table["THIS"] = 3;
    table["THAT"] = 4;

	return table;
}

// Splits the line of assembler into an instruction
Instruction parse_instruction(std::string line)
{
    Instruction out;
    std::size_t assign = line.find('=');
    std::size_t jump = line.find(';');
    std::size_t operationStart = 0;
    std::size_t operationEnd = line.find("//");

    // If we found the assignment operator, set the assignment destination and move the operation start point
    if (assign != std::string::npos)
    { 
        out.assignment = line.substr(0, assign);
        operationStart = assign + 1;
    }

    // If we found a jump instruction, set the jump instruction and move the operation end point
    if (jump != std::string::npos)
    {
        out.jump = line.substr(jump + 1, operationEnd);
        operationEnd = jump;
    }

    // Grab the operation substring
    std::string operation = line.substr(operationStart, operationEnd);

    // Grab the operation (or nothing in the case of 1 and 0, this is by design) and remove it
    for (char c : OPERATIONS)
    {
        std::size_t op = operation.find(c);
        if (op == std::string::npos) continue;

        out.operation = operation[op];
        operation.erase(op, 1);

        // We break out of the for loop here, as we error for multiple operators later
        break;
    }

    // Then the leftover *should* be the operands (we handle this later)
    out.operands = operation;

    return out;
}

// Return the value needed for proper assignment or C_INSTRUCTION in error
unsigned short evaluate_assignment(Instruction instruction)
{
    unsigned short assignment;

    for (char c : instruction.assignment)
    {
        switch (c)
        {
        case 'A':
            assignment |= STORE_ADDRESS;
            break;
        case 'D':
            assignment |= STORE_DATA;
            break;
        case 'M':
            assignment |= STORE_MEMORY;
            break;
        default:
            return C_INSTRUCTION;
        }
    }    

    return assignment;
}

// Return the jump instruction or C_INSTRUCTION in error
unsigned short evaluate_jump(Instruction instruction)
{
    unsigned short jump;

    if (instruction.jump.empty()) return 0;
    if (instruction.jump[0] != 'J') return C_INSTRUCTION;
    
    if (instruction.jump == "JNE") jump |= JUMP_LESS | JUMP_GREATER;
    else if (instruction.jump == "JMP") jump |= JUMP_GREATER | JUMP_LESS | JUMP_EQUAL;
    else
    {
        if (instruction.jump.contains('G')) jump |= JUMP_GREATER;
        if (instruction.jump.contains('L')) jump |= JUMP_LESS;
        if (instruction.jump.contains('E')) jump |= JUMP_EQUAL;
    }

    return jump;
}

// Return the needed instructions given the operator and operand(s) or C_INSTRUCTION in error
unsigned short evaluate_computation(Instruction instruction)
{
    // Do we have too many operands or no operands?
    if (instruction.operands.length() > 2 || instruction.operands.length() < 1) return C_INSTRUCTION;

    unsigned short computation;

    // SWAP-Y check, also errors if A is present
    if (instruction.operands.contains('M'))
    {
        if (instruction.operands.contains('A')) return C_INSTRUCTION;
        computation |= SWAP_Y;
    }

    // Decide what we do base on the function operators (!,-,+,&,|)
    switch (instruction.operation)
    {
    // 4 total options, two of which are x+1
    case '+':
        if (instruction.operands.length() < 2) return C_INSTRUCTION;

        switch (instruction.operands[1])
        {
            case '1':
                switch (instruction.operands[0])
                {
                    // D+1
                    case 'D':
                        computation |= ZERO_Y;
                        break;
                    
                    // A/M+1
                    case 'A':
                    case 'M':
                        computation |= ZERO_X;
                        break;
                    
                    default:
                        return C_INSTRUCTION;
                }

                computation |= NEGATE_X | NEGATE_Y | NEGATE_OUTPUT;
                break;
                
            // A/M+D
            case 'D':
                if (instruction.operands[0] != 'A' && instruction.operands[0] != 'M') return C_INSTRUCTION;
                // Since we're just adding, we do nothing here
                break;

            // D+A/M
            case 'A':
            case 'M':
                if (instruction.operands[0] != 'D') return C_INSTRUCTION;
                // Since we're just adding, we do nothing here
                break;

            default:
                return C_INSTRUCTION;
        }

        computation |= FUNCTION;
        break;

    // Probably the most "complex" instruction, as there are a lot of different options
    case '-':
        // Extending single negative operations for ease of patterning
        if (instruction.operands.length() < 2) instruction.operands = '\0' + instruction.operands;

        // Checking the last operand here
        switch (instruction.operands[1])
        {
            // x-D
            case 'D':
                switch (instruction.operands[0])
                {
                    // -D
                    case '\0':
                        computation |= ZERO_Y;
                    // A/M-D
                    case 'A':
                    case 'M':
                        computation |= NEGATE_Y;
                        break;

                    default:
                        return C_INSTRUCTION;
                }

                computation |= FUNCTION | NEGATE_OUTPUT;
                break;
            
            // x-A/M
            case 'A':
            case 'M':
                switch (instruction.operands[0])
                {
                    // -A/M
                    case '\0':
                        computation |= ZERO_X;
                    // D-A/M
                    case 'D':
                        computation |= NEGATE_X;
                        break;

                    default:
                        return C_INSTRUCTION;
                }

                computation |= FUNCTION | NEGATE_OUTPUT;
                break;
            
            // x-1
            case '1':
                switch (instruction.operands[0])
                {
                    // -1
                    case '\0':
                        computation |= ZERO_X | ZERO_Y | NEGATE_X;
                        break;

                    // D-1
                    case 'D':
                        computation |= ZERO_Y | NEGATE_Y;
                        break;

                    // A/M-1
                    case 'A':
                    case 'M':
                        computation |= ZERO_X | NEGATE_X;
                        break;

                    default:
                        return C_INSTRUCTION;
                }
                break;

            default:
                return C_INSTRUCTION;
        }

        computation |= FUNCTION;
        break;

    // The NOT operation is AND -1 with negated output
    case '!':
        if (instruction.operands.length() > 1) return C_INSTRUCTION;
        switch (instruction.operands[0])
        {
            // Make the right operand -1
            case 'D':
                computation |= ZERO_Y | NEGATE_Y;
                break;
            
            // Make the left operand -1
            case 'A':
            case 'M':
                computation |= ZERO_X | NEGATE_X;
                break;
            
            default:
                return C_INSTRUCTION;
        }

        computation |= NEGATE_OUTPUT;
        break;
    
    // OR is just AND with negated everything (DeMorgan's)
    case '|':
        computation |= NEGATE_X | NEGATE_Y | NEGATE_OUTPUT;
    // The only AND operation requires two operands, and must start with D followed by an A or M
    case '&':
        if (instruction.operands.length() < 2 || instruction.operands[0] != 'D') return C_INSTRUCTION;
        if (instruction.operands[1] != 'A' && instruction.operands[1] != 'M') return C_INSTRUCTION;
        // An AND operations is all 0's, so do nothing
        break;

    // This case only happens with single operands, under any other case this is a failure
    case '\0':
        if (instruction.operands.length() > 1) return C_INSTRUCTION;
        switch(instruction.operands[0])
        {
            // D & 1
            case 'D':
                computation |= ZERO_Y | NEGATE_Y;
                break;
            
            // A and M are the same process
            // 1 & A/M
            case 'A':           
            case 'M':
                computation |= ZERO_X | NEGATE_X;
                break;
            
            // 1 and 0 declaration have similar outputs, so fallthrough
            // -(-1+-1) i.e. two's complement
            case '1':
                computation |= NEGATE_X | NEGATE_Y | NEGATE_OUTPUT;
            // 0 + 0   
            case '0':
                computation |= ZERO_X | ZERO_Y | FUNCTION;
                break;

            default:
                return C_INSTRUCTION;
        }
        break;

    // If the operand isn't any of the listed, that's a problem ( shouldn't be possible, but better to be safe)
    default:
        return C_INSTRUCTION;
    }

    return computation;
}

int main(const int argc, const char* argv[])
{
    // Check for file argument
    if (argc <= 1)
    {
        std::cerr << "You must pass a file as an argument." << std::endl;
        return EXIT_FAILURE;
    }

    // Check file ending (yes, I know this isn't the most secure, but oh well)
    std::string fileName = argv[1];
    if (fileName.substr(fileName.find_last_of('.')) != ".asm")
    {
        std::cerr << "\"" << fileName << "\" is not an assembly file." << std::endl;
        return EXIT_FAILURE;
    }

    // Read the file and store so the file isn't kept open during processing
    std::ifstream assemblyFile(fileName, std::ios::in);
    if (!assemblyFile.is_open())
    {
        std::cerr << "\"" << fileName << "\" could not be opened." << std::endl;
        return EXIT_FAILURE;
    }

    // Read all lines from the file, stripping spaces
    std::vector<std::string> assemblyLines;
    for (std::string line; std::getline(assemblyFile, line);)
    {
        std::erase_if(line, [](const unsigned char x) { return std::isspace(x); });
        assemblyLines.push_back(line);
    }
    assemblyFile.close();

    // Symbol pass, requires a dual pass I believe
    std::unordered_map<std::string, unsigned short> symbolTable = get_symbol_table();
    for (short i = 0, lineCount = assemblyLines.size(), assemblyCount = 0; i < lineCount; ++i)
    {
        std::string line = assemblyLines[i];

        // If the line is empty, skip
        if (line.empty() || line[0] == '/') continue;

        // If it's an A instruction or C instruction, skip
        if (line[0] == '@' || OPERANDS.contains(line[0]) || OPERATIONS.contains(line[0])) 
        {
            ++assemblyCount;
            continue;
        }

        // Remove the first and last element of the line "()"
        line = line.substr(1, line.length() - 2);

        // If the token is already in the table, throw an error
        if (symbolTable.contains(line))
        {
            std::cerr << "Token assigned more than once at line " << i + 1 << "." << std::endl;
            return EXIT_FAILURE;   
        }

        symbolTable[line] = assemblyCount;
    }

    // Computation pass
    std::vector<unsigned short> binaryLines;
    // Starts after the R registers
    short variableCount = 16;
    for (short i = 0, lineCount = assemblyLines.size(); i < lineCount; ++i)
    {
        const std::string& line = assemblyLines[i];
        if (line.empty()) continue;

        unsigned short outBinary;
        
        switch (line[0])
        {
            // Skip over tokens and comments
            case '(':
            case '/':
                continue;

            // A-Instruction block
            // This will use bitwise AND, as we are ensuring the most significant bit is 0 with the mask
            case '@':
                {
                std::string destination = line.substr(1);
                short value;
                outBinary = A_INSTRUCTION_MASK;

                if (symbolTable.contains(destination)) value = symbolTable[destination];
                
                else
                {
                    try
                    {
                        int eval = std::stoi(destination);

                        // If the integer parse is greater than 32K address or negative, that's a problem
                        if (eval < 0 || eval > 0x7FFF) 
                        {
                            std::cerr << "Invalid value at line " << i + 1 << "." << std::endl;
                            return EXIT_FAILURE;
                        }

                        value = eval;
                    }
                    // Will not be using that exception for anything
                    catch (std::exception&)
                    {
                        value = variableCount++;
                        symbolTable[destination] = value;
                    }
                }

                outBinary &= value;
                }
                break;

            // C-Instruction block
            // This will use bitwise OR because it's adding to the base C_INSTRUCTION
            case '0':
            case '1':
            case 'A':
            case 'D':
            case 'M':
            case '!':
            case '-':
                {
                outBinary = C_INSTRUCTION;
                // Fills an instruction struct with data from the line, then computes each part of the instruction
                Instruction compInstruction = parse_instruction(line);

                // Assignment
                unsigned short evaluation = evaluate_assignment(compInstruction);
                if (evaluation == C_INSTRUCTION) 
                {
                    std::cerr << "Invalid assignment at line " << i + 1 << "." << std::endl;
                    return EXIT_FAILURE;
                }
                outBinary |= evaluation;

                // Jump
                evaluation = evaluate_jump(compInstruction);
                if (evaluation == C_INSTRUCTION) 
                {
                    std::cerr << "Invalid jump at line " << i + 1 << "." << std::endl;
                    return EXIT_FAILURE;
                }
                outBinary |= evaluation;

                // Computation
                evaluation = evaluate_computation(compInstruction);
                if (evaluation == C_INSTRUCTION) 
                {
                    std::cerr << "Invalid computation at line " << i + 1 << "." << std::endl;
                    return EXIT_FAILURE;
                }
                outBinary |= evaluation;

                }
                break;

            default:
                std::cerr << "Invalid instruction at line " << i + 1 << "." << std::endl;
                return EXIT_FAILURE;
            break;

        }

        // Add the instruction to the binary lines
        binaryLines.push_back(outBinary);
    }


    // Write to .hack file
    fileName = fileName.substr(0, fileName.find_last_of('.')) + ".hack";
    std::ofstream binaryFile(fileName, std::ios::out);
    if (!binaryFile.is_open())
    {
        std::cerr << "\"" << fileName << "\" could not be written to." << std::endl;
        return EXIT_FAILURE;
    }

    for (unsigned int i = 0, end = binaryLines.size(); i < end; ++i)
    {
        unsigned short line = binaryLines[i];
        binaryFile << std::bitset<16>(line) << (i < end - 1 ? "\n" : "");
    }

    binaryFile.close();

    std::cout << "Assembled to " << fileName << "!" << std::endl;
    return EXIT_SUCCESS;
}
