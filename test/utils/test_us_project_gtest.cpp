#include "test_us_project_gtest.h"

TEST_F(TestUsProjectGtest, SimplifiedTest) {
    int projectID = 123;

    NiceMock<MockUS_DB2> mockDB;
    Mock::AllowLeak(&mockDB);

      EXPECT_CALL(mockDB, next())
            .WillOnce(Return(true));  // Simplified to Return(true)

    int result = project.readFromDB(projectID, &mockDB);

    EXPECT_EQ(result, US_DB2::OK);
}

TEST_F(TestUsProjectGtest, TestReadFromDBSuccess) {
    int projectID = 123;

    NiceMock<MockUS_DB2> mockDB;
    Mock::AllowLeak(&mockDB);

    EXPECT_CALL(mockDB, query(_))
            .WillOnce([]() {
                return true;
            });

    EXPECT_CALL(mockDB, next())
            .WillOnce([]() {
                return true;
            });

    EXPECT_CALL(mockDB, value(1))
            .WillOnce(Return(QVariant("guid-value")));
    EXPECT_CALL(mockDB, value(2))
            .WillOnce(Return(QVariant("goals-value")));
    EXPECT_CALL(mockDB, value(3))
            .WillOnce(Return(QVariant("molecules-value")));
    EXPECT_CALL(mockDB, value(4))
            .WillOnce(Return(QVariant("purity-value")));
    EXPECT_CALL(mockDB, value(5))
            .WillOnce(Return(QVariant("expense-value")));
    EXPECT_CALL(mockDB, value(6))
            .WillOnce(Return(QVariant("bufferComponents-value")));
    EXPECT_CALL(mockDB, value(7))
            .WillOnce(Return(QVariant("saltInformation-value")));
    EXPECT_CALL(mockDB, value(8))
            .WillOnce(Return(QVariant("AUC_questions-value")));
    EXPECT_CALL(mockDB, value(9))
            .WillOnce(Return(QVariant("notes-value")));
    EXPECT_CALL(mockDB, value(10))
            .WillOnce(Return(QVariant("projectDesc-value")));
    EXPECT_CALL(mockDB, value(11))
            .WillOnce(Return(QVariant("status-value")));
    EXPECT_CALL(mockDB, value(13))
            .WillOnce(Return(QVariant("expDesign-value")));
    EXPECT_CALL(mockDB, value(14))
            .WillOnce(Return(QVariant(QDateTime::currentDateTime())));

    // Call the method under test
    int result = project.readFromDB(projectID, &mockDB);

    // Verify the result and state of the US_Project object
    EXPECT_EQ(result, US_DB2::OK);
    EXPECT_EQ(project.projectID, projectID);
    EXPECT_EQ(project.projectGUID, "guid-value");
    EXPECT_EQ(project.goals, "goals-value");
    EXPECT_EQ(project.molecules, "molecules-value");
    EXPECT_EQ(project.purity, "purity-value");
    EXPECT_EQ(project.expense, "expense-value");
    EXPECT_EQ(project.bufferComponents, "bufferComponents-value");
    EXPECT_EQ(project.saltInformation, "saltInformation-value");
    EXPECT_EQ(project.AUC_questions, "AUC_questions-value");
    EXPECT_EQ(project.notes, "notes-value");
    EXPECT_EQ(project.projectDesc, "projectDesc-value");
    EXPECT_EQ(project.status, "status-value");
    EXPECT_EQ(project.expDesign, "expDesign-value");
    EXPECT_TRUE(project.lastUpdated.isValid());
    EXPECT_EQ(project.saveStatus, US_Project::DB_ONLY);
}

TEST_F(TestUsProjectGtest, TestReadFromDBFailure) {
    int projectID = 123;

    NiceMock<MockUS_DB2> mockDB;
    Mock::AllowLeak(&mockDB);

    // Set up the expected sequence of calls and returns
    EXPECT_CALL(mockDB, query(_))
            .Times(1);

    EXPECT_CALL(mockDB, next())
            .WillOnce(Return(false));  // Simulate no project found

    // Call the method under test
    int result = project.readFromDB(projectID, &mockDB);

    // Verify the result and state of the US_Project object
    EXPECT_EQ(result, US_DB2::NO_PROJECT);
    EXPECT_EQ(project.projectID, 0);  // Should still be zero
    EXPECT_EQ(project.saveStatus, US_Project::NOT_SAVED);  // Should still be not saved
}