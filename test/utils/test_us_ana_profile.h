#ifndef TEST_US_ANA_PROFILE_H
#define TEST_US_ANA_PROFILE_H

#include <QObject>

class TestUSAnaProfile : public QObject
{
Q_OBJECT

private slots:
    void testConstructor();
    void testEqualityOperator();
    void testToXml();
    void testFromXml();
};

#endif // TEST_US_ANA_PROFILE_H
