#ifndef GSTREAMERLOGMODEL_H
#define GSTREAMERLOGMODEL_H

#include <QtCore/QAbstractTableModel>
#include "timestamp.h"

class GStreamerLogLine
{
    Q_GADGET
    Q_PROPERTY(Timestamp Timestamp MEMBER timestamp)
    Q_PROPERTY(int Process MEMBER pid)
    Q_PROPERTY(QString Thread MEMBER tid)
    Q_PROPERTY(QString Level MEMBER level)
    Q_PROPERTY(QString Category MEMBER category)
    Q_PROPERTY(QString Source MEMBER source)
    Q_PROPERTY(int Line MEMBER line)
    Q_PROPERTY(QString Function MEMBER function)
    Q_PROPERTY(QString Object MEMBER object)
    Q_PROPERTY(QString Message MEMBER message)

public:
    int id;
    bool gap = false;
    Timestamp timestamp;
    int pid;
    QString tid;
    QString level;
    QString category;
    QString source;
    int line;
    QString function;
    QString object;
    QString message;
};

class GStreamerLogModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column {
        TimestampColumn,
        PidColumn,
        TidColumn,
        LevelColumn,
        CategoryColumn,
        SourceColumn,
        LineColumn,
        FunctionColumn,
        ObjectColumn,
        MessageColumn,
    };
    explicit GStreamerLogModel(const QString &fileName, QObject *parent = nullptr);
    ~GStreamerLogModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void reload();

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // GSTREAMERLOGMODEL_H
