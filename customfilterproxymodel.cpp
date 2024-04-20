#include "customfilterproxymodel.h"
#include "gstreamerlogmodel.h"
#include "timestamp.h"

#include <QtCore/QMetaProperty>
#include <QtGui/QGuiApplication>
#include <QtGui/QFont>

class CustomFilterProxyModel::Private
{
public:
    Private(CustomFilterProxyModel *parent);
    QModelIndex findNearestTimestamp(int minRow, int maxRow, const Timestamp &timestamp) const;

private:
    CustomFilterProxyModel *q;
public:
    QString filter;
    mutable int progress = -1;
};

CustomFilterProxyModel::Private::Private(CustomFilterProxyModel *parent)
    : q(parent)
{}

QModelIndex CustomFilterProxyModel::Private::findNearestTimestamp(int minRow, int maxRow, const Timestamp &timestamp) const
{
    if (minRow == maxRow) {
        return q->index(minRow, GStreamerLogModel::TimestampColumn);
    }
    if (minRow + 1 == maxRow) {
        const auto minIndex  =q->index(minRow, GStreamerLogModel::TimestampColumn);
        const auto maxIndex = q->index(maxRow, GStreamerLogModel::TimestampColumn);
        const auto minTimestamp = Timestamp::fromString(minIndex.data().toString());
        const auto maxTimestamp = Timestamp::fromString(maxIndex.data().toString());
        auto former = minTimestamp.secsTo(timestamp);
        auto latter = timestamp.secsTo(maxTimestamp);
        if (former == latter) {
            former = minTimestamp.msecsTo(timestamp);
            latter = timestamp.msecsTo(maxTimestamp);
        }
        if (former == latter) {
            former = minTimestamp.usecsTo(timestamp);
            latter = timestamp.usecsTo(maxTimestamp);
        }
        if (former == latter) {
            former = minTimestamp.nsecsTo(timestamp);
            latter = timestamp.nsecsTo(maxTimestamp);
        }
        return former < latter ? minIndex : maxIndex;
    }

    const int midRow = minRow + (maxRow - minRow) / 2;
    const auto midIndex = q->index(midRow, GStreamerLogModel::TimestampColumn);
    const auto midTimestamp = Timestamp::fromString(midIndex.data().toString());
    const auto comp = timestamp <=> midTimestamp;
    if (comp < 0)
        return findNearestTimestamp(minRow, midRow, timestamp);
    else if (comp > 0)
        return findNearestTimestamp(midRow, maxRow, timestamp);
    return midIndex;
}

CustomFilterProxyModel::CustomFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel{parent}
    , d{new Private(this)}
{
    connect(this, &CustomFilterProxyModel::filterChanged, this, &CustomFilterProxyModel::invalidate);
}

CustomFilterProxyModel::~CustomFilterProxyModel() = default;

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

int CustomFilterProxyModel::progress() const
{
    return d->progress;
}

void CustomFilterProxyModel::setProgress(int progress) const
{
    if (d->progress == progress) return;
    d->progress = progress;
    auto that = const_cast<CustomFilterProxyModel *>(this);
    emit that->progressChanged(progress);
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
    static int rowCount = 1;
    if (source_row == 0)
        rowCount = sourceModel()->rowCount() - 1;
    setProgress(source_row * 100 / rowCount);

    if (!d->filter.isEmpty()) {
        static const auto mo = &GStreamerLogLine::staticMetaObject;
        static const auto properties = mo->propertyCount();
        static QHash<QString, int> name2column;
        if (name2column.isEmpty()) {
            for (int i = 0; i < properties; ++i) {
                const auto property = mo->property(i);
                name2column.insert(property.name(), i);
            }
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
    bool wrap = flags & Qt::MatchWrap;
    bool backword = flags & Qt::MatchRecursive; // abuse recursive flag for backwards search
    bool timestampOnly = flags & Qt::MatchStartsWith; // abusing this flag
    if (timestampOnly) {
        const auto index = d->findNearestTimestamp(0, rowCount() - 1, value.value<Timestamp>());
        if (index.isValid())
            ret << index;
        return ret;
    }

    auto decrementIndex = [&] (const QModelIndex &index) -> QModelIndex {
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

    auto incrementIndex = [&] (const QModelIndex &index) -> QModelIndex {
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

    auto nextIndex = [&](const QModelIndex &index) -> QModelIndex {
        return backword ? decrementIndex(index) : incrementIndex(index);
    };

    auto next = [&](const QModelIndex &index) {
        return (ret.size() < hits || hits == -1) && index.isValid();
    };

    QModelIndex index = nextIndex(start);
    if (index == start)
        return ret;

    QString text = value.toString();
    while (next(index) && index != start) {
        const auto v = index.data(role);
        if (v.toString().contains(text))
            ret.append(index);
        index = nextIndex(index);
    }

    return ret;
}
