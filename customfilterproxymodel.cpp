#include "customfilterproxymodel.h"
#include "gstreamerlogmodel.h"
#include "timestamp.h"

#include <QtCore/QMetaProperty>
#include <QtGui/QGuiApplication>
#include <QtGui/QFont>

class CustomFilterProxyModel::Private
{
public:
    QString startTimestamp;
    QString endTimestamp;
    QIntList pids;
    QStringList tids;
    QStringList levels;
    QStringList categories;
    QStringList sources;
    QIntList lines;
    QStringList functions;
    QStringList objects;
    QString filter;
};

CustomFilterProxyModel::CustomFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel{parent}
    , d{new Private}
{
    connect(this, &CustomFilterProxyModel::startTimestampChanged, this, &CustomFilterProxyModel::invalidate);
    connect(this, &CustomFilterProxyModel::endTimestampChanged, this, &CustomFilterProxyModel::invalidate);
    connect(this, &CustomFilterProxyModel::pidsChanged, this, &CustomFilterProxyModel::invalidate);
    connect(this, &CustomFilterProxyModel::tidsChanged, this, &CustomFilterProxyModel::invalidate);
    connect(this, &CustomFilterProxyModel::levelsChanged, this, &CustomFilterProxyModel::invalidate);
    connect(this, &CustomFilterProxyModel::categoriesChanged, this, &CustomFilterProxyModel::invalidate);
    connect(this, &CustomFilterProxyModel::sourcesChanged, this, &CustomFilterProxyModel::invalidate);
    connect(this, &CustomFilterProxyModel::linesChanged, this, &CustomFilterProxyModel::invalidate);
    connect(this, &CustomFilterProxyModel::functionsChanged, this, &CustomFilterProxyModel::invalidate);
    connect(this, &CustomFilterProxyModel::objectsChanged, this, &CustomFilterProxyModel::invalidate);
    connect(this, &CustomFilterProxyModel::filterChanged, this, &CustomFilterProxyModel::invalidate);
}

CustomFilterProxyModel::~CustomFilterProxyModel() = default;

QString CustomFilterProxyModel::startTimestamp() const
{
    return d->startTimestamp;
}

void CustomFilterProxyModel::setStartTimestamp(const QString &startTimestamp)
{
    if (d->startTimestamp == startTimestamp) return;
    d->startTimestamp = startTimestamp;
    emit startTimestampChanged(startTimestamp);
}

QString CustomFilterProxyModel::endTimestamp() const
{
    return d->endTimestamp;
}

void CustomFilterProxyModel::setEndTimestamp(const QString &endTimestamp)
{
    if (d->endTimestamp == endTimestamp) return;
    d->endTimestamp = endTimestamp;
    emit endTimestampChanged(endTimestamp);
}

QIntList CustomFilterProxyModel::pids() const
{
    return d->pids;
}

void CustomFilterProxyModel::setPids(const QIntList &pids)
{
    if (d->pids == pids) return;
    d->pids = pids;
    emit pidsChanged(pids);
}

QStringList CustomFilterProxyModel::tids() const
{
    return d->tids;
}

void CustomFilterProxyModel::setTids(const QStringList &tids)
{
    if (d->tids == tids) return;
    d->tids = tids;
    emit tidsChanged(tids);
}

QStringList CustomFilterProxyModel::levels() const
{
    return d->levels;
}

void CustomFilterProxyModel::setLevels(const QStringList &levels)
{
    if (d->levels == levels) return;
    d->levels = levels;
    emit levelsChanged(levels);
}

QStringList CustomFilterProxyModel::categories() const
{
    return d->categories;
}

void CustomFilterProxyModel::setCategories(const QStringList &categories)
{
    if (d->categories == categories) return;
    d->categories = categories;
    emit categoriesChanged(categories);
}

QStringList CustomFilterProxyModel::sources() const
{
    return d->sources;
}

void CustomFilterProxyModel::setSources(const QStringList &sources)
{
    if (d->sources == sources) return;
    d->sources = sources;
    emit sourcesChanged(sources);
}

QIntList CustomFilterProxyModel::lines() const
{
    return d->lines;
}

void CustomFilterProxyModel::setLines(const QIntList &lines)
{
    if (d->lines == lines) return;
    d->lines = lines;
    emit linesChanged(lines);
}

QStringList CustomFilterProxyModel::functions() const
{
    return d->functions;
}

void CustomFilterProxyModel::setFunctions(const QStringList &functions)
{
    if (d->functions == functions) return;
    d->functions = functions;
    emit functionsChanged(functions);
}

QStringList CustomFilterProxyModel::objects() const
{
    return d->objects;
}

void CustomFilterProxyModel::setObjects(const QStringList &objects)
{
    if (d->objects == objects) return;
    d->objects = objects;
    emit objectsChanged(objects);
}

QString CustomFilterProxyModel::filter() const
{
    return d->filter;
}

void CustomFilterProxyModel::setFilter(const QString &filter)
{
    if (d->filter == filter) return;
    d->filter = filter;
    emit filterChanged(filter);
}

QVariant CustomFilterProxyModel::data(const QModelIndex &index, int role) const
{
    QVariant ret = QSortFilterProxyModel::data(index, role);
    if (role == Qt::FontRole) {
        if (!d->filter.isEmpty()) {
            static const auto mo = &GStreamerLogLine::staticMetaObject;
            const auto property = mo->property(index.column());
            const auto filters = d->filter.split(QLatin1Char(' '), Qt::SkipEmptyParts);

            QVariant value = index.data(Qt::DisplayRole);
            bool matched = false;
            for (const auto &filter : filters) {
                auto keyword = filter;
                if (filter.contains(':')) {
                    const auto columnName = filter.section(':', 0, 0);
                    if (columnName == property.name()) {
                        keyword = filter.section(':', 1);
                    } else {
                        continue;
                    }
                } else if (index.column() != GStreamerLogModel::MessageColumn) {
                    continue;
                }

                switch (property.typeId()) {
                case QMetaType::QString: {
                    if (value.toString().contains(keyword, Qt::CaseInsensitive))
                        matched = true;
                    break; }
                case QMetaType::Int: {
                    if (value.toInt() == keyword.toInt())
                        matched = true;
                    break; }
                default:
                    break;
                }
                if (matched)
                    break;
            }

            if (matched) {
                QFont font = ret.value<QFont>();
                font.setBold(true);
                ret = QVariant::fromValue(font);
            }
        }
    }
    return ret;
}

bool CustomFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (source_row % 100 == 0)
        QGuiApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    if (!d->startTimestamp.isEmpty() || !d->endTimestamp.isEmpty()) {
        const auto text = sourceModel()->index(source_row, GStreamerLogModel::TimestampColumn, source_parent).data().toString();
        const auto timestamp = Timestamp::fromString(text);


        if (!d->startTimestamp.isEmpty() && timestamp < Timestamp::fromString(d->startTimestamp)) return false;
        if (!d->endTimestamp.isEmpty() && timestamp > Timestamp::fromString(d->endTimestamp)) return false;
    }

    if (!d->filter.isEmpty()) {
        static const auto mo = &GStreamerLogLine::staticMetaObject;
        const auto properties = mo->propertyCount();
        QHash<QString, int> name2column;
        for (int i = 0; i < properties; ++i) {
            const auto property = mo->property(i);
            name2column.insert(property.name(), i);
        }
        auto filters = d->filter.split(' ', Qt::SkipEmptyParts);
        for (auto i = filters.length() - 1; i >= 0; i--) {
            QString filter = filters.at(i);
            int column = GStreamerLogModel::MessageColumn;
            QString keyword = filter;
            if (filter.contains(':')) {
                const auto columnName = filter.section(':', 0, 0);
                if (name2column.contains(columnName)) {
                    column = name2column.value(columnName);
                    keyword = filter.section(':', 1);
                }
            }
            const auto value = sourceModel()->index(source_row, column, source_parent).data();
            const auto property = mo->property(column);
            switch (property.typeId()) {
            case QMetaType::QString:
                if (value.toString().contains(keyword, Qt::CaseInsensitive))
                    filters.removeAt(i);
                break;
            case QMetaType::Int:
                if (value.toInt() == keyword.toInt())
                    filters.removeAt(i);
                break;
            default:
                qWarning() << property.typeId() << "not supported";
                break;
            }
        }
        if (!filters.isEmpty()) return false;
    }
    return true;
}


QModelIndexList CustomFilterProxyModel::match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const
{
    QModelIndexList ret;
    QString text = value.toString();
    auto decrementIndex = [this] (const QModelIndex &index, bool wrap) -> QModelIndex {
        if (!index.isValid())
            return QModelIndex();
        if (index.column() > 0)
            return index.sibling(index.row(), index.column() - 1);
        if (index.row() > 0)
            return index.sibling(index.row() - 1, columnCount(index.parent()) - 1);
        if (wrap)
            return index.sibling(rowCount(index.parent()) - 1, columnCount(index.parent()) - 1);
        return QModelIndex();
    };

    auto incrementIndex = [this] (const QModelIndex &index, bool wrap) -> QModelIndex {
        if (!index.isValid())
            return QModelIndex();
        if (index.column() + 1 < columnCount(index.parent()))
            return index.sibling(index.row(), index.column() + 1);
        if (index.row() + 1 < rowCount(index.parent()))
            return index.sibling(index.row() + 1, 0);
        if (wrap)
            return index.sibling(0, 0);
        return QModelIndex();
    };

    auto nextIndex = [&](const QModelIndex &index) {
        bool wrap = flags & Qt::MatchWrap;
        bool backword = flags & Qt::MatchRecursive; // abuse recursive flag for backwards search
        return backword ? decrementIndex(index, wrap) : incrementIndex(index, wrap);
    };

    QModelIndex index = nextIndex(start);
    if (index == start)
        return ret;

    while ((ret.size() < hits || hits == -1) && index.isValid() && index != start) {
        const auto v = index.data(role);
        if (v.toString().contains(text))
            ret.append(index);
        index = nextIndex(index);
    }
    return ret;
}
