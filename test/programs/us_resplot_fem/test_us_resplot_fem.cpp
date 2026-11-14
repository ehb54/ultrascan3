#include "us3_gui_test_main.h"
#include "us_resplot_fem.h"
#include <qwt_plot_curve.h>
#include <QSignalSpy>
#include <QProcess>
#include <limits>
#include <cmath>
#include <memory>
#include <vector>

namespace
{
#if defined(__SANITIZE_ADDRESS__)
constexpr bool sanitized = true;
#elif defined(__has_feature)
#if __has_feature(address_sanitizer)
constexpr bool sanitized = true;
#else
constexpr bool sanitized = false;
#endif
#else
constexpr bool sanitized = false;
#endif
// Binary-exact inputs make arithmetic comparisons independent of decimal parsing.
const double differences[3][4] = {
    { .25, -.5, .75, -1. }, { .5, -.25, 1., -.75 }, { .75, -1., .25, -.5 }
};
const double tiValues[4] = { .125, -.25, .375, -.5 };
const double riValues[3] = { .0625, -.125, .1875 };

// Storage the dialog borrows through US_ResidPlotInputs.
struct ResidualInputs
{
    US_DataIO::EditedData edited;
    US_DataIO::RawData simulated;
    QList<int> excluded;
    US_Noise ti, ri;
    QPointer<US_ResidsBitmap> bitmap;
    QString triple = "1 / A / 280";
};

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

// The calling widget each program passes as the dialog's parent argument.
std::unique_ptr<QWidget> owner(ResidualInputs&, const QString&)
{
    return std::unique_ptr<QWidget>(new QWidget);
}

// The inputs each program builds from its own members.
US_ResidPlotInputs inputs(ResidualInputs& in)
{
    US_ResidPlotInputs result;
    result.edited = &in.edited;
    result.simulated = &in.simulated;
    result.excluded = &in.excluded;
    result.ti_noise = &in.ti;
    result.ri_noise = &in.ri;
    result.bitmap = in.bitmap;
    result.triple_info = in.triple;
    return result;
}

US_ResidPlotFem::Mode toMode(const QString& mode)
{
    if (mode == "ANALYSIS") return US_ResidPlotFem::Analysis;
    if (mode == "REPORT") return US_ResidPlotFem::Report;
    return US_ResidPlotFem::Standalone;
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
    void invalidInputs_data()
    {
        QTest::addColumn<QString>("mode");
        QTest::addColumn<QString>("scenario");
        for (const QString& mode : { QString(), QString("ANALYSIS"), QString("REPORT") })
            // The inputs no longer depend on the parent or on a mode string, so an
            // unknown mode cannot be expressed and a null parent is an ordinary case.
            for (const QString& scenario : { QString("null-parent"),
                  QString("empty-edited"), QString("empty-simulated"), QString("short-ti"),
                  QString("short-ri"), QString("short-scan"), QString("short-radius"),
                  QString("nonfinite"), QString("bad-exclusion") })
                QTest::newRow(qPrintable((mode.isEmpty() ? "FEMATCH" : mode) + "-" + scenario))
                    << mode << scenario;
    }

    void invalidInputs()
    {
        QFETCH(QString, mode); QFETCH(QString, scenario);
        const bool unsafe = scenario != "nonfinite" && scenario != "bad-exclusion" &&
                            scenario != "null-parent";
        if (unsafe && !sanitized)
            QSKIP("Unsafe legacy input requires the ASan/UBSan build; never execute UB in the parent.");
        QProcess process;
        auto environment = QProcessEnvironment::systemEnvironment();
        environment.insert("US_RESIDUAL_INVALID_CASE", scenario);
        environment.insert("US_RESIDUAL_INVALID_MODE", mode);
        environment.insert("QT_LOGGING_RULES", "*.debug=false");
        environment.insert("ASAN_OPTIONS", "detect_leaks=0:halt_on_error=1");
        environment.insert("UBSAN_OPTIONS", "halt_on_error=1:print_stacktrace=1");
        process.setProcessEnvironment(environment);
        process.setProcessChannelMode(QProcess::MergedChannels);
        process.start(QCoreApplication::applicationFilePath(), { "invalidInputProbe" });
        QVERIFY(process.waitForStarted(5000));
        if (!process.waitForFinished(20000))
        {
            process.kill(); process.waitForFinished();
            QFAIL("Invalid-input child timed out; a hang is not an accepted baseline result.");
        }
        const QByteArray output = process.readAll();
        const QString logDir = qEnvironmentVariable("US_RESIDUAL_PROBE_LOG_DIR");
        if (!logDir.isEmpty())
        {
            QVERIFY(QDir().mkpath(logDir));
            QFile log(QDir(logDir).filePath(QString::fromLatin1(QTest::currentDataTag()) + ".log"));
            QVERIFY(log.open(QIODevice::WriteOnly));
            log.write(output);
        }
        // Prove we reached the probe, rather than accepting a loader/ASan startup failure.
        QVERIFY2(output.contains("RESIDUAL_PROBE_STARTED"), output.constData());
        if (unsafe)
        {
            // D04: characterize existing unsafe rejection by crash/sanitizer. This
            // is NOT a passing safety contract; replace with safe-rejection tests before repair.
            QVERIFY2(process.exitStatus() == QProcess::CrashExit || process.exitCode() != 0, output.constData());
            QVERIFY2(output.contains("AddressSanitizer") || output.contains("runtime error:") ||
                     output.contains("ASSERT"), output.constData());
        }
        else
        {
            QCOMPARE(process.exitStatus(), QProcess::NormalExit);
            QVERIFY2(process.exitCode() == 0, output.constData());
            QVERIFY(output.contains("RESIDUAL_PROBE_COMPLETED"));
        }
    }

    void invalidInputProbe()
    {
        const QString scenario = qEnvironmentVariable("US_RESIDUAL_INVALID_CASE");
        if (scenario.isEmpty()) QSKIP("Child-process entry point, exercised by invalidInputs rows.");
        const QString mode = qEnvironmentVariable("US_RESIDUAL_INVALID_MODE");
        ResidualInputs in; populate(in);
        auto caller = owner(in, mode);
        if (scenario == "empty-edited") { in.edited.xvalues.clear(); in.edited.scanData.clear(); }
        if (scenario == "empty-simulated") { in.simulated.xvalues.clear(); in.simulated.scanData.clear(); }
        if (scenario == "short-ti") in.ti.values = QVector<double>{ .125 };
        if (scenario == "short-ri") in.ri.values = QVector<double>{ .0625 };
        if (scenario == "short-scan") in.simulated.scanData = QVector<US_DataIO::Scan>{ in.simulated.scanData[0] };
        if (scenario == "short-radius") in.simulated.xvalues = QVector<double>{ 5.8, 5.9 };
        if (scenario == "nonfinite") in.edited.scanData[0].rvalues[0] = std::numeric_limits<double>::quiet_NaN();
        if (scenario == "bad-exclusion") in.excluded = { -1, 99 };
        fprintf(stderr, "RESIDUAL_PROBE_STARTED\n"); fflush(stderr);
        US_ResidPlotFem plot(inputs(in), toMode(mode),
                             scenario == "null-parent" ? nullptr : caller.get());
        auto cs = curves(plot.rp_data_plot2());
        if (scenario == "nonfinite")
        {
            QCOMPARE(cs.size(), 4);
            QVERIFY(std::isnan(cs[1]->sample(0).y()));
            QVERIFY(field(plot, "RMSD")->text().toLower().contains("nan"));
        }
        if (scenario == "bad-exclusion")
        {
            QCOMPARE(cs.size(), 4); // Out-of-range exclusions are silently ignored.
            QCOMPARE(cs[1]->sample(0).y(), .25 - .125 - .0625);
        }
        fprintf(stderr, "RESIDUAL_PROBE_COMPLETED\n"); fflush(stderr);
    }

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
        US_ResidPlotFem plot(inputs(in), toMode(mode), caller.get());
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
        QCOMPARE(plot.parentWidget(), caller.get()); // D01: the caller owns the dialog.
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
        US_ResidPlotFem plot(inputs(in), toMode(mode), caller.get());
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
        US_ResidPlotFem plot(inputs(in), toMode(mode), caller.get());
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

    void closeDestroysDialog_data() { modeRows(); }
    void closeDestroysDialog()
    {
        QFETCH(QString, mode);
        ResidualInputs in; populate(in);
        auto caller = owner(in, mode);
        QPointer<US_ResidPlotFem> plot(new US_ResidPlotFem(inputs(in), toMode(mode), caller.get()));
        QSignalSpy closed(plot.data(), &US_ResidPlotFem::on_close);
        QSignalSpy destroyed(plot.data(), &QObject::destroyed);
        // D01: the caller is the Qt parent and closing the dialog destroys it.
        QCOMPARE(plot->parentWidget(), caller.get());
        QVERIFY(plot->testAttribute(Qt::WA_DeleteOnClose));
        auto bm = bitmap(*plot);
        QVERIFY(bm);
        QCOMPARE(bm->parentWidget(), mode == "ANALYSIS" ? plot.data() : nullptr);
        QVERIFY(bm->testAttribute(Qt::WA_DeleteOnClose));
        QVERIFY(!in.bitmap); // Newly created bitmap is not published back to caller.
        bm->close(); deferredDeletes();
        QVERIFY(!bm);
        QCOMPARE(box(*plot, "Show Residuals Bitmap")->isChecked(), mode == "REPORT");
        bm = bitmap(*plot); QVERIFY(bm);
        plot->show(); plot->close(); deferredDeletes();
        QCOMPARE(closed.count(), 1); QCOMPARE(destroyed.count(), 1);
        QVERIFY(!plot);
        // Window close does not run close_all; only a child bitmap goes with the dialog.
        QCOMPARE(bool(bm), mode != "ANALYSIS");
    }

    void closeButtonDestroysDialogAndBitmap_data() { modeRows(); }
    void closeButtonDestroysDialogAndBitmap()
    {
        QFETCH(QString, mode);
        ResidualInputs in; populate(in);
        auto caller = owner(in, mode);
        QPointer<US_ResidPlotFem> plot(new US_ResidPlotFem(inputs(in), toMode(mode), caller.get()));
        QSignalSpy closed(plot.data(), &US_ResidPlotFem::on_close);
        auto bm = bitmap(*plot); QVERIFY(bm);
        plot->show();
        QVERIFY(QMetaObject::invokeMethod(plot.data(), "close_all", Qt::DirectConnection));
        deferredDeletes();
        QVERIFY(!bm); QVERIFY(!plot); QCOMPARE(closed.count(), 1);
    }

    void callerDestructionDestroysDialog_data() { modeRows(); }
    void callerDestructionDestroysDialog()
    {
        QFETCH(QString, mode);
        ResidualInputs in; populate(in);
        auto caller = owner(in, mode);
        QPointer<US_ResidPlotFem> plot(new US_ResidPlotFem(inputs(in), toMode(mode), caller.get()));
        caller.reset();
        QVERIFY(!plot);
    }

    void borrowedBitmapAndRefresh()
    {
        ResidualInputs in; populate(in);
        QVector<QVector<double>> values = {{ .1, .2 }, { -.1, -.2 }};
        in.bitmap = new US_ResidsBitmap(values);
        QPointer<US_ResidsBitmap> original(in.bitmap);
        auto caller = owner(in, "");
        US_ResidPlotFem plot(inputs(in), US_ResidPlotFem::Standalone, caller.get());
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

    void repeatedCloseLeavesNoDialogs_data() { modeRows(); }
    void repeatedCloseLeavesNoDialogs()
    {
        QFETCH(QString, mode);
        ResidualInputs in; populate(in);
        auto caller = owner(in, mode);
        int destroyed = 0;
        QList<QPointer<US_ResidPlotFem>> observed;
        // D01: each closed dialog is destroyed, so replacing it does not accumulate.
        for (int i = 0; i < 100; ++i)
        {
            auto* dialog = new US_ResidPlotFem(inputs(in), toMode(mode), caller.get());
            observed << dialog;
            connect(dialog, &QObject::destroyed, this, [&destroyed]() { ++destroyed; });
            dialog->show();
            dialog->close();
        }
        deferredDeletes();
        QCOMPARE(destroyed, 100);
        for (const auto& dialog : observed) QVERIFY(!dialog);
    }
};

US3_GUI_TEST_MAIN(ResidualDialogTest)
#include "test_us_resplot_fem.moc"
