#ifndef TIMESTAMPVIEW_H
#define TIMESTAMPVIEW_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QTableView>

class TimestampView : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QTableView *buddy READ buddy WRITE setBuddy NOTIFY buddyChanged FINAL)
public:
    explicit TimestampView(QWidget *parent = nullptr);
    ~TimestampView() override;

    QTableView *buddy() const;

public slots:
    void setBuddy(QTableView *buddy);

signals:
    void buddyChanged(QTableView *buddy);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // TIMESTAMPVIEW_H
