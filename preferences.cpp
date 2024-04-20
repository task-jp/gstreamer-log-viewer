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
    connect(gstreamerSourceDirectoryButton, &QToolButton::clicked, [this]() {
        const auto directory = QFileDialog::getExistingDirectory(q, tr("Select GStreamer source directory"), gstreamerSourceDirectory->text());
        if (directory.isEmpty())
            return;
        gstreamerSourceDirectory->setText(directory);
    });
    gstreamerSourceDirectory->setText(settings.value(QStringLiteral("gstreamerSourceDirectory")).toString());
    externalTextEditor->setText(settings.value(QStringLiteral("externalTextEditor")).toString());
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
    d->settings.setValue(QStringLiteral("externalTextEditor"), d->externalTextEditor->text());
    QDialog::accept();
}
