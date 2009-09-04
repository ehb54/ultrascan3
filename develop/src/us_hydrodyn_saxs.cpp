#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn_saxs_options.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"

#include <time.h>
#include <qstringlist.h>
#include <qinputdialog.h>
#include <qregexp.h>

#if defined(WIN32)
# include <dos.h>
# include <stdlib.h>
#endif


#define SAXS_DEBUG
#define SAXS_DEBUG2
// #define SAXS_DEBUG_F
// #define SAXS_DEBUG_FV
// #define BUG_DEBUG
// #define RESCALE_B
#define SAXS_MIN_Q 1e-6
// #define ONLY_PHYSICAL_F
// #define I_MULT_2
#define PR_DEBUG

US_Hydrodyn_Saxs::US_Hydrodyn_Saxs(
                                   bool                           *saxs_widget,
                                   saxs_options                   *our_saxs_options,
                                   QString                        filename, 
                                   vector < residue >             residue_list,
                                   vector < PDB_model >           model_vector,
                                   vector < vector <PDB_atom> >   bead_models,
                                   vector < unsigned int >        selected_models,
                                   map < QString, vector <int> >  multi_residue_map,
                                   map < QString, QString >       residue_atom_hybrid_map,
                                   int                            source,
                                   void                           *us_hydrodyn,
                                   QWidget                        *p, 
                                   const char                     *name
                                   ) : QFrame(p, name)
{
   this->saxs_widget = saxs_widget;
   *saxs_widget = true;
   this->our_saxs_options = our_saxs_options;

   // note changes to this section should be updated in US_Hydrodyn_SaxsOptions::update_q()
   if ( our_saxs_options->wavelength == 0 )
   {
      our_saxs_options->start_q = 
         our_saxs_options->end_q = 
         our_saxs_options->delta_q = 0;
   }
   else
   {
      our_saxs_options->start_q = 4.0 * M_PI * 
         sin(our_saxs_options->start_angle * M_PI / 360.0) / 
         our_saxs_options->wavelength;
      our_saxs_options->start_q =  floor(our_saxs_options->start_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
      our_saxs_options->end_q = 4.0 * M_PI * 
         sin(our_saxs_options->end_angle * M_PI / 360.0) / 
         our_saxs_options->wavelength;
      our_saxs_options->end_q =  floor(our_saxs_options->end_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
      our_saxs_options->delta_q = 4.0 * M_PI * 
         sin(our_saxs_options->delta_angle * M_PI / 360.0) / 
         our_saxs_options->wavelength;
      our_saxs_options->delta_q =  floor(our_saxs_options->delta_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
   }         

   this->residue_list = residue_list;
   this->model_vector = model_vector;
   this->bead_models = bead_models;
   this->selected_models = selected_models;
   this->multi_residue_map = multi_residue_map;
   this->residue_atom_hybrid_map = residue_atom_hybrid_map;
   this->source = source;
   this->us_hydrodyn = us_hydrodyn;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("SAXS Plotting Functions"));
   setupGUI();
   editor->append("\n\n");
   QFileInfo fi(filename);
   switch (source)
   {
      case 0: // the source is a PDB file
      {
         lbl_filename1->setText(" PDB Filename: ");
         break;
      }
      case 1: // the source is a Bead Model file
      {
         lbl_filename1->setText(" Bead Model Filename: ");
         break;
      }
      default: // undefined
      {
         QMessageBox mb(tr("UltraScan"),
                        tr("The source file has not been defined, please try again..."), QMessageBox::Critical,
                           QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
         if (mb.exec())
         {
            exit(-1);
         }
      }
   }
   pb_plot_saxs->setEnabled(source ? false : true);
   te_filename2->setText(filename);
   model_filename = filename;
   atom_filename = USglobal->config_list.system_dir + "/etc/somo.atom";
   hybrid_filename = USglobal->config_list.system_dir + "/etc/somo.hybrid";
   saxs_filename =  USglobal->config_list.system_dir + "/etc/somo.saxs_atoms";
   select_saxs_file(saxs_filename);
   select_hybrid_file(hybrid_filename);
   select_atom_file(atom_filename);
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
   stopFlag = false;
   pb_stop->setEnabled(false);
   plot_colors.clear();
   plot_colors.push_back(Qt::yellow);
   plot_colors.push_back(Qt::green);
   plot_colors.push_back(Qt::cyan);
   plot_colors.push_back(Qt::blue);
   plot_colors.push_back(Qt::red);
}

US_Hydrodyn_Saxs::~US_Hydrodyn_Saxs()
{
   *saxs_widget = false;
}

void US_Hydrodyn_Saxs::refresh(
                               QString                        filename, 
                               vector < residue >             residue_list,
                               vector < PDB_model >           model_vector,
                               vector < vector <PDB_atom> >   bead_models,
                               vector < unsigned int >        selected_models,
                               map < QString, vector <int> >  multi_residue_map,
                               map < QString, QString >       residue_atom_hybrid_map,
                               int                            source
                               )
{
   this->residue_list = residue_list;
   this->model_vector = model_vector;
   this->bead_models = bead_models;
   this->selected_models = selected_models;
   this->multi_residue_map = multi_residue_map;
   this->residue_atom_hybrid_map = residue_atom_hybrid_map;
   this->source = source;
   QFileInfo fi(filename);
   switch (source)
   {
      case 0: // the source is a PDB file
      {
         lbl_filename1->setText(" PDB Filename: ");
         break;
      }
      case 1: // the source is a Bead Model file
      {
         lbl_filename1->setText(" Bead Model Filename: ");
         break;
      }
      default: // undefined
      {
         QMessageBox mb(tr("UltraScan"),
                        tr("The source file has not been defined, please try again..."), QMessageBox::Critical,
                           QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
         if (mb.exec())
         {
            exit(-1);
         }
      }
   }
   pb_plot_saxs->setEnabled(source ? false : true);
   te_filename2->setText(filename);
   model_filename = filename;
   pb_stop->setEnabled(false);
}


void US_Hydrodyn_Saxs::setupGUI()
{
   int minHeight1 = 30;
   lbl_info = new QLabel(tr("SAXS Plotting Functions:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_filename1 = new QLabel(tr(""), this);
   Q_CHECK_PTR(lbl_filename1);
   lbl_filename1->setMinimumHeight(minHeight1);
   lbl_filename1->setAlignment(AlignLeft|AlignVCenter);
   lbl_filename1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_filename1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   te_filename2 = new QTextEdit(this, "");
   Q_CHECK_PTR(te_filename2);
   te_filename2->setMinimumHeight(minHeight1);
   te_filename2->setAlignment(AlignLeft|AlignVCenter);
   te_filename2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   te_filename2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   te_filename2->setMinimumWidth(200);

   pb_select_atom_file = new QPushButton(tr("Load Atom Definition File"), this);
   Q_CHECK_PTR(pb_select_atom_file);
   pb_select_atom_file->setMinimumHeight(minHeight1);
   pb_select_atom_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_atom_file->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_atom_file, SIGNAL(clicked()), SLOT(select_atom_file()));

   pb_select_hybrid_file = new QPushButton(tr("Load Hybridization File"), this);
   Q_CHECK_PTR(pb_select_hybrid_file);
   pb_select_hybrid_file->setMinimumHeight(minHeight1);
   pb_select_hybrid_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_hybrid_file->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_hybrid_file, SIGNAL(clicked()), SLOT(select_hybrid_file()));

   pb_select_saxs_file = new QPushButton(tr("Load SAXS Coefficient File"), this);
   Q_CHECK_PTR(pb_select_saxs_file);
   pb_select_saxs_file->setMinimumHeight(minHeight1);
   pb_select_saxs_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_saxs_file->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_saxs_file, SIGNAL(clicked()), SLOT(select_saxs_file()));

   lbl_atom_table = new QLabel(tr(" not selected"),this);
   lbl_atom_table->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_atom_table->setAlignment(AlignLeft|AlignVCenter);
   lbl_atom_table->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_atom_table->setMinimumHeight(minHeight1);
   lbl_atom_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_hybrid_table = new QLabel(tr(" not selected"),this);
   lbl_hybrid_table->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_hybrid_table->setAlignment(AlignLeft|AlignVCenter);
   lbl_hybrid_table->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_hybrid_table->setMinimumHeight(minHeight1);
   lbl_hybrid_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_saxs_table = new QLabel(tr(" not selected"),this);
   lbl_saxs_table->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_saxs_table->setAlignment(AlignLeft|AlignVCenter);
   lbl_saxs_table->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_saxs_table->setMinimumHeight(minHeight1);
   lbl_saxs_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   pb_plot_saxs = new QPushButton(tr("Compute SAXS Curve"), this);
   Q_CHECK_PTR(pb_plot_saxs);
   pb_plot_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_plot_saxs->setMinimumHeight(minHeight1);
   pb_plot_saxs->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_plot_saxs, SIGNAL(clicked()), SLOT(show_plot_saxs()));

   pb_load_saxs = new QPushButton(tr("Load SAXS Curve"), this);
   Q_CHECK_PTR(pb_load_saxs);
   pb_load_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_saxs->setMinimumHeight(minHeight1);
   pb_load_saxs->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_saxs, SIGNAL(clicked()), SLOT(load_saxs()));

   pb_clear_plot_saxs = new QPushButton(tr("Clear SAXS Curve"), this);
   Q_CHECK_PTR(pb_clear_plot_saxs);
   pb_clear_plot_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_clear_plot_saxs->setMinimumHeight(minHeight1);
   pb_clear_plot_saxs->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_clear_plot_saxs, SIGNAL(clicked()), SLOT(clear_plot_saxs()));

   lbl_info_prr = new QLabel(tr("P(r) vs. r  Plotting Functions:"), this);
   Q_CHECK_PTR(lbl_info_prr);
   lbl_info_prr->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info_prr->setAlignment(AlignCenter|AlignVCenter);
   lbl_info_prr->setMinimumHeight(minHeight1);
   lbl_info_prr->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info_prr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_bin_size = new QLabel(tr(" Bin size (Angstrom): "), this);
   Q_CHECK_PTR(lbl_bin_size);
   lbl_bin_size->setAlignment(AlignLeft|AlignVCenter);
   lbl_bin_size->setMinimumHeight(minHeight1);
   lbl_bin_size->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_bin_size->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_bin_size= new QwtCounter(this);
   Q_CHECK_PTR(cnt_bin_size);
   cnt_bin_size->setRange(0.01, 100, 0.01);
   cnt_bin_size->setValue(our_saxs_options->bin_size);
   cnt_bin_size->setMinimumHeight(minHeight1);
   cnt_bin_size->setEnabled(true);
   cnt_bin_size->setNumButtons(3);
   cnt_bin_size->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_bin_size->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_bin_size, SIGNAL(valueChanged(double)), SLOT(update_bin_size(double)));

   pb_plot_pr = new QPushButton(tr("Compute P(r) distribution"), this);
   Q_CHECK_PTR(pb_plot_pr);
   pb_plot_pr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_plot_pr->setMinimumHeight(minHeight1);
   pb_plot_pr->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_plot_pr, SIGNAL(clicked()), SLOT(show_plot_pr()));

   pb_load_pr = new QPushButton(tr("Load P(r) Distribution"), this);
   Q_CHECK_PTR(pb_load_pr);
   pb_load_pr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_pr->setMinimumHeight(minHeight1);
   pb_load_pr->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_pr, SIGNAL(clicked()), SLOT(load_pr()));

   pb_clear_plot_pr = new QPushButton(tr("Clear P(r) Distribution"), this);
   Q_CHECK_PTR(pb_clear_plot_pr);
   pb_clear_plot_pr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_clear_plot_pr->setMinimumHeight(minHeight1);
   pb_clear_plot_pr->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_clear_plot_pr, SIGNAL(clicked()), SLOT(clear_plot_pr()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_stop = new QPushButton(tr("Stop"), this);
   Q_CHECK_PTR(pb_stop);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   plot_saxs = new QwtPlot(this);
   plot_saxs->enableOutline(true);
   plot_saxs->setOutlinePen(white);
   plot_saxs->setOutlineStyle(Qwt::VLine);
   plot_saxs->enableGridXMin();
   plot_saxs->enableGridYMin();
   plot_saxs->setPalette( QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   plot_saxs->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_saxs->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   plot_saxs->setAxisTitle(QwtPlot::xBottom, tr("q (1/Angstrom)"));
   plot_saxs->setAxisTitle(QwtPlot::yLeft, tr("Log10 I(q)"));
   plot_saxs->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_saxs->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_saxs->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_saxs->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_saxs->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_saxs->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_saxs->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_saxs->setMargin(USglobal->config_list.margin);
   plot_saxs->setTitle(tr("Simulated SAXS Curve"));
   plot_saxs->setCanvasBackground(USglobal->global_colors.plot);

   plot_pr = new QwtPlot(this);
   plot_pr->enableOutline(true);
   plot_pr->setOutlinePen(white);
   plot_pr->setOutlineStyle(Qwt::VLine);
   plot_pr->enableGridXMin();
   plot_pr->enableGridYMin();
   plot_pr->setPalette( QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   plot_pr->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_pr->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   plot_pr->setAxisTitle(QwtPlot::xBottom, tr("Distance (Angstrom)"));
   plot_pr->setAxisTitle(QwtPlot::yLeft, tr("Frequency"));
   plot_pr->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_pr->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_pr->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_pr->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_pr->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_pr->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_pr->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_pr->setMargin(USglobal->config_list.margin);
   plot_pr->setTitle(tr("P(r) Distribution Curve"));
   plot_pr->setCanvasBackground(USglobal->global_colors.plot);

   progress_saxs = new QProgressBar(this, "SAXS Progress");
   progress_saxs->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   progress_saxs->reset();

   progress_pr = new QProgressBar(this, "P(r) Progress");
   progress_pr->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   progress_pr->reset();

   editor = new QTextEdit(this);
   editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   editor->setReadOnly(true);
   editor->setMinimumWidth(300);
   m = new QMenuBar(editor, "menu" );
   m->setMinimumHeight(minHeight1);
   m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   QPopupMenu * file = new QPopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    ALT+Key_S );
   file->insertItem( tr("Print"), this, SLOT(print()),   ALT+Key_P );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
   editor->setWordWrap (QTextEdit::WidgetWidth);
   // editor->setWordWrap (QTextEdit::NoWrap);

   lbl_core_progress = new QLabel("", this);
   Q_CHECK_PTR(lbl_core_progress);
   lbl_core_progress->setAlignment(AlignHCenter|AlignVCenter);
   lbl_core_progress->setMinimumHeight(minHeight1);
   lbl_core_progress->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_core_progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));


   int rows=13, columns = 3, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   background->addMultiCellWidget(plot_saxs, j, j+10, 2, 2);
   j++;
   background->addWidget(lbl_filename1, j, 0);
   background->addWidget(te_filename2, j, 1);
   j++;
   background->addWidget(pb_select_atom_file, j, 0);
   background->addWidget(lbl_atom_table, j, 1);
   j++;
   background->addWidget(pb_select_hybrid_file, j, 0);
   background->addWidget(lbl_hybrid_table, j, 1);
   j++;
   background->addWidget(pb_select_saxs_file, j, 0);
   background->addWidget(lbl_saxs_table, j, 1);
   j++;
   background->addWidget(pb_plot_saxs, j, 0);
   background->addWidget(progress_saxs, j, 1);
   j++;
   background->addWidget(pb_load_saxs, j, 0);
   background->addWidget(pb_clear_plot_saxs, j, 1);
   j++;
   background->addMultiCellWidget(lbl_info_prr, j, j, 0, 1);
   j++;
   background->addWidget(lbl_bin_size, j, 0);
   background->addWidget(cnt_bin_size, j, 1);
   j++;
   background->addWidget(pb_plot_pr, j, 0);
   background->addWidget(progress_pr, j, 1);
   j++;
   background->addWidget(pb_load_pr, j, 0);
   background->addWidget(pb_clear_plot_pr, j, 1);
   j++;
   background->addMultiCellWidget(editor, j, j, 0, 1);
   background->addMultiCellWidget(plot_pr, j, j+3, 2, 2);
   j+=3;
   background->addWidget(pb_stop, j, 0);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
   background->addWidget(lbl_core_progress, j, 2);

   background->setColSpacing(2, 600);
   //   background->setColStretch(0, 2);
   //   background->setColStretch(1, 2);
   //   background->setColStretch(2, 10);
}

void US_Hydrodyn_Saxs::cancel()
{
   close();
}

void US_Hydrodyn_Saxs::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_saxs.html");
}

void US_Hydrodyn_Saxs::stop()
{
   stopFlag = true;
   pb_stop->setEnabled(false);
}

void US_Hydrodyn_Saxs::closeEvent(QCloseEvent *e)
{
   *saxs_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

//--------- thread for saxs p(r) plot -----------

// #define DEBUG_THREAD

saxs_pr_thr_t::saxs_pr_thr_t(int a_thread) : QThread()
{
   thread = a_thread;
   work_to_do = 0;
   work_done = 1;
   work_to_do_waiters = 0;
   work_done_waiters = 0;
}

void saxs_pr_thr_t::saxs_pr_thr_setup(
                                      vector < saxs_atom > *atoms,
                                      vector < unsigned int > *hist,
                                      double delta,
                                      unsigned int threads,
                                      QProgressBar *progress,
                                      QLabel *lbl_core_progress,
                                      bool *stopFlag
                                      )
{
   /* this starts up a new work load for the thread */
   this->atoms = atoms;
   this->hist = hist;
   this->delta = delta;
   this->threads = threads;
   this->progress = progress;
   this->lbl_core_progress = lbl_core_progress;
   this->stopFlag = stopFlag;

   work_mutex.lock();
   work_to_do = 1;
   work_done = 0;
   work_mutex.unlock();
   cond_work_to_do.wakeOne();
#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " has new work to do\n";
#endif
}

void saxs_pr_thr_t::saxs_pr_thr_shutdown()
{
   /* this signals the thread to exit the run method */
   work_mutex.lock();
   work_to_do = -1;
   work_mutex.unlock();
   cond_work_to_do.wakeOne();

#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " shutdown requested\n";
#endif
}

void saxs_pr_thr_t::saxs_pr_thr_wait()
{
   /* this is for the master thread to wait until the work is done */
   work_mutex.lock();

#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " has a waiter\n";
#endif

   while(!work_done) {
      cond_work_done.wait(&work_mutex);
   }
   work_done = 0;
   work_mutex.unlock();

#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " waiter released\n";
#endif
}

int saxs_pr_thr_t::saxs_pr_thr_work_status()
{
   work_mutex.lock();
   int retval = work_done;
   work_mutex.unlock();
   return retval;
}

void saxs_pr_thr_t::run()
{
   while(1)
   {
      work_mutex.lock();
#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " waiting for work\n";
#endif
      work_to_do_waiters++;
      while(!work_to_do)
      {
         cond_work_to_do.wait(&work_mutex);
      }
      if(work_to_do == -1)
      {
#if defined(DEBUG_THREAD)
         cerr << "thread " << thread << " shutting down\n";
#endif
         work_mutex.unlock();
         return;
      }

      work_to_do_waiters = 0;
      work_mutex.unlock();
#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " starting work\n";
#endif
      
      unsigned int as = (*atoms).size();
      unsigned int as1 = as - 1;
      unsigned int pos;
      double rik; // distance from atom i to k 
      if ( !thread ) 
      {
         progress->setTotalSteps((int)(1.15f * as1 / threads));
      }
#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " as1 = " << as1 
           << endl;
#endif

      for ( unsigned int i = thread; i < as1; i += threads )
      {
#if defined(DEBUG_THREAD)
         cerr << "thread " << thread << " i = " << i << endl;
#endif
         if ( !thread ) 
         {
            // lbl_core_progress->setText(QString("Atom %1 of %2\n").arg(i+1).arg(as));
            progress->setProgress(i+1);
            // qApp->processEvents();
         }
         if ( *stopFlag ) 
         {
            break;
         }
            
         for ( unsigned int k = i + 1; k < as; k++ )
         {
            rik = 
               sqrt(
                    ((*atoms)[i].pos[0] - (*atoms)[k].pos[0]) *
                    ((*atoms)[i].pos[0] - (*atoms)[k].pos[0]) +
                    ((*atoms)[i].pos[1] - (*atoms)[k].pos[1]) *
                    ((*atoms)[i].pos[1] - (*atoms)[k].pos[1]) +
                    ((*atoms)[i].pos[2] - (*atoms)[k].pos[2]) *
                    ((*atoms)[i].pos[2] - (*atoms)[k].pos[2])
                    );
#if defined(SAXS_DEBUG_F)
            cout << "dist atoms:  "
                 << i
                 << " "
                 << (*atoms)[i].saxs_name
                 << ","
                 << k
                 << " "
                 << (*atoms)[k].saxs_name
                 << " "
                 << rik
                 << endl;
#endif
            pos = (unsigned int)floor(rik / delta);
            if ( hist->size() <= pos )
            {
               hist->resize(pos + 128);
            }
            (*hist)[pos]++;
         }
      }

#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " finished work\n";
#endif
      work_mutex.lock();
      work_done = 1;
      work_to_do = 0;
      work_mutex.unlock();
      cond_work_done.wakeOne();
   }
}

//--------- end thread for saxs p(r) plot -----------

void US_Hydrodyn_Saxs::normalize_pr( vector < double > *pr )
{
   // set distribution to a 1 peak
   double max = 0e0;
   if ( pr->size() )
   {
      (*pr)[0];
   }
   for ( unsigned int i = 1; i < pr->size(); i++ )
   {
      if ( (*pr)[i] > max )
      {
         max = (*pr)[i];
      }
   }
   if ( max > 0e0 )
   {
      for ( unsigned int i = 0; i < pr->size(); i++ )
      {
         (*pr)[i] /= max;
      }
   }
}


void US_Hydrodyn_Saxs::update_bin_size(double val)
{
   our_saxs_options->bin_size = (float) val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Saxs::show_plot_pr()
{
   stopFlag = false;
   pb_stop->setEnabled(true);
   pb_plot_pr->setEnabled(false);
   pb_plot_saxs->setEnabled(false);
   progress_pr->reset();
   vector < unsigned int > hist;
   float delta = our_saxs_options->bin_size;

#if defined(BUG_DEBUG)
   qApp->processEvents();
   cout << " sleep 1 a" << endl;
   sleep(1);
   cout << " sleep 1 a done" << endl;
#endif
   
   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
      current_model = selected_models[i];
#if defined(PR_DEBUG)
      printf("creating pr %u\n", current_model); fflush(stdout);
#endif
      editor->append(QString("\n\nPreparing file %1 model %2 for p(r) vs r plot.\n\n")
                     .arg(te_filename2->text())
                     .arg(current_model + 1));
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor->append(tr("Terminated by user request.\n"));
         progress_pr->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs->setEnabled(source ? false : true);
         pb_plot_pr->setEnabled(true);
         return;
      }
         
      vector < saxs_atom > atoms;
      saxs_atom new_atom;
      if ( source )
      {
         // bead models
         for (unsigned int j = 0; j < bead_models[current_model].size(); j++)
         {
            PDB_atom *this_atom = &(bead_models[current_model][j]);
            new_atom.pos[0] = this_atom->bead_coordinate.axis[0];
            new_atom.pos[1] = this_atom->bead_coordinate.axis[1];
            new_atom.pos[2] = this_atom->bead_coordinate.axis[2];
            atoms.push_back(new_atom);
         }
      }
      else 
      {
         // pdb files
         for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++)
         {
            for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++)
            {
               PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
               new_atom.pos[0] = this_atom->coordinate.axis[0];
               new_atom.pos[1] = this_atom->coordinate.axis[1];
               new_atom.pos[2] = this_atom->coordinate.axis[2];
               atoms.push_back(new_atom);
            }
         }
      }
#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << "atoms size " << atoms.size() << endl;
      cout << " sleep 1 b" << endl;
      sleep(1);
      cout << " sleep 1 b done" << endl;
#endif
      // ok now we have all the atoms

      editor->append(QString("Number of atoms %1. Bin size %2.\n")
                     .arg(atoms.size())
                     .arg(delta));
      qApp->processEvents();

      if ( USglobal->config_list.numThreads > 1 )
      {
         // threaded
         
         unsigned int j;
         unsigned int threads = USglobal->config_list.numThreads;
         editor->append(QString("Using %1 threads.\n").arg(threads));
         vector < saxs_pr_thr_t* > saxs_pr_thr_threads(threads);
         for ( j = 0; j < threads; j++ )
         {
            saxs_pr_thr_threads[j] = new saxs_pr_thr_t(j);
            saxs_pr_thr_threads[j]->start();
            
         }
         vector < vector < unsigned int > > hists;
         hists.resize(threads);
         for ( j = 0; j < threads; j++ )
         {
# if defined(DEBUG_THREAD)
            cout << "thread " << j << endl;
# endif            
            saxs_pr_thr_threads[j]->saxs_pr_thr_setup(
                                                      &atoms,
                                                      &hists[j],
                                                      delta,
                                                      threads,
                                                      progress_pr,
                                                      lbl_core_progress,
                                                      &stopFlag
                                                      );

         }
         // sleep app loop
         {
            int all_done;
#if !defined(WIN32)
            timespec ns;
            timespec ns_ret;
            ns.tv_sec = 0;
            ns.tv_nsec = 50000000l;
#endif
            
            do {
               all_done = threads;
               for ( j = 0; j < threads; j++ )
               {
                  all_done -= saxs_pr_thr_threads[j]->saxs_pr_thr_work_status();
               }
               qApp->processEvents();
#if defined(WIN32)
               _sleep(1);
#else
               nanosleep(&ns, &ns_ret);
#endif
            } while(all_done);
         }
         
         // wait for work to complete

         for ( j = 0; j < threads; j++ )
         {
            saxs_pr_thr_threads[j]->saxs_pr_thr_wait();
         }

         // destroy
         
         for ( j = 0; j < threads; j++ )
         {
            saxs_pr_thr_threads[j]->saxs_pr_thr_shutdown();
         }
         
         for ( j = 0; j < threads; j++ )
         {
            saxs_pr_thr_threads[j]->wait();
         }
         
         for ( j = 0; j < threads; j++ )
         {
            delete saxs_pr_thr_threads[j];
         }
         
         // merge results
         for ( j = 0; j < threads; j++ )
         {
            if (hist.size() < hists[j].size() )
            {
               hist.resize(hists[j].size());
            }
            for ( unsigned int k = 0; k < hists[j].size(); k++ )
            {
               hist[k] += hists[j][k];
            }
         }

      } // end threaded
      else
      {
         // non-threaded
         float rik; 
         unsigned int pos;
         progress_pr->setTotalSteps((int)(atoms.size()));
         for ( unsigned int i = 0; i < atoms.size() - 1; i++ )
         {
            progress_pr->setProgress(i+1);
            qApp->processEvents();
            if ( stopFlag ) 
            {
               editor->append(tr("Terminated by user request.\n"));
               progress_pr->reset();
               lbl_core_progress->setText("");
               pb_plot_saxs->setEnabled(source ? false : true);
               pb_plot_pr->setEnabled(true);
               return;
            }
            for ( unsigned int j = i + 1; j < atoms.size(); j++ )
            {
               rik = 
                  sqrt(
                       (atoms[i].pos[0] - atoms[j].pos[0]) *
                       (atoms[i].pos[0] - atoms[j].pos[0]) +
                       (atoms[i].pos[1] - atoms[j].pos[1]) *
                       (atoms[i].pos[1] - atoms[j].pos[1]) +
                       (atoms[i].pos[2] - atoms[j].pos[2]) *
                       (atoms[i].pos[2] - atoms[j].pos[2])
                       );
               pos = (unsigned int)floor(rik / delta);
               if ( hist.size() <= pos )
               {
                  hist.resize(pos + 128);
               }
               hist[pos]++;
            }
         }
      } // end non-threaded
         
#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 bb" << endl;
      sleep(1);
      cout << " sleep 1 bb done" << endl;
#endif
      // trim hist
#if defined(PR_DEBUG)
      cout << "hist.size() " << hist.size() << endl;
#endif
      while( hist.size() && !hist[hist.size()-1] ) 
      {
         hist.pop_back();
      }
#if defined(PR_DEBUG)
      cout << "hist.size() after " << hist.size() << endl;
#endif

      // save the data to a file
      QString fpr_name = USglobal->config_list.root_dir + 
         "/somo/saxs/" + QString("%1").arg(te_filename2->text()) +
         QString("_%1").arg(current_model + 1) + 
         ".sprr";
      bool ok_to_write = true;
      if ( QFile::exists(fpr_name) )
      {
         switch( QMessageBox::information( this, 
                                           tr("Overwrite file:") + "SAXS P(r) vs. r" + tr("output file"),
                                           tr("The P(r) curve file \"") + QString("%1").arg(te_filename2->text()) +
                                           QString("_%1").arg(current_model + 1) + 
                                           ".sprr" + tr("\" will be overwriten"),
                                           "&Ok",  "&Cancel", 0,
                                           0,      // Enter == button 0
                                           1 ) ) { // Escape == button 2
         case 0: // just go ahead
            ok_to_write = true;
            break;
         case 1: // Cancel clicked or Escape pressed
            ok_to_write = false;
            break;
         }
      }

      if ( ok_to_write )
      {
         FILE *fpr = fopen(fpr_name, "w");
         if ( fpr ) 
         {
            editor->append(tr("P(r) curve file: ") + fpr_name + tr(" created.\n"));
            fprintf(fpr,
                    "SOMO p(r) vs r data generated from %s by US_SOMO %s %s bin size %f\n"
                    , model_filename.ascii()
                    , US_Version.ascii()
                    , REVISION
                    , delta
                    );
            vector < double > pr;
            pr.resize(hist.size());
            for ( unsigned int i = 0; i < hist.size(); i++) 
            {
               pr[i] = (double) hist[i];
            }
            normalize_pr(&pr);
            for ( unsigned int i = 0; i < hist.size(); i++ )
            {
               if ( hist[i] ) {
                  fprintf(fpr, "%.6e\t%.6e\n", i * delta, pr[i]);
               }
            }
            fclose(fpr);
         }
         else
         {
#if defined(PR_DEBUG)
            cout << "can't create " << fpr_name << endl;
#endif
            editor->append(tr("WARNING: Could not create PR curve file: ") + fpr_name + "\n");
            QMessageBox mb(tr("UltraScan Warning"),
                           tr("The output file ") + fpr_name + tr(" could not be created."), 
                           QMessageBox::Critical,
                           QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
            mb.exec();
         }
      }
   } // models


   long ppr = plot_pr->insertCurve("P(r) vs r");
   vector < double > r;
   vector < double > pr;
   r.resize(hist.size());
   pr.resize(hist.size());
   for ( unsigned int i = 0; i < hist.size(); i++) 
   {
      r[i] = i * delta;
      pr[i] = (double) hist[i];
#if defined(PR_DEBUG)
      printf("%e %e\n", r[i], pr[i]);
#endif
   }
   normalize_pr(&pr);

   plot_pr->setCurveStyle(ppr, QwtCurve::Lines);
   plotted_r.push_back(r);
   plotted_pr.push_back(pr);
   unsigned int p = plotted_r.size() - 1;

   plot_pr->setCurveData(ppr, (double *)&(r[0]), (double *)&(pr[0]), (int)r.size());
   plot_pr->setCurvePen(ppr, QPen(plot_colors[p % plot_colors.size()], 2, SolidLine));
   plot_pr->replot();

   progress_pr->setTotalSteps(1);
   progress_pr->setProgress(1);
   pb_plot_saxs->setEnabled(source ? false : true);
   pb_plot_pr->setEnabled(true);
}

void US_Hydrodyn_Saxs::load_pr()
{
   QString filename = QFileDialog::getOpenFileName(USglobal->config_list.root_dir + "/somo/saxs", "*", this);
   if (filename.isEmpty())
   {
      return;
   }
   QFile f(filename);
   QString ext = QFileInfo(filename).extension(FALSE).lower();
   vector < double > r;
   vector < double > pr;
   double new_r, new_pr;
   QString res = "";
   unsigned int startline = 0;
   unsigned int pop_last = 0;
   if ( f.open(IO_ReadOnly) )
   {
      if ( ext != "sprr" ) 
      {
         // check for gnom output
         QTextStream ts(&f);
         QString tmp;
         unsigned int pos = 0;
         while ( !ts.atEnd() )
         {
            tmp = ts.readLine();
            pos++;
            if ( tmp.contains("Distance distribution  function of particle") ) 
            {
               editor->append("\nRecognized GNOM output.\n");
               startline = pos + 4;
               pop_last = 2;
               break;
            }
         }
         f.close();
         f.open(IO_ReadOnly);
      }

      QTextStream ts(&f);
      editor->append(QString("\nLoading pr(r) data from %1 %2\n").arg(filename).arg(res));
      editor->append(ts.readLine());
      while ( startline > 0)
      {
         ts.readLine();
         startline--;
      }
         
      while ( !ts.atEnd() )
      {
         ts >> new_r;
         ts >> new_pr;
         ts.readLine();
         r.push_back(new_r);
         pr.push_back(new_pr);
      }
      f.close();
      while ( pop_last > 0 && r.size() )
      {
         r.pop_back();
         pr.pop_back();
         pop_last--;
      }

      long ppr = plot_pr->insertCurve("p(r) vs r");
      plot_saxs->setCurveStyle(ppr, QwtCurve::Lines);
      plotted_r.push_back(r);
      normalize_pr(&pr);
      plotted_pr.push_back(pr);
      unsigned int p = plotted_r.size() - 1;

      plot_pr->setCurveData(ppr, (double *)&(r[0]), (double *)&(pr[0]), (int)pr.size());
      plot_pr->setCurvePen(ppr, QPen(plot_colors[p % plot_colors.size()], 2, SolidLine));
      plot_pr->replot();
   }
}

void US_Hydrodyn_Saxs::clear_plot_pr()
{
   plotted_pr.clear();
   plotted_r.clear();
   plot_pr->clear();
   plot_pr->replot();
}


//--------- thread for saxs I(q) plot -----------

// #define DEBUG_THREAD

saxs_Iq_thr_t::saxs_Iq_thr_t(int a_thread) : QThread()
{
   thread = a_thread;
   work_to_do = 0;
   work_done = 1;
   work_to_do_waiters = 0;
   work_done_waiters = 0;
}

void saxs_Iq_thr_t::saxs_Iq_thr_setup(
                                      vector < saxs_atom > *atoms,
                                      vector < vector < double > > *f,
                                      vector < vector < double > > *fc,
                                      vector < vector < double > > *fp,
                                      vector < double > *I,
                                      vector < double > *Ia,
                                      vector < double > *Ic,
                                      vector < double > *q,
                                      unsigned int threads,
                                      QProgressBar *progress,
                                      QLabel *lbl_core_progress,
                                      bool *stopFlag
                                      )
{
   /* this starts up a new work load for the thread */
   this->atoms = atoms;
   this->f = f;
   this->fc = fc;
   this->fp = fp;
   this->I = I;
   this->Ia = Ia;
   this->Ic = Ic;
   this->q = q;
   this->threads = threads;
   this->progress = progress;
   this->lbl_core_progress = lbl_core_progress;
   this->stopFlag = stopFlag;

   work_mutex.lock();
   work_to_do = 1;
   work_done = 0;
   work_mutex.unlock();
   cond_work_to_do.wakeOne();
#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " has new work to do\n";
#endif
}

void saxs_Iq_thr_t::saxs_Iq_thr_shutdown()
{
   /* this signals the thread to exit the run method */
   work_mutex.lock();
   work_to_do = -1;
   work_mutex.unlock();
   cond_work_to_do.wakeOne();

#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " shutdown requested\n";
#endif
}

void saxs_Iq_thr_t::saxs_Iq_thr_wait()
{
   /* this is for the master thread to wait until the work is done */
   work_mutex.lock();

#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " has a waiter\n";
#endif

   while(!work_done) {
      cond_work_done.wait(&work_mutex);
   }
   work_done = 0;
   work_mutex.unlock();

#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " waiter released\n";
#endif
}

int saxs_Iq_thr_t::saxs_Iq_thr_work_status()
{
   work_mutex.lock();
   int retval = work_done;
   work_mutex.unlock();
   return retval;
}

void saxs_Iq_thr_t::run()
{
   while(1)
   {
      work_mutex.lock();
#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " waiting for work\n";
#endif
      work_to_do_waiters++;
      while(!work_to_do)
      {
         cond_work_to_do.wait(&work_mutex);
      }
      if(work_to_do == -1)
      {
#if defined(DEBUG_THREAD)
         cerr << "thread " << thread << " shutting down\n";
#endif
         work_mutex.unlock();
         return;
      }

      work_to_do_waiters = 0;
      work_mutex.unlock();
#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " starting work\n";
#endif
      
      unsigned int as = (*atoms).size();
      unsigned int as1 = as - 1;
      double rik; // distance from atom i to k 
      double qrik; // q * rik
      double sqrikd; // sin * q * rik / qrik
      unsigned int q_points = (*q).size();
      if ( !thread ) 
      {
         progress->setTotalSteps((int)(1.15f * as1 / threads));
      }
#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " as1 = " << as1 
           << " q_points " << q_points
           << endl;
#endif

      for ( unsigned int i = thread; i < as1; i += threads )
      {
#if defined(DEBUG_THREAD)
         cerr << "thread " << thread << " i = " << i << endl;
#endif
         if ( !thread ) 
         {
            // lbl_core_progress->setText(QString("Atom %1 of %2\n").arg(i+1).arg(as));
            progress->setProgress(i+1);
            // qApp->processEvents();
         }
         if ( *stopFlag ) 
         {
            break;
         }
            
         for ( unsigned int k = i + 1; k < as; k++ )
         {
            rik = 
               sqrt(
                    ((*atoms)[i].pos[0] - (*atoms)[k].pos[0]) *
                    ((*atoms)[i].pos[0] - (*atoms)[k].pos[0]) +
                    ((*atoms)[i].pos[1] - (*atoms)[k].pos[1]) *
                    ((*atoms)[i].pos[1] - (*atoms)[k].pos[1]) +
                    ((*atoms)[i].pos[2] - (*atoms)[k].pos[2]) *
                    ((*atoms)[i].pos[2] - (*atoms)[k].pos[2])
                    );
#if defined(SAXS_DEBUG_F)
            cout << "dist atoms:  "
                 << i
                 << " "
                 << (*atoms)[i].saxs_name
                 << ","
                 << k
                 << " "
                 << (*atoms)[k].saxs_name
                 << " "
                 << rik
                 << endl;
#endif
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               qrik = rik * (*q)[j];
               sqrikd = sin(qrik) / qrik;
               (*I)[j] += (*fp)[j][i] * (*f)[j][k] * sqrikd;
               (*Ia)[j] += (*f)[j][i] * (*f)[j][k] * sqrikd;
               (*Ic)[j] += (*fc)[j][i] * (*fc)[j][k] * sqrikd;
            }
         }
      }

#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " finished work\n";
#endif
      work_mutex.lock();
      work_done = 1;
      work_to_do = 0;
      work_mutex.unlock();
      cond_work_done.wakeOne();
   }
}

//--------- end thread for saxs I(q) plot -----------

void US_Hydrodyn_Saxs::show_plot_saxs()
{
   // don't forget to later merge deleted waters into model_vector
   // right now we are going with first residue map entry
   stopFlag = false;
   pb_stop->setEnabled(true);
   pb_plot_saxs->setEnabled(false);
   pb_plot_pr->setEnabled(false);
   progress_saxs->reset();

#if defined(BUG_DEBUG)
   qApp->processEvents();
   cout << " sleep 1 a" << endl;
   sleep(1);
   cout << " sleep 1 a done" << endl;
#endif
   
   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
      current_model = selected_models[i];
#if defined(SAXS_DEBUG)
      printf("creating sax_atoms %u\n", current_model);
#endif
      editor->append(QString("\n\nPreparing file %1 model %2 for SAXS plot.\n\n")
                     .arg(te_filename2->text())
                     .arg(current_model + 1));
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor->append(tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
         
      vector < saxs_atom > atoms;
      saxs_atom new_atom;
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++)
      {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++)
         {
            PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
            new_atom.pos[0] = this_atom->coordinate.axis[0];
            new_atom.pos[1] = this_atom->coordinate.axis[1];
            new_atom.pos[2] = this_atom->coordinate.axis[2];

            QString mapkey = QString("%1|%2").arg(this_atom->resName).arg(this_atom->name);
            if ( this_atom->name == "OXT" )
            {
               mapkey = "OXT|OXT";
            }

            QString hybrid_name = residue_atom_hybrid_map[mapkey];

            if ( !hybrid_name || !hybrid_name.length() ) 
            {
               cout << "error: hybrid name missing for " << this_atom->resName << "|" << this_atom->name << endl; 
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq));
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            if ( !hybrid_map.count(hybrid_name) )
            {
               cout << "error: hybrid_map name missing for hybrid_name " << hybrid_name << endl;
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              );
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            if ( !atom_map.count(this_atom->name + "~" + hybrid_name) )
            {
               cout << "error: atom_map missing for hybrid_name "
                    << hybrid_name 
                    << " atom name "
                    << this_atom->name
                    << endl;
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->name)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              );
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            new_atom.excl_vol = atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol;

            new_atom.saxs_name = hybrid_map[hybrid_name].saxs_name; 

            if ( !saxs_map.count(hybrid_map[hybrid_name].saxs_name) )
            {
               cout << "error: saxs_map missing for hybrid_name "
                    << hybrid_name 
                    << " saxs name "
                    << hybrid_map[hybrid_name].saxs_name
                    << endl;
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              .arg(hybrid_map[hybrid_name].saxs_name)
                              );
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

#if defined(SAXS_DEBUG2)
            cout << "Atom: "
                 << this_atom->name
                 << " Residue: "
                 << this_atom->resName
                 << " SAXS atom: "
                 << new_atom.saxs_name 
                 << " Coordinates: "
                 << new_atom.pos[0] << " , "
                 << new_atom.pos[1] << " , "
                 << new_atom.pos[2] 
                 << " Excl vol: "
                 << new_atom.excl_vol
                 << endl;
#endif
            atoms.push_back(new_atom);
         }
      }
#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 b" << endl;
      sleep(1);
      cout << " sleep 1 b done" << endl;
#endif
      // ok now we have all the atoms
      unsigned int q_points = 
         (unsigned int)floor(((our_saxs_options->end_q - our_saxs_options->start_q) / our_saxs_options->delta_q) + .5) + 1;
         
      editor->append(QString("Number of atoms %1.\n"
                             "q range %2 to %3 with a stepsize of %4 giving %5 q-points.\n")
                     .arg(atoms.size())
                     .arg(our_saxs_options->start_q)
                     .arg(our_saxs_options->end_q)
                     .arg(our_saxs_options->delta_q)
                     .arg(q_points));
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor->append(tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
#if defined(SAXS_DEBUG)
      cout << "start q:" << our_saxs_options->start_q
           << " end q:" << our_saxs_options->end_q
           << " delta q:" << our_saxs_options->delta_q
           << " q points:" << q_points
           << endl;
#endif
      vector < vector < double > > f;  // f(q,i) / atomic
      vector < vector < double > > fc;  // excluded volume
      vector < vector < double > > fp;  // f - fc
      f.resize(q_points);
      fc.resize(q_points);
      fp.resize(q_points);
      vector < double > q;  // store q grid
      vector < double > q2; // store q^2
      q.resize(q_points);
      q2.resize(q_points);

      for ( unsigned int j = 0; j < q_points; j++ )
      {
         f[j].resize(atoms.size());
         fc[j].resize(atoms.size());
         fp[j].resize(atoms.size());
         q[j] = our_saxs_options->start_q + j * our_saxs_options->delta_q;
         if ( q[j] < SAXS_MIN_Q ) 
         {
            q[j] = SAXS_MIN_Q;
         }
         q2[j] = q[j] * q[j];
      }

      double m_pi_vi23; // - pi * pow(v,2/3)
      float vi; // excluded water vol
      float vie; // excluded water * e density

#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 c" << endl;
      sleep(1);
      cout << " sleep 1 c done" << endl;
#endif
#if defined(SAXS_DEBUG_F)
      cout << "atom #\tsaxs name\tq:";
      for ( unsigned int j = 0; j < q_points; j++ )
      {
         if (1 || (q[j] > .0099 && q[j] < .0101)) {
            cout << q[j] << "\t";
         }
      }
      cout << endl;
      cout << "\t\tq^2:";
      for ( unsigned int j = 0; j < q_points; j++ )
      {
         if (1 || (q[j] > .0099 && q[j] < .0101)) {
            cout << q2[j] << "\t";
         }
      }
      cout << endl;
#endif
      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {
         saxs saxs = saxs_map[atoms[i].saxs_name];
         vi = atoms[i].excl_vol;
         vie = vi * our_saxs_options->water_e_density;
         m_pi_vi23 = -M_PI * pow((double)vi,2.0/3.0); // - pi * pow(v,2/3)
#if defined(SAXS_DEBUG_F)
         cout << i << "\t"
              << atoms[i].saxs_name << "\t";
         cout << QString("").sprintf("a1 %f b1 %f a2 %f b2 %f a3 %f b3 %f a4 %f b4 %f c %f\n"
                                     , saxs.a[0] , saxs.b[0]
                                     , saxs.a[1] , saxs.b[1]
                                     , saxs.a[2] , saxs.b[2]
                                     , saxs.a[3] , saxs.b[3]
                                     , saxs.c);
#endif
         
         for ( unsigned int j = 0; j < q_points; j++ )
         {
            // note: since there are only a few 'saxs' coefficient sets
            // the saxs.c + saxs.a[i] * exp() can be precomputed
            // possibly saving time... but this isn't our most computationally intensive step
            // so I'm holding off for now.

            f[j][i] = saxs.c + 
               saxs.a[0] * exp(-saxs.b[0] * q2[j]) +
               saxs.a[1] * exp(-saxs.b[1] * q2[j]) +
               saxs.a[2] * exp(-saxs.b[2] * q2[j]) +
               saxs.a[3] * exp(-saxs.b[3] * q2[j]);
            fc[j][i] =  vie * exp(m_pi_vi23 * q2[j]);
            fp[j][i] = f[j][i] - fc[j][i];
#if defined(SAXS_DEBUG_F)
            if (1 || (q[j] > .0099 && q[j] < .0101)) {
               cout << q[j] 
                    << "\t" 
                    << q2[j] 
                    << "\t" 
                    << f[j][i]
                    << "\n";
            }
#endif
#if defined(SAXS_DEBUG_FV)
            if (1 || (q[j] > .0099 && q[j] < .0101)) {
               cout << q[j] 
                    << "\t" 
                    << q2[j] 
                    << "\t" 
                    << vi
                    << "\t" 
                    << vie
                    << "\t" 
                    << m_pi_vi23
                    << "\t" 
                    << m_pi_vi23 * q2[j]
                    << "\t" 
                    << vie * exp(m_pi_vi23 * q2[j])
                    << "\t" 
                    << fp[j][i]
                    << "\n";
            }
#endif
#if defined(ONLY_PHYSICAL_F)
            if ( fp[j][i] < 0.0f ) 
            {
               fp[j][i] = 0.0f;
            }
#endif
         }
#if defined(SAXS_DEBUG_F)
         cout << endl;
#endif
      }
#if defined(SAXS_DEBUG)
      cout << "f' computed, now compute I\n";
#endif
      editor->append("f' computed, starting computation of I(q)\n");
      qApp->processEvents();
#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 d" << endl;
      sleep(1);
      cout << " sleep 1 d done" << endl;
#endif
      if ( stopFlag ) 
      {
         editor->append(tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
      vector < double > I;
      vector < double > Ia;
      vector < double > Ic;
      I.resize(q_points);
      Ia.resize(q_points);
      Ic.resize(q_points);
      for ( unsigned int j = 0; j < q_points; j++ )
      {
         I[j] = 0.0f;
         Ia[j] = 0.0f;
         Ic[j] = 0.0f;
      }
#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 d.1" << endl;
      sleep(1);
      cout << " sleep 1 d.1 done" << endl;
#endif
      if ( 0 && // disabled for now
           ((US_Hydrodyn *)us_hydrodyn)->advanced_config.experimental_threads &&
           USglobal->config_list.numThreads > 1 )
      {
         unsigned int j;
         unsigned int threads = USglobal->config_list.numThreads;
         editor->append(QString("Using %1 threads.\n").arg(threads));
         vector < saxs_Iq_thr_t* > saxs_Iq_thr_threads(threads);
         for ( j = 0; j < threads; j++ )
         {
            saxs_Iq_thr_threads[j] = new saxs_Iq_thr_t(j);
            saxs_Iq_thr_threads[j]->start();
            
         }
         vector < vector < double > > It;
         vector < vector < double > > Ita;
         vector < vector < double > > Itc;
         It.resize(threads);
         Ita.resize(threads);
         Itc.resize(threads);
         for ( j = 0; j < threads; j++ )
         {
# if defined(DEBUG_THREAD)
            cout << "thread " << j << endl;
# endif
            It[j].resize(q_points);
            Ita[j].resize(q_points);
            Itc[j].resize(q_points);
            for ( unsigned int k = 0; k < q_points; k++ )
            {
               It[j][k] = 0.0f;
               Ita[j][k] = 0.0f;
               Itc[j][k] = 0.0f;
            }
            
            saxs_Iq_thr_threads[j]->saxs_Iq_thr_setup(
                                                      &atoms,
                                                      &f,
                                                      &fc,
                                                      &fp,
                                                      &It[j],
                                                      &Ita[j],
                                                      &Itc[j],
                                                      &q,
                                                      threads,
                                                      progress_saxs,
                                                      lbl_core_progress,
                                                      &stopFlag
                                                      );

         }
         // sleep app loop
         {
            int all_done;
#if !defined(WIN32)
            timespec ns;
            timespec ns_ret;
            ns.tv_sec = 0;
            ns.tv_nsec = 500000000l;
#endif
            do {
               all_done = threads;
               for ( j = 0; j < threads; j++ )
               {
                  all_done -= saxs_Iq_thr_threads[j]->saxs_Iq_thr_work_status();
               }
               qApp->processEvents();
#if defined(WIN32)
               _sleep(1);
#else
               nanosleep(&ns, &ns_ret);
#endif
            } while(all_done);
         }
         
         // wait for work to complete

         for ( j = 0; j < threads; j++ )
         {
            saxs_Iq_thr_threads[j]->saxs_Iq_thr_wait();
         }

         // destroy
         
         for ( j = 0; j < threads; j++ )
         {
            saxs_Iq_thr_threads[j]->saxs_Iq_thr_shutdown();
         }
         
         for ( j = 0; j < threads; j++ )
         {
            saxs_Iq_thr_threads[j]->wait();
         }
         
         for ( j = 0; j < threads; j++ )
         {
            delete saxs_Iq_thr_threads[j];
         }
         
         if ( stopFlag ) 
         {
            editor->append(tr("Terminated by user request.\n"));
            progress_saxs->reset();
            lbl_core_progress->setText("");
            pb_plot_saxs->setEnabled(true);
            pb_plot_pr->setEnabled(true);
            return;
         }

         // merge results
         for ( j = 0; j < threads; j++ )
         {
            for ( unsigned int k = 0; k < q_points; k++ )
            {
               I[k] += It[j][k];
               Ia[k] += Ita[j][k];
               Ic[k] += Itc[j][k];
            }
         }
      }
      else
      { 
         // not threaded
         unsigned int as = atoms.size();
         unsigned int as1 = as - 1;
         double rik; // distance from atom i to k 
         double qrik; // q * rik
         double sqrikd; // sin * q * rik / qrik
         progress_saxs->setTotalSteps((int)(as1 * 1.15));
         for ( unsigned int i = 0; i < as1; i++ )
         {
            // QString lcp = QString("Atom %1 of %2").arg(i+1).arg(as);
            // cout << lcp << endl;
            // lbl_core_progress->setText(lcp);
            progress_saxs->setProgress(i+1);
            qApp->processEvents();
            if ( stopFlag ) 
            {
               editor->append(tr("Terminated by user request.\n"));
               progress_saxs->reset();
               lbl_core_progress->setText("");
               pb_plot_saxs->setEnabled(true);
               pb_plot_pr->setEnabled(true);
               return;
            }
            for ( unsigned int k = i + 1; k < as; k++ )
            {
               rik = 
                  sqrt(
                       (atoms[i].pos[0] - atoms[k].pos[0]) *
                       (atoms[i].pos[0] - atoms[k].pos[0]) +
                       (atoms[i].pos[1] - atoms[k].pos[1]) *
                       (atoms[i].pos[1] - atoms[k].pos[1]) +
                       (atoms[i].pos[2] - atoms[k].pos[2]) *
                       (atoms[i].pos[2] - atoms[k].pos[2])
                       );
#if defined(SAXS_DEBUG_F)
               cout << "dist atoms:  "
                    << i
                    << " "
                    << atoms[i].saxs_name
                    << ","
                    << k
                    << " "
                    << atoms[k].saxs_name
                    << " "
                    << rik
                    << endl;
#endif
               for ( unsigned int j = 0; j < q_points; j++ )
               {
                  qrik = rik * q[j];
                  sqrikd = sin(qrik) / qrik;
                  I[j] += fp[j][i] * fp[j][k] * sqrikd;
                  Ia[j] += f[j][i] * f[j][k] * sqrikd;
                  Ic[j] += fc[j][i] * fc[j][k] * sqrikd;
#if defined(SAXS_DEBUG_F)
                  cout << QString("").sprintf("I[%f] += (%f * %f) * (sin(%f) / %f) == %f\n"
                                              , q[j]
                                              , fp[j][i]
                                              , fp[j][k]
                                              , qrik
                                              , qrik
                                              , I[j]);
#endif
                  
                  
                  
               }
            }
         }
      } // end threads logic

#if defined(I_MULT_2)
      for ( unsigned int j = 0; j < q_points; j++ )
      {
         I[j] *= 2; // we only computed one symmetric side
         Ia[j] *= 2; // we only computed one symmetric side
         Ic[j] *= 2; // we only computed one symmetric side
#if defined(SAXS_DEBUG_F)
         cout << QString("").sprintf("I[%f] = %f\n",
                                     q[j],
                                     I[j]);
#endif
      }
#endif
#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 d.2" << endl;
      sleep(1);
      cout << " sleep 1 d.2 done" << endl;
#endif
      lbl_core_progress->setText("");
      qApp->processEvents();
      progress_saxs->reset();
#if defined(SAXS_DEBUG)
      cout << "I computed\n";
#endif
      editor->append("I(q) computed.\n");
#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 e" << endl;
      sleep(1);
      cout << " sleep 1 e done" << endl;
#endif
      long Iq = plot_saxs->insertCurve("I(q) vs q");

      plot_saxs->setCurveStyle(Iq, QwtCurve::Lines);
      plotted_q.push_back(q);
      plotted_I.push_back(I);
      unsigned int p = plotted_q.size() - 1;
#if defined(SAXS_DEBUG)
      cout << "plot # " << p << endl;
#endif
      for ( unsigned int i = 0; i < plotted_I[p].size(); i++ ) 
      {
         plotted_I[p][i] = log10(plotted_I[p][i]);
      }
      plot_saxs->setCurveData(Iq, (double *)&(plotted_q[p][0]), (double *)&(plotted_I[p][0]), q_points);
      plot_saxs->setCurvePen(Iq, QPen(plot_colors[p % plot_colors.size()], 2, SolidLine));
      plot_saxs->replot();

      // save the data to a file
      QString fsaxs_name = 
         USglobal->config_list.root_dir + 
         "/somo/saxs/" + QString("%1").arg(te_filename2->text()) +
         QString("_%1").arg(current_model + 1) + 
         ".ssaxs";
#if defined(SAXS_DEBUG)
      cout << "output file " << fsaxs_name << endl;
#endif
      bool ok_to_write = true;
      if ( QFile::exists(fsaxs_name) )
      {
         switch( QMessageBox::information( this, 
                                           tr("Overwrite file:") + "SAXS P(r) vs. r" + tr("output file"),
                                           tr("The file named \"") + QString("%1").arg(te_filename2->text()) +
                                           QString("_%1").arg(current_model + 1) + 
                                           ".sprr" + tr("\" will be overwriten"),
                                           "&Ok",  "&Cancel", 0,
                                           0,      // Enter == button 0
                                           1 ) ) { // Escape == button 1
         case 0: // just go ahead
            ok_to_write = true;
            break;
         case 1: // Cancel clicked or Escape pressed
            ok_to_write = false;
            break;
         }
      }
      
      if ( ok_to_write )
      {
         FILE *fsaxs = fopen(fsaxs_name, "w");
         if ( fsaxs ) 
         {
#if defined(SAXS_DEBUG)
            cout << "writing " << fsaxs_name << endl;
#endif
            editor->append(tr("SAXS curve file: ") + fsaxs_name + tr(" created.\n"));
            fprintf(fsaxs,
                    "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                    , model_filename.ascii()
                    , US_Version.ascii()
                    , REVISION
                    , our_saxs_options->start_q
                    , our_saxs_options->end_q
                    , our_saxs_options->delta_q
                    );
            for ( unsigned int i = 0; i < q.size(); i++ )
            {
               fprintf(fsaxs, "%.6e\t%.6e\t%.6e\t%.6e\n", q[i], I[i], Ia[i], Ic[i]);
            }
            fclose(fsaxs);
         } 
         else
         {
#if defined(SAXS_DEBUG)
            cout << "can't create " << fsaxs_name << endl;
#endif
            editor->append(tr("WARNING: Could not create SAXS curve file: ") + fsaxs_name + "\n");
            QMessageBox mb(tr("UltraScan Warning"),
                           tr("The output file ") + fsaxs_name + tr(" could not be created."), 
                           QMessageBox::Critical,
                           QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
            mb.exec();
         }
      }
   }
   pb_plot_saxs->setEnabled(true);
   pb_plot_pr->setEnabled(true);
}

void US_Hydrodyn_Saxs::print()
{
   const int MARGIN = 10;
   printer.setPageSize(QPrinter::Letter);

   if ( printer.setup(this) ) {      // opens printer dialog
      QPainter p;
      p.begin( &printer );         // paint on printer
      p.setFont(editor->font() );
      int yPos      = 0;         // y position for each line
      QFontMetrics fm = p.fontMetrics();
      QPaintDeviceMetrics metrics( &printer ); // need width/height
      // of printer surface
      for( int i = 0 ; i < editor->lines() ; i++ ) {
         if ( MARGIN + yPos > metrics.height() - MARGIN ) {
            printer.newPage();      // no more room on this page
            yPos = 0;         // back to top of page
         }
         p.drawText( MARGIN, MARGIN + yPos,
                     metrics.width(), fm.lineSpacing(),
                                   ExpandTabs | DontClip,
                                   editor->text( i ) );
         yPos = yPos + fm.lineSpacing();
      }
      p.end();            // send job to printer
   }
}

void US_Hydrodyn_Saxs::load_saxs()
{
   QString filename = QFileDialog::getOpenFileName(USglobal->config_list.root_dir + "/somo/saxs", "*", this);
   if (filename.isEmpty())
   {
      return;
   }
   QFile f(filename);
   QString ext = QFileInfo(filename).extension(FALSE).lower();
   vector < double > I;
   vector < double > q;
   double new_I, new_q;
   unsigned int Icolumn = 1;
   bool dolog10 = false;
   QString res = "";
   if ( f.open(IO_ReadOnly) )
   {
      QTextStream ts(&f);
      if ( ext == "int" ) 
      {
         dolog10 = true;
         QStringList lst;
         lst << "I(q)   Difference intensity"
             << "Ia(q)  Atomic scattering"
             << "Ic(q)  Shape scattering"
             << "Ib(q)  Border layer scattering Ib(q)";
         bool ok;
         res = QInputDialog::getItem(
                                             "Crysol's .int format has four available datasets", 
                                             "Select the set you wish to plot::", lst, 0, FALSE, &ok,
                                             this );
         if ( ok ) {
            // user selected an item and pressed OK
            Icolumn = 0;
            if ( res.contains(QRegExp("^I.q. ")) ) 
            {
               Icolumn = 1;
            } 
            if ( res.contains(QRegExp("^Ia.q. ")) ) 
            {
               Icolumn = 2;
            } 
            if ( res.contains(QRegExp("^Ic.q. ")) ) 
            {
               Icolumn = 3;
            } 
            if ( res.contains(QRegExp("^Ib.q. ")) ) 
            {
               Icolumn = 4;
            } 
            if ( !Icolumn ) 
            {
               cerr << "US_Hydrodyn_Saxs::load_saxs : unknown type error" << endl;
               f.close();
               return;
            }
            cout << " column " << Icolumn << endl;
         } 
         else
         {
            f.close();
            return;
         }
      }
      if ( ext == "ssaxs" ) 
      {
         dolog10 = true;
         QStringList lst;
         lst << "I(q)   Difference intensity"
             << "Ia(q)  Atomic scattering"
             << "Ic(q)  Shape scattering";
         bool ok;
         res = QInputDialog::getItem(
                                             "There are three available datasets", 
                                             "Select the set you wish to plot::", lst, 0, FALSE, &ok,
                                             this );
         if ( ok ) {
            // user selected an item and pressed OK
            Icolumn = 0;
            if ( res.contains(QRegExp("^I.q. ")) ) 
            {
               Icolumn = 1;
            } 
            if ( res.contains(QRegExp("^Ia.q. ")) ) 
            {
               Icolumn = 2;
            } 
            if ( res.contains(QRegExp("^Ic.q. ")) ) 
            {
               Icolumn = 3;
            } 
            if ( !Icolumn ) 
            {
               cerr << "US_Hydrodyn_Saxs::load_saxs : unknown type error" << endl;
               f.close();
               return;
            }
            cout << " column " << Icolumn << endl;
         } 
         else
         {
            f.close();
            return;
         }
      }
      editor->append(QString("Loading SAXS data from %1 %2\n").arg(filename).arg(res));
      editor->append(ts.readLine());
      while ( !ts.atEnd() )
      {
         ts >> new_q;
         for ( unsigned int i = 0; i < Icolumn; i++ )
         {
            ts >> new_I;
         }
         if ( dolog10 )
         {
            new_I = log10(new_I);
         }
         ts.readLine();
         I.push_back(new_I);
         q.push_back(new_q);
      }
      f.close();
      long Iq = plot_saxs->insertCurve("I(q) vs q");
      plot_saxs->setCurveStyle(Iq, QwtCurve::Lines);
      plotted_q.push_back(q);
      plotted_I.push_back(I);
      unsigned int q_points = q.size();
      unsigned int p = plotted_q.size() - 1;
      plot_saxs->setCurveData(Iq, (double *)&(plotted_q[p][0]), (double *)&(plotted_I[p][0]), q_points);
      plot_saxs->setCurvePen(Iq, QPen(plot_colors[p % plot_colors.size()], 2, SolidLine));
      plot_saxs->replot();
   }
}

void US_Hydrodyn_Saxs::clear_plot_saxs()
{
   plotted_q.clear();
   plotted_I.clear();
   plot_saxs->clear();
   plot_saxs->replot();
}

void US_Hydrodyn_Saxs::clear_display()
{
   editor->clear();
}

void US_Hydrodyn_Saxs::update_font()
{
   bool ok;
   QFont newFont;
   newFont = QFontDialog::getFont( &ok, ft, this );
   if ( ok )
   {
      ft = newFont;
   }
   editor->setFont(ft);
}

void US_Hydrodyn_Saxs::save()
{
   QString fn;
   fn = QFileDialog::getSaveFileName(QString::null, QString::null,this );
   if(!fn.isEmpty() )
   {
      QString text = editor->text();
      QFile f( fn );
      if ( !f.open( IO_WriteOnly | IO_Translate) )
      {
         return;
      }
      QTextStream t( &f );
      t << text;
      f.close();
      editor->setModified( false );
      setCaption( fn );
   }
}

void US_Hydrodyn_Saxs::select_atom_file()
{
   QString old_filename = atom_filename;
   atom_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.atom *.ATOM", this);
   if (atom_filename.isEmpty())
   {
      atom_filename = old_filename;
      return;
   }
   else
   {
      select_atom_file(atom_filename);
   }
}

void US_Hydrodyn_Saxs::select_atom_file(const QString &filename)
{
   QString str1;
   QFileInfo fi(filename);
   lbl_atom_table->setText(fi.baseName() + "." + fi.extension());
   atom_list.clear();
   atom_map.clear();
   QFile f(filename);
   if (f.open(IO_ReadOnly|IO_Translate))
   {
      QTextStream ts(&f);
      while (!ts.atEnd())
      {
         ts >> current_atom.name;
         ts >> current_atom.hybrid.name;
         ts >> current_atom.hybrid.mw;
         ts >> current_atom.hybrid.radius;
         ts >> current_atom.saxs_excl_vol;
         str1 = ts.readLine(); // read rest of line
         if (!current_atom.name.isEmpty() && current_atom.hybrid.radius > 0.0 && current_atom.hybrid.mw > 0.0)
         {
            atom_list.push_back(current_atom);
            atom_map[current_atom.name + "~" + current_atom.hybrid.name] = current_atom;
         }
      }
      f.close();
   }
}

void US_Hydrodyn_Saxs::select_hybrid_file()
{
   QString old_filename = hybrid_filename;
   hybrid_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.hybrid *.HYBRID", this);
   if (hybrid_filename.isEmpty())
   {
      hybrid_filename = old_filename;
      return;
   }
   else
   {
      select_hybrid_file(hybrid_filename);
   }
}

void US_Hydrodyn_Saxs::select_hybrid_file(const QString &filename)
{
   QString str1;
   QFileInfo fi(filename);
   lbl_hybrid_table->setText(fi.baseName() + "." + fi.extension());
   QFile f(filename);
   hybrid_list.clear();
   hybrid_map.clear();
   if (f.open(IO_ReadOnly|IO_Translate))
   {
      QTextStream ts(&f);
      while (!ts.atEnd())
      {
         ts >> current_hybrid.saxs_name;
         ts >> current_hybrid.name;
         ts >> current_hybrid.mw;
         ts >> current_hybrid.radius;
         str1 = ts.readLine(); // read rest of line
         if (!current_hybrid.name.isEmpty() && current_hybrid.radius > 0.0 && current_hybrid.mw > 0.0)
         {
            hybrid_list.push_back(current_hybrid);
            hybrid_map[current_hybrid.name] = current_hybrid;
         }
      }
      f.close();
   }
}

void US_Hydrodyn_Saxs::select_saxs_file()
{
   QString old_filename = saxs_filename;
   saxs_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.saxs_atoms *.SAXS_ATOMS", this);
   if (saxs_filename.isEmpty())
   {
      saxs_filename = old_filename;
      return;
   }
   else
   {
      select_saxs_file(saxs_filename);
   }
}

void US_Hydrodyn_Saxs::select_saxs_file(const QString &filename)
{
   QString str1;
   QFileInfo fi(filename);
   lbl_saxs_table->setText(fi.baseName() + "." + fi.extension());
   QFile f(filename);
   saxs_list.clear();
   saxs_map.clear();
   if (f.open(IO_ReadOnly|IO_Translate))
   {
      QTextStream ts(&f);
      while (!ts.atEnd())
      {
         ts >> current_saxs.saxs_name;
         ts >> current_saxs.a[0];
         ts >> current_saxs.b[0];
         ts >> current_saxs.a[1];
         ts >> current_saxs.b[1];
         ts >> current_saxs.a[2];
         ts >> current_saxs.b[2];
         ts >> current_saxs.a[3];
         ts >> current_saxs.b[3];
         ts >> current_saxs.c;
         ts >> current_saxs.volume;
         str1 = ts.readLine(); // read rest of line
         if (!current_saxs.saxs_name.isEmpty())
         {
#if defined(SAXS_DEBUG)
            cout << "read saxs file: ["
                 << current_saxs.saxs_name
                 << "] a[0] = "
                 << current_saxs.a[0]
                 << endl;
#endif
#if defined(RESCALE_B)
            for ( unsigned int i = 0; i < 4; i++ )
            {
               current_saxs.b[i] /= 16.0f * M_PI * M_PI;
            }
#endif
            saxs_list.push_back(current_saxs);
            saxs_map[current_saxs.saxs_name] = current_saxs;
         }
      }
      f.close();
   }
}

