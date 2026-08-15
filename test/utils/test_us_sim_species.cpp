// test_us_sim_species.cpp
#include "qt_test_base.h"
#include "us_sim_species.h"
#include "us_constants.h"

class US_SimSpeciesTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
    }
};

TEST_F(US_SimSpeciesTest, ModelIsSingleComponentAbsorbanceManual) {
    US_Model model = US_SimSpecies::model();

    EXPECT_FALSE(model.description.isEmpty());
    EXPECT_FALSE(model.modelGUID.isEmpty());
    EXPECT_EQ(model.optics, US_Model::ABSORBANCE);
    EXPECT_EQ(model.analysis, US_Model::MANUAL);
    ASSERT_EQ(model.components.count(), 1);
}

TEST_F(US_SimSpeciesTest, ModelUsesLibraryComponentDefaults) {
    US_Model model = US_SimSpecies::model();
    const US_Model::SimulationComponent& sc = model.components[0];

    EXPECT_DOUBLE_EQ(sc.mw, 50000.0);
    EXPECT_DOUBLE_EQ(sc.f_f0, 1.25);
    EXPECT_DOUBLE_EQ(sc.vbar20, TYPICAL_VBAR);
}

TEST_F(US_SimSpeciesTest, ModelCoefficientsAreComputed) {
    US_Model model = US_SimSpecies::model();
    const US_Model::SimulationComponent& sc = model.components[0];

    EXPECT_GT(sc.s, 0.0);
    EXPECT_GT(sc.D, 0.0);
}

TEST_F(US_SimSpeciesTest, DefaultComponentTracksSimulationComponent) {
    US_SimSpecies::Component c = US_SimSpecies::defaultComponent();
    US_Model::SimulationComponent sc_defaults;

    EXPECT_DOUBLE_EQ(c.vbar20, sc_defaults.vbar20);
    EXPECT_DOUBLE_EQ(c.mw.value, sc_defaults.mw);
    EXPECT_DOUBLE_EQ(c.f_f0.value, sc_defaults.f_f0);
    EXPECT_TRUE(c.mw.supplied);
    EXPECT_TRUE(c.f_f0.supplied);

    // The SimulationComponent default f must not become a third input.
    EXPECT_FALSE(c.s.supplied);
    EXPECT_FALSE(c.D.supplied);
    EXPECT_FALSE(c.f.supplied);
    EXPECT_TRUE(US_SimSpecies::validateComponent(c).isEmpty());
}

TEST_F(US_SimSpeciesTest, BareComponentSuppliesNoCoefficients) {
    US_SimSpecies::Component c;

    EXPECT_GT(c.vbar20, 0.0);
    EXPECT_FALSE(c.s.supplied);
    EXPECT_FALSE(c.D.supplied);
    EXPECT_FALSE(c.mw.supplied);
    EXPECT_FALSE(c.f.supplied);
    EXPECT_FALSE(c.f_f0.supplied);
    EXPECT_FALSE(US_SimSpecies::validateComponent(c).isEmpty());
}

TEST_F(US_SimSpeciesTest, ZeroIsASuppliedCoefficientRatherThanAMarker) {
    // Zero must remain distinguishable from an omitted coefficient.
    US_SimSpecies::Component c;
    c.s    = 0.0;
    c.f_f0 = 1.25;

    EXPECT_TRUE(c.s.supplied);
    EXPECT_DOUBLE_EQ(c.s.value, 0.0);

    // Validation may reject the pair, but not its coefficient count.
    EXPECT_FALSE(US_SimSpecies::validateComponent(c).contains("exactly two"));
}

TEST_F(US_SimSpeciesTest, ComponentAcceptsEverySolvablePair) {
    // Exercise every pair supported by calc_coefficients().
    const double s_ref    = 4.5e-13;
    const double D_ref    = 5.0e-7;
    const double mw_ref   = 50000.0;
    const double f_ref    = 8.0e-8;
    const double ff0_ref  = 1.25;

    QList<QPair<QString, US_SimSpecies::Component>> pairs;
    US_SimSpecies::Component c;

    c = US_SimSpecies::Component(); c.s  = s_ref;  c.D    = D_ref;
    pairs << qMakePair(QString("s+D"), c);
    c = US_SimSpecies::Component(); c.s  = s_ref;  c.mw   = mw_ref;
    pairs << qMakePair(QString("s+mw"), c);
    c = US_SimSpecies::Component(); c.s  = s_ref;  c.f    = f_ref;
    pairs << qMakePair(QString("s+f"), c);
    c = US_SimSpecies::Component(); c.s  = s_ref;  c.f_f0 = ff0_ref;
    pairs << qMakePair(QString("s+f_f0"), c);
    c = US_SimSpecies::Component(); c.mw = mw_ref; c.D    = D_ref;
    pairs << qMakePair(QString("mw+D"), c);
    c = US_SimSpecies::Component(); c.mw = mw_ref; c.f    = f_ref;
    pairs << qMakePair(QString("mw+f"), c);
    c = US_SimSpecies::Component(); c.mw = mw_ref; c.f_f0 = ff0_ref;
    pairs << qMakePair(QString("mw+f_f0"), c);
    c = US_SimSpecies::Component(); c.D  = D_ref;  c.f_f0 = ff0_ref;
    pairs << qMakePair(QString("D+f_f0"), c);
    c = US_SimSpecies::Component(); c.f  = f_ref;  c.f_f0 = ff0_ref;
    pairs << qMakePair(QString("f+f_f0"), c);

    for (const auto& entry : pairs) {
        SCOPED_TRACE(entry.first.toStdString());
        EXPECT_TRUE(US_SimSpecies::validateComponent(entry.second).isEmpty())
            << US_SimSpecies::validateComponent(entry.second).toStdString();

        US_Model model = US_SimSpecies::model(entry.second);
        ASSERT_EQ(model.components.count(), 1);
        const US_Model::SimulationComponent& sc = model.components[0];
        EXPECT_GT(sc.mw, 0.0);
        EXPECT_GT(sc.D, 0.0);
        EXPECT_GT(sc.s, 0.0);
        EXPECT_GE(sc.f_f0, 1.0);
    }
}

TEST_F(US_SimSpeciesTest, ComponentRoundTripsTheSuppliedPair) {
    // Only unsupplied coefficients may be calculated.
    US_SimSpecies::Component c;
    c.s = 4.5e-13;
    c.D = 5.0e-7;

    US_Model model = US_SimSpecies::model(c);
    ASSERT_EQ(model.components.count(), 1);

    EXPECT_DOUBLE_EQ(model.components[0].s, 4.5e-13);
    EXPECT_DOUBLE_EQ(model.components[0].D, 5.0e-7);
}

TEST_F(US_SimSpeciesTest, ComponentRejectsWrongNumberOfCoefficients) {
    US_SimSpecies::Component one;
    one.mw = 50000.0;
    EXPECT_FALSE(US_SimSpecies::validateComponent(one).isEmpty());

    US_SimSpecies::Component three;
    three.mw   = 50000.0;
    three.f_f0 = 1.25;
    three.s    = 4.5e-13;
    EXPECT_FALSE(US_SimSpecies::validateComponent(three).isEmpty());
}

TEST_F(US_SimSpeciesTest, ComponentRejectsUnsolvableDandF) {
    // calc_coefficients() has no D-and-f branch: neither fixes the mass.
    US_SimSpecies::Component c;
    c.D = 5.0e-7;
    c.f = 8.0e-8;

    EXPECT_FALSE(US_SimSpecies::validateComponent(c).isEmpty());
}

TEST_F(US_SimSpeciesTest, ComponentRejectsImpossibleValues) {
    US_SimSpecies::Component negative_mw;
    negative_mw.mw   = -5000.0;
    negative_mw.f_f0 = 1.25;
    EXPECT_FALSE(US_SimSpecies::validateComponent(negative_mw).isEmpty());

    US_SimSpecies::Component negative_vbar = US_SimSpecies::defaultComponent();
    negative_vbar.vbar20 = -0.6;
    EXPECT_FALSE(US_SimSpecies::validateComponent(negative_vbar).isEmpty());

    US_SimSpecies::Component compact;
    compact.mw   = 50000.0;
    compact.f_f0 = 0.5;
    EXPECT_FALSE(US_SimSpecies::validateComponent(compact).isEmpty());
}

TEST_F(US_SimSpeciesTest, ComponentRejectsSignMismatchBetweenSandVbar) {
    // Report the sign mismatch before calc_coefficients() returns false.
    US_SimSpecies::Component floating;
    floating.vbar20 = 1.2;
    floating.s      = 4.5e-13;
    floating.f_f0   = 1.25;

    EXPECT_FALSE(US_SimSpecies::validateComponent(floating).isEmpty());

    // The same species sedimenting the other way is valid.
    floating.s = -4.5e-13;
    EXPECT_TRUE(US_SimSpecies::validateComponent(floating).isEmpty());
}

TEST_F(US_SimSpeciesTest, ComponentRejectsZeroValuedCoefficients) {
    // Zero is as unsolvable as a negative for these three, and the message
    // has always said "greater than zero".
    US_SimSpecies::Component zero_mw;
    zero_mw.mw   = 0.0;
    zero_mw.f_f0 = 1.25;
    EXPECT_FALSE(US_SimSpecies::validateComponent(zero_mw).isEmpty());

    US_SimSpecies::Component zero_D;
    zero_D.D    = 0.0;
    zero_D.f_f0 = 1.25;
    EXPECT_FALSE(US_SimSpecies::validateComponent(zero_D).isEmpty());

    US_SimSpecies::Component zero_f;
    zero_f.f    = 0.0;
    zero_f.f_f0 = 1.25;
    EXPECT_FALSE(US_SimSpecies::validateComponent(zero_f).isEmpty());

    // s is the exception: zero and negative both remain meaningful.
    US_SimSpecies::Component zero_s;
    zero_s.s    = 0.0;
    zero_s.f_f0 = 1.25;
    EXPECT_FALSE(US_SimSpecies::validateComponent(zero_s).contains("greater than zero"));
}

TEST_F(US_SimSpeciesTest, ComponentDefaultsToTheLibraryConcentration) {
    US_SimSpecies::Component c;
    US_Model::SimulationComponent sc_defaults;

    EXPECT_DOUBLE_EQ(c.concentration, sc_defaults.signal_concentration);
    EXPECT_TRUE(c.name.isEmpty());
}

TEST_F(US_SimSpeciesTest, ComponentRejectsNonPositiveConcentration) {
    US_SimSpecies::Component c = US_SimSpecies::defaultComponent();

    c.concentration = 0.0;
    EXPECT_FALSE(US_SimSpecies::validateComponent(c).isEmpty());

    c.concentration = -0.5;
    EXPECT_FALSE(US_SimSpecies::validateComponent(c).isEmpty());
}

TEST_F(US_SimSpeciesTest, MixtureKeepsEveryComponentInOrder) {
    // A monomer/dimer pair: same protein, mass doubled, each with its own
    // measured s, loaded 3:1.
    US_SimSpecies::Component monomer;
    monomer.vbar20        = 0.733;
    monomer.s             = 4.58e-13;
    monomer.mw            = 66430.0;
    monomer.concentration = 0.75;
    monomer.name          = "BSA Monomer";

    US_SimSpecies::Component dimer;
    dimer.vbar20        = 0.733;
    dimer.s             = 6.50e-13;
    dimer.mw            = 132860.0;
    dimer.concentration = 0.25;
    dimer.name          = "BSA Dimer";

    QVector<US_SimSpecies::Component> mixture;
    mixture << monomer << dimer;

    ASSERT_TRUE(US_SimSpecies::validateComponents(mixture).isEmpty())
        << US_SimSpecies::validateComponents(mixture).toStdString();

    US_Model model = US_SimSpecies::model(mixture);
    ASSERT_EQ(model.components.count(), 2);

    EXPECT_EQ(model.components[0].name, QString("BSA Monomer"));
    EXPECT_EQ(model.components[1].name, QString("BSA Dimer"));
    EXPECT_DOUBLE_EQ(model.components[0].signal_concentration, 0.75);
    EXPECT_DOUBLE_EQ(model.components[1].signal_concentration, 0.25);

    // The supplied pair survives, and each component is solved on its own.
    EXPECT_DOUBLE_EQ(model.components[0].s, 4.58e-13);
    EXPECT_DOUBLE_EQ(model.components[1].s, 6.50e-13);
    EXPECT_DOUBLE_EQ(model.components[0].mw, 66430.0);
    EXPECT_DOUBLE_EQ(model.components[1].mw, 132860.0);

    // The dimer is the more extended particle, so it must be the less
    // compact one despite sedimenting faster.
    EXPECT_GT(model.components[0].f_f0, 1.0);
    EXPECT_GT(model.components[1].f_f0, model.components[0].f_f0);
}

TEST_F(US_SimSpeciesTest, MixtureComponentsMayUseDifferentPairs) {
    US_SimSpecies::Component by_s_mw;
    by_s_mw.s  = 4.5e-13;
    by_s_mw.mw = 50000.0;

    US_SimSpecies::Component by_mw_ff0;
    by_mw_ff0.mw   = 100000.0;
    by_mw_ff0.f_f0 = 1.4;

    QVector<US_SimSpecies::Component> mixture;
    mixture << by_s_mw << by_mw_ff0;

    ASSERT_TRUE(US_SimSpecies::validateComponents(mixture).isEmpty());

    US_Model model = US_SimSpecies::model(mixture);
    ASSERT_EQ(model.components.count(), 2);

    for (int ii = 0; ii < model.components.count(); ii++) {
        SCOPED_TRACE(ii);
        EXPECT_GT(model.components[ii].s, 0.0);
        EXPECT_GT(model.components[ii].D, 0.0);
        EXPECT_GT(model.components[ii].mw, 0.0);
    }
}

TEST_F(US_SimSpeciesTest, MixtureValidationIdentifiesTheFailingComponent) {
    US_SimSpecies::Component good = US_SimSpecies::defaultComponent();
    US_SimSpecies::Component bad;   // supplies no coefficients

    QVector<US_SimSpecies::Component> mixture;
    mixture << good << bad;

    QString error = US_SimSpecies::validateComponents(mixture);
    EXPECT_FALSE(error.isEmpty());
    EXPECT_TRUE(error.contains("component 2")) << error.toStdString();

    EXPECT_FALSE(US_SimSpecies::validateComponents(
        QVector<US_SimSpecies::Component>()).isEmpty());
}

TEST_F(US_SimSpeciesTest, SingleComponentModelStillBuildsThroughTheMixturePath) {
    // model(Component) delegates to the mixture overload; the single-component
    // callers must see no change.
    US_SimSpecies::Component c = US_SimSpecies::defaultComponent();
    US_Model model = US_SimSpecies::model(c);

    ASSERT_EQ(model.components.count(), 1);
    EXPECT_DOUBLE_EQ(model.components[0].signal_concentration,
                     US_Model::SimulationComponent().signal_concentration);
    EXPECT_EQ(model.components[0].name,
              US_Model::SimulationComponent().name);
}

TEST_F(US_SimSpeciesTest, TwoCallsToModelProduceDistinctGuids) {
    US_Model model1 = US_SimSpecies::model();
    US_Model model2 = US_SimSpecies::model();

    EXPECT_NE(model1.modelGUID, model2.modelGUID);
}
