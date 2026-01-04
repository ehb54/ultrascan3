#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QBuffer>
#include <QDebug>
#include <QStringList>
#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>

#include "us_ana_profile.h"
#include "qt_test_base.h"

using ::testing::_;
using ::testing::Not;
using ::testing::IsEmpty;
using ::testing::DoubleEq;
using ::testing::DoubleNear;
using ::qt_matchers::QStringContains;
using ::qt_matchers::QStringEq;

namespace {

// Test fixture for US_AnaProfile functionality
    class US_AnaProfileTest : public QtTestBase {
    protected:
        void SetUp() override {
            QtTestBase::SetUp();
            profile = std::make_unique<US_AnaProfile>();
        }

        void TearDown() override {
            profile.reset();
            QtTestBase::TearDown();
        }

        // Helper to create valid XML content for testing
        QString createValidXmlContent() {
            return R"DELIM(
<AnalysisProfileData version="1.0">
    <analysis_profile name="TestProfile" guid="12345678-1234-1234-1234-123456789012">
        <channel_parms channel="1A" chandesc="1A:UV/vis.:(unspecified)"
            load_concen_ratio="1.0" lcr_tolerance="5.0" load_volume="460.0"
            lv_tolerance="10.0" data_end="7.0" load_dens="1.42"
            grad_vbar="0.2661" grad_mw="168.36" run="1" run_report="1"
            abde_reference="0" abde_use_reference="0" wvl_edit="180"
            wvl_not_run="" chandesc_alt="1A:UV/vis.:(unspecified)"
            scan_excl_begin="0" scan_excl_end="0" replicate_group="0"
            report_id="101,102" report_guid="guid1,guid2"/>
    </analysis_profile>
</AnalysisProfileData>
        )DELIM";
        }

        // Helper to write profile to XML string
        QString profileToXmlString(const US_AnaProfile& prof) {
            QBuffer buffer;
            buffer.open(QIODevice::WriteOnly);
            QXmlStreamWriter writer(&buffer);

            bool success = const_cast<US_AnaProfile&>(prof).toXml(writer);
            buffer.close();

            return success ? QString::fromUtf8(buffer.data()) : QString();
        }

        // Helper to read profile from XML string
        bool profileFromXmlString(US_AnaProfile& prof, const QString& xmlString) {
            QBuffer buffer;
            buffer.setData(xmlString.toUtf8());
            buffer.open(QIODevice::ReadOnly);
            QXmlStreamReader reader(&buffer);

            bool success = prof.fromXml(reader);
            buffer.close();
            return success;
        }

        std::unique_ptr<US_AnaProfile> profile;
    };

// Test fixture for constructor testing
    class US_AnaProfileConstructorTest : public US_AnaProfileTest {
    };

// Test fixture for equality operator testing
    class US_AnaProfileEqualityTest : public US_AnaProfileTest {
    protected:
        void SetUp() override {
            US_AnaProfileTest::SetUp();
            profile1 = std::make_unique<US_AnaProfile>();
            profile2 = std::make_unique<US_AnaProfile>();
        }

        std::unique_ptr<US_AnaProfile> profile1, profile2;
    };

// Test fixture for XML serialization testing
    class US_AnaProfileXmlTest : public US_AnaProfileTest {
    };

// Test fixture for 2DSA controls testing
    class US_AnaProfile2DSATest : public US_AnaProfileTest {
    protected:
        void SetUp() override {
            US_AnaProfileTest::SetUp();
            twoDSA = std::make_unique<US_AnaProfile::AnaProf2DSA>();
        }

        std::unique_ptr<US_AnaProfile::AnaProf2DSA> twoDSA;
    };

// Test fixture for PCSA controls testing
    class US_AnaProfilePCSATest : public US_AnaProfileTest {
    protected:
        void SetUp() override {
            US_AnaProfileTest::SetUp();
            pcsa = std::make_unique<US_AnaProfile::AnaProfPCSA>();
        }

        std::unique_ptr<US_AnaProfile::AnaProfPCSA> pcsa;
    };

// =============================================================================
// Constructor Tests
// =============================================================================

    TEST_F(US_AnaProfileConstructorTest, DefaultConstructorInitializesBasicFields) {
    EXPECT_THAT(profile->aprofname, QStringEq(""));
    EXPECT_THAT(profile->aprofGUID, QStringEq("00000000-0000-0000-0000-000000000000"));
    EXPECT_THAT(profile->protoname, QStringEq(""));
    EXPECT_THAT(profile->protoGUID, QStringEq("00000000-0000-0000-0000-000000000000"));
}

TEST_F(US_AnaProfileConstructorTest, DefaultConstructorInitializesChannelData) {
EXPECT_EQ(profile->pchans.size(), 1);
EXPECT_THAT(profile->pchans[0], QStringEq("1A"));

EXPECT_EQ(profile->chndescs.size(), 1);
EXPECT_THAT(profile->chndescs[0], QStringEq("1A:UV/vis.:(unspecified)"));

EXPECT_EQ(profile->chndescs_alt.size(), 1);
EXPECT_THAT(profile->chndescs_alt[0], QStringEq("1A:UV/vis.:(unspecified)"));
}

TEST_F(US_AnaProfileConstructorTest, DefaultConstructorInitializesNumericLists) {
EXPECT_EQ(profile->lc_ratios.size(), 1);
EXPECT_THAT(profile->lc_ratios[0], DoubleEq(1.0));

EXPECT_EQ(profile->lc_tolers.size(), 1);
EXPECT_THAT(profile->lc_tolers[0], DoubleEq(5.0));

EXPECT_EQ(profile->l_volumes.size(), 1);
EXPECT_THAT(profile->l_volumes[0], DoubleEq(460.0));

EXPECT_EQ(profile->lv_tolers.size(), 1);
EXPECT_THAT(profile->lv_tolers[0], DoubleEq(10.0));

EXPECT_EQ(profile->data_ends.size(), 1);
EXPECT_THAT(profile->data_ends[0], DoubleEq(7.0));
}

TEST_F(US_AnaProfileConstructorTest, DefaultConstructorInitializesABDEValues) {
EXPECT_EQ(profile->ld_dens_0s.size(), 1);
EXPECT_THAT(profile->ld_dens_0s[0], DoubleEq(1.42));

EXPECT_EQ(profile->gm_vbars.size(), 1);
EXPECT_THAT(profile->gm_vbars[0], DoubleEq(0.2661));

EXPECT_EQ(profile->gm_mws.size(), 1);
EXPECT_THAT(profile->gm_mws[0], DoubleEq(168.36));

EXPECT_EQ(profile->ref_channels.size(), 1);
EXPECT_EQ(profile->ref_channels[0], 0);

EXPECT_EQ(profile->ref_use_channels.size(), 1);
EXPECT_EQ(profile->ref_use_channels[0], 0);
}

TEST_F(US_AnaProfileConstructorTest, DefaultConstructorInitializesRunSettings) {
EXPECT_EQ(profile->analysis_run.size(), 1);
EXPECT_EQ(profile->analysis_run[0], 1);

EXPECT_EQ(profile->report_run.size(), 1);
EXPECT_EQ(profile->report_run[0], 1);

EXPECT_EQ(profile->scan_excl_begin.size(), 1);
EXPECT_EQ(profile->scan_excl_begin[0], 0);

EXPECT_EQ(profile->scan_excl_end.size(), 1);
EXPECT_EQ(profile->scan_excl_end[0], 0);

EXPECT_EQ(profile->scan_excl_nth.size(), 1);
EXPECT_EQ(profile->scan_excl_nth[0], 1);

EXPECT_EQ(profile->replicates.size(), 1);
EXPECT_EQ(profile->replicates[0], 0);
}

TEST_F(US_AnaProfileConstructorTest, DefaultConstructorInitializesWavelengthData) {
EXPECT_EQ(profile->wvl_edit.size(), 1);
EXPECT_EQ(profile->wvl_edit[0], 180);

EXPECT_EQ(profile->wvl_not_run.size(), 1);
EXPECT_THAT(profile->wvl_not_run[0], QStringEq(""));

// Check ch_wvls map
EXPECT_EQ(profile->ch_wvls.size(), 1);
QString defaultChannelDesc = "1A:UV/vis.:(unspecified)";
EXPECT_TRUE(profile->ch_wvls.contains(defaultChannelDesc));
EXPECT_EQ(profile->ch_wvls[defaultChannelDesc].size(), 1);
EXPECT_THAT(profile->ch_wvls[defaultChannelDesc][0], DoubleEq(180.0));
}

TEST_F(US_AnaProfileConstructorTest, DefaultConstructorInitializesJSONFields) {
EXPECT_FALSE(profile->report_mask.isEmpty());
EXPECT_FALSE(profile->combPlots_parms.isEmpty());

// Verify JSON structure is valid
QJsonParseError error;
QJsonDocument reportDoc = QJsonDocument::fromJson(profile->report_mask.toUtf8(), &error);
EXPECT_EQ(error.error, QJsonParseError::NoError);
EXPECT_TRUE(reportDoc.isObject());

QJsonDocument plotsDoc = QJsonDocument::fromJson(profile->combPlots_parms.toUtf8(), &error);
EXPECT_EQ(error.error, QJsonParseError::NoError);
EXPECT_TRUE(plotsDoc.isObject());
}

TEST_F(US_AnaProfileConstructorTest, DefaultConstructorInitializesSubObjects) {
// Verify 2DSA defaults
EXPECT_THAT(profile->ap2DSA.fitrng, DoubleEq(0.03));
EXPECT_EQ(profile->ap2DSA.nchan, 1);
EXPECT_EQ(profile->ap2DSA.grpoints, 11);
EXPECT_TRUE(profile->ap2DSA.job1run);
EXPECT_TRUE(profile->ap2DSA.job2run);
EXPECT_EQ(profile->ap2DSA.parms.size(), 1);

// Verify PCSA defaults
EXPECT_EQ(profile->apPCSA.nchan, 0);
EXPECT_TRUE(profile->apPCSA.job_run);
EXPECT_EQ(profile->apPCSA.parms.size(), 0);
}

// =============================================================================
// Equality Operator Tests
// =============================================================================

TEST_F(US_AnaProfileEqualityTest, DefaultProfilesAreEqual) {
EXPECT_TRUE(*profile1 == *profile2);
EXPECT_FALSE(*profile1 != *profile2);
}

TEST_F(US_AnaProfileEqualityTest, ModifiedNameMakesProfilesUnequal) {
profile1->aprofname = "Modified Profile";
EXPECT_FALSE(*profile1 == *profile2);
EXPECT_TRUE(*profile1 != *profile2);
}

TEST_F(US_AnaProfileEqualityTest, ModifiedGUIDMakesProfilesUnequal) {
profile1->aprofGUID = "12345678-1234-1234-1234-123456789012";
EXPECT_FALSE(*profile1 == *profile2);
}

TEST_F(US_AnaProfileEqualityTest, ModifiedProtocolFieldsMakeProfilesUnequal) {
profile1->protoname = "Test Protocol";
EXPECT_FALSE(*profile1 == *profile2);

*profile1 = *profile2; // Reset
profile1->protoGUID = "87654321-4321-4321-4321-210987654321";
EXPECT_FALSE(*profile1 == *profile2);
}

TEST_F(US_AnaProfileEqualityTest, ModifiedNumericListsMakeProfilesUnequal) {
profile1->lc_ratios[0] = 2.0;
EXPECT_FALSE(*profile1 == *profile2);

*profile1 = *profile2; // Reset
profile1->l_volumes[0] = 500.0;
EXPECT_FALSE(*profile1 == *profile2);

*profile1 = *profile2; // Reset
profile1->ld_dens_0s[0] = 1.5;
EXPECT_FALSE(*profile1 == *profile2);
}

TEST_F(US_AnaProfileEqualityTest, ModifiedABDEValuesMakeProfilesUnequal) {
profile1->gm_vbars[0] = 0.3;
EXPECT_FALSE(*profile1 == *profile2);

*profile1 = *profile2; // Reset
profile1->gm_mws[0] = 200.0;
EXPECT_FALSE(*profile1 == *profile2);

*profile1 = *profile2; // Reset
profile1->ref_channels[0] = 1;
EXPECT_FALSE(*profile1 == *profile2);
}

TEST_F(US_AnaProfileEqualityTest, ModifiedRunSettingsMakeProfilesUnequal) {
profile1->analysis_run[0] = 0;
EXPECT_FALSE(*profile1 == *profile2);

*profile1 = *profile2; // Reset
profile1->report_run[0] = 0;
EXPECT_FALSE(*profile1 == *profile2);

*profile1 = *profile2; // Reset
profile1->scan_excl_begin[0] = 5;
EXPECT_FALSE(*profile1 == *profile2);
}

TEST_F(US_AnaProfileEqualityTest, ModifiedSubObjectsMakeProfilesUnequal) {
profile1->ap2DSA.fitrng = 0.05;
EXPECT_FALSE(*profile1 == *profile2);

*profile1 = *profile2; // Reset
profile1->apPCSA.job_run = false;
EXPECT_FALSE(*profile1 == *profile2);
}

TEST_F(US_AnaProfileEqualityTest, ModifiedJSONFieldsMakeProfilesUnequal) {
profile1->report_mask = "{}";
EXPECT_FALSE(*profile1 == *profile2);

*profile1 = *profile2; // Reset
profile1->combPlots_parms = "{}";
EXPECT_FALSE(*profile1 == *profile2);
}

// =============================================================================
// XML Serialization Tests
// =============================================================================

TEST_F(US_AnaProfileXmlTest, ToXmlProducesValidXMLStructure) {
profile->aprofname = "TestProfile";
profile->aprofGUID = "12345678-1234-1234-1234-123456789012";

QString xmlString = profileToXmlString(*profile);
EXPECT_FALSE(xmlString.isEmpty());
EXPECT_THAT(xmlString, QStringContains("<?xml"));
EXPECT_THAT(xmlString, QStringContains("DOCTYPE"));
EXPECT_THAT(xmlString, QStringContains("AnalysisProfileData"));
}

TEST_F(US_AnaProfileXmlTest, ToXmlIncludesBasicAttributes) {
profile->aprofname = "TestProfile";
profile->aprofGUID = "12345678-1234-1234-1234-123456789012";

QString xmlString = profileToXmlString(*profile);
EXPECT_THAT(xmlString, QStringContains("name=\"TestProfile\""));
EXPECT_THAT(xmlString, QStringContains("guid=\"12345678-1234-1234-1234-123456789012\""));
}

TEST_F(US_AnaProfileXmlTest, ToXmlIncludesChannelParameters) {
QString xmlString = profileToXmlString(*profile);
EXPECT_THAT(xmlString, QStringContains("channel_parms"));
EXPECT_THAT(xmlString, QStringContains("channel=\"1A\""));
EXPECT_THAT(xmlString, QStringContains("load_concen_ratio=\"1\""));
EXPECT_THAT(xmlString, QStringContains("load_volume=\"460\""));
EXPECT_THAT(xmlString, QStringContains("load_dens=\"1.42\""));
EXPECT_THAT(xmlString, QStringContains("grad_vbar=\"0.2661\""));
}

TEST_F(US_AnaProfileXmlTest, ToXmlSkipsInterferenceBChannels) {
// Add an Interference B channel (should be skipped in XML output)
profile->pchans << "1B";
profile->chndescs << "1B:Interf.:(test)";
profile->chndescs_alt << "1B:Interf.:(test)";
profile->lc_ratios << 1.0;
profile->lc_tolers << 5.0;
profile->l_volumes << 460.0;
profile->lv_tolers << 10.0;
profile->data_ends << 7.0;
profile->ld_dens_0s << 1.42;
profile->gm_vbars << 0.2661;
profile->gm_mws << 168.36;
profile->ref_channels << 0;
profile->ref_use_channels << 0;
profile->analysis_run << 1;
profile->report_run << 1;
profile->scan_excl_begin << 0;
profile->scan_excl_end << 0;
profile->scan_excl_nth << 1;
profile->replicates << 0;
profile->wvl_edit << 180;
profile->wvl_not_run << "";

QString xmlString = profileToXmlString(*profile);
EXPECT_THAT(xmlString, Not(QStringContains("1B:Interf")));
EXPECT_THAT(xmlString, QStringContains("channel=\"1A\""));
}

TEST_F(US_AnaProfileXmlTest, ToXmlHandlesMultipleChannels) {
// Add a second channel
profile->pchans << "2A";
profile->chndescs << "2A:UV/vis.:(sample)";
profile->chndescs_alt << "2A:UV/vis.:(sample)";
profile->lc_ratios << 2.0;
profile->lc_tolers << 7.5;
profile->l_volumes << 480.0;
profile->lv_tolers << 12.0;
profile->data_ends << 8.0;
profile->ld_dens_0s << 1.45;
profile->gm_vbars << 0.27;
profile->gm_mws << 180.0;
profile->ref_channels << 1;
profile->ref_use_channels << 1;
profile->analysis_run << 1;
profile->report_run << 1;
profile->scan_excl_begin << 0;
profile->scan_excl_end << 0;
profile->scan_excl_nth << 1;
profile->replicates << 0;
profile->wvl_edit << 200;
profile->wvl_not_run << "";

QString xmlString = profileToXmlString(*profile);
EXPECT_THAT(xmlString, QStringContains("channel=\"1A\""));
EXPECT_THAT(xmlString, QStringContains("channel=\"2A\""));
}

TEST_F(US_AnaProfileXmlTest, FromXmlReadsBasicAttributes) {
QString xmlContent = createValidXmlContent();
US_AnaProfile testProfile;

EXPECT_TRUE(profileFromXmlString(testProfile, xmlContent));
EXPECT_THAT(testProfile.aprofname, QStringEq("TestProfile"));
EXPECT_THAT(testProfile.aprofGUID, QStringEq("12345678-1234-1234-1234-123456789012"));
}

TEST_F(US_AnaProfileXmlTest, FromXmlReadsChannelParameters) {
QString xmlContent = createValidXmlContent();
US_AnaProfile testProfile;

EXPECT_TRUE(profileFromXmlString(testProfile, xmlContent));

EXPECT_EQ(testProfile.pchans.size(), 1);
EXPECT_THAT(testProfile.pchans[0], QStringEq("1A"));

EXPECT_EQ(testProfile.chndescs.size(), 1);
EXPECT_THAT(testProfile.chndescs[0], QStringEq("1A:UV/vis.:(unspecified)"));

EXPECT_EQ(testProfile.lc_ratios.size(), 1);
EXPECT_THAT(testProfile.lc_ratios[0], DoubleEq(1.0));

EXPECT_EQ(testProfile.l_volumes.size(), 1);
EXPECT_THAT(testProfile.l_volumes[0], DoubleEq(460.0));
}

TEST_F(US_AnaProfileXmlTest, FromXmlReadsABDEParameters) {
QString xmlContent = createValidXmlContent();
US_AnaProfile testProfile;

EXPECT_TRUE(profileFromXmlString(testProfile, xmlContent));

EXPECT_EQ(testProfile.ld_dens_0s.size(), 1);
EXPECT_THAT(testProfile.ld_dens_0s[0], DoubleEq(1.42));

EXPECT_EQ(testProfile.gm_vbars.size(), 1);
EXPECT_THAT(testProfile.gm_vbars[0], DoubleEq(0.2661));

EXPECT_EQ(testProfile.gm_mws.size(), 1);
EXPECT_THAT(testProfile.gm_mws[0], DoubleEq(168.36));
}

TEST_F(US_AnaProfileXmlTest, FromXmlHandlesMissingAttributesWithDefaults) {
QString xmlWithMissingAttrs = R"DELIM(
<AnalysisProfileData version="1.0">
    <analysis_profile name="MinimalProfile" guid="12345678-1234-1234-1234-123456789012">
        <channel_parms channel="1A" chandesc="1A:UV/vis.:(unspecified)"
            load_concen_ratio="1.0" lcr_tolerance="5.0" load_volume="460.0"
            lv_tolerance="10.0" data_end="7.0" run="1"
            wvl_edit="180" wvl_not_run="" chandesc_alt="1A:UV/vis.:(unspecified)"/>
    </analysis_profile>
</AnalysisProfileData>
    )DELIM";

US_AnaProfile testProfile;
EXPECT_TRUE(profileFromXmlString(testProfile, xmlWithMissingAttrs));

// Should have defaults for missing attributes
EXPECT_EQ(testProfile.ld_dens_0s.size(), 1);
EXPECT_THAT(testProfile.ld_dens_0s[0], DoubleEq(1.42)); // Default value

EXPECT_EQ(testProfile.ref_channels.size(), 1);
EXPECT_EQ(testProfile.ref_channels[0], 0); // Default value
}

TEST_F(US_AnaProfileXmlTest, FromXmlHandlesMalformedXML) {
QString malformedXml = "<invalid><xml</invalid>";

US_AnaProfile testProfile;
EXPECT_FALSE(profileFromXmlString(testProfile, malformedXml));
}

TEST_F(US_AnaProfileXmlTest, FromXmlHandlesEmptyContent) {
QString emptyXml = "";

US_AnaProfile testProfile;
EXPECT_FALSE(profileFromXmlString(testProfile, emptyXml));
}

TEST_F(US_AnaProfileXmlTest, RoundTripSerializationPreservesData) {
// Modify profile with custom data
profile->aprofname = "RoundTripTest";
profile->aprofGUID = "11111111-2222-3333-4444-555555555555";
profile->lc_ratios[0] = 2.5;
profile->l_volumes[0] = 500.0;

// Serialize to XML
QString xmlString = profileToXmlString(*profile);
EXPECT_FALSE(xmlString.isEmpty());

// Deserialize from XML
US_AnaProfile restoredProfile;
EXPECT_TRUE(profileFromXmlString(restoredProfile, xmlString));

// Verify data preservation
EXPECT_THAT(restoredProfile.aprofname, QStringEq(profile->aprofname));
EXPECT_THAT(restoredProfile.aprofGUID, QStringEq(profile->aprofGUID));
EXPECT_THAT(restoredProfile.lc_ratios[0], DoubleEq(profile->lc_ratios[0]));
EXPECT_THAT(restoredProfile.l_volumes[0], DoubleEq(profile->l_volumes[0]));
}

// =============================================================================
// 2DSA Controls Tests
// =============================================================================

TEST_F(US_AnaProfile2DSATest, DefaultConstructorSetsCorrectDefaults) {
EXPECT_THAT(twoDSA->fitrng, DoubleEq(0.03));
EXPECT_EQ(twoDSA->nchan, 1);
EXPECT_EQ(twoDSA->grpoints, 11);
EXPECT_EQ(twoDSA->j2rfiters, 1);
EXPECT_EQ(twoDSA->rfiters, 10);
EXPECT_EQ(twoDSA->mciters, 100);
EXPECT_TRUE(twoDSA->job1run);
EXPECT_TRUE(twoDSA->job2run);
EXPECT_TRUE(twoDSA->job3run);
EXPECT_TRUE(twoDSA->job4run);
EXPECT_TRUE(twoDSA->job5run);
EXPECT_FALSE(twoDSA->job3auto);
EXPECT_THAT(twoDSA->job1nois, QStringEq("ti"));
EXPECT_THAT(twoDSA->job2nois, QStringEq("both"));
EXPECT_THAT(twoDSA->job4nois, QStringEq("both"));
EXPECT_THAT(twoDSA->fmb, QStringEq("fm"));
}

TEST_F(US_AnaProfile2DSATest, DefaultParametersAreCorrect) {
EXPECT_EQ(twoDSA->parms.size(), 1);

const auto& parm = twoDSA->parms[0];
EXPECT_THAT(parm.s_min, DoubleEq(1.0));
EXPECT_THAT(parm.s_max, DoubleEq(10.0));
EXPECT_THAT(parm.k_min, DoubleEq(1.0));
EXPECT_THAT(parm.k_max, DoubleEq(4.0));
EXPECT_THAT(parm.ff0_const, DoubleEq(1.0));
EXPECT_EQ(parm.s_grpts, 64);
EXPECT_EQ(parm.k_grpts, 64);
EXPECT_EQ(parm.gridreps, 8);
EXPECT_FALSE(parm.varyvbar);
EXPECT_FALSE(parm.have_custg);
EXPECT_THAT(parm.channel, QStringEq("1A"));
EXPECT_THAT(parm.cust_grid, QStringEq(""));
EXPECT_THAT(parm.cgrid_name, QStringEq(""));
}

TEST_F(US_AnaProfile2DSATest, EqualityOperatorWorksCorrectly) {
US_AnaProfile::AnaProf2DSA twoDSA1;
US_AnaProfile::AnaProf2DSA twoDSA2;

EXPECT_TRUE(twoDSA1 == twoDSA2);

twoDSA1.fitrng = 0.05;
EXPECT_FALSE(twoDSA1 == twoDSA2);

twoDSA2.fitrng = 0.05;
EXPECT_TRUE(twoDSA1 == twoDSA2);

twoDSA1.job1run = false;
EXPECT_FALSE(twoDSA1 == twoDSA2);
}

TEST_F(US_AnaProfile2DSATest, InequalityOperatorWorksCorrectly) {
US_AnaProfile::AnaProf2DSA twoDSA1;
US_AnaProfile::AnaProf2DSA twoDSA2;

EXPECT_FALSE(twoDSA1 != twoDSA2);

twoDSA1.fitrng = 0.05;
EXPECT_TRUE(twoDSA1 != twoDSA2);
}

TEST_F(US_AnaProfile2DSATest, Parm2DSAEqualityOperatorWorksCorrectly) {
US_AnaProfile::AnaProf2DSA::Parm2DSA parm1;
US_AnaProfile::AnaProf2DSA::Parm2DSA parm2;

EXPECT_TRUE(parm1 == parm2);

parm1.s_min = 2.0;
EXPECT_FALSE(parm1 == parm2);

parm2.s_min = 2.0;
EXPECT_TRUE(parm1 == parm2);

parm1.channel = "2A";
EXPECT_FALSE(parm1 == parm2);

parm2.channel = "2A";
EXPECT_TRUE(parm1 == parm2);

parm1.varyvbar = true;
EXPECT_FALSE(parm1 == parm2);
}

TEST_F(US_AnaProfile2DSATest, Parm2DSAInequalityOperatorWorksCorrectly) {
US_AnaProfile::AnaProf2DSA::Parm2DSA parm1;
US_AnaProfile::AnaProf2DSA::Parm2DSA parm2;

EXPECT_FALSE(parm1 != parm2);

parm1.s_min = 2.0;
EXPECT_TRUE(parm1 != parm2);
}

// =============================================================================
// PCSA Controls Tests
// =============================================================================

TEST_F(US_AnaProfilePCSATest, DefaultConstructorSetsCorrectDefaults) {
EXPECT_EQ(pcsa->nchan, 0);
EXPECT_TRUE(pcsa->job_run);
EXPECT_EQ(pcsa->parms.size(), 0);
}

TEST_F(US_AnaProfilePCSATest, ParmPCSADefaultConstructorSetsCorrectDefaults) {
US_AnaProfile::AnaProfPCSA::ParmPCSA parm;

EXPECT_THAT(parm.x_min, DoubleEq(1.0));
EXPECT_THAT(parm.x_max, DoubleEq(10.0));
EXPECT_THAT(parm.y_min, DoubleEq(1.0));
EXPECT_THAT(parm.y_max, DoubleEq(4.0));
EXPECT_THAT(parm.z_value, DoubleEq(0.0));
EXPECT_THAT(parm.tr_alpha, DoubleEq(0.0));
EXPECT_EQ(parm.varcount, 10);
EXPECT_EQ(parm.grf_iters, 3);
EXPECT_EQ(parm.creso_pts, 200);
EXPECT_EQ(parm.noise_flag, 0);
EXPECT_EQ(parm.treg_flag, 0);
EXPECT_EQ(parm.mc_iters, 100);
EXPECT_THAT(parm.channel, QStringEq("1A"));
EXPECT_THAT(parm.curv_type, QStringEq("All"));
EXPECT_THAT(parm.x_type, QStringEq("s"));
EXPECT_THAT(parm.y_type, QStringEq("f/f0"));
EXPECT_THAT(parm.z_type, QStringEq("vbar"));
EXPECT_THAT(parm.noise_type, QStringEq("none"));
EXPECT_THAT(parm.treg_type, QStringEq("none"));
}

TEST_F(US_AnaProfilePCSATest, CurveTypeTransformationWorksCorrectly) {
// Test XML to internal format
EXPECT_THAT(pcsa->curve_type_fromXml("All"), QStringEq("All (IS + DS + SL)"));
EXPECT_THAT(pcsa->curve_type_fromXml("IS"), QStringEq("Increasing Sigmoid"));
EXPECT_THAT(pcsa->curve_type_fromXml("DS"), QStringEq("Decreasing Sigmoid"));
EXPECT_THAT(pcsa->curve_type_fromXml("SL"), QStringEq("Straight Line"));
EXPECT_THAT(pcsa->curve_type_fromXml("HL"), QStringEq("Horizontal Line"));
EXPECT_THAT(pcsa->curve_type_fromXml("2O"), QStringEq("Second Order Power"));

// Test internal to XML format
EXPECT_THAT(pcsa->curve_type_toXml("All (IS + DS + SL)"), QStringEq("All"));
EXPECT_THAT(pcsa->curve_type_toXml("Increasing Sigmoid"), QStringEq("IS"));
EXPECT_THAT(pcsa->curve_type_toXml("Decreasing Sigmoid"), QStringEq("DS"));
EXPECT_THAT(pcsa->curve_type_toXml("Straight Line"), QStringEq("SL"));
EXPECT_THAT(pcsa->curve_type_toXml("Horizontal Line"), QStringEq("HL"));
EXPECT_THAT(pcsa->curve_type_toXml("Second Order Power"), QStringEq("2O"));
}

TEST_F(US_AnaProfilePCSATest, CurveTypeTransformationHandlesEmptyStrings) {
EXPECT_THAT(pcsa->curve_type_fromXml(""), QStringEq(""));
EXPECT_THAT(pcsa->curve_type_toXml(""), QStringEq(""));
EXPECT_THAT(pcsa->curve_type_fromXml("Unknown"), QStringEq(""));
EXPECT_THAT(pcsa->curve_type_toXml("Unknown Type"), QStringEq(""));
}

TEST_F(US_AnaProfilePCSATest, EqualityOperatorWorksCorrectly) {
US_AnaProfile::AnaProfPCSA pcsa1;
US_AnaProfile::AnaProfPCSA pcsa2;

EXPECT_TRUE(pcsa1 == pcsa2);

pcsa1.job_run = false;
EXPECT_FALSE(pcsa1 == pcsa2);

pcsa2.job_run = false;
EXPECT_TRUE(pcsa1 == pcsa2);

pcsa1.nchan = 5;
EXPECT_FALSE(pcsa1 == pcsa2);
}

TEST_F(US_AnaProfilePCSATest, InequalityOperatorWorksCorrectly) {
US_AnaProfile::AnaProfPCSA pcsa1;
US_AnaProfile::AnaProfPCSA pcsa2;

EXPECT_FALSE(pcsa1 != pcsa2);

pcsa1.job_run = false;
EXPECT_TRUE(pcsa1 != pcsa2);
}

TEST_F(US_AnaProfilePCSATest, ParmPCSAEqualityOperatorWorksCorrectly) {
US_AnaProfile::AnaProfPCSA::ParmPCSA parm1;
US_AnaProfile::AnaProfPCSA::ParmPCSA parm2;

EXPECT_TRUE(parm1 == parm2);

parm1.x_min = 2.0;
EXPECT_FALSE(parm1 == parm2);

parm2.x_min = 2.0;
EXPECT_TRUE(parm1 == parm2);

parm1.curv_type = "Increasing Sigmoid";
EXPECT_FALSE(parm1 == parm2);

parm2.curv_type = "Increasing Sigmoid";
EXPECT_TRUE(parm1 == parm2);

parm1.noise_flag = 1;
EXPECT_FALSE(parm1 == parm2);
}

TEST_F(US_AnaProfilePCSATest, ParmPCSAInequalityOperatorWorksCorrectly) {
US_AnaProfile::AnaProfPCSA::ParmPCSA parm1;
US_AnaProfile::AnaProfPCSA::ParmPCSA parm2;

EXPECT_FALSE(parm1 != parm2);

parm1.x_min = 2.0;
EXPECT_TRUE(parm1 != parm2);
}

// =============================================================================
// Edge Cases and Boundary Tests
// =============================================================================

TEST_F(US_AnaProfileTest, HandlesEmptyChannelLists) {
// Clear all channel-related data
profile->pchans.clear();
profile->chndescs.clear();
profile->chndescs_alt.clear();
profile->lc_ratios.clear();
profile->lc_tolers.clear();
profile->l_volumes.clear();
profile->lv_tolers.clear();
profile->data_ends.clear();
profile->ld_dens_0s.clear();
profile->gm_vbars.clear();
profile->gm_mws.clear();
profile->ref_channels.clear();
profile->ref_use_channels.clear();
profile->analysis_run.clear();
profile->report_run.clear();
profile->scan_excl_begin.clear();
profile->scan_excl_end.clear();
profile->scan_excl_nth.clear();
profile->replicates.clear();
profile->wvl_edit.clear();
profile->wvl_not_run.clear();

QString xmlString = profileToXmlString(*profile);
EXPECT_FALSE(xmlString.isEmpty());
EXPECT_THAT(xmlString, QStringContains("analysis_profile"));
}

TEST_F(US_AnaProfileTest, HandlesSpecialCharactersInStrings) {
profile->aprofname = "Test<>&\"'Profile";
profile->pchans[0] = "1A<>&\"'";

QString xmlString = profileToXmlString(*profile);
EXPECT_FALSE(xmlString.isEmpty());

US_AnaProfile restoredProfile;
EXPECT_TRUE(profileFromXmlString(restoredProfile, xmlString));
EXPECT_THAT(restoredProfile.aprofname, QStringEq(profile->aprofname));
EXPECT_THAT(restoredProfile.pchans[0], QStringEq(profile->pchans[0]));
}

TEST_F(US_AnaProfileTest, HandlesUnicodeStrings) {
profile->aprofname = "Protócol with ñ and ü";  // Test a field that IS serialized

QString xmlString = profileToXmlString(*profile);
US_AnaProfile restoredProfile;
EXPECT_TRUE(profileFromXmlString(restoredProfile, xmlString));
EXPECT_THAT(restoredProfile.aprofname, QStringEq(profile->aprofname));
}

TEST_F(US_AnaProfileTest, HandlesExtremeNumericValues) {
profile->lc_ratios[0] = 1e-10;
profile->l_volumes[0] = 1e10;
profile->ld_dens_0s[0] = std::numeric_limits<double>::min();

QString xmlString = profileToXmlString(*profile);
EXPECT_FALSE(xmlString.isEmpty());

US_AnaProfile restoredProfile;
EXPECT_TRUE(profileFromXmlString(restoredProfile, xmlString));
EXPECT_THAT(restoredProfile.lc_ratios[0], DoubleNear(1e-10, 1e-15));
EXPECT_THAT(restoredProfile.l_volumes[0], DoubleNear(1e10, 1e5));
}

TEST_F(US_AnaProfileTest, HandlesReportIDsAndGUIDs) {
QString channelDesc = "1A:UV/vis.:(unspecified)";
QList<int> reportIDs = {101, 102, 103};
QStringList reportGUIDs = {"guid1", "guid2", "guid3"};

profile->ch_report_ids[channelDesc] = reportIDs;
profile->ch_report_guids[channelDesc] = reportGUIDs;

QString xmlString = profileToXmlString(*profile);
EXPECT_THAT(xmlString, QStringContains("report_id=\"101,102,103\""));
EXPECT_THAT(xmlString, QStringContains("report_guid=\"guid1,guid2,guid3\""));
}

TEST_F(US_AnaProfileTest, HandlesEmptyReportIDsAndGUIDs) {
QString channelDesc = "1A:UV/vis.:(unspecified)";
QList<int> emptyReportIDs;
QStringList emptyReportGUIDs;

profile->ch_report_ids[channelDesc] = emptyReportIDs;
profile->ch_report_guids[channelDesc] = emptyReportGUIDs;

QString xmlString = profileToXmlString(*profile);
EXPECT_THAT(xmlString, QStringContains("report_id=\"\""));
EXPECT_THAT(xmlString, QStringContains("report_guid=\"\""));
}

TEST_F(US_AnaProfileTest, HandlesOverlappingWavelengths) {
QString channelDesc = "1A:UV/vis.:(unspecified)";
QStringList overlappingWvls = {"280", "290", "300"};

profile->channdesc_to_overlapping_wvls_main[channelDesc] = overlappingWvls;

QString xmlString = profileToXmlString(*profile);
EXPECT_THAT(xmlString, QStringContains("replicate_wvl_overlap=\"280,290,300\""));
}

// =============================================================================
// Error Handling Tests
// =============================================================================

TEST_F(US_AnaProfileTest, ToXmlHandlesXmlWriterErrors) {
// Test with a closed buffer to simulate write errors
QBuffer closedBuffer;
// Don't open the buffer to simulate error condition
QXmlStreamWriter writer(&closedBuffer);

bool result = profile->toXml(writer);
EXPECT_FALSE(result);
}

TEST_F(US_AnaProfileTest, FromXmlHandlesIncompleteXML) {
QString incompleteXml = R"DELIM(
<AnalysisProfileData version="1.0">
    <analysis_profile name="IncompleteProfile">
        <channel_parms channel="1A"
    )DELIM"; // Missing closing tags

US_AnaProfile testProfile;
EXPECT_FALSE(profileFromXmlString(testProfile, incompleteXml));
}

TEST_F(US_AnaProfileTest, FromXmlHandlesInvalidAttributeValues) {
QString invalidXml = R"DELIM(
<AnalysisProfileData version="1.0">
    <analysis_profile name="InvalidProfile" guid="12345678-1234-1234-1234-123456789012">
        <channel_parms channel="1A" chandesc="1A:UV/vis.:(unspecified)"
            load_concen_ratio="not_a_number" lcr_tolerance="5.0" load_volume="460.0"
            lv_tolerance="10.0" data_end="7.0" run="1"
            wvl_edit="180" wvl_not_run="" chandesc_alt="1A:UV/vis.:(unspecified)"/>
    </analysis_profile>
</AnalysisProfileData>
    )DELIM";

US_AnaProfile testProfile;
EXPECT_TRUE(profileFromXmlString(testProfile, invalidXml)); // Should still parse
// Invalid number should convert to 0.0
EXPECT_THAT(testProfile.lc_ratios[0], DoubleEq(0.0));
}

// =============================================================================
// Data Validation Tests
// =============================================================================

TEST_F(US_AnaProfileTest, ValidatesChannelDataConsistency) {
// All channel-related lists should have the same size
EXPECT_EQ(profile->pchans.size(), profile->chndescs.size());
EXPECT_EQ(profile->pchans.size(), profile->lc_ratios.size());
EXPECT_EQ(profile->pchans.size(), profile->l_volumes.size());
EXPECT_EQ(profile->pchans.size(), profile->analysis_run.size());
}

TEST_F(US_AnaProfileTest, ValidatesNumericRanges) {
// Check that default values are within reasonable ranges
EXPECT_GT(profile->lc_ratios[0], 0.0);
EXPECT_GT(profile->lc_tolers[0], 0.0);
EXPECT_GT(profile->l_volumes[0], 0.0);
EXPECT_GT(profile->lv_tolers[0], 0.0);
EXPECT_GT(profile->data_ends[0], 0.0);
EXPECT_GT(profile->ld_dens_0s[0], 0.0);
EXPECT_GT(profile->gm_vbars[0], 0.0);
EXPECT_GT(profile->gm_mws[0], 0.0);
}

TEST_F(US_AnaProfileTest, ValidatesWavelengthData) {
EXPECT_GT(profile->wvl_edit[0], 0);
EXPECT_LT(profile->wvl_edit[0], 1000); // Reasonable wavelength range
}

// =============================================================================
// JSON Validation Tests
// =============================================================================

TEST_F(US_AnaProfileTest, ValidatesReportMaskJSONStructure) {
QJsonParseError error;
QJsonDocument doc = QJsonDocument::fromJson(profile->report_mask.toUtf8(), &error);

EXPECT_EQ(error.error, QJsonParseError::NoError);
EXPECT_TRUE(doc.isObject());

QJsonObject obj = doc.object();
EXPECT_TRUE(obj.contains("A. General Settings"));
EXPECT_TRUE(obj.contains("J. Analysis Profile"));
}

TEST_F(US_AnaProfileTest, ValidatesCombPlotsParmsJSONStructure) {
QJsonParseError error;
QJsonDocument doc = QJsonDocument::fromJson(profile->combPlots_parms.toUtf8(), &error);

EXPECT_EQ(error.error, QJsonParseError::NoError);
EXPECT_TRUE(doc.isObject());

QJsonObject obj = doc.object();
EXPECT_TRUE(obj.contains("s,2DSA-IT"));
EXPECT_TRUE(obj.contains("MW,PCSA"));
EXPECT_TRUE(obj.contains("Radius,raw"));
}

} // anonymous namespace