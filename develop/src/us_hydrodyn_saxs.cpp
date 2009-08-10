#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"

#include <time.h>

#define SAXS_DEBUG
#define USE_THREADS
// #define BUG_DEBUG

US_Hydrodyn_Saxs::US_Hydrodyn_Saxs(
                                   bool                           *saxs_widget,
                                   saxs_options                   *our_saxs_options,
                                   QString                        filename, 
                                   vector < residue >             residue_list,
                                   vector < PDB_model >           model_vector,
                                   vector < unsigned int >        selected_models,
                                   map < QString, vector <int> >  multi_residue_map,
                                   map < QString, QString >       residue_atom_hybrid_map,
                                   int                            source,
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
         sin(our_saxs_options->start_angle * M_PI / 180.0) / 
         our_saxs_options->wavelength;
      our_saxs_options->start_q =  floor(our_saxs_options->start_q * 1000.0 + 0.5) / 1000.0;
      our_saxs_options->end_q = 4.0 * M_PI * 
         sin(our_saxs_options->end_angle * M_PI / 180.0) / 
         our_saxs_options->wavelength;
      our_saxs_options->end_q =  floor(our_saxs_options->end_q * 1000.0 + 0.5) / 1000.0;
      our_saxs_options->delta_q = 4.0 * M_PI * 
         sin(our_saxs_options->delta_angle * M_PI / 180.0) / 
         our_saxs_options->wavelength;
      our_saxs_options->delta_q =  floor(our_saxs_options->delta_q * 1000.0 + 0.5) / 1000.0;
   }         

   this->residue_list = residue_list;
   this->model_vector = model_vector;
   this->selected_models = selected_models;
   this->multi_residue_map = multi_residue_map;
   this->residue_atom_hybrid_map = residue_atom_hybrid_map;
   this->source = source;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("SAXS Plotting Functions"));
   setupGUI();
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
   lbl_filename2->setText(fi.baseName() + "." + fi.extension( FALSE ));
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

   lbl_filename2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_filename2);
   lbl_filename2->setMinimumHeight(minHeight1);
   lbl_filename2->setAlignment(AlignLeft|AlignVCenter);
   lbl_filename2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_filename2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

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

   pb_plot_pr = new QPushButton(tr("Plot P(r) distribution"), this);
   Q_CHECK_PTR(pb_plot_pr);
   pb_plot_pr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_plot_pr->setMinimumHeight(minHeight1);
   pb_plot_pr->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_plot_pr, SIGNAL(clicked()), SLOT(show_plot_pr()));

   pb_plot_saxs = new QPushButton(tr("Plot SAXS Curve"), this);
   Q_CHECK_PTR(pb_plot_saxs);
   pb_plot_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_plot_saxs->setMinimumHeight(minHeight1);
   pb_plot_saxs->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_plot_saxs, SIGNAL(clicked()), SLOT(show_plot_saxs()));

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
   plot_saxs->setAxisTitle(QwtPlot::xBottom, tr("q"));
   plot_saxs->setAxisTitle(QwtPlot::yLeft, tr("I(q)"));
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
   plot_pr->setAxisTitle(QwtPlot::xBottom, tr("Distance (Angstrom))"));
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


   int rows=9, columns = 3, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   background->addMultiCellWidget(plot_saxs, j, j+6, 2, 2);
   j++;
   background->addWidget(lbl_filename1, j, 0);
   background->addWidget(lbl_filename2, j, 1);
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
   background->addWidget(pb_plot_pr, j, 0);
   background->addWidget(progress_pr, j, 1);
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
   background->setColStretch(0, 2);
   background->setColStretch(1, 2);
   background->setColStretch(2, 10);
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

void US_Hydrodyn_Saxs::show_plot_pr()
{
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
                                      vector < double > *I,
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
   this->I = I;
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
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               qrik = rik * (*q)[j];
               (*I)[j] += (*f)[j][i] * (*f)[j][k] + sin(qrik) / qrik;
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
                     .arg(lbl_filename2->text())
                     .arg(current_model + 1));
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor->append(tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs->setEnabled(true);
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
                  return;
               }
               continue;
            }

            if ( !atom_map.count(this_atom->name) )
            {
               cout << "error: atom_map missing for hybrid_name "
                    << hybrid_name 
                    << " atom name "
                    << this_atom->name
                    << endl;
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Atom file. Atom skipped.\n")
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
                  return;
               }
               continue;
            }

            new_atom.excl_vol = atom_map[this_atom->name].saxs_excl_vol;

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
         (unsigned int)floor(((our_saxs_options->end_q - our_saxs_options->start_q) / our_saxs_options->delta_q) + .5);
         
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
         return;
      }
#if defined(SAXS_DEBUG)
      cout << "start q:" << our_saxs_options->start_q
           << " end q:" << our_saxs_options->end_q
           << " delta q:" << our_saxs_options->delta_q
           << " q points:" << q_points
           << endl;
#endif
      vector < vector < double > > f;  // f(q,i)
      f.resize(q_points);
      vector < double > q;  // store q grid
      vector < double > q2; // store q^2
      q.resize(q_points);
      q2.resize(q_points);

      for ( unsigned int j = 0; j < q_points; j++ )
      {
         f[j].resize(atoms.size());
         q[j] = our_saxs_options->start_q + j * our_saxs_options->delta_q;
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

      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {
         saxs saxs = saxs_map[atoms[i].saxs_name];
         vi = atoms[i].excl_vol;
         vie = vi * our_saxs_options->water_e_density;
         m_pi_vi23 = -M_PI * pow(vi, 2.0/3.0); // - pi * pow(v,2/3)
         
         for ( unsigned int j = 0; j < q_points; j++ )
         {
            // note: since there are only a few 'saxs' coefficient sets
            // the saxs.c + saxs.a[i] * exp() can be precomputed
            // possibly saving time... but this isn't our most computational step
            // so I'm holding off for now.

            f[j][i] = saxs.c + 
               saxs.a[0] * exp(-saxs.b[0] * q2[j]) +
               saxs.a[1] * exp(-saxs.b[1] * q2[j]) +
               saxs.a[2] * exp(-saxs.b[2] * q2[j]) +
               saxs.a[3] * exp(-saxs.b[3] * q2[j]) -
               vie * exp(m_pi_vi23 * q2[j]);
         }
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
         return;
      }
      vector < double > I;
      I.resize(q_points);
      for ( unsigned int j = 0; j < q_points; j++ )
      {
         I[j] = 0;
      }
#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 d.1" << endl;
      sleep(1);
      cout << " sleep 1 d.1 done" << endl;
#endif
#if defined(USE_THREADS)
      {
         unsigned int j;
         unsigned int threads = USglobal->config_list.numThreads;
         vector < saxs_Iq_thr_t* > saxs_Iq_thr_threads(threads);
         for ( j = 0; j < threads; j++ )
         {
            saxs_Iq_thr_threads[j] = new saxs_Iq_thr_t(j);
            saxs_Iq_thr_threads[j]->start();
            
         }
         vector < vector < double > > It;
         It.resize(threads);
         for ( j = 0; j < threads; j++ )
         {
# if defined(DEBUG_THREAD)
            cout << "thread " << j << endl;
# endif
            It[j].resize(q_points);
            for ( unsigned int k = 0; k < q_points; k++ )
            {
               It[j][k] = 0;
            }
            
            saxs_Iq_thr_threads[j]->saxs_Iq_thr_setup(
                                                      &atoms,
                                                      &f,
                                                      &It[j],
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
            timespec ns;
            timespec ns_ret;
            ns.tv_sec = 0;
            ns.tv_nsec = 500000000l;

            do {
               all_done = threads;
               for ( j = 0; j < threads; j++ )
               {
                  all_done -= saxs_Iq_thr_threads[j]->saxs_Iq_thr_work_status();
               }
               qApp->processEvents();
               nanosleep(&ns, &ns_ret);
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
            return;
         }

         // merge results
         for ( j = 0; j < threads; j++ )
         {
            for ( unsigned int k = 0; k < q_points; k++ )
            {
               I[k] += It[j][k];
            }
         }
      }
      
#else
      unsigned int as = atoms.size();
      unsigned int as1 = as - 1;
      double rik; // distance from atom i to k 
      double qrik; // q * rik
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
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               qrik = rik * q[j];
               I[j] += f[j][i] * f[j][k] + sin(qrik) / qrik;
            }
         }
      }
      for ( unsigned int j = 0; j < q_points; j++ )
      {
         I[j] *= 2; // we only computed one symmetric side
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
   }
   pb_plot_saxs->setEnabled(true);
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
            atom_map[current_atom.name] = current_atom;
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
         ts >> current_saxs.a[1];
         ts >> current_saxs.a[2];
         ts >> current_saxs.a[3];
         ts >> current_saxs.b[0];
         ts >> current_saxs.b[1];
         ts >> current_saxs.b[2];
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
            saxs_list.push_back(current_saxs);
            saxs_map[current_saxs.saxs_name] = current_saxs;
         }
      }
      f.close();
   }
}

