#include "us_customformatter.h"
#include <QtDataVisualization/QValue3DAxis>
//#include <QtQml/QQmlExtensionPlugin>
#include <QtCore/QDebug>

using namespace QtDataVisualization;

Q_DECLARE_METATYPE(QValue3DAxisFormatter *)

CustomFormatter::CustomFormatter(qreal i_minval, qreal i_maxval) :
    QValue3DAxisFormatter()
{
    qRegisterMetaType<QValue3DAxisFormatter *>();
    minVal = i_minval;
    maxVal = i_maxval;
}

CustomFormatter::~CustomFormatter()
{
}

//! [1]
QValue3DAxisFormatter *CustomFormatter::createNewInstance() const
{
    return new CustomFormatter();
}

void CustomFormatter::populateCopy(QValue3DAxisFormatter &copy) const
{
    QValue3DAxisFormatter::populateCopy(copy);

    CustomFormatter *customFormatter = static_cast<CustomFormatter *>(&copy);
    customFormatter->minVal = minVal;
    customFormatter->maxVal = maxVal;
}
//! [1]

//! [2]
void CustomFormatter::recalculate()
{  
    int segmentCount = axis()->segmentCount();
    int subGridCount = axis()->subSegmentCount() - 1;
    QString labelFormat =  axis()->labelFormat();

    gridPositions().resize(segmentCount + 1);
    subGridPositions().resize(segmentCount * subGridCount);

    labelPositions().resize(segmentCount + 1);
    labelStrings().clear();
    labelStrings().reserve(segmentCount + 1);

    qreal segmentStep = 1.0 / qreal(segmentCount);
    qreal subSegmentStep = 0;
    if (subGridCount > 0)
        subSegmentStep = segmentStep / qreal(subGridCount + 1);

    qreal labelValue;
    QVector<qreal> values(segmentCount + 1);

    qreal delta = (maxVal - minVal) / segmentCount;
    for (int i = 0; i < segmentCount; i++) {
        values[i] = minVal + i * delta;
    }
    values[segmentCount] = maxVal;

    for (int i = 0; i < segmentCount; i++) {
        qreal gridValue = segmentStep * qreal(i);
        gridPositions()[i] = float(gridValue);
        labelPositions()[i] = float(gridValue);
        labelValue = values.at(i);
        labelStrings() << stringForValue(labelValue, labelFormat);
        if (subGridPositions().size()) {
            for (int j = 0; j < subGridCount; j++)
                subGridPositions()[i * subGridCount + j] = gridValue + subSegmentStep * (j + 1);
        }
    }
    gridPositions()[segmentCount] = 1.0f;
    labelPositions()[segmentCount] = 1.0f;
    labelValue = values.at(segmentCount);
    labelStrings() << stringForValue(labelValue, labelFormat);

}

