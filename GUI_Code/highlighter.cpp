#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // Instruction Format (Yellow)
    instructionFormat.setForeground(QColor(230, 180, 80));
    QStringList instructions = {
        "lui", "auipc", "jal", "jalr",
        "beq", "bne", "blt", "bge", "bltu", "bgeu",
        "lb", "lh", "lw", "lbu", "lhu",
        "sb", "sh", "sw",
        "addi", "slti", "sltiu", "xori", "ori", "andi",
        "slli", "srli", "srai",
        "add", "sub", "sll", "slt", "sltu", "xor",
        "srl", "sra", "or", "and",
        "mul", "mulh", "mulhsu", "mulhu", "div", "divu", "rem", "remu"
    };

    for (const QString &inst : instructions) {
        HighlightRule rule;
        // Match whole words only; negative lookahead avoids partial matches
        rule.pattern = QRegularExpression("\\b" + inst + "\\b(?![\\w.])");
        rule.format = instructionFormat;
        highlightRules.append(rule);
    }

    // Register Format (Red)
    registerFormat.setForeground(QColor(240, 113, 120));
    for (int i = 0; i <= 31; ++i) {
        HighlightRule rule;
        rule.pattern = QRegularExpression("\\bx" + QString::number(i) + "\\b(?![\\w.])");
        rule.format = registerFormat;
        highlightRules.append(rule);
    }

    // Directives Format (Orange)
    directiveFormat.setForeground(QColor(255, 143, 64)); // Orange color
    QStringList directives = {
        ".text", ".data", ".globl", ".word", ".asciz",
        ".section", ".align", ".byte", ".half", ".dword",
        ".skip", ".zero", ".string"
    };

    for (const QString &dir : directives) {
        HighlightRule rule;
        // Escape the directive (important since it starts with a dot)
        QString escapedDir = QRegularExpression::escape(dir);
        // Match if preceded by start-of-line or whitespace and followed by whitespace or end-of-line.
        rule.pattern = QRegularExpression("(?<=\\s|^)" + escapedDir + "(?=\\s|$)");
        rule.format = directiveFormat;
        highlightRules.append(rule);
    }

    // Immediate Values Format (Blue)
    immediateFormat.setForeground(QColor(230, 180, 80));
    // This regex matches a number (with optional sign and hex prefix) followed by non-space characters until the next whitespace.
    HighlightRule immediateRule;
    immediateRule.pattern = QRegularExpression(R"(\b[-+]?(0[xX][0-9a-fA-F]+|\d+)[^\s]*\b)");
    immediateRule.format = immediateFormat;
    highlightRules.append(immediateRule);

    // String Format (Green)
    stringFormat.setForeground(QColor(194, 217, 76));

    // Double-quoted strings
    HighlightRule doubleQuoteStringRule;
    doubleQuoteStringRule.pattern = QRegularExpression("\"[^\"]*\"");
    doubleQuoteStringRule.format = stringFormat;
    highlightRules.append(doubleQuoteStringRule);

    // Single-quoted strings
    HighlightRule singleQuoteStringRule;
    singleQuoteStringRule.pattern = QRegularExpression("'[^']*'");
    singleQuoteStringRule.format = stringFormat;
    highlightRules.append(singleQuoteStringRule);

    // Default Other Text Format (Gray)
    defaultFormat.setForeground(QColor(179,177,173));
}

void Highlighter::highlightBlock(const QString &text)
{
    bool matched = false;

    for (const HighlightRule &rule : highlightRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            matched = true;
        }
    }

    // Apply default gray format to any text that doesn't match any rule.
    if (!matched) {
        setFormat(0, text.length(), defaultFormat);
    }
}
