#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent): QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(QColor("#000088"));
    QStringList keywordPatterns;

    keywordPatterns << "break" << "case" << "catch" << "continue" << "default" << "delete" << "do" << "else" << "finally" << "for" << "function"
                    << "if" << "in" << "instanceof" << "new" << "return" << "switch" << "this" << "throw" << "try" << "typeof" << "var" << "void" << "while" << "with";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp("\\b"+pattern+"\\b");
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // String
    stringFormat.setForeground(QColor("#008800"));
    rule.pattern = QRegExp("\".*\"");
    rule.format = stringFormat;
    highlightingRules.append(rule);

    // Number
    numberFormat.setForeground(QColor("#006666"));
    rule.pattern = QRegExp("0[xX][0-9a-fA-F]+");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // Text
    textFormat.setForeground(Qt::black);
    rule.pattern = QRegExp("{1,}");
    rule.format = textFormat;
    highlightingRules.append(rule);

    // Function
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(QColor("#660066"));
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // Comment
    commentFormat.setForeground(QColor("#808080"));
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = commentFormat;
    highlightingRules.append(rule);

    // Multi-line comment
    multiLineCommentFormat.setForeground(QColor("#808080"));
    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");

}
void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);

        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }

    setCurrentBlockState(0);
    int startIndex = 0;

    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

    while (startIndex >= 0)
    {
        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;

        if (endIndex == -1)
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex + commentEndExpression.matchedLength();
        }

        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
}
