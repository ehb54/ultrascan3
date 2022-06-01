#include "us_customformatter.h"
#include <QtDataVisualization/QValue3DAxis>
//#include <QtQml/QQmlExtensionPlugin>
#include <QtCore/QDebug>

using namespace QtDataVisualization;

Q_DECLARE_METATYPE(QValue3DAxisFormatter *)

CustomFormatter::CustomFormatter(QObject *parent, qreal i_offSet, qreal i_scale) :
    QValue3DAxisFormatter(parent)
{
    qRegisterMetaType<QValue3DAxisFormatter *>();
    offset = i_offSet;
    scale = i_scale;
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
    customFormatter->offset = offset;
    customFormatter->scale = scale;
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
    for (int i = 0; i < segmentCount; i++) {
        qreal gridValue = segmentStep * qreal(i);
        gridPositions()[i] = float(gridValue);
        labelPositions()[i] = float(gridValue);
        labelValue = gridValue * scale + offset;
        labelStrings() << stringForValue(labelValue, labelFormat);
        if (subGridPositions().size()) {
            for (int j = 0; j < subGridCount; j++)
                subGridPositions()[i * subGridCount + j] = gridValue + subSegmentStep * (j + 1);
        }
    }
    gridPositions()[segmentCount] = 1.0f;
    labelPositions()[segmentCount] = 1.0f;
    labelValue = scale + offset;
    labelStrings() << stringForValue(labelValue, labelFormat);

}

