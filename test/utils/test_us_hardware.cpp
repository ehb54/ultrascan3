// test_us_hardware.cpp
#include "qt_test_base.h"
#include "us_hardware.h"

class US_AbstractCenterpieceTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
    }
};

TEST_F(US_AbstractCenterpieceTest, ParseChannelAcceptsLetters) {
    // A channel and its reference share a position, so the letters pair up.
    const struct { const char* text; int channel; } cases[] = {
        {"A", 0}, {"B", 0}, {"C", 1}, {"D", 1},
        {"E", 2}, {"F", 2}, {"G", 3}, {"H", 3},
        {"a", 0}, {"d", 1},          // case-insensitive
        {" C ", 1},                  // surrounding whitespace
    };

    for (const auto& c : cases) {
        SCOPED_TRACE(c.text);
        int channel = -1;
        QString error;
        EXPECT_TRUE(US_AbstractCenterpiece::parse_channel(c.text, channel, error))
            << error.toStdString();
        EXPECT_EQ(channel, c.channel);
        EXPECT_TRUE(error.isEmpty());
    }
}

TEST_F(US_AbstractCenterpieceTest, ParseChannelAcceptsChannelIndices) {
    for (int expected = 0; expected < 4; expected++) {
        int channel = -1;
        QString error;
        EXPECT_TRUE(US_AbstractCenterpiece::parse_channel(
            QString::number(expected), channel, error));
        EXPECT_EQ(channel, expected);
    }
}

TEST_F(US_AbstractCenterpieceTest, ParseChannelRejectsGarbage) {
    // QString::toInt() otherwise maps invalid text to channel 0.
    for (const char* text : {"Z", "abc", "", "  ", "1A", "2.5"}) {
        SCOPED_TRACE(text);
        int channel = -1;
        QString error;
        EXPECT_FALSE(US_AbstractCenterpiece::parse_channel(text, channel, error));
        EXPECT_FALSE(error.isEmpty());
    }
}

TEST_F(US_AbstractCenterpieceTest, ParseIndexAcceptsIndicesAndRejectsGarbage) {
    int index = -1;
    QString error;

    EXPECT_TRUE(US_AbstractCenterpiece::parse_index("3", index, error));
    EXPECT_EQ(index, 3);
    EXPECT_TRUE(US_AbstractCenterpiece::parse_index(" 7 ", index, error));
    EXPECT_EQ(index, 7);

    for (const char* text : {"abc", "", "1.5", "A"}) {
        SCOPED_TRACE(text);
        EXPECT_FALSE(US_AbstractCenterpiece::parse_index(text, index, error));
        EXPECT_FALSE(error.isEmpty());
    }
}

TEST_F(US_AbstractCenterpieceTest, ValidateRejectsOutOfRangeIndices) {
    // Skip if the centerpiece table is not reachable in this environment;
    // validate() reports that case with its own message.
    if (US_AbstractCenterpiece::validate(0, 0).contains("no centerpiece"))
        GTEST_SKIP() << "centerpiece definitions unavailable";

    EXPECT_TRUE(US_AbstractCenterpiece::validate(0, 0).isEmpty());
    EXPECT_FALSE(US_AbstractCenterpiece::validate(-1, 0).isEmpty());
    EXPECT_FALSE(US_AbstractCenterpiece::validate(9999, 0).isEmpty());
    EXPECT_FALSE(US_AbstractCenterpiece::validate(0, -1).isEmpty());

    // Centerpiece 0 has a single channel, so anything above 0 is out of range.
    EXPECT_FALSE(US_AbstractCenterpiece::validate(0, 1).isEmpty());
}
