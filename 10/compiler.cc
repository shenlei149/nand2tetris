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

    std::string DumpXml() const
    {
        std::stringstream xml;

        xml << "<tokens>\n";

        for (auto &&token : tokens)
        {
            switch (token.first)
            {
            case TokenType::KEYWORD:
            {
                xml << "<keyword> " << token.second << " </keyword>\n";
                break;
            }
            case TokenType::SYMBOL:
            {

                xml << "<symbol> " << CharToPrintString(token.second[0]) << " </symbol>\n";
                break;
            }
            case TokenType::IDENTIFIER:
            {
                xml << "<identifier> " << token.second << " </identifier>\n";
                break;
            }
            case TokenType::INT_CONST:
            {
                xml << "<integerConstant> " << token.second << " </integerConstant>\n";
                break;
            }
            case TokenType::STRING_CONST:
            {
                xml << "<stringConstant> " << token.second << " </stringConstant>\n";
                break;
            }

            default:
                throw "error";
            }
        }

        xml << "</tokens>\n";

        return xml.str();
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

    void DumpXml(std::ostream &output, int indent)
    {
        std::string indentStr(indent, ' ');
        output << indentStr;
        switch (type)
        {
        case InternalType::INT:
            output << "<keyword> int </keyword>\n";
            break;
        case InternalType::CHAR:
            output << "<keyword> char </keyword>\n";
            break;
        case InternalType::BOOLEAN:
            output << "<keyword> boolean </keyword>\n";
            break;
        case InternalType::VOID:
            output << "<keyword> void </keyword>\n";
            break;
        case InternalType::CLASS:
            output << "<identifier> " << className << " </identifier>\n";
            break;

        default:
            break;
        }
    }

private:
    enum InternalType type;
    std::string className; // if type is class
};

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

    void DumpXml(std::ostream &output, int indent)
    {
        std::string indentStr(indent, ' ');
        std::string innerIndentStr(indent + 2, ' ');
        output << indentStr << "<classVarDec>\n";
        output << innerIndentStr << "<keyword> "
               << (isStatic ? "static" : "field")
               << " </keyword>\n";

        type->DumpXml(output, indent + 2);

        for (size_t i = 0; i < varNames.size(); i++)
        {
            if (i != 0)
                output << innerIndentStr << "<symbol> , </symbol>\n";

            output << innerIndentStr << "<identifier> " << varNames[i] << " </identifier>\n";
        }

        output << innerIndentStr << "<symbol> ; </symbol>\n";
        output << indentStr << "</classVarDec>\n";
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

    void DumpXml(std::ostream &output, int indent)
    {
        std::string indentStr(indent, ' ');
        std::string innerIndentStr(indent + 2, ' ');
        output << indentStr << "<parameterList>\n";

        for (size_t i = 0; i < types.size(); i++)
        {
            if (i != 0)
                output << innerIndentStr << "<symbol> , </symbol>\n";

            types[i]->DumpXml(output, indent + 2);
            output << innerIndentStr << "<identifier> " << names[i] << " </identifier>\n";
        }

        output << indentStr << "</parameterList>\n";
    }

private:
    std::vector<std::unique_ptr<JackType>> types;
    std::vector<std::string> names;
};

class Term;

class Expression
{
public:
    static std::unique_ptr<Expression> Compile(Tokenizer *tokenizer);

    void DumpXml(std::ostream &output, int indent);

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

    void DumpXml(std::ostream &output, int indent)
    {
        std::string indentStr(indent, ' ');
        std::string innerIndentStr(indent + 2, ' ');

        expression->DumpXml(output, indent + 2);
        for (auto &&expr : expressions)
        {
            output << innerIndentStr << "<symbol> , </symbol>\n";
            expr->DumpXml(output, indent + 2);
        }
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

    void DumpXml(std::ostream &output, int indent)
    {
        std::string indentStr(indent, ' ');
        if (identifierName != "")
        {
            output << indentStr << "<identifier> " << identifierName << " </identifier>\n";
            output << indentStr << "<symbol> . </symbol>\n";
        }

        output << indentStr << "<identifier> " << subroutineName << " </identifier>\n";
        output << indentStr << "<symbol> ( </symbol>\n";
        output << indentStr << "<expressionList>\n";

        if (expressionList)
            expressionList->DumpXml(output, indent);

        output << indentStr << "</expressionList>\n";
        output << indentStr << "<symbol> ) </symbol>\n";
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

    void DumpXml(std::ostream &output, int indent)
    {
        std::string indentStr(indent, ' ');
        std::string innerIndentStr(indent + 2, ' ');
        output << indentStr << "<term>\n";

        switch (termType)
        {
        case TermType::INT_CONST:
        {
            output << innerIndentStr << "<integerConstant> " << intConst << " </integerConstant>\n";
            break;
        }
        case TermType::STRING_CONST:
        {
            output << innerIndentStr << "<stringConstant> " << stringConst << " </stringConstant>\n";
            break;
        }
        case TermType::KEYWORD_CONST:
        {
            output << innerIndentStr << "<keyword> " + KeyWordToString(keywordConst) << " </keyword>\n";
            break;
        }
        case TermType::VARNAME:
        {
            output << innerIndentStr << "<identifier> " << varName << " </identifier>\n";
            break;
        }
        case TermType::VAR_EXPRESSION:
        {
            output << innerIndentStr << "<identifier> " << varName << " </identifier>\n";
            output << innerIndentStr << "<symbol> [ </symbol>\n";
            varExpr->DumpXml(output, indent + 2);
            output << innerIndentStr << "<symbol> ] </symbol>\n";
            break;
        }
        case TermType::SUBROUTINECALL:
        {
            subroutineCall->DumpXml(output, indent + 2);
            break;
        }
        case TermType::WHOLE_EXPRESSION:
        {
            output << innerIndentStr << "<symbol> ( </symbol>\n";
            wholeExpr->DumpXml(output, indent + 2);
            output << innerIndentStr << "<symbol> ) </symbol>\n";
            break;
        }
        case TermType::UNARYOP:
        {
            output << innerIndentStr << "<symbol> " << unaryChar << " </symbol>\n";
            unaryTerm->DumpXml(output, indent + 2);
            break;
        }

        default:
            break;
        }

        output << indentStr << "</term>\n";
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

void Expression::DumpXml(std::ostream &output, int indent)
{
    std::string indentStr(indent, ' ');
    std::string innerIndentStr(indent + 2, ' ');

    output << indentStr << "<expression>\n";

    term->DumpXml(output, indent + 2);

    for (size_t i = 0; i < ops.size(); i++)
    {
        output << innerIndentStr << "<symbol> " << CharToPrintString(ops[i]) << " </symbol>\n";
        terms[i]->DumpXml(output, indent + 2);
    }

    output << indentStr << "</expression>\n";
}

class Statement
{
public:
    virtual void DumpXml(std::ostream &output, int indent) = 0;
};

class Statements
{
public:
    static std::unique_ptr<Statements> Compile(Tokenizer *tokenizer);
    void DumpXml(std::ostream &output, int indent);

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

    void DumpXml(std::ostream &output, int indent) override
    {
        std::string indentStr(indent, ' ');
        std::string innerIndentStr(indent + 2, ' ');
        output << indentStr << "<letStatement>\n";
        output << innerIndentStr << "<keyword> let </keyword>\n";
        output << innerIndentStr << "<identifier> " << varName << " </identifier>\n";

        if (indexExpr)
        {
            output << innerIndentStr << "<symbol> [ </symbol>\n";
            indexExpr->DumpXml(output, indent + 2);
            output << innerIndentStr << "<symbol> ] </symbol>\n";
        }

        output << innerIndentStr << "<symbol> = </symbol>\n";

        rightExpr->DumpXml(output, indent + 2);

        output << innerIndentStr << "<symbol> ; </symbol>\n";

        output << indentStr << "</letStatement>\n";
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

    void DumpXml(std::ostream &output, int indent) override
    {
        std::string indentStr(indent, ' ');
        std::string innerIndentStr(indent + 2, ' ');
        output << indentStr << "<ifStatement>\n";
        output << innerIndentStr << "<keyword> if </keyword>\n";
        output << innerIndentStr << "<symbol> ( </symbol>\n";
        conditionExpr->DumpXml(output, indent + 2);
        output << innerIndentStr << "<symbol> ) </symbol>\n";
        output << innerIndentStr << "<symbol> { </symbol>\n";
        ifBody->DumpXml(output, indent + 2);
        output << innerIndentStr << "<symbol> } </symbol>\n";
        if (elseBody)
        {
            output << innerIndentStr << "<keyword> else </keyword>\n";
            output << innerIndentStr << "<symbol> { </symbol>\n";
            elseBody->DumpXml(output, indent + 2);
            output << innerIndentStr << "<symbol> } </symbol>\n";
        }
        output << indentStr << "</ifStatement>\n";
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

    void DumpXml(std::ostream &output, int indent)
    {
        std::string indentStr(indent, ' ');
        std::string innerIndentStr(indent + 2, ' ');
        output << indentStr << "<whileStatement>\n";
        output << innerIndentStr << "<keyword> while </keyword>\n";
        output << innerIndentStr << "<symbol> ( </symbol>\n";
        conditionExpr->DumpXml(output, indent + 2);
        output << innerIndentStr << "<symbol> ) </symbol>\n";
        output << innerIndentStr << "<symbol> { </symbol>\n";
        whileBody->DumpXml(output, indent + 2);
        output << innerIndentStr << "<symbol> } </symbol>\n";
        output << indentStr << "</whileStatement>\n";
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

    void DumpXml(std::ostream &output, int indent)
    {
        std::string indentStr(indent, ' ');
        std::string innerIndentStr(indent + 2, ' ');
        output << indentStr << "<doStatement>\n";
        output << innerIndentStr << "<keyword> do </keyword>\n";
        subroutineCall->DumpXml(output, indent + 2);
        output << innerIndentStr << "<symbol> ; </symbol>\n";
        output << indentStr << "</doStatement>\n";
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

    void DumpXml(std::ostream &output, int indent)
    {
        std::string indentStr(indent, ' ');
        std::string innerIndentStr(indent + 2, ' ');
        output << indentStr << "<returnStatement>\n";
        output << innerIndentStr << "<keyword> return </keyword>\n";
        if (returnExpr)
            returnExpr->DumpXml(output, indent + 2);

        output << innerIndentStr << "<symbol> ; </symbol>\n";
        output << indentStr << "</returnStatement>\n";
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

void Statements::DumpXml(std::ostream &output, int indent)
{
    std::string indentStr(indent, ' ');
    output << indentStr << "<statements>\n";

    for (auto &&statement : statements)
        statement->DumpXml(output, indent + 2);

    output << indentStr << "</statements>\n";
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

    void DumpXml(std::ostream &output, int indent)
    {
        std::string indentStr(indent, ' ');
        std::string innerIndentStr(indent + 2, ' ');
        output << indentStr << "<varDec>\n";
        output << innerIndentStr << "<keyword> var </keyword>\n";
        type->DumpXml(output, indent + 2);
        for (size_t i = 0; i < names.size(); i++)
        {
            if (i != 0)
                output << innerIndentStr << "<symbol> , </symbol>\n";

            output << innerIndentStr << "<identifier> " << names[i] << " </identifier>\n";
        }

        output << innerIndentStr << "<symbol> ; </symbol>\n";
        output << indentStr << "</varDec>\n";
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

    void DumpXml(std::ostream &output, int indent)
    {
        std::string indentStr(indent, ' ');
        std::string innerIndentStr(indent + 2, ' ');
        output << indentStr << "<subroutineBody>\n";
        output << innerIndentStr << "<symbol> { </symbol>\n";

        for (auto &&var : varDecs)
            var->DumpXml(output, indent + 2);

        statements->DumpXml(output, indent + 2);

        output << innerIndentStr << "<symbol> } </symbol>\n";
        output << indentStr << "</subroutineBody>\n";
    }

private:
    std::vector<std::unique_ptr<VarDec>> varDecs;
    std::unique_ptr<Statements> statements;
};

enum class SubroutineType
{
    CONSTRUCTOR,
    FUNCTION,
    METHOD,
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

    void DumpXml(std::ostream &output, int indent)
    {
        std::string indentStr(indent, ' ');
        std::string innerIndentStr(indent + 2, ' ');
        output << indentStr << "<subroutineDec>\n";

        output << innerIndentStr << "<keyword> "
               << SubroutineTypeToString(subroutineType) << " </keyword>\n";

        returnType->DumpXml(output, indent + 2);

        output << innerIndentStr << "<identifier> " << routineName << " </identifier>\n";
        output << innerIndentStr << "<symbol> ( </symbol>\n";

        if (parameters)
            parameters->DumpXml(output, indent + 2);
        else
        {
            output << innerIndentStr << "<parameterList>\n";
            output << innerIndentStr << "</parameterList>\n";
        }

        output << innerIndentStr << "<symbol> ) </symbol>\n";

        subroutineBody->DumpXml(output, indent + 2);

        output << indentStr << "</subroutineDec>\n";
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

    void DumpXml(std::ostream &output, int indent = 0)
    {
        output << "<class>\n";

        std::string indentStr(indent + 2, ' ');

        output << indentStr << "<keyword> class </keyword>\n";
        output << indentStr << "<identifier> " << className << " </identifier>\n";
        output << indentStr << "<symbol> { </symbol>\n";

        for (auto &&varDec : varDecs)
            varDec->DumpXml(output, indent + 2);

        for (auto &&subroutineDec : subroutineDecs)
            subroutineDec->DumpXml(output, indent + 2);

        output << indentStr << "<symbol> } </symbol>\n";
        output << "</class>\n";
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
        auto filename = path.stem().string();
        path.replace_filename(filename + "T.xml.g");
        path.replace_filename(filename + "T.xml.g");
        std::ofstream tokenizerOutput(path);
        tokenizerOutput << tokenizer.DumpXml();
        tokenizerOutput.close();

        path.replace_filename(filename + ".xml.g");
        std::ofstream compilerOutput(path);
        auto jackClass = JackClass::Compile(&tokenizer);
        jackClass->DumpXml(compilerOutput);
        compilerOutput.close();
    }
}
