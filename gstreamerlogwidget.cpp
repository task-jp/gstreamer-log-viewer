#include "gstreamerlogwidget.h"
#include "ui_gstreamerlogwidget.h"
#include "gstreamerlogmodel.h"
#include "customfilterproxymodel.h"

#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtGui/QShortcut>

namespace {

QString searchFile(const QDir &directory, const QString &fileName)
{
    if (directory.exists(fileName))
        return directory.absoluteFilePath(fileName);

    QStringList directoryNames = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // 各エントリを走査
    for (const QString & directoryName : directoryNames) {
        QDir dir(directory.absoluteFilePath(directoryName));
        auto result = searchFile(dir, fileName);
        if (!result.isEmpty())
            return result;
    }
    return QString();
}
}

class GStreamerLogWidget::Private : public Ui::GStreamerLogWidget
{
public:
    Private(const QString &fileName, ::GStreamerLogWidget *parent);
    ~Private();

private:
    void open(const QString &fileName, int line) const;

private:
    ::GStreamerLogWidget *q;
    QString fileName;
    struct SearchResults {
        QString text;
        QModelIndex index;
    } searchResults;
public:
    GStreamerLogModel model;
    CustomFilterProxyModel proxyModel;
    bool busy = false;
    QSettings settings;
};

GStreamerLogWidget::Private::Private(const QString &fileName, ::GStreamerLogWidget *parent)
    : q(parent)
    , fileName(fileName)
    , model(fileName)
{
    settings.beginGroup(q->metaObject()->className());
    setupUi(q);

    tableView->setModel(&proxyModel);
    connect(tableView, &GStreamerLogView::jumpToLog, [this](int line) {
        open(this->fileName, line);
    });

    connect(tableView, &GStreamerLogView::jumpToSource, [this](const QString &source, int line) {
        QSettings settings;
        settings.beginGroup("Preferences");
        if (!settings.contains(QStringLiteral("gstreamerSourceDirectory"))) {
            emit q->openPreferences(QStringLiteral("gstreamerSourceDirectory"));
        }
        if (settings.contains(QStringLiteral("gstreamerSourceDirectory"))) {
            const auto directory = settings.value(QStringLiteral("gstreamerSourceDirectory")).toString();
            const auto path = searchFile(QDir(directory), source);
            if (path.isEmpty()) {
                emit q->errorOccurred(tr("Source file \"%1\" not found under \"%2\"").arg(source).arg(directory));
            } else {
                open(path, line);
            }
        }
    });

    connect(tableView, &GStreamerLogView::activated, [this](const QString &text) {
        auto filterText = filter->text();
        if (filterText.isEmpty()) {
            filterText = text;
        } else if (!filterText.contains(text.trimmed())) {
            if (!filterText.endsWith(QLatin1Char(' '))) {
                filterText += QLatin1Char(' ');
            }
            filterText += text;
        } else {
            return;
        }
        filter->setText(filterText);
        filter->setFocus();
        QTimer::singleShot(100, filter, &QLineEdit::returnPressed);
    });
    timestampView->setBuddy(tableView);
    proxyModel.setSourceModel(&model);
    connect(&proxyModel, &CustomFilterProxyModel::layoutAboutToBeChanged, [this]() {
        q->setBusy(true);
    });
    connect(&proxyModel, &CustomFilterProxyModel::layoutChanged, [this]() {
        q->setBusy(false);
        q->filteredCountChanged(proxyModel.rowCount());
    });
    splitter->restoreState(settings.value(QStringLiteral("splitterState")).toByteArray());

    auto shortcut = new QShortcut(QKeySequence(tr("Ctrl+L", "Filter")), q);
    connect(shortcut, &QShortcut::activated, [this]() {
        filter->setFocus();
    });
    connect(filter, &QLineEdit::returnPressed, [this]() {
        proxyModel.setFilter(filter->text());
    });

    shortcut = new QShortcut(QKeySequence(tr("Ctrl+F", "Find")), q);
    connect(shortcut, &QShortcut::activated, [this]() {
        find->setFocus();
    });
    connect(find, &LineEdit::activated, [this](Qt::KeyboardModifiers modifiers) {
        const auto text = find->text();
        searchResults.text = text;
        auto start = searchResults.index;
        if (!start.isValid() || start != tableView->currentIndex())
            start = tableView->currentIndex();
        if (!start.isValid())
            start = proxyModel.index(0, 0, QModelIndex());
        q->setBusy(true);
        Qt::MatchFlags flags = Qt::MatchContains | Qt::MatchWrap;
        if (modifiers & Qt::ShiftModifier)
            flags |= Qt::MatchRecursive; // abuse recursive flag for backwards search
        const auto indices = proxyModel.match(start, Qt::DisplayRole, text, 1, flags);
        q->setBusy(false);
        searchResults.index = indices.isEmpty() ? QModelIndex() : indices.first();

        if (!searchResults.index.isValid())
            return;

        tableView->setCurrentIndex(searchResults.index);
        tableView->selectionModel()->select(searchResults.index, QItemSelectionModel::ClearAndSelect);
        tableView->scrollTo(searchResults.index);
    });
}

GStreamerLogWidget::Private::~Private()
{
    settings.setValue(QStringLiteral("splitterState"), splitter->saveState());
}

void GStreamerLogWidget::Private::open(const QString &fileName, int line) const
{
    QSettings settings;
    settings.beginGroup("Preferences");
    if (settings.contains(QStringLiteral("externalTextEditor"))) {
        const auto method = settings.value(QStringLiteral("externalTextEditor")).toString();
        if (!method.isEmpty()) {
            QString commandline = method;
            commandline.replace(QStringLiteral("%f"), fileName);
            commandline.replace(QStringLiteral("%l"), QString::number(line));
            auto list = QProcess::splitCommand(commandline);
            QProcess::startDetached(list.takeFirst(), list);
        } else {
#ifdef Q_OS_WIN
            QProcess::startDetached(QStringLiteral("start"), {fileName});
#elif defined(Q_OS_MAC)
            QProcess::startDetached(QStringLiteral("open"), {fileName});
#elif defined(Q_OS_LINUX)
            QProcess::startDetached(QStringLiteral("xdg-open"), {fileName});
#endif
        }
    }
}

GStreamerLogWidget::GStreamerLogWidget(const QString &fileName, QWidget *parent)
    : QWidget(parent)
    , d(new Private(fileName, this))
{}

GStreamerLogWidget::~GStreamerLogWidget() = default;

bool GStreamerLogWidget::isBusy() const
{
    return d->busy;
}

void GStreamerLogWidget::setBusy(bool busy)
{
    if (d->busy == busy) return;
    d->busy = busy;
    emit busyChanged(busy);
}

int GStreamerLogWidget::count() const
{
    return d->model.rowCount();
}

int GStreamerLogWidget::filteredCount() const
{
    return d->proxyModel.rowCount();
}

void GStreamerLogWidget::reload()
{
    d->model.reload();
}
