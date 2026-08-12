#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_perceive_dialog.h"
#include "../include/us_hydrodyn.h"

#include <QCloseEvent>
#include <QDir>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTextStream>
#include <QVBoxLayout>

// Colour table from the SOMO manual (somo/doc/manual/somo/somo_residue.html, Panel 3).
// Reserved entries are listed so the code documents WHY they are absent from the combo, but they
// are never offered: 0 and 6 mark a bead for exclusion from the hydrodynamic computation, and 7
// and 8 are assigned automatically during model generation.
struct ColorDef { int idx; const char * name; const char * meaning; bool selectable; };
static const ColorDef COLORS[] = {
    {  0, "Black",         "RESERVED - very small beads, excluded from computation", false },
    {  1, "Blue",          "protein main chain",                                     true  },
    {  2, "Green",         "protein acidic side chain (D,E)",                        true  },
    {  3, "Cyan",          "protein hydrophobic side chain (A,V,L,I,F,W)",           true  },
    {  4, "Red",           "protein polar side chain (H,Y,S,T,N,Q)",                 true  },
    {  5, "Magenta",       "protein non-polar side chain (C,M,P)",                   true  },
    {  6, "Orange/brown",  "RESERVED - buried beads, assigned automatically",        false },
    {  7, "White",         "USED for fused beads",                                   false },
    {  8, "Grey",          "USED for beads found exposed on re-check",               false },
    {  9, "Light Blue",    "lipid tails, carbon monoxide",                           true  },
    { 10, "Light Green",   "non-coded residues (Automatic Bead Builder)",            true  },
    { 11, "Light Cyan",    "DNA/RNA bases, oxygen, 13P",                             true  },
    { 12, "Light Red",     "heme, NAD, cofactors, prosthetic groups, ions",          true  },
    { 13, "Light Magenta", "carbohydrates, some ions",                               true  },
    { 14, "Yellow",        "protein basic side chain (K,R)",                         true  },
    { 15, "Bright White",  "unassigned",                                             true  },
};

// One explicit colour scheme for this dialog, background AND foreground.
//
// White rather than SOMO's global frame grey: this window is mostly close reading -- a generated
// table entry and a list of flags -- and grey behind it is hard going, markedly so on Linux where
// the frame colour renders darker (Mattia, 2026-08-10: "grey is quite bad, white would be better").
//
// The foreground has to be pinned too. PALET_LABEL and PALET_FRAME build a whole palette from a
// single configured colour, and Qt's derivation of the remaining roles is not identical across
// versions and platforms -- which is why the font colour came out different on Linux and macOS
// ("make sure not just the background but also the font color"). Naming every role we rely on
// removes the derivation from the picture, so the dialog looks the same on both.
//
// Push buttons keep PALET_PUSHB deliberately: they are controls users recognise by their SOMO
// styling, and they are not the surface being read.
static QPalette perceive_palette() {
    QPalette p;
    p.setColor( QPalette::Window,     Qt::white );
    p.setColor( QPalette::Base,       Qt::white );
    p.setColor( QPalette::AlternateBase, Qt::white );
    p.setColor( QPalette::WindowText, Qt::black );
    p.setColor( QPalette::Text,       Qt::black );
    p.setColor( QPalette::ButtonText, Qt::black );
    return p;
}

US_Hydrodyn_Perceive_Dialog::US_Hydrodyn_Perceive_Dialog( const somo_perceive::Tentative & tent,
                                                          const QString & pdb_filename,
                                                          void * us_hydrodyn,
                                                          QWidget * p,
                                                          const char *,
                                                          const QString & accepted_block )
    // Qt::Window is REQUIRED, not decorative. A QFrame constructed with a parent and no window
    // flag is a CHILD WIDGET: setWindowTitle and setWindowModality below silently do nothing, and
    // the setGeometry( x, y, 0, 0 ) idiom SOMO uses throughout -- which the window manager expands
    // to the layout minimum for a real window -- leaves a child sized 0 x 0. The dialog then
    // "opens" invisibly inside the main window, and any caller waiting on isVisible() waits
    // forever. SOMO's other dialogs avoid this by passing no parent at all; keeping the parent and
    // adding the flag is better on macOS, where it also keeps the dialog stacked above its owner.
    : QFrame( p, Qt::Window ) {
    this->us_hydrodyn = us_hydrodyn;
    tent_ = tent;
    pdb_filename_ = pdb_filename;
    USglobal = new US_Config();
    setPalette( perceive_palette() );
    setAutoFillBackground( true );
    // Name the structure in the title bar: several of these dialogs can follow one another, and a
    // reviewer needs to know which structure the entry came from without going back to the main
    // window. (Mattia, review of 2026-08-09: "Add the PDB name on the Review Window".)
    setWindowTitle( pdb_filename.isEmpty()
                    ? us_tr( "SOMO: Review Non-Coded Residue" )
                    : QString( us_tr( "SOMO: Review Non-Coded Residue - %1" ) )
                    .arg( QFileInfo( pdb_filename ).fileName() ) );
    revisiting_ = !accepted_block.isEmpty();
    parse_block( revisiting_ ? accepted_block : tent_.block );
    setupGUI();
    global_Xpos += 30;
    global_Ypos += 30;
    setGeometry( global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Perceive_Dialog::~US_Hydrodyn_Perceive_Dialog() {}

// Split the proposed block into its comment header, atom rows and trailing bead line, so edits
// can be folded back in without the dialog having to re-run the pipeline.
// `data_source` supplies the positional lines -- header, atoms, bead. The commentary and the
// REVIEW block always come from the fresh proposal in tent_.block, because a stored entry has been
// collapsed to a single comment line and no longer carries the flags a reviewer needs to see.
// Revisiting an accepted residue therefore shows the accepted NUMBERS under the original REVIEW
// notes, and "Reset this residue" re-parses with tent_.block as the source to get back the
// perceived values.
void US_Hydrodyn_Perceive_Dialog::parse_block( const QString & data_source ) {
    rows_.clear();
    comment_.clear();
    for ( const QString & cl : tent_.block.split( "\n" ) ) {
        if ( cl.startsWith( "#" ) ) comment_ += cl + "\n";
    }
    const QStringList lines = data_source.split( "\n" );
    bool seen_header = false;
    for ( int i = 0; i < lines.size(); ++i ) {
        const QString & ln = lines[ i ];
        if ( ln.startsWith( "#" ) ) {
            continue;                       // commentary already taken from tent_.block
        }
        if ( ln.trimmed().isEmpty() ) continue;
        const QStringList f = ln.split( "\t" );
        if ( !seen_header ) {
            // residue header: name, type, molvol, ASA, natoms, nbeads, vbar. molvol and vbar are
            // editable and come back from the spin boxes; type and ASA are carried through
            // untouched so this dialog cannot contradict what the perceiver emitted.
            seen_header = true;
            if ( f.size() >= 7 ) {
                header_type_ = f[ 1 ].toInt();
                init_molvol_ = f[ 2 ].toDouble();
                header_asa_  = f[ 3 ].toDouble();
                init_vbar_   = f[ 6 ].toDouble();
            }
            continue;
        }
        if ( f.size() == 5 ) {
            init_color_ = f[ 1 ].toInt();   // bead line: hydration, colour, placing, chain, volume
            continue;
        }
        if ( f.size() >= 8 ) {
            AtomRow r;
            r.name      = f[ 0 ];
            r.hybrid    = f[ 1 ];
            r.mw        = f[ 2 ].toDouble();
            r.radius    = f[ 3 ].toDouble();
            r.hydration = f[ 7 ].toDouble();
            rows_ << r;
        }
    }
}

void US_Hydrodyn_Perceive_Dialog::populate_colors() {
    cmb_color->clear();
    int select_at = 0;
    for ( unsigned int i = 0; i < sizeof( COLORS ) / sizeof( COLORS[ 0 ] ); ++i ) {
        if ( !COLORS[ i ].selectable ) continue;
        cmb_color->addItem( QString( "%1 - %2 (%3)" )
                            .arg( COLORS[ i ].idx )
                            .arg( COLORS[ i ].name )
                            .arg( COLORS[ i ].meaning ),
                            COLORS[ i ].idx );
        if ( COLORS[ i ].idx == init_color_ ) {
            select_at = cmb_color->count() - 1;
        }
    }
    cmb_color->setCurrentIndex( select_at );
}

// Fill the atom table from rows_. Separate from setupGUI so "Reset this residue" can refill it
// after re-parsing. Only the hydration column is editable: the rest follow from the geometry, and
// editing them would desynchronise the entry from the structure it was derived from.
void US_Hydrodyn_Perceive_Dialog::populate_table() {
    const bool was_blocked = tbl_atoms->blockSignals( true );   // do not fire hydration_edited
    tbl_atoms->setRowCount( rows_.size() );
    for ( int r = 0; r < rows_.size(); ++r ) {
        QTableWidgetItem * i0 = new QTableWidgetItem( rows_[ r ].name );
        QTableWidgetItem * i1 = new QTableWidgetItem( rows_[ r ].hybrid );
        QTableWidgetItem * i2 = new QTableWidgetItem( QString::number( rows_[ r ].mw, 'f', 2 ) );
        QTableWidgetItem * i3 = new QTableWidgetItem( QString::number( rows_[ r ].radius, 'f', 2 ) );
        QTableWidgetItem * i4 = new QTableWidgetItem( QString::number( rows_[ r ].hydration, 'f', 2 ) );
        i0->setFlags( i0->flags() & ~Qt::ItemIsEditable );
        i1->setFlags( i1->flags() & ~Qt::ItemIsEditable );
        i2->setFlags( i2->flags() & ~Qt::ItemIsEditable );
        i3->setFlags( i3->flags() & ~Qt::ItemIsEditable );
        tbl_atoms->setItem( r, 0, i0 );
        tbl_atoms->setItem( r, 1, i1 );
        tbl_atoms->setItem( r, 2, i2 );
        tbl_atoms->setItem( r, 3, i3 );
        tbl_atoms->setItem( r, 4, i4 );
    }
    tbl_atoms->blockSignals( was_blocked );
}

void US_Hydrodyn_Perceive_Dialog::setupGUI() {
    const int minHeight1 = 26;

    lbl_info = new QLabel( QString( us_tr( "Non-coded residue: %1%2%3" ) )
                           .arg( tent_.resName )
                           .arg( tent_.chemical_name.isEmpty()
                                 ? QString() : QString( "  [%1]" ).arg( tent_.chemical_name ) )
                           .arg( pdb_filename_.isEmpty()
                                 ? QString()
                                 : QString( "   from %1" )
                                 .arg( QFileInfo( pdb_filename_ ).fileName() ) ),
                           this );
    lbl_info->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
    lbl_info->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
    lbl_info->setMinimumHeight( minHeight1 + 4 );
    lbl_info->setPalette( perceive_palette() );
    AUTFBACK( lbl_info );
    lbl_info->setFont( QFont( USglobal->config_list.fontFamily,
                              USglobal->config_list.fontSize + 1, QFont::Bold ) );

    lbl_summary = new QLabel(
        QString( us_tr( "%1 atoms, %2 instance(s) in this model, %3 atom(s) flagged for review.\n"
                        "Everything below was derived from the coordinates. Check it, correct "
                        "anything that looks wrong, then Accept or Skip." ) )
            .arg( tent_.atoms ).arg( tent_.instances ).arg( tent_.flagged ), this );
    lbl_summary->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
    lbl_summary->setWordWrap( true );
    lbl_summary->setPalette( perceive_palette() );
    AUTFBACK( lbl_summary );
    lbl_summary->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );

    pb_rasmol = new QPushButton( us_tr( "View residue in RasMol" ), this );
    pb_rasmol->setMinimumHeight( minHeight1 );
    pb_rasmol->setPalette( PALET_PUSHB );
    pb_rasmol->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
    pb_rasmol->setEnabled( !pdb_filename_.isEmpty() );
    connect( pb_rasmol, SIGNAL( clicked() ), SLOT( view_rasmol() ) );

    // ---- atom table; only the hydration column is editable -------------------------------
    tbl_atoms = new QTableWidget( rows_.size(), 5, this );
    QStringList headers;
    headers << us_tr( "Atom" ) << us_tr( "Hybrid" ) << us_tr( "Mass" )
            << us_tr( "Radius" ) << us_tr( "Hydration (edit)" );
    tbl_atoms->setHorizontalHeaderLabels( headers );
    tbl_atoms->verticalHeader()->setVisible( false );
    tbl_atoms->setSelectionMode( QAbstractItemView::SingleSelection );
    populate_table();
    tbl_atoms->resizeColumnsToContents();
    tbl_atoms->setMinimumHeight( 180 );
    connect( tbl_atoms, SIGNAL( cellChanged( int, int ) ), SLOT( hydration_edited( int, int ) ) );

    // ---- computed properties ---------------------------------------------------------------
    lbl_vbar = new QLabel( us_tr( " psv (vbar) [cm^3/g]: " ), this );
    lbl_vbar->setPalette( perceive_palette() );
    AUTFBACK( lbl_vbar );
    lbl_vbar->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
    sb_vbar = new QDoubleSpinBox( this );
    sb_vbar->setDecimals( 3 );
    sb_vbar->setRange( 0.0, 3.0 );
    sb_vbar->setSingleStep( 0.005 );
    sb_vbar->setValue( init_vbar_ );
    sb_vbar->setMinimumHeight( minHeight1 );
    sb_vbar->setToolTip( us_tr( "Durchschlag & Zipper atomic volume increments. Typically within "
                                "2-3% for ordinary neutral organic chemistry; less reliable for "
                                "charged groups, lipids and ionic detergents, and not applicable "
                                "to metal centres." ) );
    connect( sb_vbar, SIGNAL( valueChanged( double ) ), SLOT( value_changed() ) );

    lbl_molvol = new QLabel( us_tr( " Anhydrous volume [A^3]: " ), this );
    lbl_molvol->setPalette( perceive_palette() );
    AUTFBACK( lbl_molvol );
    lbl_molvol->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
    sb_molvol = new QDoubleSpinBox( this );
    sb_molvol->setDecimals( 2 );
    sb_molvol->setRange( 0.0, 100000.0 );
    sb_molvol->setSingleStep( 1.0 );
    sb_molvol->setValue( init_molvol_ );
    sb_molvol->setMinimumHeight( minHeight1 );
    sb_molvol->setToolTip( us_tr( "Solvent-excluded volume at a 1.4 A probe, scaled onto the "
                                  "convention the coded residues use. This is also the bead "
                                  "volume." ) );
    connect( sb_molvol, SIGNAL( valueChanged( double ) ), SLOT( value_changed() ) );

    lbl_hydration_total = new QLabel( this );
    lbl_hydration_total->setPalette( perceive_palette() );
    AUTFBACK( lbl_hydration_total );
    lbl_hydration_total->setFont( QFont( USglobal->config_list.fontFamily,
                                         USglobal->config_list.fontSize ) );

    // ---- bead properties -------------------------------------------------------------------
    lbl_beads = new QLabel( us_tr( " Number of beads: " ), this );
    lbl_beads->setPalette( perceive_palette() );
    AUTFBACK( lbl_beads );
    lbl_beads->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
    sb_beads = new QSpinBox( this );
    sb_beads->setRange( 1, 1 );
    sb_beads->setValue( 1 );
    sb_beads->setEnabled( false );
    sb_beads->setMinimumHeight( minHeight1 );
    sb_beads->setToolTip( us_tr( "One bead per non-coded residue for now. Splitting a residue "
                                 "across several beads needs an atom-to-bead assignment, which "
                                 "the Residue Definition Module provides." ) );

    lbl_color = new QLabel( us_tr( " Bead colour: " ), this );
    lbl_color->setPalette( perceive_palette() );
    AUTFBACK( lbl_color );
    lbl_color->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
    cmb_color = new QComboBox( this );
    cmb_color->setMinimumHeight( minHeight1 );
    cmb_color->setToolTip( us_tr( "Colour categorises the bead. Black and orange/brown mark a "
                                  "bead for exclusion from the hydrodynamic computation, and "
                                  "white and grey are assigned automatically, so none of those "
                                  "are offered here." ) );
    populate_colors();
    connect( cmb_color, SIGNAL( currentIndexChanged( int ) ), SLOT( value_changed() ) );

    lbl_position = new QLabel( us_tr( " Bead position: centre of gravity of all atoms " ), this );
    lbl_position->setPalette( perceive_palette() );
    AUTFBACK( lbl_position );
    lbl_position->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );

    // ---- review flags and the running entry ------------------------------------------------
    lbl_review = new QLabel( us_tr( "Flagged for review" ), this );
    lbl_review->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
    lbl_review->setPalette( perceive_palette() );
    AUTFBACK( lbl_review );
    lbl_review->setFont( QFont( USglobal->config_list.fontFamily,
                                USglobal->config_list.fontSize, QFont::Bold ) );

    te_review = new QTextEdit( this );
    te_review->setReadOnly( true );
    te_review->setPalette( perceive_palette() );
    te_review->setMinimumHeight( 90 );
    te_review->setFont( QFont( "Courier", USglobal->config_list.fontSize - 1 ) );
    {
        QString flags;
        const QStringList cl = comment_.split( "\n" );
        bool in_review = false;
        for ( int i = 0; i < cl.size(); ++i ) {
            if ( cl[ i ].startsWith( "# REVIEW" ) ) { in_review = true; continue; }
            if ( in_review && cl[ i ].startsWith( "#   " ) ) flags += cl[ i ].mid( 4 ) + "\n";
            else if ( in_review && !cl[ i ].startsWith( "#" ) ) in_review = false;
        }
        te_review->setText( flags.isEmpty()
                            ? us_tr( "Nothing flagged: every atom was classified confidently.\n"
                                     "The computed values are still estimates -- check them." )
                            : flags );
    }

    te_entry = new QTextEdit( this );
    te_entry->setReadOnly( true );
    te_entry->setPalette( perceive_palette() );
    te_entry->setMinimumHeight( 120 );
    te_entry->setFont( QFont( "Courier", USglobal->config_list.fontSize - 1 ) );

    cb_save = new QCheckBox( us_tr( " Also append this entry to somo.residue " ), this );
    cb_save->setChecked( false );
    cb_save->setPalette( perceive_palette() );
    AUTFBACK( cb_save );
    cb_save->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
    cb_save->setToolTip( us_tr( "Off by default. When off the entry is used for this session "
                                "only and your residue table is left untouched." ) );

    pb_accept = new QPushButton( us_tr( "Accept" ), this );
    pb_accept->setMinimumHeight( minHeight1 );
    pb_accept->setPalette( PALET_PUSHB );
    pb_accept->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
    connect( pb_accept, SIGNAL( clicked() ), SLOT( accept_entry() ) );

    pb_skip = new QPushButton( us_tr( "Skip this residue" ), this );
    pb_skip->setMinimumHeight( minHeight1 );
    pb_skip->setPalette( PALET_PUSHB );
    pb_skip->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
    pb_skip->setToolTip( us_tr( "Leave this residue to the Automatic Bead Builder, as before." ) );
    connect( pb_skip, SIGNAL( clicked() ), SLOT( skip_entry() ) );

    pb_skip_all = new QPushButton( us_tr( "Skip all remaining" ), this );
    pb_skip_all->setMinimumHeight( minHeight1 );
    pb_skip_all->setPalette( PALET_PUSHB );
    pb_skip_all->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
    pb_skip_all->setToolTip( us_tr( "Abandon the review entirely. Every remaining residue is "
                                    "left to the Automatic Bead Builder, as before. Useful when "
                                    "a structure reports far more non-coded residues than "
                                    "expected -- a file carrying explicit hydrogens or deuteriums "
                                    "will not match the residue table at all." ) );
    connect( pb_skip_all, SIGNAL( clicked() ), SLOT( skip_all_entries() ) );

    pb_reset = new QPushButton( us_tr( "Reset this residue" ), this );
    pb_reset->setMinimumHeight( minHeight1 );
    pb_reset->setPalette( PALET_PUSHB );
    pb_reset->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
    pb_reset->setToolTip( us_tr( "Discard the edits shown and go back to what perception "
                                 "proposed for this residue. Does not touch any other residue, "
                                 "and does not un-accept anything -- use Lookup Tables -> Reset "
                                 "Perceived Residues for that." ) );
    connect( pb_reset, SIGNAL( clicked() ), SLOT( reset_entry() ) );

    pb_help = new QPushButton( us_tr( "Help" ), this );
    pb_help->setMinimumHeight( minHeight1 );
    pb_help->setPalette( PALET_PUSHB );
    pb_help->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
    connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

    // ---- layout ------------------------------------------------------------------------------
    QVBoxLayout * top = new QVBoxLayout( this );
    top->setContentsMargins( 4, 4, 4, 4 );
    top->setSpacing( 4 );
    top->addWidget( lbl_info );
    top->addWidget( lbl_summary );
    top->addWidget( pb_rasmol );
    top->addWidget( tbl_atoms );

    QGridLayout * props = new QGridLayout();
    props->addWidget( lbl_vbar,   0, 0 );
    props->addWidget( sb_vbar,    0, 1 );
    props->addWidget( lbl_molvol, 0, 2 );
    props->addWidget( sb_molvol,  0, 3 );
    props->addWidget( lbl_hydration_total, 1, 0, 1, 4 );
    props->addWidget( lbl_beads,  2, 0 );
    props->addWidget( sb_beads,   2, 1 );
    props->addWidget( lbl_color,  2, 2 );
    props->addWidget( cmb_color,  2, 3 );
    props->addWidget( lbl_position, 3, 0, 1, 4 );
    top->addLayout( props );

    top->addWidget( lbl_review );
    top->addWidget( te_review );
    top->addWidget( te_entry );
    top->addWidget( cb_save );

    QHBoxLayout * buttons = new QHBoxLayout();
    buttons->addWidget( pb_help );
    buttons->addWidget( pb_reset );
    buttons->addStretch( 1 );
    buttons->addWidget( pb_skip_all );
    buttons->addWidget( pb_skip );
    buttons->addWidget( pb_accept );
    top->addLayout( buttons );

    refresh_entry();
}

// Rebuild the somo.residue block from the current widget state so what the user sees is exactly
// what Accept will hand back.
void US_Hydrodyn_Perceive_Dialog::refresh_entry() {
    double hyd_total = 0;
    for ( int r = 0; r < rows_.size(); ++r ) hyd_total += rows_[ r ].hydration;

    lbl_hydration_total->setText(
        QString( us_tr( " Residue hydration total: %1 waters   "
                        "(the total is the quantity with literature backing; the per-atom split "
                        "is convention)" ) ).arg( hyd_total, 0, 'f', 2 ) );

    // ASA must stay positive or every SOMO residue loader drops the record silently, and the
    // residue goes on being treated as non-coded no matter what else the user accepted here.
    const double asa = header_asa_ > 0
        ? header_asa_
        : somo_perceive::DEFAULT_ASA_PER_ATOM * rows_.size();

    QString s = comment_;
    s += QString( "%1\t%2\t%3\t%4\t%5\t1\t%6\n" )
             .arg( tent_.resName )
             .arg( header_type_ )
             .arg( sb_molvol->value(), 0, 'f', 2 )
             .arg( asa, 0, 'f', 2 )
             .arg( rows_.size() )
             .arg( sb_vbar->value(), 0, 'f', 3 );
    for ( int r = 0; r < rows_.size(); ++r ) {
        s += QString( "%1\t%2\t%3\t%4\t0\t0\t%5\t%6\n" )
                 .arg( rows_[ r ].name )
                 .arg( rows_[ r ].hybrid )
                 .arg( rows_[ r ].mw )
                 .arg( rows_[ r ].radius )
                 .arg( r )
                 .arg( rows_[ r ].hydration );
    }
    s += QString( "%1\t%2\t0\t0\t%3\n" )
             .arg( hyd_total, 0, 'f', 2 )
             .arg( cmb_color->currentData().toInt() )
             .arg( sb_molvol->value(), 0, 'f', 2 );
    entry_ = s;
    te_entry->setText( s );
}

void US_Hydrodyn_Perceive_Dialog::hydration_edited( int row, int col ) {
    if ( col != 4 || row < 0 || row >= rows_.size() ) return;
    QTableWidgetItem * it = tbl_atoms->item( row, col );
    if ( !it ) return;
    bool ok = false;
    const double v = it->text().toDouble( &ok );
    if ( !ok || v < 0 ) {
        it->setText( QString::number( rows_[ row ].hydration, 'f', 2 ) );
        return;
    }
    rows_[ row ].hydration = v;
    refresh_entry();
}

void US_Hydrodyn_Perceive_Dialog::value_changed() { refresh_entry(); }

// Write a one-off RasMol script that shows the whole structure faintly with this residue
// highlighted, so the user can see what they are being asked to confirm in its real context.
void US_Hydrodyn_Perceive_Dialog::view_rasmol() {
    US_Hydrodyn * uh = (US_Hydrodyn *) us_hydrodyn;
    if ( !uh || pdb_filename_.isEmpty() ) return;

    // Show ONLY the residue, as atoms. Drawing the whole structure and spacefilling the selection
    // works for a ligand in a surface pocket (benzamidine in 3PTB) and fails for anything small or
    // enclosed -- 1MBO's sulfate is simply hidden inside the surrounding wireframe. Extracting the
    // residue into its own PDB also lets RasMol fit the view to it, which no amount of guessing at
    // a zoom factor does reliably. (Mattia, review of 2026-08-09: "visual only the residue as
    // atoms".)
    //
    // Both files are throwaways in the system temp dir, regenerated on every click and read by
    // nothing downstream, so they do not belong in the user's SOMO directories.
    const QString base = QDir::tempPath() + "/somo_perceive_" + tent_.resName;
    const QString res_pdb = base + ".pdb";
    {
        QFile in( pdb_filename_ );
        if ( !in.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
            US_Static::us_message( us_tr( "Please note:" ),
                                   QString( us_tr( "Could not read %1" ) ).arg( pdb_filename_ ) );
            return;
        }
        QFile out( res_pdb );
        if ( !out.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
            US_Static::us_message( us_tr( "Please note:" ),
                                   QString( us_tr( "Could not write %1" ) ).arg( res_pdb ) );
            return;
        }
        QTextStream is( &in );
        QTextStream os( &out );
        int kept = 0;
        while ( !is.atEnd() ) {
            const QString ln = is.readLine();
            if ( ( ln.startsWith( "ATOM  " ) || ln.startsWith( "HETATM" ) ) && ln.length() > 20
                 && ln.mid( 17, 3 ).trimmed() == tent_.resName.trimmed() ) {
                os << ln << "\n";
                ++kept;
            }
        }
        os << "END\n";
        in.close();
        out.close();
        if ( !kept ) {
            US_Static::us_message( us_tr( "Please note:" ),
                                   QString( us_tr( "No atoms named %1 were found in\n%2" ) )
                                   .arg( tent_.resName ).arg( pdb_filename_ ) );
            return;
        }
    }

    const QString spt = base + ".spt";
    QFile f( spt );
    if ( !f.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
        US_Static::us_message( us_tr( "Please note:" ),
                               QString( us_tr( "Could not write %1" ) ).arg( spt ) );
        return;
    }
    QTextStream ts( &f );
    ts << "load " << res_pdb << "\n"
       << "background white\n"
       << "select all\n"
       << "spacefill\n"          // van der Waals radii -- "as atoms"
       << "colour cpk\n"
       << "wireframe 40\n"       // keeps the connectivity legible between the spheres
       << "centre all\n";
    f.close();
    uh->model_viewer( spt, "-script", false );
}

// Back to what perception proposed for this residue, discarding whatever is on screen. This is
// the per-residue counterpart of Lookup Tables -> Reset Perceived Residues, which discards the
// whole session; this one changes nothing that has already been accepted.
void US_Hydrodyn_Perceive_Dialog::reset_entry() {
    parse_block( tent_.block );
    populate_table();
    sb_vbar->setValue( init_vbar_ );
    sb_molvol->setValue( init_molvol_ );
    populate_colors();
    refresh_entry();
}

void US_Hydrodyn_Perceive_Dialog::accept_entry() {
    refresh_entry();
    accepted_ = true;
    save_requested_ = cb_save->isChecked();
    close();
}

void US_Hydrodyn_Perceive_Dialog::skip_entry() {
    accepted_ = false;
    save_requested_ = false;
    close();
}

void US_Hydrodyn_Perceive_Dialog::skip_all_entries() {
    accepted_ = false;
    save_requested_ = false;
    skip_all_ = true;
    close();
}

void US_Hydrodyn_Perceive_Dialog::help() {
    US_Help * online_help = new US_Help( this );
    online_help->show_help( "manual/somo/somo_perceive.html" );
}

void US_Hydrodyn_Perceive_Dialog::closeEvent( QCloseEvent * e ) {
    global_Xpos -= 30;
    global_Ypos -= 30;
    e->accept();
}
