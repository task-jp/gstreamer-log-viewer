#ifndef CUSTOMFILTERPROXYMODEL_H
#define CUSTOMFILTERPROXYMODEL_H

#include <QtCore/QSortFilterProxyModel>

using QIntList = QList<int>;

class CustomFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString startTimestamp READ startTimestamp WRITE setStartTimestamp NOTIFY startTimestampChanged FINAL)
    Q_PROPERTY(QString endTimestamp READ endTimestamp WRITE setEndTimestamp NOTIFY endTimestampChanged FINAL)
    Q_PROPERTY(QIntList pids READ pids WRITE setPids NOTIFY pidsChanged FINAL)
    Q_PROPERTY(QStringList tids READ tids WRITE setTids NOTIFY tidsChanged FINAL)
    Q_PROPERTY(QStringList levels READ levels WRITE setLevels NOTIFY levelsChanged FINAL)
    Q_PROPERTY(QStringList categories READ categories WRITE setCategories NOTIFY categoriesChanged FINAL)
    Q_PROPERTY(QStringList sources READ sources WRITE setSources NOTIFY sourcesChanged FINAL)
    Q_PROPERTY(QIntList lines READ lines WRITE setLines NOTIFY linesChanged FINAL)
    Q_PROPERTY(QStringList functions READ functions WRITE setFunctions NOTIFY functionsChanged FINAL)
    Q_PROPERTY(QStringList objects READ objects WRITE setObjects NOTIFY objectsChanged FINAL)
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged FINAL)

public:
    explicit CustomFilterProxyModel(QObject *parent = nullptr);
    ~CustomFilterProxyModel() override;

    QString startTimestamp() const;
    QString endTimestamp() const;
    QIntList pids() const;
    QStringList tids() const;
    QStringList levels() const;
    QStringList categories() const;
    QStringList sources() const;
    QIntList lines() const;
    QStringList functions() const;
    QStringList objects() const;
    QString filter() const;

public slots:
    void setStartTimestamp(const QString &startTimestamp);
    void setEndTimestamp(const QString &endTimestamp);
    void setPids(const QIntList &pids);
    void setTids(const QStringList &tids);
    void setLevels(const QStringList &levels);
    void setCategories(const QStringList &categories);
    void setSources(const QStringList &sources);
    void setLines(const QIntList &lines);
    void setFunctions(const QStringList &functions);
    void setObjects(const QStringList &objects);
    void setFilter(const QString &filter);

signals:
    void startTimestampChanged(const QString &startTimestamp);
    void endTimestampChanged(const QString &endTimestamp);
    void pidsChanged(const QIntList &pids);
    void tidsChanged(const QStringList &tids);
    void levelsChanged(const QStringList &levels);
    void categoriesChanged(const QStringList &categories);
    void sourcesChanged(const QStringList &sources);
    void linesChanged(const QIntList &lines);
    void functionsChanged(const QStringList &functions);
    void objectsChanged(const QStringList &objects);
    void filterChanged(const QString &filter);

protected:
    QVariant data(const QModelIndex &index, int role) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    class Private;
    QScopedPointer<Private> d;

    // QAbstractItemModel interface
public:
    QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const override;
};

#endif // CUSTOMFILTERPROXYMODEL_H
