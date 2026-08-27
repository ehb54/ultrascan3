#include <QPointer>
#include <QPushButton>
#include <QSignalSpy>

#include "qwt_counter.h"
#include "us3_gui_test_main.h"
#include "us_clipdata.h"

namespace
{
QPushButton* buttonWithText(QWidget& window, const QString& expected)
{
    for (QPushButton* button : window.findChildren<QPushButton*>())
        if (button->text() == expected)
            return button;

    return nullptr;
}
}

class US_AstfemClipDataGuiTest : public QObject
{
    Q_OBJECT

private slots:
    void acceptsSelectedSimulationDataRange()
    {
        double concentration = 4.0;
        double radius = 7.2;
        QPointer<US_ClipData> dialog =
            new US_ClipData(concentration, radius, 5.8, 1.0);
        QSignalSpy accepted(dialog, &QDialog::accepted);
        dialog->show();

        QVERIFY(dialog->isVisible());
        QCOMPARE(dialog->windowTitle(),
                 QString("Select Finite Element Simulation Data Range"));

        const QList<QwtCounter*> counters = dialog->findChildren<QwtCounter*>();
        QCOMPARE(counters.size(), 2);
        counters.at(0)->setValue(3.5);
        counters.at(1)->setValue(6.8);

        QPushButton* okButton = buttonWithText(*dialog, "OK");
        QVERIFY2(okButton != nullptr, "The simulation range OK button was not found");
        QTest::mouseClick(okButton, Qt::LeftButton);

        QCOMPARE(accepted.count(), 1);
        QCOMPARE(concentration, 3.5);
        QCOMPARE(radius, 6.8);
        QTRY_VERIFY(dialog.isNull());
    }
};

US3_GUI_TEST_MAIN(US_AstfemClipDataGuiTest)

#include "test_us_astfem_clipdata_gui.moc"
