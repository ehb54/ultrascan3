/**
 * @file test_us_util.h
 * @brief Header file for the TestUSUtil class.
 *
 * This file contains the declaration of the TestUSUtil class, which includes
 * unit tests for various utility functions defined in the US_Util class.
 */

#ifndef TEST_US_UTIL_H
#define TEST_US_UTIL_H

#include <QtTest>
#include "us_util.h"

/**
 * @class TestUSUtil
 * @brief Unit test class for testing the US_Util functions.
 *
 * This class contains unit tests for various utility functions defined in the US_Util class.
 * The tests are implemented using the QtTest framework.
 */
class TestUSUtil : public QObject
{
Q_OBJECT

private slots:
    /**
     * @brief Initialization before any test function is executed.
     *
     * This function is called before the first test function is executed.
     * It is used to set up any resources needed for the tests.
     */
    void initTestCase();

    /**
     * @brief Cleanup after all test functions have been executed.
     *
     * This function is called after the last test function is executed.
     * It is used to release any resources allocated in initTestCase().
     */
    void cleanupTestCase();

    /**
     * @brief Test case for the getToken function.
     *
     * This function tests the getToken function of the US_Util class.
     * It verifies that the function correctly extracts tokens from a string.
     */
    void testGetToken();

    /**
     * @brief Test case for the new_guid function.
     *
     * This function tests the new_guid function of the US_Util class.
     * It verifies that the function generates a valid GUID string.
     */
    void testNewGuid();

    /**
     * @brief Test case for the md5sum_file function.
     *
     * This function tests the md5sum_file function of the US_Util class.
     * It verifies that the function correctly computes the MD5 hash of a file.
     */
    void testMd5sumFile();

    /**
     * @brief Test case for the toUTCDatetimeText function.
     *
     * This function tests the toUTCDatetimeText function of the US_Util class.
     * It verifies that the function correctly converts date-time strings to UTC format.
     */
    void testToUTCDatetimeText();
};

#endif // TEST_US_UTIL_H
