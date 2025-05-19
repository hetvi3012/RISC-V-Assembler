#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit Highlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    // Helper struct for our highlighting rules
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightRule> highlightRules;
    QTextCharFormat instructionFormat;
    QTextCharFormat registerFormat;
    QTextCharFormat directiveFormat;
    QTextCharFormat immediateFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat defaultFormat;
};

#endif // HIGHLIGHTER_H
