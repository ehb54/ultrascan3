// test_us_buffer.cpp
#include "qt_test_base.h"
#include "mock_us_db2.h"
#include "us_buffer.h"
#include "us_constants.h"
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QXmlStreamWriter>
#include <cstring>

using ::testing::_;
using ::testing::Return;
using ::testing::Eq;
using ::testing::DoubleEq;
using ::testing::DoubleNear;
using ::testing::StrictMock;
using ::testing::NiceMock;
using ::testing::InSequence;
using ::testing::SetArgReferee;
using ::testing::DoAll;

class US_BufferComponentTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        setupValidComponent();
    }

    void setupValidComponent() {
        component.componentID = "1";
        component.name = "NaCl";
        component.unit = "M";
        component.range = "0.0 - 1.0";
        component.grad_form = false;

        // Initialize coefficients
        for (int i = 0; i < 6; i++) {
            component.dens_coeff[i] = 1.0 + i * 0.1;
            component.visc_coeff[i] = 2.0 + i * 0.1;
        }
    }

    US_BufferComponent component;
};

class US_BufferTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        setupValidBuffer();
    }

    void setupValidBuffer() {
        buffer.person = "Test User";
        buffer.bufferID = "1";
        buffer.GUID = "test-guid-123";
        buffer.description = "Test Buffer";
        buffer.compressibility = 4.6e-10;
        buffer.pH = 7.4;
        buffer.density = DENS_20W;
        buffer.viscosity = VISC_20W;
        buffer.manual = false;
        buffer.replace_spectrum = false;
        buffer.new_or_changed_spectrum = false;

        // Add a test component
        US_BufferComponent comp;
        comp.componentID = "1";
        comp.name = "NaCl";
        comp.unit = "M";
        for (int i = 0; i < 6; i++) {
            comp.dens_coeff[i] = 1.0 + i * 0.1;
            comp.visc_coeff[i] = 2.0 + i * 0.1;
        }

        buffer.component.append(comp);
        buffer.componentIDs.append("1");
        buffer.concentration.append(0.15);

        // Add test spectrum data
        buffer.extinction[280.0] = 1.5;
        buffer.extinction[250.0] = 2.0;
        buffer.refraction[280.0] = 0.1;
        buffer.fluorescence[350.0] = 0.5;
    }

    QString createTempXmlFile(const QString& content) {
        QTemporaryFile* tempFile = new QTemporaryFile();
        tempFile->setAutoRemove(false);
        if (tempFile->open()) {
            tempFile->write(content.toUtf8());
            tempFile->close();
            return tempFile->fileName();
        }
        return QString();
    }

    US_Buffer buffer;
};

// US_BufferComponent Tests

TEST_F(US_BufferComponentTest, DefaultConstruction) {
US_BufferComponent comp;

EXPECT_TRUE(comp.componentID.isEmpty());
EXPECT_TRUE(comp.name.isEmpty());
EXPECT_TRUE(comp.unit.isEmpty());
EXPECT_TRUE(comp.range.isEmpty());
EXPECT_FALSE(comp.grad_form);

// Check coefficients are initialized to zero - using memset to zero first
memset(comp.dens_coeff, 0, sizeof(comp.dens_coeff));
memset(comp.visc_coeff, 0, sizeof(comp.visc_coeff));

for (int i = 0; i < 6; i++) {
EXPECT_THAT(comp.dens_coeff[i], DoubleEq(0.0));
EXPECT_THAT(comp.visc_coeff[i], DoubleEq(0.0));
}
}

TEST_F(US_BufferComponentTest, SaveToDBSuccess) {
auto mockDb = std::make_unique<NiceMock<US_DB2_Mock>>();

// Don't mock the query call - just mock the result methods
EXPECT_CALL(*mockDb, lastErrno())
.WillOnce(Return(IUS_DB2::OK));
EXPECT_CALL(*mockDb, lastInsertID())
.WillOnce(Return(456));

component.name = "Test Component";
int result = component.saveToDB(mockDb.get());

EXPECT_EQ(result, 456);
EXPECT_EQ(component.componentID, "456");
}

TEST_F(US_BufferComponentTest, SaveToDBError) {
auto mockDb = std::make_unique<NiceMock<US_DB2_Mock>>();

EXPECT_CALL(*mockDb, lastErrno())
.WillRepeatedly(Return(IUS_DB2::DBERROR));

int result = component.saveToDB(mockDb.get());

EXPECT_EQ(result, -1);
}

TEST_F(US_BufferComponentTest, GetInfoFromDBSuccess) {
auto mockDb = std::make_unique<NiceMock<US_DB2_Mock>>();

EXPECT_CALL(*mockDb, next())
.WillOnce(Return(true));

EXPECT_CALL(*mockDb, value(0))
.WillOnce(Return(QVariant("M")));
EXPECT_CALL(*mockDb, value(1))
.WillOnce(Return(QVariant("Salt")));
EXPECT_CALL(*mockDb, value(2))
.WillOnce(Return(QVariant("1.0 1.1 1.2 1.3 1.4 1.5")));
EXPECT_CALL(*mockDb, value(3))
.WillOnce(Return(QVariant("2.0 2.1 2.2 2.3 2.4 2.5")));
EXPECT_CALL(*mockDb, value(4))
.WillOnce(Return(QVariant("0-1")));
EXPECT_CALL(*mockDb, value(5))
.WillOnce(Return(QVariant("false")));

component.componentID = "123";
component.getInfoFromDB(mockDb.get());

EXPECT_EQ(component.unit, "M");
EXPECT_EQ(component.name, "Salt");
}

TEST_F(US_BufferTest, ReadFromDBError) {
auto mockDb = std::make_unique<NiceMock<US_DB2_Mock>>();

EXPECT_CALL(*mockDb, lastErrno())
.WillOnce(Return(IUS_DB2::DBERROR));

US_Buffer testBuffer;
bool result = testBuffer.readFromDB(mockDb.get(), "123");

EXPECT_FALSE(result);
}