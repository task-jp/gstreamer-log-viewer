#include "preferences.h"
#include "ui_preferences.h"

#include <QtCore/QSettings>

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
    toolButton->hide();
    gstreamerSourceDirectory->setText(settings.value(QStringLiteral("gstreamerSourceDirectory")).toString());
    sourceOpenMethod->setText(settings.value(QStringLiteral("sourceOpenMethod")).toString());
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


void Preferences::accept()
{
    d->settings.setValue(QStringLiteral("gstreamerSourceDirectory"), d->gstreamerSourceDirectory->text());
    d->settings.setValue(QStringLiteral("sourceOpenMethod"), d->sourceOpenMethod->text());
    QDialog::accept();
}
