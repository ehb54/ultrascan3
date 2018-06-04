//! \file us_proto_ranges.cpp

#include "us_experiment_gui.h"
#include "us_extinction_gui.h"
#include "us_table.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_sleep.h"
#include "us_util.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setMinimum(a)      setMinValue(a)
#define setMaximum(a)      setMaxValue(a)
#define QRegularExpression(a)  QRegExp(a)
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

// Panel for Ranges parameters
US_ExperGuiRanges::US_ExperGuiRanges( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ExperimentMain*)topw;
   rpRange             = &(mainw->currProto.rpRange);
   mxrow               = 24;     // Maximum possible rows
   nrnchan             = 0;
   protname            = mainw->currProto.protname;
   chrow               = -1;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "7: Specify wavelength and radius ranges" ) );
   panel->addWidget( lb_panel );
 
   QPushButton* pb_details  = us_pushbutton( tr( "View Current Range Settings" ) );
   connect( pb_details,   SIGNAL( clicked()       ),
            this,         SLOT  ( detailRanges()  ) );

   // Show listbox with ScanCount per stage
   cb_scancount      = new QComboBox;
   rpSpeed = &(mainw->currProto.rpSpeed);

   //cb_scancount->addItem( tr( "Stage %1. Number of Scans: %2 " ).arg(1).arg( 0 ) );
   
     
   QLabel* lb_hdr1     = us_banner( tr( "Cell / Channel" ) );
   QLabel* lb_hdr2     = us_banner( tr( "Wavelengths" ) );
   QLabel* lb_hdr3     = us_banner( tr( "Radius Ranges" ) );

   QGridLayout* banners = new QGridLayout();
   // banners->setContentsMargins( 2, 2, 2, 2 );
   // banners->setHorizontalSpacing( 2 );
   int row             = 0;
   banners->addWidget( pb_details,      row,   0, 1, 8 );
   banners->addWidget( cb_scancount,    row++,   8, 1, 8 );
   banners->addWidget( lb_hdr1,         row,   0, 1, 3 );
   banners->addWidget( lb_hdr2,         row,   3, 1, 6 );
   banners->addWidget( lb_hdr3,         row++, 9, 1, 7 );

   // banners->addWidget( lb_hdr1,         row,   0,  1, 8 );
   // banners->addWidget( lb_hdr2,         row,   8,  1, 13 );
   // banners->addWidget( lb_hdr3,         row++, 21, 1, 21 );

   QGridLayout* genL   = new QGridLayout();
   genL->setSpacing        ( 2 );
   genL->setContentsMargins( 2, 2, 2, 2 );
   
   row             = 0;

   QLabel*      cclabl;
   QPushButton* pbwavln;
   QLabel*      lbwlrng;
   QwtCounter*  ctradfr;
   QLabel*      lablto;
   QwtCounter*  ctradto;
   QString swavln   = tr( "Select Wavelengths" );
   QString srngto   = tr( "to" );
   QFont   ckfont   = QFont( US_GuiSettings::fontFamily(),
                             US_GuiSettings::fontSize  (),
                             QFont::Bold );
   QPalette ckpal   = US_GuiSettings::normalColor();

   for ( int ii = 0; ii < mxrow; ii++ )
   {  // Loop to build initial place-holder ranges rows
      QString scel;
      if      ( ii == 0 ) scel = QString( "1 / A" );
      else if ( ii == 1 ) scel = QString( "1 / B" );
      else if ( ii == 2 ) scel = QString( "2 / A" );
      else if ( ii == 3 ) scel = QString( "2 / B" );
      else                scel = QString( "none" );
      cclabl           = us_label( scel );
      pbwavln          = us_pushbutton( swavln );
      lbwlrng          = us_label( "2,  278 to 282" );
      ctradfr          = us_counter( 3, 5.6, 7.4, 5.8 );
      lablto           = us_label( srngto );
      ctradto          = us_counter( 3, 5.6, 7.4, 7.2 );
      QString strow    = QString::number( ii );
      cclabl ->setObjectName( strow + ": label" );
      pbwavln->setObjectName( strow + ": pb_wavln" );
      lbwlrng->setObjectName( strow + ": lb_wlrng" );
      ctradfr->setObjectName( strow + ": ct_radfr" );
      lablto ->setObjectName( strow + ": lb_to"    );
      ctradto->setObjectName( strow + ": ct_radto" );
            
      bool is_vis      = ( ii < 4 );

      genL->addWidget( cclabl,  row,    0, 1, 3 );
      genL->addWidget( pbwavln, row,    3, 1, 3 );
      genL->addWidget( lbwlrng, row,    6, 1, 3 );
      genL->addWidget( ctradfr, row,    9, 1, 3 );
      genL->addWidget( lablto,  row,   12, 1, 1 );
      genL->addWidget( ctradto, row++, 13, 1, 3 );

      cclabl ->setVisible( is_vis );
      pbwavln->setVisible( is_vis );
      lbwlrng->setVisible( is_vis );
      ctradfr->setVisible( is_vis );
      lablto ->setVisible( is_vis );
      ctradto->setVisible( is_vis );

      // connect( pbwavln, SIGNAL( clicked()           ),
      //          this,    SLOT  ( selectWavelengths() ) );

      connect( pbwavln, SIGNAL( clicked()           ),           //ALEXEY
	       this,    SLOT  ( Wavelengths_class() ) );      
      connect( ctradfr, SIGNAL( valueChanged     ( double ) ),
               this,    SLOT  ( changedLowRadius ( double ) ) );
      connect( ctradto, SIGNAL( valueChanged     ( double ) ),
               this,    SLOT  ( changedHighRadius( double ) ) );
#if 0
      connect( ckoptim, SIGNAL( toggled    ( bool )   ),
               this,    SLOT  ( checkOptima( bool )   ) );
      connect( pbsload, SIGNAL( clicked()             ),
               this,    SLOT  ( loadSpectrum()        ) );
      connect( pbsmanu, SIGNAL( clicked()             ),
               this,    SLOT  ( manualSpectrum()      ) );
#endif
      cc_labls << cclabl;
      cc_wavls << pbwavln;
      cc_lrngs << lbwlrng;
      cc_lrads << ctradfr;
      cc_hrads << ctradto;
      cc_lbtos << lablto;
   }

#if 0
   // Build initial dummy internal values
   rchans << "1 / A" << "1 / B" << "3 / A" << "3 / B";
   stypes << "load" << "manual" << "auto" << "manual";
   QList< double > lmbd;
   lmbd << 280.0;
   QList< double > valu;
   valu << 0.0;
   pwvlens << lmbd << lmbd << lmbd << lmbd; 
   pvalues << valu << valu << valu << valu;
   swvlens << lmbd << lmbd << lmbd << lmbd; 
#endif
   chrow            = 0;

   panel->addLayout(banners);
   genL->setAlignment(Qt::AlignTop);

   QScrollArea *scrollArea = new QScrollArea(this);
   QWidget *containerWidget = new QWidget;
   containerWidget->setLayout(genL);
   scrollArea->setWidgetResizable(true);
   scrollArea->setWidget(containerWidget);
   scrollArea->verticalScrollBar()->setFixedWidth(50);
   
   panel->addWidget(scrollArea);  

   //panel->addStretch();

   initPanel();
}

// Function to rebuild the Ranges protocol after Optical change
void US_ExperGuiRanges::rebuild_Ranges( void )
{
   int nuvvis          = sibIValue( "optical", "nuvvis" );
   int nrange_sv       = rpRange->nranges;
   nrnchan             = rchans.count();
DbgLv(1) << "EGRn: rbR: nuvvis" << nuvvis << "nrange_sv" << nrange_sv
	 << "nrnchan" << nrnchan << " rpRanges->nranges " << rpRange->nranges;

//   if ( nrange_sv == nuvvis  &&  nuvvis != 0 )
//      return;                           // No optical change means no rebuild

DbgLv(1) << "EGRn: rbR:  nrnchan" << nrnchan;
   if ( nrange_sv == 0 )
   {  // No existing Ranges protocol, so init with rudimentary one
      rpRange->nranges    = nuvvis;
      rpRange->chrngs.resize( nuvvis );
      nrnchan = nuvvis;                      //ALEXEY bug fixed
      QString uvvis       = tr( "UV/visible" );
      QStringList oprof   = sibLValue( "optical", "profiles" );
      int kuv             = 0;

      //ALEXEY BUG fixed missed resizing/filling out following arrays: rchans, swvlens, locrads, hicrads 
      rchans .resize( nrnchan );
      swvlens.resize( nrnchan );
      locrads.resize( nrnchan );
      hicrads.resize( nrnchan );

      for ( int ii = 0; ii < oprof.count(); ii++ )
      {
DbgLv(1) << "Rn:CONTENT " << oprof[ ii ];	
         if ( oprof[ ii ].contains( uvvis ) )
         {
	    rpRange->chrngs[ kuv ].channel  = oprof[ ii ].section( ":", 0, 0 );
            rpRange->chrngs[ kuv ].wvlens.clear();
            rpRange->chrngs[ kuv ].wvlens <<  280.0;
DbgLv(1) << "Rn:CONTENT 11 inside: channel, wavelength: " << rpRange->chrngs[ kuv ].channel << rpRange->chrngs[ kuv ].wvlens ;	   

           //ALEXEY wokr with rchans, swvlens, locrads, hicrads 
           rchans [ kuv ]       = rpRange->chrngs[ kuv ].channel;
	   int nwavl           = rpRange->chrngs[ kuv ].wvlens.count();
	   swvlens[ kuv ].clear();
	   
	   for ( int jj = 0; jj < nwavl; jj++ )
	     {
	       double wavelen      = rpRange->chrngs[ kuv ].wvlens[ jj ];
	       swvlens[ kuv ] << wavelen;
	       DbgLv(1) << "EGRn: rbR:   kuv jj " << kuv << jj << "wavelen" << wavelen;
	     }
	   
	   locrads[ kuv ]       = rpRange->chrngs[ kuv ].lo_rad;
	   hicrads[ kuv ]       = rpRange->chrngs[ kuv ].hi_rad;
	   
	   if ( ++kuv >= nuvvis )  break;
         }
      }
 DbgLv(1) << "EGRn: rbR:  dummy proto  oprof count" << oprof.count() << "nuvvis" << nuvvis << " rpRanges->nranges " << rpRange->nranges;
      return;
   }

DbgLv(1) << "RANGE_1";
   
   QString cur_pname   = sibSValue( "general", "protocol" );
DbgLv(1) << "EGRn: rbR:  protname" << protname << "cur_pname" << cur_pname;

   if ( protname != cur_pname )
   {  // Protocol has changed:  rebuild internals
      protname            = cur_pname;
      nrnchan             = nrange_sv;
      rpRange->nranges    = nuvvis;
      rchans .resize( nrnchan );
      swvlens.resize( nrnchan );
      locrads.resize( nrnchan );
      hicrads.resize( nrnchan );
DbgLv(1) << "EGRn: rbR: rbI -- nrnchan" << nrnchan;

      for ( int ii = 0; ii < nrnchan; ii++ )
      {
         rchans [ ii ]       = rpRange->chrngs[ ii ].channel;
         int nwavl           = rpRange->chrngs[ ii ].wvlens.count();
         swvlens[ ii ].clear();

         for ( int jj = 0; jj < nwavl; jj++ )
         {
            double wavelen      = rpRange->chrngs[ ii ].wvlens[ jj ];
            swvlens[ ii ] << wavelen;
DbgLv(1) << "EGRn: rbR:   ii jj " << ii << jj << "wavelen" << wavelen;
         }

         locrads[ ii ]       = rpRange->chrngs[ ii ].lo_rad;
         hicrads[ ii ]       = rpRange->chrngs[ ii ].hi_rad;
DbgLv(1) << "EGRn: rbR:  ii lorad hirad" << locrads[ii] << hicrads[ii];
      }
      return;
   }

   // Save info from any previous protocol
   QVector< US_RunProtocol::RunProtoRanges::Ranges > chrngs_sv;
   chrngs_sv           = rpRange->chrngs;
   rpRange->chrngs.clear();
   QStringList oprofs  = sibLValue( "optical", "profiles" );
   int nochan          = oprofs.count();
DbgLv(1) << "EGRn: rbR:  nochan" << nochan;

   // Save info from current panel parameters
   int nrnchan_sv      = nrnchan;
   int ntchan          = nrnchan_sv + nochan;
DbgLv(1) << "EGRn: rbR:  nrnchan_s ntchan" << nrnchan_sv << ntchan;
   if ( nrnchan_sv > 0 )
   {
      rchans .resize( ntchan );
      swvlens.resize( ntchan );
      locrads.resize( ntchan );
      hicrads.resize( ntchan );
      int kk              = nochan;

      for ( int ii = 0; ii < nrnchan_sv; ii++ )
      {
         rchans [ kk ]    = rchans [ ii ];
         swvlens[ kk ]    = swvlens[ ii ];
         locrads[ kk ]    = locrads[ ii ];
         hicrads[ kk ]    = hicrads[ ii ];
         rchans [ ii ]    = "";
      }
   }

   // Now rebuild panel parameters and protocol
   QString uvvis       = tr( "UV/visible" );
DbgLv(1) << "EGRn: rbR:  nochan" << nochan;
   for ( int ii = 0; ii < nochan; ii++ )
   {
      QString pentry      = oprofs[ ii ];
      QString channel     = pentry.section( ":", 0, 0 ).simplified();
DbgLv(1) << "EGRn: rbR:   ii" << ii << "chan" << channel << "pentry" << pentry;

      if ( ! pentry.contains( uvvis ) )  continue;

      int spx             = -1;
      for ( int jj = 0; jj < nrange_sv; jj++ )
      {
         if ( chrngs_sv[ jj ].channel == channel )
         {  // Match in old protocol:  save its index
            spx                 = jj;
            break;
         }
      }

DbgLv(1) << "EGRn: rbR:     spx" << spx;
      if ( spx < 0 )
      {  // No such channel in old protocol:  use basic entry
         US_RunProtocol::RunProtoRanges::Ranges  chrng;
         chrng.channel      = channel;
         chrng.lo_rad       = 5.8;
         chrng.hi_rad       = 7.2;
         chrng.wvlens << 280.0;
         rpRange->chrngs << chrng;
      }
      else
      {  // Match to old protocol:  use that entry
         rpRange->chrngs << chrngs_sv[ spx ];
      }
   }

   rpRange->nranges    = rpRange->chrngs.count();
   nrnchan             = rpRange->nranges;
   if ( swvlens.count() < nrnchan )
      swvlens.resize( nrnchan );
DbgLv(1) << "EGRn: rbR:  nrnchan" << nrnchan << "nrnchan_sv" << nrnchan_sv;

   // Rebuild panel parameters
   if ( nrnchan_sv > 0 )
   {  // Use previous panel parameters
      for ( int ii = 0; ii < nrnchan; ii++ )
      {
         QString channel     = rpRange->chrngs[ ii ].channel;
         int ppx             = rchans.indexOf( channel );
DbgLv(1) << "EGRn: rbR:    ii" << ii << "channel" << channel << "ppx" << ppx;
         if ( ppx >= 0 )
         {
DbgLv(1) << "EGRn: rbR:     sizes: rch swv lor hir"
 << rchans.count() << swvlens.count() << locrads.count() << hicrads.count();
            rchans [ ii ]       = rchans [ ppx ];
            swvlens[ ii ]       = swvlens[ ppx ];
            locrads[ ii ]       = locrads[ ppx ];
            hicrads[ ii ]       = hicrads[ ppx ];
         }
         else
         {
            rchans [ ii ]       = channel;
            swvlens[ ii ]       = rpRange->chrngs[ ii ].wvlens;
            locrads[ ii ]       = rpRange->chrngs[ ii ].lo_rad;
            hicrads[ ii ]       = rpRange->chrngs[ ii ].hi_rad;
         }
      }
   }
   else
   {  // Create first shot at panel parameters
      for ( int ii = 0; ii < nrnchan; ii++ )
      {
	DbgLv(1) << "EGRn: rbR:    ii" << ii << "channel" << rpRange->chrngs[ii].channel;  //ALEXEY potential bug here # RANGES
         rchans [ ii ]       = rpRange->chrngs[ ii ].channel;
         swvlens[ ii ]       = rpRange->chrngs[ ii ].wvlens;
         locrads[ ii ]       = rpRange->chrngs[ ii ].lo_rad;
         hicrads[ ii ]       = rpRange->chrngs[ ii ].hi_rad;
      }
   }

   rchans .resize( nrnchan );
   swvlens.resize( nrnchan );
   locrads.resize( nrnchan );
   hicrads.resize( nrnchan );
}

#if 0
// Slot to manage extinction profiles
void US_ExperGuiRanges::manageEProfiles()
{
DbgLv(1) << "EGRn: mEP: IN";
   QObject* sobj       = sender();      // Sender object
   QString sname       = sobj->objectName();
   chrow               = sname.section( ":", 0, 0 ).toInt();

   //US_Extinction ediag( "BUFFER", "some buffer", this );
   US_Extinction* ediag = new US_Extinction;
   ediag->setParent(   this, Qt::Window );
   ediag->setAttribute( Qt::WA_DeleteOnClose );

   connect( ediag,  SIGNAL( get_results(     QMap<double,double>& ) ),
            this,   SLOT  ( process_results( QMap<double,double>& ) ) );

   ediag->show();
}

// Slot to handle specifications from a US_Extinction dialog
void US_ExperGuiRanges::process_results( QMap< double, double >& eprof )
{
DbgLv(1) << "EGRn: pr: eprof size" << eprof.keys().count() << "chrow" << chrow;
   pwvlens[ chrow ] = eprof.keys();
   pvalues[ chrow ].clear();

   for ( int ii = 0; ii < pwvlens[ chrow ].count(); ii++ )
   {
      pvalues[ chrow ] << eprof[ pwvlens[ chrow ][ ii ] ];
   }
}
#endif

// Slot to show details of all wavelength and radius ranges
void US_ExperGuiRanges::detailRanges()
{
   // Create a new editor text dialog with fixed font
   US_Editor* ediag = new US_Editor( US_Editor::DEFAULT, true, "", this );
   ediag->setWindowTitle( tr( "Details:  Channel Wavelength and Radius Ranges" ) );
   ediag->resize( 720, 440 );
   ediag->e->setFont( QFont( US_Widgets::fixedFont().family(),
                             US_GuiSettings::fontSize() - 1,
                             QFont::Bold ) );
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Compose the text that it displays
   QString dtext  = tr( "Wavelength, Radius Range Information:\n\n" );
   dtext += tr( "Number of Panel Channels Used:     %1\n" )
            .arg( nrnchan );
   dtext += tr( "Number of Protocol Channels Used:  %1\n" )
            .arg( rpRange->nranges );

   if ( nrnchan != rpRange->nranges )
   {  // Should not occur!
      dtext += tr( "\n*ERROR* Channel counts should be identical\n" );
      nrnchan   = 0;    // Skip channel details; too risky
   }

   for ( int ii = 0; ii < nrnchan; ii++ )
   {  // Show information for each channel
      dtext += tr( "\n  Channel " ) + rchans[ ii ] + " : \n";
      int nswavl = swvlens[ ii ].count();

      dtext += tr( "    Selected Wavelength count   : %1\n" )
               .arg( nswavl );
      if ( nswavl > 0 )
         dtext += tr( "    Selected Wavelength range   : %1 to %2\n" )
                  .arg( swvlens[ ii ][ 0 ] ).arg( swvlens[ ii ][ nswavl - 1 ] );
      dtext += tr( "    Selected Wavelengths        : " );

      int counter = 0;
      for (int  jj =0; jj < swvlens[ ii ].size(); jj++)
	{
	  ++counter;
	  dtext += tr("%1").arg(swvlens[ii][jj]);
	  if( jj != swvlens[ ii ].size() -1 )
	    dtext += tr(", ");
	  if(counter % 8 == 0)
	    dtext += tr("\n                                  ");
	}
      dtext += tr("\n");
      
      dtext += tr( "    Radius range                : %1 to %2\n" )
               .arg( locrads[ ii ] ).arg( hicrads[ ii ] );
   }

   // Load text and show the dialog
   QApplication::restoreOverrideCursor();
   qApp->processEvents();

   ediag->e->setText( dtext );
   ediag->show();
}

// Slot to select wavelengths using a dialog
void US_ExperGuiRanges::Wavelengths_class()
{
    QMessageBox msgBox;
    msgBox.setText(tr("How do you want to specify wavelengths?"));
    msgBox.setInformativeText("Wavelength(s) can be specified manually, or with the use of wavelength selector");
    msgBox.setWindowTitle(tr("Select Wavelength(s)"));
    QPushButton *Manual    = msgBox.addButton(tr("Manually"),   QMessageBox::YesRole);
    QPushButton *Selector  = msgBox.addButton(tr("Use Selector"), QMessageBox::YesRole);
    QPushButton *Cancel    = msgBox.addButton(tr("Cancel"), QMessageBox::RejectRole);
    
    msgBox.setIcon(QMessageBox::Question);
    msgBox.exec();
    
    if (msgBox.clickedButton() == Manual) {
      selectWavelengths_manual();
    } 
    else if (msgBox.clickedButton() == Selector) {
      selectWavelengths();
    }
    else if (msgBox.clickedButton() == Cancel){
      return;
    }

}

// Slot to select wavelengths using a dialog
void US_ExperGuiRanges::selectWavelengths_manual()
{
   QObject* sobj       = sender();      // Sender object
   QString sname       = sobj->objectName();
   chrow               = sname.section( ":", 0, 0 ).toInt();
   QString cclabl      = cc_labls[ chrow ]->text();
   QStringList wlpoten;                 // Potential wavelength list
   QStringList wlselec;                 // Selected wavelength list

   int nswavl          = swvlens[ chrow ].count();

   // Build the list of currently selected wavelengths, this row
   for ( int jj = 0; jj < nswavl; jj++ )
      wlselec << QString::number( swvlens[ chrow ][ jj ] );

   // Build the list of potential wavelengths, using all wavelengths
   //  selected on any rows other than this one
   for ( int ii = 0; ii < nrnchan; ii++ )
   {  // Show information for each channel
      int kswavl          = swvlens[ ii ].count();
      for ( int jj = 0; jj < kswavl; jj++ )
      {
         QString swavl       = QString::number( swvlens[ ii ][ jj ] );
         if ( ! wlpoten.contains( swavl ) )
            wlpoten << swavl;
      }
   }
   wlpoten.sort();

   // Open a wavelengths_MANUAL choice dialog and use to update selected wavelengths

   US_SelectWavelengths_manual* swdiag = new US_SelectWavelengths_manual( wlpoten, wlselec );

   swdiag->exec();

   swvlens[ chrow ].clear();
   int kswavl          = wlselec.count();
   int lswx            = qMax( 0, kswavl - 1 );
   QString labwlr;

   for ( int ii = 0; ii < kswavl; ii++ )
      swvlens[ chrow ] << wlselec[ ii ].toDouble();

   if ( kswavl == 0 )
      labwlr              = tr( "0 selected" );
   else if ( kswavl == 1 )
      labwlr              = "1,  " + wlselec[ 0 ];
   else
      labwlr              = QString::number( kswavl ) + ",  " + wlselec[ 0 ]
                            + tr( " to " ) + wlselec[ lswx ];

   cc_lrngs[ chrow ]->setText( labwlr );

   //ALEXEY
   // get a list of same-cell rows; disconnect
   QString clabl       = cc_labls[ chrow ]->text();
   QString scell       = clabl.left( 1 );
   QString labnone     = tr( "none" );
   QList< int >  ccrows;

   for ( int ii = 0; ii < mxrow; ii++ )
   {
      // Ignore the exact same row
      if ( ii == chrow )
         continue;
      // Get row label and quit loop when at end visible rows
      QString rlabl       = cc_labls[ ii ]->text();
      if ( rlabl == labnone )
         break;
      // Compare the cell value to that of the one (un)checked
      QString rcell       = rlabl.left( 1 );
      if ( rcell == scell )
      {  // Save same-cell row and disconnect the checkbox
         ccrows << ii;
      }
   }
DbgLv(1) << "EGRan: ranrows: ccrows" << ccrows;

   // Set check-state of Interference boxes in same-cell rows and reconnect
   for ( int ii = 0; ii < ccrows.count(); ii++ )
   {
      int ccrow           = ccrows[ ii ];
      cc_lrngs[ ccrow ]->setText( labwlr );

      swvlens[ ccrow ].clear();
      for ( int ii = 0; ii < kswavl; ii++ )
	swvlens[ ccrow ] << wlselec[ ii ].toDouble();
   }

   // Update ScanCount info per stage, per wavelength
   int tot_wvl = 0;
   for ( int ii = 0; ii < nrnchan; ii++ )
     tot_wvl += swvlens[ ii ].size();

   tot_wvl /= 2; // per all cells, not channels (for now)
   
   cb_scancount->clear();
   int nsp = sibIValue( "speeds",  "nspeeds" );
   for ( int i = 0; i < nsp; i++ )
     {
       double duration_sec    = rpSpeed->ssteps[ i ].duration;
       double scanint_sec     = rpSpeed->ssteps[ i ].scanintv;
       double scanint_sec_min = rpSpeed->ssteps[ i ].scanintv_min;

       int scancount = 0;
       if ( scanint_sec > scanint_sec_min*tot_wvl )
	 scancount = int( duration_sec / scanint_sec );
       else
	 scancount = int( duration_sec / (scanint_sec_min * tot_wvl) );
       
       QString scancount_stage = tr( "Stage %1. Number of Scans per Wavelength: %2 " ).arg(i+1).arg(scancount);
       cb_scancount->addItem( scancount_stage );
     }
   
}

// Slot to select wavelengths using a dialog
void US_ExperGuiRanges::selectWavelengths()
{
   QObject* sobj       = sender();      // Sender object
   QString sname       = sobj->objectName();
   chrow               = sname.section( ":", 0, 0 ).toInt();
   QString cclabl      = cc_labls[ chrow ]->text();
   QStringList wlpoten;                 // Potential wavelength list
   QStringList wlselec;                 // Selected wavelength list

   int nswavl          = swvlens[ chrow ].count();

#if 0
   if ( npwavl > nswavl )    // Potential list is the larger
   {  // Get potential wavelength list from profile wavelengths
      for ( int ii = 0; ii < npwavl; ii++ )
         wlpoten << QString::number( pwvlens[ chrow ][ ii ] );
   }

   else if ( nswavl != 0 )   // Selected list is the larger
   {  // Get potential wavelength list from (previously?) selected wavelengths
      for ( int ii = 0; ii < nswavl; ii++ )
         wlpoten << QString::number( swvlens[ chrow ][ ii ] );
   }

   else                      // Wavelength lists are empty
   {  // Default wavelength potential has a single value
      wlpoten << "280";
   }
#endif
#if 1
   // Build the list of currently selected wavelengths, this row
   for ( int jj = 0; jj < nswavl; jj++ )
      wlselec << QString::number( swvlens[ chrow ][ jj ] );

   // Build the list of potential wavelengths, using all wavelengths
   //  selected on any rows other than this one
   for ( int ii = 0; ii < nrnchan; ii++ )
   {  // Show information for each channel
      int kswavl          = swvlens[ ii ].count();
      for ( int jj = 0; jj < kswavl; jj++ )
      {
         QString swavl       = QString::number( swvlens[ ii ][ jj ] );
         if ( ! wlpoten.contains( swavl ) )
            wlpoten << swavl;
      }
   }
   wlpoten.sort();
#endif
DbgLv(1) << "EGRn: sW: wlpoten" << wlpoten;
DbgLv(1) << "EGRn: sW: wlselec" << wlselec;

   // Open a wavelengths choice dialog and use to update selected wavelengths

   US_SelectWavelengths* swdiag = new US_SelectWavelengths( wlpoten, wlselec );

   swdiag->exec();

   swvlens[ chrow ].clear();
   int kswavl          = wlselec.count();
   int lswx            = qMax( 0, kswavl - 1 );
   QString labwlr;

   for ( int ii = 0; ii < kswavl; ii++ )
      swvlens[ chrow ] << wlselec[ ii ].toDouble();

   if ( kswavl == 0 )
      labwlr              = tr( "0 selected" );
   else if ( kswavl == 1 )
      labwlr              = "1,  " + wlselec[ 0 ];
   else
      labwlr              = QString::number( kswavl ) + ",  " + wlselec[ 0 ]
                            + tr( " to " ) + wlselec[ lswx ];

   cc_lrngs[ chrow ]->setText( labwlr );
DbgLv(1) << "EGRn: sW: labwlr" << labwlr << "swvlens" << swvlens;
 
   //ALEXEY
   // get a list of same-cell rows; disconnect
   QString clabl       = cc_labls[ chrow ]->text();
   QString scell       = clabl.left( 1 );
   QString labnone     = tr( "none" );
   QList< int >  ccrows;

   for ( int ii = 0; ii < mxrow; ii++ )
   {
      // Ignore the exact same row
      if ( ii == chrow )
         continue;
      // Get row label and quit loop when at end visible rows
      QString rlabl       = cc_labls[ ii ]->text();
      if ( rlabl == labnone )
         break;
      // Compare the cell value to that of the one (un)checked
      QString rcell       = rlabl.left( 1 );
      if ( rcell == scell )
      {  // Save same-cell row and disconnect the checkbox
         ccrows << ii;
      }
   }
DbgLv(1) << "EGRan: ranrows: ccrows" << ccrows;

   // Set check-state of Interference boxes in same-cell rows and reconnect
   for ( int ii = 0; ii < ccrows.count(); ii++ )
   {
      int ccrow           = ccrows[ ii ];
      cc_lrngs[ ccrow ]->setText( labwlr );

      swvlens[ ccrow ].clear();
      for ( int ii = 0; ii < kswavl; ii++ )
	swvlens[ ccrow ] << wlselec[ ii ].toDouble();
   }

   // Update ScanCount info per stage, per wavelength
   int tot_wvl = 0;
   for ( int ii = 0; ii < nrnchan; ii++ )
     tot_wvl += swvlens[ ii ].size();

   tot_wvl /= 2; // per all cells, not channels (for now)
   
   cb_scancount->clear();
   int nsp = sibIValue( "speeds",  "nspeeds" );
   for ( int i = 0; i < nsp; i++ )
     {
       double duration_sec = rpSpeed->ssteps[ i ].duration;
       double scanint_sec  = rpSpeed->ssteps[ i ].scanintv;
       double scanint_sec_min = rpSpeed->ssteps[ i ].scanintv_min;

       int scancount = 0;
       if ( scanint_sec > scanint_sec_min*tot_wvl )
	 scancount = int( duration_sec / scanint_sec );
       else
	 scancount = int( duration_sec / (scanint_sec_min * tot_wvl) );
       
       QString scancount_stage = tr( "Stage %1. Number of Scans per Wavelength: %2 " ).arg(i+1).arg(scancount);
       cb_scancount->addItem( scancount_stage );
     }
   
 
}

#if 0
// Slot to handle the toggle of an "in Optima" checkbox
void US_ExperGuiRanges::checkOptima( bool checked )
{
   QObject* sobj       = sender();      // Sender object
   QString sname       = sobj->objectName();
   chrow               = sname.section( ":", 0, 0 ).toInt();
   QString cclabl      = cc_labls[ chrow ]->text();
DbgLv(1) << "EGRn:ckOpt: sname" << sname << "chrow" << chrow << cclabl
 << "checked" << checked;

DbgLv(1) << "EGRn:ckOpt:  OK";
}

// Slot to load a spectrum profile using us_extinction
void US_ExperGuiRanges::loadSpectrum()
{
   QObject* sobj       = sender();      // Sender object
   QString sname       = sobj->objectName();
   chrow               = sname.section( ":", 0, 0 ).toInt();
   QString cclabl      = cc_labls[ chrow ]->text();
DbgLv(1) << "EGRn:loadS: sname" << sname << "chrow" << chrow << cclabl;
   //US_Extinction ediag( "BUFFER", "some buffer", this );
   US_Extinction* ediag = new US_Extinction;
   ediag->setParent(   this, Qt::Window );
   ediag->setAttribute( Qt::WA_DeleteOnClose );

   connect( ediag,  SIGNAL( get_results(     QMap<double,double>& ) ),
            this,   SLOT  ( process_results( QMap<double,double>& ) ) );

   ediag->show();
}

// Slot to manually enter a spectrum profile using us_table
void US_ExperGuiRanges::manualSpectrum()
{
   QObject* sobj       = sender();      // Sender object
   QString sname       = sobj->objectName();
   chrow               = sname.section( ":", 0, 0 ).toInt();
   QString channel     = rchans[ chrow ];
DbgLv(1) << "EGRn:manSp: sname" << sname << "chrow" << chrow << channel;
   QMap< double, double >  extinction;

   for ( int ii = 0; ii < pwvlens[ chrow ].count(); ii++ )
   {
      double wavelen      = pwvlens[ chrow ][ ii ];
      double value        = pvalues[ chrow ][ ii ];
      extinction[ wavelen ]  = value;
DbgLv(1) << "EGRn:manSp:  ii" << ii << "w v" << wavelen << value;
   }

   QString strExtinc   = tr( "Extinction:" );
   bool changed        = false;
   US_Table* tdiag     = new US_Table( extinction, strExtinc, changed );
   tdiag->setWindowTitle( tr( "Manually Enter Spectrum (%1)" )
                          .arg( channel ) );
   tdiag->exec();

   if ( changed )
   {
      QList< double > ewavls = extinction.keys();
      int nwavl              = ewavls.count();
DbgLv(1) << "EGRn:manSP: extinc size" << nwavl << "chrow" << chrow;
      pwvlens[ chrow ] = ewavls;
      pvalues[ chrow ].clear();

      for ( int ii = 0; ii < nwavl; ii++ )
      {
         pvalues[ chrow ] << extinction[ ewavls[ ii ] ];
      }
DbgLv(1) << "EGRn:manSP:  0 : wavl valu" << pwvlens[chrow][0] << pvalues[chrow][0];
int nn=nwavl-1;
if (nwavl>0)
 DbgLv(1) << "EGRn:manSP:  n : wavl valu" << pwvlens[chrow][nn] << pvalues[chrow][nn];

      if ( swvlens[ chrow ].count() == 0 )
      {
         swvlens[ chrow ] = pwvlens[ chrow ];
      }
   }
else
DbgLv(1) << "EGRn:manSp: *NOT Accepted*";
}
#endif

// Slot to handle a change in the low radius value
void US_ExperGuiRanges::changedLowRadius( double val )
{
   QObject* sobj       = sender();      // Sender object
   QString sname       = sobj->objectName();
   chrow               = sname.section( ":", 0, 0 ).toInt();
DbgLv(1) << "chgLoRad: val" << val << "row" << chrow;
   locrads[ chrow ]    = val;

   //ALEXEY - simulate the same counters coupling for same cell as for the (Interference) checkboxes in Optics
   // get a list of same-cell rows; disconnect
   QString clabl       = cc_labls[ chrow ]->text();
   QString scell       = clabl.left( 1 );
   QString labnone     = tr( "none" );
   QList< int >  ccrows;

   for ( int ii = 0; ii < mxrow; ii++ )
   {
      // Ignore the exact same row
      if ( ii == chrow )
         continue;
      // Get row label and quit loop when at end visible rows
      QString rlabl       = cc_labls[ ii ]->text();
      if ( rlabl == labnone )
         break;
      // Compare the cell value to that of the one (un)checked
      QString rcell       = rlabl.left( 1 );
      if ( rcell == scell )
      {  // Save same-cell row and disconnect the checkbox
         ccrows << ii;
      }
   }
DbgLv(1) << "EGRan: ranrows: ccrows" << ccrows;

   // Set check-state of Interference boxes in same-cell rows and reconnect
   for ( int ii = 0; ii < ccrows.count(); ii++ )
   {
      int ccrow           = ccrows[ ii ];
      cc_lrads[ ccrow ]->setValue(val);
      locrads[ chrow ]    = val;
   }
}

// Slot to handle a change in the high radius value
void US_ExperGuiRanges::changedHighRadius( double val )
{
   QObject* sobj       = sender();      // Sender object
   QString sname       = sobj->objectName();
   chrow               = sname.section( ":", 0, 0 ).toInt();
DbgLv(1) << "chgHiRad: val" << val << "row" << chrow;
   hicrads[ chrow ]    = val;
   
   //ALEXEY
      //ALEXEY - simulate the same counters coupling for same cell as for the (Interference) checkboxes in Optics
   // get a list of same-cell rows; disconnect
   QString clabl       = cc_labls[ chrow ]->text();
   QString scell       = clabl.left( 1 );
   QString labnone     = tr( "none" );
   QList< int >  ccrows;

   for ( int ii = 0; ii < mxrow; ii++ )
   {
      // Ignore the exact same row
      if ( ii == chrow )
         continue;
      // Get row label and quit loop when at end visible rows
      QString rlabl       = cc_labls[ ii ]->text();
      if ( rlabl == labnone )
         break;
      // Compare the cell value to that of the one (un)checked
      QString rcell       = rlabl.left( 1 );
      if ( rcell == scell )
      {  // Save same-cell row and disconnect the checkbox
         ccrows << ii;
      }
   }
DbgLv(1) << "EGRan: ranrows: ccrows" << ccrows;

   // Set check-state of Interference boxes in same-cell rows and reconnect
   for ( int ii = 0; ii < ccrows.count(); ii++ )
   {
      int ccrow           = ccrows[ ii ];
      cc_hrads[ ccrow ]->setValue(val);
      hicrads[ chrow ]    = val;
   }
}


// Class dialog object for selecting wavelengths

US_SelectWavelengths::US_SelectWavelengths(
   QStringList& orig_wavls, QStringList& select_wavls )
   : US_WidgetsDialog( 0, 0 ), orig_wavls( orig_wavls ),
   select_wavls ( select_wavls )
{
   nbr_poten   = orig_wavls  .count();
   nbr_selec   = select_wavls.count();
DbgLv(1) << "SelWl: IN k_ori k_sel" << nbr_poten << nbr_selec;
   dbg_level   = US_Settings::us_debug();

   setWindowTitle( tr( "Wavelengths Selector" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* main    = new QVBoxLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
   QVBoxLayout* left    = new QVBoxLayout;
   QVBoxLayout* right   = new QVBoxLayout;
   QHBoxLayout* lists   = new QHBoxLayout;
   QHBoxLayout* buttons = new QHBoxLayout;
   QGridLayout* rngL    = new QGridLayout;
DbgLv(1) << "SelWl: IN k_ori k_sel" << nbr_poten << nbr_selec;

   // Read-only triple list count text
   le_original   = us_lineedit( tr( "%1 potential wavelengths" )
                      .arg( nbr_poten ), -1, true );
   le_selected   = us_lineedit( tr( "%1 selected wavelengths" )
                      .arg( nbr_selec ), -1, true );

   // Lambda list labels
   QLabel* lb_original = us_label( tr( "Potential Wavelengths" ) );
   QLabel* lb_selected = us_label( tr( "Selected Wavelengths" ) );

   // Lambda list widgets
   lw_original   = us_listwidget();
   lw_selected   = us_listwidget();
   lw_original->setSelectionMode( QAbstractItemView::ExtendedSelection );
   lw_selected->setSelectionMode( QAbstractItemView::ExtendedSelection );
   potential     = orig_wavls;
   selected      = select_wavls;
DbgLv(1) << "SelWl:    k_pot k_sel" << potential.count() << selected.count();

   for ( int ii = 0; ii < nbr_poten; ii++ )
   {  // Add items to the original (potential) list
      lw_original->addItem( potential[ ii ] );
   }
   for ( int ii = 0; ii < nbr_selec; ii++ )
   {  // Add items to the selected list
      lw_selected->addItem( selected[ ii ] );
   }

   // Add (=>) and Remove (<=) buttons for lists
   pb_add        = us_pushbutton( tr( "Add  ===>"    ) );
   pb_remove     = us_pushbutton( tr( "<===  Remove" ) );
   pb_addall     = us_pushbutton( tr( "Add All  ===>"    ) );
   pb_rmvall     = us_pushbutton( tr( "<===  Remove All" ) );

   // Button Row

   QPushButton* pb_reset  = us_pushbutton( tr( "Reset" ) );
   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
                pb_accept = us_pushbutton( tr( "Accept" ) );
   //pb_accept->setEnabled( nbr_selec > 0 );
   pb_add   ->setEnabled( nbr_poten > 0 );
   pb_remove->setEnabled( nbr_selec > 0 );
   pb_addall->setEnabled( nbr_poten > 0 );
   pb_rmvall->setEnabled( nbr_selec > 0 );

   // Range rows
   QLabel* bn_range       = us_banner( tr( "Potential Wavelength Range" ) );
   QLabel* lb_strwln      = us_label(  tr( "Start Wavelength" ) );
   QLabel* lb_endwln      = us_label(  tr( "End Wavelength" ) );
   QLabel* lb_incwln      = us_label(  tr( "Wavelength Increment" ) );
   ct_strwln              = us_counter( 2, 100, 800, 1 );
   ct_endwln              = us_counter( 2, 100, 800, 1 );
   ct_incwln              = us_counter( 2,   1,  20, 1 );
   const int def_swl      = 200;
   const int def_ewl      = 600;
   const int def_iwl      = 1;
   int lpx                = nbr_poten - 1;
   int ini_swl            = lpx >= 0 ? potential[   0 ].toInt() : def_swl;
   int ini_ewl            = lpx >= 0 ? potential[ lpx ].toInt() : def_ewl;
   ct_strwln ->setSingleStep(   1 );
   ct_endwln ->setSingleStep(   1 );
   ct_incwln ->setSingleStep(   1 );
   ct_strwln ->setValue     ( (double)ini_swl );
   ct_endwln ->setValue     ( (double)ini_ewl );
   ct_incwln ->setValue     ( (double)def_iwl );
   nbr_range     = ( ini_ewl - ini_swl ) / def_iwl + 1;
   int row       = 0;
   rngL->addWidget( bn_range,        row++, 0, 1, 6 );
   rngL->addWidget( lb_strwln,       row,   0, 1, 3 );
   rngL->addWidget( ct_strwln,       row++, 3, 1, 3 );
   rngL->addWidget( lb_endwln,       row,   0, 1, 3 );
   rngL->addWidget( ct_endwln,       row++, 3, 1, 3 );
   rngL->addWidget( lb_incwln,       row,   0, 1, 3 );
   rngL->addWidget( ct_incwln,       row++, 3, 1, 3 );

  // Connections
   connect( pb_add,    SIGNAL( clicked() ), SLOT( add_selections()  ) );
   connect( pb_remove, SIGNAL( clicked() ), SLOT( rmv_selections()  ) );
   connect( pb_addall, SIGNAL( clicked() ), SLOT( add_all_selects() ) );
   connect( pb_rmvall, SIGNAL( clicked() ), SLOT( rmv_all_selects() ) );
   connect( pb_reset,  SIGNAL( clicked() ), SLOT( reset()  ) );
   connect( pb_help,   SIGNAL( clicked() ), SLOT( help()   ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( done()   ) );
   connect( ct_strwln, SIGNAL( valueChanged( double ) ),
            this,      SLOT  ( new_wl_start( double ) ) );
   connect( ct_endwln, SIGNAL( valueChanged( double ) ),
            this,      SLOT  ( new_wl_end  ( double ) ) );
   connect( ct_incwln, SIGNAL( valueChanged( double ) ),
            this,      SLOT  ( new_wl_incr ( double ) ) );

   // Complete layouts
   buttons->addWidget( pb_reset  );
   buttons->addWidget( pb_help   );
   buttons->addWidget( pb_cancel );
   buttons->addWidget( pb_accept );

   left ->addWidget( lb_original );
   left ->addWidget( lw_original );
   left ->addWidget( pb_add      );
   left ->addWidget( pb_addall   );
   right->addWidget( lb_selected );
   right->addWidget( lw_selected );
   right->addWidget( pb_remove   );
   right->addWidget( pb_rmvall   );
   lists->addLayout( left   );
   lists->addLayout( right  );
   main ->addWidget( le_original );
   main ->addWidget( le_selected );
   main ->addLayout( lists   );
   main ->addLayout( rngL    );
   main ->addLayout( buttons );

DbgLv(1) << "SelWl: layout complete";
   resize( 150, 700 );
}


// Class dialog object for selecting wavelengths manually

US_SelectWavelengths_manual::US_SelectWavelengths_manual(
   QStringList& orig_wavls, QStringList& select_wavls )
   : US_WidgetsDialog( 0, 0 ), orig_wavls( orig_wavls ),
   select_wavls ( select_wavls )
{
   nbr_poten   = orig_wavls  .count();
   nbr_selec   = select_wavls.count();
DbgLv(1) << "SelWl: IN k_ori k_sel" << nbr_poten << nbr_selec;
   dbg_level   = US_Settings::us_debug();

   setWindowTitle( tr( "Wavelengths Selector" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* main    = new QVBoxLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   QGridLayout* genL   = new QGridLayout();

   QHBoxLayout* buttons = new QHBoxLayout;
   le_info = us_textedit();
   le_info->setText(tr( "NOTES ON SYNTAX: \n The following wavelength(s) specification examples are accepted: \n   * 230-550:N (take every Nth wavelength, starting with 230 nm and go up to 550 nm);  \n   * 220,230,250 (measure at 220 nm, 230 nm, and at 250 nm); \n   * 220-230 (measure at 220, 221, 222, 223... in 1 nm increments when ':N' is not specified); \n   * 230-550:2,560,570 (like above, but also measure 560 and 570 nm) " ));

   // Range rows
   QLabel* bn_range       = us_banner( tr( "Enter Wavelength(s)" ) );
   QLabel* lb_wrange      = us_label(  tr( "Wavelength Range" ) );
   le_wrange              = us_lineedit();
   le_wrange->setPlaceholderText("Enter wavelength specification");

   // Button Row
   QPushButton* pb_reset  = us_pushbutton( tr( "Reset" ) );
   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   pb_accept              = us_pushbutton( tr( "Accept" ) );

   pb_accept->setEnabled(false);

   // Complete layouts
   buttons->addWidget( pb_reset  );
   buttons->addWidget( pb_help   );
   buttons->addWidget( pb_cancel );
   buttons->addWidget( pb_accept );
   
   int row     = 0;
   genL->addWidget( le_info,   row++,   0, 1, 8 );
   genL->addWidget( bn_range,  row++,   0, 1, 8 );
   genL->addWidget( lb_wrange, row,     0, 1, 3 );
   genL->addWidget( le_wrange, row++,   3, 1, 5 );
   genL->addLayout( buttons,   row,     0, 1, 8 );
   
  // Connections
   connect( pb_reset,  SIGNAL( clicked() ), SLOT( reset()  ) );
   connect( pb_help,   SIGNAL( clicked() ), SLOT( help()   ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( done()   ) );

   connect( le_wrange, SIGNAL( textChanged(QString) ),
            this,      SLOT  ( wln_changed(QString) ) );
   // connect( le_wrange, SIGNAL( editingFinished() ),
   //          this,      SLOT  ( wln_entered() ) );

   main ->addLayout( genL );
   resize( 650, 300 );
}

// Reset the lists and buttons to their original state
void US_SelectWavelengths_manual::reset( void )
{
  le_wrange->setText("");
  pb_accept->setEnabled( false );
  QPalette *palette = new QPalette();
  palette->setColor(QPalette::Text,Qt::black);
  palette->setColor(QPalette::Base,Qt::white);
  le_wrange->setPalette(*palette);
}

// Cancel button clicked:  returned delete-selections is empty
void US_SelectWavelengths_manual::cancel( void )
{
   reset();

   reject();
   close();
}

// Entered wavelength(s):  check for syntax
void US_SelectWavelengths_manual::wln_changed( QString text )
{
  QPalette *palette = new QPalette();
  palette->setColor(QPalette::Text,Qt::black);
  palette->setColor(QPalette::Base,Qt::white);
  le_wrange->setPalette(*palette);
  le_wrange->setText(text);

  pb_accept->setEnabled( true );
}

// Entered wavelength(s):  check for syntax
bool US_SelectWavelengths_manual::wln_entered( void )
{
  // CHECK for syntax
  QString text = le_wrange->text();

  QRegExp rx1("[(\\d{3}),]+");
  QRegExp rx2("[(\\d{3}),]*(\\d{3})-(\\d{3})[,(\\d{3})]*");
  QRegExp rx3("[(\\d{3}),]*(\\d{3})-(\\d{3}):(\\d+)[,(\\d{3})]*");
  QRegExp rx4("[(\\d{3}),]*(\\d{3})-(\\d{3})[,(\\d{3})]*(\\d{3})-(\\d{3})[,(\\d{3})]*");
  QRegExp rx5("[(\\d{3}),]*(\\d{3})-(\\d{3}):(\\d+)[,(\\d{3})]*(\\d{3})-(\\d{3}):(\\d+)[,(\\d{3})]*");
  QRegExp rx6("[(\\d{3}),]*(\\d{3})-(\\d{3}):(\\d+)[,(\\d{3})]*(\\d{3})-(\\d{3})[,(\\d{3})]*");
  QRegExp rx7("[(\\d{3}),]*(\\d{3})-(\\d{3})[,(\\d{3})]*(\\d{3})-(\\d{3}):(\\d+)[,(\\d{3})]*");

  //QRegExp rx_new("[\\d{3},]*[\\d{3}-\\d{3}]*[(:\\d+)]*[,(\\d{3})]*"); //working partially
  
  if ( rx1.exactMatch(text)  ||  rx2.exactMatch(text) || rx3.exactMatch(text) 
       || rx4.exactMatch(text) || rx5.exactMatch(text) || rx6.exactMatch(text) ||  rx7.exactMatch(text) ){
    //pb_accept->setEnabled( true );

    if ( text_to_numbers() )
      return true;
    else
      {
	QPalette *palette = new QPalette();
	palette->setColor(QPalette::Text,Qt::red);
	palette->setColor(QPalette::Base,Qt::white);
	le_wrange->setPalette(*palette);
	return false;
      }
  }    
  else {
    QString mtitle_error    = tr( "Error" );
    QString message_error   = tr( "Syntax error!" );
    QMessageBox::critical( this, mtitle_error, message_error );
    
    QPalette *palette = new QPalette();
    palette->setColor(QPalette::Text,Qt::red);
    palette->setColor(QPalette::Base,Qt::white);
    le_wrange->setPalette(*palette);

    return false;
  }
}

// Transform text to numbers
bool US_SelectWavelengths_manual::text_to_numbers( void )
{
  // TRANSFORM numbers to arrays
  QString  waveln_raw  = le_wrange->text();
  DbgLv(1) << "WAVELENGTHS: " << waveln_raw;  
  
  if ( waveln_raw.contains(",") ){
    
    QStringList wvllist = waveln_raw.split( "," );
    for (QStringList::iterator it = wvllist.begin(); it != wvllist.end(); ++it){            
      QString current = *it; 
      
      if ( current == "" )
	continue;
      
      if ( current.contains(":") ){
	QStringList wvllist_semicolon      = current.split( ":" );
	QStringList wvllist_semicolon_dash = wvllist_semicolon[0].split( "-" );
	
	int wvl_iter = wvllist_semicolon[1].toInt();
	int wvl_min  = wvllist_semicolon_dash[0].toInt();
	int wvl_max  = wvllist_semicolon_dash[1].toInt();

	if ( wvl_min > wvl_max )
	  {
	    selected.clear();
	    return false;
	  }
	
	for (int i = wvl_min; i <= wvl_max; i += wvl_iter){
	  selected << QString::number(i);
	}
      }
      else if ( current.contains("-") && !current.contains(":") ){
	QStringList wvllist_dash = current.split( "-" );
	int wvl_min_dash  = wvllist_dash[0].toInt();
	int wvl_max_dash  = wvllist_dash[1].toInt();

	if ( wvl_min_dash > wvl_max_dash )
	  {
	    selected.clear();
	    return false;
	  }
	
	for (int i = wvl_min_dash; i <= wvl_max_dash; i++){
	  selected << QString::number(i);
	}
      }
      else {
	selected << current;
      }
    }
  }
  else if ( waveln_raw.contains(":") && !waveln_raw.contains(",") ){
    
    QStringList wvllist_semicolon_nocoma      = waveln_raw.split( ":" );
    QStringList wvllist_semicolon_nocoma_dash = wvllist_semicolon_nocoma[0].split( "-" );
    
    int wvl_iter = wvllist_semicolon_nocoma[1].toInt(); 
    int wvl_min  = wvllist_semicolon_nocoma_dash[0].toInt();
    int wvl_max  = wvllist_semicolon_nocoma_dash[1].toInt();

    if ( wvl_min > wvl_max )
      {
	selected.clear();
	return false;
      }
    
    for (int i = wvl_min; i <= wvl_max; i += wvl_iter){
      selected << QString::number(i);
    }
  }
  else if ( waveln_raw.contains("-") && !waveln_raw.contains(":") ){
    QStringList wvllist_nocoma_dash = waveln_raw.split( "-" );
    int wvl_min_nocoma_dash  = wvllist_nocoma_dash[0].toInt();
    int wvl_max_nocoma_dash  = wvllist_nocoma_dash[1].toInt();

    if ( wvl_min_nocoma_dash > wvl_max_nocoma_dash )
      {
	selected.clear();
	return false;
      }
	
    for (int i = wvl_min_nocoma_dash; i <= wvl_max_nocoma_dash; i++){
      selected << QString::number(i);
    }
  }
  else if ( !waveln_raw.contains("-") && !waveln_raw.contains(":") && !waveln_raw.contains(".") ){
    selected << waveln_raw;
  }
  
  selected.removeDuplicates();
  selected.sort();

  qDebug() << "First and Last element: " << selected[0].toDouble() << ", " << selected[ selected.size() -1 ].toDouble();
  
  if ( selected[0].toDouble() < 100.0 || selected[ selected.size() -1 ].toDouble() > 800.0  )   // Boundaries [100-800] nm
    {
      QPalette *palette = new QPalette();
      palette->setColor(QPalette::Text,Qt::red);
      palette->setColor(QPalette::Base,Qt::white);
      le_wrange->setPalette(*palette);
      
      QString mtitle_error    = tr( "Error" );
      QString message_error   = tr( "Selected wavelengths are outside of accepted range of [100-800] nm!" );
      QMessageBox::critical( this, mtitle_error, message_error );
      
      selected.clear();
      return false;
    }
  
  if ( selected.size() > 100 )
    {
      QPalette *palette = new QPalette();
      palette->setColor(QPalette::Text,Qt::red);
      palette->setColor(QPalette::Base,Qt::white);
      le_wrange->setPalette(*palette);
      
      QString mtitle_error    = tr( "Error" );
      QString message_error   = tr( "Number of wavelengths is %1. <br> The number of selected wavelengths is more than 100! <br> Make wavelength count less than or equal to 100." );
      QMessageBox::critical( this, mtitle_error, message_error.arg(selected.size()) );

      selected.clear();
      return false;
    }
  
  DbgLv(1) << "WVL_ARRAY " << selected;  
  //selected << waveln;
  select_wavls = selected;

  return true;
  
}


// Accept button clicked:  returned delete-selections list is the excluded list
void US_SelectWavelengths_manual::done( void )
{
  if ( wln_entered() )
    {
      accept();
      close();
    }
}


// Slot to add selections to the excluded list
void US_SelectWavelengths::add_selections()
{
DbgLv(0) << "AddSelections";
   // Get the list of selected items
   QList< QListWidgetItem* > selitems = lw_original->selectedItems();
   int kntsel   = selitems.count();

   for ( int ii = 0; ii < kntsel; ii++ )
   {  // Move selected items from original to selected
      QListWidgetItem* l_item   = selitems.at( ii );
      QString          waveln   = l_item->text();
      lw_original->setCurrentItem  ( l_item, QItemSelectionModel::Deselect );
      lw_original->setCurrentItem  ( l_item, QItemSelectionModel::Deselect );
      if ( ! selected.contains( waveln ) )
         selected << waveln;            // Add to selected (if not there yet)
      potential.removeOne( waveln );    // Remove from original
   }

   selected.sort();                     // Sort new selected list
   lw_original->clear();                // Clear list widgets
   lw_selected->clear();
   nbr_poten    = potential.count();
   nbr_selec    = selected.count();

   for ( int ii = 0; ii < nbr_poten; ii++ )     // Repopulate potential widget
      lw_original->addItem( potential[ ii ] );

   for ( int ii = 0; ii < nbr_selec; ii++ )     // Repopulate selected widget
      lw_selected->addItem( selected[ ii ] );

   report();                            // Report counts, enable buttons
}

// Slot to remove items from the selected list
void US_SelectWavelengths::rmv_selections()
{
DbgLv(0) << "RemoveSelections";
   // Get the list of selected items
   QList< QListWidgetItem* > selitems = lw_selected->selectedItems();
   int kntsel   = selitems.count();

   // Make a list of wavelengths in the currently defined range
   QStringList rng_wavls;
   int rws      = (int)ct_strwln->value();
   int rwe      = (int)ct_endwln->value() + 1;
   int rwi      = (int)ct_incwln->value();
   for ( int rwvl = rws; rwvl < rwe; rwvl += rwi )
   {
      rng_wavls << QString::number( rwvl );
   }

   // Move or remove selected
   for ( int ii = 0; ii < kntsel; ii++ )
   {  // Move selected items from selected to potential (if in range)
      QListWidgetItem* l_item   = selitems.at( ii );
      QString          waveln   = l_item->text();
      lw_selected->setCurrentItem  ( l_item, QItemSelectionModel::Deselect );
      if ( rng_wavls.contains( waveln )  &&  ! potential.contains( waveln ) )
         potential << waveln;           // Add to potential (if in range)
      selected.removeOne( waveln );     // Remove from selected
   }

   potential   .sort();                 // Sort new potential list
   lw_original->clear();                // Clear list widgets
   lw_selected->clear();
   nbr_poten    = potential.count();
   nbr_selec    = selected.count();

   for ( int ii = 0; ii < nbr_poten; ii++ )     // Repopulate potential widget
      lw_original->addItem( potential[ ii ] );

   for ( int ii = 0; ii < nbr_selec; ii++ )     // Repopulate selected widget
      lw_selected->addItem( selected[ ii ] );

   // Report the new state of things
   report();                            // Report counts, enable buttons
}

// Slot to add all potential to the selected list
void US_SelectWavelengths::add_all_selects()
{
DbgLv(0) << "AddAllSelections";
   for ( int ii = 0; ii < nbr_poten; ii++ )
   {
      if ( ! selected.contains( potential[ ii ] ) )
         selected << potential[ ii ];   // Add to selected
   }

   selected.sort();                     // Sort new selected list
   lw_original->clear();                // Clear list widgets
   lw_selected->clear();
   potential   .clear();
   nbr_poten    = 0;
   nbr_selec    = selected.count();

   for ( int ii = 0; ii < nbr_selec; ii++ )     // Repopulate selected widget
      lw_selected->addItem( selected[ ii ] );

   report();                            // Report counts, enable buttons
}

// Slot to remove all items from the selected list
void US_SelectWavelengths::rmv_all_selects()
{
DbgLv(0) << "RemoveAllSelections";
   // Make a list of wavelengths in the currently defined range
   QStringList rng_wavls;
   int rws      = (int)ct_strwln->value();
   int rwe      = (int)ct_endwln->value() + 1;
   int rwi      = (int)ct_incwln->value();

   for ( int rwvl = rws; rwvl < rwe; rwvl += rwi )
      rng_wavls << QString::number( rwvl );

   // Move selected to potential 
   for ( int ii = 0; ii < nbr_selec; ii++ )
   {
      // Move to potential list if in range and not already there
      if ( rng_wavls.contains( selected[ ii ] )  &&
           ! potential.contains( selected[ ii ] ) )
         potential << selected[ ii ];
   }

   potential   .sort();                 // Sort new potential list
   lw_original->clear();                // Clear list widgets
   lw_selected->clear();
   selected    .clear();
   nbr_poten    = potential.count();
   nbr_selec    = 0;

   for ( int ii = 0; ii < nbr_poten; ii++ )     // Repopulate potential widget
      lw_original->addItem( potential[ ii ] );

   report();                            // Report counts, enable buttons
}

// Report counts and enable/disable buttons after changes
void US_SelectWavelengths::report( void )
{
   // Report the new state of things
   le_original->setText( ( nbr_poten == 1 ) ?
                         tr( "1 potential wavelength" ) :
                         tr( "%1 potential wavelengths" ).arg( nbr_poten )
                       + tr( " (of %1 in range)" ).arg( nbr_range ) );
   le_selected->setText( ( nbr_selec == 1 ) ?
                         tr( "1 selected wavelength" ) :
                         tr( "%1 selected wavelengths" ).arg( nbr_selec ) );

   // Enable/disable buttons appropriately
//   pb_accept->setEnabled( nbr_selec > 0 );
   pb_add   ->setEnabled( nbr_poten > 0 );
   pb_remove->setEnabled( nbr_selec > 0 );
   pb_addall->setEnabled( nbr_poten > 0 );
   pb_rmvall->setEnabled( nbr_selec > 0 );
}

// Reset the lists and buttons to their original state
void US_SelectWavelengths::reset( void )
{
   lw_original->clear();
   lw_selected->clear();
   potential     = orig_wavls;
   selected      = select_wavls;
   nbr_poten     = potential.count();
   nbr_selec     = selected .count();

   for ( int ii = 0; ii < nbr_poten; ii++ )
      lw_original->addItem( potential[ ii ] );

   for ( int ii = 0; ii < nbr_selec; ii++ )
      lw_selected->addItem( selected [ ii ] );

   report();                            // Report counts, enable buttons
}

// Cancel button clicked:  returned delete-selections is empty
void US_SelectWavelengths::cancel( void )
{
//   select_wavls.clear();
   reset();

   reject();
   close();
}

// Accept button clicked:  returned delete-selections list is the excluded list
void US_SelectWavelengths::done( void )
{
  if ( selected.size() > 100 )
    {
      QString mtitle_error    = tr( "Error" );
      QString message_error   = tr( "Number of wavelengths is %1. <br> The number of selected wavelengths is more than 100! <br> Make wavelength count less than or equal to 100." );
      QMessageBox::critical( this, mtitle_error, message_error.arg(selected.size()) );
      
      //selected.clear();

      return;
    }
  
  select_wavls = selected;
  DbgLv(1) << "SelWl: done: nbr_sel" << select_wavls.count();

  accept();
  close();
}

// Slot for change in wavelength range start
void US_SelectWavelengths::new_wl_start( double val )
{
DbgLv(1) << "SelWl: neww_str: " << val;
   new_wl_range( (int)val,
                 (int)ct_endwln->value(),
                 (int)ct_incwln->value() );
}

// Slot for change in wavelength range end
void US_SelectWavelengths::new_wl_end( double val )
{
DbgLv(1) << "SelWl: neww_end: " << val;
   new_wl_range( (int)ct_strwln->value(),
                 (int)val,
                 (int)ct_incwln->value() );
}

// Slot for change in wavelength range increment
void US_SelectWavelengths::new_wl_incr( double val )
{
DbgLv(1) << "SelWl: neww_inc: " << val;
   new_wl_range( (int)ct_strwln->value(),
                 (int)ct_endwln->value(),
                 (int)val );
}

// Function for change in potential wavelength range
void US_SelectWavelengths::new_wl_range( const int wls, const int wle,
                                         const int wli )
{
   // Save a copy of the current potential wavelength list
DbgLv(1) << "SelWl: newwr: s,e,i" << wls << wle << wli;
   QStringList sv_poten = potential;
   nbr_poten            = potential.count();
   nbr_range            = ( wle - wls ) / wli + 1;
   int lpx              = nbr_poten - 1;
   potential.clear();
   int p_strwl          = lpx > 0 ? sv_poten[   0 ].toInt() : 9999;
   int p_endwl          = lpx > 0 ? sv_poten[ lpx ].toInt() :    0;
DbgLv(1) << "SelWl: newwr:  p_strwl p_endwl" << p_strwl << p_endwl;

   // Examine the expanded or contracted range of wavelengths
   for ( int iwvl = wls; iwvl <= wle; iwvl += wli )
   {
      QString wavel        = QString::number( iwvl );

      // Consider adding to potential list if not yet in either list
      if ( !potential.contains( wavel )  &&
           !selected .contains( wavel ) )
      {
         // Outside current potential range: add to new list
         if ( iwvl < p_strwl  ||  iwvl > p_endwl )
            potential << wavel;

         // In range and in current potential list: keep in the list
         else if ( sv_poten.contains( wavel ) )
            potential << wavel;
      }
   }

   // Now rebuild widget lists
   lw_original->clear();
   lw_original->addItems( potential );

   nbr_poten            = potential.count();   // Update count

   report();                                   // Report counts, enable buttons
DbgLv(1) << "SelWl: newwr: @rtn: nbr_poten nbr_selec" << nbr_poten << nbr_selec;
}

