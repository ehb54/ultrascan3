// Edit-file naming for us_mwl_species_sim.
//
// Every dataset has one edit whose filename contains a plain wavelength.
// Band and "@" forms are in-memory lookup keys and must not be written here.

#include "us3_gui_test_main.h"

#include "us_mwl_species_sim.h"

namespace
{
// The description us_sim_inputs_gen writes, "<run>.<channel><wavelength>.
// model.default". The wavelength is read out of it positionally, so a test
// that invented its own shape would not be testing anything real.
QString description(int wavelength)
{
    return QString("ISSF-synth-mwl-001.1A%1.model.default")
        .arg(wavelength, 3, 10, QChar('0'));
}

QStringList descriptions(const QList<int>& wavelengths)
{
    QStringList list;
    for (int wavelength : wavelengths)
        list << description(wavelength);

    return list;
}
}

class US_MwlSpeciesSimEditNamesTest : public QObject
{
    Q_OBJECT

private slots:
    void readsTheWavelengthOutOfAModelDescription()
    {
        QCOMPARE(US_MwlSpeciesSim::model_wavelength(description(250)),
                 QString("250"));
        QCOMPARE(US_MwlSpeciesSim::model_wavelength(description(280)),
                 QString("280"));
    }

    void singleWavelengthRunNamesTheWavelengthAlone()
    {
        const QStringList models = descriptions({280});

        QCOMPARE(US_MwlSpeciesSim::edit_wavelength_field(models, 0),
                 QString("280"));
    }

    void multiWavelengthRunStillNamesOneWavelengthPerDataset()
    {
        const QStringList models = descriptions({250, 280, 310});

        QCOMPARE(US_MwlSpeciesSim::edit_wavelength_field(models, 0),
                 QString("250"));
        QCOMPARE(US_MwlSpeciesSim::edit_wavelength_field(models, 1),
                 QString("280"));
        QCOMPARE(US_MwlSpeciesSim::edit_wavelength_field(models, 2),
                 QString("310"));
    }

    void nameDependsOnlyOnItsOwnDataset()
    {
        const QStringList ordered  = descriptions({250, 280, 310});
        const QStringList shuffled = descriptions({310, 250, 280});

        QCOMPARE(US_MwlSpeciesSim::edit_wavelength_field(shuffled, 0),
                 QString("310"));
        QCOMPARE(US_MwlSpeciesSim::edit_wavelength_field(shuffled, 1),
                 QString("250"));

        QCOMPARE(US_MwlSpeciesSim::edit_wavelength_field(ordered, 0),
                 US_MwlSpeciesSim::edit_wavelength_field(shuffled, 1));
    }

    // Hyphens and "@" select the band-key path; neither belongs on disk.
    void wavelengthFieldNeverTripsTheBandBranch()
    {
        const QList<QList<int>> runs = {
            {280}, {250, 280, 310}, {280, 280}, {240, 243, 246, 249, 252}
        };

        for (const QList<int>& wavelengths : runs)
        {
            const QStringList models = descriptions(wavelengths);

            for (int index = 0; index < models.count(); ++index)
            {
                const QString field =
                    US_MwlSpeciesSim::edit_wavelength_field(models, index);

                QVERIFY2(!field.contains('-'),
                         qPrintable(QString("wavelength field %1 carries a "
                                            "hyphen and would be read as a "
                                            "wavelength band").arg(field)));
                QVERIFY2(!field.contains('@'),
                         qPrintable(QString("wavelength field %1 carries an "
                                            "'@', which is an in-memory key "
                                            "and never belongs in a name")
                                        .arg(field)));

                bool numeric = false;
                field.toInt(&numeric);
                QVERIFY2(numeric,
                         qPrintable(QString("wavelength field %1 is not a "
                                            "plain integer; every production "
                                            "edit filename is").arg(field)));
            }
        }
    }

    void severalModelsAtOneWavelengthStayPlain()
    {
        const QStringList models = descriptions({280, 280});

        QCOMPARE(US_MwlSpeciesSim::edit_wavelength_field(models, 0),
                 QString("280"));
        QCOMPARE(US_MwlSpeciesSim::edit_wavelength_field(models, 1),
                 QString("280"));
    }

    void indexOutsideTheModelListYieldsNothing()
    {
        const QStringList models = descriptions({250, 310});

        QVERIFY(US_MwlSpeciesSim::edit_wavelength_field(models, -1).isEmpty());
        QVERIFY(US_MwlSpeciesSim::edit_wavelength_field(models, 2).isEmpty());
        QVERIFY(US_MwlSpeciesSim::edit_wavelength_field(QStringList(), 0)
                    .isEmpty());
    }
};

US3_GUI_TEST_MAIN(US_MwlSpeciesSimEditNamesTest)

#include "test_us_mwl_species_sim_editnames_gui.moc"
