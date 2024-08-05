#include "test_us_ana_profile.h"  // Include the header file
#include "us_ana_profile.h"
#include <QXmlStreamWriter>
#include <QBuffer>
#include <QtTest/QtTest>

void TestUSAnaProfile::testConstructor()
{
    US_AnaProfile profile;
    QCOMPARE(profile.aprofname, QString(""));
    QCOMPARE(profile.aprofGUID, QString("00000000-0000-0000-0000-000000000000"));
    QCOMPARE(profile.protoname, QString(""));
    QCOMPARE(profile.protoGUID, QString("00000000-0000-0000-0000-000000000000"));
    QCOMPARE(profile.pchans.size(), 1);
    QCOMPARE(profile.pchans[0], QString("1A"));
    QCOMPARE(profile.chndescs.size(), 1);
    QCOMPARE(profile.chndescs[0], QString("1A:UV/vis.:(unspecified)"));
    QCOMPARE(profile.lc_ratios.size(), 1);
    QCOMPARE(profile.lc_ratios[0], 1.0);
    QCOMPARE(profile.lc_tolers.size(), 1);
    QCOMPARE(profile.lc_tolers[0], 5.0);
    QCOMPARE(profile.l_volumes.size(), 1);
    QCOMPARE(profile.l_volumes[0], 460.0);
    QCOMPARE(profile.lv_tolers.size(), 1);
    QCOMPARE(profile.lv_tolers[0], 10.0);
    QCOMPARE(profile.data_ends.size(), 1);
    QCOMPARE(profile.data_ends[0], 7.0);
    QCOMPARE(profile.ld_dens_0s.size(), 1);
    QCOMPARE(profile.ld_dens_0s[0], 1.42);
    QCOMPARE(profile.gm_vbars.size(), 1);
    QCOMPARE(profile.gm_vbars[0], 0.2661);
    QCOMPARE(profile.gm_mws.size(), 1);
    QCOMPARE(profile.gm_mws[0], 168.36);
    QCOMPARE(profile.ref_channels.size(), 1);
    QCOMPARE(profile.ref_channels[0], 0);
    QCOMPARE(profile.ref_use_channels.size(), 1);
    QCOMPARE(profile.ref_use_channels[0], 0);
    QCOMPARE(profile.analysis_run.size(), 1);
    QCOMPARE(profile.analysis_run[0], 1);
    QCOMPARE(profile.report_run.size(), 1);
    QCOMPARE(profile.report_run[0], 1);
    QCOMPARE(profile.scan_excl_begin.size(), 1);
    QCOMPARE(profile.scan_excl_begin[0], 0);
    QCOMPARE(profile.scan_excl_end.size(), 1);
    QCOMPARE(profile.scan_excl_end[0], 0);
    QCOMPARE(profile.replicates.size(), 1);
    QCOMPARE(profile.replicates[0], 0);
    QCOMPARE(profile.wvl_edit.size(), 1);
    QCOMPARE(profile.wvl_edit[0], 180);
    QCOMPARE(profile.wvl_not_run.size(), 1);
    QCOMPARE(profile.wvl_not_run[0], QString(""));
    QCOMPARE(profile.report_mask.isEmpty(), false);
    QCOMPARE(profile.combPlots_parms.isEmpty(), false);
}

void TestUSAnaProfile::testEqualityOperator()
{
    US_AnaProfile profile1;
    US_AnaProfile profile2;

    QVERIFY(profile1 == profile2);

    profile1.aprofname = "Profile 1";
    QVERIFY(!(profile1 == profile2));

    profile2.aprofname = "Profile 1";
    QVERIFY(profile1 == profile2);

    profile1.l_volumes[0] = 500.0;
    QVERIFY(!(profile1 == profile2));
}

void TestUSAnaProfile::testToXml()
{
    US_AnaProfile profile;
    profile.aprofname = "TestProfile";
    profile.aprofGUID = "12345678-1234-1234-1234-123456789012";
    profile.pchans[0] = "2A";
    profile.chndescs[0] = "2A:UV/vis.:(modified)";

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(profile.toXml(xmlWriter));
    buffer.close();

    QString xmlContent = buffer.data();
    QVERIFY(xmlContent.contains("<analysis_profile name=\"TestProfile\" guid=\"12345678-1234-1234-1234-123456789012\""));
    QVERIFY(xmlContent.contains("<channel_parms channel=\"2A\" chandesc=\"2A:UV/vis.:(modified)\""));
}

void TestUSAnaProfile::testFromXml()
{
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
    QVERIFY(profile.fromXml(xmlReader));
    buffer.close();

    QCOMPARE(profile.aprofname, QString("TestProfile"));
    QCOMPARE(profile.aprofGUID, QString("12345678-1234-1234-1234-123456789012"));
    QCOMPARE(profile.pchans.size(), 1);
    QCOMPARE(profile.pchans[0], QString("1A"));
    QCOMPARE(profile.chndescs.size(), 1);
    QCOMPARE(profile.chndescs[0], QString("1A:UV/vis.:(unspecified)"));
    QCOMPARE(profile.lc_ratios.size(), 1);
    QCOMPARE(profile.lc_ratios[0], 1.0);
    QCOMPARE(profile.lc_tolers.size(), 1);
    QCOMPARE(profile.lc_tolers[0], 5.0);
    QCOMPARE(profile.l_volumes.size(), 1);
    QCOMPARE(profile.l_volumes[0], 460.0);
    QCOMPARE(profile.lv_tolers.size(), 1);
    QCOMPARE(profile.lv_tolers[0], 10.0);
    QCOMPARE(profile.data_ends.size(), 1);
    QCOMPARE(profile.data_ends[0], 7.0);
    QCOMPARE(profile.ld_dens_0s.size(), 1);
    QCOMPARE(profile.ld_dens_0s[0], 1.42);
    QCOMPARE(profile.gm_vbars.size(), 1);
    QCOMPARE(profile.gm_vbars[0], 0.2661);
    QCOMPARE(profile.gm_mws.size(), 1);
    QCOMPARE(profile.gm_mws[0], 168.36);
    QCOMPARE(profile.ref_channels.size(), 1);
    QCOMPARE(profile.ref_channels[0], 0);
    QCOMPARE(profile.ref_use_channels.size(), 1);
    QCOMPARE(profile.ref_use_channels[0], 0);
    QCOMPARE(profile.analysis_run.size(), 1);
    QCOMPARE(profile.analysis_run[0], 1);
    QCOMPARE(profile.report_run.size(), 1);
    QCOMPARE(profile.report_run[0], 1);
    QCOMPARE(profile.scan_excl_begin.size(), 1);
    QCOMPARE(profile.scan_excl_begin[0], 0);
    QCOMPARE(profile.scan_excl_end.size(), 1);
    QCOMPARE(profile.scan_excl_end[0], 0);
    QCOMPARE(profile.replicates.size(), 1);
    QCOMPARE(profile.replicates[0], 0);
    QCOMPARE(profile.wvl_edit.size(), 1);
    QCOMPARE(profile.wvl_edit[0], 180);
    QCOMPARE(profile.wvl_not_run.size(), 1);
    QCOMPARE(profile.wvl_not_run[0], QString(""));
}