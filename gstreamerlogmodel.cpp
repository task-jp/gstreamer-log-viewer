#include "gstreamerlogmodel.h"
#include "timestamp.h"

#include <QtCore/QFile>
#include <QtCore/QMetaProperty>
#include <QtCore/QRegularExpression>
#include <QtCore/QTextStream>
#include <QtGui/QColor>

class GStreamerLogModel::Private
{
public:
    QString fileName;
    QList<GStreamerLogLine> lines;
    static const QMetaObject *mo;
 };

const QMetaObject *GStreamerLogModel::Private::mo = &GStreamerLogLine::staticMetaObject;

GStreamerLogModel::GStreamerLogModel(const QString &fileName, QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private{fileName})
{
    reload();
}

GStreamerLogModel::~GStreamerLogModel() = default;

int GStreamerLogModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return d->lines.count();
}

int GStreamerLogModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return d->mo->propertyCount();
}

QVariant GStreamerLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant ret;
    if (orientation != Qt::Horizontal)
        return ret;
    const auto mp = d->mo->property(section);
    switch (role) {
    case Qt::DisplayRole:
        ret = mp.name();
        break;
    case Qt::TextAlignmentRole:
        ret = Qt::AlignLeft;
        break;
    default:
        break;
    }
    return ret;
}

QVariant GStreamerLogModel::data(const QModelIndex &index, int role) const
{
    static const auto foregroundColors = QHash<QString, QColor> {
        // { "WARN", QColor(Qt::white) },
        { "ERROR", QColor(Qt::yellow) },
    };

    static const auto backgroundColors = QHash<QString, QColor> {
        { "WARN", QColor(Qt::yellow) },
        { "ERROR", QColor(Qt::darkRed) },
    };

    QVariant ret;
    if (!index.isValid())
        return ret;
    const auto column = index.column();
    const auto mp = d->mo->property(column);
    const auto row = index.row();
    const auto line = d->lines.at(row);
    switch (role) {
    case Qt::DisplayRole:
        ret = mp.readOnGadget(&line);
        switch (mp.typeId()) {
        case QMetaType::QString:
        case QMetaType::Int:
            break;
        default:
            if (mp.typeId() == Timestamp::metaTypeId) {
                ret = ret.value<Timestamp>().toString();
            } else {
                qWarning() << mp.typeId() << "not supported";
            }
            break;
        }
        break;
    case Qt::TextAlignmentRole:
        if (mp.typeId() == QMetaType::Int || mp.typeId() == QMetaType::Double)
            ret = Qt::AlignRight;
        else
            ret = Qt::AlignLeft;
        break;
    case Qt::ForegroundRole:
        if (column == TimestampColumn && line.gap)
            ret = QColor(Qt::white);
        else if (foregroundColors.contains(line.level))
            ret = foregroundColors.value(line.level);
        break;
    case Qt::BackgroundRole:
        if (column == TimestampColumn && line.gap)
            ret = QColor(Qt::red);
        else if (backgroundColors.contains(line.level))
            ret = backgroundColors.value(line.level);
        break;
    case Qt::UserRole:
        ret = line.id;
        break;
    default:
        // ret = QAbstractTableModel::data(index, role);
        break;
    }
    return ret;
}

void GStreamerLogModel::reload()
{
    if (!d->lines.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, d->lines.count() - 1);
        d->lines.clear();
        endRemoveRows();
    }

    QFile file(d->fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QTextStream in(&file);
    int l = 0;
    static const QRegularExpression re("^([\\d\\.:]+)\\s+(\\d+)\\s+(0x[0-9a-f]+)\\s+([A-Z]+)\\s+([^\\s]*)\\s+([a-z0-9_\\-\\.]*):(\\d+):([^:]*):(\\s*[^\\s]*)\\s+(.+)$");
    while (!in.atEnd()) {
        l++;
        QString line = in.readLine();
        QRegularExpressionMatch match = re.match(line);
        if (match.hasMatch()) {
            GStreamerLogLine logLine;
            logLine.id = l;
            for (int i = 0; i < d->mo->propertyCount(); i++) {
                const auto mp = d->mo->property(i);
                const auto text = match.captured(i + 1);
                QVariant value = text;
                switch (mp.typeId()) {
                case QMetaType::QString:
                    break;
                case QMetaType::Int:
                    value = text.toInt();
                    break;
                default:
                    if (mp.typeId() == Timestamp::metaTypeId) {
                        value = QVariant::fromValue(Timestamp::fromString(text));
                    } else {
                        qWarning() << mp.typeId() << "not supported";
                    }
                    break;
                }
                mp.writeOnGadget(&logLine, value);
            }
            if (d->lines.isEmpty()) {
                d->lines.append(logLine);
            } else {
                auto timestamp = logLine.timestamp;
                for (auto i = d->lines.count() - 1; i >= 0; i--) {
                    if (timestamp > d->lines.at(i).timestamp) {
                        logLine.gap = d->lines.at(i).timestamp.secsTo(timestamp) > 1;
                        d->lines.insert(i + 1, logLine);
                        break;
                    }
                }
            }
        } else {
            qWarning() << line;
        }
    }
    beginInsertRows(QModelIndex(), 0, d->lines.count() - 1);
    endInsertRows();
}
