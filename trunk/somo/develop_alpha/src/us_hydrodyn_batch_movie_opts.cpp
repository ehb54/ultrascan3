#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_batch_movie_opts.h"
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>
#include <QLabel>

US_Hydrodyn_Batch_Movie_Opts::US_Hydrodyn_Batch_Movie_Opts(
                                                           QString    msg,
                                                           QString    *title,
                                                           QString    *dir,
                                                           QString    somo_dir,
                                                           QString    *file,
                                                           double     *fps,
                                                           double     *scale,
                                                           bool       *cancel_req,
                                                           bool       *clean_up,
                                                           bool       *use_tc,
                                                           QString    *tc_unit,
                                                           double     *tc_start,
                                                           double     *tc_delta,
                                                           float      *tc_pointsize,
                                                           bool       *black_background,
                                                           QWidget *p,
                                                           const char *name
                                                           ) : QDialog(p, name)
{
   this->msg = msg;
   this->title = title;
   this->dir = dir;
   this->file = file;
   this->fps = fps;
   this->scale = scale;
   this->cancel_req = cancel_req;
   *(this->cancel_req) = false;
   this->clean_up = clean_up;
   this->somo_dir = somo_dir;
   this->use_tc = use_tc;
   this->tc_unit = tc_unit;
   this->tc_start = tc_start;
   this->tc_delta = tc_delta;
   this->tc_pointsize = tc_pointsize;
   this->black_background = black_background;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setCaption("Set parameters for movie file");
   setupGUI();
   update_enables();
   global_Xpos = 200;
   global_Ypos = 150;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Batch_Movie_Opts::~US_Hydrodyn_Batch_Movie_Opts()
{
}

void US_Hydrodyn_Batch_Movie_Opts::setupGUI()
{
   int minHeight1 = 30;
   int minHeight2 = 30;

   lbl_info = new QLabel(msg, this);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight2);
   lbl_info->setPalette( PALET_LABEL );
   AUTFBACK( lbl_info );
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lbl_title = new QLabel(tr(" Title (leave blank for no title):"), this);
   Q_CHECK_PTR(lbl_title);
   lbl_title->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_title->setPalette( PALET_LABEL );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_title = new QLineEdit(this, "Title Line Edit");
   le_title->setText(*title);
   le_title->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   le_title->setPalette( PALET_NORMAL );
   AUTFBACK( le_title );
   le_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_title->setEnabled(true);
   connect(le_title, SIGNAL(textChanged(const QString &)), SLOT(update_title(const QString &)));

   lbl_dir = new QLabel(tr(" Directory for movie file:"), this);
   Q_CHECK_PTR(lbl_dir);
   lbl_dir->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_dir->setPalette( PALET_LABEL );
   AUTFBACK( lbl_dir );
   lbl_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_dir = new QLineEdit(this, "Dir Line Edit");
   le_dir->setText(*dir);
   le_dir->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   le_dir->setPalette( PALET_NORMAL );
   AUTFBACK( le_dir );
   le_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_dir->setEnabled(true);
   connect(le_dir, SIGNAL(textChanged(const QString &)), SLOT(update_dir(const QString &)));

   lbl_file = new QLabel(tr(" Name of movie file (.avi will be appended):"), this);
   Q_CHECK_PTR(lbl_file);
   lbl_file->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_file->setPalette( PALET_LABEL );
   AUTFBACK( lbl_file );
   lbl_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_file = new QLineEdit(this, "File Line Edit");
   le_file->setText(*file);
   le_file->setMinimumWidth(300);
   le_file->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   le_file->setPalette( PALET_NORMAL );
   AUTFBACK( le_file );
   le_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_file->setEnabled(true);
   connect(le_file, SIGNAL(textChanged(const QString &)), SLOT(update_file(const QString &)));

   lbl_fps = new QLabel(tr(" Frames per second:"), this);
   Q_CHECK_PTR(lbl_fps);
   lbl_fps->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_fps->setPalette( PALET_LABEL );
   AUTFBACK( lbl_fps );
   lbl_fps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_fps = new QLineEdit(this, "Fps Line Edit");
   le_fps->setText(QString("%1").arg(*fps));
   le_fps->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   le_fps->setPalette( PALET_NORMAL );
   AUTFBACK( le_fps );
   le_fps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_fps->setEnabled(true);
   connect(le_fps, SIGNAL(textChanged(const QString &)), SLOT(update_fps(const QString &)));

   lbl_scale = new QLabel(tr(" Image scale:"), this);
   Q_CHECK_PTR(lbl_scale);
   lbl_scale->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_scale->setPalette( PALET_LABEL );
   AUTFBACK( lbl_scale );
   lbl_scale->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_scale = new QLineEdit(this, "Scale Line Edit");
   le_scale->setText(QString("%1").arg(*scale));
   le_scale->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   le_scale->setPalette( PALET_NORMAL );
   AUTFBACK( le_scale );
   le_scale->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_scale->setEnabled(true);
   connect(le_scale, SIGNAL(textChanged(const QString &)), SLOT(update_scale(const QString &)));

   cb_use_tc = new QCheckBox(this);
   cb_use_tc->setText(tr("Use time code"));
   cb_use_tc->setEnabled(true);
   cb_use_tc->setChecked(*use_tc);
   cb_use_tc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_use_tc->setPalette( PALET_NORMAL );
   AUTFBACK( cb_use_tc );
   connect(cb_use_tc, SIGNAL(clicked()), this, SLOT(set_use_tc()));

   lbl_tc_unit = new QLabel(tr(" Time code unit (e.g. ns, ps):"), this);
   Q_CHECK_PTR(lbl_tc_unit);
   lbl_tc_unit->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_tc_unit->setPalette( PALET_LABEL );
   AUTFBACK( lbl_tc_unit );
   lbl_tc_unit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_tc_unit = new QLineEdit(this, "Tc_Unit Line Edit");
   le_tc_unit->setText(QString("%1").arg(*tc_unit));
   le_tc_unit->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   le_tc_unit->setPalette( PALET_NORMAL );
   AUTFBACK( le_tc_unit );
   le_tc_unit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_tc_unit->setEnabled(true);
   connect(le_tc_unit, SIGNAL(textChanged(const QString &)), SLOT(update_tc_unit(const QString &)));

   lbl_tc_start = new QLabel(tr(" Time code start:"), this);
   Q_CHECK_PTR(lbl_tc_start);
   lbl_tc_start->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_tc_start->setPalette( PALET_LABEL );
   AUTFBACK( lbl_tc_start );
   lbl_tc_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_tc_start = new QLineEdit(this, "Tc_Start Line Edit");
   le_tc_start->setText(QString("%1").arg(*tc_start));
   le_tc_start->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   le_tc_start->setPalette( PALET_NORMAL );
   AUTFBACK( le_tc_start );
   le_tc_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_tc_start->setEnabled(true);
   connect(le_tc_start, SIGNAL(textChanged(const QString &)), SLOT(update_tc_start(const QString &)));

   lbl_tc_delta = new QLabel(tr(" Time code delta per frame:"), this);
   Q_CHECK_PTR(lbl_tc_delta);
   lbl_tc_delta->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_tc_delta->setPalette( PALET_LABEL );
   AUTFBACK( lbl_tc_delta );
   lbl_tc_delta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_tc_delta = new QLineEdit(this, "Tc_Delta Line Edit");
   le_tc_delta->setText(QString("%1").arg(*tc_delta));
   le_tc_delta->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   le_tc_delta->setPalette( PALET_NORMAL );
   AUTFBACK( le_tc_delta );
   le_tc_delta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_tc_delta->setEnabled(true);
   connect(le_tc_delta, SIGNAL(textChanged(const QString &)), SLOT(update_tc_delta(const QString &)));

   lbl_tc_pointsize = new QLabel(tr(" Time code font pointsize:"), this);
   Q_CHECK_PTR(lbl_tc_pointsize);
   lbl_tc_pointsize->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_tc_pointsize->setPalette( PALET_LABEL );
   AUTFBACK( lbl_tc_pointsize );
   lbl_tc_pointsize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_tc_pointsize = new QLineEdit(this, "Tc_Pointsize Line Edit");
   le_tc_pointsize->setText(QString("%1").arg(*tc_pointsize));
   le_tc_pointsize->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   le_tc_pointsize->setPalette( PALET_NORMAL );
   AUTFBACK( le_tc_pointsize );
   le_tc_pointsize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_tc_pointsize->setEnabled(true);
   connect(le_tc_pointsize, SIGNAL(textChanged(const QString &)), SLOT(update_tc_pointsize(const QString &)));

   cb_black_background = new QCheckBox(this);
   cb_black_background->setText(tr("Black background:"));
   cb_black_background->setEnabled(true);
   cb_black_background->setChecked(*black_background);
   cb_black_background->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_black_background->setPalette( PALET_NORMAL );
   AUTFBACK( cb_black_background );
   connect(cb_black_background, SIGNAL(clicked()), this, SLOT(set_black_background()));

   cb_clean_up = new QCheckBox(this);
   cb_clean_up->setText(tr("Clean up files (ppm, gifs, spts)"));
   cb_clean_up->setEnabled(true);
   cb_clean_up->setChecked(*clean_up);
   cb_clean_up->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_clean_up->setPalette( PALET_NORMAL );
   AUTFBACK( cb_clean_up );
   connect(cb_clean_up, SIGNAL(clicked()), this, SLOT(set_clean_up()));

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton("Help", this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight2);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   label_font_ok = QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label);
   label_font_warning = QPalette(USglobal->global_colors.cg_label_warn, USglobal->global_colors.cg_label_warn, USglobal->global_colors.cg_label_warn);

   int j = 0;

   Q3GridLayout *background = new Q3GridLayout(this);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;
   background->addWidget(lbl_title, j, 0);
   background->addWidget(le_title, j, 1);
   j++;
   background->addWidget(lbl_dir, j, 0);
   background->addWidget(le_dir, j, 1);
   j++;
   background->addWidget(lbl_file, j, 0);
   background->addWidget(le_file, j, 1);
   j++;
   background->addWidget(lbl_fps, j, 0);
   background->addWidget(le_fps, j, 1);
   j++;
   background->addWidget(lbl_scale, j, 0);
   background->addWidget(le_scale, j, 1);
   j++;
   background->addMultiCellWidget(cb_use_tc, j, j, 0, 1);
   j++;
   background->addWidget(lbl_tc_unit, j, 0);
   background->addWidget(le_tc_unit, j, 1);
   j++;
   background->addWidget(lbl_tc_start, j, 0);
   background->addWidget(le_tc_start, j, 1);
   j++;
   background->addWidget(lbl_tc_delta, j, 0);
   background->addWidget(le_tc_delta, j, 1);
   j++;
   background->addWidget(lbl_tc_pointsize, j, 0);
   background->addWidget(le_tc_pointsize, j, 1);
   j++;
   background->addMultiCellWidget(cb_black_background, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(cb_clean_up, j, j, 0, 1);
   j++;
   Q3HBoxLayout *hbl = new Q3HBoxLayout;
   hbl->addWidget(pb_help);
   hbl->addWidget(pb_cancel);
   background->addMultiCellLayout(hbl, j, j, 0, 1);
}

void US_Hydrodyn_Batch_Movie_Opts::cancel()
{
   close();
}

void US_Hydrodyn_Batch_Movie_Opts::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_batch_movie_opts.html");
}

void US_Hydrodyn_Batch_Movie_Opts::update_title(const QString &str)
{
   *title = str;
}

void US_Hydrodyn_Batch_Movie_Opts::update_dir(const QString &str)
{
   *dir = str;
   if ( str == "" )
   {
      *dir = somo_dir;
      le_dir->setText(*dir);
   }
   update_dir_msg();
}

void US_Hydrodyn_Batch_Movie_Opts::update_file(const QString &str)
{
   *file = str;
}

void US_Hydrodyn_Batch_Movie_Opts::update_fps(const QString &str)
{
   *fps = str.toDouble();
}

void US_Hydrodyn_Batch_Movie_Opts::update_scale(const QString &str)
{
   *scale = str.toDouble();
}

void US_Hydrodyn_Batch_Movie_Opts::update_tc_unit(const QString &str)
{
   *tc_unit = str;
}

void US_Hydrodyn_Batch_Movie_Opts::update_tc_start(const QString &str)
{
   *tc_start = str.toDouble();
}

void US_Hydrodyn_Batch_Movie_Opts::update_tc_delta(const QString &str)
{
   *tc_delta = str.toDouble();
}

void US_Hydrodyn_Batch_Movie_Opts::update_tc_pointsize(const QString &str)
{
   *tc_pointsize = str.toDouble();
}

void US_Hydrodyn_Batch_Movie_Opts::set_clean_up()
{
   *clean_up = cb_clean_up->isChecked();
}

void US_Hydrodyn_Batch_Movie_Opts::set_use_tc()
{
   *use_tc = cb_use_tc->isChecked();
   update_enables();
}

void US_Hydrodyn_Batch_Movie_Opts::set_black_background()
{
   *black_background = cb_black_background->isChecked();
}

void US_Hydrodyn_Batch_Movie_Opts::update_enables()
{
   le_tc_unit->setEnabled(*use_tc);
   le_tc_start->setEnabled(*use_tc);
   le_tc_delta->setEnabled(*use_tc);
   le_tc_pointsize->setEnabled(*use_tc);
}

void US_Hydrodyn_Batch_Movie_Opts::update_dir_msg()
{
   QFileInfo fi(*dir);
   if ( !fi.exists() )
   {
      lbl_dir->setPalette(label_font_warning);
      lbl_dir->setText(tr(" Directory for movie file (Warning: does not exist):"));
   } else {
      if ( !fi.isDir() )
      {
         lbl_dir->setPalette(label_font_warning);
         lbl_dir->setText(tr(" Directory for movie file (Error: not a directory!):"));
      } else {
         if ( !fi.isWritable() )
         {
            lbl_dir->setPalette(label_font_warning);
            lbl_dir->setText(tr(" Directory for movie file (Error: not writable!):"));
         } else {
            lbl_dir->setPalette(label_font_ok);
            lbl_dir->setText(tr(" Directory for movie file:"));
         }
      }
   }
}
