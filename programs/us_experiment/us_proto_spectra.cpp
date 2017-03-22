//! \file us_proto_spectra.cpp

#include "us_experiment_main.h"
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

// Panel for Spectra parameters
US_ExperGuiSpectra::US_ExperGuiSpectra( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ExperimentMain*)topw;
   rpSpect             = &(mainw->currProto.rpSpect);
   mxrow               = 24;     // Maximum possible rows
   nspchan             = 0;
   protname            = mainw->currProto.protname;
   chrow               = -1;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "7: Specify wavelength/value spectra" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();

   QPushButton* pb_details  = us_pushbutton( tr( "View Current Spectra Settings" ) );
   connect( pb_details,   SIGNAL( clicked()       ),
            this,         SLOT  ( detailSpectra() ) );

   QLabel* lb_hdr1     = us_banner( tr( "Cell / Channel" ) );
   QLabel* lb_hdr2     = us_banner( tr( "Wavelengths" ) );
   QLabel* lb_hdr3     = us_banner( tr( "Spectra Input" ) );
   QLabel* lb_hdr4     = us_banner( tr( "All Specified?" ) );

   int row             = 0;
   genL->addWidget( pb_details,      row++, 3, 1, 3 );
   genL->addWidget( lb_hdr1,         row,   0, 1, 1 );
   genL->addWidget( lb_hdr2,         row,   1, 1, 1 );
   genL->addWidget( lb_hdr3,         row,   2, 1, 3 );
   genL->addWidget( lb_hdr4,         row++, 5, 1, 1 );

   QLabel*      cclabl;
   QPushButton* pbwavln;
   QPushButton* pbsload;
   QPushButton* pbsmanu;
   QCheckBox*   ckoptim;
   QCheckBox*   cksdone;
   QString swavln   = tr( "Select Wavelengths" );
   QString soptim   = tr( "Auto in Optima" );
   QString sloads   = tr( "Load Spectrum" );
   QString smanus   = tr( "Manual Spectrum" );
   QString ssdone   = tr( "Done" );
   QFont   ckfont   = QFont( US_GuiSettings::fontFamily(),
                             US_GuiSettings::fontSize  (),
                             QFont::Bold );
   QPalette ckpal   = US_GuiSettings::normalColor();

   for ( int ii = 0; ii < mxrow; ii++ )
   {  // Loop to build initial place-holder spectr rows
      QString scel;
      if      ( ii == 0 ) scel = QString( "1 / A" );
      else if ( ii == 1 ) scel = QString( "1 / B" );
      else if ( ii == 2 ) scel = QString( "2 / A" );
      else if ( ii == 3 ) scel = QString( "2 / B" );
      else                scel = QString( "none" );
      cclabl           = us_label( scel );
      pbwavln          = us_pushbutton( swavln );
      ckoptim          = new QCheckBox( soptim, this );
      pbsload          = us_pushbutton( sloads );
      pbsmanu          = us_pushbutton( smanus );
      cksdone          = new QCheckBox( ssdone, this );
      cksdone->setEnabled( false );
      QString strow    = QString::number( ii );
      cclabl ->setObjectName( strow + ": label" );
      pbwavln->setObjectName( strow + ": pb_wavln" );
      ckoptim->setObjectName( strow + ": ck_optim" );
      pbsload->setObjectName( strow + ": pb_sload" );
      pbsmanu->setObjectName( strow + ": pb_smanu" );
      cksdone->setObjectName( strow + ": ck_sdone" );

      ckoptim->setFont   ( ckfont );
      ckoptim->setPalette( ckpal );
      ckoptim->setChecked( false );
      ckoptim->setAutoFillBackground( true );
      cksdone->setFont   ( ckfont );
      cksdone->setPalette( ckpal );
      cksdone->setChecked( false );
      cksdone->setAutoFillBackground( true );

      bool is_vis      = ( ii < 4 );

      genL->addWidget( cclabl,  row,   0, 1, 1 );
      genL->addWidget( pbwavln, row,   1, 1, 1 );
      genL->addWidget( ckoptim, row,   2, 1, 1 );
      genL->addWidget( pbsload, row,   3, 1, 1 );
      genL->addWidget( pbsmanu, row,   4, 1, 1 );
      genL->addWidget( cksdone, row++, 5, 1, 1 );

      cclabl ->setVisible( is_vis );
      pbwavln->setVisible( is_vis );
      ckoptim->setVisible( is_vis );
      pbsload->setVisible( is_vis );
      pbsmanu->setVisible( is_vis );
      cksdone->setVisible( is_vis );

      connect( pbwavln, SIGNAL( clicked()           ),
               this,    SLOT  ( selectWavelengths() ) );
      connect( ckoptim, SIGNAL( toggled    ( bool ) ),
               this,    SLOT  ( checkOptima( bool ) ) );
      connect( pbsload, SIGNAL( clicked()           ),
               this,    SLOT  ( loadSpectrum()      ) );
      connect( pbsmanu, SIGNAL( clicked()           ),
               this,    SLOT  ( manualSpectrum()    ) );

      cc_labls << cclabl;
      cc_wavls << pbwavln;
      cc_optis << ckoptim;
      cc_loads << pbsload;
      cc_manus << pbsmanu;
      cc_dones << cksdone;
   }

#if 0
   // Build initial dummy internal values
   schans << "1 / A" << "1 / B" << "3 / A" << "3 / B";
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

   panel->addLayout( genL );
   panel->addStretch();

   initPanel();
}

// Function to rebuild the Spectra protocol after Optical change
void US_ExperGuiSpectra::rebuild_Spect( void )
{
   int nuvvis          = sibIValue( "optical", "nuvvis" );
   int nspect_sv       = rpSpect->nspect;
   nspchan             = schans.count();
DbgLv(1) << "EGwS: rbS: nuvvis" << nuvvis << "nspect_sv" << nspect_sv
 << "nspchan" << nspchan;

//   if ( nspect_sv == nuvvis  &&  nuvvis != 0 )
//      return;                           // No optical change means no rebuild

DbgLv(1) << "EGwS: rbS:  nspchan" << nspchan;
   if ( nspect_sv == 0 )
   {  // No existing Spectra protocol, so init with rudimentary one
      rpSpect->nspect     = nuvvis;
      rpSpect->chspecs.resize( nuvvis );
      QString uvvis       = tr( "UV/visible" );
      QStringList oprof   = sibLValue( "optical", "profiles" );
      int kuv             = 0;
      for ( int ii = 0; ii < oprof.count(); ii++ )
      {
         if ( oprof[ ii ].contains( uvvis ) )
         {
            rpSpect->chspecs[ kuv ].channel  = oprof[ ii ].section( ":", 0, 0 );
            rpSpect->chspecs[ kuv ].typeinp  = "manual";
            rpSpect->chspecs[ kuv ].wvlens.clear();
            rpSpect->chspecs[ kuv ].values.clear();
            rpSpect->chspecs[ kuv ].wvlens <<  280.0;
            rpSpect->chspecs[ kuv ].values <<  0.0;
            if ( ++kuv >= nuvvis )  break;
         }
      }
DbgLv(1) << "EGwS: rbS:  dummy proto  oprof count" << oprof.count() << "nuvvis" << nuvvis;
      return;
   }

   QString cur_pname   = sibSValue( "general", "protocol" );
DbgLv(1) << "EGwS: rbS:  protname" << protname << "cur_pname" << cur_pname;

   if ( protname != cur_pname )
   {  // Protocol has changed:  rebuild internals
      protname            = cur_pname;
      nspchan             = nspect_sv;
      rpSpect->nspect     = nuvvis;
      schans .resize( nspchan );
      stypes .resize( nspchan );
      swvlens.resize( nspchan );
      pwvlens.resize( nspchan );
      pvalues.resize( nspchan );
DbgLv(1) << "EGwS: rbS: rbI -- nspchan" << nspchan;

      for ( int ii = 0; ii < nspchan; ii++ )
      {
         schans [ ii ]       = rpSpect->chspecs[ ii ].channel;
         stypes [ ii ]       = rpSpect->chspecs[ ii ].typeinp;
         int nwavl           = rpSpect->chspecs[ ii ].wvlens.count();
         swvlens[ ii ].clear();
         pwvlens[ ii ].clear();
         pvalues[ ii ].clear();

         for ( int jj = 0; jj < nwavl; jj++ )
         {
            double wavelen      = rpSpect->chspecs[ ii ].wvlens[ jj ];
            double value        = rpSpect->chspecs[ ii ].values[ jj ];
            swvlens[ ii ] << wavelen;
            pwvlens[ ii ] << wavelen;
            pvalues[ ii ] << value;
DbgLv(1) << "EGwS: rbS:   ii jj " << ii << jj << "value" << value;
         }
      }
      return;
   }

   // Save info from any previous protocol
   QVector< US_RunProtocol::RunProtoSpectra::Spectrum > chspecs_sv;
   chspecs_sv          = rpSpect->chspecs;
   rpSpect->chspecs.clear();
   QStringList oprofs  = sibLValue( "optical", "profiles" );
   int nochan          = oprofs.count();
DbgLv(1) << "EGwS: rbS:  nochan" << nochan;

   // Save info from current panel parameters
   int nspchan_sv      = nspchan;
   int ntchan          = nspchan_sv + nochan;
DbgLv(1) << "EGwS: rbS:  nspchan_s ntchan" << nspchan_sv << ntchan;
   if ( nspchan_sv > 0 )
   {
      schans .resize( ntchan );
      stypes .resize( ntchan );
      swvlens.resize( ntchan );
      pwvlens.resize( ntchan );
      pvalues.resize( ntchan );
      int kk              = nochan;

      for ( int ii = 0; ii < nspchan_sv; ii++ )
      {
         schans [ kk ]    = schans [ ii ];
         stypes [ kk ]    = stypes [ ii ];
         swvlens[ kk ]    = swvlens[ ii ];
         pwvlens[ kk ]    = pwvlens[ ii ];
         pvalues[ kk ]    = pvalues[ ii ];
         schans [ ii ]    = "";
      }
   }

   // Now rebuild panel parameters and protocol
   QString uvvis       = tr( "UV/visible" );
DbgLv(1) << "EGwS: rbS:  nochan" << nochan;
   for ( int ii = 0; ii < nochan; ii++ )
   {
      QString pentry      = oprofs[ ii ];
      QString channel     = pentry.section( ":", 0, 0 ).simplified();
DbgLv(1) << "EGwS: rbS:   ii" << ii << "chan" << channel << "pentry" << pentry;

      if ( ! pentry.contains( uvvis ) )  continue;

      int spx             = -1;
      for ( int jj = 0; jj < nspect_sv; jj++ )
      {
         if ( chspecs_sv[ jj ].channel == channel )
         {  // Match in old protocol:  save its index
            spx                 = jj;
            break;
         }
      }

DbgLv(1) << "EGwS: rbS:     spx" << spx;
      if ( spx < 0 )
      {  // No such channel in old protocol:  use basic entry
         US_RunProtocol::RunProtoSpectra::Spectrum  chspec;
         chspec.channel      = channel;
         chspec.wvlens << 280.0;
         chspec.values << 0.0;
         rpSpect->chspecs << chspec;
      }
      else
      {  // Match to old protocol:  use that entry
         rpSpect->chspecs << chspecs_sv[ spx ];
      }
   }

   rpSpect->nspect     = rpSpect->chspecs.count();
   nspchan             = rpSpect->nspect;
   schans .resize( nspchan );
   stypes .resize( nspchan );
   swvlens.resize( nspchan );
   pwvlens.resize( nspchan );
   pvalues.resize( nspchan );
DbgLv(1) << "EGwS: rbS:  nspchan" << nspchan << "nspchan_sv" << nspchan_sv;

   // Rebuild panel parameters
   if ( nspchan_sv > 0 )
   {  // Use previous panel parameters
      for ( int ii = 0; ii < nspchan; ii++ )
      {
         QString channel     = rpSpect->chspecs[ ii ].channel;
         int ppx             = schans.indexOf( channel );
DbgLv(1) << "EGwS: rbS:    ii" << ii << "channel" << channel << "ppx" << ppx;
         if ( ppx >= 0 )
         {
            schans [ ii ]       = schans [ ppx ];
            stypes [ ii ]       = stypes [ ppx ];
            swvlens[ ii ]       = swvlens[ ppx ];
            pwvlens[ ii ]       = pwvlens[ ppx ];
            pvalues[ ii ]       = pvalues[ ppx ];
         }
         else
         {
            schans [ ii ]       = channel;
            stypes [ ii ]       = rpSpect->chspecs[ ii ].typeinp;
            swvlens[ ii ]       = rpSpect->chspecs[ ii ].wvlens;
            pwvlens[ ii ]       = rpSpect->chspecs[ ii ].wvlens;
            pvalues[ ii ]       = rpSpect->chspecs[ ii ].values;
         }
DbgLv(1) << "EGwS: rbS:     typ" << stypes[ii] << "wv0" << pwvlens[ii][0];
      }
   }
   else
   {  // Create first shot at panel parameters
      for ( int ii = 0; ii < nspchan; ii++ )
      {
DbgLv(1) << "EGwS: rbS:    ii" << ii << "channel" << rpSpect->chspecs[ii].channel;
         schans [ ii ]       = rpSpect->chspecs[ ii ].channel;
         stypes [ ii ]       = rpSpect->chspecs[ ii ].typeinp;
         swvlens[ ii ]       = rpSpect->chspecs[ ii ].wvlens;
         pwvlens[ ii ]       = rpSpect->chspecs[ ii ].wvlens;
         pvalues[ ii ]       = rpSpect->chspecs[ ii ].values;
      }
   }

}

// Slot to manage extinction profiles
void US_ExperGuiSpectra::manageEProfiles()
{
DbgLv(1) << "EGwS: mEP: IN";
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
void US_ExperGuiSpectra::process_results( QMap< double, double >& eprof )
{
DbgLv(1) << "EGwS: pr: eprof size" << eprof.keys().count() << "chrow" << chrow;
   pwvlens[ chrow ] = eprof.keys();
   pvalues[ chrow ].clear();

   for ( int ii = 0; ii < pwvlens[ chrow ].count(); ii++ )
   {
      pvalues[ chrow ] << eprof[ pwvlens[ chrow ][ ii ] ];
   }
}

// Slot to show details of all wavelength and spectra profiles
void US_ExperGuiSpectra::detailSpectra()
{
   // Create a new editor text dialog with fixed font
   US_Editor* ediag = new US_Editor( US_Editor::DEFAULT, true, "", this );
   ediag->setWindowTitle( tr( "Details: Wavelength / Extinction|Voltage|Gain Profiles" ) );
   ediag->resize( 720, 440 );
   ediag->e->setFont( QFont( US_Widgets::fixedFont().family(),
                             US_GuiSettings::fontSize() - 1,
                             QFont::Bold ) );
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Compose the text that it displays
   QString dtext  = tr( "Wavelength/Value Profile Information:\n\n" );
   dtext += tr( "Number of Panel Channels Used:     %1\n" )
            .arg( nspchan );
   dtext += tr( "Number of Protocol Channels Used:  %1\n" )
            .arg( rpSpect->nspect );

   if ( nspchan != rpSpect->nspect )
   {  // Should not occur!
      dtext += tr( "\n*ERROR* Channel counts should be identical\n" );
      nspchan   = 0;    // Skip channel details; too risky
   }

   for ( int ii = 0; ii < nspchan; ii++ )
   {  // Show information for each channel
      dtext += tr( "\n  Channel " ) + schans[ ii ] + " : \n";
      dtext += tr( "    Type Input                  : %1\n" )
               .arg( stypes[ ii ] );
      int nswavl = swvlens[ ii ].count();
      int npwavl = pwvlens[ ii ].count();
      int npvalu = pvalues[ ii ].count();
      int nowavl = rpSpect->chspecs[ ii ].wvlens.count();
      int novalu = rpSpect->chspecs[ ii ].values.count();

      dtext += tr( "    Selected Wavelength count   : %1\n" )
               .arg( nswavl );
      dtext += tr( "    Selected Wavelength range   : %1 to %2\n" )
               .arg( swvlens[ ii ][ 0 ] ).arg( swvlens[ ii ][ nswavl - 1 ] );
      dtext += tr( "    Profile Wavelength count    : %1\n" )
               .arg( npwavl );
      dtext += tr( "    Profile Wavelength range    : %1 to %2\n" )
               .arg( pwvlens[ ii ][ 0 ] ).arg( pwvlens[ ii ][ npwavl - 1 ] );
      dtext += tr( "    Profile Value range         : " );
      if ( stypes[ ii ] == "auto" )                    // Auto has no values
         dtext += tr( "(determined in the Optima)\n" );
      else if ( npvalu > 0 )                           // Non-auto value range
         dtext += tr( "%1 to %2\n" ).arg( pvalues[ ii ][ 0 ] )
                                    .arg( pvalues[ ii ][ npvalu - 1 ] );
      else                                             // Non-auto, no values
         dtext += tr( "(no values)\n" );

      dtext += tr( "    Protocol Wavelength count   : %1\n" )
               .arg( nowavl );
      dtext += tr( "    Protocol Wavelength range   : %1 to %2\n" )
               .arg( rpSpect->chspecs[ ii ].wvlens[ 0 ] )
               .arg( rpSpect->chspecs[ ii ].wvlens[ nowavl - 1 ] );
      dtext += tr( "    Protocol Value range        : " );
      if ( rpSpect->chspecs[ ii ].typeinp == "auto" )  // Auto has no values
         dtext += tr( "(determined in the Optima)\n" );
      else if ( novalu > 0 )                           // Non-auto values
      {
         dtext += tr( "%1 to %2\n" )
                  .arg( rpSpect->chspecs[ ii ].values[ 0 ] )
                  .arg( rpSpect->chspecs[ ii ].values[ novalu - 1 ] );
         double valavg = 0.0;
         for ( int jj = 0; jj < novalu; jj++ )
            valavg += rpSpect->chspecs[ ii ].values[ jj ];
         valavg /= (double)novalu;
         dtext += tr( "    Protocol Value average      : %1\n" )
                  .arg( valavg );
      }
      else                                             // Non-auto, no values
         dtext += tr( "(no values)\n" );
   }

   // Load text and show the dialog
   QApplication::restoreOverrideCursor();
   qApp->processEvents();

   ediag->e->setText( dtext );
   ediag->show();
}

// Slot to select wavelengths using a dialog
void US_ExperGuiSpectra::selectWavelengths()
{
   QObject* sobj       = sender();      // Sender object
   QString sname       = sobj->objectName();
   chrow               = sname.section( ":", 0, 0 ).toInt();
   QString cclabl      = cc_labls[ chrow ]->text();
   QStringList wlpoten;                 // Potential wavelength list
   QStringList wlselec;                 // Selected wavelength list

   int nswavl          = swvlens[ chrow ].count();
   int npwavl          = pwvlens[ chrow ].count();

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
DbgLv(1) << "EGwS: sW: wlpoten" << wlpoten;
DbgLv(1) << "EGwS: sW: wlselec" << wlselec;

   // Open a wavelengths choice dialog and use to update selected wavelengths

   US_SelectWavelengths* swdiag = new US_SelectWavelengths( wlpoten, wlselec );

   swdiag->exec();

   swvlens[ chrow ].clear();
   for ( int ii = 0; ii < wlselec.count(); ii++ )
      swvlens[ chrow ] << wlselec[ ii ].toDouble();
DbgLv(1) << "EGwS: sW: swvlens" << swvlens;
}

// Slot to handle the toggle of an "in Optima" checkbox
void US_ExperGuiSpectra::checkOptima( bool checked )
{
   QObject* sobj       = sender();      // Sender object
   QString sname       = sobj->objectName();
   chrow               = sname.section( ":", 0, 0 ).toInt();
   QString cclabl      = cc_labls[ chrow ]->text();
DbgLv(1) << "EGwS:ckOpt: sname" << sname << "chrow" << chrow << cclabl;
   cc_loads[ chrow ]->setEnabled( !checked );
   cc_manus[ chrow ]->setEnabled( !checked );


   if ( checked )
   {  // "Auto" has been turned on
DbgLv(1) << "EGwS:ckOpt:  count pwv pva swv" << pwvlens.count() << pvalues.count()
 << swvlens.count();
      cc_dones[ chrow ]->setChecked( checked );   // Also check "Done"
DbgLv(1) << "EGwS:ckOpt:  count styp" << stypes.count();
      stypes.replace( chrow, "auto" );
      int nvals           = swvlens[ chrow ].count();
DbgLv(1) << "EGwS:ckOpt:  nvals(swv[r])" << nvals;
      pwvlens[ chrow ]    = swvlens[ chrow ];     // Profile wls == Selected
      pvalues[ chrow ].clear();
      for ( int jj = 0; jj < nvals; jj++ )        // Values all zero
         pvalues[ chrow ] << 0.0;
   }
   else
   {  // "Auto" has been unchecked
      stypes.replace( chrow, "manual" );          // Default alternative
      cc_dones[ chrow ]->setChecked( checked );   // Uncheck "Done" for now
   }
DbgLv(1) << "EGwS:ckOpt:  OK";
}

// Slot to load a spectrum profile using us_extinction
void US_ExperGuiSpectra::loadSpectrum()
{
   QObject* sobj       = sender();      // Sender object
   QString sname       = sobj->objectName();
   chrow               = sname.section( ":", 0, 0 ).toInt();
   QString cclabl      = cc_labls[ chrow ]->text();
DbgLv(1) << "EGwS:loadS: sname" << sname << "chrow" << chrow << cclabl;
   //US_Extinction ediag( "BUFFER", "some buffer", this );
   US_Extinction* ediag = new US_Extinction;
   ediag->setParent(   this, Qt::Window );
   ediag->setAttribute( Qt::WA_DeleteOnClose );

   connect( ediag,  SIGNAL( get_results(     QMap<double,double>& ) ),
            this,   SLOT  ( process_results( QMap<double,double>& ) ) );

   ediag->show();
}

// Slot to manually enter a spectrum profile using us_table
void US_ExperGuiSpectra::manualSpectrum()
{
   QObject* sobj       = sender();      // Sender object
   QString sname       = sobj->objectName();
   chrow               = sname.section( ":", 0, 0 ).toInt();
   QString channel     = schans[ chrow ];
DbgLv(1) << "EGwS:manSp: sname" << sname << "chrow" << chrow << channel;
   QMap< double, double >  extinction;

   for ( int ii = 0; ii < pwvlens[ chrow ].count(); ii++ )
   {
      double wavelen      = pwvlens[ chrow ][ ii ];
      double value        = pvalues[ chrow ][ ii ];
      extinction[ wavelen ]  = value;
DbgLv(1) << "EGwS:manSp:  ii" << ii << "w v" << wavelen << value;
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
DbgLv(1) << "EGwS:manSP: extinc size" << nwavl << "chrow" << chrow;
      pwvlens[ chrow ] = ewavls;
      pvalues[ chrow ].clear();

      for ( int ii = 0; ii < nwavl; ii++ )
      {
         pvalues[ chrow ] << extinction[ ewavls[ ii ] ];
      }
DbgLv(1) << "EGwS:manSP:  0 : wavl valu" << pwvlens[chrow][0] << pvalues[chrow][0];
int nn=nwavl-1;
if (nwavl>0)
 DbgLv(1) << "EGwS:manSP:  n : wavl valu" << pwvlens[chrow][nn] << pvalues[chrow][nn];

      if ( swvlens[ chrow ].count() == 0 )
      {
         swvlens[ chrow ] = pwvlens[ chrow ];
      }
   }
else
DbgLv(1) << "EGwS:manSp: *NOT Accepted*";
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
   pb_accept->setEnabled( nbr_selec > 0 );
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
   pb_accept->setEnabled( nbr_selec > 0 );
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
   select_wavls.clear();

   reject();
   close();
}

// Accept button clicked:  returned delete-selections list is the excluded list
void US_SelectWavelengths::done( void )
{
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

