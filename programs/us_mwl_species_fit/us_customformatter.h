#ifndef CUSTOMFORMATTER_H
#define CUSTOMFORMATTER_H

#include <QtDataVisualization/QValue3DAxisFormatter>
#include <QtCore>

using namespace QtDataVisualization;

//! [2]
class CustomFormatter : public QValue3DAxisFormatter
{  
    Q_OBJECT

public:
    explicit CustomFormatter(qreal i_minval = 0, qreal i_maxval = 1);
    virtual ~CustomFormatter();

    virtual QValue3DAxisFormatter *createNewInstance() const;
    virtual void populateCopy(QValue3DAxisFormatter &copy) const;
//    virtual void recalculate();
    virtual QString stringForValue(qreal value, const QString &format) const;

private:
    Q_DISABLE_COPY(CustomFormatter)

    qreal minVal;
    qreal maxVal;

};

#endif // CUSTOMFORMATTER_H
