#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QtWidgets/QDialog>

class Preferences : public QDialog
{
    Q_OBJECT

public:
    explicit Preferences(QWidget *parent = nullptr);
    ~Preferences() override;

public slots:
    void setCurrentField(const QString &currentField);
    void accept() override;

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // PREFERENCES_H
