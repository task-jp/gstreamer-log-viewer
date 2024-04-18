#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QtWidgets/QLineEdit>

class LineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit LineEdit(QWidget *parent = nullptr);
    ~LineEdit() override = default;

signals:
    void activated(Qt::KeyboardModifiers modifiers);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
};

#endif // LINEEDIT_H
