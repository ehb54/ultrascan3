#ifndef TEST_US_PROJECT_H
#define TEST_US_PROJECT_H

#include <QObject>
#include <QtTest/QtTest>
#include "us_project.h"  // Include the header file for the class you're testing
#include "us_settings.h"

class TestUSProject : public QObject
{
Q_OBJECT

private slots:
    void initTestCase();       // Runs before all tests
    void cleanupTestCase();    // Runs after all tests
    void init();               // Runs before each test function
    void cleanup();            // Runs after each test function

    // Unit test functions
    void testClear();
    void testSaveToDisk();
    void testReadFromDisk();
    void testDeleteFromDisk();
};

#endif // TEST_US_PROJECT_H
