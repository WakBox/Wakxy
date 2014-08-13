#ifndef COMPLETER_H
#define COMPLETER_H

#include <QPlainTextEdit>
#include <QPaintEvent>
#include <QTextBlock>
#include <QPainter>
#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QtDebug>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>

class QCompleter;
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

class LineNumberArea;

class TextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    TextEdit(QWidget *parent = 0);
    ~TextEdit();

    void setCompleter(QCompleter *c);
    QCompleter *completer() const;

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *e);

    void resizeEvent(QResizeEvent *event);

private slots:
    void insertCompletion(const QString &completion);

    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);

private:
    QString textUnderCursor() const;
    QCompleter *c;

    QWidget *lineNumberArea;
};


class LineNumberArea : public QWidget
{
public:
    LineNumberArea(TextEdit *editor) : QWidget(editor) {
        codeEditor = editor;
    }

    QSize sizeHint() const {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    TextEdit *codeEditor;
};

#endif // COMPLETER_H
