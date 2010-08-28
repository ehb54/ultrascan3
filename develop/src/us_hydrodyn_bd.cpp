#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn_saxs_options.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"

#include <time.h>
#include <qstringlist.h>
#include <qinputdialog.h>
#include <qregexp.h>

US_Hydrodyn_BD::US_Hydrodyn_BD(
                               bool              *bd_widget, 
                               BD_Options        *our_bd_options,
                               Anaflex_Options   *our_anaflex_options,
                               void              *us_hydrodyn,
                               QWidget           *p, 
                               const char        *name
                               ) : QFrame(p, name)
{
   this->bd_widget = bd_widget;
   *bd_widget = true;
   this->our_bd_options = our_bd_options;
   this->our_anaflex_options = our_anaflex_options;

   this->us_hydrodyn = us_hydrodyn;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Brownian Dyanmics"));
   //   bd_ready_to_run = false;
   //   anaflex_ready_to_run = false;
   //   bd_last_file = "";
   //   anaflex_last_file = "";
   setupGUI();
   //   browflex = NULL;
   //   anaflex = NULL;
   //   anaflex_return_to_bd_load_results = false;
   //   ((US_Hydrodyn *)us_hydrodyn)->bd_window = this;
   //   ((US_Hydrodyn *)us_hydrodyn)->bd_anaflex_enables(false);
   //   editor->append("\n\n");
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
   stopFlag = false;
   //   pb_stop->setEnabled(false);
}

US_Hydrodyn_BD::~US_Hydrodyn_BD()
{
   *bd_widget = false;
}

void US_Hydrodyn_BD::setupGUI()
{
   int minHeight1 = 30;
   lbl_info = new QLabel(tr("Brownian Dynamics Functions:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_browflex = new QLabel(tr("Browflex:"), this);
   Q_CHECK_PTR(lbl_browflex);
   lbl_browflex->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_browflex->setAlignment(AlignCenter|AlignVCenter);
   lbl_browflex->setMinimumHeight(minHeight1);
   lbl_browflex->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_browflex->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_anaflex = new QLabel(tr("Anaflex:"), this);
   Q_CHECK_PTR(lbl_anaflex);
   lbl_anaflex->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_anaflex->setAlignment(AlignCenter|AlignVCenter);
   lbl_anaflex->setMinimumHeight(minHeight1);
   lbl_anaflex->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_anaflex->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   pb_bd_prepare = new QPushButton(tr("Create files"), this);
   pb_bd_prepare->setMinimumHeight(minHeight1);
   pb_bd_prepare->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_bd_prepare->setEnabled(false);
   pb_bd_prepare->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_bd_prepare, SIGNAL(clicked()), SLOT(bd_prepare()));

   pb_bd_load = new QPushButton(tr("Load files"), this);
   pb_bd_load->setMinimumHeight(minHeight1);
   pb_bd_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_bd_load->setEnabled(false);
   pb_bd_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_bd_load, SIGNAL(clicked()), SLOT(bd_load()));

   pb_bd_edit = new QPushButton(tr("View/Edit"), this);
   pb_bd_edit->setMinimumHeight(minHeight1);
   pb_bd_edit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_bd_edit->setEnabled(false);
   pb_bd_edit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_bd_edit, SIGNAL(clicked()), SLOT(bd_edit()));

   pb_bd_run = new QPushButton(tr("Run"), this);
   pb_bd_run->setMinimumHeight(minHeight1);
   pb_bd_run->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_bd_run->setEnabled(false);
   pb_bd_run->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_bd_run, SIGNAL(clicked()), SLOT(bd_run()));

   pb_bd_load_results = new QPushButton(tr("Load/Process results"), this);
   pb_bd_load_results->setMinimumHeight(minHeight1);
   pb_bd_load_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_bd_load_results->setEnabled(false);
   pb_bd_load_results->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_bd_load_results, SIGNAL(clicked()), SLOT(bd_load_results()));

   // ***** anaflex *******
   pb_anaflex_prepare = new QPushButton(tr("Create files"), this);
   pb_anaflex_prepare->setMinimumHeight(minHeight1);
   pb_anaflex_prepare->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_anaflex_prepare->setEnabled(false);
   pb_anaflex_prepare->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_anaflex_prepare, SIGNAL(clicked()), SLOT(anaflex_prepare()));

   pb_anaflex_load = new QPushButton(tr("Load files"), this);
   pb_anaflex_load->setMinimumHeight(minHeight1);
   pb_anaflex_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_anaflex_load->setEnabled(false);
   pb_anaflex_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_anaflex_load, SIGNAL(clicked()), SLOT(anaflex_load()));

   pb_anaflex_edit = new QPushButton(tr("View/Edit"), this);
   pb_anaflex_edit->setMinimumHeight(minHeight1);
   pb_anaflex_edit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_anaflex_edit->setEnabled(false);
   pb_anaflex_edit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_anaflex_edit, SIGNAL(clicked()), SLOT(anaflex_edit()));

   pb_anaflex_run = new QPushButton(tr("Run"), this);
   pb_anaflex_run->setMinimumHeight(minHeight1);
   pb_anaflex_run->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_anaflex_run->setEnabled(false);
   pb_anaflex_run->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_anaflex_run, SIGNAL(clicked()), SLOT(anaflex_run()));

   pb_anaflex_load_results = new QPushButton(tr("Load results"), this);
   pb_anaflex_load_results->setMinimumHeight(minHeight1);
   pb_anaflex_load_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_anaflex_load_results->setEnabled(false);
   pb_anaflex_load_results->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_anaflex_load_results, SIGNAL(clicked()), SLOT(anaflex_load_results()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   //   pb_stop = new QPushButton(tr("Stop"), this);
   //   Q_CHECK_PTR(pb_stop);
   //   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   //   pb_stop->setMinimumHeight(minHeight1);
   //   pb_stop->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   pb_bd_options = new QPushButton(tr("Browflex Options"), this);
   Q_CHECK_PTR(pb_bd_options);
   pb_bd_options->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_bd_options->setMinimumHeight(minHeight1);
   pb_bd_options->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_bd_options, SIGNAL(clicked()), SLOT(bd_options()));

   pb_anaflex_options = new QPushButton(tr("Anaflex Options"), this);
   Q_CHECK_PTR(pb_anaflex_options);
   pb_anaflex_options->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_anaflex_options->setMinimumHeight(minHeight1);
   pb_anaflex_options->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_anaflex_options, SIGNAL(clicked()), SLOT(anaflex_options()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   // progress = new QProgressBar(this, "Progress");
   // progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   // progress->reset();

   // editor = new QTextEdit(this);
   // editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   // editor->setReadOnly(true);
   // editor->setMinimumWidth(300);
   // editor->setMinimumHeight(minHeight1 * 6);
   // m = new QMenuBar(editor, "menu" );
   // m->setMinimumHeight(minHeight1);
   // m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   // QPopupMenu * file = new QPopupMenu(editor);
   // m->insertItem( tr("&File"), file );
   // file->insertItem( tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   // file->insertItem( tr("Save"),  this, SLOT(save()),    ALT+Key_S );
   // file->insertItem( tr("Print"), this, SLOT(print()),   ALT+Key_P );
   // file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
   // editor->setWordWrap (QTextEdit::WidgetWidth);
   // // editor->setWordWrap (QTextEdit::NoWrap);

   int rows=0, columns = 0, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(lbl_browflex, j, j, 0, 1);
   j++;
   background->addWidget(pb_bd_prepare, j, 0);
   background->addWidget(pb_bd_load, j, 1);
   j++;
   background->addWidget(pb_bd_run, j, 0);
   background->addWidget(pb_bd_edit, j, 1);
   j++;
   background->addWidget(pb_bd_options, j, 0);
   background->addWidget(pb_bd_load_results, j, 1);
   j++;
   background->addMultiCellWidget(lbl_anaflex, j, j, 0, 1);
   j++;
   background->addWidget(pb_anaflex_prepare, j, 0);
   background->addWidget(pb_anaflex_load, j, 1);
   j++;
   background->addWidget(pb_anaflex_run, j, 0);
   background->addWidget(pb_anaflex_edit, j, 1);
   j++;
   background->addWidget(pb_anaflex_options, j, 0);
   background->addWidget(pb_anaflex_load_results, j, 1);
   j++;
   //   background->addWidget(pb_stop, j, 0);
   //   background->addWidget(progress, j, 1);
   //   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
   //   background->addMultiCellWidget(editor, 0, j, 2, 3);
}

void US_Hydrodyn_BD::cancel()
{
   close();
}

void US_Hydrodyn_BD::bd_options()
{
   ((US_Hydrodyn *)us_hydrodyn)->show_bd_options();
}

void US_Hydrodyn_BD::anaflex_options()
{
   ((US_Hydrodyn *)us_hydrodyn)->show_anaflex_options();
}

void US_Hydrodyn_BD::bd_prepare()
{
   ((US_Hydrodyn *)us_hydrodyn)->bd_prepare();
}

void US_Hydrodyn_BD::bd_load()
{
   ((US_Hydrodyn *)us_hydrodyn)->bd_load();
}

void US_Hydrodyn_BD::bd_edit()
{
   ((US_Hydrodyn *)us_hydrodyn)->bd_edit();
}

void US_Hydrodyn_BD::bd_run()
{
   ((US_Hydrodyn *)us_hydrodyn)->bd_run();
}

void US_Hydrodyn_BD::bd_load_results()
{
   ((US_Hydrodyn *)us_hydrodyn)->bd_load_results();
}

void US_Hydrodyn_BD::anaflex_prepare()
{
   ((US_Hydrodyn *)us_hydrodyn)->anaflex_prepare();
}

void US_Hydrodyn_BD::anaflex_load()
{
   ((US_Hydrodyn *)us_hydrodyn)->anaflex_load();
}

void US_Hydrodyn_BD::anaflex_edit()
{
   ((US_Hydrodyn *)us_hydrodyn)->anaflex_edit();
}

void US_Hydrodyn_BD::anaflex_run()
{
   ((US_Hydrodyn *)us_hydrodyn)->anaflex_run();
}

void US_Hydrodyn_BD::anaflex_load_results()
{
   ((US_Hydrodyn *)us_hydrodyn)->anaflex_load_results();
}

void US_Hydrodyn_BD::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_bd.html");
}

void US_Hydrodyn_BD::stop()
{
   stopFlag = true;
   ((US_Hydrodyn *)us_hydrodyn)->stop_calc();
   //   anaflex_return_to_bd_load_results = false;
   //   if ( browflex && browflex->isRunning() )
   //   {
   //      browflex->tryTerminate();
   //      QTimer::singleShot( 1000, browflex, SLOT( kill() ) );
   //   }
   //   if ( anaflex && anaflex->isRunning() )
   //   {
   //      anaflex->tryTerminate();
   //      QTimer::singleShot( 1000, anaflex, SLOT( kill() ) );
   //   }
   //   pb_stop->setEnabled(false);
}

void US_Hydrodyn_BD::closeEvent(QCloseEvent *e)
{
   *bd_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

// void US_Hydrodyn_BD::clear_display()
// {
//    editor->clear();
// }

// void US_Hydrodyn_BD::update_font()
// {
//    bool ok;
//    QFont newFont;
//    newFont = QFontDialog::getFont( &ok, ft, this );
//    if ( ok )
//    {
//       ft = newFont;
//    }
//    editor->setFont(ft);
// }

// void US_Hydrodyn_BD::save()
// {
//    QString fn;
//    fn = QFileDialog::getSaveFileName(QString::null, QString::null,this );
//    if(!fn.isEmpty() )
//    {
//       QString text = editor->text();
//       QFile f( fn );
//       if ( !f.open( IO_WriteOnly | IO_Translate) )
//       {
//          return;
//       }
//       QTextStream t( &f );
//       t << text;
//       f.close();
//       editor->setModified( false );
//       setCaption( fn );
//    }
// }

// void US_Hydrodyn_BD::print()
// {
//    const int MARGIN = 10;
//    printer.setPageSize(QPrinter::Letter);
// 
//    if ( printer.setup(this) ) {      // opens printer dialog
//       QPainter p;
//       p.begin( &printer );         // paint on printer
//       p.setFont(editor->font() );
//       int yPos      = 0;         // y position for each line
//       QFontMetrics fm = p.fontMetrics();
//       QPaintDeviceMetrics metrics( &printer ); // need width/height
//       // of printer surface
//       for( int i = 0 ; i < editor->lines() ; i++ ) {
//          if ( MARGIN + yPos > metrics.height() - MARGIN ) {
//             printer.newPage();      // no more room on this page
//             yPos = 0;         // back to top of page
//          }
//          p.drawText( MARGIN, MARGIN + yPos,
//                      metrics.width(), fm.lineSpacing(),
//                                    ExpandTabs | DontClip,
//                                    editor->text( i ) );
//          yPos = yPos + fm.lineSpacing();
//       }
//       p.end();            // send job to printer
//    }
// }

// void US_Hydrodyn_BD::editor_msg( QString color, QString msg )
// {
//    QColor save_color = editor->color();
//    editor->setColor(color);
//    editor->append(msg);
//    editor->setColor(save_color);
// }

// void US_Hydrodyn_BD::printError(const QString &str)
// {
//    QMessageBox::warning(this, tr("UltraScan Warning"), tr("Please note:\n\n") +
//                         tr(str), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
// }
