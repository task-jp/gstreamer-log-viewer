#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QtWidgets/QDialog>

class Preferences : public QDialog
{
    Q_OBJECT

public:
    explicit Preferences(QWidget *parent = nullptr);
    ~Preferences();

public slots:
    void accept() override;

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // PREFERENCES_H
