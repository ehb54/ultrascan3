// test_us_util.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "qt_test_base.h"
#include "us_util.h"
#include <QTemporaryFile>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDateTime>
#include <QUuid>

using ::testing::_;
using ::testing::Return;
using ::testing::HasSubstr;
using ::testing::Not;
using ::qt_matchers::QStringContains;
using ::qt_matchers::QStringEq;
using ::qt_matchers::QStringIsEmpty;

class US_UtilTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        // Additional setup if needed
    }

    void TearDown() override {
        // Clean up any temporary files
        for (QTemporaryFile* file : tempFiles) {
            if (file) {
                file->deleteLater();
            }
        }
        tempFiles.clear();
        cleanupTempDirs();
        QtTestBase::TearDown();
    }

    // Helper method to create temporary files with content
    QString createTempFileWithContent(const QString& content) {
        QTemporaryFile* tempFile = new QTemporaryFile();
        tempFiles.append(tempFile);

        if (!tempFile->open()) {
            return QString();
        }

        tempFile->write(content.toUtf8());
        tempFile->flush();

        QString fileName = tempFile->fileName();
        tempFile->close();

        return fileName;
    }

    // Helper method to create temporary directory
    QString createTempDir() {
        QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        QString dirName = QString("us_util_test_%1").arg(QCoreApplication::applicationPid());
        QString fullPath = tempPath + "/" + dirName;

        QDir().mkpath(fullPath);
        tempDirs.append(fullPath);

        return fullPath;
    }

    // Helper method to clean up temp directories
    void cleanupTempDirs() {
        for (const QString& dir : tempDirs) {
            QDir(dir).removeRecursively();
        }
        tempDirs.clear();
    }

private:
    QList<QTemporaryFile*> tempFiles;
    QStringList tempDirs;
};

// Test getToken method
TEST_F(US_UtilTest, GetTokenBasicFunctionality) {
    QString testString = "token1,token2,token3";
    QString separator = ",";

    QString token1 = US_Util::getToken(testString, separator);
    EXPECT_THAT(token1, QStringEq("token1"));

    QString token2 = US_Util::getToken(testString, separator);
    EXPECT_THAT(token2, QStringEq("token2"));

    QString token3 = US_Util::getToken(testString, separator);
    EXPECT_THAT(token3, QStringEq("token3"));

    // String should be empty after all tokens extracted
    EXPECT_TRUE(testString.isEmpty());
}

TEST_F(US_UtilTest, GetTokenWithSpaces) {
    QString testString = "  token1  ,  token2  ,  token3  ";
    QString separator = ",";

    QString token1 = US_Util::getToken(testString, separator);
    EXPECT_THAT(token1, QStringEq("  token1  "));

    QString token2 = US_Util::getToken(testString, separator);
    EXPECT_THAT(token2, QStringEq("  token2  "));
}

TEST_F(US_UtilTest, GetTokenNoSeparator) {
    QString testString = "single_token";
    QString separator = ",";

    QString token = US_Util::getToken(testString, separator);
    EXPECT_THAT(token, QStringEq("single_token"));
    EXPECT_TRUE(testString.isEmpty());
}

TEST_F(US_UtilTest, GetTokenEmptyString) {
    QString testString = "";
    QString separator = ",";

    QString token = US_Util::getToken(testString, separator);
    EXPECT_TRUE(token.isEmpty());
    EXPECT_TRUE(testString.isEmpty());
}

TEST_F(US_UtilTest, GetTokenLeadingDelimiters) {
    QString testString = ",,token1,token2";
    QString separator = ",";

    QString token1 = US_Util::getToken(testString, separator);
    EXPECT_THAT(token1, QStringEq("token1"));

    QString token2 = US_Util::getToken(testString, separator);
    EXPECT_THAT(token2, QStringEq("token2"));
}

// Test new_guid method
TEST_F(US_UtilTest, NewGuidGeneratesValidFormat) {
    QString guid = US_Util::new_guid();

    // Should be 36 characters long
    EXPECT_EQ(guid.length(), 36);

    // Should have hyphens in correct positions (8-4-4-4-12 format)
    EXPECT_EQ(guid.at(8), '-');
    EXPECT_EQ(guid.at(13), '-');
    EXPECT_EQ(guid.at(18), '-');
    EXPECT_EQ(guid.at(23), '-');
}

TEST_F(US_UtilTest, NewGuidGeneratesUniqueValues) {
    QString guid1 = US_Util::new_guid();
    QString guid2 = US_Util::new_guid();
    QString guid3 = US_Util::new_guid();

    // All should be different
    EXPECT_NE(guid1, guid2);
    EXPECT_NE(guid2, guid3);
    EXPECT_NE(guid1, guid3);
}

// Test md5sum_file method
TEST_F(US_UtilTest, Md5sumFileValidFile) {
    QString content = "Test file content for MD5 calculation";
    QString fileName = createTempFileWithContent(content);
    ASSERT_FALSE(fileName.isEmpty());

    QString result = US_Util::md5sum_file(fileName);

    // Should contain hash and size separated by space
    QStringList parts = result.split(" ");
    EXPECT_EQ(parts.size(), 2);

    // Hash should be 32 characters (MD5 hex)
    EXPECT_EQ(parts[0].length(), 32);

    // Size should match content size
    EXPECT_EQ(parts[1].toInt(), content.toUtf8().size());
}

TEST_F(US_UtilTest, Md5sumFileNonExistentFile) {
    QString nonExistentFile = "/this/file/does/not/exist.txt";

    QString result = US_Util::md5sum_file(nonExistentFile);

    // Should return "0 0" for non-existent file
    EXPECT_THAT(result, QStringEq("0 0"));
}

TEST_F(US_UtilTest, Md5sumFileEmptyFile) {
    QString fileName = createTempFileWithContent("");
    ASSERT_FALSE(fileName.isEmpty());

    QString result = US_Util::md5sum_file(fileName);

    QStringList parts = result.split(" ");
    EXPECT_EQ(parts.size(), 2);

    // Hash should still be 32 characters
    EXPECT_EQ(parts[0].length(), 32);

    // Size should be 0
    EXPECT_EQ(parts[1].toInt(), 0);
}

TEST_F(US_UtilTest, Md5sumFileConsistency) {
    QString content = "Consistent content for hash testing";
    QString fileName = createTempFileWithContent(content);
    ASSERT_FALSE(fileName.isEmpty());

    QString result1 = US_Util::md5sum_file(fileName);
    QString result2 = US_Util::md5sum_file(fileName);

    // Same file should produce same result
    EXPECT_THAT(result1, QStringEq(result2));
}

// Test toUTCDatetimeText method
TEST_F(US_UtilTest, ToUTCDatetimeTextKnownUTC) {
    QString input = "2023-12-25T10:30:45";
    bool knownUTC = true;

    QString result = US_Util::toUTCDatetimeText(input, knownUTC);

    // Should replace T with space and add UTC
    EXPECT_THAT(result, QStringEq("2023-12-25 10:30:45 UTC"));
}

TEST_F(US_UtilTest, ToUTCDatetimeTextAlreadyUTC) {
    QString input = "2023-12-25 10:30:45 UTC";
    bool knownUTC = true;

    QString result = US_Util::toUTCDatetimeText(input, knownUTC);

    // Should remain unchanged
    EXPECT_THAT(result, QStringEq("2023-12-25 10:30:45 UTC"));
}

TEST_F(US_UtilTest, ToUTCDatetimeTextUnknownFormat) {
    QString input = "2023-12-25T10:30:45";
    bool knownUTC = false;

    QString result = US_Util::toUTCDatetimeText(input, knownUTC);

    // Should contain UTC at the end
    EXPECT_THAT(result, QStringContains(" UTC"));
    // Should not contain T
    EXPECT_THAT(result, Not(QStringContains("T")));
}

// Test toISODatetimeText method
TEST_F(US_UtilTest, ToISODatetimeTextFromUTC) {
    QString input = "2023-12-25 10:30:45 UTC";

    QString result = US_Util::toISODatetimeText(input);

    // Should convert to ISO format with T
    EXPECT_THAT(result, QStringEq("2023-12-25T10:30:45"));
}

TEST_F(US_UtilTest, ToISODatetimeTextAlreadyISO) {
    QString input = "2023-12-25T10:30:45";

    QString result = US_Util::toISODatetimeText(input);

    // Should remain unchanged
    EXPECT_THAT(result, QStringEq("2023-12-25T10:30:45"));
}

// Test uuid_parse and uuid_unparse methods
TEST_F(US_UtilTest, UuidParseUnparseRoundTrip) {
    QString originalUuid = "550e8400-e29b-41d4-a716-446655440000";
    unsigned char binary[16];

    // Parse to binary
    US_Util::uuid_parse(originalUuid, binary);

    // Unparse back to string
    QString resultUuid = US_Util::uuid_unparse(binary);

    // Should match original
    EXPECT_THAT(resultUuid, QStringEq(originalUuid));
}

TEST_F(US_UtilTest, UuidParseUnparseWithHyphens) {
    QString uuidWithHyphens = "12345678-90ab-cdef-1234-567890abcdef";
    unsigned char binary[16];

    US_Util::uuid_parse(uuidWithHyphens, binary);
    QString result = US_Util::uuid_unparse(binary);

    EXPECT_THAT(result, QStringEq(uuidWithHyphens));
}

// Test compressed_triple and expanded_triple methods
TEST_F(US_UtilTest, CompressedTripleBasic) {
    QString expanded = "1 / A / 290";

    QString compressed = US_Util::compressed_triple(expanded);

    EXPECT_THAT(compressed, QStringEq("1A290"));
}

TEST_F(US_UtilTest, CompressedTripleNoSpaces) {
    QString expanded = "4/B/350";

    QString compressed = US_Util::compressed_triple(expanded);

    EXPECT_THAT(compressed, QStringEq("4B350"));
}

TEST_F(US_UtilTest, ExpandedTripleWithSpaces) {
    QString compressed = "2C180";
    bool spaces = true;

    QString expanded = US_Util::expanded_triple(compressed, spaces);

    EXPECT_THAT(expanded, QStringEq("2 / C / 180"));
}

TEST_F(US_UtilTest, ExpandedTripleWithoutSpaces) {
    QString compressed = "3D220";
    bool spaces = false;

    QString expanded = US_Util::expanded_triple(compressed, spaces);

    EXPECT_THAT(expanded, QStringEq("3/D/220"));
}

TEST_F(US_UtilTest, CompressedExpandedRoundTrip) {
    QString original = "12 / Z / 450";

    QString compressed = US_Util::compressed_triple(original);
    QString expanded = US_Util::expanded_triple(compressed, true);

    EXPECT_THAT(expanded, QStringEq("12 / Z / 450"));
}

// Test ithTime method
TEST_F(US_UtilTest, IthTimeFunction) {
    // Test with small increment to increase probability
    int increment = 2;
    bool hitOnce = false;

    // Try multiple times - should hit true at least once with increment=2
    for (int i = 0; i < 100; ++i) {
        if (US_Util::ithTime(increment)) {
            hitOnce = true;
            break;
        }
    }

    // With increment=2, we should hit true sometime in 100 tries
    EXPECT_TRUE(hitOnce);
}

// Test bool_flag method
TEST_F(US_UtilTest, BoolFlagTrueValues) {
    EXPECT_TRUE(US_Util::bool_flag("1"));
    EXPECT_TRUE(US_Util::bool_flag("T"));
}

TEST_F(US_UtilTest, BoolFlagFalseValues) {
    EXPECT_FALSE(US_Util::bool_flag("0"));
    EXPECT_FALSE(US_Util::bool_flag("F"));
    EXPECT_FALSE(US_Util::bool_flag(""));
    EXPECT_FALSE(US_Util::bool_flag("false"));
    EXPECT_FALSE(US_Util::bool_flag("anything_else"));
}

// Test bool_string method
TEST_F(US_UtilTest, BoolStringFunction) {
    EXPECT_THAT(US_Util::bool_string(true), QStringEq("1"));
    EXPECT_THAT(US_Util::bool_string(false), QStringEq("0"));
}

// Test listlistBuild and listlistParse methods
TEST_F(US_UtilTest, ListlistBuildParseRoundTrip) {
    QList<QStringList> originalList;
    originalList << QStringList({"a", "b", "c"});
    originalList << QStringList({"1", "2", "3"});
    originalList << QStringList({"x", "y", "z"});

    QString llstring;
    int buildCount = US_Util::listlistBuild(originalList, llstring);

    EXPECT_EQ(buildCount, 3);
    EXPECT_FALSE(llstring.isEmpty());

    QList<QStringList> parsedList;
    int parseCount = US_Util::listlistParse(parsedList, llstring);

    EXPECT_EQ(parseCount, 3);
    EXPECT_EQ(parsedList.size(), originalList.size());

    for (int i = 0; i < originalList.size(); ++i) {
        EXPECT_EQ(parsedList[i].size(), originalList[i].size());
        for (int j = 0; j < originalList[i].size(); ++j) {
            EXPECT_THAT(parsedList[i][j], QStringEq(originalList[i][j]));
        }
    }
}

TEST_F(US_UtilTest, ListlistBuildEmptyList) {
    QList<QStringList> emptyList;
    QString llstring;

    int count = US_Util::listlistBuild(emptyList, llstring);

    EXPECT_EQ(count, 0);
}

TEST_F(US_UtilTest, ListlistBuildSingleElementLists) {
    QList<QStringList> singleElementLists;
    singleElementLists << QStringList({"only"});
    singleElementLists << QStringList({"one"});
    singleElementLists << QStringList({"each"});

    QString llstring;
    int buildCount = US_Util::listlistBuild(singleElementLists, llstring);

    EXPECT_EQ(buildCount, 3);

    QList<QStringList> parsedList;
    int parseCount = US_Util::listlistParse(parsedList, llstring);

    EXPECT_EQ(parseCount, 3);
    EXPECT_EQ(parsedList[0][0].toStdString(), "only");
    EXPECT_EQ(parsedList[1][0].toStdString(), "one");
    EXPECT_EQ(parsedList[2][0].toStdString(), "each");
}

TEST_F(US_UtilTest, ListlistParseInvalidString) {
    QList<QStringList> parsedList;
    QString invalidString = "x";  // Too short to have delimiters

    int count = US_Util::listlistParse(parsedList, invalidString);

    EXPECT_EQ(count, 0);
    EXPECT_TRUE(parsedList.isEmpty());
}

// Edge case and error handling tests
class US_UtilEdgeCaseTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
    }

    void TearDown() override {
        QtTestBase::TearDown();
    }
};

TEST_F(US_UtilEdgeCaseTest, GetTokenWithMultiCharSeparator) {
    QString testString = "token1::token2::token3";
    QString separator = "::";

    QString token1 = US_Util::getToken(testString, separator);
    EXPECT_THAT(token1, QStringEq("token1"));

    QString token2 = US_Util::getToken(testString, separator);
    EXPECT_THAT(token2, QStringEq("token2"));
}

TEST_F(US_UtilEdgeCaseTest, CompressedTripleEdgeCases) {
    // Test with longer cell numbers
    QString expanded = "12 / A / 1000";
    QString compressed = US_Util::compressed_triple(expanded);
    EXPECT_THAT(compressed, QStringEq("12A1000"));

    // Test expansion back
    QString expandedBack = US_Util::expanded_triple(compressed, true);
    EXPECT_THAT(expandedBack, QStringEq("12 / A / 1000"));
}

TEST_F(US_UtilEdgeCaseTest, ListlistWithSpecialCharacters) {
    QList<QStringList> specialList;
    specialList << QStringList({"hello world", "test string"});
    specialList << QStringList({"number 123", "symbol @#$"});

    QString llstring;
    int buildCount = US_Util::listlistBuild(specialList, llstring);

    EXPECT_EQ(buildCount, 2);

    QList<QStringList> parsedList;
    int parseCount = US_Util::listlistParse(parsedList, llstring);

    EXPECT_EQ(parseCount, 2);
    EXPECT_THAT(parsedList[0][0], QStringEq("hello world"));
    EXPECT_THAT(parsedList[1][1], QStringEq("symbol @#$"));
}