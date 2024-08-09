#ifndef TEST_US_TIME_STATE_H
#define TEST_US_TIME_STATE_H

#include <QObject>
#include <QtTest/QtTest>

class TestUSTimeState : public QObject {
Q_OBJECT

private slots:
    void testOpenWriteData();
    void testSetKey();
    void testSetValue();
    void testFlushRecord();
    void testCloseWriteData();
    void testOpenReadData();
    void testReadRecord();
    void testTimeValues();
    void cleanupTestCase();
};

#endif // TEST_US_TIME_STATE_H
