#ifndef CUSTOMFILTERPROXYMODEL_H
#define CUSTOMFILTERPROXYMODEL_H

#include <QtCore/QSortFilterProxyModel>

using QIntList = QList<int>;

class CustomFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged FINAL)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged FINAL)
public:
    explicit CustomFilterProxyModel(QObject *parent = nullptr);
    ~CustomFilterProxyModel() override;

    QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const override;

    QString filter() const;
    int progress() const;

public slots:
    void setFilter(const QString &filter);

private slots:
    void setProgress(int progress) const;

signals:
    void filterChanged(const QString &filter);
    void progressChanged(int progress);

protected:
    QVariant data(const QModelIndex &index, int role) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // CUSTOMFILTERPROXYMODEL_H
