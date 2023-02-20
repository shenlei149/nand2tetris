#include <string>
#include <vector>
#include <memory>
#include <set>
#include <map>
#include <fstream>
#include <variant>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <boost/algorithm/string.hpp>

enum class TokenType
{
    KEYWORD,
    SYMBOL,
    IDENTIFIER,
    INT_CONST,
    STRING_CONST,
};

enum class KeyWord
{
    CLASS,
    METHOD,
    FUNCTION,
    CONSTRUCTOR,
    INT,
    BOOLEAN,
    CHAR,
    VOID,
    VAR,
    STATIC,
    FIELD,
    LET,
    DO,
    IF,
    ELSE,
    WHILE,
    RETURN,
    TRUE,
    FALSE,
    NULLKEY,
    THIS,
};

static std::set<char> symbols = {'[', ']', '{', '}', '(', ')', '.', ',', ';', '+', '-', '*', '/', '&', '|', '>', '<', '=', '~'};
static std::map<std::string, KeyWord> keyWordMapping =
    {
        {"class", KeyWord::CLASS},
        {"method", KeyWord::METHOD},
        {"function", KeyWord::FUNCTION},
        {"constructor", KeyWord::CONSTRUCTOR},
        {"int", KeyWord::INT},
        {"boolean", KeyWord::BOOLEAN},
        {"char", KeyWord::CHAR},
        {"void", KeyWord::VOID},
        {"var", KeyWord::VAR},
        {"static", KeyWord::STATIC},
        {"field", KeyWord::FIELD},
        {"let", KeyWord::LET},
        {"do", KeyWord::DO},
        {"if", KeyWord::IF},
        {"else", KeyWord::ELSE},
        {"while", KeyWord::WHILE},
        {"return", KeyWord::RETURN},
        {"true", KeyWord::TRUE},
        {"false", KeyWord::FALSE},
        {"null", KeyWord::NULLKEY},
        {"this", KeyWord::THIS},
};

static std::string KeyWordToString(KeyWord key)
{
    switch (key)
    {
    case KeyWord::CLASS:
        return "class";
    case KeyWord::METHOD:
        return "method";
    case KeyWord::FUNCTION:
        return "function";
    case KeyWord::CONSTRUCTOR:
        return "constructor";
    case KeyWord::INT:
        return "int";
    case KeyWord::BOOLEAN:
        return "boolean";
    case KeyWord::CHAR:
        return "char";
    case KeyWord::VOID:
        return "void";
    case KeyWord::VAR:
        return "var";
    case KeyWord::STATIC:
        return "static";
    case KeyWord::FIELD:
        return "field";
    case KeyWord::LET:
        return "let";
    case KeyWord::DO:
        return "do";
    case KeyWord::IF:
        return "if";
    case KeyWord::ELSE:
        return "else";
    case KeyWord::WHILE:
        return "while";
    case KeyWord::RETURN:
        return "return";
    case KeyWord::TRUE:
        return "true";
    case KeyWord::FALSE:
        return "false";
    case KeyWord::NULLKEY:
        return "null";
    case KeyWord::THIS:
        return "this";
    }

    __builtin_unreachable();
}

static std::string CharToPrintString(char c)
{
    switch (c)
    {
    case '<':
        return "&lt;";
    case '>':
        return "&gt;";
    case '"':
        return "&quot;";
    case '&':
        return "&amp;";

    default:
        return std::string{c};
    }
}

class Tokenizer
{
public:
    Tokenizer(const std::string &filename)
        : index(-1)
    {
        std::ifstream inputstream{filename};
        std::string line;
        bool blockcomment = false;
        while (std::getline(inputstream, line))
        {
            boost::trim(line);
            if (blockcomment)
                line = HandleBlockComment(line, blockcomment);
            else
            {
                int doubleslash = line.find("//");
                int slashasterisk = line.find("/*");
                int doccomment = line.find("/**");

                // suppose we don't mix them in source code
                bool haslinecoment = doubleslash >= 0;
                bool hasblockcomment = slashasterisk >= 0;
                bool hasdoccomment = doccomment >= 0;
                if (haslinecoment)
                    line = line.substr(0, doubleslash);
                else if (hasblockcomment || hasdoccomment)
                {
                    blockcomment = true;
                    line = HandleBlockComment(line, blockcomment);
                }
            }

            boost::trim(line);
            if (line.empty())
                continue;

            auto words = Split(line);
            for (auto &&word : words)
            {
                boost::trim(word);
                if (word.empty())
                    continue;

                // KEYWORD
                auto pair = keyWordMapping.find(word);
                if (pair != keyWordMapping.end())
                {
                    tokens.push_back(std::make_pair(TokenType::KEYWORD, word));
                    continue;
                }

                // SYMBOL
                if (word.length() == 1 &&
                    symbols.find(word[0]) != symbols.end())
                {
                    tokens.push_back(std::make_pair(TokenType::SYMBOL, word));
                    continue;
                }

                // INT_CONST
                if (std::isdigit(word[0]))
                {
                    tokens.push_back(std::make_pair(TokenType::INT_CONST, word));
                    continue;
                }

                // STRING_CONST
                if (word[0] == '"')
                {
                    tokens.push_back(std::make_pair(TokenType::STRING_CONST, word.substr(1, word.length() - 2)));
                    continue;
                }

                // IDENTIFIER
                tokens.push_back(std::make_pair(TokenType::IDENTIFIER, word));
            }
        }
    }

    bool HasMoreTokens() const
    {
        return index < (int)tokens.size();
    }

    void Advance()
    {
        index++;
    }

    void GoBack()
    {
        index--;
    }

    TokenType GetTokenType() const
    {
        return tokens[index].first;
    }

    KeyWord GetKeyWord() const
    {
        return keyWordMapping[tokens[index].second];
    }

    char GetSymbol() const
    {
        return tokens[index].second[0];
    }

    const std::string &GetIdentifier() const
    {
        return tokens[index].second;
    }

    int GetIntVal() const
    {
        return std::stoi(tokens[index].second);
    }

    const std::string &GetStringVal() const
    {
        return tokens[index].second;
    }

private:
    std::string HandleBlockComment(const std::string &line, bool &begincomment)
    {
        int index = line.find("*/");
        if (index >= 0)
        {
            begincomment = false;
            return line.substr(index + 2);
        }
        else
            return "";
    }

    std::vector<std::string> Split(const std::string &line)
    {
        int previous = 0;
        std::vector<std::string> words;
        for (int i = 0; i < line.size(); i++)
        {
            if (line[i] == '"')
            {
                words.push_back(line.substr(previous, i - previous));

                int begin = i;
                i++;
                while (line[i] != '"')
                    i++;

                int end = i + 1;
                words.push_back(line.substr(begin, end - begin));
                previous = i + 1;
            }

            if (line[i] == ' ' || symbols.find(line[i]) != symbols.end())
            {
                words.push_back(line.substr(previous, i - previous));

                words.push_back(line.substr(i, 1));
                previous = i + 1;
            }
        }

        return words;
    }

private:
    std::vector<std::pair<TokenType, std::string>> tokens;
    int index;
};

enum class Segment
{
    CONSTANT,
    ARGUMENT,
    LOCAL,
    STATIC,
    THIS,
    THAT,
    POINTER,
    TEMP,
};

std::string SegemntToString(Segment seg)
{
    switch (seg)
    {
    case Segment::CONSTANT:
        return "constant";
    case Segment::ARGUMENT:
        return "argument";
    case Segment::LOCAL:
        return "local";
    case Segment::STATIC:
        return "static";
    case Segment::THIS:
        return "this";
    case Segment::THAT:
        return "that";
    case Segment::POINTER:
        return "pointer";
    case Segment::TEMP:
        return "temp";
    }

    __builtin_unreachable();
}

enum class Command
{
    ADD,
    SUB,
    NEG,
    EQ,
    GT,
    LT,
    AND,
    OR,
    NOT,
};

class VMWriter
{
public:
    VMWriter(std::ostream &output)
        : o(output)
    {
    }

    void WritePush(Segment seg, int index) const
    {
        o << "push " << SegemntToString(seg) << " " << index << "\n";
    }

    void WritePop(Segment seg, int index) const
    {
        o << "pop " << SegemntToString(seg) << " " << index << "\n";
    }

    void WriteArithemic(Command command) const
    {
        switch (command)
        {
        case Command::ADD:
            o << "add\n";
            break;
        case Command::SUB:
            o << "sub\n";
            break;
        case Command::NEG:
            o << "neg\n";
            break;
        case Command::NOT:
            o << "not\n";
            break;
        case Command::AND:
            o << "and\n";
            break;
        case Command::OR:
            o << "or\n";
            break;
        case Command::GT:
            o << "gt\n";
            break;
        case Command::LT:
            o << "lt\n";
            break;
        case Command::EQ:
            o << "eq\n";
            break;

        default:
            break;
        }
    }

    void WriteLabel(const std::string &label) const
    {
        o << "label " << label << "\n";
    }

    void WriteGoto(const std::string &label) const
    {
        o << "goto " << label << "\n";
    }

    void WriteIf(const std::string &label) const
    {
        o << "if-goto " << label << "\n";
    }

    void WriteCall(const std::string &name, int nArgs) const
    {
        o << "call " << name << " " << nArgs << "\n";
    }

    void WriteFunction(const std::string &name, int nArgs) const
    {
        o << "function " << name << " " << nArgs << "\n";
    }

    void WriteReturn() const
    {
        o << "return\n";
    }

private:
    std::ostream &o;
};

static void ConsumeChar(Tokenizer *tokenizer, char expectedChar)
{
    tokenizer->Advance();
    if (!(tokenizer->GetTokenType() == TokenType::SYMBOL &&
          tokenizer->GetSymbol() == expectedChar))
        throw "expect a symbol - " + expectedChar;
}

class JackType
{
private:
    enum class InternalType
    {
        INT,
        CHAR,
        BOOLEAN,
        VOID, // only for return
        CLASS,
    };

public:
    // int|char|boolean|className
    static std::unique_ptr<JackType> Compile(Tokenizer *tokenizer)
    {
        auto type = std::make_unique<JackType>();
        tokenizer->Advance();
        if (tokenizer->GetTokenType() == TokenType::KEYWORD)
        {
            auto keyword = tokenizer->GetKeyWord();
            switch (keyword)
            {
            case KeyWord::INT:
                type->type = InternalType::INT;
                break;
            case KeyWord::CHAR:
                type->type = InternalType::CHAR;
                break;
            case KeyWord::BOOLEAN:
                type->type = InternalType::BOOLEAN;
                break;
            case KeyWord::VOID:
                type->type = InternalType::VOID;
                break;

            default:
                throw "expect int|char|boolean";
            }
        }
        else if (tokenizer->GetTokenType() == TokenType::IDENTIFIER)
        {
            type->type = InternalType::CLASS;
            type->className = tokenizer->GetIdentifier();
        }
        else
        {
            throw "expect int|char|boolean|className";
        }

        return type;
    }

    std::string GetName() const
    {
        if (type == InternalType::CLASS)
            return className;

        return "";
    }

private:
    enum InternalType type;
    std::string className; // if type is class
};

enum class VarKind
{
    STATIC,
    FIELD,
    ARG,
    VAR,
    NONE,
};

static Segment VarKindToSegment(VarKind kind)
{
    switch (kind)
    {
    case VarKind::STATIC:
        return Segment::STATIC;
    case VarKind::FIELD:
        return Segment::THIS;
    case VarKind::ARG:
        return Segment::ARGUMENT;
    case VarKind::VAR:
        return Segment::LOCAL;

    default:
        throw "should not reach here...";
    }
}

class SymbolTable
{
    class Property
    {
    public:
        JackType type;
        VarKind kind;
        int index;
    };

public:
    SymbolTable()
        : indexes{0, 0, 0, 0}
    {
    }

    void Reset()
    {
        table.clear();
        indexes[0] = indexes[1] = indexes[2] = indexes[3] = 0;
    }

    void Define(const std::string &name, JackType type, VarKind kind)
    {
        if (table.find(name) == table.end())
        {
            table.emplace(name, Property{type, kind, VarCount(kind)});
            indexes[(int)kind]++;
        }
    }

    int VarCount(VarKind kind) const
    {
        return indexes[(int)kind];
    }

    VarKind KindOf(const std::string &name) const
    {
        auto pair = table.find(name);
        if (pair != table.end())
            return pair->second.kind;

        return VarKind::NONE;
    }

    JackType TypdOf(const std::string &name) const
    {
        return table.find(name)->second.type;
    }

    int IndexOf(const std::string &name) const
    {
        return table.find(name)->second.index;
    }

private:
    int indexes[4];
    std::map<const std::string, Property> table;
};

// TODO put them into context
static SymbolTable ClassVariables;
static SymbolTable LocalVariables;

static void GetPropertyByName(const std::string &name, VarKind &kind, int &index, JackType *type = nullptr)
{
    kind = LocalVariables.KindOf(name);
    if (kind == VarKind::NONE)
    {
        kind = ClassVariables.KindOf(name);
        if (kind != VarKind::NONE)
        {
            index = ClassVariables.IndexOf(name);
            if (type)
                *type = ClassVariables.TypdOf(name);
        }

        return;
    }

    index = LocalVariables.IndexOf(name);
    if (type)
        *type = LocalVariables.TypdOf(name);
}

class ClassVarDec
{
public:
    // (static | field) type varName(, varName)* ';'
    static std::unique_ptr<ClassVarDec> Compile(Tokenizer *tokenizer)
    {
        tokenizer->Advance();
        if (tokenizer->GetTokenType() == TokenType::KEYWORD)
        {
            auto dec = std::make_unique<ClassVarDec>();
            auto keyword = tokenizer->GetKeyWord();
            if (keyword == KeyWord::STATIC)
                dec->isStatic = true;
            else if (keyword == KeyWord::FIELD)
                dec->isStatic = false;
            else
                goto EXIT;

            dec->type = JackType::Compile(tokenizer);

            tokenizer->Advance();
            if (tokenizer->GetTokenType() == TokenType::IDENTIFIER)
                dec->varNames.push_back(tokenizer->GetIdentifier());
            else
                throw "expect an identifier for var name";

            while (true)
            {
                tokenizer->Advance();
                if (tokenizer->GetTokenType() == TokenType::SYMBOL)
                {
                    auto symbol = tokenizer->GetSymbol();
                    if (symbol == ';')
                        break;
                    else if (symbol == ',')
                    {
                        tokenizer->Advance();
                        if (tokenizer->GetTokenType() == TokenType::IDENTIFIER)
                            dec->varNames.push_back(tokenizer->GetIdentifier());
                        else
                            throw "expect an identifier for var name";
                    }
                    else
                        throw "expect a symbol ,|;";
                }
                else
                    throw "expect a symbol ,|;";
            }

            return dec;
        }

    EXIT:
        tokenizer->GoBack();
        return nullptr;
    }

    void FillVarTable() const
    {
        VarKind kind = isStatic ? VarKind::STATIC : VarKind::FIELD;

        for (auto &&name : varNames)
            ClassVariables.Define(name, *(type.get()), kind);
    }

    int FieldCount() const
    {
        if (isStatic)
            return 0;

        return varNames.size();
    }

private:
    bool isStatic; // false means field
    std::unique_ptr<JackType> type;
    std::vector<std::string> varNames;
};

class ParameterList
{
public:
    // ((type varName) (, type varName)*)?
    static std::unique_ptr<ParameterList> Compile(Tokenizer *tokenizer)
    {
        tokenizer->Advance();
        if (tokenizer->GetTokenType() == TokenType::KEYWORD ||
            tokenizer->GetTokenType() == TokenType::IDENTIFIER)
        {
            auto list = std::make_unique<ParameterList>();
            tokenizer->GoBack();
            auto type = JackType::Compile(tokenizer);
            list->types.push_back(std::move(type));

            tokenizer->Advance();
            if (tokenizer->GetTokenType() == TokenType::IDENTIFIER)
                list->names.push_back(tokenizer->GetIdentifier());
            else
                throw "expect an identifier for var name";

            while (true)
            {
                tokenizer->Advance();
                if (tokenizer->GetTokenType() == TokenType::SYMBOL &&
                    tokenizer->GetSymbol() == ',')
                {
                    auto type = JackType::Compile(tokenizer);
                    list->types.push_back(std::move(type));

                    tokenizer->Advance();
                    if (tokenizer->GetTokenType() == TokenType::IDENTIFIER)
                        list->names.push_back(tokenizer->GetIdentifier());
                    else
                        throw "expect an identifier for var name";
                }
                else
                {
                    tokenizer->GoBack();
                    break;
                }
            }

            return list;
        }

        tokenizer->GoBack();
        return nullptr;
    }

    void FillVarTable() const
    {
        for (size_t i = 0; i < types.size(); i++)
            LocalVariables.Define(names[i], *(types[i].get()), VarKind::ARG);
    }

private:
    std::vector<std::unique_ptr<JackType>> types;
    std::vector<std::string> names;
};

enum class SubroutineType
{
    CONSTRUCTOR,
    FUNCTION,
    METHOD,
};

class Context
{
public:
    std::string className;
    int nFields;
    SubroutineType subroutineType;
};

class Term;

class Expression
{
public:
    static std::unique_ptr<Expression> Compile(Tokenizer *tokenizer);

    void GenVMCode(const VMWriter &writer, Context &context);

private:
    std::unique_ptr<Term> term;

    std::vector<char> ops;
    std::vector<std::unique_ptr<Term>> terms;
};

class ExpressionList
{
public:
    // only used by SubroutineCall, which will check if ExpressionList is empty
    // so this contains at least one expression
    // (expression (,expression)*)?
    static std::unique_ptr<ExpressionList> Compile(Tokenizer *tokenizer)
    {
        auto list = std::make_unique<ExpressionList>();

        list->expression = Expression::Compile(tokenizer);
        while (true)
        {
            tokenizer->Advance();
            if (tokenizer->GetTokenType() == TokenType::SYMBOL &&
                tokenizer->GetSymbol() == ',')
                list->expressions.push_back(Expression::Compile(tokenizer));
            else
            {
                tokenizer->GoBack();
                break;
            }
        }

        return list;
    }

    void GenVMCode(const VMWriter &writer, Context &context)
    {
        expression->GenVMCode(writer, context);
        for (auto &&expr : expressions)
            expr->GenVMCode(writer, context);
    }

    int Count() const
    {
        return expressions.size() + 1;
    }

private:
    std::unique_ptr<Expression> expression;
    std::vector<std::unique_ptr<Expression>> expressions;
};

class SubroutineCall
{
public:
    // subroutineName( expressionList ) | (className|varName).subroutineName( expressionList )
    static std::unique_ptr<SubroutineCall> Compile(Tokenizer *tokenizer)
    {
        auto call = std::make_unique<SubroutineCall>();
        tokenizer->Advance();
        if (tokenizer->GetTokenType() == TokenType::IDENTIFIER)
        {
            auto firstId = tokenizer->GetIdentifier();

            tokenizer->Advance();
            if (tokenizer->GetTokenType() == TokenType::SYMBOL)
            {
                auto symbol = tokenizer->GetSymbol();
                if (symbol == '(')
                {
                    call->subroutineName = firstId;
                    call->identifierName = "";

                    HandleExpressionList(tokenizer, call.get());
                }
                else if (symbol == '.')
                {
                    tokenizer->Advance();
                    if (tokenizer->GetTokenType() == TokenType::IDENTIFIER)
                    {
                        call->subroutineName = tokenizer->GetIdentifier();
                        call->identifierName = firstId;

                        ConsumeChar(tokenizer, '(');

                        HandleExpressionList(tokenizer, call.get());
                    }
                    else
                        throw "expect an identifier for subroutine name";
                }
                else
                    throw "expect a symbol - (|.";
            }
            else
                throw "expect a symbol - (|.";
        }
        else
            throw "expect an identifier for subroutine name or class|var name";

        return call;
    }

    void GenVMCode(const VMWriter &writer, Context &context)
    {
        int initNArgs = 0;
        if (identifierName == "")
        {
            // call method, push this
            writer.WritePush(Segment::POINTER, 0);
            initNArgs = 1;
            identifierName = context.className;
        }
        else
        {
            VarKind kind = VarKind::NONE;
            int index;
            JackType type;
            GetPropertyByName(identifierName, kind, index, &type);

            switch (kind)
            {
            case VarKind::FIELD:
                writer.WritePush(Segment::THIS, index);
                initNArgs = 1;
                identifierName = type.GetName();
                break;
            case VarKind::STATIC:
                break;
            case VarKind::VAR:
                writer.WritePush(Segment::LOCAL, index);
                initNArgs = 1;
                identifierName = type.GetName();
                break;
            case VarKind::ARG:
                break;

            default:
                // not found. call function.
                break;
            }
        }

        if (expressionList)
            expressionList->GenVMCode(writer, context);

        auto name = identifierName + "." + subroutineName;
        writer.WriteCall(name, expressionList ? expressionList->Count() + initNArgs : initNArgs);
    }

private:
    static void HandleExpressionList(Tokenizer *tokenizer, SubroutineCall *call)
    {
        tokenizer->Advance();
        auto nextType = tokenizer->GetTokenType();
        if (nextType == TokenType::SYMBOL &&
            tokenizer->GetSymbol() == ')')
            call->expressionList = nullptr;
        else
        {
            tokenizer->GoBack();

            call->expressionList = ExpressionList::Compile(tokenizer);

            ConsumeChar(tokenizer, ')');
        }
    }

private:
    std::string subroutineName;
    std::string identifierName;
    std::unique_ptr<ExpressionList> expressionList;
};

static std::set<char> UnaryOps{'-', '~'};

enum class TermType
{
    INT_CONST,
    STRING_CONST,
    KEYWORD_CONST,
    VARNAME,
    VAR_EXPRESSION,
    WHOLE_EXPRESSION,
    UNARYOP,
    SUBROUTINECALL,
};

static void PushVar(const VMWriter &writer, const Context &context, const std::string &varName)
{
    VarKind kind = VarKind::NONE;
    int index = 0;
    GetPropertyByName(varName, kind, index);

    if (context.subroutineType == SubroutineType::METHOD &&
        kind == VarKind::ARG)
        index++;

    writer.WritePush(VarKindToSegment(kind), index);
}

class Term
{
public:
    // integerConstant | stringConstant | keywordConst | varName |
    // varName'['expression']' | '('expression')' | (unaryOp term) | subroutineCall
    static std::unique_ptr<Term> Compile(Tokenizer *tokenizer)
    {
        auto term = std::make_unique<Term>();
        tokenizer->Advance();
        switch (tokenizer->GetTokenType())
        {
        case TokenType::INT_CONST:
        {
            term->termType = TermType::INT_CONST;
            term->intConst = tokenizer->GetIntVal();
            break;
        }
        case TokenType::STRING_CONST:
        {
            term->termType = TermType::STRING_CONST;
            term->stringConst = tokenizer->GetStringVal();
            break;
        }
        case TokenType::KEYWORD:
        {
            term->termType = TermType::KEYWORD_CONST;
            auto keyword = tokenizer->GetKeyWord();
            switch (keyword)
            {
            case KeyWord::TRUE:
            case KeyWord::FALSE:
            case KeyWord::NULLKEY:
            case KeyWord::THIS:
                term->keywordConst = keyword;
                break;

            default:
                throw "not expect keyword " + KeyWordToString(keyword);
            }
            break;
        }
        case TokenType::IDENTIFIER:
        {
            term->termType = TermType::VARNAME;
            term->varName = tokenizer->GetIdentifier();

            tokenizer->Advance();
            if (tokenizer->GetTokenType() == TokenType::SYMBOL)
            {
                auto symbol = tokenizer->GetSymbol();
                if (symbol == '[')
                {
                    term->termType = TermType::VAR_EXPRESSION;
                    term->varExpr = Expression::Compile(tokenizer);

                    ConsumeChar(tokenizer, ']');
                }
                else if (symbol == '.')
                {
                    term->termType = TermType::SUBROUTINECALL;

                    tokenizer->GoBack(); // .
                    tokenizer->GoBack(); // identifier
                    term->subroutineCall = SubroutineCall::Compile(tokenizer);
                }
                else
                    tokenizer->GoBack();
            }

            break;
        }
        case TokenType::SYMBOL:
        {
            auto symbol = tokenizer->GetSymbol();
            if (symbol == '(')
            {
                term->termType = TermType::WHOLE_EXPRESSION;
                term->wholeExpr = Expression::Compile(tokenizer);
                ConsumeChar(tokenizer, ')');
            }
            else if (UnaryOps.find(symbol) != UnaryOps.end())
            {
                term->termType = TermType::UNARYOP;
                term->unaryChar = symbol;
                term->unaryTerm = Term::Compile(tokenizer);
            }
            else
                throw "expect a symbol (|-|~";

            break;
        }

        default:
            throw "unexpected token";
        }

        return term;
    }

    void GenVMCode(const VMWriter &writer, Context &context)
    {
        switch (termType)
        {
        case TermType::INT_CONST:
        {
            writer.WritePush(Segment::CONSTANT, intConst);
            break;
        }
        case TermType::STRING_CONST:
        {
            writer.WritePush(Segment::CONSTANT, stringConst.size());
            writer.WriteCall("String.new", 1);

            for (auto &&c : stringConst)
            {
                writer.WritePush(Segment::CONSTANT, c);
                writer.WriteCall("String.appendChar", 2);
            }

            break;
        }
        case TermType::KEYWORD_CONST:
        {
            switch (keywordConst)
            {
            case KeyWord::TRUE:
                writer.WritePush(Segment::CONSTANT, 0);
                writer.WriteArithemic(Command::NOT);
                break;
            case KeyWord::FALSE:
                writer.WritePush(Segment::CONSTANT, 0);
                break;
            case KeyWord::THIS:
                writer.WritePush(Segment::POINTER, 0);
                break;
            case KeyWord::NULLKEY:
                writer.WritePush(Segment::CONSTANT, 0);
                break;

            default:
                break;
            }
            break;
        }
        case TermType::VARNAME:
        {
            PushVar(writer, context, varName);
            break;
        }
        case TermType::VAR_EXPRESSION:
        {
            varExpr->GenVMCode(writer, context);
            PushVar(writer, context, varName);
            writer.WriteArithemic(Command::ADD);
            writer.WritePop(Segment::POINTER, 1);
            writer.WritePush(Segment::THAT, 0);
            break;
        }
        case TermType::SUBROUTINECALL:
        {
            subroutineCall->GenVMCode(writer, context);
            break;
        }
        case TermType::WHOLE_EXPRESSION:
        {
            wholeExpr->GenVMCode(writer, context);
            break;
        }
        case TermType::UNARYOP:
        {
            unaryTerm->GenVMCode(writer, context);
            if (unaryChar == '~')
                writer.WriteArithemic(Command::NOT);
            else if (unaryChar == '-')
                writer.WriteArithemic(Command::NEG);
            break;
        }

        default:
            break;
        }
    }

private:
    TermType termType;
    std::string varName;
    std::unique_ptr<Expression> varExpr;
    std::unique_ptr<Expression> wholeExpr;
    char unaryChar;
    std::unique_ptr<Term> unaryTerm;
    KeyWord keywordConst;
    std::unique_ptr<SubroutineCall> subroutineCall;
    int intConst;
    std::string stringConst;
};

static std::set<char> Ops{'+', '-', '*', '/', '&', '|', '>', '<', '='};

static void GenOpVMCode(char op, const VMWriter &writer)
{
    switch (op)
    {
    case '+':
        writer.WriteArithemic(Command::ADD);
        break;
    case '-':
        writer.WriteArithemic(Command::SUB);
        break;
    case '*':
        writer.WriteCall("Math.multiply", 2);
        break;
    case '/':
        writer.WriteCall("Math.divide", 2);
        break;
    case '&':
        writer.WriteArithemic(Command::AND);
        break;
    case '|':
        writer.WriteArithemic(Command::OR);
        break;
    case '>':
        writer.WriteArithemic(Command::GT);
        break;
    case '<':
        writer.WriteArithemic(Command::LT);
        break;
    case '=':
        writer.WriteArithemic(Command::EQ);
        break;

    default:
        break;
    }
}

// term (op term)*
std::unique_ptr<Expression> Expression::Compile(Tokenizer *tokenizer)
{
    auto expr = std::make_unique<Expression>();

    expr->term = Term::Compile(tokenizer);
    while (true)
    {
        tokenizer->Advance();
        if (tokenizer->GetTokenType() == TokenType::SYMBOL)
        {
            auto symbol = tokenizer->GetSymbol();
            if (Ops.find(symbol) != Ops.end())
            {
                expr->ops.push_back(symbol);
                expr->terms.push_back(Term::Compile(tokenizer));
            }
            else
                goto BREAK;
        }
        else
        {
        BREAK:
            tokenizer->GoBack();
            break;
        }
    }

    return expr;
}

void Expression::GenVMCode(const VMWriter &writer, Context &context)
{
    term->GenVMCode(writer, context);

    for (size_t i = 0; i < ops.size(); i++)
    {
        terms[i]->GenVMCode(writer, context);
        GenOpVMCode(ops[i], writer);
    }
}

// TODO put them into context
static int whilelabel = -1;
static int iflabel = -1;

static void ResetLabelIndex()
{
    whilelabel = -1;
    iflabel = -1;
}

static int GetWhileLabelIndex()
{
    whilelabel++;
    return whilelabel;
}

static std::string GetWhileExprLabel(int index)
{
    return "WHILE_EXP" + std::to_string(index);
}

static std::string GetWhileEndLabel(int index)
{
    return "WHILE_END" + std::to_string(index);
}

static int GetIfLabelIndex()
{
    iflabel++;
    return iflabel;
}

static std::string GetIfTrueLabel(int index)
{
    return "IF_TRUE" + std::to_string(index);
}

static std::string GetIfFalseLabel(int index)
{
    return "IF_FALSE" + std::to_string(index);
}
static std::string GetIfEndLabel(int index)
{
    return "IF_END" + std::to_string(index);
}

class Statement
{
public:
    virtual void GenVMCode(const VMWriter &writer, Context &context) = 0;
};

class Statements
{
public:
    static std::unique_ptr<Statements> Compile(Tokenizer *tokenizer);
    void GenVMCode(const VMWriter &writer, Context &context);

private:
    std::vector<std::unique_ptr<Statement>> statements;
};

class LetStatement : public Statement
{
public:
    // let varName([expression])?=expression;
    // 'let' has been taken by Statement
    static std::unique_ptr<LetStatement> Compile(Tokenizer *tokenizer)
    {
        auto ret = std::make_unique<LetStatement>();

        tokenizer->Advance();
        if (tokenizer->GetTokenType() == TokenType::IDENTIFIER)
            ret->varName = tokenizer->GetIdentifier();
        else
            throw "expect an identifier for var name";

        tokenizer->Advance();
        if (tokenizer->GetTokenType() == TokenType::SYMBOL)
        {
            auto symbol = tokenizer->GetSymbol();
            if (symbol == '=')
                ret->indexExpr = nullptr;
            else if (symbol == '[')
            {
                ret->indexExpr = Expression::Compile(tokenizer);

                ConsumeChar(tokenizer, ']');

                ConsumeChar(tokenizer, '=');
            }
            else
                throw "expect a symbol - =|[";
        }
        else
            throw "expect a symbol - =|[";

        ret->rightExpr = Expression::Compile(tokenizer);

        ConsumeChar(tokenizer, ';');

        return ret;
    }

    void GenVMCode(const VMWriter &writer, Context &context) override
    {
        if (indexExpr)
        {
            indexExpr->GenVMCode(writer, context);
            PushVar(writer, context, varName);
            writer.WriteArithemic(Command::ADD);
        }

        rightExpr->GenVMCode(writer, context);

        if (indexExpr)
        {
            writer.WritePop(Segment::TEMP, 0);    // save right
            writer.WritePop(Segment::POINTER, 1); // save left to that
            writer.WritePush(Segment::TEMP, 0);
            writer.WritePop(Segment::THAT, 0);
        }
        else
        {
            VarKind kind = VarKind::NONE;
            int index = 0;
            GetPropertyByName(varName, kind, index);
            writer.WritePop(VarKindToSegment(kind), index);
        }
    }

private:
    std::string varName;
    std::unique_ptr<Expression> indexExpr;
    std::unique_ptr<Expression> rightExpr;
};

class IfStatement : public Statement
{
public:
    // if (expression) {statements} (else {statements})?
    // 'if' has been taken by Statement
    static std::unique_ptr<IfStatement> Compile(Tokenizer *tokenizer)
    {
        auto ret = std::make_unique<IfStatement>();
        ConsumeChar(tokenizer, '(');

        ret->conditionExpr = Expression::Compile(tokenizer);

        ConsumeChar(tokenizer, ')');

        ConsumeChar(tokenizer, '{');

        ret->ifBody = Statements::Compile(tokenizer);

        ConsumeChar(tokenizer, '}');

        tokenizer->Advance();
        if (tokenizer->GetTokenType() == TokenType::KEYWORD &&
            tokenizer->GetKeyWord() == KeyWord::ELSE)
        {
            ConsumeChar(tokenizer, '{');

            ret->elseBody = Statements::Compile(tokenizer);

            ConsumeChar(tokenizer, '}');
        }
        else
        {
            ret->elseBody = nullptr;
            tokenizer->GoBack();
        }

        return ret;
    }

    void GenVMCode(const VMWriter &writer, Context &context) override
    {
        int index = GetIfLabelIndex();
        auto iflabel = GetIfTrueLabel(index);
        auto elselabel = GetIfFalseLabel(index);
        auto ifend = GetIfEndLabel(index);

        conditionExpr->GenVMCode(writer, context);
        writer.WriteIf(iflabel);
        writer.WriteGoto(elselabel);

        writer.WriteLabel(iflabel);
        ifBody->GenVMCode(writer, context);

        // if no else, elselabel is endlabel
        if (elseBody)
            writer.WriteGoto(ifend);

        writer.WriteLabel(elselabel);
        if (elseBody)
            elseBody->GenVMCode(writer, context);

        if (elseBody)
            writer.WriteLabel(ifend);
    }

private:
    std::unique_ptr<Expression> conditionExpr;
    std::unique_ptr<Statements> ifBody;
    std::unique_ptr<Statements> elseBody;
};

class WhileStatement : public Statement
{
public:
    // while (expression) {statements}
    // 'while' has been taken by Statement
    static std::unique_ptr<WhileStatement> Compile(Tokenizer *tokenizer)
    {
        auto ret = std::make_unique<WhileStatement>();

        ConsumeChar(tokenizer, '(');

        ret->conditionExpr = Expression::Compile(tokenizer);

        ConsumeChar(tokenizer, ')');

        ConsumeChar(tokenizer, '{');

        ret->whileBody = Statements::Compile(tokenizer);

        ConsumeChar(tokenizer, '}');

        return ret;
    }

    void GenVMCode(const VMWriter &writer, Context &context) override
    {
        int index = GetWhileLabelIndex();
        auto exprlabel = GetWhileExprLabel(index);
        auto endlabel = GetWhileEndLabel(index);

        writer.WriteLabel(exprlabel);
        conditionExpr->GenVMCode(writer, context);
        writer.WriteArithemic(Command::NOT);
        writer.WriteIf(endlabel);
        whileBody->GenVMCode(writer, context);
        writer.WriteGoto(exprlabel);
        writer.WriteLabel(endlabel);
    }

private:
    std::unique_ptr<Expression> conditionExpr;
    std::unique_ptr<Statements> whileBody;
};

class DoStatement : public Statement
{
public:
    // do subroutineCall;
    // 'do' has been taken by Statement
    static std::unique_ptr<DoStatement> Compile(Tokenizer *tokenizer)
    {
        auto ret = std::make_unique<DoStatement>();
        ret->subroutineCall = SubroutineCall::Compile(tokenizer);

        ConsumeChar(tokenizer, ';');

        return ret;
    }

    void GenVMCode(const VMWriter &writer, Context &context) override
    {
        subroutineCall->GenVMCode(writer, context);

        writer.WritePop(Segment::TEMP, 0);
    }

private:
    std::unique_ptr<SubroutineCall> subroutineCall;
};

class ReturnsStatement : public Statement
{
public:
    // return expression? ;
    static std::unique_ptr<ReturnsStatement> Compile(Tokenizer *tokenizer)
    {
        auto ret = std::make_unique<ReturnsStatement>();

        tokenizer->Advance();
        if (tokenizer->GetTokenType() == TokenType::SYMBOL &&
            tokenizer->GetSymbol() == ';')
            ret->returnExpr = nullptr;
        else
        {
            tokenizer->GoBack();
            ret->returnExpr = Expression::Compile(tokenizer);

            ConsumeChar(tokenizer, ';');
        }

        return ret;
    }

    void GenVMCode(const VMWriter &writer, Context &context) override
    {
        if (returnExpr)
            returnExpr->GenVMCode(writer, context);
        else
            writer.WritePush(Segment::CONSTANT, 0);

        writer.WriteReturn();
    }

private:
    std::unique_ptr<Expression> returnExpr;
};

std::unique_ptr<Statements> Statements::Compile(Tokenizer *tokenizer)
{
    auto ret = std::make_unique<Statements>();

    while (true)
    {
        // Statement: let | if | while | do | return
        tokenizer->Advance();
        if (tokenizer->GetTokenType() == TokenType::KEYWORD)
        {
            // here we don't go back then sub statement should
            // not handle it again.
            auto keyword = tokenizer->GetKeyWord();
            switch (keyword)
            {
            case KeyWord::LET:
                ret->statements.push_back(LetStatement::Compile(tokenizer));
                continue;
            case KeyWord::IF:
                ret->statements.push_back(IfStatement::Compile(tokenizer));
                continue;
            case KeyWord::WHILE:
                ret->statements.push_back(WhileStatement::Compile(tokenizer));
                continue;
            case KeyWord::DO:
                ret->statements.push_back(DoStatement::Compile(tokenizer));
                continue;
            case KeyWord::RETURN:
                ret->statements.push_back(ReturnsStatement::Compile(tokenizer));
                continue;

            default:
                goto BREAK;
            }
        }

    BREAK:
        tokenizer->GoBack();
        break;
    }

    return ret;
}

void Statements::GenVMCode(const VMWriter &writer, Context &context)
{
    for (auto &&statement : statements)
        statement->GenVMCode(writer, context);
}

class VarDec
{
public:
    // var type varName (, varName)* ;
    static std::unique_ptr<VarDec> Compile(Tokenizer *tokenizer)
    {
        tokenizer->Advance();
        if (tokenizer->GetTokenType() == TokenType::KEYWORD &&
            tokenizer->GetKeyWord() == KeyWord::VAR)
        {
            auto var = std::make_unique<VarDec>();
            var->type = JackType::Compile(tokenizer);

            while (true)
            {
                tokenizer->Advance();
                if (tokenizer->GetTokenType() == TokenType::IDENTIFIER)
                    var->names.push_back(tokenizer->GetIdentifier());
                else
                    throw "expect an identifier for var name";

                tokenizer->Advance();
                if (tokenizer->GetTokenType() == TokenType::SYMBOL)
                {
                    auto symbol = tokenizer->GetSymbol();
                    if (symbol == ';')
                        break;
                    else if (symbol == ',')
                        continue;
                    else
                        throw "expect a symbol - ,|;";
                }
                else
                    throw "expect a symbol - ,|;";
            }

            return var;
        }

        tokenizer->GoBack();
        return nullptr;
    }

    void FillVarTable() const
    {
        for (auto &&name : names)
            LocalVariables.Define(name, *(type.get()), VarKind::VAR);
    }

    int VarCount() const
    {
        return names.size();
    }

private:
    std::unique_ptr<JackType> type;
    std::vector<std::string> names;
};

class SubroutineBody
{
public:
    // '{' varDec* statements '}'
    static std::unique_ptr<SubroutineBody> Compile(Tokenizer *tokenizer)
    {
        ConsumeChar(tokenizer, '{');

        auto body = std::make_unique<SubroutineBody>();

        std::unique_ptr<VarDec> var = nullptr;
        while ((var = VarDec::Compile(tokenizer)) != nullptr)
            body->varDecs.push_back(std::move(var));

        body->statements = Statements::Compile(tokenizer);

        ConsumeChar(tokenizer, '}');

        return body;
    }

    void GenVMCode(const VMWriter &writer, Context &context)
    {
        for (auto &&var : varDecs)
            var->FillVarTable();

        if (context.subroutineType == SubroutineType::CONSTRUCTOR)
        {
            writer.WritePush(Segment::CONSTANT, context.nFields);
            writer.WriteCall("Memory.alloc", 1);
            writer.WritePop(Segment::POINTER, 0);
        }
        else if (context.subroutineType == SubroutineType::METHOD)
        {
            writer.WritePush(Segment::ARGUMENT, 0);
            writer.WritePop(Segment::POINTER, 0);
        }

        statements->GenVMCode(writer, context);
    }

    int VarCount() const
    {
        int sum = 0;
        for (auto &&var : varDecs)
            sum += var->VarCount();

        return sum;
    }

private:
    std::vector<std::unique_ptr<VarDec>> varDecs;
    std::unique_ptr<Statements> statements;
};

static std::string SubroutineTypeToString(SubroutineType type)
{
    switch (type)
    {
    case SubroutineType::CONSTRUCTOR:
        return "constructor";
    case SubroutineType::FUNCTION:
        return "function";
    case SubroutineType::METHOD:
        return "method";
    }

    __builtin_unreachable();
}

class SubroutineDec
{
public:
    // (constructor|function|method) type subroutineName '(' parameterList ')' subroutineBody
    static std::unique_ptr<SubroutineDec> Compile(Tokenizer *tokenizer)
    {
        tokenizer->Advance();
        if (tokenizer->GetTokenType() == TokenType::KEYWORD)
        {
            auto dec = std::make_unique<SubroutineDec>();
            auto keyword = tokenizer->GetKeyWord();
            switch (keyword)
            {
            case KeyWord::CONSTRUCTOR:
                dec->subroutineType = SubroutineType::CONSTRUCTOR;
                break;
            case KeyWord::FUNCTION:
                dec->subroutineType = SubroutineType::FUNCTION;
                break;
            case KeyWord::METHOD:
                dec->subroutineType = SubroutineType::METHOD;
                break;
            default:
                goto EXIT;
            }

            dec->returnType = JackType::Compile(tokenizer);

            tokenizer->Advance();
            if (tokenizer->GetTokenType() == TokenType::IDENTIFIER)
                dec->routineName = tokenizer->GetIdentifier();
            else
                throw "expect an identifier for subroutineName";

            ConsumeChar(tokenizer, '(');

            dec->parameters = ParameterList::Compile(tokenizer);

            ConsumeChar(tokenizer, ')');

            dec->subroutineBody = SubroutineBody::Compile(tokenizer);

            return dec;
        }

    EXIT:
        tokenizer->GoBack();
        return nullptr;
    }

    void GenVMCode(const VMWriter &writer, Context &context)
    {
        writer.WriteFunction(context.className + "." + routineName, subroutineBody->VarCount());

        LocalVariables.Reset();
        ResetLabelIndex();
        if (parameters)
            parameters->FillVarTable();

        context.subroutineType = subroutineType;

        subroutineBody->GenVMCode(writer, context);
    }

private:
    SubroutineType subroutineType;
    std::unique_ptr<JackType> returnType;
    std::string routineName;
    std::unique_ptr<ParameterList> parameters;
    std::unique_ptr<SubroutineBody> subroutineBody;
};

class JackClass
{
public:
    // 'class' className '{' classVarDec* subroutineDec* '}'
    static std::unique_ptr<JackClass> Compile(Tokenizer *tokenizer)
    {
        tokenizer->Advance();
        if (!(tokenizer->GetTokenType() == TokenType::KEYWORD &&
              tokenizer->GetKeyWord() == KeyWord::CLASS))
            throw "expect a class key word";

        auto jackClass = std::make_unique<JackClass>();

        tokenizer->Advance();
        if (tokenizer->GetTokenType() == TokenType::IDENTIFIER)
            jackClass->className = tokenizer->GetIdentifier();
        else
            throw "expect an identifier for class name";

        ConsumeChar(tokenizer, '{');

        std::unique_ptr<ClassVarDec> varDec = nullptr;
        while ((varDec = ClassVarDec::Compile(tokenizer)) != nullptr)
            jackClass->varDecs.push_back(std::move(varDec));

        std::unique_ptr<SubroutineDec> subroutineDec = nullptr;
        while ((subroutineDec = SubroutineDec::Compile(tokenizer)) != nullptr)
            jackClass->subroutineDecs.push_back(std::move(subroutineDec));

        ConsumeChar(tokenizer, '}');

        return jackClass;
    }

    void GenVMCode(const VMWriter &writer)
    {
        ClassVariables.Reset();
        Context context;
        context.className = className;

        int nFields = 0;
        for (auto &&varDec : varDecs)
        {
            varDec->FillVarTable();
            nFields += varDec->FieldCount();
        }

        context.nFields = nFields;

        for (auto &&subroutineDec : subroutineDecs)
            subroutineDec->GenVMCode(writer, context);
    }

private:
    std::string className;
    std::vector<std::unique_ptr<ClassVarDec>> varDecs;
    std::vector<std::unique_ptr<SubroutineDec>> subroutineDecs;
};

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: /bin /path/to/input/file\n";
        return 0;
    }

    std::filesystem::path input_filename(argv[1]);

    std::filesystem::path dir;
    if (std::filesystem::is_directory(input_filename)) // dir
    {
        dir = input_filename;
    }
    else // file
    {
        dir = input_filename.parent_path();
    }

    for (const auto &entry : std::filesystem::directory_iterator(dir))
    {
        auto path = entry.path();
        if (path.extension() != ".jack")
            continue;

        Tokenizer tokenizer(path.string());
        auto jackClass = JackClass::Compile(&tokenizer);

        auto filename = path.stem().string();
        path.replace_filename(filename + ".vm.g");
        std::ofstream output(path);
        VMWriter writer(output);
        jackClass->GenVMCode(writer);
        output.close();
    }
}
