#include "qt_test_base.h"
#include "../../programs/us_xpn_viewer/us_xpn_export.h"

TEST(XpnExport, LaterSuccessPreservesEarlierOpticsFailures)
{
    QStringList lastErrors;
    QVector<int> visited;
    const auto result = us_xpn_export_optics(QVector<int>{1, 0}, [&](int optics) {
        visited << optics;
        lastErrors.clear();
        if (optics == 1)
            lastErrors << "run.RA.1.A.280.auc: write failed\nDisk full";
        return US_XpnExportResult{optics == 1 ? 2 : 3, lastErrors};
    });

    EXPECT_EQ(visited, (QVector<int>{1, 0}));
    EXPECT_TRUE(lastErrors.isEmpty());
    EXPECT_EQ(result.files, 5);
    ASSERT_EQ(result.errors.size(), 1);
    EXPECT_EQ(result.errors.first(), "run.RA.1.A.280.auc: write failed\nDisk full");
}

TEST(XpnExport, CollectsFailuresFromEveryOpticalSystem)
{
    const auto result = us_xpn_export_optics(QVector<int>{0, 1}, [](int optics) {
        return US_XpnExportResult{0, {QString("failed optics %1").arg(optics)}};
    });
    EXPECT_EQ(result.files, 0);
    EXPECT_EQ(result.errors, (QStringList{"failed optics 0", "failed optics 1"}));
}
