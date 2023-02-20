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
        case CommandType::C_FUNCTION:
        case CommandType::C_CALL:
            arg1 = words[1];
            arg2 = words[2];
            break;
        case CommandType::C_LABEL:
        case CommandType::C_IF:
        case CommandType::C_GOTO:
            arg1 = words[1];
            break;
        case CommandType::C_RETURN:
            // do nothing
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
        else if (command == "label")
            return CommandType::C_LABEL;
        else if (command == "if-goto")
            return CommandType::C_IF;
        else if (command == "goto")
            return CommandType::C_GOTO;
        else if (command == "function")
            return CommandType::C_FUNCTION;
        else if (command == "return")
            return CommandType::C_RETURN;
        else if (command == "call")
            return CommandType::C_CALL;
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
        : outputstream(filename),
          label(0),
          fileName(""),
          functionName(""),
          functionLabel(0) {}

    ~CodeWriter() { outputstream.close(); }

    void Init()
    {
        outputstream << "@256\n";
        outputstream << "D=A\n";
        outputstream << "@SP\n";
        outputstream << "M=D\n";
        WriteCall("Sys.init", 0);
    }

    void SetFileName(const std::string &fileName)
    {
        this->fileName = fileName;
    }

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
            PushDToSP();
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

                PushDToSP();
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
                PopSPToD();

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
                R = fileName + "." + std::to_string(index);

            if (type == CommandType::C_PUSH)
            {
                // D=R[index+offset]
                outputstream << "@" << R << "\n";
                outputstream << "D=M\n";

                PushDToSP();
            }
            else // pop
            {
                // D=value
                PopSPToD();

                // D=R[index+5]
                outputstream << "@" << R << "\n";
                outputstream << "M=D\n";
            }
        }
    }

    void WriteLabel(const std::string &label)
    {
        InternalWriteLabel(GetFunctionLabel(label));
    }

    void WriteIf(const std::string &label)
    {
        PopSPToD();
        AtLabel(GetFunctionLabel(label));
        outputstream << "D;JNE\n";
    }

    void WriterGoto(const std::string &label)
    {
        Goto(GetFunctionLabel(label));
    }

    void WriteFunction(const std::string &functionName, int nVars)
    {
        // init function
        this->functionName = functionName;
        this->functionLabel = 0;

        InternalWriteLabel(functionName);

        for (int i = 0; i < nVars; i++)
        {
            WritePushPop(CommandType::C_PUSH, "constant", 0);
        }
    }

    void WriteReturn()
    {
        // frame = LCL
        outputstream << "@LCL\n";
        outputstream << "D=M\n";
        outputstream << "@R13\n"; // R13 as frame
        outputstream << "M=D\n";

        // retAddr = *(frame - 5)
        SetFrameTo("R14", 5);

        // *ARG=pop()
        PopSPToD(); // D=pop()
        outputstream << "@ARG\n";
        outputstream << "A=M\n"; // select ARG
        outputstream << "M=D\n";

        // SP = ARG+1
        outputstream << "@ARG\n";
        outputstream << "D=M+1\n";
        outputstream << "@SP\n";
        outputstream << "M=D\n";

        // THAT = *(frame - 1)
        SetFrameTo("THAT", 1);

        // THIS = *(frame - 2)
        SetFrameTo("THIS", 2);

        // ARG = *(frame - 3)
        SetFrameTo("ARG", 3);

        // LCL = *(frame - 4)
        SetFrameTo("LCL", 4);

        // goto retAddr
        outputstream << "@R14\n";
        outputstream << "A=M\n";
        outputstream << "0;JMP\n";
    }

    void WriteCall(const std::string &functionName, int nVars)
    {
        // push return address
        auto retLabel = GenFunctionReturnLabel();
        AtLabel(retLabel);
        outputstream << "D=A\n";
        PushDToSP();

        // push LCL
        SetRegToSP("LCL");

        // push ARG
        SetRegToSP("ARG");

        // push THIS
        SetRegToSP("THIS");

        // push THAT
        SetRegToSP("THAT");

        // ARG = SP-5-nVars
        outputstream << "@SP\n";
        outputstream << "D=M\n"; // D = SP
        outputstream << "@5\n";
        outputstream << "D=D-A\n";
        outputstream << "@" + std::to_string(nVars) + "\n";
        outputstream << "D=D-A\n";
        outputstream << "@ARG\n";
        outputstream << "M=D\n";

        // LCL = SP
        outputstream << "@SP\n";
        outputstream << "D=M\n";
        outputstream << "@LCL\n";
        outputstream << "M=D\n";

        // goto f
        outputstream << "@" + functionName + "\n";
        outputstream << "0;JMP\n";

        InternalWriteLabel(retLabel);
    }

private:
    void SetFrameTo(const std::string &varname, int offset)
    {
        // varname = *(frame - offset)
        outputstream << "@R13\n";
        outputstream << "D=M\n";                             // D=frame
        outputstream << "@" + std::to_string(offset) + "\n"; // A=offset
        outputstream << "A=D-A\n";                           // select frame-offset
        outputstream << "D=M\n";                             // D=*(frame-offset)
        outputstream << "@" + varname + "\n";
        outputstream << "M=D\n";
    }

    void SetRegToSP(const std::string &reg)
    {
        outputstream << "@" + reg + "\n";
        outputstream << "D=M\n";
        PushDToSP();
    }

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

    void PopSPToD()
    {
        SPDesc();
        SetSPToD();
    }

    void PushDToSP()
    {
        SetDToSP();
        SPInc();
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

    std::string GetFunctionLabel(const std::string &label)
    {
        if (functionName.empty())
            return label;

        return functionName + "$" + label;
    }

    std::string GenFunctionReturnLabel()
    {
        return functionName + "$ret." + std::to_string(functionLabel++);
    }

    void AtLabel(const std::string &label)
    {
        outputstream << "@" + label + "\n";
    }

    void AtLabel(int l)
    {
        AtLabel(LabelName(l));
    }

    void WriteLabel(int l)
    {
        InternalWriteLabel(LabelName(l));
    }

    void InternalWriteLabel(const std::string &label)
    {
        outputstream << "(" + label + ")\n";
    }

    void Goto(const std::string &label)
    {
        AtLabel(label);
        outputstream << "0;JMP\n";
    }

    void Goto(int l)
    {
        AtLabel(l);
        outputstream << "0;JMP\n";
    }

private:
    std::ofstream outputstream;
    int label;
    std::string fileName;
    std::string functionName;
    int functionLabel;
};

void Run(Parser &parser, CodeWriter &writer)
{
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
        case CommandType::C_LABEL:
            writer.WriteLabel(parser.Arg1());
            break;
        case CommandType::C_IF:
            writer.WriteIf(parser.Arg1());
            break;
        case CommandType::C_GOTO:
            writer.WriterGoto(parser.Arg1());
            break;
        case CommandType::C_FUNCTION:
            writer.WriteFunction(parser.Arg1(), std::stoi(parser.Arg2()));
            break;
        case CommandType::C_RETURN:
            writer.WriteReturn();
            break;
        case CommandType::C_CALL:
            writer.WriteCall(parser.Arg1(), std::stoi(parser.Arg2()));
            break;
        default:
            throw "should not reach here...";
        }
    }
}

// g++ --std=c++17 -g -O0 translator.cc -o translator
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: /bin /path/to/input/file\n";
        return 0;
    }

    std::filesystem::path input_filename(argv[1]);

    std::filesystem::path dir;
    std::filesystem::path output_filename;
    bool needInit = false;
    if (std::filesystem::is_directory(input_filename)) // dir
    {
        dir = input_filename;
        auto dirname = std::filesystem::canonical(input_filename).filename();
        auto filename = dirname.string() + ".asm";
        output_filename = input_filename.append(filename);

        needInit = true;
    }
    else // file
    {
        output_filename = input_filename.replace_extension(".asm");
        dir = input_filename.parent_path();
    }

    CodeWriter writer(output_filename);
    if (needInit)
        writer.Init();

    for (const auto &entry : std::filesystem::directory_iterator(dir))
    {
        auto path = entry.path();
        if (path.extension() != ".vm")
            continue;

        Parser parser(path.string());
        writer.SetFileName(path.stem());

        Run(parser, writer);
    }
}
