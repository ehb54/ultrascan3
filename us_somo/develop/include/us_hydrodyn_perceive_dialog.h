#ifndef US_HYDRODYN_PERCEIVE_DIALOG_H
#define US_HYDRODYN_PERCEIVE_DIALOG_H

// Review dialog for a residue somo.residue does not code.
//
// The pipeline proposes; the user decides. Everything the perceiver derived from geometry is
// shown, everything it could not derive confidently is flagged, and every proposed number is
// editable before the entry is accepted. Nothing is written to somo.residue unless the user
// explicitly asks for it.
//
// Layout follows the review order Mattia specified:
//   1  view the residue in RasMol
//   2  atom table: name, hybrid, mass, radius -- perceived, shown for confirmation
//   3  atomic hydration, proposed and editable (the weakest of the three computed numbers)
//   4  anhydrous volume from the grid calculation, editable
//   5  bead count -- single bead for now, so the control is present but fixed
//   6  bead position -- centre of gravity of all atoms, the only method in use
//   7  bead colour, chosen from the selectable colours only (reserved ones are never offered)
//   8  psv, editable, with the running entry shown for final validation

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTextEdit>

#include "us_hydrodyn_perceive_somo.h"
#include "us_util.h"

class US_EXTERN US_Hydrodyn_Perceive_Dialog : public QFrame {
    Q_OBJECT

public:
    // `tent` is the proposal to review; `us_hydrodyn` is the owning US_Hydrodyn (void* to match
    // the convention used by the other option dialogs and avoid a circular include).
    US_Hydrodyn_Perceive_Dialog( const somo_perceive::Tentative & tent,
                                 const QString & pdb_filename,
                                 void * us_hydrodyn,
                                 QWidget * p = 0,
                                 const char * name = 0 );
    ~US_Hydrodyn_Perceive_Dialog();

    // Result of the review, valid once exec-style use returns / the dialog is closed.
    bool accepted() const { return accepted_; }
    bool save_requested() const { return save_requested_; }
    QString entry() const { return entry_; }          // the final somo.residue block
    QStringList new_hybrids() const { return tent_.new_hybrids; }

private:
    US_Config * USglobal;
    void      * us_hydrodyn;
    somo_perceive::Tentative tent_;
    QString     pdb_filename_;
    QString     entry_;
    bool        accepted_ = false;
    bool        save_requested_ = false;

    // parsed from the proposed block so the dialog can rebuild it after edits
    struct AtomRow {
        QString name, hybrid;
        double  mw = 0, radius = 0, hydration = 0;
    };
    QVector<AtomRow> rows_;
    QString comment_;                                  // the "# ..." header lines

    QLabel      * lbl_info;
    QLabel      * lbl_summary;
    QPushButton * pb_rasmol;

    QTableWidget * tbl_atoms;

    QLabel         * lbl_vbar;
    QDoubleSpinBox * sb_vbar;
    QLabel         * lbl_molvol;
    QDoubleSpinBox * sb_molvol;
    QLabel         * lbl_hydration_total;

    QLabel    * lbl_beads;
    QSpinBox  * sb_beads;
    QLabel    * lbl_color;
    QComboBox * cmb_color;
    QLabel    * lbl_position;

    QLabel    * lbl_review;
    QTextEdit * te_review;
    QTextEdit * te_entry;

    QCheckBox   * cb_save;
    QPushButton * pb_accept;
    QPushButton * pb_skip;
    QPushButton * pb_help;

    void setupGUI();
    void parse_block();
    void refresh_entry();
    void populate_colors();

private slots:
    void view_rasmol();
    void hydration_edited( int row, int col );
    void value_changed();
    void accept_entry();
    void skip_entry();
    void help();

protected:
    void closeEvent( QCloseEvent * );
};

#endif
