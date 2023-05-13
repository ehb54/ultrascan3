#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_addsaxs.h"
//Added by qt3to4:
#include <QTextStream>
#include <QCloseEvent>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>

US_AddSaxs::US_AddSaxs(bool *widget_flag, QWidget *p, const char *) : QWidget( p )
{
   this->widget_flag = widget_flag;
   *widget_flag = true;
   USglobal = new US_Config();
   saxs_filename = US_Config::get_home_dir() + "etc/somo.saxs_atoms";
   setPalette( PALET_FRAME );
   setWindowTitle(us_tr("SoMo: Modify Saxs Lookup Tables"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_AddSaxs::~US_AddSaxs()
{
}

void US_AddSaxs::setupGUI()
{
   int minHeight1 = 30;

   lbl_info = new QLabel(us_tr("Add/Edit SAXS Lookup Table:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info );
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   pb_select_file = new QPushButton(us_tr("Load SAXS Definition File"), this);
   Q_CHECK_PTR(pb_select_file);
   pb_select_file->setMinimumHeight(minHeight1);
   pb_select_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_file->setPalette( PALET_PUSHB );
   connect(pb_select_file, SIGNAL(clicked()), SLOT(select_file()));

   lbl_table = new QLabel(us_tr(" not selected"),this);
   lbl_table->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_table->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_table->setPalette( PALET_EDIT );
   AUTFBACK( lbl_table );
   lbl_table->setMinimumHeight(minHeight1);
   lbl_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   cmb_saxs = new QComboBox(  this );    cmb_saxs->setObjectName( "SAXS Entry Listing" );
   cmb_saxs->setPalette( PALET_NORMAL );
   AUTFBACK( cmb_saxs );
   cmb_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
 //   cmb_saxs->setSizeLimit(5);
   cmb_saxs->setMinimumHeight(minHeight1);
   connect(cmb_saxs, SIGNAL(activated(int)), this, SLOT(select_saxs(int)));

   lbl_number_of_saxs = new QLabel(us_tr(" Number of SAXS Entries in File: 0"), this);
   Q_CHECK_PTR(lbl_number_of_saxs);
   lbl_number_of_saxs->setMinimumHeight(minHeight1);
   lbl_number_of_saxs->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_number_of_saxs->setPalette( PALET_LABEL );
   AUTFBACK( lbl_number_of_saxs );
   lbl_number_of_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_saxs_name = new QLabel(us_tr(" SAXS Atom Identifier:"), this);
   Q_CHECK_PTR(lbl_saxs_name);
   lbl_saxs_name->setMinimumHeight(minHeight1);
   lbl_saxs_name->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_name->setPalette( PALET_LABEL );
   AUTFBACK( lbl_saxs_name );
   lbl_saxs_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_saxs_name = new QLineEdit( this );    le_saxs_name->setObjectName( "SAXS name Line Edit" );
   le_saxs_name->setPalette( PALET_NORMAL );
   AUTFBACK( le_saxs_name );
   le_saxs_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_saxs_name->setMinimumHeight(minHeight1);
   connect(le_saxs_name, SIGNAL(textChanged(const QString &)), SLOT(update_saxs_name(const QString &)));

   lbl_4term = new QLabel(us_tr(" 4 Term coefficients:"), this);
   Q_CHECK_PTR(lbl_4term);
   lbl_4term->setMinimumHeight(minHeight1);
   lbl_4term->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_4term->setPalette( PALET_LABEL );
   AUTFBACK( lbl_4term );
   lbl_4term->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_5term = new QLabel(us_tr(" 5 Term coefficients:"), this);
   Q_CHECK_PTR(lbl_5term);
   lbl_5term->setMinimumHeight(minHeight1);
   lbl_5term->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_5term->setPalette( PALET_LABEL );
   AUTFBACK( lbl_5term );
   lbl_5term->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_a1 = new QLabel(us_tr(" Coefficient a(1):"), this);
   Q_CHECK_PTR(lbl_a1);
   lbl_a1->setMinimumHeight(minHeight1);
   lbl_a1->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_a1->setPalette( PALET_LABEL );
   AUTFBACK( lbl_a1 );
   lbl_a1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_a1 = new QLineEdit( this );    le_a1->setObjectName( "Coefficient a(1) Line Edit" );
   le_a1->setPalette( PALET_NORMAL );
   AUTFBACK( le_a1 );
   le_a1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_a1->setMinimumHeight(minHeight1);
   connect(le_a1, SIGNAL(textChanged(const QString &)), SLOT(update_a1(const QString &)));

   lbl_a2 = new QLabel(us_tr(" Coefficient a(2):"), this);
   Q_CHECK_PTR(lbl_a2);
   lbl_a2->setMinimumHeight(minHeight1);
   lbl_a2->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_a2->setPalette( PALET_LABEL );
   AUTFBACK( lbl_a2 );
   lbl_a2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_a2 = new QLineEdit( this );    le_a2->setObjectName( "Coefficient a(2) Line Edit" );
   le_a2->setPalette( PALET_NORMAL );
   AUTFBACK( le_a2 );
   le_a2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_a2->setMinimumHeight(minHeight1);
   connect(le_a2, SIGNAL(textChanged(const QString &)), SLOT(update_a2(const QString &)));

   lbl_a3 = new QLabel(us_tr(" Coefficient a(3):"), this);
   Q_CHECK_PTR(lbl_a3);
   lbl_a3->setMinimumHeight(minHeight1);
   lbl_a3->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_a3->setPalette( PALET_LABEL );
   AUTFBACK( lbl_a3 );
   lbl_a3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_a3 = new QLineEdit( this );    le_a3->setObjectName( "Coefficient a(3) Line Edit" );
   le_a3->setPalette( PALET_NORMAL );
   AUTFBACK( le_a3 );
   le_a3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_a3->setMinimumHeight(minHeight1);
   connect(le_a3, SIGNAL(textChanged(const QString &)), SLOT(update_a3(const QString &)));

   lbl_a4 = new QLabel(us_tr(" Coefficient a(4):"), this);
   Q_CHECK_PTR(lbl_a4);
   lbl_a4->setMinimumHeight(minHeight1);
   lbl_a4->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_a4->setPalette( PALET_LABEL );
   AUTFBACK( lbl_a4 );
   lbl_a4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_a4 = new QLineEdit( this );    le_a4->setObjectName( "Coefficient a(4) Line Edit" );
   le_a4->setPalette( PALET_NORMAL );
   AUTFBACK( le_a4 );
   le_a4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_a4->setMinimumHeight(minHeight1);
   connect(le_a4, SIGNAL(textChanged(const QString &)), SLOT(update_a4(const QString &)));
   
   lbl_a5 = new QLabel(us_tr(" Coefficient a(5):"), this);
   Q_CHECK_PTR(lbl_a5);
   lbl_a5->setMinimumHeight(minHeight1);
   lbl_a5->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_a5->setPalette( PALET_LABEL );
   AUTFBACK( lbl_a5 );
   lbl_a5->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_b1 = new QLabel(us_tr(" Coefficient b(1):"), this);
   Q_CHECK_PTR(lbl_b1);
   lbl_b1->setMinimumHeight(minHeight1);
   lbl_b1->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_b1->setPalette( PALET_LABEL );
   AUTFBACK( lbl_b1 );
   lbl_b1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_b1 = new QLineEdit( this );    le_b1->setObjectName( "Coefficient b(1) Line Edit" );
   le_b1->setPalette( PALET_NORMAL );
   AUTFBACK( le_b1 );
   le_b1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_b1->setMinimumHeight(minHeight1);
   connect(le_b1, SIGNAL(textChanged(const QString &)), SLOT(update_b1(const QString &)));

   lbl_b2 = new QLabel(us_tr(" Coefficient b(2):"), this);
   Q_CHECK_PTR(lbl_b2);
   lbl_b2->setMinimumHeight(minHeight1);
   lbl_b2->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_b2->setPalette( PALET_LABEL );
   AUTFBACK( lbl_b2 );
   lbl_b2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_b2 = new QLineEdit( this );    le_b2->setObjectName( "Coefficient b(2) Line Edit" );
   le_b2->setPalette( PALET_NORMAL );
   AUTFBACK( le_b2 );
   le_b2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_b2->setMinimumHeight(minHeight1);
   connect(le_b2, SIGNAL(textChanged(const QString &)), SLOT(update_b2(const QString &)));

   lbl_b3 = new QLabel(us_tr(" Coefficient b(3):"), this);
   Q_CHECK_PTR(lbl_b3);
   lbl_b3->setMinimumHeight(minHeight1);
   lbl_b3->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_b3->setPalette( PALET_LABEL );
   AUTFBACK( lbl_b3 );
   lbl_b3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_b3 = new QLineEdit( this );    le_b3->setObjectName( "Coefficient b(3) Line Edit" );
   le_b3->setPalette( PALET_NORMAL );
   AUTFBACK( le_b3 );
   le_b3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_b3->setMinimumHeight(minHeight1);
   connect(le_b3, SIGNAL(textChanged(const QString &)), SLOT(update_b3(const QString &)));

   lbl_b4 = new QLabel(us_tr(" Coefficient b(4):"), this);
   Q_CHECK_PTR(lbl_b4);
   lbl_b4->setMinimumHeight(minHeight1);
   lbl_b4->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_b4->setPalette( PALET_LABEL );
   AUTFBACK( lbl_b4 );
   lbl_b4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_b4 = new QLineEdit( this );    le_b4->setObjectName( "Coefficient b(4) Line Edit" );
   le_b4->setPalette( PALET_NORMAL );
   AUTFBACK( le_b4 );
   le_b4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_b4->setMinimumHeight(minHeight1);
   connect(le_b4, SIGNAL(textChanged(const QString &)), SLOT(update_b4(const QString &)));

   lbl_b5 = new QLabel(us_tr(" Coefficient b(5):"), this);
   Q_CHECK_PTR(lbl_b5);
   lbl_b5->setMinimumHeight(minHeight1);
   lbl_b5->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_b5->setPalette( PALET_LABEL );
   AUTFBACK( lbl_b5 );
   lbl_b5->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_c = new QLabel(us_tr(" Coefficient c:"), this);
   Q_CHECK_PTR(lbl_c);
   lbl_c->setMinimumHeight(minHeight1);
   lbl_c->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_c->setPalette( PALET_LABEL );
   AUTFBACK( lbl_c );
   lbl_c->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_c = new QLineEdit( this );    le_c->setObjectName( "Coefficient c Line Edit" );
   le_c->setPalette( PALET_NORMAL );
   AUTFBACK( le_c );
   le_c->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_c->setMinimumHeight(minHeight1);
   connect(le_c, SIGNAL(textChanged(const QString &)), SLOT(update_c(const QString &)));

   lbl_volume = new QLabel(us_tr(" Atomic Volume (A^3):"), this);
   Q_CHECK_PTR(lbl_volume);
   lbl_volume->setMinimumHeight(minHeight1);
   lbl_volume->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_volume->setPalette( PALET_LABEL );
   AUTFBACK( lbl_volume );
   lbl_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_volume = new QLineEdit( this );    le_volume->setObjectName( "Atomic Volume Line Edit" );
   le_volume->setPalette( PALET_NORMAL );
   AUTFBACK( le_volume );
   le_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_volume->setMinimumHeight(minHeight1);
   connect(le_volume, SIGNAL(textChanged(const QString &)), SLOT(update_volume(const QString &)));

   le_5a1 = new QLineEdit( this );    le_5a1->setObjectName( "Coefficient a(1) Line Edit" );
   le_5a1->setPalette( PALET_NORMAL );
   AUTFBACK( le_5a1 );
   le_5a1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_5a1->setMinimumHeight(minHeight1);
   connect(le_5a1, SIGNAL(textChanged(const QString &)), SLOT(update_5a1(const QString &)));

   le_5a2 = new QLineEdit( this );    le_5a2->setObjectName( "Coefficient a(1) Line Edit" );
   le_5a2->setPalette( PALET_NORMAL );
   AUTFBACK( le_5a2 );
   le_5a2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_5a2->setMinimumHeight(minHeight1);
   connect(le_5a2, SIGNAL(textChanged(const QString &)), SLOT(update_5a2(const QString &)));

   le_5a3 = new QLineEdit( this );    le_5a3->setObjectName( "Coefficient a(1) Line Edit" );
   le_5a3->setPalette( PALET_NORMAL );
   AUTFBACK( le_5a3 );
   le_5a3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_5a3->setMinimumHeight(minHeight1);
   connect(le_5a3, SIGNAL(textChanged(const QString &)), SLOT(update_5a3(const QString &)));

   le_5a4 = new QLineEdit( this );    le_5a4->setObjectName( "Coefficient a(1) Line Edit" );
   le_5a4->setPalette( PALET_NORMAL );
   AUTFBACK( le_5a4 );
   le_5a4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_5a4->setMinimumHeight(minHeight1);
   connect(le_5a4, SIGNAL(textChanged(const QString &)), SLOT(update_5a4(const QString &)));

   le_5a5 = new QLineEdit( this );    le_5a5->setObjectName( "Coefficient a(1) Line Edit" );
   le_5a5->setPalette( PALET_NORMAL );
   AUTFBACK( le_5a5 );
   le_5a5->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_5a5->setMinimumHeight(minHeight1);
   connect(le_5a5, SIGNAL(textChanged(const QString &)), SLOT(update_5a5(const QString &)));

   le_5b1 = new QLineEdit( this );    le_5b1->setObjectName( "Coefficient a(1) Line Edit" );
   le_5b1->setPalette( PALET_NORMAL );
   AUTFBACK( le_5b1 );
   le_5b1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_5b1->setMinimumHeight(minHeight1);
   connect(le_5b1, SIGNAL(textChanged(const QString &)), SLOT(update_5b1(const QString &)));

   le_5b2 = new QLineEdit( this );    le_5b2->setObjectName( "Coefficient a(1) Line Edit" );
   le_5b2->setPalette( PALET_NORMAL );
   AUTFBACK( le_5b2 );
   le_5b2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_5b2->setMinimumHeight(minHeight1);
   connect(le_5b2, SIGNAL(textChanged(const QString &)), SLOT(update_5b2(const QString &)));

   le_5b3 = new QLineEdit( this );    le_5b3->setObjectName( "Coefficient a(1) Line Edit" );
   le_5b3->setPalette( PALET_NORMAL );
   AUTFBACK( le_5b3 );
   le_5b3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_5b3->setMinimumHeight(minHeight1);
   connect(le_5b3, SIGNAL(textChanged(const QString &)), SLOT(update_5b3(const QString &)));

   le_5b4 = new QLineEdit( this );    le_5b4->setObjectName( "Coefficient a(1) Line Edit" );
   le_5b4->setPalette( PALET_NORMAL );
   AUTFBACK( le_5b4 );
   le_5b4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_5b4->setMinimumHeight(minHeight1);
   connect(le_5b4, SIGNAL(textChanged(const QString &)), SLOT(update_5b4(const QString &)));

   le_5b5 = new QLineEdit( this );    le_5b5->setObjectName( "Coefficient a(1) Line Edit" );
   le_5b5->setPalette( PALET_NORMAL );
   AUTFBACK( le_5b5 );
   le_5b5->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_5b5->setMinimumHeight(minHeight1);
   connect(le_5b5, SIGNAL(textChanged(const QString &)), SLOT(update_5b5(const QString &)));

   le_5c = new QLineEdit( this );    le_5c->setObjectName( "Coefficient c Line Edit" );
   le_5c->setPalette( PALET_NORMAL );
   AUTFBACK( le_5c );
   le_5c->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_5c->setMinimumHeight(minHeight1);
   connect(le_5c, SIGNAL(textChanged(const QString &)), SLOT(update_5c(const QString &)));

   le_5volume = new QLineEdit( this );    le_5volume->setObjectName( "Atomic Volume Line Edit" );
   le_5volume->setPalette( PALET_NORMAL );
   AUTFBACK( le_5volume );
   le_5volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_5volume->setMinimumHeight(minHeight1);
   connect(le_5volume, SIGNAL(textChanged(const QString &)), SLOT(update_5volume(const QString &)));

   pb_add = new QPushButton(us_tr("Add SAXS Atom to File"), this);
   Q_CHECK_PTR(pb_add);
   pb_add->setEnabled(true);
   pb_add->setMinimumHeight(minHeight1);
   pb_add->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_add->setPalette( PALET_PUSHB );
   connect(pb_add, SIGNAL(clicked()), SLOT(add()));

   pb_help = new QPushButton(us_tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_close = new QPushButton(us_tr("Close"), this);
   Q_CHECK_PTR(pb_close);
   pb_close->setMinimumHeight(minHeight1);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_close->setPalette( PALET_PUSHB );
   connect(pb_close, SIGNAL(clicked()), SLOT(close()));

   int /* rows=3, columns = 2,*/ spacing = 2, j=0, margin=4;
   QGridLayout * background = new QGridLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 ); background->setSpacing( spacing ); background->setContentsMargins( margin, margin, margin, margin );

   qDebug() << "gui1";
   background->addWidget( lbl_info , j , 0 , 1 , 4 );
   j++;
   qDebug() << "gui2";
   background->addWidget(pb_select_file, j, 0, 1, 2);
   background->addWidget(lbl_table, j, 2, 1, 2 );
   j++;
   background->addWidget(lbl_number_of_saxs, j, 0, 1, 2);
   background->addWidget(cmb_saxs, j, 2, 1, 2);
   j++;
   background->addWidget(lbl_saxs_name, j, 0, 1, 2);
   background->addWidget(le_saxs_name, j, 2, 1, 2);
   j++;
   background->addWidget(lbl_4term, j, 2);
   background->addWidget(lbl_5term, j, 3);
   qDebug() << "gui3";
   j++;
   background->addWidget(lbl_a1, j, 0, 1, 2);
   background->addWidget(le_a1, j, 2);
   background->addWidget(le_5a1, j, 3);
   j++;
   background->addWidget(lbl_b1, j, 0, 1, 2);
   background->addWidget(le_b1, j, 2);
   background->addWidget(le_5b1, j, 3);
   j++;
   background->addWidget(lbl_a2, j, 0, 1, 2);
   background->addWidget(le_a2, j, 2);
   background->addWidget(le_5a2, j, 3);
   j++;
   background->addWidget(lbl_b2, j, 0, 1, 2);
   background->addWidget(le_b2, j, 2);
   background->addWidget(le_5b2, j, 3);
   j++;
   background->addWidget(lbl_a3, j, 0, 1, 2);
   background->addWidget(le_a3, j, 2);
   background->addWidget(le_5a3, j, 3);
   j++;
   background->addWidget(lbl_b3, j, 0, 1, 2);
   background->addWidget(le_b3, j, 2);
   background->addWidget(le_5b3, j, 3);
   qDebug() << "gui5";
   j++;
   background->addWidget(lbl_a4, j, 0, 1, 2);
   background->addWidget(le_a4, j, 2);
   background->addWidget(le_5a4, j, 3);
   j++;
   background->addWidget(lbl_b4, j, 0, 1, 2);
   background->addWidget(le_b4, j, 2);
   background->addWidget(le_5b4, j, 3);
   j++;
   background->addWidget(lbl_a5, j, 0, 1, 2);
   background->addWidget(le_5a5, j, 3);
   j++;
   background->addWidget(lbl_b5, j, 0, 1, 2);
   background->addWidget(le_5b5, j, 3);
   j++;
   background->addWidget(lbl_c, j, 0, 1, 2);
   background->addWidget(le_c, j, 2);
   background->addWidget(le_5c, j, 3);
   j++;
   qDebug() << "gui7";
   background->addWidget(lbl_volume, j, 0, 1, 2);
   background->addWidget(le_volume, j, 2, 1, 2);
   background->addWidget(le_5volume, j, 3);
   j++;
   background->addWidget( pb_add , j , 0, 1, 4 );
   j++;
   background->addWidget(pb_help, j, 0, 1, 2 );
   background->addWidget(pb_close, j, 2, 1, 2 );

   le_5volume->hide();
}

void US_AddSaxs::add()
{
   int item = -1;
   QString str1;
   for (int i=0; i<(int) saxs_list.size(); i++)
   {
      if (saxs_list[i].saxs_name.toUpper() == current_saxs.saxs_name.toUpper())
      {
         item = i;
         saxs_list[i].a[0] = current_saxs.a[0];
         saxs_list[i].b[0] = current_saxs.b[0];
         saxs_list[i].a[1] = current_saxs.a[1];
         saxs_list[i].b[1] = current_saxs.b[1];
         saxs_list[i].a[2] = current_saxs.a[2];
         saxs_list[i].b[2] = current_saxs.b[2];
         saxs_list[i].a[3] = current_saxs.a[3];
         saxs_list[i].b[3] = current_saxs.b[3];
         saxs_list[i].c = current_saxs.c;
         saxs_list[i].a5[0] = current_saxs.a5[0];
         saxs_list[i].b5[0] = current_saxs.b5[0];
         saxs_list[i].a5[1] = current_saxs.a5[1];
         saxs_list[i].b5[1] = current_saxs.b5[1];
         saxs_list[i].a5[2] = current_saxs.a5[2];
         saxs_list[i].b5[2] = current_saxs.b5[2];
         saxs_list[i].a5[3] = current_saxs.a5[3];
         saxs_list[i].b5[3] = current_saxs.b5[3];
         saxs_list[i].a5[4] = current_saxs.a5[4];
         saxs_list[i].b5[4] = current_saxs.b5[4];
         saxs_list[i].c5 = current_saxs.c5;
         saxs_list[i].volume = current_saxs.volume;
      }
   }
   if (item < 0)
   {
      saxs_list.push_back(current_saxs);
   }
   QFile f(saxs_filename);
   if (f.open(QIODevice::WriteOnly|QIODevice::Text))
   {
      cmb_saxs->clear( );
      str1 = QString( us_tr( " Number of SAXS Entries in File: %1" ) ).arg( saxs_list.size() );
      QTextStream ts(&f);
      ts << qSetRealNumberPrecision(8);
      for (unsigned int i=0; i<saxs_list.size(); i++)
      {
         ts << saxs_list[i].saxs_name.toUpper() << "\t"
            << saxs_list[i].a[0] << "\t"
            << saxs_list[i].b[0] << "\t"
            << saxs_list[i].a[1] << "\t"
            << saxs_list[i].b[1] << "\t"
            << saxs_list[i].a[2] << "\t"
            << saxs_list[i].b[2] << "\t"
            << saxs_list[i].a[3] << "\t"
            << saxs_list[i].b[3] << "\t"
            << saxs_list[i].c << "\t"
            << saxs_list[i].volume << Qt::endl;
         if ( saxs_list[i].a5[0] &&
              saxs_list[i].b5[0] ) {
            ts << saxs_list[i].saxs_name.toUpper() << "\t"
               << saxs_list[i].a5[0] << "\t"
               << saxs_list[i].b5[0] << "\t"
               << saxs_list[i].a5[1] << "\t"
               << saxs_list[i].b5[1] << "\t"
               << saxs_list[i].a5[2] << "\t"
               << saxs_list[i].b5[2] << "\t"
               << saxs_list[i].a5[3] << "\t"
               << saxs_list[i].b5[3] << "\t"
               << saxs_list[i].a5[4] << "\t"
               << saxs_list[i].b5[4] << "\t"
               << saxs_list[i].c5 << "\t"
               << saxs_list[i].volume << Qt::endl;
         }
         str1.sprintf("%d: ", i+1);
         str1 += saxs_list[i].saxs_name.toUpper();
         cmb_saxs->addItem(str1);
      }
      f.close();
      lbl_number_of_saxs->setText( QString( us_tr( " Number of SAXS Entries in File: %1" ) ).arg( saxs_list.size() ) );
   }
   else
   {
      US_Static::us_message("Attention:", "Could not open the SAXS Coefficient file:\n\n" + saxs_filename);
   }
}

void US_AddSaxs::select_file()
{
   QString old_filename = saxs_filename, str1, str2;
   saxs_filename = QFileDialog::getOpenFileName( this , windowTitle() , US_Config::get_home_dir() + "etc" , "*.saxs_atoms *.SAXS_ATOMS" );
   if (saxs_filename.isEmpty())
   {
      saxs_filename = old_filename;
   }
   else
   {
      lbl_table->setText(saxs_filename);
      QFile f(saxs_filename);
      saxs_list.clear( );
      cmb_saxs->clear( );
      unsigned int i=1;
      map < QString, struct saxs > saxs_map;

      if (f.open(QIODevice::ReadOnly|QIODevice::Text))
      {
         QTextStream ts(&f);
         while (!ts.atEnd())
         {
            QStringList qsl = ( ts.readLine() ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );
            int pos = 0;
            if ( qsl.size() == 11 ) {
               saxs_map[ qsl[0] ].saxs_name = qsl[pos++];
               saxs_map[ qsl[0] ].a[0]      = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].b[0]      = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].a[1]      = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].b[1]      = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].a[2]      = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].b[2]      = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].a[3]      = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].b[3]      = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].c         = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].volume    = qsl[pos++].toFloat();
            } else if ( qsl.size() == 13 ) {
               saxs_map[ qsl[0] ].saxs_name = qsl[pos++];
               saxs_map[ qsl[0] ].a5[0]     = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].b5[0]     = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].a5[1]     = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].b5[1]     = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].a5[2]     = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].b5[2]     = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].a5[3]     = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].b5[3]     = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].a5[4]     = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].b5[4]     = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].c5        = qsl[pos++].toFloat();
               saxs_map[ qsl[0] ].volume    = qsl[pos++].toFloat();
            } else {
               qDebug() << "invalid line in saxs atom file";
            }
         }
         f.close();
      }
      for ( auto it = saxs_map.begin();
            it != saxs_map.end();
            ++it ) {
         if ( !it->first.isEmpty() ) {
            saxs_list.push_back( it->second );
            str1.sprintf("%d: ", i);
            str1 += it->second.saxs_name;
            cmb_saxs->addItem(str1);
            i++;
         }
      }
   }
   str1 = QString( us_tr( " Number of SAXS Entries in File: %1" ) ).arg( saxs_list.size() );
   lbl_number_of_saxs->setText(str1);
   pb_add->setEnabled(true);
}

#define TSO QTextStream( stdout )
#define LEQ "============================================================\n"
#define LD  "------------------------------------------------------------\n"

void US_AddSaxs::info_saxs( const QString & msg, const struct saxs & saxs ) {
   TSO << LEQ << "info_saxs(): " << saxs.saxs_name << " " << msg << Qt::endl;
   TSO << LD << "4 term gaussians:\n";
   for ( int i = 0; i < 4; ++i ) {
      TSO << "\ta & b [" << i << "] = " << saxs.a[i] << " & " << saxs.b[i] << Qt::endl;
   }      
   TSO << "\tc & volume = " << saxs.c << " & " << saxs.volume << Qt::endl;
   TSO << LD << "5 term gaussians:\n";
   for ( int i = 0; i < 5; ++i ) {
      TSO << "\ta & b [" << i << "] = " << saxs.a5[i] << " & " << saxs.b5[i] << Qt::endl;
   }      
   TSO << "\tc5 = " << saxs.c5 << Qt::endl;
}

void US_AddSaxs::update_saxs_name(const QString &str)
{
   current_saxs.saxs_name = str;
}

void US_AddSaxs::update_a1(const QString &str)
{
   current_saxs.a[0] = str.toFloat();
}

void US_AddSaxs::update_a2(const QString &str)
{
   current_saxs.a[1] = str.toFloat();
}

void US_AddSaxs::update_a3(const QString &str)
{
   current_saxs.a[2] = str.toFloat();
}

void US_AddSaxs::update_a4(const QString &str)
{
   current_saxs.a[3] = str.toFloat();
}

void US_AddSaxs::update_b1(const QString &str)
{
   current_saxs.b[0] = str.toFloat();
}

void US_AddSaxs::update_b2(const QString &str)
{
   current_saxs.b[1] = str.toFloat();
}

void US_AddSaxs::update_b3(const QString &str)
{
   current_saxs.b[2] = str.toFloat();
}

void US_AddSaxs::update_b4(const QString &str)
{
   current_saxs.b[3] = str.toFloat();
}

void US_AddSaxs::update_c(const QString &str)
{
   current_saxs.c = str.toFloat();
}

void US_AddSaxs::update_volume(const QString &str)
{
   current_saxs.volume = str.toFloat();
}

void US_AddSaxs::update_5a1(const QString &str)
{
   current_saxs.a5[0] = str.toFloat();
}

void US_AddSaxs::update_5a2(const QString &str)
{
   current_saxs.a5[1] = str.toFloat();
}

void US_AddSaxs::update_5a3(const QString &str)
{
   current_saxs.a5[2] = str.toFloat();
}

void US_AddSaxs::update_5a4(const QString &str)
{
   current_saxs.a5[3] = str.toFloat();
}

void US_AddSaxs::update_5a5(const QString &str)
{
   current_saxs.a5[4] = str.toFloat();
}

void US_AddSaxs::update_5b1(const QString &str)
{
   current_saxs.b5[0] = str.toFloat();
}

void US_AddSaxs::update_5b2(const QString &str)
{
   current_saxs.b5[1] = str.toFloat();
}

void US_AddSaxs::update_5b3(const QString &str)
{
   current_saxs.b5[2] = str.toFloat();
}

void US_AddSaxs::update_5b4(const QString &str)
{
   current_saxs.b5[3] = str.toFloat();
}

void US_AddSaxs::update_5b5(const QString &str)
{
   current_saxs.b5[4] = str.toFloat();
}

void US_AddSaxs::update_5c(const QString &str)
{
   current_saxs.c5 = str.toFloat();
}

void US_AddSaxs::update_5volume(const QString &str)
{
   current_saxs.volume = str.toFloat();
}

void US_AddSaxs::select_saxs(int val)
{
   QString str;
   le_saxs_name->setText(saxs_list[val].saxs_name.toUpper());
   str.sprintf("%3.6f", saxs_list[val].a[0]);
   le_a1->setText(str);
   str.sprintf("%3.6f", saxs_list[val].b[0]);
   le_b1->setText(str);
   str.sprintf("%3.6f", saxs_list[val].a[1]);
   le_a2->setText(str);
   str.sprintf("%3.6f", saxs_list[val].b[1]);
   le_b2->setText(str);
   str.sprintf("%3.6f", saxs_list[val].a[2]);
   le_a3->setText(str);
   str.sprintf("%3.6f", saxs_list[val].b[2]);
   le_b3->setText(str);
   str.sprintf("%3.6f", saxs_list[val].a[3]);
   le_a4->setText(str);
   str.sprintf("%3.6f", saxs_list[val].b[3]);
   le_b4->setText(str);
   str.sprintf("%3.6f", saxs_list[val].c);
   le_c->setText(str);
   str.sprintf("%3.6f", saxs_list[val].volume);
   le_volume->setText(str);

   str.sprintf("%3.6f", saxs_list[val].a5[0]);
   le_5a1->setText(str);
   str.sprintf("%3.6f", saxs_list[val].b5[0]);
   le_5b1->setText(str);
   str.sprintf("%3.6f", saxs_list[val].a5[1]);
   le_5a2->setText(str);
   str.sprintf("%3.6f", saxs_list[val].b5[1]);
   le_5b2->setText(str);
   str.sprintf("%3.6f", saxs_list[val].a5[2]);
   le_5a3->setText(str);
   str.sprintf("%3.6f", saxs_list[val].b5[2]);
   le_5b3->setText(str);
   str.sprintf("%3.6f", saxs_list[val].a5[3]);
   le_5a4->setText(str);
   str.sprintf("%3.6f", saxs_list[val].b5[3]);
   le_5b4->setText(str);
   str.sprintf("%3.6f", saxs_list[val].a5[4]);
   le_5a5->setText(str);
   str.sprintf("%3.6f", saxs_list[val].b5[4]);
   le_5b5->setText(str);
   str.sprintf("%3.6f", saxs_list[val].c5);
   le_5c->setText(str);
   
}

void US_AddSaxs::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;

   *widget_flag = false;
   e->accept();
}

void US_AddSaxs::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_add_saxs.html");
}

