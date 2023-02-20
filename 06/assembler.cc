#include <iostream>
#include <filesystem>
#include <fstream>
#include <bitset>
#include <map>
#include <boost/algorithm/string.hpp>

enum class InstructionType
{
    A_INSTRUCTION,
    C_INSTRUCTION,
    L_INSTRUCTION,
};

class Parser
{
public:
    Parser(const std::string &filename)
        : inputstream(filename) {}

    ~Parser()
    {
        inputstream.close();
    }

    bool HasMoreLines()
    {
        return inputstream.peek() != EOF;
    }

    int Advance()
    {
        while (std::getline(inputstream, instruction))
        {
            boost::trim(instruction);
            if (instruction.empty() || instruction.substr(0, 2) == "//")
                continue;

            auto pos = instruction.find_first_of("//");
            if (pos != std::string::npos)
            {
                instruction = instruction.substr(0, pos);
                boost::trim(instruction);
            }

            break;
        }

        switch (GetInstructionType())
        {
        case InstructionType::A_INSTRUCTION:
            symbol = instruction.substr(1, instruction.size() - 1);
            lineno++;
            break;
        case InstructionType::L_INSTRUCTION:
            symbol = instruction.substr(1, instruction.size() - 2);
            break;
        case InstructionType::C_INSTRUCTION:
        {
            lineno++;

            // dest=comp;jump
            std::vector<std::string> tokens;
            boost::split(tokens, instruction, boost::is_any_of("=;"));
            auto equalPos = instruction.find('=');
            if (equalPos != std::string::npos)
            {
                dest = tokens[0];
                comp = tokens[1];
            }
            else
            {
                dest = "";
                comp = tokens[0];
            }

            auto semicolonPos = instruction.find(';');
            if (semicolonPos != std::string::npos)
                jump = tokens.back();
            else
                jump = "";

            break;
        }
        default:
            throw "should not reach here...";
        }

        return lineno;
    }

    InstructionType GetInstructionType() const
    {
        if (instruction[0] == '@')
            return InstructionType::A_INSTRUCTION;

        if (instruction[0] == '(')
            return InstructionType::L_INSTRUCTION;

        return InstructionType::C_INSTRUCTION;
    }

    std::string Symbol() const
    {
        return symbol;
    }

    std::string Dest() const
    {
        return dest;
    }

    std::string Comp() const
    {
        return comp;
    }

    std::string Jump() const
    {
        return jump;
    }

private:
    std::ifstream inputstream;
    std::string instruction;
    std::string symbol;
    std::string dest;
    std::string comp;
    std::string jump;
    int lineno = -1;
};

class Code
{
public:
    Code() = delete;

    static std::string Dest(const std::string &code)
    {
        std::string dest(3, '0');
        if (code.find('A') != std::string::npos)
            dest[0] = '1';
        if (code.find('D') != std::string::npos)
            dest[1] = '1';
        if (code.find('M') != std::string::npos)
            dest[2] = '1';

        return dest;
    }

    static std::string Comp(const std::string &code)
    {
        if (code == "0")
            return "0101010";
        else if (code == "1")
            return "0111111";
        else if (code == "-1")
            return "0111010";
        else if (code == "D")
            return "0001100";
        else if (code == "A")
            return "0110000";
        else if (code == "M")
            return "1110000";
        else if (code == "!D")
            return "0001101";
        else if (code == "!A")
            return "0110001";
        else if (code == "!M")
            return "1110001";
        else if (code == "-D")
            return "0001111";
        else if (code == "-A")
            return "0110011";
        else if (code == "-M")
            return "1110011";
        else if (code == "D+1")
            return "0011111";
        else if (code == "A+1")
            return "0110111";
        else if (code == "M+1")
            return "1110111";
        else if (code == "D-1")
            return "0001110";
        else if (code == "A-1")
            return "0110010";
        else if (code == "M-1")
            return "1110010";
        else if (code == "D+A")
            return "0000010";
        else if (code == "D+M")
            return "1000010";
        else if (code == "D-A")
            return "0010011";
        else if (code == "D-M")
            return "1010011";
        else if (code == "A-D")
            return "0000111";
        else if (code == "M-D")
            return "1000111";
        else if (code == "D&A")
            return "0000000";
        else if (code == "D&M")
            return "1000000";
        else if (code == "D|A")
            return "0010101";
        else if (code == "D|M")
            return "1010101";
        else
            throw "should not reach here...\n";
    }

    static std::string Jump(const std::string &code)
    {
        if (code == "")
            return "000";
        else if (code == "JGT")
            return "001";
        else if (code == "JEQ")
            return "010";
        else if (code == "JGE")
            return "011";
        else if (code == "JLT")
            return "100";
        else if (code == "JNE")
            return "101";
        else if (code == "JLE")
            return "110";
        else if (code == "JMP")
            return "111";
        else
            throw "should not reach here...\n";
    }
};

class SymbolTable
{
public:
    SymbolTable() = default;

    void AddEntry(const std::string &symbol, int address)
    {
        table[symbol] = address;
    }

    bool Contains(const std::string &symbol) const
    {
        return table.find(symbol) != table.end();
    }

    int GetAddress(const std::string &symbol) const
    {
        if (Contains(symbol))
            return table.at(symbol);

        return -1;
    }

private:
    std::map<std::string, int> table;
};

// g++ --std=c++17 -g -O0 assembler.cc -o assembler
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: /bin /path/to/input/file\n";
        return 0;
    }

    SymbolTable symbolTable;
    { // Build symbol table

        symbolTable.AddEntry("R0", 0);
        symbolTable.AddEntry("R1", 1);
        symbolTable.AddEntry("R2", 2);
        symbolTable.AddEntry("R3", 3);
        symbolTable.AddEntry("R4", 4);
        symbolTable.AddEntry("R5", 5);
        symbolTable.AddEntry("R6", 6);
        symbolTable.AddEntry("R7", 7);
        symbolTable.AddEntry("R8", 8);
        symbolTable.AddEntry("R9", 9);
        symbolTable.AddEntry("R10", 10);
        symbolTable.AddEntry("R11", 11);
        symbolTable.AddEntry("R12", 12);
        symbolTable.AddEntry("R13", 13);
        symbolTable.AddEntry("R14", 14);
        symbolTable.AddEntry("R15", 15);

        symbolTable.AddEntry("SP", 0);
        symbolTable.AddEntry("LCL", 1);
        symbolTable.AddEntry("ARG", 2);
        symbolTable.AddEntry("THIS", 3);
        symbolTable.AddEntry("THAT", 4);

        symbolTable.AddEntry("SCREEN", 16384);
        symbolTable.AddEntry("KBD", 24576);

        Parser parser(argv[1]);
        while (parser.HasMoreLines())
        {
            int lineno = parser.Advance();
            auto type = parser.GetInstructionType();
            switch (type)
            {
            case InstructionType::A_INSTRUCTION:
                break;
            case InstructionType::L_INSTRUCTION:
            {
                auto symbol = parser.Symbol();
                if (!symbolTable.Contains(symbol))
                    symbolTable.AddEntry(symbol, lineno + 1);

                break;
            }
            case InstructionType::C_INSTRUCTION:
                break;

            default:
                break;
            }
        }
    }

    std::vector<std::string> codes;

    int var_address = 16;
    Parser parser(argv[1]);
    while (parser.HasMoreLines())
    {
        parser.Advance();
        auto type = parser.GetInstructionType();
        switch (type)
        {
        case InstructionType::A_INSTRUCTION:
        {
            auto symbol = parser.Symbol();
            uint16_t address = 0;
            if (!isdigit(symbol[0]))
            {
                if (!symbolTable.Contains(symbol))
                {
                    symbolTable.AddEntry(symbol, var_address);
                    var_address++;
                }

                address = symbolTable.GetAddress(symbol);
            }
            else
                address = std::stoi(symbol);

            auto code = std::bitset<16>(address).to_string();
            codes.push_back(code);
            break;
        }
        case InstructionType::L_INSTRUCTION:
            break;
        case InstructionType::C_INSTRUCTION:
            codes.push_back("111" +
                            Code::Comp(parser.Comp()) +
                            Code::Dest(parser.Dest()) +
                            Code::Jump(parser.Jump()));
            break;

        default:
            break;
        }
    }

    std::filesystem::path input_filename(argv[1]);
    std::filesystem::path output_filename = input_filename.replace_extension(".hack");

    std::ofstream output_file(output_filename);
    std::ostream_iterator<std::string> output_iterator(output_file, "\n");
    std::copy(codes.begin(), codes.end(), output_iterator);
}
