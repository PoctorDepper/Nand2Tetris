#include <algorithm>
#include <format>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

const std::unordered_map<std::string, std::string> SEGMENT = 
{
    {"local", "LCL"},
    {"argument", "ARG"},
    {"this", "THIS"},
    {"that", "THAT"}
};

const std::unordered_map<std::string, short> STATIC =
{
    {"static", 16},
    {"temp", 5},
    {"pointer", 3},
};

// For a string switch statement, 4 digit primes should suffice
constexpr unsigned int hash(const char *s) {
    unsigned int hash = 3797;

    while (*s)
    {
        hash = (hash * 5381) ^ (s[0] * 4523);
        ++s;
    }

    return hash;                           
}   

std::vector<std::string> parse_arguments(std::string line)
{
    std::vector<std::string> arguments;

    for (auto it = line.begin(); it != line.end(); ++it)
    {
        // Skip over any whitespace characters
        if (std::isspace(*it)) continue;

        // If we found a non-whitespace character, iterate until we don't
        std::string argument = "";
        while (!std::isspace(*it) && it != line.end())
        {
            argument.push_back(*it);
            ++it;
        }

        arguments.push_back(argument);
    }
    
    return arguments;
}

// Validates that i is a valid number, returns -1 if not
short validate_i(std::string i_string)
{
    short i;

    try
    {
        int eval = std::stoi(i_string);
        if (eval < 0 || eval > 0x7FFF) throw std::exception(); // used as an error break
        i = eval;
    }
    // Not using the exception
    catch (std::exception&) 
    {
        return -1;
    }

    return i;
}

// push segment i
std::vector<std::string> push(std::vector<std::string> arguments)
{
    std::vector<std::string> assembly;
    if (arguments.size() < 3) return assembly;

    short i = validate_i(arguments[2]);
    if (i < 0) return assembly;

    assembly.push_back(std::format("// push {} {}", arguments[1], arguments[2]));

    std::vector<std::string> segment;

    if (SEGMENT.contains(arguments[1]))
    {
        // Get segment + i
        assembly.push_back("@" + arguments[2]);
        assembly.push_back("D=A");
        assembly.push_back("@" + SEGMENT.at(arguments[1]));
        assembly.push_back("A=D+M");
        // Grab value at segment + i
        assembly.push_back("D=M");

    }
    else if(STATIC.contains(arguments[1]))
    {
        assembly.push_back(std::format("@{}", i + STATIC.at(arguments[1])));
        assembly.push_back("D=M");
    }
    else
    {
        assembly.push_back("@" + arguments[2]);
        assembly.push_back("D=A");
    }

    // Increment stack pointer and go to previous stack location
    assembly.push_back("@SP");
    assembly.push_back("AM=M+1");
    assembly.push_back("A=A-1");
    // Push the value to the top of the stack
    assembly.push_back("M=D");
    

    return assembly;
}

// pop segment i
std::vector<std::string> pop(std::vector<std::string> arguments)
{
    std::vector<std::string> assembly;
    if (arguments.size() < 3) return assembly;

    short i = validate_i(arguments[2]);
    if (i < 0) return assembly;

    assembly.push_back(std::format("// pop {} {}", arguments[1], arguments[2]));

    if (SEGMENT.contains(arguments[1]))
    {
        // Get RAM[segment+i]
        assembly.push_back("@" + arguments[2]);
        assembly.push_back("D=A");
        assembly.push_back("@" + SEGMENT.at(arguments[1]));
        assembly.push_back("D=D+M");
        // Go to stack and decrement pointer
        assembly.push_back("@SP");
        assembly.push_back("AM=M-1");
        // Swap RAM[SP] and D
        assembly.push_back("M=D+M");
        assembly.push_back("D=M-D");
        // Go to segment+i
        assembly.push_back("A=M-D");
    }
    else if (STATIC.contains(arguments[1]))
    {
        // Go to stack and decrement pointer
        assembly.push_back("@SP");
        assembly.push_back("AM=M-1");
        // Grab RAM[SP]
        assembly.push_back("D=M");
        // Go to segement+i
        assembly.push_back(std::format("@{}", i + STATIC.at(arguments[1])));
    }
    else
    {
        return std::vector<std::string>();
    }

    // RAM[segment+i] <- RAM[SP] (the only common line between all symbols)
    assembly.push_back("M=D");

    return assembly;
} 

// add, sub, and, or
std::vector<std::string> double_arithmetic(const char operation)
{
    std::vector<std::string> assembly;

    assembly.push_back(std::format("// x{}y", operation));
    // Decrement and grab the stack pointer
    assembly.push_back("@SP");
    assembly.push_back("AM=M-1");
    // Grab 'y'
    assembly.push_back("D=M");
    // Go to 'x' and replace it with "x[OP]y"
    assembly.push_back("A=A-1");
    // We write to D as well for the evaluation call
    assembly.push_back(std::format("MD=M{}D", operation));

    return assembly;
}

// neg, not
std::vector<std::string> single_arithmetic(const char operation)
{
    std::vector<std::string> assembly;

    assembly.push_back(std::format("// {}x", operation));
    // Grab the stack pointer - 1
    assembly.push_back("@SP");
    assembly.push_back("A=M-1");
    // Replace 'x' with "[OP]x"
    assembly.push_back(std::format("M={}M", operation));

    return assembly;
}

// eq, gt, lt
std::vector<std::string> evalutation(std::string eval, short line)
{
    std::vector<std::string> assembly = double_arithmetic('-');

    std::string jump, comment = "// ";
    switch (hash(eval.c_str()))
    {
        case hash("eq"):
            comment += "x=y";
            jump = "JNE";
            break;

        case hash("gt"):
            comment += "x>y";
            jump = "JLE";
            break;

        case hash("lt"):
            comment += "x<y";
            jump = "JGE";
            break;
    }
    assembly.push_back(comment);

    // D and A come primed from double_arithmetic
    assembly.push_back("M=0");
    assembly.push_back(std::format("@EVAL{}", line));
    assembly.push_back("D;" + jump);
    // True
    assembly.push_back("@SP");
    assembly.push_back("A=M-1");
    assembly.push_back("M=-1");
    // False
    assembly.push_back(std::format("(EVAL{})", line));
    

    return assembly;
}


int main(const int argc, const char* argv[])
{
    // Check for file argument
    if (argc <= 1)
    {
        std::cerr << "You must pass a file as an argument." << std::endl;
        return EXIT_FAILURE;
    }

    // Check file ending (not super secure, but it's something)
    std::string fileName = argv[1];
    if (fileName.substr(fileName.find_last_of('.')) != ".vm")
    {
        std::cerr << std::format("\"{}\" is not a VM file.", fileName) << std::endl;
        return EXIT_FAILURE;
    }

    // Check that the file actually opened
    std::ifstream vmFile(fileName, std::ios::in);
    if (!vmFile.is_open())
    {
        std::cerr << std::format("\"{}\" could not be opened.", fileName) << std::endl;
        return EXIT_FAILURE;
    }

    // Read and store all the lines from the file
    std::vector<std::string> content;
    for (std::string line; std::getline(vmFile, line);)
    {
        content.push_back(line);
    }
    vmFile.close();

    std::vector<std::string> assembly;
    short evalCount;
    for (auto it = content.begin(); it != content.end(); ++it)
    {
        // Ignore whitespace lines and comments
        if (std::all_of(it->begin(),it->end(),isspace) || it->substr(0, 2) == "//") continue;

        // Parse the line
        std::vector<std::string> arguments = parse_arguments(it->substr(0,it->find("//")));
        // std::vector<std::string> arguments = parse_arguments(*it);

        std::vector<std::string> output;

        switch (hash(arguments[0].c_str()))
        {
            case hash("push"):
                output = push(arguments);
                break;
                
            case hash("pop"):
                output = pop(arguments);
                break;

            case hash("add"):
                output = double_arithmetic('+');
                break;

            case hash("sub"):
                output = double_arithmetic('-');
                break;

            case hash("neg"):
                output = single_arithmetic('-');
                break;

            case hash("eq"):
                output = evalutation("eq", std::distance(content.begin(), it));
                break;

            case hash("gt"):
                output = evalutation("gt", std::distance(content.begin(), it));
                break;

            case hash("lt"):
                output = evalutation("lt", std::distance(content.begin(), it));
                break;

            case hash("and"):
                output = double_arithmetic('&');
                break;

            case hash("or"):
                output = double_arithmetic('|');
                break;

            case hash("not"):
                output = single_arithmetic('!');
                break;

            case hash("label"):
                if (arguments.size() < 2) break;
                output.push_back(std::format("({})", arguments[1]));
                break;

            case hash("goto"):
                if (arguments.size() < 2) break;
                output.push_back(std::format("@{}", arguments[1]));
                output.push_back("0;JMP");
                break;

            case hash("if-goto"):
                break;

            case hash("Function"):
                break;

            case hash("Call"):
                break;

            case hash("return"):
                break;
            
            default:
                std::cerr << std::format("Invalid operation at line {}.", std::distance(content.begin(), it) + 1) << std::endl;
                return EXIT_FAILURE;
        }

        // If the output is empty, error, otherwise append
        if (output.empty())
        {
            std::cerr << std::format("Invalid argument(s) at line {}.", std::distance(content.begin(), it) + 1) << std::endl;
            return EXIT_FAILURE; 
        }
        assembly.insert(assembly.end(), output.begin(), output.end());
    }

    // Write to .asm file
    fileName = fileName.substr(0, fileName.find_last_of('.')) + ".asm";
    std::ofstream asmFile(fileName, std::ios::out);
    if (!asmFile.is_open())
    {
        std::cerr << "\"" << fileName << "\" could not be written to." << std::endl;
        return EXIT_FAILURE;
    }

    for (auto eIt = assembly.begin(); eIt != assembly.end(); ++eIt)
    {
        asmFile << *eIt;
        if (eIt != --assembly.end()) asmFile << std::endl;
    }

    asmFile.close();

    std::cout << "Translated to " << fileName << "!" << std::endl;
    return EXIT_SUCCESS;
}