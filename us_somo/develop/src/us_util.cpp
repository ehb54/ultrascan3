#include "../include/us3_defines.h"
#include "../include/us_util.h"
#include "qdatetime.h"
//Added by qt3to4:
#include <QTextStream>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>

OneLiner::OneLiner(const QString &message, QWidget *parent, 
                   const char *) : QDialog( parent )
{

   this->setGeometry(200,200,370, 320);
   
   lbl1 = new QLabel(message, this);
   lbl1->setGeometry(10,10,350,220);
   lbl1->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

   parameter1 = new QLineEdit(  this );    parameter1->setObjectName( "lineEdit" );
   parameter1->setGeometry( 10, 240, 350, 30 );
   connect( parameter1, SIGNAL(textChanged(const QString &)),
            SLOT(update(const QString &)) );

   ok = new QPushButton( us_tr("Ok"), this );
   ok->setAutoDefault(false);
   ok->setGeometry( 10,280, 174, 30 );
   connect( ok, SIGNAL(clicked()), SLOT(accept()) );

   cancel = new QPushButton( us_tr("Cancel"), this );
   cancel->setAutoDefault(false);
   cancel->setGeometry( 186,280, 174, 30 );
   connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

}

OneLiner::~OneLiner()
{
   delete cancel;
   delete ok;
   delete lbl1;
   delete parameter1;
}

void OneLiner::update(const QString &Textstr)
{
   string = Textstr;
}

US_FitParameter::US_FitParameter(QWidget *parent, const char *) : QWidget( parent )
{
   USglobal = new US_Config();
   int buttonh = 26;
   int xpos = 0;
   int ypos = 0;

   setPalette( PALET_FRAME );
   
   le_value = new QLineEdit(this);
   le_value->setGeometry(xpos, ypos, column2, buttonh);
   le_value->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_value->setPalette( PALET_NORMAL );
   AUTFBACK( le_value );

   xpos += column2 + spacing;

   le_range = new QLineEdit(this);
   le_range->setGeometry(xpos, ypos, column3, buttonh);
   le_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_range->setPalette( PALET_NORMAL );
   AUTFBACK( le_range );

   xpos += column3 + spacing;

   lbl_unit = new QLabel("", this);
   lbl_unit->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_unit->setPalette( PALET_FRAME );
   AUTFBACK( lbl_unit );
   lbl_unit->setGeometry(xpos+5, ypos, column4-5, buttonh);
   lbl_unit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   xpos += column4 + spacing;

   cb_float = new QCheckBox(this);
   cb_float->setGeometry(xpos + (unsigned int) (column5/2) - 7, ypos+5, 14, 14);
   cb_float->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_float->setPalette( PALET_NORMAL );
   AUTFBACK( cb_float );

   xpos += column5 + spacing;

   cb_constrained = new QCheckBox(this);
   cb_constrained->setGeometry(xpos + (unsigned int) (column5/2) - 7, ypos+5, 14, 14);
   cb_constrained->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_constrained->setPalette( PALET_NORMAL );
   AUTFBACK( cb_constrained );

   xpos += column5 + spacing;

   lbl_bounds = new QLabel("", this);
   lbl_bounds->setGeometry(xpos + (unsigned int) (column6/2) - 7, ypos+5, 14, 14);
   lbl_bounds->setFixedSize(15,15);
   lbl_bounds->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_bounds->setPalette( USglobal->global_colors.cg_dk_green );
   
      
   QGridLayout * legendLayout = new QGridLayout( this ); legendLayout->setContentsMargins( 0, 0, 0, 0 ); legendLayout->setSpacing( 0 ); legendLayout->setSpacing( 0 ); legendLayout->setContentsMargins( 0, 0, 0, 0 );
   legendLayout->addWidget(le_value,0,0);
   legendLayout->setColumnMinimumWidth(0,column2);
   legendLayout->addWidget(le_range,0,1);
   legendLayout->setColumnMinimumWidth(1,column3);
   legendLayout->addWidget(lbl_unit,0,2);
   legendLayout->setColumnMinimumWidth(2,column4-5);
   legendLayout->addWidget(cb_float,0,3);
   legendLayout->setColumnMinimumWidth(3,column5/2-7);
   legendLayout->setColumnMinimumWidth(4,column5/2);
   legendLayout->addWidget(cb_constrained,0,5);
   legendLayout->setColumnMinimumWidth(5,column5/2-7);
   legendLayout->setColumnMinimumWidth(6,column5/2);
   legendLayout->addWidget(lbl_bounds,0,7);
   legendLayout->setColumnMinimumWidth(7,column6/2-7);
   legendLayout->setColumnMinimumWidth(8,column5/2);



}

US_FitParameter::~US_FitParameter()
{
}

void US_FitParameter::setEnabled(bool choice)
{
   QString str;
   if (!choice)
   {
      le_value->setText("");
      le_range->setText("");
   }
   le_value->setEnabled(choice);
   le_range->setEnabled(false);
   cb_float->setChecked(false);
   cb_float->setEnabled(choice);
   cb_constrained->setChecked(false);
   cb_constrained->setEnabled(choice);
}

void US_FitParameter::updateValue(float value)
{
   QString str;
   str.sprintf("%1.5e", value);
   le_value->setText(str);
}

void US_FitParameter::updateRange(float range)
{
   QString str;
   str.sprintf("%1.5e", range);
   le_range->setText(str);
}

void US_FitParameter::setUnit(const QString &str)
{
   lbl_unit->setText(str);
}

void US_FitParameter::setFloatRed()
{
   lbl_bounds->setPalette( USglobal->global_colors.cg_red );
}

void US_FitParameter::setFloatGreen()
{
   lbl_bounds->setPalette( USglobal->global_colors.cg_green );
}

void US_FitParameter::setFloatDisabled()
{
   lbl_bounds->setPalette( USglobal->global_colors.cg_dk_green );
}

US_FitParameterLegend::US_FitParameterLegend(QWidget *parent, const char *) : QWidget( parent )
{
   USglobal = new US_Config();

   int buttonh = 26;
   int xpos = 0;
   int ypos = 0;

   QString str;
   
   lbl_legend1 = new QLabel(us_tr("Parameter:"),this);
   lbl_legend1->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_legend1->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_legend1->setPalette( PALET_LABEL );
   AUTFBACK( lbl_legend1 );
   lbl_legend1->setGeometry(xpos, ypos, span, buttonh);
   lbl_legend1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column1 + spacing;

   lbl_legend2 = new QLabel(us_tr("Initial Guess:"),this);
   lbl_legend2->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
   lbl_legend2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_legend2->setPalette( PALET_LABEL );
   AUTFBACK( lbl_legend2 );
   lbl_legend2->setGeometry(xpos, ypos, column2, buttonh);
   lbl_legend2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column2 + spacing;

   lbl_legend3 = new QLabel(us_tr("+/- Bounds:"),this);
   lbl_legend3->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
   lbl_legend3->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_legend3->setPalette( PALET_LABEL );
   AUTFBACK( lbl_legend3 );
   lbl_legend3->setGeometry(xpos, ypos, column3, buttonh);
   lbl_legend3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column3 + column4 + 2 * spacing;

   lbl_legend4 = new QLabel(us_tr("Float:"),this);
   lbl_legend4->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
   lbl_legend4->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_legend4->setPalette( PALET_LABEL );
   AUTFBACK( lbl_legend4 );
   lbl_legend4->setGeometry(xpos, ypos, column5, buttonh);
   lbl_legend4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column5 + spacing;

   lbl_legend5 = new QLabel(us_tr("Lock:"),this);
   lbl_legend5->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_legend5->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
   lbl_legend5->setPalette( PALET_LABEL );
   AUTFBACK( lbl_legend5 );
   lbl_legend5->setGeometry(xpos, ypos, column6, buttonh);
   lbl_legend5->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column5 + spacing;

   lbl_legend6 = new QLabel(us_tr("Bound:"),this);
   lbl_legend6->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_legend6->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
   lbl_legend6->setPalette( PALET_LABEL );
   AUTFBACK( lbl_legend6 );
   lbl_legend6->setGeometry(xpos, ypos, column6, buttonh);
   lbl_legend6->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   setGeometry(xpos, ypos, span, buttonh);
   show();
   
   QGridLayout * legendLayout = new QGridLayout( this ); legendLayout->setContentsMargins( 0, 0, 0, 0 ); legendLayout->setSpacing( 0 ); legendLayout->setSpacing( 0 ); legendLayout->setContentsMargins( 0, 0, 0, 0 );
   legendLayout->addWidget(lbl_legend1,0,0);
   legendLayout->setColumnMinimumWidth(0,column1+column5);
   legendLayout->addWidget(lbl_legend2,0,1);
   legendLayout->setColumnMinimumWidth(1,column2);
   legendLayout->addWidget(lbl_legend3,0,2);
   legendLayout->setColumnMinimumWidth(2,column3);
   legendLayout->addWidget(lbl_legend4,0,3);
   //   legendLayout->setColumnMinimumWidth(3,column5);
   legendLayout->addWidget(lbl_legend5,0,4);
   //   legendLayout->setColumnMinimumWidth(4,column6);
   legendLayout->addWidget(lbl_legend6,0,5);
   //   legendLayout->setColumnMinimumWidth(5,column6);
   
}

US_FitParameterLegend::~US_FitParameterLegend()
{
}

void getHeader(QString *header, const QString title)
{
   QDate date;
   date = QDate::currentDate();
   *header = "<html>\n<head>\n   <title>" + title + "</title>\n"
      "   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n"
      "   <!-- Author: UltraScan II for Unix Data Analysis Software " + date.toString() + " -->\n"
      "   <link href=\"mainstyle.css\" rel=\"styleSheet\" type=\"text/css\">\n</head>\n"
      "<body text=\"#000000\" bgcolor=\"#FFFFFF\" link=\"#0000EF\" vlink=\"#55188A\" alink=\"#FF0000\">\n\n";
   *header += "<img src=ultrascan1.png><p><hr>\n";
}

void getFooter(QString *footer)
{
   QDate date;
   date = QDate::currentDate();
   QString year;
   year.sprintf("%d", date.year());
   *footer = "<hr><font face=helvetica size=-1>\nUltraScan Software Contact: <a href=mailto:borries.demeler@umontana.edu>Borries Demeler</a>\n";
   *footer += "<br>\nThis document has been generated with the <i><b>UltraScan II</i></b> Data Analysis Software distribution.\n<br>\n";
   *footer += "All rights reserved, Copyright " + year + " <a href=http://www.uthscsa.edu>The University of";
   *footer += "Texas Health Science Center at San Antonio</a>.\n<br>UltraScan Home Page: ";
   *footer += "<a href=http://www.ultrascan.uthscsa.edu>http://www.ultrascan.uthscsa.edu</a>\n<p>\n";
   *footer += "This report was generated on " + date.toString() + "\n</font></body>\n</html>";
}

QString getToken(QString *str, const QString &separator)
{
// beginning from the left, look for the separator and 
// copy the left portion into token (to be returned as
// QString) and return the remaining string on the right
// without the separator in *str.
   int pos;
   QString token;
   pos = str->indexOf(separator, 0, Qt::CaseInsensitive );
   if (pos < 0)
   {
      if (str->length() > 0)
      {
         token = (*str);
         (*str) = "";
         return (token);
      }
      else
      {
         return((QString) "");
      }
   }
   while (pos == 0)
   {
      (*str) = str->mid(pos + 1, str->length());   
      pos = str->indexOf(separator, 0, Qt::CaseInsensitive );
      if (pos < 0)
      {
         if (str->length() > 0)
         {
            token = (*str);
            (*str) = "";
            return (token);
         }
         else
         {
            return((QString) "");
         }
      }
   }
   token = str->left(pos);
   (*str) = str->mid(pos + 1, str->length());
   return(token);
}

bool readCenterpieceInfo(vector <struct centerpieceInfo> *cp_info_vector)
{
   struct centerpieceInfo temp_cp_info;
   US_Config *USglobal;
   USglobal = new US_Config();   
   QString str1, str2;
   bool ok;
#ifdef WIN32
   QFile cp_file(USglobal->config_list.system_dir + "\\etc\\centerpiece.dat");
#else
   QFile cp_file(USglobal->config_list.system_dir + "/etc/centerpiece.dat");
#endif
   if(cp_file.open(QIODevice::ReadOnly))
   {
      QTextStream ts(&cp_file);
      while(!ts.atEnd())
      {
         str1 = ts.readLine();   // process line by line
         str2 = getToken(&str1, " "); // get the first token of the line
         temp_cp_info.serial_number = str2.toInt(&ok, 10);
         if (ok)  // if the first token is not an integer, we got a comment and we'll skip this line
         {
            temp_cp_info.material = getToken(&str1, " ");
            str2 = getToken(&str1, " ");
            temp_cp_info.channels = str2.toInt(&ok, 10);
            for (unsigned int i=0; i<temp_cp_info.channels; i++)
            {
               str2 = getToken(&str1, " ");
               temp_cp_info.bottom_position[i] = str2.toFloat(&ok);
            }
            str2 = getToken(&str1, " ");
            temp_cp_info.sector = str2.toInt(&ok, 10);
            str2 = getToken(&str1, " ");
            temp_cp_info.pathlength = str2.toFloat(&ok);
            str2 = getToken(&str1, " ");
            temp_cp_info.angle = str2.toFloat(&ok);
            str2 = getToken(&str1, " ");
            temp_cp_info.width = str2.toFloat(&ok);
            (*cp_info_vector).push_back(temp_cp_info);
         }
      }
      cp_file.close();
      return(true);
   }
   else
   {
      return(false);
   }
}

bool readRotorInfo(vector <struct rotorInfo> *rotor_info_vector)
{
   struct rotorInfo temp_rotor_info;
   US_Config *USglobal;
   QString str1, str2;
   bool ok;
   USglobal = new US_Config();   
#ifdef WIN32
   QFile rotor_file(USglobal->config_list.system_dir + "\\etc\\rotor.dat");
#else
   QFile rotor_file(USglobal->config_list.system_dir + "/etc/rotor.dat");
#endif
   delete USglobal;
   if(rotor_file.open(QIODevice::ReadOnly))
   {
      QTextStream ts(&rotor_file);
      while(!ts.atEnd())
      {
         str1 = ts.readLine();   // process line by line
         str2 = getToken(&str1, " "); // get the first token of the line
         temp_rotor_info.serial_number = str2.toInt(&ok, 10);
         if (ok)  // if the first token is not an integer, we got a comment and we'll skip this line
         {
            temp_rotor_info.type = getToken(&str1, " ");
            for (int i=0; i<5; i++)
            {
               str2 = getToken(&str1, " ");
               temp_rotor_info.coefficient[i] = str2.toFloat(&ok);
            }
            (*rotor_info_vector).push_back(temp_rotor_info);
         }
      }
      rotor_file.close();
      return (true);
   }
   else
   {
      return(false);
   }
}

int Move (const QString &sourcefile, const QString &destfile)
{
   QFile f1, f2;
   char *buf;
   buf = new char[1024];
   f1.setFileName(sourcefile);
   f2.setFileName(destfile);

   if (f1.exists())
   {
      if(!f1.open(QIODevice::ReadOnly))
      {
         return -1;
      }
      if(!f2.open(QIODevice::WriteOnly))
      {
         return -2;
      }
      int size;
      while((size = f1.read(buf,1024)) != 0)
      {
         f2.write(buf, size);
      }      
      f1.close();
      f2.close();
      if(f1.remove() == false)
      {
         return -3;
      }
   }
   else
   {
      return -4;
   }
   delete buf;
   return 0;

}

int copy(const QString &sourcefile, const QString &destfile)
{
   if (sourcefile == destfile) return (-3);
   QFile f1, f2;
   char *buf;
   buf = new char[1024];
   f1.setFileName(sourcefile);
   f2.setFileName(destfile);
   if (f1.exists())
   {
      if(!f1.open(QIODevice::ReadOnly))
      {
         return -1;
      }
      if(!f2.open(QIODevice::WriteOnly))
      {
         return -2;
      }
      int size;
      while((size = f1.read(buf, 1024)) != 0)
      {
         f2.write(buf, size);
      }      
      f1.close();
      f2.close();
   }
   else
   {
      return -4;
   }
   delete buf;
   return 0;
}

/*
  void view_file(const QString &filename)
  {
  TextEdit *e;
  e = new TextEdit();
  e->setPalette( PALET_NORMAL );
  AUTFBACK( e );
  e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
  e->load(filename);
  e->show();
  }
*/
void view_image(
                const QString 
#if QT_VERSION < 0x040000
                &filename
#endif
                )
{
#if QT_VERSION < 0x040000
   US_ImageViewer *us_imgviewer;
   us_imgviewer = new US_ImageViewer(0, "new window", Qt::WDestructiveClose | Qt::WResizeNoErase);
   us_imgviewer->openFile(filename);
   us_imgviewer->show();   
#endif
}


/*
// initialize all static run_inf struct members
void initialize_runinfo(struct runinfo run_inf)   
{
run_inf.data_dir = "";
run_inf.run_id = "";
run_inf.avg_temperature = 0.0;
run_inf.time_correction = 0.0;
run_inf.duration = 0.0;
run_inf.total_scans = 0;
run_inf.delta_r = 0.0;
run_inf.expdata_id = 0;
run_inf.investigator = -1;
run_inf.date = "";
run_inf.description = "";
run_inf.dbname = "";
run_inf.dbhost = "";
run_inf.dbdriver = "";
for (int i=0; i<8; i++)   
{
   run_inf.centerpiece[i] = -2;
   run_inf.cell_id[i] = "";
   run_inf.wavelength_count[i] = 0;
   run_inf.meniscus[i] = 0.0;
   for (int j=0; j<4; j++)
   {
      run_inf.DNA_serialnumber[i][j] = -1;
      run_inf.buffer_serialnumber[i][j] = -1;
      run_inf.peptide_serialnumber[i][j] = -1;
   }
   for (int j=0; j<3; j++)
   {
      run_inf.wavelength[i][j] = 0;
      run_inf.scans[i][j] = 0;
      run_inf.baseline[i][j] = 0;
      for (int k=0; k<4; k++)
      {
         run_inf.range_left[i][j][k] = 0.0;
         run_inf.range_right[i][j][k] = 0.0;
         run_inf.points[i][j][k] = 0;
         run_inf.point_density[i][j][k] = 0.0;
      }
   }
}
}*/


bool US_Static::lv_any_selected( QTreeWidget *lv ) {
   bool any_selected = false;
#if QT_VERSION < 0x040000
   QTreeWidgetItem *lvi = lv->firstChild();
   if ( lvi )
   {
      do {
         if ( lvi->isSelected() )
         {
            any_selected = true;
         }

      } while ( ( lvi = lvi->nextSibling() ) );
   }
#else
   any_selected = lv->selectedItems().count() > 0;
#endif
   return any_selected;
}

bool US_Static::lv_any_unselected( QTreeWidget *lv ) {
   bool any_unselected = false;
#if QT_VERSION < 0x040000
   QTreeWidgetItem *lvi = lv->firstChild();
   if ( lvi )
   {
      do {
         if ( !lvi->isSelected() )
         {
            any_unselected = true;
         }

      } while ( ( lvi = lvi->nextSibling() ) );
   }
#else
   QTreeWidgetItemIterator it( lv , QTreeWidgetItemIterator::Unselected );
   any_unselected = *it;
#endif
   return any_unselected;
}

void US_Static::lv_select_all_or_none( QTreeWidget *lv ) {
   bool any_not_selected = US_Static::lv_any_unselected( lv );
#if QT_VERSION < 0x040000
   QTreeWidgetItem *lvi = lv->firstChild();
   if ( lvi )
   {
      do {
         lv->setSelected( lvi, any_not_selected );
      } while ( ( lvi = lvi->nextSibling() ) );
   }
#else
   if ( any_not_selected ) {
      lv->selectAll();
   } else {
      lv->clearSelection();
   }
#endif
}
      
int US_Static::lvi_depth( QTreeWidgetItem *lvi ) {
#if QT_VERSION < 0x040000
   return lvi->depth();
#else
   int depth = 0;
   while ( lvi ) {
      depth++;
      lvi = lvi->parent();
   }
   if ( depth ) {
      --depth;
   }
   return depth;
#endif
}

QTreeWidgetItem * US_Static::lv_lastItem( QTreeWidget *lv ) {
#if QT_VERSION < 0x040000
   return lv->lastItem();
#else
   QTreeWidgetItemIterator it( lv );
   QTreeWidgetItem *lvi = (QTreeWidgetItem *)0;

   while ( *it ) {
      lvi = *it;
      ++it;
   }
   return lvi;
#endif
}

#if QT_VERSION >= 0x040000
// QString us_tr( QString qs ) {
//    return QObject::tr( qPrintable( qs ) );
// }

// const char * us_trp( QString qs ) {
//    return qPrintable( us_tr( qPrintable( qs ) ) );
// }

// void us_qdebug( QString qs ) {
//    qDebug() << qPrintable( qs );
// }

// FILE * US_Static::us_fopen( QString f, const char *mode ) {
//    return fopen( qPrintable( f ), mode );
// }


double US_Static::getDouble(const QString & title, const QString & label, double value, double min, double max, int decimals, bool * ok, QWidget * parent, const char * ) {
    return QInputDialog::getDouble(parent, title, label, value, min, max, decimals, ok );
}

int US_Static::getInteger(const QString & title, const QString & label, int value, int min, int max, int step, bool * ok, QWidget * parent, const char * ) {
#if QT_VERSION >= 0x050000
   return QInputDialog::getInt(parent, title, label, value, min, max, step, ok );
#else
   return QInputDialog::getInteger(parent, title, label, value, min, max, step, ok );
#endif
}

QString US_Static::getItem(const QString & title, const QString & label, const QStringList & list, int current, bool editable, bool * ok, QWidget * parent, const char * ) {
   return QInputDialog::getItem(parent, title, label, list, current, editable, ok ) ;
}

QString US_Static::getText(const QString & title, const QString & label, QLineEdit::EchoMode echo, const QString & text, bool * ok, QWidget * parent, const char * ) {
   return QInputDialog::getText(parent, title, label, echo, text, ok );
}

int US_Static::us_message(const QString & title, const QString & text, const QString & buttonText, QWidget * parent, const char *) {
   if ( !buttonText.isEmpty() ) {
      qDebug() << "us_message title: '" << title << "' text '" << text << "' has buttonText '" << buttonText << "'";
   }
   return QMessageBox::information( parent, title, text );
}
#endif
