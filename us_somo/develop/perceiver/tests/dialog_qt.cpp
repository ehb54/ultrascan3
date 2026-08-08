// Offscreen exercise of the review dialog: construct it from a synthetic proposal, check it
// parses the block, rebuilds it faithfully, honours a hydration edit, and never offers a
// reserved bead colour. Run with QT_QPA_PLATFORM=offscreen -- no display needed.
#include <QApplication>
#include <QComboBox>
#include <QTableWidget>
#include <QDoubleSpinBox>
#include <cstdio>
#include "../../include/us_hydrodyn_perceive_dialog.h"

static int checks = 0, fails = 0;
#define CK(c) do{ ++checks; if(!(c)){ ++fails; std::printf("  FAIL %s (line %d)\n", #c, __LINE__);} }while(0)

int main( int argc, char ** argv ) {
    QApplication app( argc, argv );

    somo_perceive::Tentative t;
    t.resName = "TST";
    t.chemical_name = "test ligand";
    t.atoms = 3;
    t.instances = 2;
    t.flagged = 1;
    t.vbar = 0.712;
    t.molvol = 123.45;
    t.hydration = 1.0;
    t.block =
        "# test ligand [TST]  (generated on-the-fly by perceiver)\n"
        "# REVIEW - check these before accepting the entry:\n"
        "#   O1 (O2H1): proposed 0 but the coded residues agree only 49% of 115 -- please check\n"
        "TST\t0\t123.45\t0\t3\t1\t0.712\n"
        "N\tN3H1\t15.02\t1.64\t0\t0\t0\t1\n"
        "CA\tC4H1\t13.02\t1.88\t0\t0\t1\t0\n"
        "O1\tO2H1\t17.01\t1.46\t0\t0\t2\t0\n"
        "1\t10\t0\t0\t123.45\n";

    US_Hydrodyn_Perceive_Dialog dlg( t, QString(), 0 );

    // the block round-trips: same atoms, same computed values
    const QString e = dlg.entry();
    std::printf("--- rebuilt entry ---\n%s---\n", e.toLatin1().constData());
    CK( e.contains( "TST\t0\t123.45" ) );
    CK( e.contains( "0.712" ) );
    CK( e.contains( "N\tN3H1" ) );
    CK( e.contains( "O1\tO2H1" ) );
    CK( !dlg.accepted() );                       // nothing accepted until the user says so
    CK( !dlg.save_requested() );                 // saving is opt-in

    // the review flag survived into the dialog
    QTextEdit * te = dlg.findChild<QTextEdit *>();
    CK( te != 0 );

    // colour combo: only selectable colours, defaulting to the configured one
    QComboBox * cmb = dlg.findChild<QComboBox *>();
    CK( cmb != 0 );
    if ( cmb ) {
        bool reserved_offered = false;
        for ( int i = 0; i < cmb->count(); ++i ) {
            const int idx = cmb->itemData( i ).toInt();
            if ( somo_perceive::bead_color_is_reserved( idx ) ) reserved_offered = true;
        }
        std::printf("  colour combo has %d entries, default %d\n",
                    cmb->count(), cmb->currentData().toInt() );
        CK( !reserved_offered );                                   // 0/6/7/8 never offered
        CK( cmb->count() == 12 );                                  // 16 colours minus 4 reserved
        CK( cmb->currentData().toInt() == somo_perceive::DEFAULT_BEAD_COLOR );
    }

    // editing a hydration cell must change the rebuilt entry and the bead total
    QTableWidget * tbl = dlg.findChild<QTableWidget *>();
    CK( tbl != 0 );
    if ( tbl ) {
        CK( tbl->rowCount() == 3 );
        // name/hybrid/mass/radius are read-only; hydration is not
        CK( !( tbl->item( 0, 0 )->flags() & Qt::ItemIsEditable ) );
        CK(  ( tbl->item( 0, 4 )->flags() & Qt::ItemIsEditable ) );
        tbl->item( 2, 4 )->setText( "3.5" );                        // O1: 0 -> 3.5
        const QString after = dlg.entry();
        std::printf("--- after editing O1 hydration to 3.5 ---\n%s---\n",
                    after.toLatin1().constData());
        CK( after.contains( "O1\tO2H1\t17.01\t1.46\t0\t0\t2\t3.5" ) );
        CK( after.contains( "4.50\t10\t0\t0\t123.45" ) );           // bead total 1 + 0 + 3.5
        // a nonsense edit is rejected and the old value restored
        tbl->item( 2, 4 )->setText( "not a number" );
        CK( dlg.entry().contains( "2\t3.5" ) );
    }

    std::printf("\n%d checks, %d failures\n", checks, fails);
    return fails ? 1 : 0;
}
