// parser.cpp - MyLang parser implementation
#include "parser.hpp"
#include "utils.hpp"
#include <stdexcept>
#include <iostream>

static int pos = 0;
static std::vector<Token> toks;

// 静态函数，返回一个空的EOFToken
static Token dummy_eof_token() {
    // 创建一个空的EOFToken
    return Token{ TokenType::EOFToken, "" };
}

// 静态函数，用于查看下一个Token
static Token peek() {
    // 如果pos大于等于toks的大小，说明已经到达Token的末尾
    if (pos >= toks.size()) {
#if _DEBUG
        // 如果是调试模式，输出错误信息
        std::cerr << "[ERROR] peek: pos=" << pos << ", toks.size=" << toks.size() << "\n";
#endif
        // 返回一个空的Token
        return dummy_eof_token();
    }
    // 获取当前pos位置的Token
    auto t = toks[pos];
#if _DEBUG
    // 如果是调试模式，输出当前pos和Token的信息
    std::cerr << "[peek] pos=" << pos << ", token=(" << t.value << ")\n";
#endif
    // 返回当前Token
    return t;
}

// 静态函数，用于向前移动一个位置，并返回当前位置的Token
static Token advance() {
    // 如果当前位置大于等于Token列表的大小，则返回一个空的EOF Token
    if (pos >= toks.size()) {
#if _DEBUG
        // 如果是调试模式，则输出错误信息
        std::cerr << "[ERROR] advance: pos=" << pos << ", toks.size=" << toks.size() << "\n";
#endif
        return dummy_eof_token();
    }
    // 获取当前位置的Token，并将位置加1
    auto t = toks[pos++];
#if _DEBUG
    // 如果是调试模式，则输出当前位置和Token的值
    std::cerr << "[advance] pos=" << pos << ", token=(" << t.value << ")\n";
#endif
    // 返回当前位置的Token
    return t;
}

// 静态函数，用于跳过换行符
static void skip_newlines() {
    // 当下一个字符的类型为换行符时，继续前进
    while (peek().type == TokenType::Newline) {
        // 前进一个字符
        advance();
    }
}

std::shared_ptr<Statement> parse_for_statement();
std::shared_ptr<Statement> parse_statement();

AST parse(const std::vector<Token>& tokens) {
    // 将传入的tokens赋值给全局变量toks
    toks = tokens;
    // 将pos置为0
    pos = 0;
    // 创建一个AST对象
    AST ast;

    // 当pos小于toks的大小
    while (pos < toks.size()) {
        // 获取当前token
        Token current = peek();
        // 如果当前token的类型是EOFToken，则跳出循环
        if (current.type == TokenType::EOFToken) break;

        // 解析语句
        auto stmt = parse_statement();
        // 如果解析成功，则将解析结果添加到ast的statements中
        if (stmt) {
            ast.statements.push_back(stmt);
        }
        // 否则，将pos加1
        else {
            advance();
        }
    }

    // 返回ast
    return ast;
}

// 解析代码块
std::vector<std::shared_ptr<Statement>> parse_block() {
    // 存储代码块的语句
    std::vector<std::shared_ptr<Statement>> body;
    // 安全计数器，防止死循环
    int safety_counter = 0;

    while (true) {
        // 跳过空行
        skip_newlines();

        // 获取当前token
        Token current = peek();
        // 如果当前token是关键字end，则跳出循环
        if (current.type == TokenType::Keyword && current.value == "end") {
            advance(); // consume "end"
            break;
        }
        // 如果当前token是文件结束符，则抛出异常
        if (current.type == TokenType::EOFToken) {
            throw std::runtime_error("Unexpected end of file inside block.");
        }

        // 解析语句
        auto stmt = parse_statement();
        // 如果解析成功，则将语句加入代码块
        if (stmt) {
            body.push_back(stmt);
        }
        // 否则，跳过当前token
        else {
            advance();
        }

        // 如果安全计数器超过10000，则抛出异常
        if (++safety_counter > 10000) {
            throw std::runtime_error("Too many statements parsed without encountering 'end'");
        }
    }

    // 返回代码块
    return body;
}

std::shared_ptr<Statement> parse_for_statement() {
    // 消耗'for'关键字
    advance();
    
    // 获取循环变量
    Token var_token = advance();
    if (var_token.type != TokenType::Identifier) {
        throw std::runtime_error("Expected identifier after 'for'");
    }
    
    // 检查'from'关键字
    Token from_token = advance();
    if (from_token.type != TokenType::Keyword || from_token.value != "from") {
        throw std::runtime_error("Expected 'from' after loop variable");
    }
    
    // 获取起始值
    Token start_token = advance();
    try {
        int start = std::stoi(start_token.value);
    } catch (...) {
        throw std::runtime_error("Expected number after 'from'");
    }
    int start = std::stoi(start_token.value);
    
    // 检查'to'关键字
    Token to_token = advance();
    if (to_token.type != TokenType::Keyword || to_token.value != "to") {
        throw std::runtime_error("Expected 'to' after start value");
    }
    
    // 获取结束值
    Token end_token = advance();
    try {
        int end = std::stoi(end_token.value);
    } catch (...) {
        throw std::runtime_error("Expected number after 'to'");
    }
    int end = std::stoi(end_token.value);
    
    // 解析循环体
    skip_newlines();
    auto body = parse_block();
    
    // 创建并返回ForStatement
    return std::make_shared<ForStatement>(var_token.value, start, end, body);
}

std::shared_ptr<Statement> parse_statement() {
    // 跳过新行
    skip_newlines();

    // 查看下一个token
    Token tok = peek();

    // 如果是EOFToken，则返回空指针
    if (tok.type == TokenType::EOFToken) {
        return nullptr;
    }

    // for循环
    if (tok.type == TokenType::Keyword && tok.value == "for") {
        return parse_for_statement();
    }

    // function definition
    if (tok.type == TokenType::Keyword && tok.value == "function") {
        advance(); // consume 'function'

        Token name = advance();
        Token maybe_param_or_colon = advance();

        std::string param = "";

        Token colon;
        if (maybe_param_or_colon.type == TokenType::Symbol && maybe_param_or_colon.value == ":") {
            colon = maybe_param_or_colon;
        }
        else {
            param = maybe_param_or_colon.value;
            colon = advance();
            if (colon.value != ":") {
                throw std::runtime_error("Expected ':' after parameter in function definition");
            }
        }

        auto body = parse_block();
        return std::make_shared<FunctionDef>(name.value, param, body);
    }

    // start block
    if (tok.type == TokenType::Keyword && tok.value == "start") {
        advance();
        Token colon = advance();
        if (colon.value != ":") throw std::runtime_error("Expected ':' after start");
        auto body = parse_block();
        return std::make_shared<StartBlock>(body);
    }

    // say
    if (tok.type == TokenType::Keyword && tok.value == "say") {
        advance(); // consume 'say'

        std::vector<std::string> args;
        std::vector<bool> is_vars;
        std::string ending = "\\n"; // default end

        while (true) {
            Token next = peek();
#if _DEBUG
            std::cerr << "[DEBUG] say loop: next=" << next.value << ", type=" << static_cast<int>(next.type) << "\n";
#endif

            if (next.type == TokenType::Keyword && next.value == "end") {
                advance(); // consume 'end'

                Token eq = peek();
                if (eq.type != TokenType::Symbol || eq.value != "=") {
                    throw std::runtime_error("Expected '=' after 'end'");
                }
                advance(); // consume '='

                Token val = peek();
                if (val.type != TokenType::StringLiteral) {
                    throw std::runtime_error("Expected string literal after end=");
                }
                ending = advance().value;

                break;
            }

            if (next.type == TokenType::Newline || next.type == TokenType::EOFToken) {
                advance(); // consume newline/EOF
                break;
            }

            
            if (next.type == TokenType::StringLiteral || next.type == TokenType::Identifier) {
                Token arg = advance();
                args.push_back(arg.value);
                is_vars.push_back(arg.type == TokenType::Identifier);

                
                Token comma = peek();
                if (comma.type == TokenType::Symbol && comma.value == ",") {
                    advance(); // consume comma
                }
            }
            else {
                throw std::runtime_error("Unexpected token in 'say': " + next.value);
            }
        }

        if (args.size() != is_vars.size()) {
            throw std::runtime_error("Internal error: say args/vars mismatch.");
        }

        return std::make_shared<SayStatement>(args, is_vars, ending);
    }

    // set
    if (tok.type == TokenType::Keyword && tok.value == "set") {
        advance();
        Token var = advance();
        return std::make_shared<SetStatement>(var.value);
    }

    // function call
    if (tok.type == TokenType::Identifier) {
        Token func = advance();
        Token next = peek();
        if (next.type == TokenType::StringLiteral || next.type == TokenType::Identifier) {
            Token arg = advance();
#if _DEBUG
            std::cerr << "[DEBUG] function call arg " << arg.value << " ";
            switch (arg.type) {
            case TokenType::Keyword:        std::cerr << "Keyword    "; break;
            case TokenType::Identifier:     std::cerr << "Identifier "; break;
            case TokenType::StringLiteral:  std::cerr << "String     "; break;
            case TokenType::Newline:        std::cerr << "Newline    "; break;
            case TokenType::EOFToken:       std::cerr << "EOF        "; break;
            case TokenType::Symbol:         std::cerr << "Symbol     "; break;
            }
            std::cerr << std::endl;
#endif
            return std::make_shared<FunctionCall>(func.value, arg.value, arg.type);
        }
        else {
            return std::make_shared<FunctionCall>(func.value, "", TokenType::EOFToken);
        }
    }


    advance();
    return nullptr;
}
