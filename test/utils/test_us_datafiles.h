/**
 * @file test_us_datafiles.h
 * @brief Header file for the TestUSDataFiles class
 *
 * This file contains the declaration of the TestUSDataFiles class, which includes
 * unit tests for various functions defined in the US_DataFiles class.
 */

#ifndef TEST_US_DATAFILES_H
#define TEST_US_DATAFILES_H

#include <gtest/gtest.h>
#include "qt_test_base.h"
// Forward declaration to avoid multiple inclusion issues
class US_DataFiles;

/**
 * @class TestUSDataFiles
 * @brief Unit test class for testing the US_DataFiles functions - Google Test version
 *
 * This class contains unit tests for various functions defined in the US_DataFiles class.
 * The tests are implemented using the Google Test framework.
 */
class TestUSDataFiles : public QtTestBase {
protected:
    void SetUp() override;
    void TearDown() override;

    // Suite-level setup and cleanup declarations
    static void SetUpTestSuite();
    static void TearDownTestSuite();

protected:
    QString path; /**< Path to the test data directory */

    /**
     * @brief Helper function to create an XML file with the specified GUID
     * @param filename The name of the file to create
     * @param tag The XML tag name
     * @param att The attribute name
     * @param guid The GUID value to write
     */
    void createXmlFile(const QString& filename, const QString& tag,
                       const QString& att, const QString& guid);
};

#endif // TEST_US_DATAFILES_H