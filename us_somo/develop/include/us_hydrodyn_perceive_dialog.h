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
    // `accepted_block` is the entry previously accepted for this residue in this session, if any.
    // When present the dialog opens showing THOSE values -- the point of revisiting is to see and
    // adjust what you accepted, not to be handed a fresh proposal that silently discards it. The
    // perceived proposal is still held, and "Reset this residue" returns to it.
    US_Hydrodyn_Perceive_Dialog( const somo_perceive::Tentative & tent,
                                 const QString & pdb_filename,
                                 void * us_hydrodyn,
                                 QWidget * p = 0,
                                 const char * name = 0,
                                 const QString & accepted_block = QString() );
    ~US_Hydrodyn_Perceive_Dialog();

    // Result of the review, valid once exec-style use returns / the dialog is closed.
    bool accepted() const { return accepted_; }
    // True when the user asked to stop reviewing: the caller must abandon the remaining
    // residues rather than opening another dialog. A neutron structure with explicit
    // deuteriums can make every residue look non-coded, and without this the user has to
    // dismiss one modal per residue with no way out.
    bool skip_all() const { return skip_all_; }
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
    bool        skip_all_ = false;

    // parsed from the proposed block so the dialog can rebuild it after edits
    struct AtomRow {
        QString name, hybrid;
        double  mw = 0, radius = 0, hydration = 0;
        // Deprotonated alternate, written as fields 10-16 of a 16-field atom line exactly as the
        // coded ionizable residues are (ASP OD2, LYS NZ, ARG NH2). Empty ion_hybrid = 8 fields.
        QString ion_hybrid;
        double  ion_mw = 0, ion_radius = 0, ion_hydration = 0;
        // Per ROW, not per residue: under the phosphate convention an entry can carry two
        // switchable groups with different pKas (citrate 4.76 and 6.40), and the header stores
        // one pair per ionization, matched to the atom by its ionization index.
        double  ion_pKa = 0;
        // True when the alternate is shown for information only: the group is held permanently
        // deprotonated because somo.residue holds at most two ionizations per residue, so the
        // primary fields already ARE the deprotonated species and there is nothing to switch.
        bool    ion_display_only = false;
    };
    QVector<AtomRow> rows_;
    QString comment_;                                  // the "# ..." header lines
    // Fields of the proposed header line that the dialog does not edit, kept so refresh_entry()
    // can put them back verbatim. They must NOT be re-invented here: this class rebuilds the
    // whole entry from its widgets, so anything it hardcodes silently overrides the perceiver.
    // A hardcoded 0 in the ASA slot is exactly how generated entries stayed unloadable after the
    // emitter itself was fixed -- the headless path was right and the GUI path was not.
    int    header_type_ = 0;
    double header_asa_  = 0;
    QList<double> header_pKas_;  // one per declared ionization, in header order; index n -> pair n
    // values the widgets open with: the perceived proposal, or what was previously accepted
    double init_vbar_   = 0;
    double init_molvol_ = 0;
    int    init_color_  = somo_perceive::DEFAULT_BEAD_COLOR;
    bool   revisiting_  = false;

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
    QPushButton * pb_reset;
    QPushButton * pb_accept;
    QPushButton * pb_skip;
    QPushButton * pb_skip_all;
    QPushButton * pb_help;

    void setupGUI();
    void parse_block( const QString & data_source );
    void populate_table();
    void refresh_entry();
    void populate_colors();

private slots:
    void view_rasmol();
    void hydration_edited( int row, int col );
    void value_changed();
    void reset_entry();
    void accept_entry();
    void skip_entry();
    void skip_all_entries();
    void help();

protected:
    void closeEvent( QCloseEvent * );
};

#endif
