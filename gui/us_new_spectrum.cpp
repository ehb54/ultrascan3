//! \file us_solution_gui.cpp

#include "us_solution_gui.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_investigator.h"
#include "us_buffer_gui.h"
#include "us_analyte_gui.h"
#include "us_solution.h"
#include "us_new_spectrum.h"
#include "us_editor_gui.h"
#include "us_table.h"


#if QT_VERSION < 0x050000
#define setSamples(a,b,c) setData(a,b,c)
#define setSymbol(a) setSymbol(*a)
#endif

US_NewSpectrum::US_NewSpectrum( QString tmp_type, const QString& tmp_text, const QString &tmp_text_e280, US_Buffer *tmp_buffer ) : US_Widgets()
{
   buffer      = tmp_buffer;
   text        = tmp_text;
   type        = tmp_type;
   text_e280   = tmp_text_e280;
   
   setPalette( US_GuiSettings::frameColor() );
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
   
   this->setMinimumSize( 480, 45 );

   setWindowTitle( tr( "Spectrum Management" ) );

   QLabel* bn_info     = us_banner( tr( "Select how spectrum will be uploaded" ) );
   bn_info->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   pb_cancel     = us_pushbutton( tr( "Cancel" ) );
   pb_manual     = us_pushbutton( tr( "Enter Manually" ) );
   pb_uploadDisk = us_pushbutton( tr( "Upload from Disk" ) );
   pb_uploadFit  = us_pushbutton( tr( "Upload and Fit" ) );

   int row = 0;
   main->addWidget( bn_info,         row++, 0, 1, 8 );
   main->addWidget( pb_cancel,       row,   0, 1, 2 );
   main->addWidget( pb_manual,       row,   2, 1, 2 );
   main->addWidget( pb_uploadDisk,   row,   4, 1, 2 );
   main->addWidget( pb_uploadFit,    row,   6, 1, 2 );

   connect( pb_cancel,      SIGNAL( clicked()  ),
            this,           SLOT  ( cancel() ) ); 
   connect( pb_manual,      SIGNAL( clicked()  ),
            this,           SLOT  ( entermanually() ) );     
   connect( pb_uploadDisk,  SIGNAL( clicked()  ),
            this,           SLOT  ( uploadDisk() ) );
   connect( pb_uploadFit,   SIGNAL( clicked()  ),
	    this,           SLOT  ( uploadFit() ) );

}

US_NewSpectrum::US_NewSpectrum( QString tmp_type, const QString& tmp_text, const QString &tmp_text_e280, US_Analyte *tmp_analyte ) : US_Widgets()
{
   analyte     = tmp_analyte;
   text        = tmp_text;
   type        = tmp_type;
   text_e280   = tmp_text_e280;
   
   setPalette( US_GuiSettings::frameColor() );
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
   
   this->setMinimumSize( 480, 45 );
   
   setWindowTitle( tr( "Spectrum Management" ) );

   QLabel* bn_info     = us_banner( tr( "Select how spectrum will be uploaded" ) );
   bn_info->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   pb_cancel     = us_pushbutton( tr( "Cancel" ) );
   pb_manual     = us_pushbutton( tr( "Enter Manually" ) );
   pb_uploadDisk = us_pushbutton( tr( "Upload from Disk" ) );
   pb_uploadFit  = us_pushbutton( tr( "Upload and Fit" ) );

   int row = 0;
   main->addWidget( bn_info,         row++, 0, 1, 8 );
   main->addWidget( pb_cancel,       row,   0, 1, 2 );
   main->addWidget( pb_manual,       row,   2, 1, 2 );
   main->addWidget( pb_uploadDisk,   row,   4, 1, 2 );
   main->addWidget( pb_uploadFit,    row,   6, 1, 2 );

   connect( pb_cancel,      SIGNAL( clicked()  ),
            this,           SLOT  ( cancel() ) ); 
   connect( pb_manual,      SIGNAL( clicked()  ),
            this,           SLOT  ( entermanually() ) );     
   connect( pb_uploadDisk,  SIGNAL( clicked()  ),
            this,           SLOT  ( uploadDisk() ) );
   connect( pb_uploadFit,   SIGNAL( clicked()  ),
	    this,           SLOT  ( uploadFit() ) );
}


US_NewSpectrum::US_NewSpectrum( QString tmp_type, const QString& tmp_text, const QString &tmp_text_e280, US_Solution *tmp_solution ) : US_Widgets()
{
   solution    = tmp_solution;
   text        = tmp_text;
   type        = tmp_type;
   text_e280   = tmp_text_e280;
   
   setPalette( US_GuiSettings::frameColor() );
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
   
   this->setMinimumSize( 480, 45 );
   
   setWindowTitle( tr( "Spectrum Management" ) );

   QLabel* bn_info     = us_banner( tr( "Select how spectrum will be uploaded" ) );
   bn_info->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   pb_cancel     = us_pushbutton( tr( "Cancel" ) );
   pb_manual     = us_pushbutton( tr( "Enter Manually" ) );
   pb_uploadDisk = us_pushbutton( tr( "Upload from Disk" ) );
   pb_uploadFit  = us_pushbutton( tr( "Upload and Fit" ) );

   int row = 0;
   main->addWidget( bn_info,         row++, 0, 1, 8 );
   main->addWidget( pb_cancel,       row,   0, 1, 2 );
   main->addWidget( pb_manual,       row,   2, 1, 2 );
   main->addWidget( pb_uploadDisk,   row,   4, 1, 2 );
   main->addWidget( pb_uploadFit,    row,   6, 1, 2 );

   connect( pb_cancel,      SIGNAL( clicked()  ),
            this,           SLOT  ( cancel() ) ); 
   connect( pb_manual,      SIGNAL( clicked()  ),
            this,           SLOT  ( entermanually() ) );     
   connect( pb_uploadDisk,  SIGNAL( clicked()  ),
            this,           SLOT  ( uploadDisk() ) );
   connect( pb_uploadFit,   SIGNAL( clicked()  ),
	    this,           SLOT  ( uploadFit() ) );
}

void US_NewSpectrum::uploadDisk(void)
{
  add_spectrumDisk();  
}

void US_NewSpectrum::uploadFit(void)
{    
    w_spec = new US_Extinction(type, text, text_e280, (QWidget*)this); 
    
    connect( w_spec, SIGNAL( get_results(QMap < double, double > & )), this, SLOT(process_results( QMap < double, double > & ) ) );
    
    w_spec->setParent(this, Qt::Window);
    w_spec->setAttribute(Qt::WA_DeleteOnClose);
    w_spec->show(); 

}

void US_NewSpectrum::process_results(QMap < double, double > &xyz)
{
  if ( type == "BUFFER")
    buffer->extinction = xyz;
  if ( type == "ANALYTE")
    {
      analyte->extinction = xyz;
      //US_AnalyteMgrNew::le_protein_e280->setText( QString::number(analyte->extinction[280.0]) );
    }
  if ( type == "SOLUTION")
    solution->extinction = xyz;
   
   QMap<double, double>::iterator it;
   QString output;

   for (it = xyz.begin(); it != xyz.end(); ++it) {
      // Format output here.
      output += QString(" %1 : %2 /n").arg(it.key()).arg(it.value());
   }

   QMessageBox::information( this,
      tr( "Test: Data transmitted" ),
      tr( "Number of keys in extinction QMAP: %1 . You may click 'Accept'"
          " from the main window to write new %2 into DB" )
			     .arg( xyz.keys().count() ).arg( type ) );  

   if ( type == "BUFFER")
     buffer->new_or_changed_spectrum = true;
   if ( type == "ANALYTE")
     analyte->new_or_changed_spectrum = true;
   if ( type == "SOLUTION")
     solution->new_or_changed_spectrum = true;
    
   w_spec->close(); 
   this->close();
   
   emit change_prot_e280();
}

void US_NewSpectrum::add_spectrumDisk( void )
{
  QStringList files;
  QFile f;
  
  QFileDialog dialog (this);
  //dialog.setNameFilter(tr("Text (*.txt *.csv *.dat *.wa *.dsp)"));

  dialog.setNameFilter(tr("Text files (*.[Tt][Xx][Tt] *.[Cc][Ss][Vv] *.[Dd][Aa][Tt] *.[Ww][Aa]* *.[Dd][Ss][Pp]);;All files (*)"));
    
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setViewMode(QFileDialog::Detail);
  //dialog.setDirectory("/home/alexsav/ultrascan/data/spectra");
  
  QString work_dir_data  = US_Settings::dataDir();
  qDebug() << work_dir_data;
  dialog.setDirectory(work_dir_data);
  
  if(dialog.exec())
    {
      files = dialog.selectedFiles();
      readingspectra(files[0]);
    }
  //qDebug() << "Files: " << files[0];
}

void US_NewSpectrum::readingspectra(const QString &fileName)
{
  QString str1;
  QStringList strl;
  float temp_x, temp_y;
  QMap< double, double > temp_extinct;
  
  if(!fileName.isEmpty())
    {
      QFile f(fileName);
     
      if(f.open(QIODevice::ReadOnly | QIODevice::Text))
	{
	  QTextStream ts(&f);
	  while(!ts.atEnd())
	    {
	      if( !ts.atEnd() )
		{
		  str1 = ts.readLine();
		}
	      str1 = str1.simplified();
	      str1 = str1.replace("\"", " ");
	      str1 = str1.replace(",", " ");
	      strl = str1.split(" ");
	      temp_x = strl.at(0).toFloat();
	      temp_y = strl.at(1).toFloat();

	      //qDebug() << temp_x << ", " << temp_y;

	      if (temp_x != 0)
		{
		  temp_extinct[double(temp_x)] = double(temp_y);
		}
	    }
	}
      if ( type == "BUFFER")
	{
	  buffer->extinction = temp_extinct;
	  buffer->new_or_changed_spectrum = true;
	}
      if ( type == "ANALYTE")
	{
	  analyte->extinction = temp_extinct;
	  analyte->new_or_changed_spectrum = true;
	}
      if ( type == "SOLUTION")
	{
	  solution->extinction = temp_extinct;
	  solution->new_or_changed_spectrum = true;
	}
    }
  this->close();
  emit change_prot_e280();
}

void US_NewSpectrum::entermanually( void )
{
    US_Table* sdiag;
    
    loc_extinct.clear();

    if ( type == "BUFFER")
      loc_extinct = buffer->extinction;
    if ( type == "ANALYTE")
      loc_extinct = analyte->extinction;
    if ( type == "SOLUTION")
      loc_extinct = solution->extinction;
   
    QString stype( "Extinction" );
    bool changed = false;
    sdiag        = new US_Table( loc_extinct, stype, changed, this );
    sdiag->setWindowTitle( "Manage Extinction Spectrum" );
    sdiag->exec();
    
    if ( changed )
      {
	qDebug() << "Existing: Inside Changed: "; 
	
	if ( type == "BUFFER")
	  {
	    buffer->extinction.clear();                                                   
	    buffer->extinction = loc_extinct;
	    buffer->new_or_changed_spectrum = true;
	  }
	if ( type == "ANALYTE")
	  {
	    analyte->extinction.clear();                                                   
	    analyte->extinction = loc_extinct;
	    analyte->new_or_changed_spectrum = true;
	  }
	if ( type == "SOLUTION")
	  {
	    solution->extinction.clear();                                                   
	    solution->extinction = loc_extinct;
	    solution->new_or_changed_spectrum = true;
	  }

	////Update via STORED Procedures ....
	//analyte->replace_spectrum = true; 
      }
    this->close();
    emit change_prot_e280();
}

void US_NewSpectrum::cancel(void)
{
  this->close();
}

/// View Spectrum ///////////////////////////
US_ViewSpectrum::US_ViewSpectrum(QMap<double,double>& tmp_extinction) : US_Widgets()
{
  extinction = tmp_extinction;
  
  data_plot = new QwtPlot();
  //changedCurve = NULL;
  plotLayout = new US_Plot(data_plot, tr(""), tr("Wavelength(nm)"), tr(""));
  data_plot->setCanvasBackground(Qt::black);
  data_plot->setTitle("Extinction Profile");
  data_plot->setMinimumSize(560, 240);
  //data_plot->enableAxis(1, true);
  data_plot->setAxisTitle(0, "Extinction OD/(mol*cm)");

  us_grid(data_plot);

  QPushButton *pb_save = us_pushbutton(tr("Save to CSV"));
  connect(pb_save, SIGNAL(clicked()), this, SLOT(save_csv()));
   
  QGridLayout* main;
  main = new QGridLayout(this);
  main->setSpacing(2);
  main->setContentsMargins(0,0,0,0);
  main->addLayout(plotLayout, 0, 0, 1, 8);
  main->addWidget(pb_save,    1, 6, 1, 2);

  plot_extinction();

}

void US_ViewSpectrum::plot_extinction()
{ 
  QVector <double> x;
  QVector <double> y;
  
  QMap<double, double>::iterator it;
  
  for (it = extinction.begin(); it != extinction.end(); ++it) {
    x.push_back(it.key());
    y.push_back(it.value());
  }
  
  QwtSymbol* symbol = new QwtSymbol;
  symbol->setSize(10);
  symbol->setPen(QPen(Qt::blue));
  symbol->setBrush(Qt::yellow);
  symbol->setStyle(QwtSymbol::Ellipse);
  
  QwtPlotCurve *spectrum;
  spectrum = us_curve(data_plot, "Spectrum Data");
  spectrum->setSymbol(symbol);    
  spectrum->setSamples( x.data(), y.data(), (int) x.size() );
  data_plot->replot();
}

void US_ViewSpectrum::save_csv(){
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                  US_Settings::reportDir(),
                                                  tr("CSV (*.csv)"));
  if (fileName.isEmpty()) return;

  if (! (fileName.endsWith(".csv") or fileName.endsWith(".CSV"))) {
    fileName.append(".csv");
  }  

  QVector <double> x;
  QVector <double> y;

  QMap<double, double>::iterator it;

  for (it = extinction.begin(); it != extinction.end(); ++it) {
    x.push_back(it.key());
    y.push_back(it.value());
  }

  QFile file{fileName};
  if (file.open(QIODevice::WriteOnly)) {
    QTextStream outStream{&file};
    outStream << tr("Lamda,Data\n");
    for (int i = 0; i < x.size(); i++){
        outStream << QString::number(x.at(i)) << ",";
        outStream << QString::number(y.at(i)) << "\n";
    }
  }
  file.close();
}
