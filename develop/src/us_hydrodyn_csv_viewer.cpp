#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_csv_viewer.h"

US_Hydrodyn_Csv_Viewer::US_Hydrodyn_Csv_Viewer(
                                               csv csv1,
                                               void *us_hydrodyn, 
                                               QWidget *p, 
                                               const char *name
                                               ) : QFrame(p, name)
{
   this->csv1 = csv1;
   this->us_hydrodyn = us_hydrodyn;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("View CSV"));
   order_ascending = false;
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;

   unsigned int csv_height = t_csv->rowHeight(0);
   unsigned int csv_width = t_csv->columnWidth(0);
   for ( int i = 0; i < t_csv->numRows(); i++ )
   {
      csv_height += t_csv->rowHeight(i);
   }
   for ( int i = 0; i < t_csv->numCols(); i++ )
   {
      csv_width += t_csv->columnWidth(i);
   }
   if ( csv_height > 700 )
   {
      csv_height = 700;
   }
   if ( csv_width > 1000 )
   {
      csv_width = 1000;
   }

   // cout << QString("csv size %1 %1\n").arg(csv_height).arg(csv_width);

   setGeometry(global_Xpos, global_Ypos, csv_width, 100 + csv_height );
}

US_Hydrodyn_Csv_Viewer::~US_Hydrodyn_Csv_Viewer()
{
}

void US_Hydrodyn_Csv_Viewer::setupGUI()
{
   int minHeight1 = 30;

   lbl_title = new QLabel(csv1.name.left(80), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   t_csv = new QTable(csv1.data.size(), csv1.header.size(), this);
   t_csv->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // t_csv->setMinimumHeight(minHeight1 * 3);
   // t_csv->setMinimumWidth(minWidth1);
   t_csv->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   t_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   t_csv->setEnabled(true);

   for ( unsigned int i = 0; i < csv1.num_data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.num_data[i].size(); j++ )
      {
         t_csv->setText(i, j, csv1.data[i][j]);
      }
   }

   for ( unsigned int i = 0; i < csv1.header.size(); i++ )
   {
      t_csv->horizontalHeader()->setLabel(i, csv1.header[i]);
   }

   t_csv->setSorting(false);
   t_csv->setRowMovingEnabled(true);
   t_csv->setColumnMovingEnabled(true);
   t_csv->setReadOnly(true);

   
   t_csv->horizontalHeader()->setClickEnabled(true);
   connect(t_csv->horizontalHeader(), SIGNAL(clicked(int)), SLOT(sort_column(int)));
   
   // probably I'm not understanding something, but these next two lines don't seem to do anything
   t_csv->horizontalHeader()->adjustHeaderSize();
   t_csv->adjustSize();
   
   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout

   QHBoxLayout *hbl_bottom = new QHBoxLayout;
   hbl_bottom->addSpacing(4);
   hbl_bottom->addWidget(pb_help);
   hbl_bottom->addSpacing(4);
   hbl_bottom->addWidget(pb_cancel);
   hbl_bottom->addSpacing(4);

   QVBoxLayout *background = new QVBoxLayout(this);
   background->addSpacing(4);
   background->addWidget(lbl_title);
   background->addSpacing(4);
   background->addWidget(t_csv);
   background->addSpacing(4);
   background->addLayout(hbl_bottom);
   background->addSpacing(4);
}

void US_Hydrodyn_Csv_Viewer::cancel()
{
   close();
}

void US_Hydrodyn_Csv_Viewer::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_csv_viewer.html");
}

void US_Hydrodyn_Csv_Viewer::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Csv_Viewer::sort_column( int section )
{
   t_csv->sortColumn(section, order_ascending, true);
   order_ascending = !order_ascending;
}
