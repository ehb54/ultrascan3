//! \file us_eqreporter.cpp

#include "us_eqreporter.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math2.h"
#include "us_editor.h"

// Main constructor
US_EqReporter::US_EqReporter(
      QVector< US_DataIO2::EditedData >& dataList,
      QStringList&                       triples,
      QVector< ScanEdit >&               scedits,
      QVector< EqScanFit >&              scanfits,
      EqRunFit&                          runfit,
      QWidget*                           wparent )
 : QObject(),
   dataList   ( dataList ),
   triples    ( triples  ),
   scedits    ( scedits  ),
   scanfits   ( scanfits ),
   runfit     ( runfit   ),
   wparent    ( wparent )
{

   dbg_level = US_Settings::us_debug();
   asters    = QString().fill( '*', 80 ).append( "\n" );
}

// Scan Diagnostics and display a report in a text dialog
void US_EqReporter::scan_diagnostics()
{
DbgLv(1) << "SCAN_DIAGS()";

   QString plsrd  = tr( "PLEASE READ THIS!" );
   QString rs;

   // Compose opening general notes
   rs  = "\n" + centerInLine( plsrd, 80, false, ' ' ) + "\n\n";
   rs += tr( "Below is a listing of the ratios of slopes in the endpoints"
             " of each indicated\n" );
   rs += tr( "scan. If the ratios are less than 30, then there is little"
             " information content\n" );
   rs += tr( "in the scan and chances are that the experiment was"
             " improperly set up\nand should be repeated.\n\n" );
   rs += tr( "Additional warnings will be generated if the scan does not"
             " contain enough\n" );
   rs += tr( "data points or if the experimenter did not use the majority"
             " of the linear\n" );
   rs += tr( "absorbance range available (at least 0.6 OD between 0.0 OD"
             " and 0.9 OD).\n\n" );
   rs += tr( "These warnings are for your information only; they have no"
             " effect on the\n" );
   rs += tr( "rest of the program, since there are valid exceptions to"
             " these warnings\n" );
   rs += tr( "when including such scans is appropriate. Please refer to"
             " the global\n" );
   rs += tr( "equilibrium analysis tutorial for more information.\n\n" );

   bool scprobs  = false;
   int  dimvs    = dataList[ 0 ].x.size() * 3 / 2;
   QVector< double > xvec( dimvs );
   QVector< double > yvec( dimvs );
   double* xx    = xvec.data();
   double* yy    = yvec.data();

   // Compose notes on each scan
   for ( int jes = 0; jes < scedits.size(); jes++ )
   {
      int     jdx    = scedits[ jes ].dsindex;  // data set index
      double  radlo  = scedits[ jes ].rad_lo;   // radius low value
      double  radhi  = scedits[ jes ].rad_hi;   // radius high value

      // Scan information header
      rs += scanInfoHeader( jes, jdx );

      // Point to data, scan and this scan's data range
      US_DataIO2::EditedData* sdata = &dataList[ jdx ];
      int ivstx = index_radius( sdata, radlo );
      int ivenx = index_radius( sdata, radhi );
      int ivenn = ivenx + 1;
      int npts  = ivenn - ivstx;
      scanfits[ jes ].start_ndx = ivstx;
      scanfits[ jes ].stop_ndx  = ivenx;

      if ( npts > dimvs )
      {  // If need be (unlikely), resize the work x,y vectors
         dimvs  = npts + 10;
         xvec.resize( dimvs );
         yvec.resize( dimvs );
         xx     = xvec.data();
         yy     = yvec.data();
      }
DbgLv(1) << "SDiag: jes" << jes << "ivstx ivenx npts" << ivstx << ivenx << npts;
DbgLv(1) << "SDiag:  radlo radhi" << radlo << radhi
   << " rs re" << sdata->radius(ivstx) << sdata->radius(ivenx)
   << " r0 rn" << sdata->radius(0) << sdata->radius(sdata->x.size()-1);

      int nwarns = 0;             // Initialize for scan analysis

      if ( npts > 50 )
      {  // If sufficient points, analyze slopes and ratios
         int    nspts  = npts / 5;
         int    knt    = 0;
         double slope1 = 0.0;
         double slope2 = 0.0;
         double icept  = 0.0;
         double sigma  = 0.0;
         double corr   = 0.0;

         for ( int jj = ivstx; jj < ivstx + nspts; jj++ )
         {  // Accumulate work arrays of beginning points
            xx[ knt   ] = scanfits[ jes ].xvs[ jj ];
            yy[ knt++ ] = scanfits[ jes ].yvs[ jj ];
         }

         // Get the slope at the beginning, then get ystart
         US_Math2::linefit( &xx, &yy, &slope1, &icept, &sigma, &corr, knt );
DbgLv(1) << "SDiag:   knt slope1 icept" << knt << slope1 << icept;
         double xstart = xx[ 0 ];
         double ystart = slope1 * xstart + icept;

         knt        = 0;
         nspts      = npts / 10;

         for ( int jj = ivenn - nspts; jj < ivenn; jj++ )
         {  // Accumulate work arrays of ending points
            xx[ knt   ] = scanfits[ jes ].xvs[ jj ];
            yy[ knt++ ] = scanfits[ jes ].yvs[ jj ];
         }

         // Get the slope at the end, then get yend
         US_Math2::linefit( &xx, &yy, &slope2, &icept, &sigma, &corr, knt );
         double xend   = xx[ knt - 1 ];
         double yend   = slope2 * xend + icept;

         // Get slope ratio and absorbance range
         slope1        = ( slope1 == 0.0 ) ? 9.999999e-21 : slope1;
         double ratio  = slope2 / slope1;
         double rangea = yend - ystart;
DbgLv(1) << "SDiag:   knt slope2 icept" << knt << slope2 << icept;
DbgLv(1) << "SDiag:    y0 y1 ym yn" << yy[0] << yy[1] << yy[knt-2] << yy[knt-1];
DbgLv(1) << "SDiag:     xend yend" << xend << yend;
DbgLv(1) << "SDiag:      ratio rangea" << ratio << rangea;

         rs += tr( "Slope at beginning: %1,  Slope at end %2,  "
                   "Ratio: %3\n\n" ).arg( slope1 ).arg( slope2 ).arg( ratio );

         // Determine and add notes for any warnings

         if ( ratio > 0.0  &&  ratio < 1.5 )
         {
            rs += tr( "Warning: The ratio is very small - there is"
                      " probably not enough\n" 
                      "information in this scan.\n"
                      "Suggestion: use a higher speed. Also, check"
                      " for aggregation!\n\n" );
            nwarns++;
         }

         if ( slope1 < 0.0 )
         {
            rs += tr( "Warning: The start point slope for this scan"
                      " is negative!\n"
                      "Possible reasons: excessive noise in the data,"
                      " or time invariant noise\n" 
                      "from interference data has not been subtracted.\n\n" );
            nwarns++;
         }

         if ( slope2 < 0.0 )
         {
            rs += tr( "Warning: The end point slope for this scan"
                      " is negative!\n"
                      "Possible reasons: excessive noise in the data,"
                      " or time invariant noise\n" 
                      "from interference data has not been subtracted.\n\n" );
            nwarns++;
         }

         if ( rangea < 0.4 )
         {
            rs += tr( "Warning: This scan only spans %1 OD of the"
                      " possible\n" ). arg( rangea );
            rs += tr( "0.9 - 1.0 OD range the instrument allows.\n\n" );
            nwarns++;
         }

         if ( yend < 0.6 )
         {
            rs += tr( "Warning: This scan's maximum absorbance is only "
                      " %1 OD.\n" ).arg( yend );
            rs += tr( "This is lower than the linear range of the XL-A"
                      " which generally extends\n"
                      "up to ~0.9 OD. You may be discarding information."
                      " Check for Aggregation!\n\n" );
            nwarns++;
         }
      }

      // Add final notes on points and warnings

      rs += tr( "Number of points in this scan: %1" ).arg( npts );

      if ( npts >= 100 )
         rs += tr( "\n\n" );

      else
      {
         if ( npts >= 50 )
            rs += tr( " (low!)\n\n" );
         else
            rs += tr( " (too low! Are the data below the OD cutoff?)\n\n" );
         nwarns++;
      }

      if ( nwarns == 1 )
         rs += tr( "There was 1 warning generated for this scan.\n" );

      else
         rs += tr( "There were %1 warnings generated for this scan.\n" )
            .arg( nwarns );

      if ( nwarns > 2 )
         rs += tr( "Please check the scan to make sure it is appropriate"
                   " for inclusion in a global fit!\n" );

      rs += "\n";

      if ( nwarns == 0 )
      {  // Mark scan as fit/non-excluded
         scanfits[ jes ].scanFit  = true;
         scanfits[ jes ].autoExcl = false;
      }

      else
      {  // Mark scan as non-fit/excluded
         scanfits[ jes ].scanFit  = false;
         scanfits[ jes ].autoExcl = true;
         scprobs                  = true;
      }
   }  // End:  scans loop

   if ( scprobs )
   {  // Pop up dialog warning of potential problems
      QMessageBox::warning( wparent, tr( "Scan Problem(s)" ),
         tr( "One or more scans have been excluded from the fit.\n"
             "The Diagnostics report will help you to determine\n"
             "which problems occurred. You can manually override\n"
             "scan exclusions and include them, once you identify\n"
             "the reasons for the exclusion." ) );

   }

   // Display scan diagnostics in an editor text dialog
   US_Editor* ediag = new US_Editor( US_Editor::DEFAULT, true, "*.res",
         wparent );
   ediag->setWindowTitle( tr( "Scan Diagnostics" ) );
   QFont efont( US_GuiSettings::fontFamily(),
                US_GuiSettings::fontSize() - 2 );
   ediag->e->setFont( efont );
   ediag->e->setText( rs );
   QFontMetrics fm( efont );
   int dwid = maxLineWidth( fm, rs ) + fm.width( "WWW" );
DbgLv(1) << "dwid" << dwid;
   int dhgt = fm.lineSpacing() * 50;
   dwid     = ( ( dwid / 12 + 2 ) * 12 );
   dhgt     = ( ( dhgt / 12 + 2 ) * 12 );
   ediag->resize( dwid, dhgt );
   ediag->move( wparent->pos() + QPoint( 400, 100 ) );
   ediag->show();

   // Output the text, also, to a reports file
   QString basedir  = US_Settings::reportDir();
   QString repdir   = dataList[ 0 ].runID;
   QDir folder( basedir );

   if ( ! folder.exists( repdir ) )
   {
      if ( ! folder.mkdir( repdir ) )
      {
         QMessageBox::warning( wparent, tr( "File Error" ),
            tr( "Could not create the directory:\n" )
            + basedir + "/" + repdir );
         return;
      }
   }

   QString filename = basedir + "/" + repdir + "/globeq.diagnostics.rpt";
   QFile drf( filename );

   if ( ! drf.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      QMessageBox::warning( wparent, tr( "File Error" ),
         tr( "Unable to open the file:\n" ) + filename );
      return;
   }

   QTextStream ts( &drf );       // Write report text to file
   ts << rs;
   drf.close();
}

// Check scan fit
bool US_EqReporter::check_scan_fit( int modelx )
{
DbgLv(1) << "  EqRep:CHECK_SCAN_FIT()";
   int  ffitx    = 0;
   bool critical = false;

   while ( ! scanfits[ ffitx ].scanFit )
      ffitx++;

DbgLv(1) << "  EqRep:CSF: ffitx" << ffitx;
   QString rs;

   rs += tr( "In order to assure that the proper parameters are used"
             " for the fitting process,\n" );
   rs += tr( "the following information has been compiled about the"
             " components in your\n" );
   rs += tr( "model and the scans included in the fit --\n\n"
             "Component Information:\n\n" );

   for ( int jj = 0; jj < runfit.nbr_comps; jj++ )
   {
      int cnum = jj + 1;

      if ( runfit.mw_fits[ jj ] )
      {
         if ( runfit.mw_vals[ jj ] == 0.0 )
         {
            rs += tr( "Although the molecular weight parameter for"
                      "component %1 has been floated,\n" ).arg( cnum );
            rs += tr( "the value for this parameter is equal to zero"
                      " - fitting aborted!\n" );
            critical = true;
         }

         if ( runfit.mw_rngs[ jj ] == 0.0 )
         {
            rs += tr( "Although the molecular weight parameter for"
                      "component %1 has been floated,\n" ).arg( cnum );
            rs += tr( "the range for this parameter is equal to zero"
                      " - fitting aborted!\n" );
            critical = true;
         }
      }

      else
      {
         rs += tr( "The molecular weight parameter for component %1"
                   " has been fixed.\n" ).arg( cnum );
      }

      if ( runfit.vbar_fits[ jj ] )
      {
         if ( runfit.vbar_vals[ jj ] == 0.0 )
         {
            rs += tr( "Although the vbar parameter for"
                      "component %1 has been floated,\n" ).arg( cnum );
            rs += tr( "the value for this parameter is equal to zero"
                      " - fitting aborted!\n" );
            critical = true;
         }

         if ( runfit.vbar_rngs[ jj ] == 0.0 )
         {
            rs += tr( "Although the vbar parameter for"
                      "component %1 has been floated,\n" ).arg( cnum );
            rs += tr( "the range for this parameter is equal to zero"
                      " - fitting aborted!\n" );
            critical = true;
         }
      }

      else
      {
         rs += tr( "The vbar parameter for component %1"
                   " has been fixed.\n" ).arg( cnum );
      }

      if ( (int)( 1000000.0 * runfit.vbar_vals[ jj ] + 0.5 ) == 720000 )
      {
         rs += tr( "The vbar value for component %1 has been"
                   " left at 0.72,\n" ).arg( cnum );
         rs += tr( "which is the default value - are you sure"
                   " you want to use this value?\n" );
      }

   }

   for ( int jj = 0; jj < runfit.nbr_assocs; jj++ )
   {
      int anum = jj + 1;

      if ( runfit.eq_fits[ jj ] )
      {
         if ( runfit.eq_vals[ jj ] == 0.0 )
         {
            rs += tr( "Although the equilibrium constant %1 has"
                      " been floated,\n" ).arg( anum );
            rs += tr( "the value for this parameter is equal to zero"
                      " - fitting aborted!\n" );
            critical = true;
         }

         if ( runfit.eq_rngs[ jj ] == 0.0 )
         {
            rs += tr( "Although the equilibrium constant %1 has"
                      " been floated,\n" ).arg( anum );
            rs += tr( "the range for this parameter is equal to zero"
                      " - fitting aborted!\n" );
            critical = true;
         }
      }

      else
      {
         rs += tr( "The equilibrium constant %1 has been fixed"
                   " - are you sure you want to do that?\n" ).arg( anum );
      }
   }

   if ( modelx > 3 )
   {
      bool   same_lambda = true;
      bool   same_extinc = true;
      int    test_lambda = scanfits[ ffitx ].lambda;
DbgLv(1) << "  EqRep:CSF: fx extsz" << ffitx << scanfits[ffitx].extincts.size();
DbgLv(1) << "  EqRep:CSF: test_lambda" << test_lambda;
      double test_extinc = scanfits[ ffitx ].extincts[ 0 ];
DbgLv(1) << "  EqRep:CSF: test_extinc" << test_extinc;

      for ( int jj = 0; jj < scanfits.size(); jj++ )
      {
         if ( scanfits[ jj ].scanFit )
         {
            if ( scanfits[ jj ].lambda != test_lambda )
               same_lambda = false;

            if ( scanfits[ jj ].extincts[ 0 ] != test_extinc )
               same_extinc = false;
         }
      }

      if ( ! same_lambda  &&  same_extinc )
         rs += tr( "\nWarning:\n"
                   "Your project contains scans with different wavelengths\n"
                   "but identical extinction coefficients!\n" );

      else if ( ! same_lambda  &&  ! same_extinc )
         rs += tr( "\nWarning:\n"
                   "Your project contains scans with different wavelengths;\n"
                   "make sure that the extinction coefficients match!\n" );
   }

   // Compose notes on each scan
   for ( int jes = 0; jes < scedits.size(); jes++ )
   {
      int     jdx    = scedits[ jes ].dsindex;  // data set index

      // Scan information header
      rs += scanInfoHeader( jes, jdx );

      if ( scanfits[ jes ].scanFit )
      {
         for ( int jj = 0; jj < runfit.nbr_comps; jj++ )
         {
            int cnum = jj + 1;

            if ( scanfits[ jes ].amp_fits[ jj ] )
            {
               if ( scanfits[ jes ].amp_vals[ jj ] == 0.0 )
               {
                  rs += tr( "Although the amplitude for component %1 has"
                            " been floated,\nthe value for this parameter"
                            " is equal to zero - fitting aborted!\n" );
                  critical = true;
               }

               if ( scanfits[ jes ].amp_rngs[ jj ] == 0.0 )
               {
                  rs += tr( "Although the amplitude for component %1 has"
                            " been floated,\nthe range for this parameter"
                            " is equal to zero - fitting aborted!\n" );
                  critical = true;
               }

            }

            else
            {
               rs += tr( "The amplitude for component %1 has been fixed -"
                         " are you sure you want to do that?\n" ).arg( cnum );
            }

            if ( runfit.nbr_assocs > 0 )
            {
               if ( scanfits[ jes ].extincts[ jj ] == 0.0 )
               {
                  rs += tr( "The extinction coefficient for component %1 is"
                            " equal to zero - are you sure?\n" ).arg( cnum );
                  rs += tr( "(This could be valid if this component does"
                            " not absort at %1 nm)\n" )
                        .arg( scanfits[ jes ].wavelen );
               }
            }
         }

         if ( scanfits[ jes ].baseln_fit )
         {
            if ( scanfits[ jes ].baseln_rng == 0.0 )
            {
               rs += tr( "Although the baseline for this scan has"
                         " been floated,\nthe range for this parameter"
                         " is equal to zero - fitting aborted!\n" );
               critical = true;
            }
         }

         else
         {
            rs += tr( "The baseline for this scan has been fixed -"
                      " are you sure you want to do that?\n" );
         }

         if ( (int)( 1000000.0 * scanfits[ jes ].density + 0.5 ) == 998234 )
         {
            rs += tr( "The density setting corresponds to pure water -"
                      " are you sure you want to use that?\n" );
         }

         rs += "\n";
      }

      else
      {
         rs += tr( "This scan has been excluded from the fit.\n" );
      }
   }

   // Display scan fit checks in an editor text dialog
   US_Editor* ediag = new US_Editor( US_Editor::DEFAULT, true, "*.res",
         wparent );
   ediag->setWindowTitle( tr( "Scan Fit Check" ) );
   QFont efont( US_GuiSettings::fontFamily(),
                US_GuiSettings::fontSize() - 2 );
   ediag->e->setFont( efont );
   ediag->e->setText( rs );
   QFontMetrics fm( efont );
   int dwid = maxLineWidth( fm, rs ) + fm.width( "WWW" );
DbgLv(1) << "  dwid" << dwid << " astw" << fm.width( asters );
   int dhgt = fm.lineSpacing() * 50;
   dwid     = ( ( dwid / 12 + 2 ) * 12 );
   dhgt     = ( ( dhgt / 12 + 2 ) * 12 );
DbgLv(1) << "    dwid" << dwid;
   ediag->resize( dwid, dhgt );
   ediag->move( wparent->pos() + QPoint( 450, 150 ) );
   ediag->show();

   // Output the text, also, to a reports file
   QString basedir  = US_Settings::reportDir();
   QString repdir   = dataList[ 0 ].runID;
   QDir folder( basedir );

   if ( ! folder.exists( repdir ) )
   {
      if ( ! folder.mkdir( repdir ) )
      {
         QMessageBox::warning( wparent, tr( "File Error" ),
            tr( "Could not create the directory:\n" )
            + basedir + "/" + repdir );
         return true;
      }
   }

   QString filename = basedir + "/" + repdir + "/globeq.scan_check.rpt";
   QFile drf( filename );

   if ( ! drf.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      QMessageBox::warning( wparent, tr( "File Error" ),
         tr( "Unable to open the file:\n" ) + filename );
      return true;
   }

   QTextStream ts( &drf );       // Write report text to file
   ts << rs;
   drf.close();

   return critical;
}

// Determine the index in the radius vector of a given radius
int US_EqReporter::index_radius( US_DataIO2::EditedData* edat, double radius )
{
   int l_index = edat->x.size() - 1;
   int r_index = -1;

   while ( ++r_index < l_index )
   {
      if ( radius <= edat->radius( r_index ) )
         break;
   }

   return r_index;
}

// Compose a string that centers a title string within a line
QString US_EqReporter::centerInLine( const QString& titl_text, int linelen,
      bool rightPad, const QChar end_char )
{
   int  tlen     = titl_text.length();       // title text length
   int  plen     = ( linelen - tlen ) / 2;   // pad characters length
   bool have_end = ( ! end_char.isNull() && end_char != ' ' );  // end char?

   // Pad to the left in order to center the given text
   QString linestr = QString().fill( ' ', plen ) + titl_text;

   if ( rightPad )
   {  // If also right-padding, add pad spaces to the right
      plen     = linelen - tlen - plen;
      linestr += QString().fill( ' ', plen );

      // If an end character was given, use it at the end of the line
      if ( have_end )
         linestr.replace( linestr.length() - 1, 1, end_char );
   }

   // If an end character was given, use it at the beginning of the line
   if ( have_end )
      linestr.replace( 0, 1, end_char );

   return linestr;
}

// Compose a scan information header string
QString US_EqReporter::scanInfoHeader( int jes, int jdx )
{
   QString s_scnn = QString::number( jes + 1 );
   QString s_trip = triples[ jdx ];
   QString s_cell = s_trip.section( "/", 0, 0 ).simplified();
   QString s_chan = s_trip.section( "/", 1, 1 ).simplified();
   QString s_wave = s_trip.section( "/", 2, 2 ).simplified();
   QString s_rpm  = QString::number( scanfits[ jes ].rpm );

   return ( "\n" + asters
            + tr( "Information for Scan " ) + s_scnn
            + tr( ",  Cell " ) + s_cell
            + tr( ", Channel " ) + s_chan
            + tr( ", Wavelength " ) + s_wave
            + " nm,  " + s_rpm + tr( " rpm" )
            + "\n   [  " + scanfits[ jes ].descript + "  ]\n" + asters );
}

// Return the maximum line in a report string
int US_EqReporter::maxLineWidth( QFontMetrics& fm, const QString& repstr )
{
   QStringList rlines = repstr.split( "\n" );
   int mxlen  = 0;
   int lnlen  = 0;

   for ( int ii = 0; ii < rlines.size(); ii++ )
   {
      if ( ( lnlen = fm.width( rlines[ ii ] ) ) > mxlen )
      {
         mxlen = lnlen;
DbgLv(1) << "  mLW: ii lnlen" << ii << lnlen << " line:" << rlines[ii];
      }
   }

   return mxlen;
}

