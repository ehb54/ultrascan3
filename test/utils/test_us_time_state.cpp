// test_us_time_state.cpp - Unit tests for US_TimeState
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "qt_test_base.h"
#include "mock_us_db2.h"
#include "us_time_state.h"
#include <QTemporaryDir>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDataStream>

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;

// =============================================================================
// CORE FUNCTIONALITY TESTS
// =============================================================================

class US_TimeStateTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        timeState = std::make_unique<US_TimeState>();
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
    }

    void TearDown() override {
        timeState.reset();
        tempDir.reset();
        QtTestBase::TearDown();
    }

    QString getTempFilePath() {
        return tempDir->path() + "/test_file.tmst";
    }

    void createValidFiles(const QString& tmstPath) {
        QString xmlPath = QString(tmstPath).replace(".tmst", ".xml");

        // Create TMST file
        QFile tmstFile(tmstPath);
        ASSERT_TRUE(tmstFile.open(QIODevice::WriteOnly));
        QDataStream stream(&tmstFile);
        stream.writeRawData("USTS", 4);
        stream.writeRawData("2.1", 3);
        int record = 42;
        stream.writeRawData(reinterpret_cast<const char*>(&record), 4);
        tmstFile.close();

        // Create XML file
        QFile xmlFile(xmlPath);
        ASSERT_TRUE(xmlFile.open(QIODevice::WriteOnly | QIODevice::Text));
        QXmlStreamWriter xml(&xmlFile);
        xml.setAutoFormatting(true);
        xml.writeStartDocument();
        xml.writeDTD("<!DOCTYPE US_TimeState>");
        xml.writeStartElement("TimeState");
        xml.writeAttribute("version", "2.1");
        xml.writeAttribute("import_type", "XLA");
        xml.writeStartElement("file");
        xml.writeAttribute("time_count", "1");
        xml.writeAttribute("constant_incr", "1");
        xml.writeAttribute("time_increment", "1.0");
        xml.writeAttribute("first_time", "0.0");
        xml.writeAttribute("ss_reso", "100");
        xml.writeStartElement("value");
        xml.writeAttribute("key", "testkey");
        xml.writeAttribute("format", "I4");
        xml.writeEndElement();
        xml.writeEndElement();
        xml.writeEndElement();
        xml.writeEndDocument();
        xmlFile.close();
    }

    std::unique_ptr<US_TimeState> timeState;
    std::unique_ptr<QTemporaryDir> tempDir;
};

// Constructor
TEST_F(US_TimeStateTest, Constructor) {
    EXPECT_EQ(timeState->time_count(), 0);
    EXPECT_EQ(timeState->ss_resolution(), 100);
    EXPECT_TRUE(timeState->last_error_message().isEmpty());
}

// Write operations
TEST_F(US_TimeStateTest, OpenWriteData) {
    QString filePath = getTempFilePath();
    int result = timeState->open_write_data(filePath, 1.0, 0.0);
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(QFile::exists(filePath));
    timeState->close_write_data();
}

TEST_F(US_TimeStateTest, OpenWriteDataInvalidPath) {
    QString invalidPath = "/invalid/path/file.tmst";
    int result = timeState->open_write_data(invalidPath);
    EXPECT_EQ(result, 100);
    EXPECT_FALSE(timeState->last_error_message().isEmpty());
}

// Key management
TEST_F(US_TimeStateTest, SetKeyValid) {
    QString filePath = getTempFilePath();
    timeState->open_write_data(filePath);

    EXPECT_EQ(timeState->set_key("intkey", "I4"), 0);
    EXPECT_EQ(timeState->set_key("floatkey", "F4"), 0);
    EXPECT_EQ(timeState->set_key("charkey", "C8"), 0);

    timeState->close_write_data();
}

TEST_F(US_TimeStateTest, SetKeyInvalidFormat) {
    QString filePath = getTempFilePath();
    timeState->open_write_data(filePath);

    int result = timeState->set_key("badkey", "X4");
    EXPECT_EQ(result, 101);
    EXPECT_FALSE(timeState->last_error_message().isEmpty());

    timeState->close_write_data();
}

TEST_F(US_TimeStateTest, SetKeysLists) {
    QString filePath = getTempFilePath();
    timeState->open_write_data(filePath);

    QStringList keys = {"key1", "key2"};
    QStringList formats = {"I4", "F4"};

    int result = timeState->set_keys(keys, formats);
    EXPECT_EQ(result, 0);

    timeState->close_write_data();
}

TEST_F(US_TimeStateTest, SetKeysMismatchedSizes) {
    QString filePath = getTempFilePath();
    timeState->open_write_data(filePath);

    QStringList keys = {"key1", "key2"};
    QStringList formats = {"I4"};  // Size mismatch

    int result = timeState->set_keys(keys, formats);
    EXPECT_EQ(result, 102);

    timeState->close_write_data();
}

// Value setting
TEST_F(US_TimeStateTest, SetValueInteger) {
    QString filePath = getTempFilePath();
    timeState->open_write_data(filePath);
    timeState->set_key("intkey", "I4");

    int result = timeState->set_value("intkey", 42);
    EXPECT_EQ(result, 0);

    timeState->close_write_data();
}

TEST_F(US_TimeStateTest, SetValueDouble) {
    QString filePath = getTempFilePath();
    timeState->open_write_data(filePath);
    timeState->set_key("floatkey", "F4");

    int result = timeState->set_value("floatkey", 3.14);
    EXPECT_EQ(result, 0);

    timeState->close_write_data();
}

TEST_F(US_TimeStateTest, SetValueString) {
    QString filePath = getTempFilePath();
    timeState->open_write_data(filePath);
    timeState->set_key("strkey", "C8");

    int result = timeState->set_value("strkey", "test");
    EXPECT_EQ(result, 0);

    timeState->close_write_data();
}

TEST_F(US_TimeStateTest, SetValueInvalidKey) {
    QString filePath = getTempFilePath();
    timeState->open_write_data(filePath);
    timeState->set_key("validkey", "I4");

    int result = timeState->set_value("invalidkey", 42);
    EXPECT_EQ(result, 901);

    timeState->close_write_data();
}

// Record operations
TEST_F(US_TimeStateTest, FlushRecord) {
    QString filePath = getTempFilePath();
    timeState->open_write_data(filePath);
    timeState->set_key("intkey", "I4");
    timeState->set_value("intkey", 42);

    int result = timeState->flush_record();
    EXPECT_EQ(result, 0);

    timeState->close_write_data();
}

TEST_F(US_TimeStateTest, WriteDefinitions) {
    QString filePath = getTempFilePath();
    timeState->open_write_data(filePath);
    timeState->set_key("testkey", "I4");
    timeState->close_write_data();

    int result = timeState->write_defs();
    EXPECT_EQ(result, 0);

    QString xmlPath = QString(filePath).replace(".tmst", ".xml");
    EXPECT_TRUE(QFile::exists(xmlPath));
}

// Read operations
TEST_F(US_TimeStateTest, OpenReadData) {
    QString filePath = getTempFilePath();
    createValidFiles(filePath);

    int result = timeState->open_read_data(filePath);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(timeState->time_count(), 1);

    timeState->close_read_data();
}

TEST_F(US_TimeStateTest, OpenReadDataNonExistent) {
    QString nonExistentPath = "/non/existent/file.tmst";

    int result = timeState->open_read_data(nonExistentPath);
    EXPECT_EQ(result, 500);
    EXPECT_FALSE(timeState->last_error_message().isEmpty());
}

TEST_F(US_TimeStateTest, ReadRecord) {
    QString filePath = getTempFilePath();
    createValidFiles(filePath);

    timeState->open_read_data(filePath);
    int result = timeState->read_record();
    EXPECT_EQ(result, 0);

    timeState->close_read_data();
}

// Value retrieval
TEST_F(US_TimeStateTest, TimeIvalue) {
    QString filePath = getTempFilePath();
    createValidFiles(filePath);

    timeState->open_read_data(filePath);
    timeState->read_record();

    int status;
    int value = timeState->time_ivalue("testkey", &status);
    EXPECT_EQ(status, 0);

    timeState->close_read_data();
}

TEST_F(US_TimeStateTest, TimeIvalueInvalidKey) {
    QString filePath = getTempFilePath();
    createValidFiles(filePath);

    timeState->open_read_data(filePath);
    timeState->read_record();

    int status;
    timeState->time_ivalue("invalidkey", &status);
    EXPECT_EQ(status, 901);

    timeState->close_read_data();
}

// Utility methods
TEST_F(US_TimeStateTest, TimeRange) {
    bool constTi;
    double timeInc;
    double firstTime;

    int result = timeState->time_range(&constTi, &timeInc, &firstTime);
    EXPECT_EQ(result, 0);
}

TEST_F(US_TimeStateTest, Origin) {
    QString version;
    QString importType;

    bool hasType = timeState->origin(&version, &importType);
    EXPECT_TRUE(hasType);
    EXPECT_EQ(version, "2.1");
    EXPECT_EQ(importType, "XLA");
}

TEST_F(US_TimeStateTest, FieldKeys) {
    QStringList keys;
    QStringList formats;

    int count = timeState->field_keys(&keys, &formats);
    EXPECT_EQ(count, 0);
    EXPECT_TRUE(keys.isEmpty());
    EXPECT_TRUE(formats.isEmpty());
}

// Error handling
TEST_F(US_TimeStateTest, ErrorMessage) {
    QString msg = timeState->error_message(100);
    EXPECT_FALSE(msg.isEmpty());
    EXPECT_TRUE(msg.contains("Write-file open error"));
}

TEST_F(US_TimeStateTest, ErrorMessageUnknown) {
    QString msg = timeState->error_message(999999);
    EXPECT_EQ(msg, "UNKNOWN");
}

// =============================================================================
// DATABASE TESTS
// =============================================================================

class US_TimeStateDbTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        mockDb = std::make_unique<StrictMock<US_DB2_Mock>>();
    }

    void TearDown() override {
        mockDb.reset();
        QtTestBase::TearDown();
    }

    std::unique_ptr<StrictMock<US_DB2_Mock>> mockDb;
};

TEST_F(US_TimeStateDbTest, DbCreateNullDatabase) {
    int result = US_TimeState::dbCreate(nullptr, 1, "test.tmst");
    EXPECT_EQ(result, -1);
}

TEST_F(US_TimeStateDbTest, DbCreateInvalidParams) {
    IUS_DB2* db = mockDb.get();
    EXPECT_EQ(US_TimeState::dbCreate(db, 0, "test.tmst"), -1);  // Invalid expID
    EXPECT_EQ(US_TimeState::dbCreate(db, 1, ""), -1);           // Empty path
}

TEST_F(US_TimeStateDbTest, DbDeleteValid) {
    EXPECT_CALL(*mockDb, statusQuery(testing::An<const QStringList&>()))
            .WillOnce(Return(IUS_DB2::OK));

    IUS_DB2* db = mockDb.get();
    int result = US_TimeState::dbDelete(db, 123);
    EXPECT_EQ(result, IUS_DB2::OK);
}

TEST_F(US_TimeStateDbTest, DbExamineNullParams) {
    IUS_DB2* db = mockDb.get();
    int result = US_TimeState::dbExamine(db, nullptr, nullptr);
    EXPECT_EQ(result, IUS_DB2::NO_EXPERIMENT);
}

TEST_F(US_TimeStateDbTest, DbDownloadValid) {
    EXPECT_CALL(*mockDb, readBlobFromDB(
            testing::Eq(QString("test.tmst")),
            testing::Eq(QString("download_timestate")),
            testing::Eq(123)))
            .WillOnce(Return(IUS_DB2::OK));

    IUS_DB2* db = mockDb.get();
    int result = US_TimeState::dbDownload(db, 123, "test.tmst");
    EXPECT_EQ(result, IUS_DB2::OK);
}

TEST_F(US_TimeStateDbTest, DbUploadValid) {
    EXPECT_CALL(*mockDb, writeBlobToDB(
            testing::Eq(QString("test.tmst")),
            testing::Eq(QString("upload_timestate")),
            testing::Eq(123)))
            .WillOnce(Return(IUS_DB2::OK));

    IUS_DB2* db = mockDb.get();
    int result = US_TimeState::dbUpload(db, 123, "test.tmst");
    EXPECT_EQ(result, IUS_DB2::OK);
}

TEST_F(US_TimeStateDbTest, DbSyncToLFNullDatabase) {
    bool result = US_TimeState::dbSyncToLF(nullptr, "test.tmst", 456);
    EXPECT_FALSE(result);
}