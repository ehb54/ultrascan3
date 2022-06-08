#ifndef CUSTOMFORMATTER_H
#define CUSTOMFORMATTER_H

#include <QtDataVisualization/QValue3DAxisFormatter>
#include <QtCore>

using namespace QtDataVisualization;

//! [2]
class CustomFormatter : public QValue3DAxisFormatter
{  
    Q_OBJECT

    //! [1]
//    Q_PROPERTY(QDate originDate READ originDate WRITE setOriginDate NOTIFY originDateChanged)
    //! [1]
    //! [3]
//    Q_PROPERTY(QString selectionFormat READ selectionFormat WRITE setSelectionFormat NOTIFY selectionFormatChanged)
    //! [3]
public:
//    explicit CustomFormatter(QObject *parent = 0);
    explicit CustomFormatter(qreal i_minval = 0, qreal i_maxval = 1);
    virtual ~CustomFormatter();

    //! [0]
    virtual QValue3DAxisFormatter *createNewInstance() const;
    virtual void populateCopy(QValue3DAxisFormatter &copy) const;
    virtual void recalculate();
//    virtual QString stringForValue(qreal value, const QString &format) const;
    //! [0]

//    QDate originDate() const;
//    QString selectionFormat() const;

//public Q_SLOTS:
//    void setOriginDate(const QDate &date);
//    void setSelectionFormat(const QString &format);

//Q_SIGNALS:
//    void originDateChanged(const QDate &date);
//    void selectionFormatChanged(const QString &format);

private:
    Q_DISABLE_COPY(CustomFormatter)

//    QDateTime valueToDateTime(qreal value) const;

//    QDate m_originDate;
//    QString m_selectionFormat;
    qreal minVal;
    qreal maxVal;

};

#endif // CUSTOMFORMATTER_H
