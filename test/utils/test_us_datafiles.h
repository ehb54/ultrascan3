/**
 * @file test_us_datafiles.h
 * @brief Header file for the TestUSDataFiles class.
 *
 * This file contains the declaration of the TestUSDataFiles class, which includes
 * unit tests for various functions defined in the US_DataFiles class.
 */

#ifndef TEST_US_DATAFILES_H
#define TEST_US_DATAFILES_H

#include <QtTest>
#include "us_datafiles.h"

/**
 * @class TestUSDataFiles
 * @brief Unit test class for testing the US_DataFiles functions.
 *
 * This class contains unit tests for various functions defined in the US_DataFiles class.
 * The tests are implemented using the QtTest framework.
 */
class TestUSDataFiles : public QObject {
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
     * @brief Test case for no existing files.
     *
     * This function tests the behavior of US_DataFiles when there are no existing files.
     * It verifies that the function correctly creates a new file.
     */
    void test_no_existing_files();

    /**
     * @brief Test case for an existing file with a matching GUID.
     *
     * This function tests the behavior of US_DataFiles when there is an existing file
     * with a matching GUID. It verifies that the function correctly returns the existing file.
     */
    void test_existing_file_with_matching_guid();

    /**
     * @brief Test case for existing files without a matching GUID.
     *
     * This function tests the behavior of US_DataFiles when there are existing files
     * without a matching GUID. It verifies that the function correctly creates a new file.
     */
    void test_existing_files_without_matching_guid();

    /**
     * @brief Test case for a gap in numbering.
     *
     * This function tests the behavior of US_DataFiles when there is a gap in the numbering
     * of files. It verifies that the function correctly fills the gap with a new file.
     */
    void test_gap_in_numbering();

private:
    QString path; /**< Path to the test data directory */
};

#endif // TEST_US_DATAFILES_H
