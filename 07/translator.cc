#include <iostream>
#include <filesystem>
#include <fstream>
#include <bitset>
#include <map>
#include <vector>
#include <boost/algorithm/string.hpp>

enum class CommandType
{
    C_ARITHMETIC,
    C_PUSH,
    C_POP,
    C_LABEL,
    C_GOTO,
    C_IF,
    C_FUNCTION,
    C_RETURN,
    C_CALL,
};

static const std::vector<std::string> operators = {"add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not"};

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

    void Advance()
    {
        while (std::getline(inputstream, command))
        {
            boost::trim(command);
            if (command.empty() || command.substr(0, 2) == "//")
                continue;

            auto pos = command.find_first_of("//");
            if (pos != std::string::npos)
            {
                command = command.substr(0, pos);
                boost::trim(command);
            }

            break;
        }

        std::vector<std::string> words;
        boost::split(words, command, boost::is_any_of(" "));
        command = words[0];

        switch (GetCommandType())
        {
        case CommandType::C_ARITHMETIC:
            arg1 = words[0];
            break;
        case CommandType::C_PUSH:
        case CommandType::C_POP:
            arg1 = words[1];
            arg2 = words[2];
            break;
        default:
            throw "should not reach here...";
        }
    }

    CommandType GetCommandType() const
    {
        if (std::find(operators.begin(), operators.end(), command) != operators.end())
            return CommandType::C_ARITHMETIC;
        else if (command == "push")
            return CommandType::C_PUSH;
        else if (command == "pop")
            return CommandType::C_POP;
        else
            throw "should not reach here...";
    }

    std::string Arg1() const
    {
        return arg1;
    }

    std::string Arg2() const
    {
        return arg2;
    }

private:
    std::ifstream inputstream;
    std::string command;
    std::string arg1;
    std::string arg2;
};

class CodeWriter
{
public:
    CodeWriter(const std::filesystem::path &filename)
        : outputstream(filename), label(0), prefix(filename.stem()) {}

    ~CodeWriter() { outputstream.close(); }

    void WriteOperator(const std::string &op)
    {
        if (op != "neg" && op != "not")
        {
            SPDesc();

            // D=RAM[SP] D=y
            outputstream << "@SP\n";
            outputstream << "A=M\n";
            outputstream << "D=M\n";

            SPDesc();

            // M=D+M M=x
            outputstream << "@SP\n";
            outputstream << "A=M\n";

            if (op == "add")
                outputstream << "M=D+M\n";
            else if (op == "sub")
                outputstream << "M=M-D\n";
            else if (op == "eq" || op == "gt" || op == "lt")
            {
                outputstream << "D=M-D\n";
                auto to = NewLabel();
                auto back = NewLabel();

                std::string jump;
                if (op == "eq")
                    jump = "JEQ";
                else if (op == "gt")
                    jump = "JGT";
                else
                    jump = "JLT";

                AtLabel(to);
                outputstream << "D;" + jump + "\n";
                SetSP(0);
                Goto(back);

                WriteLabel(to);
                SetSP(-1);
                Goto(back);

                WriteLabel(back);
            }
            else if (op == "and")
                outputstream << "M=D&M\n";
            else if (op == "or")
                outputstream << "M=D|M\n";

            SPInc();
        }
        else // eq not
        {
            SPDesc();

            // D=RAM[SP] M=x
            outputstream << "@SP\n";
            outputstream << "A=M\n";

            if (op == "neg")
                outputstream << "M=-M\n";
            else
                outputstream << "M=!M\n";

            SPInc();
        }
    }

    void WritePushPop(CommandType type, const std::string &segment, int index)
    {
        if (segment == "constant")
        {
            assert(type == CommandType::C_PUSH);

            // D=index
            outputstream << "@" << std::to_string(index) << "\n";
            outputstream << "D=A\n";

            // RAM[SP]=D
            SetDToSP();

            SPInc();
        }
        // local, argument, this, that, and temp
        // LCL, ARG, THIS, and THAT
        else if (segment == "local" || segment == "argument" || segment == "this" || segment == "that")
        {
            std::string base;
            if (segment == "local")
                base = "LCL";
            else if (segment == "argument")
                base = "ARG";
            else if (segment == "this")
                base = "THIS";
            else if (segment == "that")
                base = "THAT";

            if (type == CommandType::C_PUSH)
            {
                // D=index
                outputstream << "@" << std::to_string(index) << "\n";
                outputstream << "D=A\n";

                // D = value at base+index
                outputstream << "@" + base + "\n";
                outputstream << "A=D+M\n";
                outputstream << "D=M\n";

                SetDToSP();
                SPInc();
            }
            else // pop
            {
                // D=index
                outputstream << "@" << std::to_string(index) << "\n";
                outputstream << "D=A\n";

                // D = address(base+index)
                outputstream << "@" + base + "\n";
                outputstream << "D=D+M\n";

                // R13=D=address(base+index)
                outputstream << "@R13\n";
                outputstream << "M=D\n";

                // D=value
                SPDesc();
                SetSPToD();

                // select base+index
                outputstream << "@R13\n";
                outputstream << "A=M\n";

                outputstream << "M=D\n";
            }
        }
        else if (segment == "temp" || segment == "pointer" || segment == "static")
        {
            int offset = 5;
            if (segment == "pointer")
                offset = 3;

            auto R = "R" + std::to_string(index + offset);
            if (segment == "static")
                R = prefix + "." + std::to_string(index);

            if (type == CommandType::C_PUSH)
            {
                // D=R[index+offset]
                outputstream << "@" << R << "\n";
                outputstream << "D=M\n";

                SetDToSP();
                SPInc();
            }
            else // pop
            {
                // D=value
                SPDesc();
                SetSPToD();

                // D=R[index+5]
                outputstream << "@" << R << "\n";
                outputstream << "M=D\n";
            }
        }
    }

private:
    void SPInc()
    {
        // SP++
        outputstream << "@SP\n";
        outputstream << "M=M+1\n";
    }

    void SPDesc()
    {
        // SP--
        outputstream << "@SP\n";
        outputstream << "M=M-1\n";
    }

    void SelectSP()
    {
        outputstream << "@SP\n";
        outputstream << "A=M\n";
    }

    void SetDToSP()
    {
        SelectSP();
        outputstream << "M=D\n";
    }

    void SetSPToD()
    {
        SelectSP();
        outputstream << "D=M\n";
    }

    void SetSP(int value)
    {
        SelectSP();
        outputstream << "M=" + std::to_string(value) + "\n";
    }

    int NewLabel()
    {
        return label++;
    }

    static std::string LabelName(int label)
    {
        return "Label" + std::to_string(label);
    }

    void AtLabel(int l)
    {
        outputstream << "@" + LabelName(l) + "\n";
    }

    void WriteLabel(int l)
    {
        outputstream << "(" + LabelName(l) + ")\n";
    }

    void Goto(int l)
    {
        AtLabel(l);
        outputstream << "0;JMP\n";
    }

private:
    std::ofstream outputstream;
    int label;
    std::string prefix;
};

// g++ --std=c++17 -g -O0 translator.cc -o translator
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: /bin /path/to/input/file\n";
        return 0;
    }

    Parser parser(argv[1]);

    std::filesystem::path input_filename(argv[1]);
    std::filesystem::path output_filename = input_filename.replace_extension(".asm");
    CodeWriter writer(output_filename);

    while (parser.HasMoreLines())
    {
        parser.Advance();
        auto type = parser.GetCommandType();
        switch (type)
        {
        case CommandType::C_ARITHMETIC:
            writer.WriteOperator(parser.Arg1());
            break;
        case CommandType::C_PUSH:
        case CommandType::C_POP:
            writer.WritePushPop(type, parser.Arg1(), std::stoi(parser.Arg2()));
            break;
        default:
            throw "should not reach here...";
        }
    }
}
