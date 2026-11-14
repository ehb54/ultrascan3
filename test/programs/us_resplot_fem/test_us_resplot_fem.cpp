#include "us3_gui_test_main.h"
#include "owner_doubles.h"
#include "us_resplot_fem.h"
#include <qwt_plot_curve.h>
#include <QSignalSpy>
#include <cmath>
#include <memory>
#include <vector>

namespace
{
// Binary-exact inputs make arithmetic comparisons independent of decimal parsing.
const double differences[3][4] = {
    { .25, -.5, .75, -1. }, { .5, -.25, 1., -.75 }, { .75, -1., .25, -.5 }
};
const double tiValues[4] = { .125, -.25, .375, -.5 };
const double riValues[3] = { .0625, -.125, .1875 };

void populate(ResidualInputs& in, int noise = 3, int exclusions = 0)
{
    in.edited.xvalues = { 5.8, 5.9, 6.0, 6.1 };
    in.simulated.xvalues = in.edited.xvalues;
    in.edited.wavelength = "280";
    for (int scan = 0; scan < 3; ++scan)
    {
        US_DataIO::Scan e{}, s{};
        for (int point = 0; point < 4; ++point)
        {
            s.rvalues << double(1 + scan + point);
            e.rvalues << double(1 + scan + point) + differences[scan][point];
        }
        in.edited.scanData << e;
        in.simulated.scanData << s;
    }
    in.ti.count = noise & 1 ? 4 : 0;
    in.ri.count = noise & 2 ? 3 : 0;
    if (in.ti.count) for (double v : tiValues) in.ti.values << v;
    if (in.ri.count) for (double v : riValues) in.ri.values << v;
    if (exclusions >= 1 && exclusions <= 3) in.excluded << exclusions - 1;
    if (exclusions == 4) in.excluded = { 0, 1, 2 };
}

std::unique_ptr<QWidget> owner(ResidualInputs& in, const QString& mode)
{
    if (mode == "ANALYSIS") return std::unique_ptr<QWidget>(new US_Analysis_auto(in));
    if (mode == "REPORT") return std::unique_ptr<QWidget>(new US_ReporterGMP(in));
    return std::unique_ptr<QWidget>(new US_FeMatch(in));
}

QCheckBox* box(US_ResidPlotFem& plot, const QString& text)
{
    for (auto* b : plot.findChildren<QCheckBox*>()) if (b->text() == text) return b;
    qFatal("Missing checkbox: %s", qPrintable(text));
    return nullptr;
}

QLineEdit* field(US_ResidPlotFem& plot, const QString& name)
{
    for (auto* f : plot.findChildren<QLineEdit*>())
        if (f->accessibleName() == name) return f;
    qFatal("Missing field: %s", qPrintable(name));
    return nullptr;
}

QList<QwtPlotCurve*> curves(QwtPlot* plot)
{
    QList<QwtPlotCurve*> result;
    for (auto* item : plot->itemList(QwtPlotItem::Rtti_PlotCurve))
        result << static_cast<QwtPlotCurve*>(item);
    return result;
}

void deferredDeletes()
{
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

QPointer<US_ResidsBitmap> bitmap(US_ResidPlotFem& plot)
{
    box(plot, "Show Residuals Bitmap")->setChecked(true);
    // REPORT's checkbox slot does nothing; a subsequent refresh creates it.
    QMetaObject::invokeMethod(&plot, "plot_data", Qt::DirectConnection);
    for (auto* w : QApplication::topLevelWidgets())
        if (auto* b = qobject_cast<US_ResidsBitmap*>(w)) return b;
    return nullptr;
}

void modeRows()
{
    QTest::addColumn<QString>("mode");
    QTest::newRow("fematch") << QString();
    QTest::newRow("analysis") << QString("ANALYSIS");
    QTest::newRow("report") << QString("REPORT");
}
}

class ResidualDialogTest : public QObject
{
    Q_OBJECT
private slots:
    void cleanup()
    {
        // Also isolate later rows if an assertion exits a bitmap test early.
        for (auto* w : QApplication::topLevelWidgets())
            if (auto* b = qobject_cast<US_ResidsBitmap*>(w)) delete b;
        deferredDeletes();
    }

    void curvesAndStatistics_data()
    {
        QTest::addColumn<QString>("mode");
        QTest::addColumn<int>("noise");
        QTest::addColumn<int>("exclusions");
        for (const QString& mode : { QString(), QString("ANALYSIS"), QString("REPORT") })
            for (int noise = 0; noise < 4; ++noise)
                for (int exclusions = 0; exclusions < 5; ++exclusions)
                    QTest::newRow(qPrintable(QString("%1-noise%2-exclude%3")
                        .arg(mode.isEmpty() ? "FEMATCH" : mode).arg(noise).arg(exclusions)))
                        << mode << noise << exclusions;
    }

    void curvesAndStatistics()
    {
        QFETCH(QString, mode); QFETCH(int, noise); QFETCH(int, exclusions);
        ResidualInputs in; populate(in, noise, exclusions);
        auto caller = owner(in, mode);
        US_ResidPlotFem plot(caller.get(), mode);
        auto upper = curves(plot.rp_data_plot1());
        auto lower = curves(plot.rp_data_plot2());
        const int included = 3 - in.excluded.size();
        QCOMPARE(upper.size(), 2 * included);
        QCOMPARE(lower.size(), 1 + included);
        QCOMPARE(lower[0]->title().text(), QString("zero-line"));
        QCOMPARE(lower[0]->dataSize(), size_t(2));
        QCOMPARE(lower[0]->sample(0), QPointF(5.8 - .02, 0.));
        QCOMPARE(lower[0]->sample(1), QPointF(6.1 + .02, 0.));
        double sumSquares = 0.;
        int k = 0;
        for (int scan = 0; scan < 3; ++scan)
        {
            if (in.excluded.contains(scan)) continue;
            QCOMPARE(upper[k]->title().text(), QString("Curve %1").arg(scan));
            QCOMPARE(upper[k + included]->title().text(), QString("S-Curve %1").arg(scan));
            QCOMPARE(lower[k + 1]->title().text(), QString("resids %1").arg(scan));
            QCOMPARE(lower[k + 1]->style(), QwtPlotCurve::Dots);
            QCOMPARE(upper[k]->dataSize(), size_t(4));
            QCOMPARE(upper[k + included]->dataSize(), size_t(4));
            QCOMPARE(lower[k + 1]->dataSize(), size_t(4));
            for (int point = 0; point < 4; ++point)
            {
                const double correction = (noise & 1 ? tiValues[point] : 0.)
                                        + (noise & 2 ? riValues[scan] : 0.);
                const double residual = differences[scan][point] - correction;
                const double simulation = 1 + scan + point;
                const double radius = in.edited.xvalues[point];
                QCOMPARE(upper[k]->sample(point), QPointF(radius, simulation + residual));
                QCOMPARE(upper[k + included]->sample(point), QPointF(radius, simulation));
                QCOMPARE(lower[k + 1]->sample(point), QPointF(radius, residual));
                sumSquares += residual * residual;
            }
            ++k;
        }
        if (included)
        {
            QCOMPARE(field(plot, "Variance")->text(), QString::number(sumSquares / (included * 4)));
            QCOMPARE(field(plot, "RMSD")->text(), QString::number(std::sqrt(sumSquares / (included * 4))));
        }
        else
        {
            // Characterize current zero-denominator output; this is not desired validation policy.
            QVERIFY(field(plot, "Variance")->text().toLower().contains("nan"));
            QVERIFY(field(plot, "RMSD")->text().toLower().contains("nan"));
        }
        QCOMPARE(box(plot, "Subtract Time Invariant Noise")->isEnabled(), bool(noise & 1));
        QCOMPARE(box(plot, "Subtract Radially Invariant Noise")->isEnabled(), bool(noise & 2));
        QCOMPARE(box(plot, "Subtract Time Invariant Noise")->isChecked(), bool(noise & 1));
        QCOMPARE(box(plot, "Subtract Radially Invariant Noise")->isChecked(), bool(noise & 2));
        QCOMPARE(plot.isVisible(), mode != "REPORT");
        QCOMPARE(plot.parentWidget(), static_cast<QWidget*>(nullptr));
        const QString suffix = mode == "ANALYSIS" ? " " + in.triple : mode == "REPORT" ? " " : "";
        QCOMPARE(plot.rp_data_plot1()->title().text(), "Experimental and Simulated Data" + suffix);
        QCOMPARE(plot.rp_data_plot1()->axisTitle(QwtPlot::yLeft).text(), QString("Absorbance at 280 nm"));
        QCOMPARE(plot.rp_data_plot1()->minimumWidth(), mode == "ANALYSIS" ? 670 : 560);
    }

    void noiseAndVisibilityControls_data() { modeRows(); }
    void noiseAndVisibilityControls()
    {
        QFETCH(QString, mode);
        ResidualInputs in; populate(in);
        auto caller = owner(in, mode);
        US_ResidPlotFem plot(caller.get(), mode);
        for (int cycle = 0; cycle < 2; ++cycle)
            for (int mask = 0; mask < 16; ++mask)
            {
                box(plot, "Subtract Time Invariant Noise")->setChecked(mask & 1);
                box(plot, "Subtract Radially Invariant Noise")->setChecked(mask & 2);
                box(plot, "Add Time Invariant Noise")->setChecked(mask & 4);
                box(plot, "Add Radially Invariant Noise")->setChecked(mask & 8);
                auto upper = curves(plot.rp_data_plot1());
                auto lower = curves(plot.rp_data_plot2());
                QCOMPARE(upper.size(), 6); QCOMPARE(lower.size(), 4);
                for (int s = 0; s < 3; ++s) for (int p = 0; p < 4; ++p)
                {
                    const double sub = (mask & 1 ? tiValues[p] : 0.) + (mask & 2 ? riValues[s] : 0.);
                    const double add = (mask & 4 ? tiValues[p] : 0.) + (mask & 8 ? riValues[s] : 0.);
                    QCOMPARE(upper[s]->sample(p).y(), 1 + s + p + differences[s][p] - sub);
                    QCOMPARE(upper[s + 3]->sample(p).y(), 1 + s + p + add);
                    QCOMPARE(lower[s + 1]->sample(p).y(), differences[s][p] - sub - add);
                }
            }
        box(plot, "Plot Experimental Data")->setChecked(false);
        QCOMPARE(curves(plot.rp_data_plot1()).size(), 3);
        QVERIFY(!box(plot, "Subtract Time Invariant Noise")->isEnabled());
        box(plot, "Plot Simulated/Modeled Data")->setChecked(false);
        QCOMPARE(curves(plot.rp_data_plot1()).size(), 0);
        QVERIFY(!box(plot, "Add Radially Invariant Noise")->isEnabled());
        box(plot, "Plot Experimental Data")->setChecked(true);
        box(plot, "Plot Simulated/Modeled Data")->setChecked(true);
        QCOMPARE(curves(plot.rp_data_plot1()).size(), 6);
    }

    void lowerPlotSelection_data() { modeRows(); }
    void lowerPlotSelection()
    {
        QFETCH(QString, mode);
        ResidualInputs in; populate(in);
        auto caller = owner(in, mode);
        US_ResidPlotFem plot(caller.get(), mode);
        plot.set_plot(1);
        auto cs = curves(plot.rp_data_plot2());
        QCOMPARE(cs.size(), 2); QCOMPARE(cs[1]->title().text(), QString("ti_noise"));
        for (int p = 0; p < 4; ++p) QCOMPARE(cs[1]->sample(p), QPointF(in.edited.xvalues[p], tiValues[p]));
        plot.set_plot(2);
        // D03: exporting the lower plot must preserve its scan-number axis.
        QwtPlot* lower = nullptr;
        for (auto* candidate : plot.findChildren<QwtPlot*>())
            if (candidate->accessibleName() == "Residuals Plot") lower = candidate;
        QVERIFY(lower);
        QCOMPARE(lower->axisTitle(QwtPlot::xBottom).text(), QString("Scan Number"));
        cs = curves(plot.rp_data_plot2());
        QCOMPARE(cs.size(), 2); QCOMPARE(cs[1]->title().text(), QString("ri_noise"));
        for (int s = 0; s < 3; ++s) QCOMPARE(cs[1]->sample(s), QPointF(s + 1, riValues[s]));
        QCOMPARE(plot.rp_data_plot2()->axisTitle(QwtPlot::xBottom).text(), QString("Scan Number"));
        box(plot, "Plot Random Noise")->setChecked(true);
        cs = curves(plot.rp_data_plot2());
        QCOMPARE(cs.size(), 4);
        for (int s = 0; s < 3; ++s)
        {
            QCOMPARE(cs[s + 1]->title().text(), QString("random noise %1").arg(s));
            for (int p = 0; p < 4; ++p)
                QCOMPARE(cs[s + 1]->sample(p).y(), differences[s][p] - tiValues[p] - riValues[s]);
        }
        plot.set_plot(0);
        QVERIFY(box(plot, "Plot Residuals")->isChecked());
        QVERIFY(!box(plot, "Plot Random Noise")->isChecked());
        QCOMPARE(curves(plot.rp_data_plot2())[1]->title().text(), QString("resids 0"));
    }

    void closeAndReopen_data() { modeRows(); }
    void closeAndReopen()
    {
        QFETCH(QString, mode);
        ResidualInputs in; populate(in);
        auto caller = owner(in, mode);
        std::unique_ptr<US_ResidPlotFem> plot(new US_ResidPlotFem(caller.get(), mode));
        QPointer<US_ResidPlotFem> observed(plot.get());
        QSignalSpy closed(plot.get(), &US_ResidPlotFem::on_close);
        QSignalSpy destroyed(plot.get(), &QObject::destroyed);
        auto bm = bitmap(*plot);
        QVERIFY(bm);
        QCOMPARE(bm->parentWidget(), mode == "ANALYSIS" ? plot.get() : nullptr);
        QVERIFY(bm->testAttribute(Qt::WA_DeleteOnClose));
        QVERIFY(!plot->testAttribute(Qt::WA_DeleteOnClose));
        QVERIFY(!in.bitmap); // Newly created bitmap is not published back to caller.
        bm->close(); deferredDeletes();
        QVERIFY(!bm);
        QCOMPARE(box(*plot, "Show Residuals Bitmap")->isChecked(), mode == "REPORT");
        bm = bitmap(*plot); QVERIFY(bm);
        plot->show(); plot->close(); deferredDeletes();
        QCOMPARE(closed.count(), 1); QCOMPARE(destroyed.count(), 0);
        QVERIFY(observed); QVERIFY(!plot->isVisible());
        QVERIFY(bm); // Window-manager-style close does not run close_all.
        plot->show();
        QVERIFY(plot->isVisible());
        QVERIFY(QMetaObject::invokeMethod(plot.get(), "close_all", Qt::DirectConnection));
        deferredDeletes();
        QVERIFY(!bm); QCOMPARE(closed.count(), 2); QCOMPARE(destroyed.count(), 0);
        caller.reset(); // No Qt parent relationship; do not access caller data afterwards.
        QVERIFY(observed);
        plot.reset();
        QVERIFY(!observed); QCOMPARE(destroyed.count(), 1);
    }

    void borrowedBitmapAndRefresh()
    {
        ResidualInputs in; populate(in);
        QVector<QVector<double>> values = {{ .1, .2 }, { -.1, -.2 }};
        in.bitmap = new US_ResidsBitmap(values);
        QPointer<US_ResidsBitmap> original(in.bitmap);
        auto caller = owner(in, "");
        US_ResidPlotFem plot(caller.get());
        QCOMPARE(bitmap(plot), original);
        in.edited.scanData[0].rvalues[0] += .5;
        in.excluded << 1;
        QVERIFY(QMetaObject::invokeMethod(&plot, "plot_data", Qt::DirectConnection));
        auto cs = curves(plot.rp_data_plot2());
        QCOMPARE(cs.size(), 3);
        QCOMPARE(cs[1]->sample(0).y(), .25 + .5 - .125 - .0625);
        QCOMPARE(cs[2]->title().text(), QString("resids 2"));
        original->close(); deferredDeletes();
        QVERIFY(!original); QVERIFY(!in.bitmap);
        QVERIFY(!box(plot, "Show Residuals Bitmap")->isChecked());
    }

    void repeatedCloseRetainsDialogsUntilExplicitDeletion_data() { modeRows(); }
    void repeatedCloseRetainsDialogsUntilExplicitDeletion()
    {
        QFETCH(QString, mode);
        ResidualInputs in; populate(in);
        auto caller = owner(in, mode);
        int destroyed = 0;
        std::vector<std::unique_ptr<US_ResidPlotFem>> dialogs;
        QList<QPointer<US_ResidPlotFem>> observed;
        // Characterize the retention defect, not a desired ownership policy.
        // Explicit cleanup keeps the test itself from leaking the 100 dialogs.
        for (int i = 0; i < 100; ++i)
        {
            dialogs.emplace_back(new US_ResidPlotFem(caller.get(), mode));
            observed << dialogs.back().get();
            connect(dialogs.back().get(), &QObject::destroyed, this, [&destroyed]() { ++destroyed; });
            dialogs.back()->close();
        }
        deferredDeletes();
        QCOMPARE(destroyed, 0);
        for (const auto& dialog : observed) QVERIFY(dialog && !dialog->isVisible());
        dialogs.clear();
        QCOMPARE(destroyed, 100);
        for (const auto& dialog : observed) QVERIFY(!dialog);
    }
};

US3_GUI_TEST_MAIN(ResidualDialogTest)
#include "test_us_resplot_fem.moc"
