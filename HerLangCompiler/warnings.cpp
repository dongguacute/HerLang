// warnings.cpp - Indentation warning analyzer
#include "warnings.hpp"
#include "utils.hpp"
#include <sstream>
#include <iostream>
#include <stack>

void check_indentation(const std::string& source) {
    // 从源代码中读取每一行
    std::istringstream in(source);
    std::string line;
    int lineno = 1;

    // 使用栈来存储缩进
    std::stack<int> indent_stack;

    // 遍历每一行
    while (std::getline(in, line)) {
        // 去除行首和行尾的空格
        std::string trimmed = trim(line);
        // 如果行是空的或者以#开头，则跳过
        if (trimmed.empty() || trimmed[0] == '#') {
            lineno++;
            continue;
        }

        // 计算行的缩进
        int indent = 0;
        for (char c : line) {
            if (c == ' ') indent++;
            else break;
        }

        // 如果行是end，则检查栈顶的缩进是否匹配
        if (trimmed == "end") {
            if (indent_stack.empty()) {
                std::cerr << "[Warning] Line " << lineno << ": 'end' without matching block start.\n";
            }
            else {
                int expected_indent = indent_stack.top();
                if (indent != expected_indent) {
                    std::cerr << "[Warning] Line " << lineno << ": 'end' indentation mismatch. Expected "
                        << expected_indent << " spaces but got " << indent << ".\n";
                }
                indent_stack.pop();
            }
        }
        // 如果行是function、start:、if、elif、else，则将缩进压入栈中
        else if (trimmed.find("function") == 0 || trimmed.find("start:") == 0 ||
            trimmed.find("if") == 0 || trimmed.find("elif") == 0 || trimmed.find("else") == 0) {
            indent_stack.push(indent);
        }
        else {
            if (!indent_stack.empty()) {
                int expected_indent = indent_stack.top();
                if (indent <= expected_indent) {
                    std::cerr << "[Warning] Line " << lineno << ": Inconsistent indentation. Expected greater than "
                        << expected_indent << " spaces but got " << indent << ".\n";
                }
            }
        }
        lineno++;
    }

    if (!indent_stack.empty()) {
        std::cerr << "[Warning] EOF: Some blocks not closed properly (missing 'end').\n";
    }
}
