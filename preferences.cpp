#include "preferences.h"
#include "ui_preferences.h"

#include <QtCore/QSettings>

#include <QtWidgets/QFileDialog>

class Preferences::Private : public Ui::Preferences
{
public:
    Private(::Preferences *parent);
    ~Private();

private:
    ::Preferences *q;

public:
    QSettings settings;
};

Preferences::Private::Private(::Preferences *parent)
    : q(parent)
{
    settings.beginGroup(q->metaObject()->className());
    setupUi(q);
    gstreamerSourceDirectory->setText(settings.value(QStringLiteral("gstreamerSourceDirectory")).toString());
    connect(gstreamerSourceDirectoryButton, &QToolButton::clicked, [this]() {
        const auto directory = QFileDialog::getExistingDirectory(q, tr("Select GStreamer source directory"), gstreamerSourceDirectory->text());
        if (directory.isEmpty())
            return;
        gstreamerSourceDirectory->setText(directory);
    });

    // QOperatingSystemVersion doesn't support Linux yet
    externalTextEditor->addItems({
#ifdef Q_OS_WIN
                QStringLiteral("notepad %f"),
                QStringLiteral("notepad++ %f -n%l"),
                QStringLiteral("sublime_text %f:%l"),
                QStringLiteral("code -g %f:%l"),
#elif defined(Q_OS_MAC)
                QStringLiteral("open -a TextEdit %f"),
                QStringLiteral("subl %f:%l"),
                QStringLiteral("code -g %f:%l"),
#elif defined(Q_OS_LINUX)
                QStringLiteral("gedit %f +%l"),
                QStringLiteral("kate %f -l %l"),
                QStringLiteral("qtcreator %f:%l"),
                QStringLiteral("code -g %f:%l"),
                QStringLiteral("gnome-terminal -- nano +%l %f"),
                QStringLiteral("gnome-terminal -- vim +%l %f"),
                QStringLiteral("konsole -e nano +%l %f"),
                QStringLiteral("konsole -e vim +%l %f"),
                QStringLiteral("xterm -e nano +%l %f"),
                QStringLiteral("xterm -e vim +%l %f"),
#endif
            });

    externalTextEditor->setCurrentText(settings.value(QStringLiteral("externalTextEditor")).toString());
    q->restoreGeometry(settings.value(QStringLiteral("geometry")).toByteArray());
}

Preferences::Private::~Private()
{
    settings.setValue(QStringLiteral("geometry"), q->saveGeometry());
}

Preferences::Preferences(QWidget *parent)
    : QDialog(parent)
    , d(new Private(this))
{}

Preferences::~Preferences() = default;

void Preferences::setCurrentField(const QString &currentField)
{
    QWidget *field = findChild<QWidget *>(currentField);
    if (!field)
        return;
    field->setFocus();
}

void Preferences::accept()
{
    d->settings.setValue(QStringLiteral("gstreamerSourceDirectory"), d->gstreamerSourceDirectory->text());
    d->settings.setValue(QStringLiteral("externalTextEditor"), d->externalTextEditor->currentText());
    QDialog::accept();
}
