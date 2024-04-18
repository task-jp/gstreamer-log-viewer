#include "lineedit.h"

#include <QtGui/QKeyEvent>

LineEdit::LineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    connect(this, &QLineEdit::returnPressed, this, [this]() {
        emit activated(modifiers);
    });
}

void LineEdit::keyPressEvent(QKeyEvent *event)
{
    modifiers = event->modifiers();
    QLineEdit::keyPressEvent(event);
}


void LineEdit::keyReleaseEvent(QKeyEvent *event)
{
    QLineEdit::keyReleaseEvent(event);
    modifiers = Qt::NoModifier;
}
