#ifndef TEST_US_PROJECT_H
#define TEST_US_PROJECT_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "us_project.h"
#include "mock/mock_us_db2.h"
#include <memory>

/**
 * @brief Test fixture for US_Project tests using Google Test framework
 *
 * This test fixture provides a clean US_Project instance for each test
 * and includes common setup/teardown functionality.
 */
class TestUsProject : public ::testing::Test {
protected:
    /**
     * @brief Set up function called before each test
     *
     * Initializes the project to a clean state for consistent testing.
     */
    void SetUp() override;

    /**
     * @brief Tear down function called after each test
     *
     * Performs cleanup operations after each test completes.
     */
    void TearDown() override;

    /**
     * @brief US_Project instance available to all tests
     *
     * This project instance is reset before each test in SetUp().
     */
    US_Project project;

    /**
     * @brief Helper method to create a mock database with common expectations
     * @return Unique pointer to a configured mock database
     */
    std::unique_ptr<US_DB2_MockNice> createMockDB();

    /**
     * @brief Helper method to set up a project with test data
     * @param proj Reference to the project to populate
     */
    void setupTestProject(US_Project& proj);

    /**
     * @brief Helper method to setup mock expectations for successful project read
     * @param mockDB Reference to the mock database
     * @param projectID The project ID to read
     */
    void setupMockForSuccessfulRead(US_DB2_MockNice& mockDB, int projectID);

    /**
     * @brief Helper method to setup mock expectations for new project save
     * @param mockDB Reference to the mock database
     * @param guid The project GUID
     * @param newProjectID The ID that should be returned for the new project
     */
    void setupMockForNewProjectSave(US_DB2_MockNice& mockDB,
                                    const QString& guid,
                                    int newProjectID);

    /**
     * @brief Helper method to setup mock expectations for existing project update
     * @param mockDB Reference to the mock database
     * @param guid The project GUID
     * @param existingProjectID The existing project ID
     */
    void setupMockForExistingProjectUpdate(US_DB2_MockNice& mockDB,
                                           const QString& guid,
                                           int existingProjectID);
};

// Implementation of helper methods (inline for header-only convenience)
inline std::unique_ptr<US_DB2_MockNice> TestUsProject::createMockDB() {
    return std::make_unique<US_DB2_MockNice>();
}

inline void TestUsProject::setupTestProject(US_Project& proj) {
    proj.projectGUID = "test-guid-12345";
    proj.goals = "Test project goals";
    proj.molecules = "Test molecules";
    proj.purity = "95%";
    proj.expense = "Medium";
    proj.status = "submitted";
}

inline void TestUsProject::setupMockForSuccessfulRead(US_DB2_MockNice& mockDB, int projectID) {
    QStringList expectedQuery;
    expectedQuery << "get_project_info" << QString::number(projectID);

    EXPECT_CALL(mockDB, queryStringList(expectedQuery))
            .Times(1);

    EXPECT_CALL(mockDB, next())
            .WillOnce(::testing::Return(true));

    // Set up mock return values for all database fields
    EXPECT_CALL(mockDB, value(1))   // projectGUID
            .WillOnce(::testing::Return(QVariant("test-guid-123")));
    EXPECT_CALL(mockDB, value(2))   // goals
            .WillOnce(::testing::Return(QVariant("Test research goals")));
    EXPECT_CALL(mockDB, value(3))   // molecules
            .WillOnce(::testing::Return(QVariant("Test molecule")));
    EXPECT_CALL(mockDB, value(4))   // purity
            .WillOnce(::testing::Return(QVariant("99%")));
    EXPECT_CALL(mockDB, value(5))   // expense
            .WillOnce(::testing::Return(QVariant("High")));
    EXPECT_CALL(mockDB, value(6))   // bufferComponents
            .WillOnce(::testing::Return(QVariant("Buffer A")));
    EXPECT_CALL(mockDB, value(7))   // saltInformation
            .WillOnce(::testing::Return(QVariant("NaCl 150mM")));
    EXPECT_CALL(mockDB, value(8))   // AUC_questions
            .WillOnce(::testing::Return(QVariant("Binding affinity")));
    EXPECT_CALL(mockDB, value(9))   // notes
            .WillOnce(::testing::Return(QVariant("Test notes")));
    EXPECT_CALL(mockDB, value(10))  // projectDesc
            .WillOnce(::testing::Return(QVariant("Test description")));
    EXPECT_CALL(mockDB, value(11))  // status
            .WillOnce(::testing::Return(QVariant("submitted")));
    EXPECT_CALL(mockDB, value(13))  // expDesign
            .WillOnce(::testing::Return(QVariant("Test design")));
    EXPECT_CALL(mockDB, value(14))  // lastUpdated
            .WillOnce(::testing::Return(QVariant(QDateTime::currentDateTime())));
}

inline void TestUsProject::setupMockForNewProjectSave(US_DB2_MockNice& mockDB,
                                                      const QString& guid,
                                                      int newProjectID) {
    QStringList guidQuery;
    guidQuery << "get_projectID_from_GUID" << guid;

    EXPECT_CALL(mockDB, queryStringList(guidQuery))
            .Times(1);

    EXPECT_CALL(mockDB, lastErrno())
            .WillOnce(::testing::Return(IUS_DB2::NOROWS));

    EXPECT_CALL(mockDB, statusQueryStringList(::testing::_))
            .WillOnce(::testing::Return(IUS_DB2::OK));

    EXPECT_CALL(mockDB, lastInsertID())
            .WillOnce(::testing::Return(newProjectID));
}

inline void TestUsProject::setupMockForExistingProjectUpdate(US_DB2_MockNice& mockDB,
                                                             const QString& guid,
                                                             int existingProjectID) {
    QStringList guidQuery;
    guidQuery << "get_projectID_from_GUID" << guid;

    EXPECT_CALL(mockDB, queryStringList(guidQuery))
            .Times(1);

    EXPECT_CALL(mockDB, lastErrno())
            .WillOnce(::testing::Return(IUS_DB2::OK));

    EXPECT_CALL(mockDB, next())
            .WillOnce(::testing::Return(true));

    EXPECT_CALL(mockDB, value(0))
            .WillOnce(::testing::Return(QVariant(existingProjectID)));

    EXPECT_CALL(mockDB, statusQueryStringList(::testing::_))
            .WillOnce(::testing::Return(IUS_DB2::OK));
}

#endif // TEST_US_PROJECT_H