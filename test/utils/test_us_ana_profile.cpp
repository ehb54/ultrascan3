// test_us_ana_profile.cpp
#include "test_us_ana_profile.h"
#include "us_ana_profile.h"
#include <QXmlStreamWriter>
#include <QBuffer>
#include <QXmlStreamReader>

// Use your custom Qt matchers
using namespace qt_matchers;

// Convert Qt Test methods to Google Test
TEST_F(TestUSAnaProfile, Constructor) {
    US_AnaProfile profile;

    // Qt Test → Google Test conversions:
    // QCOMPARE(a, b) → EXPECT_EQ(a, b)
    // QVERIFY(condition) → EXPECT_TRUE(condition)

    EXPECT_EQ(profile.aprofname, QString(""));
    EXPECT_EQ(profile.aprofGUID, QString("00000000-0000-0000-0000-000000000000"));
    EXPECT_EQ(profile.protoname, QString(""));
    EXPECT_EQ(profile.protoGUID, QString("00000000-0000-0000-0000-000000000000"));

    EXPECT_EQ(profile.pchans.size(), 1);
    EXPECT_EQ(profile.pchans[0], QString("1A"));

    EXPECT_EQ(profile.chndescs.size(), 1);
    EXPECT_EQ(profile.chndescs[0], QString("1A:UV/vis.:(unspecified)"));

    EXPECT_EQ(profile.lc_ratios.size(), 1);
    EXPECT_EQ(profile.lc_ratios[0], 1.0);

    EXPECT_EQ(profile.lc_tolers.size(), 1);
    EXPECT_EQ(profile.lc_tolers[0], 5.0);

    EXPECT_EQ(profile.l_volumes.size(), 1);
    EXPECT_EQ(profile.l_volumes[0], 460.0);

    EXPECT_EQ(profile.lv_tolers.size(), 1);
    EXPECT_EQ(profile.lv_tolers[0], 10.0);

    EXPECT_EQ(profile.data_ends.size(), 1);
    EXPECT_EQ(profile.data_ends[0], 7.0);

    EXPECT_EQ(profile.ld_dens_0s.size(), 1);
    EXPECT_EQ(profile.ld_dens_0s[0], 1.42);

    EXPECT_EQ(profile.gm_vbars.size(), 1);
    EXPECT_EQ(profile.gm_vbars[0], 0.2661);

    EXPECT_EQ(profile.gm_mws.size(), 1);
    EXPECT_EQ(profile.gm_mws[0], 168.36);

    EXPECT_EQ(profile.ref_channels.size(), 1);
    EXPECT_EQ(profile.ref_channels[0], 0);

    EXPECT_EQ(profile.ref_use_channels.size(), 1);
    EXPECT_EQ(profile.ref_use_channels[0], 0);

    EXPECT_EQ(profile.analysis_run.size(), 1);
    EXPECT_EQ(profile.analysis_run[0], 1);

    EXPECT_EQ(profile.report_run.size(), 1);
    EXPECT_EQ(profile.report_run[0], 1);

    EXPECT_EQ(profile.scan_excl_begin.size(), 1);
    EXPECT_EQ(profile.scan_excl_begin[0], 0);

    EXPECT_EQ(profile.scan_excl_end.size(), 1);
    EXPECT_EQ(profile.scan_excl_end[0], 0);

    EXPECT_EQ(profile.replicates.size(), 1);
    EXPECT_EQ(profile.replicates[0], 0);

    EXPECT_EQ(profile.wvl_edit.size(), 1);
    EXPECT_EQ(profile.wvl_edit[0], 180);

    EXPECT_EQ(profile.wvl_not_run.size(), 1);
    EXPECT_EQ(profile.wvl_not_run[0], QString(""));

    EXPECT_FALSE(profile.report_mask.isEmpty());
    EXPECT_FALSE(profile.combPlots_parms.isEmpty());
}

TEST_F(TestUSAnaProfile, EqualityOperator) {
    US_AnaProfile profile1;
    US_AnaProfile profile2;

    // Test initial equality
    EXPECT_TRUE(profile1 == profile2);

    // Test inequality after change
    profile1.aprofname = "Profile 1";
    EXPECT_FALSE(profile1 == profile2);

    // Test equality after matching change
    profile2.aprofname = "Profile 1";
    EXPECT_TRUE(profile1 == profile2);

    // Test inequality with different field
    profile1.l_volumes[0] = 500.0;
    EXPECT_FALSE(profile1 == profile2);
}

TEST_F(TestUSAnaProfile, ToXml) {
    US_AnaProfile profile;
    profile.aprofname = "TestProfile";
    profile.aprofGUID = "12345678-1234-1234-1234-123456789012";
    profile.pchans[0] = "2A";
    profile.chndescs[0] = "2A:UV/vis.:(modified)";

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QXmlStreamWriter xmlWriter(&buffer);

    EXPECT_TRUE(profile.toXml(xmlWriter));
    buffer.close();

    QString xmlContent = buffer.data();

    // Using your custom Qt matchers for cleaner assertions
    EXPECT_THAT(xmlContent, QStringContains(QString("<analysis_profile name=\"TestProfile\" guid=\"12345678-1234-1234-1234-123456789012\"")));
    EXPECT_THAT(xmlContent, QStringContains(QString("<channel_parms channel=\"2A\" chandesc=\"2A:UV/vis.:(modified)\"")));

    // Alternative: traditional Google Test assertions
    EXPECT_TRUE(xmlContent.contains("<analysis_profile name=\"TestProfile\" guid=\"12345678-1234-1234-1234-123456789012\""));
    EXPECT_TRUE(xmlContent.contains("<channel_parms channel=\"2A\" chandesc=\"2A:UV/vis.:(modified)\""));
}

TEST_F(TestUSAnaProfile, FromXml) {
    QString xmlContent = R"DELIM(
<AnalysisProfileData version="1.0">
    <analysis_profile name="TestProfile" guid="12345678-1234-1234-1234-123456789012">
        <channel_parms channel="1A" chandesc="1A:UV/vis.:(unspecified)"
    load_concen_ratio="1.0" lcr_tolerance="5.0" load_volume="460.0"
    lv_tolerance="10.0" data_end="7.0" load_dens="1.42"
    grad_vbar="0.2661" grad_mw="168.36" run="1" run_report="1"
    abde_reference="0" abde_use_reference="0" wvl_edit="180"
    wvl_not_run="" chandesc_alt="1A:UV/vis.:(unspecified)"
    scan_excl_begin="0" scan_excl_end="0" replicate_group="0"
    report_id="" report_guid=""/>
                             </analysis_profile>
                               </AnalysisProfileData>
    )DELIM";

    QBuffer buffer;
    buffer.setData(xmlContent.toUtf8());
    buffer.open(QIODevice::ReadOnly);
    QXmlStreamReader xmlReader(&buffer);

    US_AnaProfile profile;
    EXPECT_TRUE(profile.fromXml(xmlReader));
    buffer.close();

    // Verify parsed values
    EXPECT_EQ(profile.aprofname, QString("TestProfile"));
    EXPECT_EQ(profile.aprofGUID, QString("12345678-1234-1234-1234-123456789012"));

    EXPECT_EQ(profile.pchans.size(), 1);
    EXPECT_EQ(profile.pchans[0], QString("1A"));

    EXPECT_EQ(profile.chndescs.size(), 1);
    EXPECT_EQ(profile.chndescs[0], QString("1A:UV/vis.:(unspecified)"));

    EXPECT_EQ(profile.lc_ratios.size(), 1);
    EXPECT_EQ(profile.lc_ratios[0], 1.0);

    EXPECT_EQ(profile.lc_tolers.size(), 1);
    EXPECT_EQ(profile.lc_tolers[0], 5.0);

    EXPECT_EQ(profile.l_volumes.size(), 1);
    EXPECT_EQ(profile.l_volumes[0], 460.0);

    EXPECT_EQ(profile.lv_tolers.size(), 1);
    EXPECT_EQ(profile.lv_tolers[0], 10.0);

    EXPECT_EQ(profile.data_ends.size(), 1);
    EXPECT_EQ(profile.data_ends[0], 7.0);

    EXPECT_EQ(profile.ld_dens_0s.size(), 1);
    EXPECT_EQ(profile.ld_dens_0s[0], 1.42);

    EXPECT_EQ(profile.gm_vbars.size(), 1);
    EXPECT_EQ(profile.gm_vbars[0], 0.2661);

    EXPECT_EQ(profile.gm_mws.size(), 1);
    EXPECT_EQ(profile.gm_mws[0], 168.36);

    EXPECT_EQ(profile.ref_channels.size(), 1);
    EXPECT_EQ(profile.ref_channels[0], 0);

    EXPECT_EQ(profile.ref_use_channels.size(), 1);
    EXPECT_EQ(profile.ref_use_channels[0], 0);

    EXPECT_EQ(profile.analysis_run.size(), 1);
    EXPECT_EQ(profile.analysis_run[0], 1);

    EXPECT_EQ(profile.report_run.size(), 1);
    EXPECT_EQ(profile.report_run[0], 1);

    EXPECT_EQ(profile.scan_excl_begin.size(), 1);
    EXPECT_EQ(profile.scan_excl_begin[0], 0);

    EXPECT_EQ(profile.scan_excl_end.size(), 1);
    EXPECT_EQ(profile.scan_excl_end[0], 0);

    EXPECT_EQ(profile.replicates.size(), 1);
    EXPECT_EQ(profile.replicates[0], 0);

    EXPECT_EQ(profile.wvl_edit.size(), 1);
    EXPECT_EQ(profile.wvl_edit[0], 180);

    EXPECT_EQ(profile.wvl_not_run.size(), 1);
    EXPECT_EQ(profile.wvl_not_run[0], QString(""));
}