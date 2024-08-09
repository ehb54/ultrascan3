#ifndef TEST_US_MEMORY_H
#define TEST_US_MEMORY_H

#include <QObject>
#include <QtTest/QtTest>

class TestUSMemory : public QObject {
Q_OBJECT

private slots:
    void testRssNow();
    void testRssMax();
    void testMemoryProfile();
};

#endif // TEST_US_MEMORY_H
