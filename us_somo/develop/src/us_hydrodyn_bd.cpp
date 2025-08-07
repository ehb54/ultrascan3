#include <qinputdialog.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <time.h>

#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn_saxs_options.h"
#include "../include/us_revision.h"
// Added by qt3to4:
#include <QCloseEvent>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>

US_Hydrodyn_BD::US_Hydrodyn_BD(bool *bd_widget, BD_Options *our_bd_options,
                               Anaflex_Options *our_anaflex_options,
                               void *us_hydrodyn, QWidget *p, const char *)
    : QFrame(p) {
  this->bd_widget = bd_widget;
  *bd_widget = true;
  this->our_bd_options = our_bd_options;
  this->our_anaflex_options = our_anaflex_options;

  this->us_hydrodyn = us_hydrodyn;
  USglobal = new US_Config();
  setPalette(PALET_FRAME);
  setWindowTitle(us_tr("Brownian Dyanmics"));
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
#if QT_VERSION >= 0x040000
  setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
#else
  setWFlags(getWFlags() | Qt::WStyle_StaysOnTop);
#endif
  stopFlag = false;
  //   pb_stop->setEnabled(false);
}

US_Hydrodyn_BD::~US_Hydrodyn_BD() { *bd_widget = false; }

void US_Hydrodyn_BD::setupGUI() {
  int minHeight1 = 30;
  lbl_info = new QLabel(us_tr("Brownian Dynamics Functions:"), this);
  Q_CHECK_PTR(lbl_info);
  lbl_info->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_info->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_info->setMinimumHeight(minHeight1);
  lbl_info->setPalette(PALET_FRAME);
  AUTFBACK(lbl_info);
  lbl_info->setFont(QFont(USglobal->config_list.fontFamily,
                          USglobal->config_list.fontSize + 1, QFont::Bold));

  lbl_credits = new QLabel(
      "SIMUFLEX BD suite, by J. Garcia de la Torre et al. (JCTC 5:2606-18, "
      "2009)",
      this);
  Q_CHECK_PTR(lbl_credits);
  lbl_credits->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_credits->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_credits->setMinimumHeight(minHeight1);
  lbl_credits->setPalette(PALET_FRAME);
  AUTFBACK(lbl_credits);
  lbl_credits->setFont(QFont(USglobal->config_list.fontFamily,
                             USglobal->config_list.fontSize + 1, QFont::Bold));

  lbl_browflex = new QLabel(us_tr("Browflex:"), this);
  Q_CHECK_PTR(lbl_browflex);
  lbl_browflex->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_browflex->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_browflex->setMinimumHeight(minHeight1);
  lbl_browflex->setPalette(PALET_FRAME);
  AUTFBACK(lbl_browflex);
  lbl_browflex->setFont(QFont(USglobal->config_list.fontFamily,
                              USglobal->config_list.fontSize + 1, QFont::Bold));

  lbl_anaflex = new QLabel(us_tr("Anaflex:"), this);
  Q_CHECK_PTR(lbl_anaflex);
  lbl_anaflex->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_anaflex->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_anaflex->setMinimumHeight(minHeight1);
  lbl_anaflex->setPalette(PALET_FRAME);
  AUTFBACK(lbl_anaflex);
  lbl_anaflex->setFont(QFont(USglobal->config_list.fontFamily,
                             USglobal->config_list.fontSize + 1, QFont::Bold));

  pb_bd_prepare = new QPushButton(us_tr("Create files"), this);
  pb_bd_prepare->setMinimumHeight(minHeight1);
  pb_bd_prepare->setFont(QFont(USglobal->config_list.fontFamily,
                               USglobal->config_list.fontSize + 1));
  pb_bd_prepare->setEnabled(false);
  pb_bd_prepare->setPalette(PALET_PUSHB);
  connect(pb_bd_prepare, SIGNAL(clicked()), SLOT(bd_prepare()));

  pb_bd_load = new QPushButton(us_tr("Load files"), this);
  pb_bd_load->setMinimumHeight(minHeight1);
  pb_bd_load->setFont(QFont(USglobal->config_list.fontFamily,
                            USglobal->config_list.fontSize + 1));
  pb_bd_load->setEnabled(false);
  pb_bd_load->setPalette(PALET_PUSHB);
  connect(pb_bd_load, SIGNAL(clicked()), SLOT(bd_load()));

  pb_bd_edit = new QPushButton(us_tr("View/Edit"), this);
  pb_bd_edit->setMinimumHeight(minHeight1);
  pb_bd_edit->setFont(QFont(USglobal->config_list.fontFamily,
                            USglobal->config_list.fontSize + 1));
  pb_bd_edit->setEnabled(false);
  pb_bd_edit->setPalette(PALET_PUSHB);
  connect(pb_bd_edit, SIGNAL(clicked()), SLOT(bd_edit()));

  pb_bd_run = new QPushButton(us_tr("Run"), this);
  pb_bd_run->setMinimumHeight(minHeight1);
  pb_bd_run->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize + 1));
  pb_bd_run->setEnabled(false);
  pb_bd_run->setPalette(PALET_PUSHB);
  connect(pb_bd_run, SIGNAL(clicked()), SLOT(bd_run()));

  pb_bd_load_results = new QPushButton(us_tr("Load/Process results"), this);
  pb_bd_load_results->setMinimumHeight(minHeight1);
  pb_bd_load_results->setFont(QFont(USglobal->config_list.fontFamily,
                                    USglobal->config_list.fontSize + 1));
  pb_bd_load_results->setEnabled(false);
  pb_bd_load_results->setPalette(PALET_PUSHB);
  connect(pb_bd_load_results, SIGNAL(clicked()), SLOT(bd_load_results()));

  // ***** anaflex *******
  pb_anaflex_prepare = new QPushButton(us_tr("Create files"), this);
  pb_anaflex_prepare->setMinimumHeight(minHeight1);
  pb_anaflex_prepare->setFont(QFont(USglobal->config_list.fontFamily,
                                    USglobal->config_list.fontSize + 1));
  pb_anaflex_prepare->setEnabled(false);
  pb_anaflex_prepare->setPalette(PALET_PUSHB);
  connect(pb_anaflex_prepare, SIGNAL(clicked()), SLOT(anaflex_prepare()));

  pb_anaflex_load = new QPushButton(us_tr("Load files"), this);
  pb_anaflex_load->setMinimumHeight(minHeight1);
  pb_anaflex_load->setFont(QFont(USglobal->config_list.fontFamily,
                                 USglobal->config_list.fontSize + 1));
  pb_anaflex_load->setEnabled(false);
  pb_anaflex_load->setPalette(PALET_PUSHB);
  connect(pb_anaflex_load, SIGNAL(clicked()), SLOT(anaflex_load()));

  pb_anaflex_edit = new QPushButton(us_tr("View/Edit"), this);
  pb_anaflex_edit->setMinimumHeight(minHeight1);
  pb_anaflex_edit->setFont(QFont(USglobal->config_list.fontFamily,
                                 USglobal->config_list.fontSize + 1));
  pb_anaflex_edit->setEnabled(false);
  pb_anaflex_edit->setPalette(PALET_PUSHB);
  connect(pb_anaflex_edit, SIGNAL(clicked()), SLOT(anaflex_edit()));

  pb_anaflex_run = new QPushButton(us_tr("Run"), this);
  pb_anaflex_run->setMinimumHeight(minHeight1);
  pb_anaflex_run->setFont(QFont(USglobal->config_list.fontFamily,
                                USglobal->config_list.fontSize + 1));
  pb_anaflex_run->setEnabled(false);
  pb_anaflex_run->setPalette(PALET_PUSHB);
  connect(pb_anaflex_run, SIGNAL(clicked()), SLOT(anaflex_run()));

  pb_anaflex_load_results = new QPushButton(us_tr("Load results"), this);
  pb_anaflex_load_results->setMinimumHeight(minHeight1);
  pb_anaflex_load_results->setFont(QFont(USglobal->config_list.fontFamily,
                                         USglobal->config_list.fontSize + 1));
  pb_anaflex_load_results->setEnabled(false);
  pb_anaflex_load_results->setPalette(PALET_PUSHB);
  connect(pb_anaflex_load_results, SIGNAL(clicked()),
          SLOT(anaflex_load_results()));

  pb_cancel = new QPushButton(us_tr("Close"), this);
  Q_CHECK_PTR(pb_cancel);
  pb_cancel->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize + 1));
  pb_cancel->setMinimumHeight(minHeight1);
  pb_cancel->setPalette(PALET_PUSHB);
  connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

  //   pb_stop = new QPushButton(us_tr("Stop"), this);
  //   Q_CHECK_PTR(pb_stop);
  //   pb_stop->setFont(QFont( USglobal->config_list.fontFamily,
  //   USglobal->config_list.fontSize + 1));
  //   pb_stop->setMinimumHeight(minHeight1);
  //   pb_stop->setPalette( PALET_PUSHB );
  //   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

  pb_bd_options = new QPushButton(us_tr("Browflex Options"), this);
  Q_CHECK_PTR(pb_bd_options);
  pb_bd_options->setFont(QFont(USglobal->config_list.fontFamily,
                               USglobal->config_list.fontSize + 1));
  pb_bd_options->setMinimumHeight(minHeight1);
  pb_bd_options->setPalette(PALET_PUSHB);
  connect(pb_bd_options, SIGNAL(clicked()), SLOT(bd_options()));

  pb_anaflex_options = new QPushButton(us_tr("Anaflex Options"), this);
  Q_CHECK_PTR(pb_anaflex_options);
  pb_anaflex_options->setFont(QFont(USglobal->config_list.fontFamily,
                                    USglobal->config_list.fontSize + 1));
  pb_anaflex_options->setMinimumHeight(minHeight1);
  pb_anaflex_options->setPalette(PALET_PUSHB);
  connect(pb_anaflex_options, SIGNAL(clicked()), SLOT(anaflex_options()));

  pb_help = new QPushButton(us_tr("Help"), this);
  Q_CHECK_PTR(pb_help);
  pb_help->setFont(QFont(USglobal->config_list.fontFamily,
                         USglobal->config_list.fontSize + 1));
  pb_help->setMinimumHeight(minHeight1);
  pb_help->setPalette(PALET_PUSHB);
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));

  // progress = new QProgressBar(this, "Progress");
  // progress->setPalette( PALET_NORMAL );
  // progress->reset();

  // editor = new QTextEdit(this);
  // editor->setPalette( PALET_NORMAL );
  // editor->setReadOnly(true);
  // editor->setMinimumWidth(300);
  // editor->setMinimumHeight(minHeight1 * 6);
  // m = new QMenuBar( editor );  m->setObjectName( "menu" );
  // m->setMinimumHeight(minHeight1);
  // m->setPalette( PALET_NORMAL );
  // QPopupMenu * file = new QPopupMenu(editor);
  // m->insertItem( us_tr("&File"), file );
  // file->insertItem( us_tr("Font"),  this, SLOT(update_font()),    ALT+Key_F
  // ); file->insertItem( us_tr("Save"),  this, SLOT(save()),    ALT+Key_S );
  // file->insertItem( us_tr("Print"), this, SLOT(print()),   ALT+Key_P );
  // file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),
  // ALT+Key_X ); editor->setWordWrapMode (QTextOption::WordWrap);
  // // editor->setWordWrapMode (QTextOption::NoWrap);

  int /* rows=0, columns = 0, */ spacing = 2, j = 0, margin = 4;
  QGridLayout *background = new QGridLayout(this);
  background->setContentsMargins(0, 0, 0, 0);
  background->setSpacing(0);
  background->setSpacing(spacing);
  background->setContentsMargins(margin, margin, margin, margin);

  background->addWidget(lbl_info, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;
  background->addWidget(lbl_credits, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;
  background->addWidget(lbl_browflex, j, 0, 1 + (j) - (j), 1 + (1) - (0));
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
  background->addWidget(lbl_anaflex, j, 0, 1 + (j) - (j), 1 + (1) - (0));
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
  //   background->addWidget( editor , 0 , 2 , 1 + ( j ) - ( 0 ) , 1 + ( 3 ) - (
  //   2 ) );
}

void US_Hydrodyn_BD::cancel() { close(); }

void US_Hydrodyn_BD::bd_options() {
  ((US_Hydrodyn *)us_hydrodyn)->show_bd_options();
}

void US_Hydrodyn_BD::anaflex_options() {
  ((US_Hydrodyn *)us_hydrodyn)->show_anaflex_options();
}

void US_Hydrodyn_BD::bd_prepare() {
  ((US_Hydrodyn *)us_hydrodyn)->bd_prepare();
}

void US_Hydrodyn_BD::bd_load() { ((US_Hydrodyn *)us_hydrodyn)->bd_load(); }

void US_Hydrodyn_BD::bd_edit() { ((US_Hydrodyn *)us_hydrodyn)->bd_edit(); }

void US_Hydrodyn_BD::bd_run() { ((US_Hydrodyn *)us_hydrodyn)->bd_run(); }

void US_Hydrodyn_BD::bd_load_results() {
  ((US_Hydrodyn *)us_hydrodyn)->bd_load_results();
}

void US_Hydrodyn_BD::anaflex_prepare() {
  ((US_Hydrodyn *)us_hydrodyn)->anaflex_prepare();
}

void US_Hydrodyn_BD::anaflex_load() {
  ((US_Hydrodyn *)us_hydrodyn)->anaflex_load();
}

void US_Hydrodyn_BD::anaflex_edit() {
  ((US_Hydrodyn *)us_hydrodyn)->anaflex_edit();
}

void US_Hydrodyn_BD::anaflex_run() {
  ((US_Hydrodyn *)us_hydrodyn)->anaflex_run();
}

void US_Hydrodyn_BD::anaflex_load_results() {
  ((US_Hydrodyn *)us_hydrodyn)->anaflex_load_results();
}

void US_Hydrodyn_BD::help() {
  US_Help *online_help;
  online_help = new US_Help(this);
  online_help->show_help("manual/somo/somo_bd.html");
}

void US_Hydrodyn_BD::stop() {
  stopFlag = true;
  ((US_Hydrodyn *)us_hydrodyn)->stop_calc();
  //   anaflex_return_to_bd_load_results = false;
  //   if ( browflex && browflex->state() == QProcess::Running )
  //   {
  //      browflex->terminate();
  //      QTimer::singleShot( 1000, browflex, SLOT( kill() ) );
  //   }
  //   if ( anaflex && anaflex->state() == QProcess::Running )
  //   {
  //      anaflex->terminate();
  //      QTimer::singleShot( 1000, anaflex, SLOT( kill() ) );
  //   }
  //   pb_stop->setEnabled(false);
}

void US_Hydrodyn_BD::closeEvent(QCloseEvent *e) {
  *bd_widget = false;
  global_Xpos -= 30;
  global_Ypos -= 30;
  e->accept();
}

// void US_Hydrodyn_BD::clear_display()
// {
//    editor->clear( );
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
//    fn = QFileDialog::getSaveFileName(QString(), QString(),this );
//    if(!fn.isEmpty() )
//    {
//       QString text = editor->toPlainText();
//       QFile f( fn );
//       if ( !f.open( IO_WriteOnly | IO_Translate) )
//       {
//          return;
//       }
//       QTextStream t( &f );
//       t << text;
//       f.close();
////       editor->setModified( false );
//       setWindowTitle( fn );
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
//          if ( MARGIN + yPos > printer.height() - MARGIN ) {
//             printer.newPage();      // no more room on this page
//             yPos = 0;         // back to top of page
//          }
//          p.drawText( MARGIN, MARGIN + yPos,
//                      printer.width(), fm.lineSpacing(),
//                                    ExpandTabs | DontClip,
//                                    editor->toPlainText( i ) );
//          yPos = yPos + fm.lineSpacing();
//       }
//       p.end();            // send job to printer
//    }
// }

// void US_Hydrodyn_BD::editor_msg( QString color, QString msg )
// {
//    QColor save_color = editor->textColor();
//    editor->setTextColor(color);
//    editor->append(msg);
//    editor->setTextColor(save_color);
// }

// void US_Hydrodyn_BD::printError(const QString &str)
// {
//    QMessageBox::warning(this, us_tr("UltraScan Warning"), us_tr("Please
//    note:\n\n") +
//                         us_tr(str), QMessageBox::Ok, QMessageBox::NoButton,
//                         QMessageBox::NoButton);
// }
