#include "../include/us_saxs_util.h"

/* To do notes:
   for wiki:
   ? support multiple waxs samples for wgsbs 
   full parameterization (search for "these should be parameterized")
   pnggnuplot.pl should be integrated, instead of as an external perl program
   mkgnom.pl should be integrated
   ? system calls should be replaced with qprocess
   final cropping of waves
   better fit mechanism for wgsbs (current one is too slow, I'm sure some more efficient method is possible)
   ? setup dammin/gasbor scripts, add results to wiki 
*/   

#include <list>

#if defined(WIN32)
#   include <dos.h>
#   include <stdlib.h>
#   include <float.h>
//Added by qt3to4:
#include <QTextStream>
// #   define isnan _isnan
#   undef SHOW_TIMING
#endif

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

void US_Saxs_Scan::clear( )
{
   filename = "";
   header = "";
   header_cols.clear( );
   q.clear( );
   r.clear( );
   s.clear( );
}

US_Saxs_Util::US_Saxs_Util()
{
   debug = 0;
   wgsbs_gsm_setup = false;
   nsa_gsm_setup = false;
   this_rank = 0;
   nsa_mpi = false;
   us_log = (US_Log *) 0;
   us_udp_msg = (US_Udp_Msg *) 0;
}

US_Saxs_Util::~US_Saxs_Util()
{
   if ( us_log )
   {
      delete us_log;
   }
   if ( us_udp_msg )
   {
      delete us_udp_msg;
   }
}

void US_Saxs_Util::clear( )
{
   wave.clear( );
}

QString US_Saxs_Util::list_waves()
{
   QString result;
   for ( map < QString, US_Saxs_Scan >::iterator it = wave.begin();
         it != wave.end();
         it++ 
         )
   {
      result += it->first + "\n";
   }
   return result;
}

bool US_Saxs_Util::read( QString filename, QString tag )
{
   // read filename into wave[tag]
   wave[tag].clear( );
   wave[tag].filename = filename;

   errormsg = "";

   QFile f(filename);
   
   if ( f.open(QIODevice::ReadOnly) )
   {
      QTextStream ts(&f);

      QString firstline;

      QRegExp rxskip("^#");
      QRegExp rx3("^\\s*(\\S*)\\s+(\\S*)\\s+(\\S*)\\s*$");
      // this next one should work, but doesn't for some reason
      //  QRegExp rxdigits("^\\s*(\\d*(|\\.\\d*)\\s+(\\d*(|\\.\\d*)\\s+(\\d*(|\\.\\d*)\\s*$");
      // just check to see if it starts with a digit
      QRegExp rxdigits("^\\s*\\d");

      while ( !ts.atEnd() )
      {
         firstline = ts.readLine();
         if ( rxskip.indexIn(firstline) == -1 )
         {
            break;
         }
      }

      wave[tag].header = firstline;
      if ( rxdigits.indexIn(firstline) != -1 )
      { 
         // line is 3 numbers force a new header
         cout << "force default header\n";
         wave[tag].header = "q I(q) error";
      } else {
         firstline = "";
      }

      if ( rx3.indexIn(wave[tag].header) == -1 )
      {
         errormsg = "could not find 3 columns in file header " + filename;
         f.close();
         return false;
      }

      wave[tag].header_cols.push_back(rx3.cap(1));
      wave[tag].header_cols.push_back(rx3.cap(2));
      wave[tag].header_cols.push_back(rx3.cap(3));

      QString line;

      while ( !ts.atEnd() ) 
      {
         if ( !firstline.isEmpty() )
         {
            line = firstline;
            firstline = "";
         } else {
            line = ts.readLine();
         }

         if ( rxskip.indexIn(line) != -1 )
         {
            continue;
         }

         if ( rx3.indexIn(line) == -1 )
         {
            errormsg = "could not find 3 columns in file line " + filename;
            f.close();
            return false;
         }
         
         wave[tag].q.push_back(rx3.cap(1).toDouble());
         wave[tag].r.push_back(rx3.cap(2).toDouble());
         wave[tag].s.push_back(rx3.cap(3).toDouble());
      }
      f.close();
      return true;
   }
   errormsg = "could not open file " + filename;
   return false;
}

bool US_Saxs_Util::write(QString filename, QString tag)
{
   // write wave[tag] into filename 
   errormsg = "";

   QFile f(filename);

   if ( f.open(QIODevice::WriteOnly ) )
   {
      QTextStream ts(&f);
      ts << wave[tag].header << "\n";
      for ( unsigned int i = 0; i < wave[tag].q.size(); i++ )
      {
         ts << 
            QString("%1\t%2\t%3\n")
            .arg(wave[tag].q[i])
            .arg(wave[tag].r[i])
            .arg(wave[tag].s[i]);
      }
      f.close();
      return true;
   }

   errormsg = "can not create file " + filename + "\n";
   return false;
}

bool US_Saxs_Util::compat(QString tag1, QString tag2)
{
   if ( wave[tag1].q.size() != wave[tag2].q.size() )
   {
      errormsg = QString("q not same size %1 %2").arg(tag1).arg(tag2);
      return false;
   }

   for ( unsigned int i = 0; i < wave[tag1].q.size(); i++ )
   {
      if ( QString("%1").arg(wave[tag1].q[i]) != QString("%1").arg(wave[tag2].q[i]) )
      {
         errormsg = QString("q not on same %1 %2 at pos %3 %4 %5")
            .arg(tag1)
            .arg(tag2)
            .arg(i+1)
            .arg(wave[tag1].q[i])
            .arg(wave[tag2].q[i]);
         return false;
      }
   }
   return true;
}

bool US_Saxs_Util::avg(QString outtag, vector < QString > tags)
{
   errormsg = "";
   
   for ( unsigned int i = 1; i < tags.size(); i++ )
   {
      if ( !compat(tags[0],tags[i]) )
      {
         return false;
      }
   }
      
   wave[outtag] = wave[tags[0]];
   wave[outtag].filename = "average";

   for ( unsigned int i = 1; i < tags.size(); i++ )
   {
      for ( unsigned int j = 0; j < wave[outtag].q.size(); j++ )
      {
         wave[outtag].r[j] += wave[tags[i]].r[j];
         wave[outtag].s[j] += wave[tags[i]].s[j];
      }
   }

   for ( unsigned int j = 0; j < wave[outtag].q.size(); j++ )
   {
      wave[outtag].r[j] /= tags.size();
      wave[outtag].s[j] /= tags.size();
   }
   return true;
}

bool US_Saxs_Util::scalesum(QString outtag, vector < QString > tags, vector < double > multipliers )
{
   errormsg = "";
   
   for ( unsigned int i = 1; i < tags.size(); i++ )
   {
      if ( !compat(tags[0],tags[i]) )
      {
         return false;
      }
   }
      
   wave[outtag] = wave[tags[0]];
   wave[outtag].filename = "scalesum";

   for ( unsigned int i = 1; i < tags.size(); i++ )
   {
      for ( unsigned int j = 0; j < wave[outtag].q.size(); j++ )
      {
         wave[outtag].r[j] += wave[tags[i]].r[j] * multipliers[ i ];
         wave[outtag].s[j] += wave[tags[i]].s[j] * multipliers[ i ];
      }
   }

   return true;
}

void US_Saxs_Util::scale(QString outtag, QString intag, double multiplier)
{
   errormsg = "";
      
   wave[outtag] = wave[intag];
   wave[outtag].filename += QString("scaled by %1").arg(multiplier);

   for ( unsigned int j = 0; j < wave[outtag].q.size(); j++ )
   {
      wave[outtag].r[j] *= multiplier;
   }
}

bool US_Saxs_Util::crop(QString outtag, QString fromtag, double low, double high)
{
   errormsg = "";

   wave[outtag].clear( );
   wave[outtag].filename = QString("crop_%1_%2_%3").arg(fromtag).arg(low).arg(high);
   wave[outtag].header = wave[fromtag].header;
   wave[outtag].header_cols = wave[fromtag].header_cols;
   
   for ( unsigned int i = 0; i < wave[fromtag].q.size(); i++ )
   {
      if ( wave[fromtag].q[i] >= low &&
           wave[fromtag].q[i] <= high )
      {
         wave[outtag].q.push_back(wave[fromtag].q[i]);
         wave[outtag].r.push_back(wave[fromtag].r[i]);
         wave[outtag].s.push_back(wave[fromtag].s[i]);
      }
   }
   return true;
}

bool US_Saxs_Util::interpolate(QString outtag, QString totag, QString fromtag)
{
   // linearly interpolates the "from" q grid to the "to" qgrid returning in outtag
   // important note: "from" grid must cover "to" grid

   wave[outtag].clear( );
   wave[outtag].filename = QString("interpolate_%1_%2").arg(totag).arg(fromtag);
   wave[outtag].header = wave[fromtag].header;
   wave[outtag].header_cols = wave[fromtag].header_cols;
   wave[outtag].q = wave[totag].q;
   wave[outtag].r.resize(wave[totag].q.size());
   wave[outtag].s.resize(wave[totag].q.size());

   unsigned int p1 = 0;
   unsigned int p2;
   double pct;

   if ( debug > 3 ) 
   {
      for ( unsigned int i = 0; i < wave[fromtag].q.size(); i++ )
      {
         cout << 
            QString("US_Saxs_Util::interpolate from %1 q %2 r %3\n")
            .arg(i)
            .arg(wave[fromtag].q[i])
            .arg(wave[fromtag].r[i]);
      }
      for ( unsigned int i = 0; i < wave[totag].q.size(); i++ )
      {
         cout << 
            QString("US_Saxs_Util::interpolate to %1 q %2 r %3\n")
            .arg(i)
            .arg(wave[totag].q[i])
            .arg(wave[totag].r[i]);
      }
   }
      
   for( p2 = 0; p2 < wave[totag].q.size(); p2++ )
   {
      while ( wave[fromtag].q[p1] < wave[totag].q[p2] && p1 < wave[fromtag].q.size() ) 
      {
         p1++;
      }
      if ( p1 < 1 ) 
      {
         errormsg = "interpolate error 1";
         return false;
      }
      if ( wave[fromtag].q[p1 - 1] > wave[totag].q[p2] ) 
      {
         errormsg = "interpolate error 2";
         return false;
      }
      if ( p1 >= wave[fromtag].q.size() ) {
         errormsg = "interpolate error 3";
      }
      pct = (wave[totag].q[p2] - wave[fromtag].q[p1 - 1]) / (wave[fromtag].q[p1] - wave[fromtag].q[p1 - 1]);
      wave[outtag].r[p2] = pct * wave[fromtag].r[p1] + ( 1e0 - pct ) * wave[fromtag].r[p1 - 1];
      wave[outtag].s[p2] = pct * wave[fromtag].s[p1] + ( 1e0 - pct ) * wave[fromtag].s[p1 - 1];
   }
   return true;
}

bool US_Saxs_Util::interpolate( vector < double > & results,
                                vector < double > & to_r, 
                                vector < double > & from_r, 
                                vector < double > & from_pr )
{
   errormsg = "";

   vector < double > new_from_r;
   vector < double > new_from_pr;
   new_from_r.push_back(-1);
   new_from_pr.push_back(0);

   for ( int i = 0; i < (int) from_r.size(); i++ )
   {
      new_from_r.push_back(from_r[i]);
      new_from_pr.push_back(from_pr[i]);
   }
   new_from_r.push_back(1e99);
   new_from_pr.push_back(0);
   
   wave["from"].q = new_from_r;
   wave["from"].r = new_from_pr;
   wave["from"].s = new_from_pr;
   wave["to"].q = to_r;
   wave["to"].r = to_r;

   bool ok = interpolate( "out", "to", "from" );
   results = wave["out"].r;
   return ok;
}

double US_Saxs_Util::rmsd(QString tag1, QString tag2)
{
   errormsg = "";

   if ( !compat(tag1, tag2) )
   {
      errormsg = "incompatible vectors for rmsd calc";
      return 9e99;
   }

   double rmsd = 0e0;

   for ( unsigned int i = 0; i < wave[tag1].q.size(); i++ )
   {
      rmsd += ( wave[tag1].r[i] - wave[tag2].r[i] ) * ( wave[tag1].r[i] - wave[tag2].r[i] );
   }
   return sqrt(rmsd / wave[tag1].q.size());
}

bool US_Saxs_Util::join(QString outtag, QString tag1, QString tag2, double pt)
{
   errormsg = "";
   wave[outtag].clear( );
   wave[outtag].filename = QString("join_%1_%2_%3").arg(tag1).arg(tag2).arg(pt);
   wave[outtag].header = wave[tag1].header;
   
   for( unsigned int i = 0; i < wave[tag1].q.size(); i++ )
   {
      if ( wave[tag1].q[i] <= pt )
      {
         wave[outtag].q.push_back(wave[tag1].q[i]);
         wave[outtag].r.push_back(wave[tag1].r[i]);
         wave[outtag].s.push_back(wave[tag1].s[i]);
      }
   }

   for( unsigned int i = 0; i < wave[tag2].q.size(); i++ )
   {
      if ( wave[tag2].q[i] > pt )
      {
         wave[outtag].q.push_back(wave[tag2].q[i]);
         wave[outtag].r.push_back(wave[tag2].r[i]);
         wave[outtag].s.push_back(wave[tag2].s[i]);
      }
   }
   return true;
}

bool US_Saxs_Util::normalize( QString outtag, QString tag )
{
   errormsg = "";

   wave[outtag] = wave[tag];
   wave[outtag].filename = QString("normalize_%1").arg(tag);

   double sum = 0e0;

   for ( unsigned int i = 0; i < wave[tag].r.size(); i++ )
   {
      sum += wave[tag].r[i];
   }

   if ( sum == 0e0 )
   {
      errormsg = QString("US_Saxs_Util::normalize %1 has zero sum").arg(tag);
      return false;
   }

   double oneoversum = 1e0 / sum;

   for ( unsigned int i = 0; i < wave[tag].r.size(); i++ )
   {
      wave[outtag].r[i] *= oneoversum;
   }
   return true;
}

bool US_Saxs_Util::subbackground(QString outtag, QString solutiontag, QString buffertag, double alpha)
{
   errormsg = "";

   wave[outtag].clear( );
   wave[outtag].filename = QString("sbg_%1_%2_%3")
      .arg(solutiontag).arg(buffertag).arg(alpha);
   wave[outtag].header = wave[solutiontag].header;
   wave[outtag].header_cols = wave[solutiontag].header_cols;
   wave[outtag].q = wave[solutiontag].q;
   wave[outtag].r.resize(wave[solutiontag].q.size());
   wave[outtag].s.resize(wave[solutiontag].q.size());

   if ( !compat(solutiontag, buffertag) )
   {
      return false;
   }
   for ( unsigned int i = 0; i < wave[solutiontag].q.size(); i++ )
   {
      wave[outtag].r[i] = wave[solutiontag].r[i] - alpha * wave[buffertag].r[i];
      wave[outtag].s[i] = wave[solutiontag].s[i] - alpha * wave[buffertag].s[i];
   }
   return true;
}

bool US_Saxs_Util::subcellwave(QString outtag, QString solutiontag, QString buffertag, QString emptytag, double alpha)
{
   errormsg = "";

   wave[outtag].clear( );
   wave[outtag].filename = QString("scw_%1_%2_%3").arg(solutiontag).arg(buffertag).arg(alpha);
   wave[outtag].header = wave[solutiontag].header;
   wave[outtag].header_cols = wave[solutiontag].header_cols;
   wave[outtag].q = wave[solutiontag].q;
   wave[outtag].r.resize(wave[solutiontag].q.size());
   wave[outtag].s.resize(wave[solutiontag].q.size());

   if ( !compat(solutiontag, buffertag) ||
        !compat(solutiontag, emptytag) )
   {
      return false;
   }

   for ( unsigned int i = 0; i < wave[solutiontag].q.size(); i++ )
   {
      wave[outtag].r[i] = wave[solutiontag].r[i] - alpha * wave[buffertag].r[i] - ( 1e0 - alpha ) * wave[emptytag].r[i];
      wave[outtag].s[i] = wave[solutiontag].s[i] - alpha * wave[buffertag].s[i] - ( 1e0 - alpha ) * wave[emptytag].s[i];
   }
   return true;
}

bool US_Saxs_Util::waxsguidedsaxsbuffersub(double &nrmsd,
                                           QString outtag, 
                                           QString solutiontag, 
                                           QString buffertag, 
                                           QString waxstag, 
                                           double alpha,
                                           double beta,
                                           double dconst,
                                           double low,
                                           double high,
                                           QString &waxscrop,
                                           QString &waxscropinterp,
                                           bool prechecked,
                                           bool waxsprecropinterp
                                           )
{
   errormsg = "";

   // make outtag 

   wave[outtag].clear( );
   wave[outtag].filename = 
      QString("wgsbs_%1_%2_%3_%4_%5")
      .arg(solutiontag)
      .arg(buffertag)
      .arg(alpha)
      .arg(beta)
      .arg(dconst);
   wave[outtag].header = wave[solutiontag].header;
   wave[outtag].header_cols = wave[solutiontag].header_cols;
   wave[outtag].q = wave[solutiontag].q;
   wave[outtag].r.resize(wave[solutiontag].q.size());
   wave[outtag].s.resize(wave[solutiontag].q.size());

   if ( !prechecked &&
        !compat(solutiontag, buffertag) )
   {
      return false;
   }

   for ( unsigned int i = 0; i < wave[solutiontag].q.size(); i++ )
   {
      wave[outtag].r[i] = beta * ( wave[solutiontag].r[i] - alpha * wave[buffertag].r[i] - dconst );
      wave[outtag].s[i] = beta * ( wave[solutiontag].s[i] - alpha * wave[buffertag].s[i] - dconst );
   }

   // crop to comparison range

   QString outcrop = outtag + "_crop";
   if ( !waxsprecropinterp )
   {
      waxscrop = waxstag + "_crop";
   }

   if ( !crop( outcrop, solutiontag, low, high ) || 
        !( waxsprecropinterp || crop( waxscrop, waxstag, low, high ) ) )
   {
      return false;
   }

   // interpolate to grids for rmsd calc

   QString outcropinterp = outtag + "_crop_interp";
   waxscropinterp = waxstag + "_crop_interp";

   if ( !interpolate( outcropinterp, waxscrop, outtag ) || 
        !( waxsprecropinterp || interpolate( waxscropinterp, outcrop, waxstag ) ) )
   {
      return false;
   }

   // compute rmsd
   nrmsd = 
      ( rmsd(outcropinterp, waxscrop) + rmsd(waxscropinterp, outcrop) ) /
      ( sqrt(2e0) * 
        ( ( wave[waxscrop].q[wave[waxscrop].q.size() - 1] - wave[waxscrop].q[0] ) + 
          ( wave[outcrop].q[wave[outcrop].q.size() - 1] - wave[outcrop].q[0] ) / 2e0 ) );
   return true;
}


bool US_Saxs_Util::waxsguidedsaxsbuffersubgridsearch(double &nrmsdmin,
                                                     double &alphamin,
                                                     double &betamin,
                                                     double &dconstmin,
                                                     QString &log,
                                                     QString outtag, 
                                                     QString solutiontag, 
                                                     QString buffertag, 
                                                     QString waxstag, 
                                                     double alphalow,
                                                     double alphahigh,
                                                     double alphainc,
                                                     double betalow,
                                                     double betahigh,
                                                     double betainc,
                                                     double dconstlow,
                                                     double dconsthigh,
                                                     double dconstinc,
                                                     double low,
                                                     double high
                                                     )
{
   errormsg = "";

   if ( !alphainc )
   {
      alphainc = 1e99;
   }
   if ( !betainc )
   {
      betainc = 1e99;
   }
   if ( !dconstinc )
   {
      dconstinc = 1e99;
   }

   if ( debug )
   {
      cout << 
         QString("waxsguidedsaxsbuffersubgridsearch %1 %2 %3 %4 %5 %6 %7 %8 %9\n")
         .arg(alphalow)
         .arg(alphahigh)
         .arg(alphainc)
         .arg(betalow)
         .arg(betahigh)
         .arg(betainc)
         .arg(dconstlow)
         .arg(dconsthigh)
         .arg(dconstinc);
   }

   double alpha;
   double beta;
   double dconst;

   QString waxscrop = "";
   QString waxscropinterp = "";

   double nrmsd;
   nrmsdmin = 1e99;

   bool prechecked = false;
   bool waxsprecropinterp = false;

   log = "rmsd|alpha|beta|dconst|low|high\n";
   
   unsigned int count = 0;

   for ( alpha = alphalow; alpha <= alphahigh; alpha += alphainc )
   {
      for ( beta = betalow; beta <= betahigh; beta += betainc )
      {
         for ( dconst = dconstlow; dconst <= dconsthigh; dconst += dconstinc )
         {
            if ( debug )
            {
               cout << QString("trying alpha %1 beta %2 const %3\n").arg(alpha).arg(beta).arg(dconst);
            }

            if ( !waxsguidedsaxsbuffersub(nrmsd,
                                          outtag, 
                                          solutiontag, 
                                          buffertag, 
                                          waxstag, 
                                          alpha,
                                          beta,
                                          dconst,
                                          low,
                                          high,
                                          waxscrop,
                                          waxscropinterp,
                                          prechecked,
                                          waxsprecropinterp
                                          ) )
            {
               cout << "waxsguidedsaxsbuffersub returned false\n";
               return false;
            }
            prechecked = true;
            waxsprecropinterp = true;

            if ( nrmsd < nrmsdmin )
            {
               nrmsdmin = nrmsd;
               alphamin = alpha;
               betamin = beta;
               dconstmin = dconst;
               wave[outtag + ":best"] = wave[outtag];
            }
            if ( debug )
            {
               cout << 
                  QString("%1|%2|%3|%4|%5|%6\n")
                  .arg(nrmsd)
                  .arg(alpha)
                  .arg(beta)
                  .arg(dconst)
                  .arg(low)
                  .arg(high);
            }
            log += 
               QString("%1|%2|%3|%4|%5|%6\n")
               .arg(nrmsd)
               .arg(alpha)
               .arg(beta)
               .arg(dconst)
               .arg(low)
               .arg(high);
            if ( !(count++ % 256 ) )
            {
               cout << '.' << flush;
            }
         }
      }
   }
   cout << endl;
   wave[outtag] = wave[outtag + ":best"];
   log += 
      QString("minimum %1|%2|%3|%4|%5|%6\n")
      .arg(nrmsdmin)
      .arg(alphamin)
      .arg(betamin)
      .arg(dconstmin)
      .arg(low)
      .arg(high);
   return true;
}

bool US_Saxs_Util::setup_wgsbs_gsm_f_df(
                                        QString outtag,
                                        QString solutiontag, 
                                        QString buffertag, 
                                        QString waxstag, 
                                        double alphalow,
                                        double alphahigh,
                                        double alphainc,
                                        double betalow,
                                        double betahigh,
                                        double betainc,
                                        double dconstlow,
                                        double dconsthigh,
                                        double dconstinc,
                                        double low,
                                        double high
                                        ) {

   errormsg = "";

   QString waxscrop = waxstag + "_crop";
   QString outcrop = solutiontag + "_crop";
   if ( !crop( outcrop, solutiontag, low, high ) || 
        !crop( waxscrop, waxstag, low, high ) )
   {
      return false;
   }

   QString waxscropinterp = waxstag + "_crop_interp";

   if ( !interpolate( waxscropinterp, outcrop, waxstag ) )
   {
      return false;
   }

   wave[outcrop].clear( );

   gsm_outtag = outtag;
   gsm_solutiontag = solutiontag;
   gsm_buffertag = buffertag;
   gsm_waxstag = waxstag;

   gsm_alphalow = alphalow;
   gsm_alphahigh = alphahigh;
   gsm_alphainc = alphainc;

   if ( gsm_alphalow == gsm_alphahigh ||
        gsm_alphainc == 0e0 )
   {
      gsm_alpha2_r = 0e0;
   } else {
      gsm_alpha2_r = 1e0 / ( 2e0 * gsm_alphainc );
   }

   gsm_betalow = betalow;
   gsm_betahigh = betahigh;
   gsm_betainc = betainc;

   if ( gsm_betalow == gsm_betahigh ||
        gsm_betainc == 0e0 )
   {
      gsm_beta2_r = 0e0;
   } else {
      gsm_beta2_r = 1e0 / ( 2e0 * gsm_betainc );
   }

   gsm_dconstlow = dconstlow;
   gsm_dconsthigh = dconsthigh;
   gsm_dconstinc = dconstinc;

   if ( gsm_dconstlow == gsm_dconsthigh ||
        gsm_dconstinc == 0e0 )
   {
      gsm_dconst2_r = 0e0;
   } else {
      gsm_dconst2_r = 1e0 / ( 2e0 * gsm_dconstinc );
   }

   gsm_low = low;
   gsm_high = high;
   gsm_waxscrop = waxscrop;
   gsm_waxscropinterp = waxscropinterp;
   wgsbs_gsm_setup = true;
   return true;
}

bool US_Saxs_Util::wgsbs_gsm(
                             double &nrmsd,
                             double &alpha,
                             double &beta,
                             double &dconst,
                             int gsm_method,
                             long max_iterations
                             )
{
   errormsg = "";
   if ( !wgsbs_gsm_setup )
   {
      errormsg = "wgsbs_gsm not setup, call US_Saxs_Util::setup_wgsbs_gsm_f_df() first";
      return false;
   }

   our_vector *v = new_our_vector(3);
   v->d[0] = alpha;
   v->d[1] = beta;
   v->d[2] = dconst;
   switch(gsm_method) {
   case CONJUGATE_GRADIENT :
      min_fr_pr_cgd(v, EPS_DEFAULT, max_iterations); 
      break;
   case STEEPEST_DESCENT :      
      min_gsm_5_1(v, EPS_DEFAULT, max_iterations); 
      break;
   case INVERSE_HESSIAN :  
      min_hessian_bfgs(v, EPS_DEFAULT, max_iterations); 
      break;
   default : 
      {
         errormsg = "unknown gsm method"; 
         free_our_vector(v); 
         return false; 
      }
      break;
   }
   // one extra to make sure best was last and to get nrmsd
   alpha = v->d[0];
   beta = v->d[1];
   dconst = v->d[2];
   nrmsd = wgsbs_gsm_f(v);
   free_our_vector(v);
   return true;
}

double US_Saxs_Util::wgsbs_gsm_f(our_vector *v) {
   
   if ( v->d[0] < gsm_alphalow )
   {
      v->d[0] = gsm_alphalow;
   } else {
      if ( v->d[0] > gsm_alphahigh )
      {
         v->d[0] = gsm_alphahigh;
      }
   }
   if ( v->d[1] < gsm_betalow )
   {
      v->d[1] = gsm_betalow;
   } else {
      if ( v->d[1] > gsm_betahigh )
      {
         v->d[1] = gsm_betahigh;
      }
   }
   if ( v->d[2] < gsm_dconstlow )
   {
      v->d[2] = gsm_dconstlow;
   } else {
      if ( v->d[2] > gsm_dconsthigh )
      {
         v->d[2] = gsm_dconsthigh;
      }
   }
         
   double nrmsd;
   if ( !waxsguidedsaxsbuffersub(nrmsd,
                                 gsm_outtag,
                                 gsm_solutiontag, 
                                 gsm_buffertag, 
                                 gsm_waxstag, 
                                 v->d[0],
                                 v->d[1],
                                 v->d[2],
                                 gsm_low,
                                 gsm_high,
                                 gsm_waxscrop,
                                 gsm_waxscropinterp,
                                 true,
                                 true
                                 ) )
   {
      cout << "waxsguidedsaxsbuffersub returned false\n";
      cout << errormsg << endl;
      return 1e99;
   }
   
   return nrmsd;
}

void US_Saxs_Util::wgsbs_gsm_df(our_vector *vd, our_vector *v) 
{
   double y0;
   double y2;
   double sav_ve;

   // compute partials for each our_vector element

   if ( gsm_alpha2_r )
   {
      sav_ve = v->d[0];
      v->d[0] -= gsm_alphainc;
      y0 = wgsbs_gsm_f(v);
      v->d[0] = sav_ve + gsm_alphainc;
      y2 = wgsbs_gsm_f(v);
      vd->d[0] = (y2 - y0) * gsm_alpha2_r;
      v->d[0] = sav_ve;
   } else {
      vd->d[0] = 0e0;
   }

   if ( gsm_beta2_r )
   {
      sav_ve = v->d[1];
      v->d[1] -= gsm_betainc;
      y0 = wgsbs_gsm_f(v);
      v->d[1] = sav_ve + gsm_betainc;
      y2 = wgsbs_gsm_f(v);
      vd->d[1] = (y2 - y0) * gsm_beta2_r;
      v->d[1] = sav_ve;
   } else {
      vd->d[1] = 0e0;
   }

   if ( gsm_dconst2_r )
   {
      sav_ve = v->d[2];
      v->d[2] -= gsm_dconstinc;
      y0 = wgsbs_gsm_f(v);
      v->d[2] = sav_ve + gsm_dconstinc;
      y2 = wgsbs_gsm_f(v);
      vd->d[2] = (y2 - y0) * gsm_dconst2_r;
      v->d[2] = sav_ve;
   } else {
      vd->d[2] = 0e0;
   }
   if ( debug > 1 )
   {
      cout << QString("gradient: %1 %2 %3\n").arg(vd->d[0]).arg(vd->d[1]).arg(vd->d[2]);
   }
}

/* gsm stuff */
our_matrix *US_Saxs_Util::new_our_matrix(int rows, int cols) {
   our_matrix *m;
   if((m = (our_matrix *)malloc(sizeof(our_matrix))) == NULL) {
      fputs("new_our_matrix malloc error\n", stderr);
      exit(-1);
   }
   m->rows = rows;
   m->cols = cols;
   if((m->d = (double *)malloc(sizeof(double) * rows * cols)) == NULL) {
      fputs("new_our_matrix malloc error\n", stderr);
      exit(-1);
   }
   return(m);
}

void US_Saxs_Util::free_our_matrix(our_matrix *m) {
   free(m->d);
   free(m);
}

void US_Saxs_Util::set_our_matrix(our_matrix *m, double s) {
   int i, limit;
   limit = m->rows * m->cols;
   /* could do this with ceil(log2(len)) memcpy()s */
   for(i = 0; i < limit; i++) {
      m->d[i] = s;
   }
}

void US_Saxs_Util::identity_our_matrix(our_matrix *m) {
   int i;
   set_our_matrix(m, 0e0);

   for(i = 0; i < m->rows; i++) {
      m->d[i * (m->cols + 1)] = 1e0;
   }
}

void US_Saxs_Util::print_our_matrix(our_matrix *m) {
   int i, j;

   for(i = 0; i < m->rows; i++) {
      printf("%d: %d:", this_rank, i);
      for(j = 0; j <= m->cols; j++) {
         printf(" %.6g", m->d[(i * m->cols) + j]);
      }
      puts("");
   }
}

our_vector *US_Saxs_Util::new_our_vector(int len) {
   our_vector *v;
   if((v = (our_vector *)malloc(sizeof(our_vector))) == NULL) {
      fputs("new_our_vector malloc error\n", stderr);
      exit(-1);
   }
   v->len = len;
   if((v->d = (double *)malloc(sizeof(double) * len)) == NULL) {
      fputs("new_our_vector malloc error\n", stderr);
      exit(-1);
   }
   return(v);
}

void US_Saxs_Util::free_our_vector(our_vector *v) {
   free(v->d);
   free(v);
}

void US_Saxs_Util::print_our_vector(our_vector *v) {
   int i;

   for(i = 0; i < v->len; i++) {
      printf("%.8g ", v->d[i]);
   }
   puts("");
}

double US_Saxs_Util::l2norm_our_vector(our_vector *v1, our_vector *v2) {
   double norm = 0e0;
   int i;
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len) {
      fputs("l2norm_our_vector incompatable", stderr);
      exit(-1);
   }
#endif
   for(i = 0; i < v1->len; i++) {
      norm += pow(v1->d[i] - v2->d[i], 2e0);
   }
   norm = sqrt(norm);
   return(norm);
}

void US_Saxs_Util::copy_our_vector(our_vector *v1, our_vector *v2) {
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len) {
      fputs("copy_our_vector incompatable", stderr);
      exit(-1);
   }
#endif
   memcpy(v1->d, v2->d, sizeof(double) * v1->len);
}

void US_Saxs_Util::set_our_vector(our_vector *v1, double s) {
   int i;
   /* could do this with ceil(log2(len)) memcpy()s */
   for(i = 0; i < v1->len; i++) {
      v1->d[i] = s;
   }
}

void US_Saxs_Util::add_our_vector_vv(our_vector *v1, our_vector *v2) {
   int i;
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len) {
      fputs("add_our_vector_vv incompatable", stderr);
      exit(-1);
   }
#endif
   for(i = 0; i < v1->len; i++) {
      v1->d[i] += v2->d[i];
   }
}

void US_Saxs_Util::add_our_vector_vs(our_vector *v1, double s) {
   int i;
   for(i = 0; i < v1->len; i++) {
      v1->d[i] += s;
   }
}

void US_Saxs_Util::mult_our_vector_vv(our_vector *v1, our_vector *v2) {
   int i;
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len) {
      fputs("mult_our_vector_vv incompatable", stderr);
      exit(-1);
   }
#endif
   for(i = 0; i < v1->len; i++) {
      v1->d[i] *= v2->d[i];
   }
}

void US_Saxs_Util::mult_our_vector_vs(our_vector *v1, double s) {
   int i;
   for(i = 0; i < v1->len; i++) {
      v1->d[i] *= s;
   }
}

void US_Saxs_Util::add_our_vector_vvv(our_vector *vd, our_vector *v1, our_vector *v2) {
   int i;
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len || vd->len != v1->len) {
      fputs("add_our_vector_vvv incompatable", stderr);
      exit(-1);
   }
#endif
   for(i = 0; i < v1->len; i++) {
      vd->d[i] = v1->d[i] + v2->d[i];
   }
}

void US_Saxs_Util::add_our_vector_vvs(our_vector *vd, our_vector *v1, double s) {
   int i;
   for(i = 0; i < v1->len; i++) {
      vd->d[i] = v1->d[i] + s;
   }
}

void US_Saxs_Util::sub_our_vector_vvv(our_vector *vd, our_vector *v1, our_vector *v2) {
   int i;
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len || vd->len != v1->len) {
      fputs("add_our_vector_vvv incompatable", stderr);
      exit(-1);
   }
#endif
   for(i = 0; i < v1->len; i++) {
      vd->d[i] = v1->d[i] - v2->d[i];
   }
}

void US_Saxs_Util::sub_our_vector_vvs(our_vector *vd, our_vector *v1, double s) {
   int i;
   for(i = 0; i < v1->len; i++) {
      vd->d[i] = v1->d[i] - s;
   }
}

void US_Saxs_Util::mult_our_vector_vvv(our_vector *vd, our_vector *v1, our_vector *v2) {
   int i;
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len || vd->len != v1->len) {
      fputs("mult_our_vector_vvv incompatable", stderr);
      exit(-1);
   }
#endif
   for(i = 0; i < v1->len; i++) {
      vd->d[i] = v1->d[i] * v2->d[i];
   }
}

void US_Saxs_Util::mult_our_vector_vvs(our_vector *vd, our_vector *v1, double s) {
   int i;
   for(i = 0; i < v1->len; i++) {
      vd->d[i] = v1->d[i] * s;
   }
}

double US_Saxs_Util::dot_our_vector(our_vector *v1, our_vector *v2) {
   int i;
   double ret = 0e0;
   for(i = 0; i < v1->len; i++) {
      ret += v1->d[i] * v2->d[i];
   }
   return(ret);
}


void US_Saxs_Util::mult_our_matrix_vmv(our_vector *vd, our_matrix *m, our_vector *vs) {
   int i, j;

#if !defined(SUPRESS_ERROR_CHECKING)
   if(vs->len != m->cols || vd->len != m->rows) {
      fputs("mult_our_matrix_vmv incompatable", stderr);
      exit(-1);
   }
#endif

   set_our_vector(vd, 0e0);
   for(i = 0; i < m->rows; i++) {
      for(j = 0; j < m->cols; j++) {
         vd->d[i] += m->d[(i * m->cols) + j] * vs->d[j];
      }
   }
}

void US_Saxs_Util::our_vector_test() {
   our_vector *v1, *v2, *v3;

   v1 = new_our_vector(16);
   v2 = new_our_vector(16);
   v3 = new_our_vector(16);
  
   set_our_vector(v1, 0);
   set_our_vector(v2, 1e0);
   mult_our_vector_vvs(v3, v2, 2.5e-1);
   print_our_vector(v1);
   print_our_vector(v2);
   print_our_vector(v3);
   printf("%d: %.12g %.12g %.12g\n", this_rank, l2norm_our_vector(v1, v2), l2norm_our_vector(v1, v3), l2norm_our_vector(v2,v3));
}

long US_Saxs_Util::min_gsm_5_1(our_vector *i, double epsilon, long max_iter) {
   /* try to find a local minimum via a gradient search method */
#if defined(SHOW_TIMING)
   struct timeval tv1, tv2;
#endif
   our_vector *u, *zero;
   //  double t;
   double s1, s2, s3;
   double g_s1, g_s2, g_s3, g_s4;
   our_vector *v_s1, *v_s2, *v_s3, *v_tmp, *v_s4;
   double a, b, x;
   double s1_s2, s1_s3, s2_s3;
   double s1_2, s2_2, s3_2;
   // double prev_s2;
   double prev_g_s2;
   double fitness;
   long iter = 0l;
   int reps, last_reps = 0;
   //  int j;

   //  printf("conjugate gradient initial position: ");
   //  print_our_vector(i); 

   zero = new_our_vector(i->len);
   set_our_vector(zero, 0e0);

   u = new_our_vector(i->len);
   wgsbs_gsm_df(u, i);
   /*  mult_our_vector_vs(u, 1e0 / l2norm_our_vector(u, zero)); */

#if defined(DEBUG_GSM)
   printf("initial gradient ||=%.12g : ", l2norm_our_vector(u, zero)); 
   print_our_vector(u); 
#endif
  
   v_s1 = new_our_vector(i->len);
   v_s2 = new_our_vector(i->len);
   v_s3 = new_our_vector(i->len);
   v_s4 = new_our_vector(i->len);

   //  printf("norm %g\n", l2norm_our_vector(u, zero));

   while(l2norm_our_vector(u, zero) >= epsilon && iter++ < max_iter) {
#if defined(DEBUG_GSM)
      printf("begin\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif
      printf("%d: global %ld, iter %ld fitness %.12g |grad|=%.12g last reps %d\n", this_rank, global_iter++, 
             iter, fitness = wgsbs_gsm_f(i), l2norm_our_vector(u, zero), last_reps); fflush(stdout);
      printf("%d: ", this_rank);
      print_our_vector(i);
      fflush(stdout);
      if(!fitness) {
         free_our_vector(v_s1);
         free_our_vector(v_s2);
         free_our_vector(v_s3);
         free_our_vector(v_s4);
         free_our_vector(zero);
         free_our_vector(u);
         return(0);
      }
      /*    printf("i : ");
            print_our_vector(i); */
      /* find minimum of wgsbs_gsm_f(i - t u) */
      /* alg 5_2 */

      s1 = 0e0;
      copy_our_vector(v_s1, i);
      g_s1 = fitness;

      s3 = 1e0;
      
      s2 = 5e-1;
      mult_our_vector_vvs(v_s2, u, -s2);
      /*    printf("v_s2 after mult_our_vector_vvs : ");
            print_our_vector(v_s2); */
      /*    printf("i : "); */
      add_our_vector_vv(v_s2, i);
      g_s2 = wgsbs_gsm_f(v_s2);

      /* cut down interval until we have a decrease */
      printf("%d: decrease\n", this_rank); 
      fflush(stdout);
#if defined(SHOW_TIMING)
      gettimeofday(&tv1, NULL);
#endif

      while(g_s2 > g_s1 && s2 - s1 > epsilon) {
         s3 = s2;
         s2 *= 5e-1;
         mult_our_vector_vvs(v_s2, u, -s2);
         add_our_vector_vv(v_s2, i);
         g_s2 = wgsbs_gsm_f(v_s2);
      }

      if(s2 - s1 <= epsilon || s3 - s2 <= epsilon) {
         /* ugh, no decrease */
         printf("%d: no initial decrease, terminating pos = ", this_rank);
         /*      print_our_vector(i); */
         free_our_vector(v_s1);
         free_our_vector(v_s2);
         free_our_vector(v_s3);
         free_our_vector(v_s4);
         free_our_vector(zero);
         free_our_vector(u);
         return(max_iter - iter);
      }

      mult_our_vector_vvs(v_s3, u,-s3);
      add_our_vector_vv(v_s3, i);
      g_s3 = wgsbs_gsm_f(v_s3);


#if defined(DEBUG_GSM)
      printf("pre\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3); 
#endif

    
      reps = 0;
#if defined(SHOW_TIMING)
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
#endif
      printf("%d: start line search\n", this_rank); 
      fflush(stdout);
#if defined(SHOW_TIMING)
      gettimeofday(&tv1, NULL);
#endif
      while(s2 - s1 > epsilon && s3 - s2 > epsilon && reps++ < MAX_REPS) {
      
#if defined(DEBUG_GSM)
         printf("start\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif

         s1_s2 = 1e0 / (s1 - s2);
         s1_s3 = 1e0 / (s1 - s3);
         s2_s3 = 1e0 / (s2 - s3);

         s1_2 = s1 * s1;
         s2_2 = s2 * s2;
         s3_2 = s3 * s3;

         a = ((g_s1 - g_s3) * s1_s3 - (g_s2 - g_s3) * s2_s3) * s1_s2;

         /* printf("new a = %.12g\n",a); */
         b = (g_s3 * (s2_2 - s1_2) + g_s2 * (s1_2 - s3_2) + g_s1 * (s3_2 - s2_2)) *
            s1_s2 * s1_s3 * s2_s3;

         /* printf("new b = %.12g\n",b); */

         if(fabs(a) < MIN_A) {
            /* maybe we should switch to a bisection method? */
            printf("%d: a limit reached\n", this_rank);
            printf("%d: done iter %ld, i = ", this_rank, iter); 
            print_our_vector(i); fflush(stdout);
            free_our_vector(v_s1);
            free_our_vector(v_s2);
            free_our_vector(v_s3);
            free_our_vector(v_s4);
            free_our_vector(zero);
            free_our_vector(u);
            return(max_iter - iter);
         }

         x = -b / (2e0 * a);

         prev_g_s2 = g_s2;
         // prev_s2 = s2;

#if defined(DEBUG_GSM)
         printf("new x = %.12g\n", x);
#endif

         if(x < s1) {
#if defined(DEBUG_GSM)
            printf("p1 ");
#endif
            if(x < (s1 + s1 - s2)) { /* keep it close */
               x = s1 + s1 - s2;
               if(x < 0) {
                  x = s1 / 2;
               }
            }
            if(x < 0) { /* ugh we're in the wrong direction! */
               printf("%d: unexpected minimum pos %.12g\n", this_rank, x);
               exit(-1);
            } 
            /* ok, take x, s1, s2 */
            v_tmp = v_s3;
            v_s3 = v_s2;
            g_s3 = g_s2;
            s3 = s2;
            v_s2 = v_s1;
            g_s2 = g_s1;
            s2 = s1;
            v_s1 = v_tmp;
   
            s1 = x;
            mult_our_vector_vvs(v_s1, u, -s1);
            add_our_vector_vv(v_s1, i);
            g_s1 = wgsbs_gsm_f(v_s1);
         } else {
            if(x < s2) {
#if defined(DEBUG_GSM)
               printf("p2 ");
#endif
               /* ok, take s1, x, s2 */
               v_tmp = v_s3;
               v_s3 = v_s2;
               g_s3 = g_s2;
               s3 = s2;
               v_s2 = v_tmp;
   
               s2 = x;
               /*     printf("x = %.12g\n", x); */
               mult_our_vector_vvs(v_s2, u, -s2);
               add_our_vector_vv(v_s2, i);
               g_s2 = wgsbs_gsm_f(v_s2);
            } else {
               if(x < s3) {
#if defined(DEBUG_GSM)
                  printf("p3 ");
#endif
                  /* ok, take s2, x, s3 */
                  v_tmp = v_s1;
                  v_s1 = v_s2;
                  g_s1 = g_s2;
                  s1 = s2;
                  v_s2 = v_tmp;
   
                  s2 = x;
                  mult_our_vector_vvs(v_s2, u, -s2);
                  add_our_vector_vv(v_s2, i);
                  g_s2 = wgsbs_gsm_f(v_s2);
               } else {
                  /* ugh x >= s3.. well why not? */
                  if(x > (s3 + s3 - s2)) { /* keep it close */
                     mult_our_vector_vvs(v_s4, u, -x);
                     add_our_vector_vv(v_s4, i);
                     g_s4 = wgsbs_gsm_f(v_s4);
                     if(g_s4 > g_s2 &&
                        g_s4 > g_s3 &&
                        g_s4 > g_s1) {
                        x = (s3 + s3 - s2);
                     }
                  }
                  /* take s2, s3, x */
#if defined(DEBUG_GSM)
                  printf("p4 ");
#endif
       
                  v_tmp = v_s1;
                  v_s1 = v_s2;
                  g_s1 = g_s2;
                  s1 = s2;
                  v_s2 = v_s3;
                  g_s2 = g_s3;
                  s2 = s3;
                  v_s3 = v_tmp;
   
                  s3 = x;
                  mult_our_vector_vvs(v_s3, u, -s3);
                  add_our_vector_vv(v_s3, i);
                  g_s3 = wgsbs_gsm_f(v_s3);
               }
            }
         }
      
         if(fabs(prev_g_s2 - g_s2) < epsilon) {
            printf("%d: fabs(g-prev) < epsilon\n", this_rank); fflush(stdout);
            break;
         }
         /*      puts(""); */
      }
      last_reps = reps;
      /*    printf("v_s2 ");
            print_our_vector(v_s2); */

      if(g_s2 < g_s3 && g_s2 < g_s1) {
         copy_our_vector(i, v_s2);
         g_s4 = g_s2;
      } else {
         if(g_s1 < g_s3) {
            copy_our_vector(i, v_s1);
            g_s4 = g_s1;
         } else {
            copy_our_vector(i, v_s3);
            g_s4 = g_s3;
         }
      }

      if(evenn) {
         switch(queries) {
         case 8 : i->d[(6 * N_2) - 1] = 0; [[fallthrough]];
         case 7 : [[fallthrough]];
         case 6 : i->d[(4 * N_2) - 1] = 0; [[fallthrough]];
         case 5 : [[fallthrough]];
         case 4 : i->d[(2 * N_2) - 1] = 0; [[fallthrough]];
         default : break;
         }
      }
#if defined(SHOW_TIMING)
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
#endif
      printf("%d: df start\n", this_rank); 
      fflush(stdout);
      wgsbs_gsm_df(u, i);
      /*    mult_our_vector_vs(u, 1e0 / l2norm_our_vector(u, zero)); */
#if defined(DEBUG_GSM)
      printf("end\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif

      if(!(global_iter % 5)) {
         //      int r, s, j;
      
         printf("%d: query4i_intermediate|alg4|%s|%d|%.12g|%d|%d|%d|%d|%d|%.12g|%.12g|%.12g|%.12g|%ld|%d|%d|%d|%.12g|%ld|%ld", this_rank,
                (wgsbs_gsm_f(i) <= 0e0) ? "success" : "fail",
                N, 
                wgsbs_gsm_f(i),
                queries,
                fitness_type,
                0,
                this_iterations + prev_iterations, 
                go, 
                0e0,
                0e0,
                0e0,
                0e0,
                iterations_max,
                0,
                0,
                0,
                min_acceptable,
                0L, 0L
                ); 
         puts(""); 
      }
   }

   printf("%d: done iter %ld, i = ", this_rank, iter);
   print_our_vector(i);
   free_our_vector(v_s1);
   free_our_vector(v_s2);
   free_our_vector(v_s3);
   free_our_vector(v_s4);
   free_our_vector(zero);
   free_our_vector(u);
   return(0);
}

long US_Saxs_Util::min_fr_pr_cgd(our_vector *i, double epsilon, long max_iter) {
   /* polak-ribiere version of fletcher-reeves conjugate gradient  */

#if defined(SHOW_TIMING)
   struct timeval tv1, tv2;
#endif
   our_vector *u, *zero;
   //  double t;
   double s1, s2, s3;
   double g_s1, g_s2, g_s3, g_s4;
   our_vector *v_s1, *v_s2, *v_s3, *v_tmp, *v_s4, *v_h, *v_g;
   double a, b, x;
   double s1_s2, s1_s3, s2_s3;
   double s1_2, s2_2, s3_2;
   // double prev_s2;
   double prev_g_s2;
   long iter = 0l;
   int reps, last_reps = 0;
   double gg, ggd;
   double fitness;

   /*  printf("initial position: ");
       print_our_vector(i); */

   puts("fr.1");
   zero = new_our_vector(i->len);
   set_our_vector(zero, 0e0);

   u = new_our_vector(i->len);
   wgsbs_gsm_df(u, i);
   /*  mult_our_vector_vs(u, 1e0 / l2norm_our_vector(u, zero)); */

   /*  printf("initial gradient ||=%.12g : ", l2norm_our_vector(u, zero));
       print_our_vector(u); */
  
   v_s1 = new_our_vector(i->len);
   v_s2 = new_our_vector(i->len);
   v_s3 = new_our_vector(i->len);
   v_s4 = new_our_vector(i->len);
   v_h = new_our_vector(i->len);
   v_g = new_our_vector(i->len);

   copy_our_vector(v_g, u);
   copy_our_vector(v_h, u);

   while(l2norm_our_vector(u, zero) >= epsilon && iter++ < max_iter) {
      this_iterations++;
      printf("%d: global %ld, iter %ld fitness %.12g |grad|=%.12g last reps %d\n", this_rank, global_iter++, 
             iter, fitness = wgsbs_gsm_f(i), l2norm_our_vector(u, zero), last_reps); fflush(stdout);
      printf("%d: ", this_rank);
      print_our_vector(i);
#if defined(DEBUG_GSM)
      print_our_vector(u);
#endif
      fflush(stdout);
      if(!fitness) {
         free_our_vector(v_s1);
         free_our_vector(v_s2);
         free_our_vector(v_s3);
         free_our_vector(v_s4);
         free_our_vector(v_h);
         free_our_vector(v_g);
         free_our_vector(zero);
         free_our_vector(u);
         return(0);
      }
      /*    printf("i : ");
            print_our_vector(i); */
      /* find minimum of wgsbs_gsm_f(i - t u) */
      /* alg 5_2 */

      s1 = 0e0;
      copy_our_vector(v_s1, i);
      g_s1 = fitness;

      s3 = 1e0;
      
      s2 = 5e-1;
      mult_our_vector_vvs(v_s2, u, -s2);
      /*    printf("v_s2 after mult_our_vector_vvs : ");
            print_our_vector(v_s2); */
      /*    printf("i : "); */
      add_our_vector_vv(v_s2, i);
      print_our_vector(v_s2);
      g_s2 = wgsbs_gsm_f(v_s2);

      /* cut down interval until we have a decrease */
      printf("%d: decrease\n", this_rank); 
      fflush(stdout);

#if defined(SHOW_TIMING)
      gettimeofday(&tv1, NULL);
#endif
#if defined(DEBUG_GSM)
      printf("s values\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif

      while(g_s2 > g_s1 && s2 - s1 > epsilon) {
         s3 = s2;
         s2 *= 5e-1;
         mult_our_vector_vvs(v_s2, u, -s2);
         add_our_vector_vv(v_s2, i);
         g_s2 = wgsbs_gsm_f(v_s2);
      }

      if(s2 - s1 <= epsilon || s3 - s2 <= epsilon) {
         /* ugh, no decrease */
         printf("%d: no initial decrease, terminating pos = ", this_rank);
         /*      print_our_vector(i); */
         free_our_vector(v_s1);
         free_our_vector(v_s2);
         free_our_vector(v_s3);
         free_our_vector(v_s4);
         free_our_vector(v_h);
         free_our_vector(v_g);
         free_our_vector(zero);
         free_our_vector(u);
         return(max_iter - iter);
      }

      mult_our_vector_vvs(v_s3, u,-s3);
      add_our_vector_vv(v_s3, i);
      g_s3 = wgsbs_gsm_f(v_s3);


#if defined(DEBUG_GSM)
      printf("pre\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3); 
#endif

      reps = 0;

#if defined(SHOW_TIMING)
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
#endif
      printf("%d: start line search\n", this_rank); 
      fflush(stdout);
#if defined(SHOW_TIMING)
      gettimeofday(&tv1, NULL);
#endif
    
      while(s2 - s1 > epsilon && s3 - s2 > epsilon && reps++ < MAX_REPS) {
      
#if defined(DEBUG_GSM)
         printf("start\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif

         s1_s2 = 1e0 / (s1 - s2);
         s1_s3 = 1e0 / (s1 - s3);
         s2_s3 = 1e0 / (s2 - s3);

         s1_2 = s1 * s1;
         s2_2 = s2 * s2;
         s3_2 = s3 * s3;

         a = ((g_s1 - g_s3) * s1_s3 - (g_s2 - g_s3) * s2_s3) * s1_s2;

         /* printf("new a = %.12g\n",a); */
         b = (g_s3 * (s2_2 - s1_2) + g_s2 * (s1_2 - s3_2) + g_s1 * (s3_2 - s2_2)) *
            s1_s2 * s1_s3 * s2_s3;

         /* printf("new b = %.12g\n",b); */

         if(fabs(a) < MIN_A) {
            /* maybe we should switch to a bisection method? */
            printf("%d: a limit reached", this_rank);
            printf("done iter %ld, i = ", iter);
            print_our_vector(i);
            free_our_vector(v_s1);
            free_our_vector(v_s2);
            free_our_vector(v_s3);
            free_our_vector(v_s4);
            free_our_vector(v_h);
            free_our_vector(v_g);
            free_our_vector(zero);
            free_our_vector(u);
            return(max_iter - iter);
         }

         x = -b / (2e0 * a);

         prev_g_s2 = g_s2;
         // prev_s2 = s2;

#if defined(DEBUG_GSM)
         printf("new x = %.12g\n", x);
#endif

         if(x < s1) {
#if defined(DEBUG_GSM)
            printf("p1 ");
#endif
            if(x < (s1 + s1 - s2)) { /* keep it close */
               x = s1 + s1 - s2;
               if(x < 0) {
                  x = s1 / 2;
               }
            }
            if(x < 0) { /* ugh we're in the wrong direction! */
               printf("%d: unexpected minimum pos %.12g\n", this_rank, x);
               //     exit(-1);
               if(s1 < 0) {
                  s1 = 0;
               }
               x = 0;
            } 
            /* ok, take x, s1, s2 */
            v_tmp = v_s3;
            v_s3 = v_s2;
            g_s3 = g_s2;
            s3 = s2;
            v_s2 = v_s1;
            g_s2 = g_s1;
            s2 = s1;
            v_s1 = v_tmp;
   
            s1 = x;
            mult_our_vector_vvs(v_s1, u, -s1);
            add_our_vector_vv(v_s1, i);
            g_s1 = wgsbs_gsm_f(v_s1);
         } else {
            if(x < s2) {
#if defined(DEBUG_GSM)
               printf("p2 ");
#endif
               /* ok, take s1, x, s2 */
               v_tmp = v_s3;
               v_s3 = v_s2;
               g_s3 = g_s2;
               s3 = s2;
               v_s2 = v_tmp;
   
               s2 = x;
               /*     printf("x = %.12g\n", x); */
               mult_our_vector_vvs(v_s2, u, -s2);
               add_our_vector_vv(v_s2, i);
               g_s2 = wgsbs_gsm_f(v_s2);
            } else {
               if(x < s3) {
#if defined(DEBUG_GSM)
                  printf("p3 ");
#endif
                  /* ok, take s2, x, s3 */
                  v_tmp = v_s1;
                  v_s1 = v_s2;
                  g_s1 = g_s2;
                  s1 = s2;
                  v_s2 = v_tmp;
   
                  s2 = x;
                  mult_our_vector_vvs(v_s2, u, -s2);
                  add_our_vector_vv(v_s2, i);
                  g_s2 = wgsbs_gsm_f(v_s2);
               } else {
                  /* ugh x >= s3.. well why not? */
                  if(x > (s3 + s3 - s2)) { /* keep it close */
                     mult_our_vector_vvs(v_s4, u, -x);
                     add_our_vector_vv(v_s4, i);
                     g_s4 = wgsbs_gsm_f(v_s4);
                     if(g_s4 > g_s2 &&
                        g_s4 > g_s3 &&
                        g_s4 > g_s1) {
                        x = (s3 + s3 - s2);
                     }
                  }
                  /* take s2, s3, x */
#if defined(DEBUG_GSM)
                  printf("p4 ");
#endif
       
                  v_tmp = v_s1;
                  v_s1 = v_s2;
                  g_s1 = g_s2;
                  s1 = s2;
                  v_s2 = v_s3;
                  g_s2 = g_s3;
                  s2 = s3;
                  v_s3 = v_tmp;
   
                  s3 = x;
                  mult_our_vector_vvs(v_s3, u, -s3);
                  add_our_vector_vv(v_s3, i);
                  g_s3 = wgsbs_gsm_f(v_s3);
               }
            }
         }
      
         if(fabs(prev_g_s2 - g_s2) < epsilon) {
            printf("%d: fabs(g-prev) < epsilon\n", this_rank); fflush(stdout);
            break;
         }
         /*      puts(""); */
      }
      last_reps = reps;
      /*    printf("v_s2 ");
            print_our_vector(v_s2); */
      if(g_s2 < g_s3 && g_s2 < g_s1) {
         copy_our_vector(i, v_s2);
         g_s4 = g_s2;
      } else {
         if(g_s1 < g_s3) {
            copy_our_vector(i, v_s1);
            g_s4 = g_s1;
         } else {
            copy_our_vector(i, v_s3);
            g_s4 = g_s3;
         }
      }


      if(evenn) {
         switch(queries) {
         case 8 : i->d[(6 * N_2) - 1] = 0; [[fallthrough]];
         case 7 : [[fallthrough]];
         case 6 : i->d[(4 * N_2) - 1] = 0; [[fallthrough]];
         case 5 : [[fallthrough]];
         case 4 : i->d[(2 * N_2) - 1] = 0; [[fallthrough]];
         default : break;
         }
      }

#if defined(SHOW_TIMING)
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
#endif
      printf("%d: df start\n", this_rank); 
      fflush(stdout);
      wgsbs_gsm_df(u, i);
      puts("conj dir start");
      fflush(stdout);
#if defined(SHOW_TIMING)
      gettimeofday(&tv1, NULL);
#endif
    
      ggd = dot_our_vector(v_g, v_g);
      /*    printf("ggd = %.12g\n", ggd);*/
      if(ggd == 0e0) {
         puts("ggd == 0e0, returning");
         free_our_vector(v_s1);
         free_our_vector(v_s2);
         free_our_vector(v_s3);
         free_our_vector(v_s4);
         free_our_vector(v_h);
         free_our_vector(v_g);
         free_our_vector(zero);
         free_our_vector(u);
         return(0);
      }
      sub_our_vector_vvv(v_s4, u, v_g);
      gg = dot_our_vector(v_s4, u);
      /*    printf("gg = %.12g\n", gg);*/
      mult_our_vector_vs(v_h, gg / ggd);
      copy_our_vector(v_g, u);
      add_our_vector_vv(v_h, u);
      copy_our_vector(u, v_h);
#if defined(SHOW_TIMING)
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
#endif
      fflush(stdout);

      if(!(global_iter % 5)) {
         //      int r, s, j;
      
         printf("%d: query4i_intermediate|alg3|%s|%d|%.12g|%d|%d|%d|%d|%d|%.12g|%.12g|%.12g|%.12g|%ld|%d|%d|%d|%.12g|%ld|%ld", this_rank,
                (wgsbs_gsm_f(i) <= 0e0) ? "success" : "fail",
                N, 
                wgsbs_gsm_f(i),
                queries,
                fitness_type,
                0,
                this_iterations + prev_iterations, 
                go, 
                0e0,
                0e0,
                0e0,
                0e0,
                iterations_max,
                0,
                0,
                0,
                min_acceptable,
                0L, 0L
                ); 
      
         puts(""); 
      }
   }

   printf("%d: done iter %ld, i = ", this_rank, iter);
   print_our_vector(i);
   free_our_vector(v_s1);
   free_our_vector(v_s2);
   free_our_vector(v_s3);
   free_our_vector(v_s4);
   free_our_vector(v_h);
   free_our_vector(v_g);
   free_our_vector(zero);
   free_our_vector(u);
   return(0);
}
  
/* inverse hessian */


long US_Saxs_Util::min_hessian_bfgs(our_vector *ip, double epsilon, long max_iter) 
{
  
#if defined(SHOW_TIMING)
   struct timeval tv1, tv2;
#endif
   our_matrix *hessian;

   our_vector *u, *zero;
   //  double t;
   double s1, s2, s3;
   double g_s1, g_s2, g_s3, g_s4;
   our_vector *v_s1, *v_s2, *v_s3, *v_tmp, *v_s4;
   our_vector *v_g, *v_dg, *v_hdg, *v_p, *v_dx;
   double a, b, x;
   double s1_s2, s1_s3, s2_s3;
   double s1_2, s2_2, s3_2;
   // double prev_s2;
   double prev_g_s2;
   long iter = 0l;
   int reps, last_reps = 0;
   int i,j;
   double fitness;
   double fac, fad, fae, sumdg, sumxi;

   /*  printf("initial position: ");
       print_our_vector(i); */

   zero = new_our_vector(ip->len);
   set_our_vector(zero, 0e0);

   u = new_our_vector(ip->len);
   wgsbs_gsm_df(u, ip);

   v_p = new_our_vector(ip->len); /* the new point */

   /*  mult_our_vector_vs(u, 1e0 / l2norm_our_vector(u, zero)); */

   /*  printf("initial gradient ||=%.12g : ", l2norm_our_vector(u, zero)); */
   /*  print_our_vector(u); */

   hessian = new_our_matrix(ip->len, ip->len);
   identity_our_matrix(hessian);
  
   v_s1 = new_our_vector(ip->len);
   v_s2 = new_our_vector(ip->len);
   v_s3 = new_our_vector(ip->len);
   v_s4 = new_our_vector(ip->len);

   v_g = new_our_vector(ip->len); /* new gradient */
   v_dg = new_our_vector(ip->len); /* dgradient */
   v_hdg = new_our_vector(ip->len); /* hdg */

   v_p = new_our_vector(ip->len); /* the new point */
   v_dx = new_our_vector(ip->len); /* p - ip, the direction (xi) */


   while(l2norm_our_vector(u, zero) >= epsilon && iter++ < max_iter) {
      this_iterations++;
#if defined(DEBUG_GSM)
      printf("begin\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif
      printf("%d: global %ld, iter %ld fitness %.12g |grad|=%.12g last reps %d\n", this_rank, global_iter++, 
             iter, fitness = wgsbs_gsm_f(ip), l2norm_our_vector(u, zero), last_reps); fflush(stdout);
      printf("%d: ", this_rank);
      print_our_vector(ip);
      /*    print_our_vector(u); */
      fflush(stdout);
      /*    printf("ip : ");
            print_our_vector(ip); */
      /* find minimum of wgsbs_gsm_f(ip - t u) */
      /* alg 5_2 */
      if(!fitness) {
         free_our_vector(v_s1);
         free_our_vector(v_s2);
         free_our_vector(v_s3);
         free_our_vector(v_s4);
         free_our_vector(v_g);
         free_our_vector(v_dg);
         free_our_vector(v_hdg);
         free_our_vector(v_p);
         free_our_vector(v_dx);
         free_our_vector(zero);
         free_our_vector(u);
         free_our_matrix(hessian);
         return(0);
      }

      s1 = 0e0;
      copy_our_vector(v_s1, ip);
      g_s1 = fitness;

      s3 = 1e0;
      
      s2 = 5e-1;
      mult_our_vector_vvs(v_s2, u, -s2);
      /*    printf("v_s2 after mult_our_vector_vvs : ");
            print_our_vector(v_s2); */
      /*    printf("i : "); */
      add_our_vector_vv(v_s2, ip);
      g_s2 = wgsbs_gsm_f(v_s2);

      /* cut down interval until we have a decrease */
      printf("%d: decrease\n", this_rank); 
      fflush(stdout);
#if defined(SHOW_TIMING)
      gettimeofday(&tv1, NULL);
#endif
      while(g_s2 > g_s1 && s2 - s1 > epsilon) {
         s3 = s2;
         s2 *= 5e-1;
         mult_our_vector_vvs(v_s2, u, -s2);
         add_our_vector_vv(v_s2, ip);
         g_s2 = wgsbs_gsm_f(v_s2);
      }

      if(s2 - s1 <= epsilon || s3 - s2 <= epsilon) {
         /* ugh, no decrease */
         printf("%d: no initial decrease, terminating pos = ", this_rank);
         /*      print_our_vector(ip); */
         free_our_vector(v_s1);
         free_our_vector(v_s2);
         free_our_vector(v_s3);
         free_our_vector(v_s4);
         free_our_vector(v_g);
         free_our_vector(v_dg);
         free_our_vector(v_hdg);
         free_our_vector(v_p);
         free_our_vector(v_dx);
         free_our_vector(zero);
         free_our_vector(u);
         free_our_matrix(hessian);
         return(max_iter - iter);
      }

      mult_our_vector_vvs(v_s3, u,-s3);
      add_our_vector_vv(v_s3, ip);
      g_s3 = wgsbs_gsm_f(v_s3);


#if defined(DEBUG_GSM)
      printf("pre\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3); 
#endif

    
      reps = 0;

#if defined(SHOW_TIMING)
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
#endif
      printf("%d: start line search\n", this_rank); 
      fflush(stdout);
#if defined(SHOW_TIMING)
      gettimeofday(&tv1, NULL);
#endif

      while(s2 - s1 > epsilon && s3 - s2 > epsilon && reps++ < MAX_REPS) {
      
#if defined(DEBUG_GSM)
         printf("start\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif

         s1_s2 = 1e0 / (s1 - s2);
         s1_s3 = 1e0 / (s1 - s3);
         s2_s3 = 1e0 / (s2 - s3);

         s1_2 = s1 * s1;
         s2_2 = s2 * s2;
         s3_2 = s3 * s3;

         a = ((g_s1 - g_s3) * s1_s3 - (g_s2 - g_s3) * s2_s3) * s1_s2;

         /* printf("new a = %.12g\n",a); */
         b = (g_s3 * (s2_2 - s1_2) + g_s2 * (s1_2 - s3_2) + g_s1 * (s3_2 - s2_2)) *
            s1_s2 * s1_s3 * s2_s3;

         /* printf("new b = %.12g\n",b); */

         if(fabs(a) < MIN_A) {
            /* maybe we should switch to a bisection method? */
            printf("%d: a limit reached", this_rank);
            printf("done iter %ld, i = ", iter);
            print_our_vector(ip);
            free_our_vector(v_s1);
            free_our_vector(v_s2);
            free_our_vector(v_s3);
            free_our_vector(v_s4);
            free_our_vector(v_g);
            free_our_vector(v_dg);
            free_our_vector(v_hdg);
            free_our_vector(v_p);
            free_our_vector(v_dx);
            free_our_vector(zero);
            free_our_vector(u);
            free_our_matrix(hessian);
            return(max_iter - iter);
         }

         x = -b / (2e0 * a);

         prev_g_s2 = g_s2;
         // prev_s2 = s2;

#if defined(DEBUG_GSM)
         printf("new x = %.12g\n", x);
#endif

         if(x < s1) {
#if defined(DEBUG_GSM)
            printf("p1 ");
#endif
            if(x < (s1 + s1 - s2)) { /* keep it close */
               x = s1 + s1 - s2;
               if(x < 0) {
                  x = s1 / 2;
               }
            }
            if(x < 0) { /* ugh we're in the wrong direction! */
               printf("%d: unexpected minimum pos %.12g\n", this_rank, x);
               if(s1 < 0) {
                  s1 = 0;
               }
               x = 0;
               //     exit(-1);
            } 
            /* ok, take x, s1, s2 */
            v_tmp = v_s3;
            v_s3 = v_s2;
            g_s3 = g_s2;
            s3 = s2;
            v_s2 = v_s1;
            g_s2 = g_s1;
            s2 = s1;
            v_s1 = v_tmp;
   
            s1 = x;
            mult_our_vector_vvs(v_s1, u, -s1);
            add_our_vector_vv(v_s1, ip);
            g_s1 = wgsbs_gsm_f(v_s1);
         } else {
            if(x < s2) {
#if defined(DEBUG_GSM)
               printf("p2 ");
#endif
               /* ok, take s1, x, s2 */
               v_tmp = v_s3;
               v_s3 = v_s2;
               g_s3 = g_s2;
               s3 = s2;
               v_s2 = v_tmp;
   
               s2 = x;
               /*     printf("x = %.12g\n", x); */
               mult_our_vector_vvs(v_s2, u, -s2);
               add_our_vector_vv(v_s2, ip);
               g_s2 = wgsbs_gsm_f(v_s2);
            } else {
               if(x < s3) {
#if defined(DEBUG_GSM)
                  printf("p3 ");
#endif
                  /* ok, take s2, x, s3 */
                  v_tmp = v_s1;
                  v_s1 = v_s2;
                  g_s1 = g_s2;
                  s1 = s2;
                  v_s2 = v_tmp;
   
                  s2 = x;
                  mult_our_vector_vvs(v_s2, u, -s2);
                  add_our_vector_vv(v_s2, ip);
                  g_s2 = wgsbs_gsm_f(v_s2);
               } else {
                  /* ugh x >= s3.. well why not? */
                  if(x > (s3 + s3 - s2)) { /* keep it close */
                     mult_our_vector_vvs(v_s4, u, -x);
                     add_our_vector_vv(v_s4, ip);
                     g_s4 = wgsbs_gsm_f(v_s4);
                     if(g_s4 > g_s2 &&
                        g_s4 > g_s3 &&
                        g_s4 > g_s1) {
                        x = (s3 + s3 - s2);
                     }
                  }
                  /* take s2, s3, x */
#if defined(DEBUG_GSM)
                  printf("p4 ");
#endif
       
                  v_tmp = v_s1;
                  v_s1 = v_s2;
                  g_s1 = g_s2;
                  s1 = s2;
                  v_s2 = v_s3;
                  g_s2 = g_s3;
                  s2 = s3;
                  v_s3 = v_tmp;
   
                  s3 = x;
                  mult_our_vector_vvs(v_s3, u, -s3);
                  add_our_vector_vv(v_s3, ip);
                  g_s3 = wgsbs_gsm_f(v_s3);
               }
            }
         }
      
         if(fabs(prev_g_s2 - g_s2) < epsilon) {
            printf("%d: fabs(g-prev) < epsilon\n", this_rank); fflush(stdout);
            break;
         }
         /*      puts(""); */
      }
      last_reps = reps;
      if(g_s2 < g_s3 && g_s2 < g_s1) {
         copy_our_vector(v_p, v_s2);
         g_s4 = g_s2;
      } else {
         if(g_s1 < g_s3) {
            copy_our_vector(v_p, v_s1);
            g_s4 = g_s1;
         } else {
            copy_our_vector(v_p, v_s3);
            g_s4 = g_s3;
         }
      }


      if(evenn) {
         switch(queries) {
         case 8 : v_p->d[(6 * N_2) - 1] = 0; [[fallthrough]];
         case 7 : [[fallthrough]];
         case 6 : v_p->d[(4 * N_2) - 1] = 0; [[fallthrough]];
         case 5 : [[fallthrough]];
         case 4 : v_p->d[(2 * N_2) - 1] = 0; [[fallthrough]];
         default : break;
         }
      }

      /*    printf("v_dx:"); print_our_vector(v_dx); */

#if defined(SHOW_TIMING)
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
#endif
      printf("%d: df start\n", this_rank); 
      fflush(stdout);
      wgsbs_gsm_df(v_g, v_p);                 /* new gradient in v_g (old in u) */
      printf("%d: hessian start\n", this_rank);
#if defined(SHOW_TIMING)
      gettimeofday(&tv1, NULL);
#endif

      sub_our_vector_vvv(v_dx, v_p, ip); /* calc dx */
      copy_our_vector(ip, v_p);

      sub_our_vector_vvv(v_dg, v_g, u); /* dgradient */
      /*    printf("v_dg:"); print_our_vector(v_dg); */

      mult_our_matrix_vmv(v_hdg, hessian, v_dg);
      /*    printf("v_hdg:"); print_our_vector(v_hdg); */

      fac = dot_our_vector(v_dg, v_dx);
      fae = dot_our_vector(v_dg, v_hdg);
      sumdg = dot_our_vector(v_dg, v_dg);
      sumxi = dot_our_vector(v_dx, v_dx);

      /*    printf("fac %.12g fae %.12g sumdg %.12g sumxi %.12g\n",
            fac, fae, sumdg, sumxi); */

      if(fac > sqrt(epsilon * sumdg * sumxi)) {
         fac = 1e0/fac;
         fad = 1e0/fae;

         for(i = 0; i < v_dg->len; i++) {
            v_dg->d[i] = fac * v_dx->d[i] - fad * v_hdg->d[i];
         }
         for(i = 0; i < v_dg->len; i++) {
            for(j = i; j < v_dg->len; j++) {
               hessian->d[(i * hessian->cols) + j] +=
                  fac * v_dx->d[i] * v_dx->d[j] - 
                  fad * v_hdg->d[i] * v_hdg->d[j] +
                  fae * v_dg->d[i] * v_dg->d[j];
               hessian->d[(j * hessian->cols) + i] =
                  hessian->d[(i * hessian->cols) + j];
            }
         }
      }
      mult_our_matrix_vmv(u, hessian, v_g);
#if defined(SHOW_TIMING)
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      fflush(stdout);
#endif
      /*    mult_our_vector_vs(u, -1e0); */
      /*
        print_our_vector(u);
        print_our_matrix(hessian);
        print_our_vector(v_g);
        printf("|u|=%.12g\n", l2norm_our_vector(u, zero)); 
      */

#if defined(DEBUG_GSM)
      printf("end\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif

      if(!(global_iter % 5)) {
         //      int r, s, j;
      
         printf("%d: query4i_intermediate|alg4|%s|%d|%.12g|%d|%d|%.12g|%d|%d|%.12g|%.12g|%.12g|%.12g|%ld|%d|%d|%d|%.12g|%ld|%ld",
                this_rank,
                (wgsbs_gsm_f(ip) <= 0e0) ? "success" : "fail",
                N, 
                wgsbs_gsm_f(ip),
                queries,
                fitness_type,
                0e0,
                this_iterations + prev_iterations, 
                go, 
                0e0,
                0e0,
                0e0,
                0e0,
                iterations_max,
                0,
                0,
                0,
                min_acceptable,
                0L, 0L
                ); 
      
         puts(""); fflush(stdout);
      }
   }

   printf("%d done iter %ld, i = ", this_rank, iter);
   print_our_vector(ip);
   free_our_vector(v_s1);
   free_our_vector(v_s2);
   free_our_vector(v_s3);
   free_our_vector(v_s4);
   free_our_vector(v_g);
   free_our_vector(v_dg);
   free_our_vector(v_hdg);
   free_our_vector(v_p);
   free_our_vector(v_dx);
   free_our_vector(zero);
   free_our_vector(u);
   free_our_matrix(hessian);
   return(0);
}


void US_Saxs_Util::clear_project()
{
   wave.clear( );
   p_project = "";
   p_wiki = "";
   p_wiki_prefix = "";
   p_name = "";
   p_description = "";
   p_short_description = "";
   p_comment = "";
   p_mw = 0e0;
   p_conc_mult = 0e0;
   p_saxs_lowq = 0e0;
   p_saxs_highq = 0e0;
   p_waxs_lowq = 0e0;
   p_waxs_highq = 0e0;
   p_waxs_zlowq = 0e0;
   p_waxs_zhighq = 0e0;
   p_overlap_lowq = 0e0;
   p_overlap_highq = 0e0;
   p_alpha_min = 0.98e0;
   p_alpha_max = 1e0;
   p_join_q = .1e0;
   p_guinier_maxq = .05e0;
   p_iterations_grid = 10;
   p_iterations_gsm = 50;
   p_rmax_start = 50;
   p_rmax_end = 200;
   p_rmax_inc = 50;
   p_crop_low = 0;
   p_crop_high = 1e6;
   p_iterations_gsm = 50;
   wave_names_vector.clear( );
   wave_names.clear( );
   wave_file_names.clear( );
   wave_types.clear( );
   wave_concs.clear( );
   wave_alphas.clear( );
   wave_betas.clear( );
   wave_consts.clear( );
   wave_exposure_times.clear( );
   wave_buffer_names.clear( );
   wave_empty_names.clear( );
   wave_comments.clear( );
   wave_Rgs.clear( );
   wave_I0s.clear( );
   wave_smins.clear( );
   wave_smaxs.clear( );
   wave_sRgmins.clear( );
   wave_sRgmaxs.clear( );
   wave_chi2s.clear( );
   wave_alpha_starts.clear( );
   wave_alpha_ends.clear( );
   wave_alpha_incs.clear( );
   any_saxs = false;
   any_waxs = false;
   wave_sb.clear( );
   wave_wgsbs.clear( );
   wave_join.clear( );
}   

bool US_Saxs_Util::check_project_files()
{
   errormsg = "";
   for (
        map < QString, QString >::iterator it = wave_file_names.begin();
        it != wave_file_names.end();
        it++ 
         )
   {
      QFileInfo fi("wave/" + it->second);
      if ( !fi.exists() )
      {
         errormsg +=
            QString("%1file %2 does not exist")
            .arg(errormsg.isEmpty() ? "" : "\n" )
            .arg(fi.filePath());
      } else {
         if ( !fi.isReadable() )
         {
            errormsg +=
               QString("%1file %2 is not readable (check permissions)")
               .arg(errormsg.isEmpty() ? "" : "\n" )
               .arg(fi.filePath());
         }
         if ( fi.isDir() )
         {
            errormsg +=
               QString("%1file %2 is a directory!")
               .arg(errormsg.isEmpty() ? "" : "\n" )
               .arg(fi.filePath());
         }
      }
   }
   for ( unsigned int i = 0; i < wave_names_vector.size(); i++ )
   {
      if ( 
          wave_concs[wave_names_vector[i]] &&
          !wave_buffer_names[wave_names_vector[i]].size()
          )
      {
         errormsg +=
            QString("%1wave %2 has no associated buffers")
            .arg(errormsg.isEmpty() ? "" : "\n" )
            .arg(wave_names_vector[i]);
      }
   }
   return errormsg.isEmpty();
}

bool US_Saxs_Util::read_project( QString subdir )
{
   errormsg = "";
   QFile f(QString("%1project").arg(subdir.isEmpty() ? "" : subdir + QDir::separator()));
   if ( !f.exists() )
   {
      errormsg = "project file does not exist";
      return false;
   }

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      errormsg = "project file can not be opened, check permissions";
      return false;
   }

   clear_project();

   QString line;
   QRegExp rx("^(\\S+)\\s+(\\S.*)$");
   QRegExp rxempty("^(\\s*#|\\s*$)");
   QRegExp rxtrailingspaces("\\s*$");
   QRegExp rxvalid(
                   "^("
                   "wiki|"
                   "wikiprefix|"
                   "name|"
                   "description|"
                   "shortdescription|"
                   "comment|"
                   "mw|"
                   "concmultiplier|"
                   "saxslowq|"
                   "saxshighq|"
                   "waxslowq|"
                   "waxshighq|"
                   "waxszlowq|"
                   "waxszhighq|"
                   "overlaplowq|"
                   "overlaphighq|"
                   "alphamin|"
                   "alphamax|"
                   "iterationsgrid|"
                   "iterationsgsm|"
                   "joinq|"
                   "guiniermaxq|"
                   "wavename|"
                   "wavefilename|"
                   "wavetype|"
                   "waveconc|"
                   "waveexposuretime|"
                   "wavebuffername|"
                   "waveemptyname|"
                   "wavealpha|"
                   "wavebeta|"
                   "waveconst|"
                   "wavealphastart|"
                   "wavealphaend|"
                   "wavealphainc|"
                   "waveoverlaplowq|"
                   "waveoverlaphighq|"
                   "gnomrmaxstart|"
                   "gnomrmaxend|"
                   "gnomrmaxinc|"
                   "gnomcroplow|"
                   "gnomcrophigh|"
                   "remark)$"
                   );

   QRegExp rxvalidwavetype("^(saxs|waxs)$");

   unsigned int linepos = 0;
   QString last_wave_name = "";

   QTextStream ts(&f);

   while ( !ts.atEnd() )
   {
      ++linepos;
      line = ts.readLine().replace(rxtrailingspaces,"");
      p_project += line + "\n";

      if ( line.isEmpty() || line.contains(rxempty) )
      {
         continue;
      }

      if ( rx.indexIn(line) == -1 )
      {
         errormsg = QString("error in project file line %1.  At least two tokens not found <%2>\n")
            .arg(linepos)
            .arg(line);
         return false;
      }

      QString token = rx.cap(1).toLower();
      QString data = rx.cap(2);

      if ( !token.contains(rxvalid) )
      {
         errormsg = QString("error in project file line %1.  Unrecognized token <%2>\n")
            .arg(linepos)
            .arg(line);
         return false;
      }

      if ( token == "wiki" )
      {
         p_wiki = data;
         continue;
      }
   
      if ( token == "wikiprefix" )
      {
         p_wiki_prefix = data;
         continue;
      }

      if ( token == "name" )
      {
         p_name = data;
         continue;
      }

      if ( token == "description" )
      {
         p_description += data;
         continue;
      }

      if ( token == "shortdescription" )
      {
         p_short_description = data;
         continue;
      }

      if ( last_wave_name.isEmpty() && token == "comment" )
      {
         p_comment += data;
         continue;
      }

      if ( token == "mw" )
      {
         p_mw = data.toDouble();
         continue;
      }

      if ( token == "concmultiplier" )
      {
         p_conc_mult = data.toDouble();
         continue;
      }

      if ( token == "saxslowq" )
      {
         p_saxs_lowq = data.toDouble();
         continue;
      }

      if ( token == "saxshighq" )
      {
         p_saxs_highq = data.toDouble();
         continue;
      }

      if ( token == "waxslowq" )
      {
         p_waxs_lowq = data.toDouble();
         continue;
      }

      if ( token == "waxshighq" )
      {
         p_waxs_highq = data.toDouble();
         continue;
      }

      if ( token == "waxszlowq" )
      {
         p_waxs_zlowq = data.toDouble();
         continue;
      }

      if ( token == "waxszhighq" )
      {
         p_waxs_zhighq = data.toDouble();
         continue;
      }

      if ( token == "overlaplowq" )
      {
         p_overlap_lowq = data.toDouble();
         continue;
      }

      if ( token == "overlaphighq" )
      {
         p_overlap_highq = data.toDouble();
         continue;
      }

      if ( token == "alphamin" )
      {
         p_alpha_min = data.toDouble();
         continue;
      }

      if ( token == "alphamax" )
      {
         p_alpha_max = data.toDouble();
         continue;
      }

      if ( token == "iterationsgrid" )
      {
         p_iterations_grid = data.toUInt();
         continue;
      }

      if ( token == "iterationsgsm" )
      {
         p_iterations_gsm = data.toUInt();
         continue;
      }

      if ( token == "joinq" )
      {
         p_join_q = data.toDouble();
         continue;
      }

      if ( token == "guiniermaxq" )
      {
         p_guinier_maxq = data.toDouble();
         continue;
      }

      if ( token == "gnomrmaxstart" )
      {
         p_rmax_start = data.toDouble();
         continue;
      }

      if ( token == "gnomrmaxend" )
      {
         p_rmax_end = data.toDouble();
         continue;
      }

      if ( token == "gnomrmaxinc" )
      {
         p_rmax_inc = data.toDouble();
         continue;
      }

      if ( token == "gnomcroplow" )
      {
         p_crop_low = data.toDouble();
         continue;
      }

      if ( token == "gnomcrophigh" )
      {
         p_crop_high = data.toDouble();
         continue;
      }

      if ( token == "wavename" )
      {
         if ( wave_names.count(data) )
         {
            errormsg = QString("error in project file line %1.  duplicate waveName previously defined <%2>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
         last_wave_name = data;
         wave_names[data] = true;
         wave_names_vector.push_back(data);
         wave_concs[data] = 0e0;
         wave_alphas[data] = 0e0;
         wave_types[data] = "";
         wave_comments[data] = "";
         wave_alpha_starts[data] = 0e0;
         wave_alpha_ends[data] = 0e0;
         wave_alpha_incs[data] = 0e0;
         wave_overlap_lowq[data] = 0e0;
         wave_overlap_highq[data] = 0e0;
         continue;
      }

      // all wave based data setup follows

      if ( last_wave_name.isEmpty() )
      {
         errormsg = QString("error in project file line %1.  waveName must be defined first <%2>\n")
            .arg(linepos)
            .arg(line);
         return false;
      }

      if ( token == "wavefilename" )
      {
         wave_file_names[last_wave_name] = data;
         continue;
      }

      if ( token == "wavetype" )
      {
         if ( !data.contains(rxvalidwavetype) )
         {
            errormsg = QString("error in project file line %1.  Invalid waveType <%2>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
         wave_types[last_wave_name] = data;
         continue;
      }

      if ( token == "waveconc" )
      {
         wave_concs[last_wave_name] = data.toDouble();
         continue;
      }

      if ( token == "waveexposuretime" )
      {
         wave_exposure_times[last_wave_name] = data.toDouble();
         continue;
      }

      if ( token == "wavebuffername" )
      {
         if ( !wave_names[data] )
         {
            errormsg = QString("error in project file line %1.  waveName for buffer not previously defined <%2>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
      
         wave_buffer_names[last_wave_name].push_back(data);
         continue;
      }
 
      if ( token == "waveemptyname" )
      {
         if ( !wave_names[data] )
         {
            errormsg = QString("error in project file line %1.  waveName for empty not previously defined <%2>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
         wave_empty_names[last_wave_name].push_back(data);
         continue;
      }

      if ( token == "wavealpha" )
      {
         wave_alphas[last_wave_name] = data.toDouble();
         continue;
      }
      if ( token == "wavebeta" )
      {
         if ( wave_types[last_wave_name] != "waxs" )
         {
            errormsg = QString("error in project file line %1.  waveType must be waxs for beta assignment <%2>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
         wave_betas[last_wave_name] = data.toDouble();
         continue;
      }

      if ( token == "waveconst" )
      {
         if ( wave_types[last_wave_name] != "waxs" )
         {
            errormsg = QString("error in project file line %1.  waveType must be waxs for const assignment <%2>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
         wave_consts[last_wave_name] = data.toDouble();
         continue;
      }

      if ( token == "wavealphastart" )
      {
         if ( wave_types[last_wave_name] != "waxs" )
         {
            errormsg = QString("error in project file line %1.  waveType must be waxs for beta assignment <%2>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
         if ( !wave_concs[last_wave_name] )
         {
            errormsg = QString("error in project file line %1.  waveConc must be nonzero for waveAlphaStart <%2>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
         wave_alpha_starts[last_wave_name] = data.toDouble();
         continue;
      }

      if ( token == "wavealphaend" )
      {
         if ( wave_types[last_wave_name] != "waxs" )
         {
            errormsg = QString("error in project file line %1.  waveType must be waxs for beta assignment <%2>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
         if ( !wave_concs[last_wave_name] )
         {
            errormsg = QString("error in project file line %1.  waveConc must be nonzero for waveAlphaEnd <%2>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
         wave_alpha_ends[last_wave_name] = data.toDouble();
         continue;
      }

      if ( token == "wavealphainc" )
      {
         if ( wave_types[last_wave_name] != "waxs" )
         {
            errormsg = QString("error in project file line %1.  waveType must be waxs for beta assignment <%2>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
         if ( !wave_concs[last_wave_name] )
         {
            errormsg = QString("error in project file line %1.  waveConc must be nonzero for waveAlphaInc <%2>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
         wave_alpha_incs[last_wave_name] = data.toDouble();
         continue;
      }

      if ( token == "waveoverlaplowq" )
      {
         if ( wave_types[last_wave_name] != "saxs" )
         {
            errormsg = QString("error in project file line %1.  waveType must be saxs for waveOverlapLowQ <%2>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
         if ( !wave_concs[last_wave_name] )
         {
            errormsg = QString("error in project file line %1.  waveConc must be nonzero for waveOverlapLowQ <%2>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
         wave_overlap_lowq[last_wave_name] = data.toDouble();
         continue;
      }

      if ( token == "waveoverlaphighq" )
      {
         if ( wave_types[last_wave_name] != "saxs" )
         {
            errormsg = QString("error in project file line %1.  waveType must be saxs for waveOverlapHighq <%2>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
         if ( !wave_concs[last_wave_name] )
         {
            errormsg = QString("error in project file line %1.  waveConc must be nonzero for waveOverlapHighq <%2>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
         wave_overlap_highq[last_wave_name] = data.toDouble();
         continue;
      }

      if ( token == "comment" )
      {
         wave_comments[last_wave_name] += data;
         continue;
      }
   }
   f.close();

   cout << "project file ok\n";


   cout << "wiki file name:" << wiki_file_name() << endl;

   return true;
}
  
bool US_Saxs_Util::build_wiki()
{
   errormsg = "";

   if ( !read_project_waves() )
   {
      return false;
   }

   cout << "waves ok\n";

   if ( !compute_averages() )
   {
      return false;
   }

   compute_alphas();
   
   QFile f(wiki_file_name());
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      errormsg = "error: can not create " + wiki_file_name() + " for writing\n";
      return false;
   }
   QTextStream ts(&f);
   QString result;
   if ( !wiki(result) )
   {
      f.close();
      return false;
   }
   ts << result;
   f.close();
   return true;
}

bool US_Saxs_Util::read_project_waves()
{
   errormsg = "";
   if ( !check_project_files() )
   {
      return false;
   }

   for (
        map < QString, QString >::iterator it = wave_file_names.begin();
        it != wave_file_names.end();
        it++ 
         )
   {
      if ( !read("wave/" + it->second, it->first ) )
      {
         return false;
      }
   }
   return true;
}

bool US_Saxs_Util::compute_averages()
{
   errormsg = "";
   for ( unsigned int i = 0; i < wave_names_vector.size(); i++ )
   {
      if ( wave_buffer_names[wave_names_vector[i]].size() )
      {
         if ( wave_buffer_names[wave_names_vector[i]].size() > 1 )
         {
            cout << "averaging buffer for " << wave_names_vector[i] << endl;
            if ( !avg(wave_names_vector[i] + "|buffer", wave_buffer_names[wave_names_vector[i]]) )
            {
               return false;
            }
         } else {
            cout << "setting buffer for " << wave_names_vector[i] << endl;
            wave[wave_names_vector[i] + "|buffer"] = wave[wave_buffer_names[wave_names_vector[i]][0]];
         }

         if ( !write(get_file_name(wave_names_vector[i], "buffer"),wave_names_vector[i] + "|buffer") )
         {
            return false;
         }
      }
      if ( wave_empty_names[wave_names_vector[i]].size() )
      {
         if ( wave_empty_names[wave_names_vector[i]].size() > 1 )
         {
            cout << "averaging empty for " << wave_names_vector[i] << endl;
            if ( !avg(wave_names_vector[i] + "|empty", wave_empty_names[wave_names_vector[i]]) )
            {
               return false;
            }
         } else {
            cout << "setting empty for " << wave_names_vector[i] << endl;
            wave[wave_names_vector[i] + "|empty"] = wave[wave_empty_names[wave_names_vector[i]][0]];
         }
         if ( !write(get_file_name(wave_names_vector[i], "empty"),wave_names_vector[i] + "|empty") )
         {
            return false;
         }
      }
   }
   return true;
}

void US_Saxs_Util::compute_alphas()
{
   errormsg = "";
   for ( unsigned int i = 0; i < wave_names_vector.size(); i++ )
   {
      if ( !wave_alphas[wave_names_vector[i]] &&
           wave_types[wave_names_vector[i]] == "saxs" )
      {
         wave_alphas[wave_names_vector[i]] = 1e0 - wave_concs[wave_names_vector[i]] * p_conc_mult;
      }
   }
}

QString US_Saxs_Util::wiki_file_name()
{
   return p_wiki_prefix + p_name;
}

QString US_Saxs_Util::get_file_name(QString base, QString type)
{
   base.replace(QRegExp("^cwave."),"");
   return
      QString("cwave%1%2_%3.dat")
      .arg(QDir::separator())
      .arg(base.replace(QRegExp("\\.(dat|DAT)$"),""))
      .arg(type);
}


bool US_Saxs_Util::compute_wgsbs(
                                 QString outfile,
                                 QString solutiontag,
                                 QString buffertag,
                                 QString waxstag,
                                 int grids,
                                 int gsm_type,
                                 long max_iterations,
                                 double gsmpercent,
                                 double alphalow,
                                 double alphahigh,
                                 double alphaincg,
                                 double alphaincgsm,
                                 double betalow,
                                 double betahigh,
                                 double betaincg,
                                 double betaincgsm,
                                 double dconstlow,
                                 double dconsthigh,
                                 double dconstincg,
                                 double dconstincgsm,
                                 double low,
                                 double high,
                                 double &nrmsd,
                                 double &alphamin,
                                 double &betamin,
                                 double &dconstmin,
                                 QString &cliperrors
                                 )
{
   errormsg = "";
   cliperrors = "";

   // for clipping

   double alphalowlimit = alphalow;
   double alphahighlimit = alphahigh;
   double betalowlimit = betalow;
   double betahighlimit = betahigh;
   double dconstlowlimit = dconstlow;
   double dconsthighlimit = dconsthigh;
   
   // for incg recomputations
   double alphagridpoints = alphaincg ? ( alphahigh - alphalow ) / alphaincg : 0;
   double betagridpoints = betaincg ? ( betahigh - betalow ) / betaincg : 0;
   double dconstgridpoints = dconstincg ? ( dconsthigh - dconstlow ) / dconstincg : 0;

   QString logstring;

   for ( int g = 0; g < grids; g++ )
   {
      cout << 
         QString("ranges,inc for next grid alpha %1:%2 %3 beta %4:%5 %6 dconst %7:%8 %9\n")
         .arg(alphalow)
         .arg(alphahigh)
         .arg(alphaincg)
         .arg(betalow)
         .arg(betahigh)
         .arg(betaincg)
         .arg(dconstlow)
         .arg(dconsthigh)
         .arg(dconstincg);

      if ( !waxsguidedsaxsbuffersubgridsearch(nrmsd, 
                                              alphamin,
                                              betamin,
                                              dconstmin,
                                              logstring,
                                              outfile,
                                              solutiontag,
                                              buffertag,
                                              waxstag,
                                              alphalow,
                                              alphahigh,
                                              alphaincg,
                                              betalow,
                                              betahigh,
                                              betaincg,
                                              dconstlow,
                                              dconsthigh,
                                              dconstincg,
                                              low,
                                              high) )
      {
         return false;
      }
      cout << 
         QString("minimum from grid %1 nrmsd %2 alpha %3 beta %4 dconst %5\n")
         .arg(g)
         .arg(nrmsd)
         .arg(alphamin)
         .arg(betamin)
         .arg(dconstmin);
      
      // reduce grid
      
      double alphadelta = (alphahigh - alphalow) * 0.5 * gsmpercent / 100e0;
      alphalow = alphamin - alphadelta;
      alphahigh = alphamin + alphadelta;
      
      double betadelta = (betahigh - betalow) * 0.5 * gsmpercent / 100e0;
      betalow = betamin - betadelta;
      betahigh = betamin + betadelta;
      
      double dconstdelta = (dconsthigh - dconstlow) * 0.5 * gsmpercent / 100e0;
      dconstlow = dconstmin - dconstdelta;
      dconsthigh = dconstmin + dconstdelta;
      
      // clip

      if ( alphalow < alphalowlimit ) 
      {
         alphalow = alphalowlimit;
      } else {
         if ( alphalow > alphahighlimit )
         {
            alphalow = alphahighlimit;
         }
      }
      
      if ( alphahigh < alphalowlimit ) 
      {
         alphahigh = alphalowlimit;
      } else {
         if ( alphahigh > alphahighlimit )
         {
            alphahigh = alphahighlimit;
         }
      }
      
      if ( betalow < betalowlimit ) 
      {
         betalow = betalowlimit;
      } else {
         if ( betalow > betahighlimit )
         {
            betalow = betahighlimit;
         }
      }
      
      if ( betahigh < betalowlimit ) 
      {
         betahigh = betalowlimit;
      } else {
         if ( betahigh > betahighlimit )
         {
            betahigh = betahighlimit;
         }
      }
      
      if ( dconstlow < dconstlowlimit ) 
      {
         dconstlow = dconstlowlimit;
      } else {
         if ( dconstlow > dconsthighlimit )
         {
            dconstlow = dconsthighlimit;
         }
      }
      
      if ( dconsthigh < dconstlowlimit ) 
      {
         dconsthigh = dconstlowlimit;
      } else {
         if ( dconsthigh > dconsthighlimit )
         {
            dconsthigh = dconsthighlimit;
         }
      }
      
      // compute spacing
      
      alphaincg = ( ( ( alphahigh - alphalow ) > 0 ) && alphagridpoints ) ? ( alphahigh - alphalow ) / alphagridpoints : 0;
      betaincg = ( ( ( betahigh - betalow ) > 0 ) && betagridpoints ) ? ( betahigh - betalow ) / betagridpoints : 0;
      dconstincg = ( ( ( dconsthigh - dconstlow ) > 0 ) && dconstgridpoints ) ? ( dconsthigh - dconstlow ) / dconstgridpoints : 0;
   }

   cout << 
      QString("ranges for next gsm alpha %1:%2 beta %3:%4 dconst %5:%6\n")
      .arg(alphalow)
      .arg(alphahigh)
      .arg(betalow)
      .arg(betahigh)
      .arg(dconstlow)
      .arg(dconsthigh);

   if ( max_iterations )
   {
      if ( !setup_wgsbs_gsm_f_df(
                                 outfile, 
                                 solutiontag, 
                                 buffertag, 
                                 waxstag,
                                 alphalow,
                                 alphahigh,
                                 alphaincgsm,
                                 betalow,
                                 betahigh,
                                 betaincgsm,
                                 dconstlow,
                                 dconsthigh,
                                 dconstincgsm,
                                 low,
                                 high) ||
           !wgsbs_gsm(nrmsd,
                      alphamin,
                      betamin,
                      dconstmin,
                      gsm_type,
                      max_iterations) )
      {
         return false;
      }

      cout << "nrmsd " << nrmsd << endl;
      QString logbest = 
         QString("minimum %1|%2|%3|%4|%5|%6\n")
         .arg(nrmsd)
         .arg(alphamin)
         .arg(betamin)
         .arg(dconstmin)
         .arg(low)
         .arg(high);

      if ( 
          alphaincg && 
          ( alphamin <= alphalowlimit ||
            alphamin >= alphahighlimit ) )
      {
         cliperrors += "*********** warning alpha pegged at limit ************\n";
      }
      if ( 
          betaincg && 
          ( betamin <= betalowlimit ||
            betamin >= betahighlimit ) )
      {
         cliperrors += "*********** warning beta pegged at limit ************\n";
      }
      if ( 
          dconstincg && 
          ( dconstmin <= dconstlowlimit ||
            dconstmin >= dconsthighlimit ) ) 
      {
         cliperrors += "*********** warning const pegged at limit ************\n";
      }
      
      cout << logbest;
      cout << cliperrors;
   }

   return true;
}

// for sorting by concentration
class sortable_series {
public:
   QString name;
   double sort1;
   double sort2;
   bool operator < (const sortable_series& objIn) const
   {
      return sort1 == objIn.sort1 ? sort2 < objIn.sort2 : sort1 < objIn.sort1 ;
   }
};

bool US_Saxs_Util::wiki(QString &result)
{
   errormsg = "";
   result =
      QString(
              "= %1 =\n"
              "== Overview: %2 ==\n"
              " * %3\n"
              " * MW %4 Daltons\n"
              " * concentration multiplier for standard saxs buffer subtraction: %5\n"
              " * %6\n"
              "== Project source file ==\n"
              "{{{\n%7}}}\n"
              "== Sample summary ==\n"
              "|| name || saxs or waxs || conc (mg/ml) || exposure time (s) || alpha || source file || comments ||\n"
              )
      .arg(p_short_description)
      .arg(p_name)
      .arg(p_description)
      .arg(p_mw)
      .arg(p_conc_mult)
      .arg(p_comment)
      .arg(p_project)
      ;

   // bool any_saxs_at_all = false;
   // bool any_saxs_samples = false;
   bool any_saxs_buffers = false;
   bool any_waxs_at_all = false;
   // bool any_waxs_samples = false;
   bool any_waxs_buffers = false;

   for ( unsigned int i = 0; i < wave_names_vector.size(); i++ )
   {
      result +=
         QString(
                 "|| %1 || %2 || %3 || %4 || %5 || %6 || %7 ||\n"
                 )
         .arg(wave_names_vector[i])
         .arg(wave_types[wave_names_vector[i]])
         .arg(wave_concs[wave_names_vector[i]] ? QString("%1").arg(wave_concs[wave_names_vector[i]]) : "buffer" )
         .arg(wave_exposure_times[wave_names_vector[i]] ? QString("%1").arg(wave_exposure_times[wave_names_vector[i]]) : "" )
         .arg(wave_concs[wave_names_vector[i]] ? QString("%1").arg(wave_alphas[wave_names_vector[i]]) : "" )
         .arg(wave_file_names[wave_names_vector[i]])
         .arg(wave_comments[wave_names_vector[i]])
         ;
      if ( wave_types[wave_names_vector[i]] == "saxs" )
      {
         // any_saxs_at_all = true;
         if ( wave_concs[wave_names_vector[i]] ) 
         {
            // any_saxs_samples = true;
         } else {
            any_saxs_buffers = true;
         }
      }
      if ( wave_types[wave_names_vector[i]] == "waxs" )
      {
         any_waxs_at_all = true;
         if ( wave_concs[wave_names_vector[i]] ) 
         {
            // any_waxs_samples = true;
         } else {
            any_waxs_buffers = true;
         }
      }
   }

   result += 
      "== SAXS standard buffer subtraction ==\n"
      ;

   if ( any_saxs_buffers ) 
   {
      result += 
         "=== SAXS buffer summary ===\n"
         ;

      QString pngfile = QString("%1_saxs_buffers.png").arg(p_name);

      QString cmd =
         QString(
                 "pnggnuplot.pl -c %1 %2 %3"
                 )
         .arg(p_saxs_lowq)
         .arg(p_saxs_highq)
         .arg(QString("pngs%1%2").arg(QDir::separator()).arg(pngfile));

      for ( unsigned int i = 0; i < wave_names_vector.size(); i++ )
      {
         if ( 
             wave_types[wave_names_vector[i]] == "saxs" &&
             !wave_concs[wave_names_vector[i]]
          )
         {
            cmd += " " + QString("wave%1%2").arg(QDir::separator()).arg(wave_file_names[wave_names_vector[i]]);
            result += 
               QString(
                       "|| %1 || %2 || %3 || %4 || %5 || %6 || %7 ||\n"
                       )
               .arg(wave_names_vector[i])
               .arg(wave_types[wave_names_vector[i]])
               .arg(wave_concs[wave_names_vector[i]] ? QString("%1").arg(wave_concs[wave_names_vector[i]]) : "buffer" )
               .arg(wave_exposure_times[wave_names_vector[i]] ? QString("%1").arg(wave_exposure_times[wave_names_vector[i]]) : "" )
               .arg(wave_concs[wave_names_vector[i]] ? QString("%1").arg(wave_alphas[wave_names_vector[i]]) : "" )
               .arg(wave_file_names[wave_names_vector[i]])
               .arg(wave_comments[wave_names_vector[i]])
               ;
         }
      }

      result += QString("[[Image(htdocs:pngs%1%2)]]\n").arg(QDir::separator()).arg(pngfile);
      cmd += "\n";
      cout << cmd;
      system(cmd.toLatin1().data());
   }

   for ( unsigned int i = 0; i < wave_names_vector.size(); i++ )
   {
      if ( 
          wave_types[wave_names_vector[i]] == "saxs" &&
          wave_concs[wave_names_vector[i]]
          )
      {
         any_saxs = true;
         // first the main sample with concentration
         result +=
            QString(
                    "=== Sample %1 ===\n"
                    "|| name || saxs or waxs || conc (mg/ml) || exposure time (s) || alpha || source file || comments ||\n"
                    "|| %2 || %3 || %4 || %5 || %6 || %7 || %8 ||\n"
                    )
            .arg(wave_names_vector[i])
            .arg(wave_names_vector[i])
            .arg(wave_types[wave_names_vector[i]])
            .arg(wave_concs[wave_names_vector[i]] ? QString("%1").arg(wave_concs[wave_names_vector[i]]) : "buffer" )
            .arg(wave_exposure_times[wave_names_vector[i]] ? QString("%1").arg(wave_exposure_times[wave_names_vector[i]]) : "" )
            .arg(wave_concs[wave_names_vector[i]] ? QString("%1").arg(wave_alphas[wave_names_vector[i]]) : "" )
            .arg(wave_file_names[wave_names_vector[i]])
            .arg(wave_comments[wave_names_vector[i]])
            ;
         // then the associated buffers
         for ( unsigned int j = 0; j < wave_buffer_names[wave_names_vector[i]].size(); j++ )
         {
            result += 
               QString(
                       "|| %1 || %2 || %3 || %4 || %5 || %6 || %7 ||\n"
                       )
               .arg(wave_buffer_names[wave_names_vector[i]][j])
               .arg(wave_types[wave_buffer_names[wave_names_vector[i]][j]])
               .arg(wave_concs[wave_buffer_names[wave_names_vector[i]][j]]? QString("%1").arg(wave_concs[wave_buffer_names[wave_names_vector[i]][j]]) : "buffer" )
               .arg(wave_exposure_times[wave_buffer_names[wave_names_vector[i]][j]] ? QString("%1").arg(wave_exposure_times[wave_buffer_names[wave_names_vector[i]][j]]) : "" )
               .arg(wave_concs[wave_buffer_names[wave_names_vector[i]][j]] ? QString("%1").arg(wave_alphas[wave_buffer_names[wave_names_vector[i]][j]]) : "" )
               .arg(wave_file_names[wave_buffer_names[wave_names_vector[i]][j]])
               .arg(wave_comments[wave_buffer_names[wave_names_vector[i]][j]])
               ;
         }
         QString pngfile = QString("%1_sbs_%2.png").arg(p_name).arg(wave_names_vector[i]);
         result += QString("[[Image(htdocs:pngs%1%2)]]\n").arg(QDir::separator()).arg(pngfile);
         // compute the wave now
         QString outfile = get_file_name(wave_names_vector[i],"bsub");
         wave_sb[wave_names_vector[i]] = outfile;
         if ( 
             !subbackground(outfile, wave_names_vector[i], wave_names_vector[i] + "|buffer", wave_alphas[wave_names_vector[i]]) ||
             !write(outfile, outfile) 
             )
         {
            return false;
         }
         QString cmd =
            QString(
                    "pnggnuplot.pl -c %1 %2 %3 %4 %5 %6\n"
                    )
            .arg(p_saxs_lowq)
            .arg(p_saxs_highq)
            .arg(QString("pngs%1%2").arg(QDir::separator()).arg(pngfile))
            .arg(QString("wave%1%2").arg(QDir::separator()).arg(wave_file_names[wave_names_vector[i]]))
            .arg(get_file_name(wave_names_vector[i],"buffer"))
            .arg(get_file_name(wave_names_vector[i],"bsub"))
            ;
         cout << cmd;
         system(cmd.toLatin1().data());
      }
   }

   // saxs concentration series
   // potential exposure time series merge (loop around ?)

   // sort 
   sortable_series tmp_conc;
   sortable_series tmp_exposure_time;
   list < sortable_series > conc_series;
   list < sortable_series > exposure_time_series;
   map < double, bool > conc_map;
   map < double, bool > exposure_time_map;
   list < sortable_series > conc_list;
   list < sortable_series > exposure_time_list;
   vector < double > concs;
   vector < double > exposure_times;

   bool multi_exposure_time = false;
   bool multi_conc = false;
   double last_conc = 0e0;
   double last_exposure_time = 0e0;


   for ( unsigned int i = 0; i < wave_names_vector.size(); i++ )
   {
      if ( 
          wave_types[wave_names_vector[i]] == "saxs" &&
          wave_concs[wave_names_vector[i]]
          )
      {
         conc_map[wave_concs[wave_names_vector[i]]] = true;
         exposure_time_map[wave_exposure_times[wave_names_vector[i]]] = true;

         tmp_conc.name = wave_names_vector[i];
         tmp_conc.sort1 = wave_concs[wave_names_vector[i]];
         tmp_conc.sort2 = wave_exposure_times[wave_names_vector[i]];
         conc_series.push_back(tmp_conc);
         tmp_exposure_time.name = wave_names_vector[i];
         tmp_exposure_time.sort1 = wave_exposure_times[wave_names_vector[i]];
         tmp_exposure_time.sort2 = wave_concs[wave_names_vector[i]];
         exposure_time_series.push_back(tmp_exposure_time);
         if ( !multi_conc && wave_concs[wave_names_vector[i]] )
         {
            if ( last_conc )
            {
               if ( last_conc != wave_concs[wave_names_vector[i]] )
               {
                  multi_conc = true;
               }
            } else {
               last_conc = wave_concs[wave_names_vector[i]];
            }
         }
         if ( !multi_exposure_time && wave_exposure_times[wave_names_vector[i]] )
         {
            if ( last_exposure_time )
            {
               if ( last_exposure_time != wave_exposure_times[wave_names_vector[i]] )
               {
                  multi_exposure_time = true;
               }
            } else {
               last_exposure_time = wave_exposure_times[wave_names_vector[i]];
            }
         }
            
      }
   }
   conc_series.sort();
   exposure_time_series.sort();

   // build a sorted vector of exposure times and concentrations

   tmp_conc.name = "";
   tmp_conc.sort1 = 0e0;
   tmp_conc.sort2 = 0e0;

   for ( map < double, bool >::iterator it = conc_map.begin();
         it != conc_map.end();
         it++ )
   {
      tmp_conc.sort1 = it->first;
      conc_list.push_back(tmp_conc);
   }

   tmp_exposure_time.name = "";
   tmp_exposure_time.sort1 = 0e0;
   tmp_exposure_time.sort2 = 0e0;

   for ( map < double, bool >::iterator it = exposure_time_map.begin();
         it != exposure_time_map.end();
         it++ )
   {
      tmp_exposure_time.sort1 = it->first;
      exposure_time_list.push_back(tmp_exposure_time);
   }

   conc_list.sort();
   exposure_time_list.sort();

   for ( list < sortable_series >::iterator it = conc_list.begin();
         it != conc_list.end();
         it++ )
   {
      concs.push_back(it->sort1);
   }

   for ( list < sortable_series >::iterator it = exposure_time_list.begin();
         it != exposure_time_list.end();
         it++ )
   {
      exposure_times.push_back(it->sort1);
   }

   result += "== SAXS concentration series ==\n";

   result +=
      "|| name || saxs or waxs || conc (mg/ml) || exposure time (s) || alpha || source file || comments ||\n";

   QString pngfile = QString("pngs%1%2_saxs_conc_series.png")
      .arg(QDir::separator())
      .arg(p_name);
      
   QString cmd = 
      QString("pnggnuplot.pl -c %1 %2 %3")
      .arg(p_saxs_lowq)
      .arg(p_saxs_highq)
      .arg(pngfile);

   vector < QString > saxs_conc_names; 

   for ( list < sortable_series >::iterator it = conc_series.begin();
         it != conc_series.end();
         it++ )
   {
      it->name;
      saxs_conc_names.push_back(it->name);
      QString outfile = get_file_name(it->name,"bsub");
      cmd += " " + outfile;
      result +=
         QString(
                 "|| %1 || %2 || %3 || %4 || %5 || %6 || %7 ||\n"
                 )
         .arg(it->name)
         .arg(wave_types[it->name])
         .arg(wave_concs[it->name] ? QString("%1").arg(wave_concs[it->name]) : "buffer" )
         .arg(wave_exposure_times[it->name] ? QString("%1").arg(wave_exposure_times[it->name]) : "" )
         .arg(wave_concs[it->name] ? QString("%1").arg(wave_alphas[it->name]) : "" )
         .arg(QString("%1").arg(outfile).replace(QRegExp("^cwave."),""))
         .arg(wave_comments[it->name])
         ;
   }
   result += QString("[[Image(htdocs:%1)]]\n").arg(pngfile);
   cout << cmd << endl;
   system(cmd.toLatin1().data());

   if ( exposure_times.size() > 1 )
   {
      for ( unsigned int i = 0; i < exposure_times.size(); i++ )
      {
         result += QString("== SAXS concentration series for exposure time %1 s ==\n").arg(exposure_times[i]);
      
         result +=
            "|| name || saxs or waxs || conc (mg/ml) || exposure time (s) || alpha || source file || comments ||\n";
         
         QString pngfile = QString("pngs%1%2_saxs_conc_series_et%3.png")
            .arg(QDir::separator())
            .arg(p_name)
            .arg(exposure_times[i]);
         
         QString cmd = 
            QString("pnggnuplot.pl -c %1 %2 %3")
            .arg(p_saxs_lowq)
            .arg(p_saxs_highq)
            .arg(pngfile);
         
         for ( unsigned int j = 0; j < saxs_conc_names.size(); j++ )
         {
            if ( wave_exposure_times[saxs_conc_names[j]] == exposure_times[i] )
            {
               QString outfile = get_file_name(saxs_conc_names[j],"bsub");
               cmd += " " + outfile;
               result +=
                  QString(
                          "|| %1 || %2 || %3 || %4 || %5 || %6 || %7 ||\n"
                          )
                  .arg(saxs_conc_names[j])
                  .arg(wave_types[saxs_conc_names[j]])
                  .arg(wave_concs[saxs_conc_names[j]] ? QString("%1").arg(wave_concs[saxs_conc_names[j]]) : "buffer" )
                  .arg(wave_exposure_times[saxs_conc_names[j]] ? QString("%1").arg(wave_exposure_times[saxs_conc_names[j]]) : "" )
                  .arg(wave_concs[saxs_conc_names[j]] ? QString("%1").arg(wave_alphas[saxs_conc_names[j]]) : "" )
                  .arg(QString("%1").arg(outfile).replace(QRegExp("^cwave."),""))
                  .arg(wave_comments[saxs_conc_names[j]])
                  ;
            }
         }
         result += QString("[[Image(htdocs:%1)]]\n").arg(pngfile);
         cout << cmd << endl;
         system(cmd.toLatin1().data());
      }
   }

   vector < QString > saxs_exposure_time_names; 

   if ( multi_exposure_time )
   {
      result += "== SAXS exposure time series ==\n";

      result +=
         "|| name || saxs or waxs || conc (mg/ml) || exposure time (s) || alpha || source file || comments ||\n";
      
      QString pngfile = QString("pngs%1%2_saxs_exposure_time_series.png")
         .arg(QDir::separator())
         .arg(p_name);
      
      QString cmd = 
         QString("pnggnuplot.pl -c %1 %2 %3")
         .arg(p_saxs_lowq)
         .arg(p_saxs_highq)
         .arg(pngfile);

      for ( list < sortable_series >::iterator it = exposure_time_series.begin();
            it != exposure_time_series.end();
            it++ )
      {
         it->name;
         saxs_exposure_time_names.push_back(it->name);
         QString outfile = get_file_name(it->name,"bsub");
         cmd += " " + outfile;
         result +=
            QString(
                    "|| %1 || %2 || %3 || %4 || %5 || %6 || %7 ||\n"
                    )
            .arg(it->name)
            .arg(wave_types[it->name])
            .arg(wave_concs[it->name] ? QString("%1").arg(wave_concs[it->name]) : "buffer" )
            .arg(wave_exposure_times[it->name] ? QString("%1").arg(wave_exposure_times[it->name]) : "" )
            .arg(wave_concs[it->name] ? QString("%1").arg(wave_alphas[it->name]) : "" )
            .arg(QString("%1").arg(outfile).replace(QRegExp("^cwave."),""))
            .arg(wave_comments[it->name])
            ;
      }
      result += QString("[[Image(htdocs:%1)]]\n").arg(pngfile);
      cout << cmd << endl;
      system(cmd.toLatin1().data());

      if ( concs.size() > 1 )
      {
         for ( unsigned int i = 0; i < concs.size(); i++ )
         {
            result += QString("== SAXS exposure time series for concentration %1 mg/ml ==\n").arg(concs[i]);
            
            result +=
               "|| name || saxs or waxs || conc (mg/ml) || exposure time (s) || alpha || source file || comments ||\n";
            
            QString pngfile = QString("pngs%1%2_saxs_exposure_time_series_conc%3.png")
               .arg(QDir::separator())
               .arg(p_name)
               .arg(concs[i]);
            
            QString cmd = 
               QString("pnggnuplot.pl -c %1 %2 %3")
               .arg(p_saxs_lowq)
               .arg(p_saxs_highq)
               .arg(pngfile);
            
            for ( unsigned int j = 0; j < saxs_exposure_time_names.size(); j++ )
            {
               if ( wave_concs[saxs_exposure_time_names[j]] == concs[i] )
               {
                  QString outfile = get_file_name(saxs_exposure_time_names[j],"bsub");
                  cmd += " " + outfile;
                  result +=
                     QString(
                             "|| %1 || %2 || %3 || %4 || %5 || %6 || %7 ||\n"
                             )
                     .arg(saxs_exposure_time_names[j])
                     .arg(wave_types[saxs_exposure_time_names[j]])
                     .arg(wave_concs[saxs_exposure_time_names[j]] ? QString("%1").arg(wave_concs[saxs_exposure_time_names[j]]) : "buffer" )
                     .arg(wave_exposure_times[saxs_exposure_time_names[j]] ? QString("%1").arg(wave_exposure_times[saxs_exposure_time_names[j]]) : "" )
                     .arg(wave_concs[saxs_exposure_time_names[j]] ? QString("%1").arg(wave_alphas[saxs_exposure_time_names[j]]) : "" )
                     .arg(QString("%1").arg(outfile).replace(QRegExp("^cwave."),""))
                     .arg(wave_comments[saxs_exposure_time_names[j]])
                     ;
               }
            }
            result += QString("[[Image(htdocs:%1)]]\n").arg(pngfile);
            cout << cmd << endl;
            system(cmd.toLatin1().data());
         }
      }
   }

   // saxs Rg/I0 series

   result += "== SAXS Rg/I0 computations ==\n";

   QString guinier_summary = 
      "=== SAXS Guinier Summary ===\n"
      "|| name || saxs or waxs || conc (mg/ml) || exposure time (s) || alpha || source file || comments || Rg || I0 || I0/conc || qRg min || qRg max || q min || q max || chi2 || start point || end point || points used ||\n"
      ;

   for ( unsigned int i = 0; i < saxs_conc_names.size(); i++ )
   {
      QString outfile = get_file_name(saxs_conc_names[i],"bsub");

      // compute Rg/I0
      QString guinier = get_file_name(outfile, "guinier");

      // these should be parameterized
      int pointsmin = 10;
      int pointsmax = 100;
      double sRgmaxlimit = 1.3e0;
      double pointweightpower = 3e0;
      
      // these are function output values
      double a;
      double b;
      double siga;
      double sigb;
      double chi2;
      double Rg;
      double I0;
      double smin;
      double smax;
      double sRgmin;
      double sRgmax;
      QString log;
      unsigned int beststart;
      unsigned int bestend;
      cout << "computing guinier fit for " << outfile << endl;
      
      if ( !guinier_plot(guinier, outfile) ||
           !write(guinier, guinier) ||
           !guinier_fit2(
                        log,
                        guinier, 
                        pointsmin,
                        pointsmax,
                        sRgmaxlimit,
                        pointweightpower,
                        p_guinier_maxq,
                        a,
                        b,
                        siga,
                        sigb,
                        chi2,
                        Rg,
                        I0,
                        smax, // don't know why these are flipped
                        smin,
                        sRgmin,
                        sRgmax,
                        beststart,
                        bestend
                        ) 
           ) 
      {
         return false;
      }
      wave_Rgs[saxs_conc_names[i]] = Rg;
      wave_I0s[saxs_conc_names[i]] = I0;
      wave_smins[saxs_conc_names[i]] = smin;
      wave_smaxs[saxs_conc_names[i]] = smax;
      wave_sRgmins[saxs_conc_names[i]] = sRgmin;
      wave_sRgmaxs[saxs_conc_names[i]] = sRgmax;
      wave_chi2s[saxs_conc_names[i]] = chi2;

      QString pngfile = QString("pngs%1%2_%3_saxs_guinier.png")
         .arg(QDir::separator())
         .arg(p_name)
         .arg(saxs_conc_names[i]);
      
      QString cmd = 
         QString("pnggnuplot.pl -p 1.5 -g -l points -c %1 %2 -m %3 %4 %5 %6 %7 %8\t\n")
         .arg(wave[guinier].q[beststart] * .2 )
         .arg(wave[guinier].q[bestend] * 1.2 )
         .arg(a)
         .arg(b)
         .arg(smin)
         .arg(smax)
         .arg(pngfile)
         .arg(guinier);
      cout << cmd.toLatin1().data();
      system(cmd.toLatin1().data());

      QString this_data_line = 
         QString(
#if QT_VERSION < 0x040000
                 "|| %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 ||\n"
#else
                 "|| %1 || %2 || %3 || %4 || %5 || %6 || %7 || %8 || %9 || %10 || %11 || %12 || %13 || %14 || %15 || %16 || %17 || %18 ||\n"
#endif
                 )
         .arg(saxs_conc_names[i])
         .arg(wave_types[saxs_conc_names[i]])
         .arg(wave_concs[saxs_conc_names[i]] ? QString("%1").arg(wave_concs[saxs_conc_names[i]]) : "buffer" )
         .arg(wave_exposure_times[saxs_conc_names[i]] ? QString("%1").arg(wave_exposure_times[saxs_conc_names[i]]) : "" )
         .arg(wave_concs[saxs_conc_names[i]] ? QString("%1").arg(wave_alphas[saxs_conc_names[i]]) : "" )
         .arg(QString("%1").arg(outfile).replace(QRegExp("^cwave."),""))
         .arg(wave_comments[saxs_conc_names[i]])
         .arg(Rg)
         .arg(I0)
         .arg(I0/wave_concs[saxs_conc_names[i]])
         .arg(sRgmin)
         .arg(sRgmax)
         .arg(smin)
         .arg(smax)
         .arg(chi2)
         .arg(beststart)
         .arg(bestend)
         .arg(bestend - beststart + 1)
         ;

      result +=
         QString("=== Sample %1 ===\n"
                 "|| name || saxs or waxs || conc (mg/ml) || exposure time (s) || alpha || source file || comments || Rg || I0 || I0/conc || qRg min || qRg max || q min || q max || chi2 || start point || end point || points used ||\n"
                 "%2"
                 "[[Image(htdocs:%3)]]\n"
                 )
         .arg(saxs_conc_names[i])
         .arg(this_data_line)
         .arg(pngfile)
         ;
      guinier_summary += this_data_line;
   }
   result += guinier_summary;




   // waxs subtract background, cell wave

   cout << "waxs subtract background, cell wave\n";

   if ( any_waxs_at_all )
   {
      result +=
         "== WAXS subtract background wave (and empty cell wave, if available) ==\n";
   }

   if ( any_waxs_buffers ) 
   {
      result += 
         "=== WAXS buffer summary ===\n"
         ;

      QString pngfile = QString("%1_waxs_buffers.png").arg(p_name);

      QString cmd =
         QString(
                 "pnggnuplot.pl -c %1 %2 %3"
                 )
         .arg(p_waxs_lowq)
         .arg(p_waxs_highq)
         .arg(QString("pngs%1%2").arg(QDir::separator()).arg(pngfile));

      for ( unsigned int i = 0; i < wave_names_vector.size(); i++ )
      {
         if ( 
             wave_types[wave_names_vector[i]] == "waxs" &&
             !wave_concs[wave_names_vector[i]]
          )
         {
            cmd += " " + QString("wave%1%2").arg(QDir::separator()).arg(wave_file_names[wave_names_vector[i]]);
            result += 
               QString(
                       "|| %1 || %2 || %3 || %4 || %5 || %6 || %7 ||\n"
                       )
               .arg(wave_names_vector[i])
               .arg(wave_types[wave_names_vector[i]])
               .arg(wave_concs[wave_names_vector[i]] ? QString("%1").arg(wave_concs[wave_names_vector[i]]) : "buffer" )
               .arg(wave_exposure_times[wave_names_vector[i]] ? QString("%1").arg(wave_exposure_times[wave_names_vector[i]]) : "" )
               .arg(wave_concs[wave_names_vector[i]] ? QString("%1").arg(wave_alphas[wave_names_vector[i]]) : "" )
               .arg(wave_file_names[wave_names_vector[i]])
               .arg(wave_comments[wave_names_vector[i]])
               ;
         }
      }

      result += QString("[[Image(htdocs:pngs%1%2)]]\n").arg(QDir::separator()).arg(pngfile);
      cmd += "\n";
      cout << cmd;
      system(cmd.toLatin1().data());
   }
      
   // for each waxs sample with nonzero concentration

   for ( unsigned int i = 0; i < wave_names_vector.size(); i++ )
   {
      if ( 
          wave_types[wave_names_vector[i]] == "waxs" &&
          wave_concs[wave_names_vector[i]] &&
          wave_alpha_starts[wave_names_vector[i]] &&
          wave_alpha_ends[wave_names_vector[i]] &&
          wave_alpha_incs[wave_names_vector[i]]
          )
      {
         result +=
            QString(
                    "=== Sample %1 ===\n"
                    "|| name || saxs or waxs || conc (mg/ml) || exposure time (s) || alpha || source file || comments ||\n"
                    "|| %2 || %3 || %4 || %5 || %6 || %7 || %8 ||\n"
                    )
            .arg(wave_names_vector[i])
            .arg(wave_names_vector[i])
            .arg(wave_types[wave_names_vector[i]])
            .arg(wave_concs[wave_names_vector[i]] ? QString("%1").arg(wave_concs[wave_names_vector[i]]) : "buffer" )
            .arg(wave_exposure_times[wave_names_vector[i]] ? QString("%1").arg(wave_exposure_times[wave_names_vector[i]]) : "" )
            .arg(wave_concs[wave_names_vector[i]] ? QString("%1").arg(wave_alphas[wave_names_vector[i]]) : "" )
            .arg(wave_file_names[wave_names_vector[i]])
            .arg(wave_comments[wave_names_vector[i]])
            ;

         // then the associated buffers
         for ( unsigned int j = 0; j < wave_buffer_names[wave_names_vector[i]].size(); j++ )
         {
            result += 
               QString(
                       "|| %1 || %2 || %3 || %4 || %5 || %6 || %7 ||\n"
                       )
               .arg(wave_buffer_names[wave_names_vector[i]][j])
               .arg(wave_types[wave_buffer_names[wave_names_vector[i]][j]])
               .arg(wave_concs[wave_buffer_names[wave_names_vector[i]][j]]? QString("%1").arg(wave_concs[wave_buffer_names[wave_names_vector[i]][j]]) : "buffer" )
               .arg(wave_exposure_times[wave_buffer_names[wave_names_vector[i]][j]] ? QString("%1").arg(wave_exposure_times[wave_buffer_names[wave_names_vector[i]][j]]) : "" )
               .arg(wave_concs[wave_buffer_names[wave_names_vector[i]][j]] ? QString("%1").arg(wave_alphas[wave_buffer_names[wave_names_vector[i]][j]]) : "" )
               .arg(wave_file_names[wave_buffer_names[wave_names_vector[i]][j]])
               .arg(wave_comments[wave_buffer_names[wave_names_vector[i]][j]])
               ;
         }

         // then the associated empty cell
         bool hasEmpty = wave_empty_names.count(wave_names_vector[i]) && wave_empty_names[wave_names_vector[i]].size();
         if ( hasEmpty )
         {
            cout << "empty cell found\n";
            for ( unsigned int j = 0; j < wave_empty_names[wave_names_vector[i]].size(); j++ )
            {
               result += 
                  QString(
                          "|| %1 || %2 || %3 || %4 || %5 || %6 || %7 ||\n"
                          )
                  .arg(wave_empty_names[wave_names_vector[i]][j])
                  .arg(wave_types[wave_empty_names[wave_names_vector[i]][j]])
                  .arg(wave_concs[wave_empty_names[wave_names_vector[i]][j]]? QString("%1").arg(wave_concs[wave_empty_names[wave_names_vector[i]][j]]) : "buffer" )
                  .arg(wave_exposure_times[wave_empty_names[wave_names_vector[i]][j]] ? QString("%1").arg(wave_exposure_times[wave_empty_names[wave_names_vector[i]][j]]) : "" )
                  .arg(wave_concs[wave_empty_names[wave_names_vector[i]][j]] ? QString("%1").arg(wave_alphas[wave_empty_names[wave_names_vector[i]][j]]) : "" )
                  .arg(wave_file_names[wave_empty_names[wave_names_vector[i]][j]])
                  .arg(wave_comments[wave_empty_names[wave_names_vector[i]][j]])
                  ;
            }
         } else {
            cout << "No empty cell found\n";
         }

         bool hasPresetAlpha = wave_alphas[wave_names_vector[i]] != 0;
         any_waxs |= hasPresetAlpha;
         cout << ( hasPresetAlpha ? "has preset alpha\n" : "no preset alpha found, visually determine\n" );

         QString pngfile = QString("%1_wsbc_%2.png").arg(p_name).arg(wave_names_vector[i]);
         result += QString("[[Image(htdocs:pngs%1%2)]]\n").arg(QDir::separator()).arg(pngfile);

         // the zoomed in version
         QString pngfilez = QString("%1_wsbc_%2z.png").arg(p_name).arg(wave_names_vector[i]);
         result += QString("[[Image(htdocs:pngs%1%2)]]\n").arg(QDir::separator()).arg(pngfilez);

         // if hasPresetAlpha
         QString pngfilea = QString("%1_wsbc_%2a.png").arg(p_name).arg(wave_names_vector[i]);
         QString pngfileaz = QString("%1_wsbc_%2az.png").arg(p_name).arg(wave_names_vector[i]);
         if ( hasPresetAlpha )
         {
            result += 
               QString("=== Visually chosen alpha %1 ===\n[[Image(htdocs:pngs%2%3)]]\n")
               .arg(wave_alphas[wave_names_vector[i]])
               .arg(QDir::separator()).arg(pngfilea);
            result += QString("[[Image(htdocs:pngs%1%2)]]\n").arg(QDir::separator()).arg(pngfileaz);
         }

         // compute the wave(s) now

         QString cmd =
            QString(
                    "pnggnuplot.pl -c %1 %2 %3 %4 %5"
                    )
            .arg(p_waxs_lowq)
            .arg(p_waxs_highq)
            .arg(QString("pngs%1%2").arg(QDir::separator()).arg(pngfile))
            .arg(QString("wave%1%2").arg(QDir::separator()).arg(wave_file_names[wave_names_vector[i]]))
            .arg(get_file_name(wave_names_vector[i],"buffer"));

         QString cmdz =
            QString(
                    "pnggnuplot.pl -c %1 %2 %3 %4 %5"
                    )
            .arg(p_waxs_zlowq)
            .arg(p_waxs_zhighq)
            .arg(QString("pngs%1%2").arg(QDir::separator()).arg(pngfilez))
            .arg(QString("wave%1%2").arg(QDir::separator()).arg(wave_file_names[wave_names_vector[i]]))
            .arg(get_file_name(wave_names_vector[i],"buffer"));

         QString cmda =
            QString(
                    "pnggnuplot.pl -c %1 %2 %3 %4 %5"
                    )
            .arg(p_waxs_lowq)
            .arg(p_waxs_highq)
            .arg(QString("pngs%1%2").arg(QDir::separator()).arg(pngfilea))
            .arg(QString("wave%1%2").arg(QDir::separator()).arg(wave_file_names[wave_names_vector[i]]))
            .arg(get_file_name(wave_names_vector[i],"buffer"));

         QString cmdaz =
            QString(
                    "pnggnuplot.pl -c %1 %2 %3 %4 %5"
                    )
            .arg(p_waxs_zlowq)
            .arg(p_waxs_zhighq)
            .arg(QString("pngs%1%2").arg(QDir::separator()).arg(pngfileaz))
            .arg(QString("wave%1%2").arg(QDir::separator()).arg(wave_file_names[wave_names_vector[i]]))
            .arg(get_file_name(wave_names_vector[i],"buffer"));

         if ( hasEmpty )
         {
            cmd += QString(" %1").arg(get_file_name(wave_names_vector[i],"empty"));
            cmdz += QString(" %1").arg(get_file_name(wave_names_vector[i],"empty"));
         }

         for ( double alpha = wave_alpha_starts[wave_names_vector[i]];
               alpha <= wave_alpha_ends[wave_names_vector[i]];
               alpha += wave_alpha_incs[wave_names_vector[i]] )
         {
            cout << QString("processing %1 for alpha %2\n").arg(wave_names_vector[i]).arg(alpha);
            QString outfile = get_file_name(wave_names_vector[i],QString("wsbc_a%1").arg(alpha));
            cmd += " " + outfile;
            cmdz += " " + outfile;
            if ( hasEmpty )
            {
               if ( 
                   !subcellwave(outfile, 
                                wave_names_vector[i], 
                                wave_names_vector[i] + "|buffer",
                                wave_names_vector[i] + "|empty",
                                alpha) ||
                   !write(outfile, outfile)
                   )
               {
                  return false;
               }
            } else {
               if ( 
                   !subbackground(outfile,
                                  wave_names_vector[i],
                                  wave_names_vector[i] + "|buffer",
                                  alpha) ||
                   !write(outfile, outfile) 
                   )
               {
                  return false;
               }
            }

         }
         if ( hasPresetAlpha )
         {
            double alpha = wave_alphas[wave_names_vector[i]];
            cout << QString("processing %1 for alpha %2\n").arg(wave_names_vector[i]).arg(alpha);
            QString outfile = get_file_name(wave_names_vector[i],QString("wsbc_a%1").arg(alpha));
            wave_sb[wave_names_vector[i]] = outfile;
            cmda += " " + outfile;
            cmdaz += " " + outfile;
            if ( hasEmpty )
            {
               if ( 
                   !subcellwave(outfile, 
                                wave_names_vector[i], 
                                wave_names_vector[i] + "|buffer",
                                wave_names_vector[i] + "|empty",
                                alpha) ||
                   !write(outfile, outfile)
                   )
               {
                  return false;
               }
            } else {
               if ( 
                   !subbackground(outfile,
                                  wave_names_vector[i],
                                  wave_names_vector[i] + "|buffer",
                                  alpha) ||
                   !write(outfile, outfile) 
                   )
               {
                  return false;
               }
            }
         }

         cmd += "\n";
         cout << cmd;
         system(cmd.toLatin1().data());
         cmdz += "\n";
         cout << cmdz;
         system(cmdz.toLatin1().data());
         if ( hasPresetAlpha )
         {
            cmda += "\n";
            cout << cmda;
            system(cmda.toLatin1().data());
            cmdaz += "\n";
            cout << cmdaz;
            system(cmdaz.toLatin1().data());
         }
      }
   }

   // waxs guided saxs background subtraction
   cout << "wave list:\n" << list_waves();
   cout << "waxs guided saxs background subtraction\n";

   bool any_wgsbs = false;

   if ( any_waxs )
   {
      for ( unsigned int i = 0; i < wave_names_vector.size(); i++ )
      {
         if ( 
             wave_types[wave_names_vector[i]] == "saxs" &&
             wave_concs[wave_names_vector[i]]
             )
         {
            for ( unsigned int j = 0; j < wave_names_vector.size(); j++ )
            {
               if ( 
                   wave_types[wave_names_vector[j]] == "waxs" &&
                   wave_alphas[wave_names_vector[j]]
                   )
               {
                  // ok, we have a waxs & saxs buffer
                  if ( !any_wgsbs )
                  {
                     any_wgsbs = true;
                     result += "== WAXS guided saxs background subtraction ==\n";
                  }
                  
                  if ( !wave_sb.count(wave_names_vector[i]) ||
                       !wave_sb.count(wave_names_vector[j]) )
                  {
                     errormsg = "internal error: we should not be here without the subtracted saxs & waxs waves available";
                     return false;
                  }

                  // perform fit

                  QString outfile = get_file_name(wave_names_vector[i],QString("%1_wgsbs").arg(wave_names_vector[j]));
                  cout << "output is :" << outfile << endl;
                  wave_wgsbs[wave_names_vector[i]] = outfile;

                  // us_cmdline_t saxs_wgsbsnggsm all_00207.dat glog_bsa2_41 rbsa2_41_avg.dat rbsabuf1_39_avg.dat all_00200.dat 10 1 50 50 0.98 1.03 .005 .005 1 1000 2.5 1 1e-6 1e-6 0 0 .1 .23

                  double nrmsd;
                  double alphamin;
                  double betamin;
                  double dconstmin;
                  QString cliperrors;

                  cout << QString("wgsbs files: %1 %2 %3\n")
                     .arg(wave_sb[wave_names_vector[i]])
                     .arg(wave_names_vector[i] + "|buffer")
                     .arg(wave_sb[wave_names_vector[j]]);

                  double use_overlap_lowq = 
                     wave_overlap_lowq[wave_names_vector[i]] ?
                     wave_overlap_lowq[wave_names_vector[i]] :
                     p_overlap_lowq;

                  double use_overlap_highq = 
                     wave_overlap_highq[wave_names_vector[i]] ?
                     wave_overlap_highq[wave_names_vector[i]] :
                     p_overlap_highq;

                  if ( !compute_wgsbs(
                                      outfile,
                                      wave_names_vector[i],
                                      wave_names_vector[i] + "|buffer",
                                      wave_sb[wave_names_vector[j]],
                                      p_iterations_grid,
                                      1,
                                      p_iterations_gsm,
                                      50,
                                      p_alpha_min,
                                      p_alpha_max,
                                      .005e0,
                                      .005e0,
                                      1,
                                      1000,
                                      2.5e0,
                                      1e0,
                                      1e-6,
                                      1e-6,
                                      0e0,
                                      0e0,
                                      use_overlap_lowq,
                                      use_overlap_highq,
                                      nrmsd,
                                      alphamin,
                                      betamin,
                                      dconstmin,
                                      cliperrors
                                      ) ||
                       !write(outfile, outfile)
                       )
                  {
                     return false;
                  }

                  if ( betamin == 0 )
                  {
                     errormsg = "beta is zero, error";
                     return false;
                  }

                  QString unscaled_outfile = get_file_name(wave_names_vector[i],QString("%1_wgsbs_unscaled").arg(wave_names_vector[j]));
                  
                  scale(unscaled_outfile, outfile, 1e0 / betamin);

                  if ( !write(unscaled_outfile, unscaled_outfile) )
                  {
                     return false;
                  }

                  wave_wgsbs_unscaled[wave_names_vector[i]] = unscaled_outfile;

                  cliperrors = 
                     QString("Grid iterations %1[[br]]GSM iterations %2%3")
                     .arg(p_iterations_grid)
                     .arg(p_iterations_gsm)
                     .arg(cliperrors.isEmpty() ? "" : QString("[[br]]%1").arg(cliperrors));

                  if ( use_overlap_lowq != p_overlap_lowq ||
                       use_overlap_highq != p_overlap_highq )
                  {
                     cliperrors += QString("[[br]]overlap fit range %1 %2")
                        .arg(use_overlap_lowq)
                        .arg(use_overlap_highq);
                  }

                  result += 
                     QString(
#if QT_VERSION < 0x040000
                             "=== Sample SAXS %1 WAXS %1 ===\n"
                             "|| name || saxs or waxs || conc (mg/ml) || exposure time (s) || alpha || source file || comments || alpha || beta || constant || nrmsd || fit comments ||\n"
                             "|| %1 || %1 || %1 || %1 || %1 || %1 || %1 || || || || || ||\n"
                             "|| %1 || %1 || %1 || %1 || %1 || %1 || %1 || || || || || ||\n"
                             "|| %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 ||\n"
#else
                             "=== Sample SAXS %1 WAXS %2 ===\n"
                             "|| name || saxs or waxs || conc (mg/ml) || exposure time (s) || alpha || source file || comments || alpha || beta || constant || nrmsd || fit comments ||\n"
                             "|| %3 || %4 || %5 || %6 || %7 || %8 || %9 || || || || || ||\n"
                             "|| %10 || %11 || %12 || %13 || %14 || %15 || %16 || || || || || ||\n"
                             "|| %17 || %18 || %19 || %20 || %21 || %22 || %23 || %24 || %25 || %26 || %27 || %28 ||\n"
#endif
                             )
                     .arg(wave_names_vector[i])
                     .arg(wave_names_vector[j])

                     .arg(wave_names_vector[i])
                     .arg(wave_types[wave_names_vector[i]])
                     .arg(wave_concs[wave_names_vector[i]] ? QString("%1").arg(wave_concs[wave_names_vector[i]]) : "buffer" )
                     .arg(wave_exposure_times[wave_names_vector[i]] ? QString("%1").arg(wave_exposure_times[wave_names_vector[i]]) : "" )
                     .arg(wave_concs[wave_names_vector[i]] ? QString("%1").arg(wave_alphas[wave_names_vector[i]]) : "" )
                     .arg(wave_file_names[wave_sb[wave_names_vector[i]]])
                     .arg(wave_comments[wave_names_vector[i]])

                     .arg(wave_names_vector[j])
                     .arg(wave_types[wave_names_vector[j]])
                     .arg(wave_concs[wave_names_vector[j]] ? QString("%1").arg(wave_concs[wave_names_vector[j]]) : "buffer" )
                     .arg(wave_exposure_times[wave_names_vector[j]] ? QString("%1").arg(wave_exposure_times[wave_names_vector[j]]) : "" )
                     .arg(wave_concs[wave_names_vector[j]] ? QString("%1").arg(wave_alphas[wave_names_vector[j]]) : "" )
                     .arg(wave_file_names[wave_sb[wave_names_vector[j]]])
                     .arg(wave_comments[wave_names_vector[j]])

                     .arg(QString("%1").arg(outfile).replace(QRegExp("\\.dat$"),"").replace(QRegExp("^cwave."),""))
                     .arg("saxs plus waxs")
                     .arg(wave_concs[wave_names_vector[i]] ? QString("%1").arg(wave_concs[wave_names_vector[i]]) : "buffer" )
                     .arg(wave_exposure_times[wave_names_vector[i]] ? QString("%1").arg(wave_exposure_times[wave_names_vector[i]]) : "" )
                     .arg(wave_concs[wave_names_vector[i]] ? QString("%1").arg(wave_alphas[wave_names_vector[i]]) : "" )
                     .arg(outfile)
                     .arg(wave_comments[wave_names_vector[i]])

                     .arg(alphamin)
                     .arg(betamin)
                     .arg(dconstmin)
                     .arg(nrmsd)
                     .arg(cliperrors);

                  QString pngfile = QString("%1_%2_%3_wgsbs.png").arg(p_name).arg(wave_names_vector[i]).arg(wave_names_vector[j]);
                  QString pngfilez = QString("%1_%2_%3_wgsbsz.png").arg(p_name).arg(wave_names_vector[i]).arg(wave_names_vector[j]);
                  result += QString("[[Image(htdocs:pngs%1%2)]]\n").arg(QDir::separator()).arg(pngfile);
                  result += QString("[[Image(htdocs:pngs%1%2)]]\n").arg(QDir::separator()).arg(pngfilez);

                  QString cmd =
                     QString(
                             "pnggnuplot.pl %1 %2 %3"
                             )
                     .arg(QString("pngs%1%2").arg(QDir::separator()).arg(pngfile))
                     .arg(outfile)
                     .arg(wave_sb[wave_names_vector[j]]);

                  QString cmdz =
                     QString(
                             "pnggnuplot.pl -c %1 %2 %3 %4 %5"
                             )
                     .arg(use_overlap_lowq)
                     .arg(use_overlap_highq)
                     .arg(QString("pngs%1%2").arg(QDir::separator()).arg(pngfilez))
                     .arg(outfile)
                     .arg(wave_sb[wave_names_vector[j]]);

                  cmd += "\n";
                  cout << cmd;
                  system(cmd.toLatin1().data());
                  cmdz += "\n";
                  cout << cmdz;
                  system(cmdz.toLatin1().data());

                  // join waves
                  cout << "joining waves\n";

                  QString joinfile = get_file_name(wave_names_vector[i],QString("%1_wgsbs_join").arg(wave_names_vector[j]));
                  if ( !join(
                             joinfile, 
                             outfile,
                             wave_sb[wave_names_vector[j]],
                             p_join_q
                             ) ||
                       !write(joinfile, joinfile) )
                  {
                     return false;
                  }
                  wave_join[wave_names_vector[i]] = joinfile;
               }
            }
         }
      }
   }

   if ( any_wgsbs )
   {
      
      // wgsbs saxs concentration series
      // potential exposure time series merge (loop around ?)
      
      result += "== WAXS guided SAXS buffer subtraction concentration series ==\n";
      
      result +=
         "|| name || saxs or waxs || conc (mg/ml) || exposure time (s) || alpha || source file || comments ||\n";
      
      QString pngfile = QString("pngs%1%2_saxs_wgsbs_conc_series.png")
         .arg(QDir::separator())
         .arg(p_name);
      
      QString pngfilez = QString("pngs%1%2_saxs_wgsbs_conc_seriesz.png")
         .arg(QDir::separator())
         .arg(p_name);
      
      QString cmd = 
         QString("pnggnuplot.pl -c %1 %2 %3")
         .arg(p_waxs_lowq)
         .arg(p_waxs_highq)
         .arg(pngfile);

      QString cmdz = 
         QString("pnggnuplot.pl -c %1 %2 %3")
         .arg(p_saxs_lowq)
         .arg(p_saxs_highq)
         .arg(pngfilez);
      
      for ( unsigned int i = 0; i < saxs_conc_names.size(); i++ )
      {
         if ( 
             wave_join.count(saxs_conc_names[i]) &&
             !wave_join[saxs_conc_names[i]].isEmpty()
             )
         {
            QString outfile = wave_join[saxs_conc_names[i]];
            cmd += " " + outfile;
            cmdz += " " + outfile;
            result +=
               QString(
                       "|| %1 || %2 || %3 || %4 || %5 || %6 || %7 ||\n"
                       )
               .arg(saxs_conc_names[i])
               .arg(wave_types[saxs_conc_names[i]])
               .arg(wave_concs[saxs_conc_names[i]] ? QString("%1").arg(wave_concs[saxs_conc_names[i]]) : "buffer" )
               .arg(wave_exposure_times[saxs_conc_names[i]] ? QString("%1").arg(wave_exposure_times[saxs_conc_names[i]]) : "" )
               .arg(wave_concs[saxs_conc_names[i]] ? QString("%1").arg(wave_alphas[saxs_conc_names[i]]) : "" )
               .arg(QString("%1").arg(outfile).replace(QRegExp("^cwave."),""))
               .arg(wave_comments[saxs_conc_names[i]])
               ;
         }
      }
      result += QString("[[Image(htdocs:%1)]]\n").arg(pngfile);
      cout << cmd << endl;
      system(cmd.toLatin1().data());

      result += QString("[[Image(htdocs:%1)]]\n").arg(pngfilez);
      cout << cmdz << endl;
      system(cmdz.toLatin1().data());
      
      // wgsbs Rg/I0 series
      
      result += "== WAXS guided SAXS buffer subtraction Rg/I0 computations ==\n";
      
      QString wgsbs_guinier_summary = 
         "=== WGSBS SAXS Guinier Summary ===\n"
         "|| name || saxs or waxs || conc (mg/ml) || exposure_time (s) || alpha || source file || comments || Rg || I0 || I0/conc || qRg min || qRg max || q min || q max || chi2 || start point || end point || points used ||\n"
         ;
      
      for ( unsigned int i = 0; i < saxs_conc_names.size(); i++ )
      {
         if ( 
             wave_join.count(saxs_conc_names[i]) &&
             !wave_join[saxs_conc_names[i]].isEmpty()
             )
         {
            QString outfile = wave_join[saxs_conc_names[i]];
            
            // compute Rg/I0
            QString guinier = get_file_name(outfile, "wgsbs_guinier");
            
            // these should be parameterized
            int pointsmin = 10;
            int pointsmax = 100;
            double sRgmaxlimit = 1.3e0;
            double pointweightpower = 3e0;
            
            // these are function output values
            double a;
            double b;
            double siga;
            double sigb;
            double chi2;
            double Rg;
            double I0;
            double smin;
            double smax;
            double sRgmin;
            double sRgmax;
            QString log;
            unsigned int beststart;
            unsigned int bestend;
            cout << "computing guinier fit for " << outfile << endl;
            
            if ( !guinier_plot(guinier, outfile) ||
                 !write(guinier, guinier) ||
                 !guinier_fit2(
                               log,
                               guinier, 
                               pointsmin,
                               pointsmax,
                               sRgmaxlimit,
                               pointweightpower,
                               p_guinier_maxq,
                               a,
                               b,
                               siga,
                               sigb,
                               chi2,
                               Rg,
                               I0,
                               smax, // don't know why these are flipped
                               smin,
                               sRgmin,
                               sRgmax,
                               beststart,
                               bestend
                               ) 
                 ) 
            {
               return false;
            }
            wave_Rgs[saxs_conc_names[i]] = Rg;
            wave_I0s[saxs_conc_names[i]] = I0;
            wave_smins[saxs_conc_names[i]] = smin;
            wave_smaxs[saxs_conc_names[i]] = smax;
            wave_sRgmins[saxs_conc_names[i]] = sRgmin;
            wave_sRgmaxs[saxs_conc_names[i]] = sRgmax;
            wave_chi2s[saxs_conc_names[i]] = chi2;
            
            QString pngfile = QString("pngs%1%2_%3_saxs_wgsbs_guinier.png")
               .arg(QDir::separator())
               .arg(p_name)
               .arg(saxs_conc_names[i]);
            
            QString cmd = 
               QString("pnggnuplot.pl -p 1.5 -g -l points -c %1 %2 -m %3 %4 %5 %6 %7 %8\t\n")
               .arg(wave[guinier].q[beststart] * .2 )
               .arg(wave[guinier].q[bestend] * 1.2 )
               .arg(a)
               .arg(b)
               .arg(smin)
               .arg(smax)
               .arg(pngfile)
               .arg(guinier);
            cout << cmd.toLatin1().data();
            system(cmd.toLatin1().data());
            
            QString this_data_line = 
               QString(
#if QT_VERSION < 0x040000
                       "|| %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 ||\n"
#else
                       "|| %1 || %2 || %3 || %4 || %5 || %6 || %7 || %8 || %9 || %10 || %11 || %12 || %13 || %14 || %15 || %16 || %17 || %18 ||\n"
#endif
                       )
               .arg(saxs_conc_names[i])
               .arg(wave_types[saxs_conc_names[i]])
               .arg(wave_concs[saxs_conc_names[i]] ? QString("%1").arg(wave_concs[saxs_conc_names[i]]) : "buffer" )
               .arg(wave_exposure_times[saxs_conc_names[i]] ? QString("%1").arg(wave_exposure_times[saxs_conc_names[i]]) : "" )
               .arg(wave_concs[saxs_conc_names[i]] ? QString("%1").arg(wave_alphas[saxs_conc_names[i]]) : "" )
               .arg(QString("%1").arg(outfile).replace(QRegExp("^cwave."),""))
               .arg(wave_comments[saxs_conc_names[i]])
               .arg(Rg)
               .arg(I0)
               .arg(I0/wave_concs[saxs_conc_names[i]])
               .arg(sRgmin)
               .arg(sRgmax)
               .arg(smin)
               .arg(smax)
               .arg(chi2)
               .arg(beststart)
               .arg(bestend)
               .arg(bestend - beststart + 1)
               ;
            
            result +=
               QString("=== Sample %1 ===\n"
                       "|| name || saxs or waxs || conc (mg/ml) || exposure time (s) || alpha || source file || comments || Rg || I0 || I0/conc || qRg min || qRg max || q min || q max || chi2 || start point || end point || points used ||\n"
                       "%2"
                       "[[Image(htdocs:%3)]]\n"
                       )
               .arg(saxs_conc_names[i])
               .arg(this_data_line)
               .arg(pngfile)
               ;
            wgsbs_guinier_summary += this_data_line;
         }
      }

      // wgsbs unscaled saxs concentration series
      // potential exposure time series merge (loop around ?)
      
      result += "== WAXS guided SAXS buffer subtraction unscaled concentration series ==\n";
      
      result +=
         "|| name || saxs or waxs || conc (mg/ml) || exposure time (s) || alpha || source file || comments ||\n";
      
      pngfile = QString("pngs%1%2_saxs_wgsbs_unscaled_conc_series.png")
         .arg(QDir::separator())
         .arg(p_name);
      
      cmd = 
         QString("pnggnuplot.pl -c %1 %2 %3")
         .arg(p_saxs_lowq)
         .arg(p_saxs_highq)
         .arg(pngfile);
      
      for ( unsigned int i = 0; i < saxs_conc_names.size(); i++ )
      {
         if ( 
             wave_wgsbs_unscaled.count(saxs_conc_names[i]) &&
             !wave_wgsbs_unscaled[saxs_conc_names[i]].isEmpty()
             )
         {
            QString outfile = wave_wgsbs_unscaled[saxs_conc_names[i]];
            cmd += " " + outfile;
            cmdz += " " + outfile;
            result +=
               QString(
                       "|| %1 || %2 || %3 || %4 || %5 || %6 || %7 ||\n"
                       )
               .arg(saxs_conc_names[i])
               .arg(wave_types[saxs_conc_names[i]])
               .arg(wave_concs[saxs_conc_names[i]] ? QString("%1").arg(wave_concs[saxs_conc_names[i]]) : "buffer" )
               .arg(wave_exposure_times[saxs_conc_names[i]] ? QString("%1").arg(wave_exposure_times[saxs_conc_names[i]]) : "" )
               .arg(wave_concs[saxs_conc_names[i]] ? QString("%1").arg(wave_alphas[saxs_conc_names[i]]) : "" )
               .arg(QString("%1").arg(outfile).replace(QRegExp("^cwave."),""))
               .arg(wave_comments[saxs_conc_names[i]])
               ;
         }
      }
      result += QString("[[Image(htdocs:%1)]]\n").arg(pngfile);
      cout << cmd << endl;
      system(cmd.toLatin1().data());
      
      // wgsbs Rg/I0 series
      
      result += "== WAXS guided SAXS buffer subtraction unscaled Rg/I0 computations ==\n";
      
      QString wgsbs_unscaled_guinier_summary = 
         "=== WGSBS SAXS Unscaled Guinier Summary ===\n"
         "|| name || saxs or waxs || conc (mg/ml) || exposure time (s) || alpha || source file || comments || Rg || I0 || I0/conc || qRg min || qRg max || q min || q max || chi2 || start point || end point || points used ||\n"
         ;
      
      for ( unsigned int i = 0; i < saxs_conc_names.size(); i++ )
      {
         if ( 
             wave_wgsbs_unscaled.count(saxs_conc_names[i]) &&
             !wave_wgsbs_unscaled[saxs_conc_names[i]].isEmpty()
             )
         {
            QString outfile = wave_wgsbs_unscaled[saxs_conc_names[i]];
            
            // compute Rg/I0
            QString guinier = get_file_name(outfile, "wgsbs_unscaled_guinier");
            
            // these should be parameterized
            int pointsmin = 10;
            int pointsmax = 100;
            double sRgmaxlimit = 1.3e0;
            double pointweightpower = 3e0;
            
            // these are function output values
            double a;
            double b;
            double siga;
            double sigb;
            double chi2;
            double Rg;
            double I0;
            double smin;
            double smax;
            double sRgmin;
            double sRgmax;
            QString log;
            unsigned int beststart;
            unsigned int bestend;
            cout << "computing guinier fit for " << outfile << endl;
            
            if ( !guinier_plot(guinier, outfile) ||
                 !write(guinier, guinier) ||
                 !guinier_fit2(
                               log,
                               guinier, 
                               pointsmin,
                               pointsmax,
                               sRgmaxlimit,
                               pointweightpower,
                               p_guinier_maxq,
                               a,
                               b,
                               siga,
                               sigb,
                               chi2,
                               Rg,
                               I0,
                               smax, // don't know why these are flipped
                               smin,
                               sRgmin,
                               sRgmax,
                               beststart,
                               bestend
                               ) 
                 ) 
            {
               return false;
            }
            wave_Rgs[saxs_conc_names[i]] = Rg;
            wave_I0s[saxs_conc_names[i]] = I0;
            wave_smins[saxs_conc_names[i]] = smin;
            wave_smaxs[saxs_conc_names[i]] = smax;
            wave_sRgmins[saxs_conc_names[i]] = sRgmin;
            wave_sRgmaxs[saxs_conc_names[i]] = sRgmax;
            wave_chi2s[saxs_conc_names[i]] = chi2;
            
            QString pngfile = QString("pngs%1%2_%3_saxs_wgsbs_unscaled_guinier.png")
               .arg(QDir::separator())
               .arg(p_name)
               .arg(saxs_conc_names[i]);
            
            QString cmd = 
               QString("pnggnuplot.pl -p 1.5 -g -l points -c %1 %2 -m %3 %4 %5 %6 %7 %8\t\n")
               .arg(wave[guinier].q[beststart] * .2 )
               .arg(wave[guinier].q[bestend] * 1.2 )
               .arg(a)
               .arg(b)
               .arg(smin)
               .arg(smax)
               .arg(pngfile)
               .arg(guinier);
            cout << cmd.toLatin1().data();
            system(cmd.toLatin1().data());
            
            QString this_data_line = 
               QString(
#if QT_VERSION < 0x040000
                       "|| %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 ||\n"
#else
                       "|| %1 || %2 || %3 || %4 || %5 || %6 || %7 || %8 || %9 || %10 || %11 || %12 || %13 || %14 || %15 || %16 || %17 || %18 ||\n"
#endif
                       )
               .arg(saxs_conc_names[i])
               .arg(wave_types[saxs_conc_names[i]])
               .arg(wave_concs[saxs_conc_names[i]] ? QString("%1").arg(wave_concs[saxs_conc_names[i]]) : "buffer" )
               .arg(wave_exposure_times[saxs_conc_names[i]] ? QString("%1").arg(wave_exposure_times[saxs_conc_names[i]]) : "" )
               .arg(wave_concs[saxs_conc_names[i]] ? QString("%1").arg(wave_alphas[saxs_conc_names[i]]) : "" )
               .arg(QString("%1").arg(outfile).replace(QRegExp("^cwave."),""))
               .arg(wave_comments[saxs_conc_names[i]])
               .arg(Rg)
               .arg(I0)
               .arg(I0/wave_concs[saxs_conc_names[i]])
               .arg(sRgmin)
               .arg(sRgmax)
               .arg(smin)
               .arg(smax)
               .arg(chi2)
               .arg(beststart)
               .arg(bestend)
               .arg(bestend - beststart + 1)
               ;
            
            result +=
               QString("=== Sample %1 ===\n"
                       "|| name || saxs or waxs || conc (mg/ml) || exposure time (s) || alpha || source file || comments || Rg || I0 || I0/conc || qRg min || qRg max || q min || q max || chi2 || start point || end point || points used ||\n"
                       "%2"
                       "[[Image(htdocs:%3)]]\n"
                       )
               .arg(saxs_conc_names[i])
               .arg(this_data_line)
               .arg(pngfile)
               ;
            wgsbs_unscaled_guinier_summary += this_data_line;
         }
      }
      result += wgsbs_guinier_summary;
      result += wgsbs_unscaled_guinier_summary;
      result += guinier_summary;
   }
   return true;
}

bool US_Saxs_Util::merge_projects( 
                                  QString            outfile, 
                                  double             reference_mw_multiplier, 
                                  vector < QString > projects,
                                  bool               gnom_run 
                                  )
{
   clear_project();

   vector < QString > gnom_files;
   vector < double > gnom_files_rmax_start;
   vector < double > gnom_files_rmax_end;
   vector < double > gnom_files_rmax_inc;
   map < QString, unsigned int > gnom_file_map;

   vector < QString > gnom_crop_files;
   vector < double > gnom_crop_low;
   vector < double > gnom_crop_high;
   map < QString, unsigned int > gnom_crop_file_map;

   bool use_merge_gnom = false;

   if ( gnom_run )
   {
      QFile f("merge_gnom");
      // merge_gnom format
      // crop
      if ( f.exists() )
      {
         use_merge_gnom = true;
         if ( !f.open(QIODevice::ReadOnly) )
         {
            errormsg = "can not open merge_gnom";
            return false;
         }
         QRegExp rxempty("^\\s*$");
         QRegExp rxskip("^#");

         QRegExp rxrmax("^\\s*rmax");
         QRegExp rx1rmax("^\\s*rmax\\s+(\\S+)\\s*$");
         QRegExp rx4rmax("^\\s*rmax\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s*$");

         QRegExp rxcrop("^\\s*crop");
         QRegExp rx3crop("^\\s*crop\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s*$");

         QTextStream ts(&f);
         while ( !ts.atEnd() )
         {
            QString line = ts.readLine();
            if ( rxskip.indexIn(line) != -1 ||
                 rxempty.indexIn(line) != -1 )
            {
               continue;
            }

            if ( rxrmax.indexIn(line) != -1 )
            {
               if ( rx4rmax.indexIn(line) == -1 &&
                    rx1rmax.indexIn(line) == -1 )
               {
                  errormsg = "merge_gnom rmax lines must contain either two or five columns";
                  return false;
               }
               
               if ( rx4rmax.indexIn(line) != -1 )
               {
                  gnom_file_map[rx4rmax.cap(1)] = gnom_files.size();
                  gnom_files.push_back(rx4rmax.cap(1));
                  gnom_files_rmax_start.push_back(rx4rmax.cap(2).toDouble());
                  gnom_files_rmax_end.push_back(rx4rmax.cap(3).toDouble());
                  gnom_files_rmax_inc.push_back(rx4rmax.cap(4).toDouble());
                  continue;
               }
               
               gnom_file_map[rx1rmax.cap(1)] = gnom_files.size();
               gnom_files.push_back(rx1rmax.cap(1));
               gnom_files_rmax_start.push_back(0e0);
               gnom_files_rmax_end.push_back(0e0);
               gnom_files_rmax_inc.push_back(0e0);
               continue;
            }

            if ( rxcrop.indexIn(line) != -1 )
            {
               if ( rx3crop.indexIn(line) == -1 )
               {
                  errormsg = "merge_gnom crop lines must contain four columns";
                  return false;
               }
               
               gnom_crop_file_map[rx3crop.cap(1)] = gnom_files.size();
               gnom_crop_files.push_back(rx3crop.cap(1));
               gnom_crop_low.push_back(rx3crop.cap(2).toDouble());
               gnom_crop_high.push_back(rx3crop.cap(3).toDouble());
               continue;
            }
            
            errormsg = "unrecognized merge_gnom line: " + line;
            return false;
         }
      }
   }

   QString result = 
      QString(
              "= Multi Sample Guinier Summary =\n"
              " * Reference MW multiplier %1\n" 
              )
      .arg(reference_mw_multiplier)
      ;
   errormsg = "";

   QRegExp rx("^(\\S+)\\s+(\\S.*)$");
   QRegExp rxtrailingspaces("\\s*$");
   QRegExp rxcapturefields(
                           "^\\|\\|"
                           "(.*)\\|\\|"
                           "(.*)\\|\\|"
                           "(.*)\\|\\|"
                           "(.*)\\|\\|"
                           "(.*)\\|\\|"
                           "(.*)\\|\\|"
                           "(.*)\\|\\|"
                           "(.*)\\|\\|"
                           "(.*)\\|\\|"
                           "(.*)\\|\\|"
                           "(.*)\\|\\|"
                           "(.*)\\|\\|"
                           "(.*)\\|\\|"
                           "(.*)\\|\\|"
                           "(.*)\\|\\|"
                           "(.*)\\|\\|"
                           "(.*)\\|\\|"
                           "(.*)\\|\\|"
                           "$"
                           );

   QRegExp rxheadertowiki("= (.+) =");

   for ( unsigned int i = 0; i < projects.size(); i++ )
   {
      result += "\n----\n";
      QFile f(projects[i] + QDir::separator() + "project");
      vector < QString > files;
      vector < double > use_rmax_start;
      vector < double > use_rmax_end;
      vector < double > use_rmax_inc;
      vector < double > use_crop_low;
      vector < double > use_crop_high;

      if ( !f.open(QIODevice::ReadOnly) )
      {
         errormsg = "error: can not open file " + f.fileName();
         return false;
      }
      QTextStream ts(&f);
      QString prefix;
      QString name;

      while ( !ts.atEnd() )
      {
         QString line = ts.readLine().replace(rxtrailingspaces,"");

         if ( rx.indexIn(line) == -1 )
         {
            continue;
         }

         if ( rx.cap(1).toLower() == "wikiprefix" ) 
         {
            prefix = rx.cap(2);
         }

         if ( rx.cap(1).toLower() == "name" ) 
         {
            name = rx.cap(2);
         }
      }

      f.close();

      if ( prefix.isEmpty() )
      {
         errormsg = "error: no 'wikiPrefix' line found in " + f.fileName();
         return false;
      }
      if ( name.isEmpty() )
      {
         errormsg = "error: no 'name' line found in " + f.fileName();
         return false;
      }

      QFile f2a(projects[i] + QDir::separator() + prefix + name);
      if ( !f2a.open(QIODevice::ReadOnly) )
      {
         errormsg = "error: can not open file " + f2a.fileName();
         return false;
      }
      QTextStream ts2a(&f2a);
      QStringList count_guinier;

      while ( !ts2a.atEnd() )
      {
         count_guinier << ts2a.readLine();
      }
      f2a.close();

      int collect_limit = 1;
      if ( count_guinier.filter("Guinier Summary").size() == 1 )
      {
         collect_limit = 0;
      }

      QFile f2(projects[i] + QDir::separator() + prefix + name);
      if ( !f2.open(QIODevice::ReadOnly) )
      {
         errormsg = "error: can not open file " + f2.fileName();
         return false;
      }

      QTextStream ts2(&f2);

      QString saxs_guinier;
      // bool start_collecting_saxs_guinier = false;
      int start_collecting = -1;
      bool link_done = false;

      while ( !ts2.atEnd() )
      {
         QString line = ts2.readLine();
         line.replace(rxtrailingspaces,"");
         if ( line.contains("Guinier Summary") )
         {
            start_collecting++;
            // start_collecting_saxs_guinier = true;
         }
         if ( line.contains("== Project source file ==") )
         {
            start_collecting = 0;
         }

         if ( start_collecting < 0 )
         {
            if ( !link_done &&
                 rxheadertowiki.indexIn(line) != -1 )
            {
               link_done = true;
               result += QString("= [wiki:%1 %2] =\n").arg(QFileInfo(f2a.fileName()).fileName()).arg(rxheadertowiki.cap(1));
            } else {
               result += line + "\n";
            }
         }  
          
         if ( start_collecting > collect_limit )
         {
            if ( line.contains("Guinier Summary") )
            {
               result += line + "\n";
            } else {
               if ( line.contains(QRegExp("^.. name ")) )
               {
                  result += "|| sample " + line + " computed mw (Da) ||\n";
               } else {
                  if ( rxcapturefields.indexIn(line) == -1 )
                  {
                     errormsg = QString("can not find correct number of fields in line %1").arg(line);
                     return false;
                  }
                  double estmw = rxcapturefields.cap(10).trimmed().toDouble() * reference_mw_multiplier;
                  if ( gnom_run )
                  {
                     bool gnom_this_file = false;
                     if ( use_merge_gnom )
                     {
                        for ( unsigned int g = 0; g < gnom_files.size(); g++ )
                        {
                           if ( rxcapturefields.cap(6).contains(QRegExp(gnom_files[g])) )
                           {
                              gnom_this_file = true;
                              files.push_back(rxcapturefields.cap(6).trimmed());
                              use_rmax_start.push_back(gnom_files_rmax_start[g] ? gnom_files_rmax_start[g] : p_rmax_start);
                              use_rmax_end.push_back(gnom_files_rmax_end[g] ? gnom_files_rmax_end[g] : p_rmax_end);
                              use_rmax_inc.push_back(gnom_files_rmax_inc[g] ? gnom_files_rmax_inc[g] : p_rmax_inc);
                              bool found_crop = false;
                              for ( unsigned int c = 0; c < gnom_crop_files.size(); c++ )
                              {
                                 if ( rxcapturefields.cap(6).contains(QRegExp(gnom_crop_files[c])) )
                                 {
                                    use_crop_low.push_back(gnom_crop_low[c]);
                                    use_crop_high.push_back(gnom_crop_high[c]);
                                    found_crop = true;
                                    break;
                                 }
                              }

                              if ( !found_crop )
                              {
                                 use_crop_low.push_back(p_crop_low);
                                 use_crop_high.push_back(p_crop_high);
                              }
                              
                              break;
                           }
                        }
                     } else {
                        gnom_this_file = true;
                        files.push_back(rxcapturefields.cap(6).trimmed());
                        use_rmax_start.push_back(p_rmax_start);
                        use_rmax_end.push_back(p_rmax_end);
                        use_rmax_inc.push_back(p_rmax_inc);
                        use_crop_low.push_back(p_crop_low);
                        use_crop_high.push_back(p_crop_high);
                        break;
                     }

                     if ( gnom_this_file )
                     {
                        // cout << "cap.6 is " + rxcapturefields.cap(6) + "\n";
                        line.replace(QString("|| %1 ||").arg(rxcapturefields.cap(6).trimmed()),
                                     QString("|| [wiki:%1%2_gnom_%3_c%4-%5 %6] ||")
                                     .arg(prefix)
                                     .arg(projects[i])
                                     .arg(QString("%1").arg(rxcapturefields.cap(6).trimmed()).replace(QRegExp("\\.(dat|DAT)$"),""))
                                     .arg(use_crop_low.back())
                                     .arg(use_crop_high.back())
                                     .arg(rxcapturefields.cap(6).trimmed()));
                     }
                  }
                  
                  result += QString("|| %1 %2 %3 ||\n")
                     .arg(name)
                     .arg(line)
                     .arg(estmw);
               }
            }
         }
      }
      f2.close();
      if ( gnom_run )
      {
         // for ( unsigned int g = 0; g < files.size(); g++ )
         // {
         // cout << QString("gnom params files rmax etc %1 %2 %3 %4 %5\n")
         // .arg(files[g])
         // .arg(use_rmax_start.size())
         // .arg(use_rmax_start[g])
         // .arg(use_rmax_end[g])
         // .arg(use_rmax_inc[g]);
         // }
            
         if (!run_gnom(
                       projects[i],
                       prefix,
                       files,
                       use_rmax_start,
                       use_rmax_end,
                       use_rmax_inc,
                       use_crop_low,
                       use_crop_high
                       )
             )
         {
            return false;
         }
      }
   }

   QFile f(outfile);
   if ( !f.open(QIODevice::WriteOnly) )
   {
      errormsg = "error: can not create file " + f.fileName();
      return false;
   }

   QTextStream ts(&f);
   ts << result;
   f.close();
   return true;
}

bool US_Saxs_Util::project_1d(
                              QString wikitag,
                              unsigned int pngsplits
                              )
{
   errormsg = "";

   QString dir = QDir::currentPath() + QDir::separator() + "1d";

   QString pngdir = dir + QDir::separator() + "pngs";
   QString avgdir = dir + QDir::separator() + "averages";

   QDir p1d(dir);

   if ( !p1d.exists() )
   {
      errormsg = "error: directory '" + dir + "' does not exist";
      return false;
   }

   if ( !p1d.setCurrent(dir) )
   {
      errormsg = "error: could not change to directory '" + dir + "'";
      return false;
   }

   QDir pngdird(pngdir);

   if ( !pngdird.exists() )
   {
      cout << "making directory " + pngdir + "\n";
      if ( !pngdird.mkdir(pngdir) )
      {
         errormsg = "error: could not create directory: " + pngdir;
         return false;
      }
   }

   QDir avgdird(avgdir);

   if ( !avgdird.exists() )
   {
      cout << "making directory " + avgdir + "\n";
      if ( !avgdird.mkdir(avgdir) )
      {
         errormsg = "error: could not create directory: " + avgdir;
         return false;
      }
   }
   
   // get a list of data files in directory

   QStringList files = p1d.entryList( QStringList() << "*.dat" << "*.DAT", QDir::Files, QDir::Name);

   if ( !files.size() )
   {
      errormsg = "there are no .dat files in directory '" + dir + "'";
      return false;
   }

   // group files

   QRegExp rxgetbasename("^(.*)_(\\d+)\\.(dat|DAT)");
   
   vector < vector < QString > > filegroups;
   vector < QString > filegroup;
   vector < QString > filegroups_base_name;

   QString last_base_name;

   cout << "reading files:\n";
   for ( unsigned int i = 0; i < (unsigned int)files.size(); i++ )
   {
      if ( rxgetbasename.indexIn(files[i]) == -1 )
      {
         errormsg = "error parsing file name: " + files[i];
         return false;
      }
      
      if ( !read(files[i],files[i]) )
      {
         return false;
      }

      QString base_name = rxgetbasename.cap(1);

      if ( last_base_name.isEmpty() ) 
      {
         cout << base_name << endl;
         last_base_name = base_name;
      } else {
         if ( base_name != last_base_name )
         {
            cout << base_name << endl;
            filegroups.push_back(filegroup);
            filegroups_base_name.push_back(last_base_name);
            last_base_name = base_name;
            
            filegroup.clear( );
         } 
      }
      filegroup.push_back(files[i]);
   }

   if ( filegroup.size() )
   {
      filegroups.push_back(filegroup);
      filegroups_base_name.push_back(last_base_name);
   }
   cout << endl;

   QString result;

   for ( unsigned int i = 0; i < filegroups.size(); i++ )
   {
      cout << QString("processing basename %1\n").arg(filegroups_base_name[i]);

      QString outfile = filegroups_base_name[i] + "_avg";

      if (
          !avg(outfile, filegroups[i]) ||
          !write(avgdir + QDir::separator() + outfile + ".dat", outfile)
          )
      {
         return false;
      }

      if ( i )
      {
         result += "----\n";
      }

      result += 
         QString(
                 "== Base name %1 ==\n"
                 "|| file name ||\n"
                 )
         .arg(filegroups_base_name[i]);

      QString pngfile = QString("pngs%1%2_%3.png").arg(QDir::separator()).arg(outfile).arg(wikitag);

      QString cmd = "pnggnuplot.pl " + pngfile + " ";

      vector < QString > pngfiles(pngsplits);
      vector < QString > cmds(pngsplits);

      if ( pngsplits > 1 )
      {
         // determine split points
         if ( !filegroups[i].size() )
         {
            errormsg = "internal error: filegroup has zero size!";
            return false;
         }
         unsigned int spsize = wave[filegroups[i][0]].q.size();
         unsigned int spdelta = spsize / pngsplits;
         vector < double > spstart(pngsplits);
         vector < double > spend(pngsplits);
         unsigned int sp = 0;
         
         for ( unsigned int k = 0; k < pngsplits; k++ )
         {
            spstart[k] = wave[filegroups[i][0]].q[sp];
            sp += spdelta;
            if ( sp >= spsize )
            {
               sp = spsize - 1;
            }
            spend[k] = wave[filegroups[i][0]].q[sp];
         }
         
         for ( unsigned int k = 0; k < pngsplits; k++ )
         {
            pngfiles[k] = QString("pngs%1%2_z%3_%4.png").arg(QDir::separator()).arg(outfile).arg(k).arg(wikitag);
            cmds[k] = QString("pnggnuplot.pl -c %1 %2 %3 ")
               .arg(spstart[k])
               .arg(spend[k])
               .arg(pngfiles[k]);
         }
      }

      for ( unsigned int j = 0; j < filegroups[i].size(); j++ )
      {
         cmd += " " + filegroups[i][j];
         if ( pngsplits > 1 )
         {
            for ( unsigned int k = 0; k < pngsplits; k++ )
            {
               cmds[k] += " " + filegroups[i][j];
            }
         }
         result += 
            QString(
                    "|| %1 ||\n"
                    )
            .arg(filegroups[i][j]);
      }

      result += QString("[[Image(htdocs:%1)]]\n").arg(pngfile);

      cmd += "\n";
      cout << cmd.toLatin1().data();
      system(cmd.toLatin1().data());
      if ( pngsplits > 1 )
      {
         for ( unsigned int k = 0; k < pngsplits; k++ )
         {
            result += QString("[[Image(htdocs:%1)]]\n").arg(pngfiles[k]);
            cmds[k] += "\n";
            cout << cmds[k].toLatin1().data();
            system(cmds[k].toLatin1().data());
         }
      }
   }

   QFile f(QString("wiki_1d_average_%1").arg(wikitag));
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      errormsg = "error: can not create file " + f.fileName();
      return false;
   }
   
   QTextStream ts(&f);
   ts << result;
   f.close();
   return true;
}   

bool US_Saxs_Util::run_gnom( 
                            QString             project, 
                            QString             prefix, 
                            vector < QString >  files,
                            vector < double >   use_rmax_start,
                            vector < double >   use_rmax_end,
                            vector < double >   use_rmax_inc,
                            vector < double >   use_crop_low,
                            vector < double >   use_crop_high
                            )
{
   errormsg = "";

   // cout << QString("run_gnom files.size() %1\n").arg(files.size());

   // read project for rmax info
   if ( !read_project(project) )
   {
      errormsg = project + " " + errormsg;
      cout << "read project false\n";
      return false;
   }

   // make gnom subdir
   QString dir = QDir::currentPath() + QDir::separator() + project + QDir::separator() + "gnom";
   QString pdir = dir + QDir::separator() + "pngs";

   QDir qdir(dir);
   QDir qpdir(pdir);

   cout << "dir:    " << dir << endl;
   cout << "pngdir: " << pdir << endl;


   if ( !qdir.exists() )
   {
      cout << "making directory " + dir + "\n";
      if ( !qdir.mkdir(dir) )
      {
         errormsg = "error: could not create directory: " + dir;
         return false;
      }
   }

   if ( !qpdir.exists() )
   {
      cout << "making pdirectory " + pdir + "\n";
      if ( !qpdir.mkdir(pdir) )
      {
         errormsg = "error: could not create directory: " + pdir;
         return false;
      }
   }

   for ( unsigned int i = 0; i < files.size(); i++ )
   {
      QFile f(project + QDir::separator() + "cwave" + QDir::separator() + files[i]);
      
      if ( !f.exists() )
      {
         errormsg = "error: file " + f.fileName() + " does not exist";
         return false;
      }

      QString cmd = 
         QString(
#if QT_VERSION < 0x040000
                 "mkgnom.pl %1 %1%1_gnom_ %1 %1 %1 %1 %1 %1\n"
                 "mv %1%1%1*.png %1%1\n"
#else
                 "mkgnom.pl %1 %2%3_gnom_ %4 %5 %6 %7 %8 %9\n"
                 "mv %10%11%12*.png %13%14\n"
#endif
                 )
         .arg(dir)
         .arg(prefix)
         .arg(project)
         .arg(f.fileName())
         .arg(use_rmax_start[i])
         .arg(use_rmax_end[i])
         .arg(use_rmax_inc[i])
         .arg(use_crop_low[i])
         .arg(use_crop_high[i])

         .arg(dir)
         .arg(QDir::separator())
         .arg(QString("%1").arg(files[i]).replace(QRegExp("\\.(dat|DAT)$"),""))
         .arg(pdir)
         .arg(QDir::separator())
         ;
      cout << cmd;
      system(cmd.toLatin1().data());
   }
   return true;
}

double US_Saxs_Util::calc_rmsd( vector < double > v1, vector < double > v2 )
{
   double rmsd = 0e0;

   if ( v1.size() != v2.size() )
   {
      return 9e99;
   }

   for ( unsigned int i = 0; i < v1.size(); i++ )
   {
      rmsd += ( v1[i] - v2[i] ) * ( v1[i] - v2[i] );
   }
   return sqrt(rmsd / v1.size());
}

double US_Saxs_Util::calc_nrmsd( vector < double > v1, vector < double > v2 )
{
   if ( v1.size() != v2.size() || v1.size() < 2 )
   {
      return 9e99;
   }
   double rmsd = calc_rmsd( v1, v2 );
   double v2max = v2[0];
   double v2min = v2[0];

   for ( unsigned int i = 1; i < v2.size(); i++ )
   {
      if ( v2max < v2[i] )
      {
         v2max = v2[i];
      }
      if ( v2min > v2[i] )
      {
         v2min = v2[i];
      }
   }

   if ( v2min == v2max )
   {
      return 9e99;
   }

   return 100.0 * rmsd / fabs(v2max - v2min);
}

bool US_Saxs_Util::calc_chisq1( vector < double > bins,
                                vector < double > ebins,
                                unsigned int      &df,
                                double            &chisq,
                                double            &prob )
{
   chisq = 0.0;

   if ( bins.size() != ebins.size() )
   {
      return false;
   }
   df = bins.size() - 1;

   for ( unsigned int i = 0; i < bins.size(); i++ )
   {
      if ( ebins[i] == 0.0 )
      {
         return false;
      }
      chisq += ( bins[i] - ebins[i] ) * ( bins[i] - ebins[i] ) / ebins[i];
   }
   prob = -1e0;
   return true; // calc_chisq_prob( 0.5 * df, 0.5 * chisq, prob );
}

bool US_Saxs_Util::calc_chisq2( vector < double > bins1,
                                vector < double > bins2,
                                unsigned int      &df,
                                double            &chisq,
                                double            &prob )
{
   chisq = 0.0;

   if ( bins1.size() != bins2.size() )
   {
      return false;
   }
   df = bins1.size() - 1;

   for ( unsigned int i = 0; i < bins1.size(); i++ )
   {
      if ( bins1[i] + bins2[i] != 0.0 )
      {
         chisq += 
            ( bins1[i] - bins2[i] ) * ( bins1[i] - bins2[i] ) / 
            ( bins1[i] + bins2[i] );
      } else {
         df--;
      }
   }
   prob = -1e0;
   return true; // calc_chisq_prob( 0.5 * df, 0.5 * chisq, prob );
}

double US_Saxs_Util::calc_gammaln( double val )
{
   double x = val;
   double y = val;
   double ser = 1.000000000190015;;

   static double cof[6] = 
      { 76.18009172947146, 
        -86.50532032941677,
        24.01409824083091, 
        -1.231739572450155,
	0.1208650973866179e-2,
        -0.5395239384953e-5
      };

   double tmp;

   tmp = x + 5.5;
   tmp -= ( x + 0.5 ) * log( tmp );
   for ( unsigned int j = 0; j <= 5 ; j++ )
   {
      ser += cof[j] / ++y;
   }
   return -tmp + log( 2.5066282746310005 * ser / x );
}


bool US_Saxs_Util::calc_chisq_prob( double a, double x, double &prob )
{
   if ( x < 0.0 || a <= 0.0 )
   {
      return false;
   }

   double gln = calc_gammaln(a);

   if ( x < ( a + 1.0 ) ) 
   {
      double sum;
      double del;
      double ap;
      if (x <= 0.0) {
         if ( x < 0.0 ) 
         {
            cerr << "x < 0 US_Saxs_Util::calc_chi2q_prob\n";
            return false;
         }
         prob = 1.0;
         return true;
      } else {
         ap = a;
         del = sum = 1.0 / a;
         for ( unsigned int n = 1; n <= 1000; n++ )
         {
            ap++;
            del *= x / ap;
            sum += del;
            if ( fabs(del) < fabs(sum) * 3e-7 ) 
            {
               prob =  1.0 - sum * exp( -x + a * log(x) - gln );
               return true;
            }
         }
         cerr << "too many iterations in US_Saxs_Util::calc_chi2q_prob\n";
         return false;
      }
      // return false; // never gets here
   } else {
      double an;
      double b;
      double c;
      double d;
      double del;
      double h;

      b = x + 1.0 -a;
      c = 1e30;
      h = d = 1.0 / b;

      int i;
      for ( i = 1; i <= 1000; i++ ) 
      {
         an = -i * ( i - a );
         b += 2.0;
         d = an * d + b;
         if ( fabs(d) < 1e-30 )
         {
            d = 1e-30;
         }
         c = b + an / c;
         if ( fabs(c) < 1e-30 )
         {
            c = 1e-30;
         }
         d = 1.0 / d;
         del = d * c;
         h *= del;
         if ( fabs( del - 1.0 ) < 3e-7)
         {
            break;
         }
      }
      if ( i > 1000 )
      {
         cerr << "too many iterations in US_Saxs_Util::calc_chi2q_prob\n";
         return false;
      }
        
      prob = exp( -x + a * log( x ) - gln ) * h;
      return true;
   }
   // return false; // never gets here
}

bool US_Saxs_Util::scaling_fit( 
                              vector < double > x, 
                              vector < double > y, 
                              double &k,
                              double &rmsd
                              )
{
   if ( x.size() != y.size() )
   {
      cerr << 
         QString( "US_Saxs_Util::scaling_fit() incompatible vector sizes %1 %2\n" )
         .arg( x.size() )
         .arg( y.size() );
      k = 1e0;
      rmsd = 9e99;
      return false;
   }

   k = 0e0;

   double Sxx = 0e0;
   double Sxy = 0e0;

   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      Sxx += x[i] * x[i];
      Sxy += x[i] * y[i];
   }

   if ( Sxx != 0e0 )
   {
      k = Sxy / Sxx;
   } else {
      k = 1e0;
   }

   rmsd = 0e0;
   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      rmsd += ( k * x[i] - y[i] ) * ( k * x[i] - y[i] );
   }
   rmsd = sqrt(rmsd);
   return true;
}

bool US_Saxs_Util::scaling_fit( 
                              vector < double > x, 
                              vector < double > y, 
                              vector < double > sd, 
                              double &k,
                              double &chi2
                              )
{
   if ( x.size() != y.size() ||
        x.size() != sd.size() )
   {
      cerr << 
         QString( "US_Saxs_Util::scaling_fit() incompatible vector sizes %1 %2 %3\n" )
         .arg( x.size() )
         .arg( y.size() )
         .arg( sd.size() );
      k = 1e0;
      chi2 = 9e99;
      return false;
   }

   k = 0e0;

   double Sxx = 0e0;
   double Sxy = 0e0;

   vector < double > oneoversd2( sd.size() );

   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      oneoversd2[ i ] =  1.0 / (sd[ i ] * sd[ i ]);
      Sxx += x[i] * x[i] * oneoversd2[ i ];
      Sxy += x[i] * y[i] * oneoversd2[ i ];
   }

   if ( Sxx != 0 )
   {
      k = Sxy / Sxx;
   } else {
      k = 1e0;
   }

   chi2 = 0e0;
   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      chi2 += ( k * x[i] - y[i] ) * ( k * x[i] - y[i] ) * oneoversd2[ i ];
   }
   return true;
}

bool US_Saxs_Util::nnls_fit( 
                            vector < vector < double > > A, 
                            vector < double >            y, 
                            vector < double >            &x,
                            double                       &nnls_rmsd
                            )
{
   if ( !A.size() )
   {
      errormsg = "US_Saxs_Util::nnls_fit matrix A is empty";
      return false;
   }

   x.resize( A.size() );
   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      x[ i ] = 0e0;
   }

   if ( A[0].size() != y.size() )
   {
      errormsg =  "US_Saxs_Util::nnls_fit incompatible vector sizes";
      nnls_rmsd = 9e99;
      return false;
   }

   // check A

   for ( unsigned int i = 1; i < A.size(); i++ )
   {
      if ( A[ i ].size() != A[ 0 ].size() )
      {
         errormsg =  "US_Saxs_Util::nnls_fit inconsistant vector sizes in A";
         nnls_rmsd = 9e99;
         return false;
      }
   }

   int vectors = (int) A.size();
   int len     = (int) y.size();
   
   vector < double > nnls_wp     ( A.size() );
   vector < double > nnls_zzp    ( y.size() );
   vector < int >    nnls_indexp ( A.size() );

   vector < double > use_A;

   for ( unsigned int i = 0; i < A.size(); i++ )
   {
      for ( unsigned int j = 0; j < A[ i ].size(); j++ )
      {
         use_A.push_back( A[ i ][ j ] );
      }
   }

   int result =
      nnls( (double *)&use_A[0],
            len,
            len,
            vectors,
            (double *)&y[0],
            (double *)&x[0],
            &nnls_rmsd,
            (double *)&nnls_wp[0],
            (double *)&nnls_zzp[0],
            (int  *)&nnls_indexp[0]
            );
            
   if ( result != 0 )
   {
      cout << "ending nnls 1\n";
      errormsg = QString("US_Saxs_Util::nnls_fit NNLS error %1").arg( result );
      return false;
   }
   return true;
   // cerr << QString("Residual Euclidian norm of NNLS fit %1\n").arg(nnls_rmsd);
}

void US_Saxs_Util::nnls_fit( 
                              vector < double > x, 
                              vector < double > y, 
                              double &k,
                              double &nnls_rmsd
                              )
{
   if ( x.size() != y.size() )
   {
      cerr << "US_Saxs_Util::nnls_fit() incompatible vector sizes\n";
      k = 1e0;
      nnls_rmsd = 9e99;
   }

   int const_one = 1;
   int len = (int) x.size();
   
   double            nnls_wp;
   vector < double > nnls_zzp( x.size() );
   int               nnls_indexp;

   int result =
      nnls( (double *)&x[0],
            len,
            len,
            const_one,
            (double *)&y[0],
            (double *)&k,
            &nnls_rmsd,
            (double *)&nnls_wp,
            (double *)&nnls_zzp[0],
            (int  *)&nnls_indexp
            );
            
   if ( result != 0 )
   {
      cerr << "NNLS error!\n";
   }
   // cerr << QString("Residual Euclidian norm of NNLS fit %1\n").arg(nnls_rmsd);
}

bool US_Saxs_Util::iqq_sphere(
                              QString tag,
                              double  radius,
                              double  delta_rho,
                              double  min_q,
                              double  max_q,
                              double  delta_q 
                              )
{
   errormsg = "";

   if ( radius <= 0e0 )
   {
      errormsg = "US_Saxs_Util::iqq_sphere radius must be greater than zero";
      return false;
   }

   wave[ tag ].clear( );
   wave[ tag ].filename = tag;
   wave[ tag ].header   = 
      QString( 
              "# iqq_sphere %1 radius %2 delta_rho %3 min_q %4 max_q %5 delta_q %6\n"
              "#\tq\tI(q)"
              )
      .arg( tag )
      .arg( radius )
      .arg( delta_rho )
      .arg( min_q )
      .arg( max_q )
      .arg( delta_q )
      ;

   double delta_rho2   = delta_rho * delta_rho;
   double v            = ( 4.0 / 3.0 ) * M_PI * radius * radius * radius;
   double v2           = v * v;
   double delta_rho2v2 = delta_rho2 * v2;
   
   if ( min_q == 0e0 )
   {
      wave[ tag ].q.push_back( min_q );
      wave[ tag ].r.push_back( delta_rho2v2 );
      wave[ tag ].s.push_back( 0e0 );
      min_q += delta_q;
   }
   
   max_q += delta_q / 2e0;

   for ( double q = min_q; q <= max_q; q += delta_q )
   {
      double qradius = q * radius;
      double arg     =
         3.0 * ( sin( qradius ) - qradius * cos( qradius ) ) / ( qradius * qradius * qradius );
      double arg2    = arg * arg;

      wave[ tag ].q.push_back( q );
      wave[ tag ].r.push_back( delta_rho2v2 * arg2 );
      wave[ tag ].s.push_back( 0e0 );
   }
   return true;
}

bool US_Saxs_Util::iqq_sphere_grid(
                                   QString outtag,
                                   QString intag,
                                   double  radius,
                                   double  delta_rho
                                   )
{
   errormsg = "";

   if ( radius <= 0 )
   {
      errormsg = "US_Saxs_Util::iqq_sphere_grid radius must be greater than zero";
      return false;
   }

   wave[ outtag ].clear( );
   wave[ outtag ].filename = outtag;
   wave[ outtag ].header   = 
      QString( 
              "# iqq_sphere %1 radius %2 delta_rho %3 grid matched to %4" 
              "#\tq\tI(q)"
              )
      .arg( outtag )
      .arg( radius )
      .arg( delta_rho )
      .arg( intag )
      ;

   double delta_rho2   = delta_rho * delta_rho;
   double v            = ( 4.0 / 3.0 ) * M_PI * radius * radius * radius;
   double v2           = v * v;
   double delta_rho2v2 = delta_rho2 * v2;
   
   for ( unsigned int i = 0; i < wave[ intag ].q.size(); i++ )
   {
      double q =  wave[ intag ].q[ i ];
      wave[ outtag ].q.push_back( wave[ intag ].q[ i ] );
      if ( q == 0e0 )
      {
         wave[ outtag ].r.push_back( delta_rho2v2 );
      } else {
         double qradius = q * radius;
         double arg     =
            3.0 * ( sin( qradius ) - qradius * cos( qradius ) ) / ( qradius * qradius * qradius );
         double arg2    = arg * arg;
         
         wave[ outtag ].r.push_back( delta_rho2v2 * arg2 );
      }
      wave[ outtag ].s.push_back( 0e0 );
   }
   return true;
}

bool US_Saxs_Util::iqq_sphere_fit( 
                                  QString outfile,
                                  QString infile,
                                  double  min_radius,
                                  double  max_radius,
                                  double  delta_radius,
                                  double  min_delta_rho,
                                  double  max_delta_rho,
                                  double  delta_delta_rho,
                                  double  min_q,
                                  double  max_q,
                                  bool    do_normalize
                                  )
{
   errormsg = "";

   // load infile
   if ( !read( infile, infile ) )
   {
      return false;
   }

   QString cropped = infile + "_cropped";
   if ( !crop( cropped, infile, min_q, max_q ) )
   {
      return false;
   }      

   if ( !wave[ cropped ].q.size() )
   {
      errormsg = "US_Saxs_Util::iqq_sphere_fit cropped data leaves nothing to fit\n";
      return false;
   }
   cout << "US_Saxs_Util::iqq_sphere_fit q points after cropping: " << wave[ cropped ].q.size() << endl;

   // build a matrix of the models in A

   vector < vector < double > > A;
   vector < QString >           A_tag;
   vector < double >            A_radius;
   vector < double >            A_delta_rho;

   for ( double radius = min_radius; 
         radius <= max_radius; 
         radius += delta_radius )
   {
      for ( double delta_rho = min_delta_rho;
            delta_rho <= max_delta_rho; 
            delta_rho += delta_delta_rho )
      {
         QString tag = QString("radius %1 delta_rho %2").arg( radius ).arg( delta_rho );
         if ( !iqq_sphere_grid( tag, cropped, radius, delta_rho ) )
         {
            return false;
         }
         if ( do_normalize )
         {
            if ( !normalize( tag, tag ) )
            {
               return false;
            }
         }
         A.          push_back( wave[ tag ].r );
         A_tag.      push_back( tag );
         A_radius.   push_back( radius );
         A_delta_rho.push_back( delta_rho );
      }
   }

   if ( !A.size() )
   {
      errormsg = "US_Saxs_Util::iqq_sphere_fit no models created!\n";
      return false;
   }
      
   // find single best fit:

   unsigned int best_pos  = 0;
   double       k;
   double       best_rmsd;

   nnls_fit(
            A[ 0 ],
            wave[ cropped ].r,
            k,
            best_rmsd
            );
             
   for ( unsigned int i = 1; i < A.size(); i++ )
   {
      double rmsd;
      nnls_fit(
            A[ i ],
            wave[ cropped ].r,
            k,
            rmsd
            );
      if ( rmsd < best_rmsd )
      {
         best_rmsd = rmsd;
         best_pos = i;
      }
   }
   
   cout << QString("best single model %1 rmsd %2\n").arg( A_tag[ best_pos ] ).arg( best_rmsd );

   vector < double > x;
   double            nnls_rmsd;

   bool result = nnls_fit( A, wave[ cropped ].r, x, nnls_rmsd );

   if ( !result )
   {
      return false;
   }

   cout << QString( "nnls rmsd %1\n" ).arg( nnls_rmsd );


   double totconc = 0e0;

   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      if ( x[ i ] > 0e0 )
      {
         totconc += x[ i ];
      }
   }

   if ( totconc == 0e0 )
   {
      totconc = 1e0;
   }

   double oneovertotconc = 1e0 / totconc;

   double avgradius      = 0e0;
   double avgdelta_rho   = 0e0;

   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      if ( x[ i ] > 0e0 )
      {
         avgradius    += x[ i ] * oneovertotconc * A_radius[ i ];
         avgdelta_rho += x[ i ] * oneovertotconc * A_delta_rho[ i ];
         cout << QString("nnls model %1 conc %2\n").arg( A_tag[ i ] ).arg( x[ i ] * oneovertotconc );
      }
   }

   cout << QString("nnls avg radius %1 avg delta_rho %2\n").arg( avgradius ).arg( avgdelta_rho );

   // build output
   
   vector < double >             by_radius;
   vector < double >             by_delta_rho;
   vector < double >             val_radius;
   vector < double >             val_delta_rho;
   map < double, unsigned int >  index_radius;
   map < double, unsigned int >  index_delta_rho;

   for ( double radius = min_radius; 
         radius <= max_radius; 
         radius += delta_radius )
   {
      index_radius[ radius ] = by_radius.size();
      by_radius .push_back( 0e0 );
      val_radius.push_back( radius );
   }

   for ( double delta_rho = min_delta_rho;
         delta_rho <= max_delta_rho; 
         delta_rho += delta_delta_rho )
   {
      index_delta_rho[ delta_rho ] = by_delta_rho.size();
      by_delta_rho .push_back( 0e0 );
      val_delta_rho.push_back( delta_rho );
   }

   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      by_radius   [ index_radius   [ A_radius   [ i ] ] ] += x[ i ] * oneovertotconc;
      by_delta_rho[ index_delta_rho[ A_delta_rho[ i ] ] ] += x[ i ] * oneovertotconc;
   }

   QString out_radius;
   QString out_delta_rho;
   QString out;

   for ( unsigned int i = 0; i < by_radius.size(); i++ )
   {
      out_radius += QString( "%1 %2\n" ).arg( val_radius[ i ] ).arg( by_radius[ i ] );
   }

   for ( unsigned int i = 0; i < by_delta_rho.size(); i++ )
   {
      out_delta_rho += QString( "%1 %2\n" ).arg( val_delta_rho[ i ] ).arg( by_delta_rho[ i ] );
   }
   
   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      out += QString( "%1 %2 %3\n" )
         .arg( val_radius   [ index_radius   [ A_radius   [ i ] ] ] )
         .arg( val_delta_rho[ index_delta_rho[ A_delta_rho[ i ] ] ] )
         .arg( x[ i ] * oneovertotconc );
   }


   {
      QFile f( outfile + "-radius.txt" );
      
      if ( !f.open( QIODevice::WriteOnly ) )
      {
         errormsg = "US_Saxs_Util::iqq_sphere_fit can not create file " + f.fileName();
         return false;
      }
      QTextStream ts(&f);
      ts << out_radius;
      f.close();      
   }

   {
      QFile f( outfile + "-delta_rho.txt" );
      
      if ( !f.open( QIODevice::WriteOnly ) )
      {
         errormsg = "US_Saxs_Util::iqq_sphere_fit can not create file " + f.fileName();
         return false;
      }
      QTextStream ts(&f);
      ts << out_delta_rho;
      f.close();
   }

   {
      QFile f( outfile + ".txt" );
      
      if ( !f.open( QIODevice::WriteOnly ) )
      {
         errormsg = "US_Saxs_Util::iqq_sphere_fit can not create file " + f.fileName();
         return false;
      }
      QTextStream ts(&f);
      ts << out;
      f.close();
   }
   
   return true;
}

bool US_Saxs_Util::iqq_sphere_fit( 
                                  QString                       tag,
                                  vector < double >             q,
                                  vector < double >             I,
                                  vector < double >             I_errors,
                                  double                        min_radius,
                                  double                        max_radius,
                                  double                        delta_radius,
                                  double                        min_delta_rho,
                                  double                        max_delta_rho,
                                  double                        delta_delta_rho,
                                  double                        min_q,
                                  double                        max_q,
                                  vector < double >             &by_radius,
                                  vector < double >             &by_delta_rho,
                                  vector < double >             &val_radius,
                                  vector < double >             &val_delta_rho,
                                  map < double, unsigned int >  &index_radius,
                                  map < double, unsigned int >  &index_delta_rho,
                                  double                        &best_fit_radius,
                                  double                        &best_fit_delta_rho,
                                  double                        &average_radius,
                                  double                        &average_delta_rho,
                                  double                        &start_q,
                                  double                        &end_q,
                                  double                        &delta_q,
                                  unsigned int                  &points_q,
                                  QString                       &best_tag,
                                  QString                       &nnls_tag,
                                  bool                          do_normalize
                                  )
{
   errormsg = "";
   noticemsg = "";

   wave[ tag ].clear( );
   wave[ tag ].q = q;
   wave[ tag ].r = I;
   wave[ tag ].s = I_errors;

   QString cropped = tag + "_cropped";
   if ( !crop( cropped, tag, min_q, max_q ) )
   {
      return false;
   }      

   if ( wave[ cropped ].q.size() < 3 )
   {
      errormsg = "US_Saxs_Util::iqq_sphere_fit cropped data leaves to little to fit\n";
      return false;
   }

   start_q  = wave[ cropped ].q[ 0 ];
   end_q    = wave[ cropped ].q[ wave[ cropped ].q.size() - 1 ];
   delta_q  = wave[ cropped ].q[ 1 ] - wave[ cropped ].q[ 0 ];
   points_q = wave[ cropped ].q.size();

   bool all_nonzero = true;
   if ( wave[ cropped ].s.size() == wave[ cropped ].q.size() )
   {
      for ( unsigned int i = 0; i < wave[ cropped ].s.size(); i++ )
      {
         if ( wave[ cropped ].s[ i ] == 0 )
         {
            all_nonzero = false;
         }
      }
   } else {
      all_nonzero = false;
   }

   noticemsg +=  QString("US_Saxs_Util::iqq_sphere_fit q points after cropping: %1\n").arg(wave[ cropped ].q.size());

   if ( all_nonzero )
   {
      noticemsg += "Using standard deviations of target in fit\n";
      for ( unsigned int i = 0; i < wave[ cropped ].r.size(); i++ )
      {
         wave[ cropped ].r[ i ] /= wave[ cropped ].s[ i ];
      }
   }

   // build a matrix of the models in A

   vector < vector < double > > A;
   vector < QString >           A_tag;
   vector < double >            A_radius;
   vector < double >            A_delta_rho;

   //   cout << QString("%1 %2 %3 %4 %5 %6\n")
   //      .arg(min_radius).arg(max_radius).arg(delta_radius)
   //      .arg(min_delta_rho).arg(max_delta_rho).arg(delta_delta_rho);

   for ( double radius = min_radius; 
         radius <= max_radius; 
         radius += delta_radius )
   {
      for ( double delta_rho = min_delta_rho;
            delta_rho <= max_delta_rho; 
            delta_rho += delta_delta_rho )
      {
         QString tag = QString("radius %1 delta_rho %2").arg( radius ).arg( delta_rho );
         // cout << tag << endl;
         if ( !iqq_sphere_grid( tag, cropped, radius, delta_rho ) )
         {
            return false;
         }
         if ( do_normalize )
         {
            if ( !normalize( tag, tag ) )
            {
               return false;
            }
         }
         if ( all_nonzero )
         {
            for ( unsigned int i = 0; i < wave[ cropped ].r.size(); i++ )
            {
               wave[ tag ].r[ i ] /=  wave[ cropped ].s[ i ];
            }
         }
         A.          push_back( wave[ tag ].r );
         A_tag.      push_back( tag );
         A_radius.   push_back( radius );
         A_delta_rho.push_back( delta_rho );
      }
   }

   if ( !A.size() )
   {
      errormsg = "US_Saxs_Util::iqq_sphere_fit no models created!\n";
      return false;
   }
      
   // find single best fit:

   unsigned int best_pos  = 0;
   double       k;
   double       best_rmsd;
   double       best_k    = 0e0;

   nnls_fit(
            A[ 0 ],
            wave[ cropped ].r,
            k,
            best_rmsd
            );
             
   for ( unsigned int i = 1; i < A.size(); i++ )
   {
      double rmsd;
      nnls_fit(
            A[ i ],
            wave[ cropped ].r,
            k,
            rmsd
            );
      if ( rmsd < best_rmsd )
      {
         best_rmsd = rmsd;
         best_pos  = i;
         best_k    = k;
      }
   }
   
   best_tag = A_tag[ best_pos ] + "_scaled";
   scale( best_tag, A_tag[ best_pos ], best_k );

   noticemsg += QString("best single model %1 rmsd %2\n").arg( A_tag[ best_pos ] ).arg( best_rmsd );
   best_fit_radius = A_radius[ best_pos ];
   best_fit_delta_rho = A_delta_rho[ best_pos ];

   vector < double > x;
   double            nnls_rmsd;

   bool result = nnls_fit( A, wave[ cropped ].r, x, nnls_rmsd );

   if ( !result )
   {
      return false;
   }

   nnls_tag = cropped + "_nnls_model";
   scalesum( nnls_tag, A_tag, x );

   noticemsg += QString( "nnls rmsd %1\n" ).arg( nnls_rmsd );

   double totconc = 0e0;

   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      if ( x[ i ] > 0e0 )
      {
         totconc += x[ i ];
      }
   }

   if ( totconc == 0e0 )
   {
      totconc = 1e0;
   }

   double oneovertotconc = 1e0 / totconc;

   average_radius      = 0e0;
   average_delta_rho   = 0e0;

   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      if ( x[ i ] > 0e0 )
      {
         average_radius    += x[ i ] * oneovertotconc * A_radius[ i ];
         average_delta_rho += x[ i ] * oneovertotconc * A_delta_rho[ i ];
         noticemsg         += QString("nnls model %1 conc %2\n").arg( A_tag[ i ] ).arg( x[ i ] * oneovertotconc );
      }
   }

   noticemsg += QString("nnls avg radius %1 avg delta_rho %2\n")
      .arg( average_radius )
      .arg( average_delta_rho );

   // build output
   
   by_radius      .clear( );
   by_delta_rho   .clear( );
   val_radius     .clear( );
   val_delta_rho  .clear( );
   index_radius   .clear( );
   index_delta_rho.clear( );

   for ( double radius = min_radius; 
         radius <= max_radius; 
         radius += delta_radius )
   {
      index_radius[ radius ] = by_radius.size();
      by_radius .push_back( 0e0 );
      val_radius.push_back( radius );
   }

   for ( double delta_rho = min_delta_rho;
         delta_rho <= max_delta_rho; 
         delta_rho += delta_delta_rho )
   {
      index_delta_rho[ delta_rho ] = by_delta_rho.size();
      by_delta_rho .push_back( 0e0 );
      val_delta_rho.push_back( delta_rho );
   }

   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      by_radius   [ index_radius   [ A_radius   [ i ] ] ] += x[ i ] * oneovertotconc;
      by_delta_rho[ index_delta_rho[ A_delta_rho[ i ] ] ] += x[ i ] * oneovertotconc;
   }

   return true;
}

bool US_Saxs_Util::merge( 
                         QString outtag,
                         QString tag1,
                         double  w1,
                         QString tag2,
                         double   w2 )
{
   errormsg = "";

   wave[outtag].clear( );
   wave[outtag].filename = QString("merge_%1_%2_%3_%4").arg(tag1).arg(w1).arg(tag2).arg(w2);
   wave[outtag].header = wave[tag1].header;

   // grid range
   if ( wave[tag1].q.size() < 3 )
   {
      errormsg = QString("US_Saxs_Util::merge too few points wave[%1].q.size() == %2").arg(tag1).arg(wave[tag1].q.size());
      return false;
   }
      
   double min_q = wave[tag1].q[1];
   double max_q = wave[tag1].q[wave[tag1].q.size() - 2];

   if ( min_q < wave[tag2].q[0] )
   {
      min_q = wave[tag2].q[0];
   }
   if ( max_q > wave[tag2].q[wave[tag2].q.size() - 1] )
   {
      max_q = wave[tag2].q[wave[tag2].q.size() - 1];
   }

   for( unsigned int i = 0; i < wave[tag1].q.size(); i++ )
   {
      if ( wave[tag1].q[i] >= min_q && 
           wave[tag1].q[i] <= max_q )
      {
         wave[outtag].q.push_back(wave[tag1].q[i]);
         wave[outtag].r.push_back(wave[tag1].r[i] * w1);
         wave[outtag].s.push_back(wave[tag1].s[i]);
      }
   }
         
   QString tag2_interp = "tag2_interp";
   if ( !interpolate( tag2_interp, outtag, tag2 ) )
   {
      return false;
   }

   for( unsigned int i = 0; i < wave[tag2_interp].q.size(); i++ )
   {
      if ( wave[outtag].q[i] != wave[tag2_interp].q[i] )
      {
         errormsg = QString("US_Saxs_Util::merge q values inconsistant point %1").arg(i);
         return false;
      }
      wave[outtag].r[i] += wave[tag2_interp].r[i] * w2;
      wave[outtag].s[i] += wave[tag2_interp].s[i];
   }
   return true;
}

bool US_Saxs_Util::set_excluded_volume( 
                                       PDB_atom                 &this_atom, 
                                       double                   &vol, 
                                       double                   &scaled_vol, 
                                       saxs_options             &our_saxs_options, 
                                       map < QString, QString > &residue_atom_hybrid_map
                                       )
{
   errormsg = "";

   QString mapkey = QString("%1|%2").arg(this_atom.resName).arg(this_atom.name);
   if ( this_atom.name == "OXT" )
   {
      mapkey = "OXT|OXT";
   }
   QString hybrid_name = residue_atom_hybrid_map[mapkey];

   if ( hybrid_name.isEmpty() || !hybrid_name.length() )
   {
      errormsg = QString("Warning: hybrid name missing for %1|%2. not added to excluded volume").arg(this_atom.resName).arg(this_atom.name);
      return false;
   }

   if ( !hybrid_map.count(hybrid_name) )
   {
      errormsg = QString("Warning: hybrid_map name missing for hybrid_name %1, not added to excluded volume").arg(hybrid_name);
      return false;
   }

   if ( !atom_map.count(this_atom.name + "~" + hybrid_name) )
   {
      errormsg = QString("Warning: atom_map missing for hybrid_name %1 atom name %2, not added to excluded volume").arg(hybrid_name).arg(this_atom.name);
      return false;
   }

   double use_vol = atom_map[this_atom.name + "~" + hybrid_name].saxs_excl_vol;
   if ( this_atom.name == "OW" && our_saxs_options.swh_excl_vol > 0e0 )
   {
      use_vol = our_saxs_options.swh_excl_vol;
   }
   vol = use_vol;
   if ( our_saxs_options.hybrid_radius_excl_vol )
   {
      use_vol = M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius;
   }
   if ( this_atom.name == "OW" )
   {
      scaled_vol = use_vol;
   } else {
      scaled_vol = use_vol * our_saxs_options.scale_excl_vol;
   } 
   return true;
}

bool US_Saxs_Util::set_excluded_volume( 
                                       PDB_atom                 &this_atom, 
                                       double                   &vol, 
                                       double                   &scaled_vol, 
                                       saxs_options             &our_saxs_options, 
                                       map < QString, QString > &residue_atom_hybrid_map,
                                       unsigned int             &total_electrons,
                                       unsigned int             &total_electrons_noh
                                       )
{
   errormsg = "";

   QString mapkey = QString("%1|%2").arg(this_atom.resName).arg(this_atom.name);
   if ( this_atom.name == "OXT" )
   {
      mapkey = "OXT|OXT";
   }
   QString hybrid_name = residue_atom_hybrid_map[mapkey];

   if ( hybrid_name.isEmpty() || !hybrid_name.length() )
   {
      errormsg = QString("Warning: hybrid name missing for %1|%2, not added to excluded volume").arg(this_atom.resName).arg(this_atom.name);
      return false;
   }

   if ( !hybrid_map.count(hybrid_name) )
   {
      errormsg = QString("Warning: hybrid_map name missing for hybrid_name %1, not added to excluded volume").arg(hybrid_name);
      return false;
   }

   if ( !atom_map.count(this_atom.name + "~" + hybrid_name) )
   {
      errormsg = QString("Warning: atom_map missing for hybrid_name %1 atom name %2, not added to excluded volume").arg(hybrid_name).arg(this_atom.name);
      return false;
   }

   double use_vol  = atom_map[this_atom.name + "~" + hybrid_name].saxs_excl_vol;
   total_electrons = hybrid_map[ hybrid_name ].num_elect;
   if ( this_atom.name == "OW" && our_saxs_options.swh_excl_vol > 0e0 )
   {
      use_vol = our_saxs_options.swh_excl_vol;
   }
   vol = use_vol;
   if ( our_saxs_options.hybrid_radius_excl_vol )
   {
      use_vol = M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius;
   }
   if ( this_atom.name == "OW" )
   {
      scaled_vol  = use_vol;
      total_electrons_noh = hybrid_map[ hybrid_name ].num_elect;
   } else {
      scaled_vol = use_vol * our_saxs_options.scale_excl_vol;
   } 
   return true;
}

bool US_Saxs_Util::calc_mychi2( vector < double > x,
                                vector < double > y,
                                vector < double > sds,
                                double            &chi2 )
{
   if ( x.size() != y.size() ||
        x.size() != sds.size() )
   {
      return false;
   }

   chi2 = 0e0;
   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      if ( sds[i] == 0e0 )
      {
         return false;
      }

      double tmp = ( x[i] - y[i] ) / sds[i];

      chi2 += tmp * tmp;
   }
   return true;
}

bool US_Saxs_Util::calc_myrmsd( vector < double > x,
                                vector < double > y,
                                double            &rmsd )
{
   if ( x.size() != y.size() )
   {
      return false;
   }

   rmsd = 0e0;
   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      double tmp = ( x[i] - y[i] );

      rmsd += tmp * tmp;
   }

   rmsd = sqrt( rmsd );
   return true;
}   


point US_Saxs_Util::minus( point p1, point p2 ) // p1 - p2
{
   point result;
   result.axis[0] = p1.axis[0] - p2.axis[0];
   result.axis[1] = p1.axis[1] - p2.axis[1];
   result.axis[2] = p1.axis[2] - p2.axis[2];
   return result;
}

point US_Saxs_Util::plus( point p1, point p2 ) // p1 + p2
{
   point result;
   result.axis[0] = p1.axis[0] + p2.axis[0];
   result.axis[1] = p1.axis[1] + p2.axis[1];
   result.axis[2] = p1.axis[2] + p2.axis[2];
   return result;
}

point US_Saxs_Util::cross( point p1, point p2) // p1 cross p2
{
   point result;
   result.axis[0] = p1.axis[1] * p2.axis[2] -  p1.axis[2] * p2.axis[1];
   result.axis[1] = p1.axis[2] * p2.axis[0] -  p1.axis[0] * p2.axis[2];
   result.axis[2] = p1.axis[0] * p2.axis[1] -  p1.axis[1] * p2.axis[0];
   return result;
}

float US_Saxs_Util::dot( point p1, point p2) // p1 dot p2
{
   return 
      p1.axis[0] * p2.axis[0] +
      p1.axis[1] * p2.axis[1] +
      p1.axis[2] * p2.axis[2];
}

point US_Saxs_Util::normal( point p1 )
{
   point result = p1;
   float divisor = sqrt( result.axis[0] * result.axis[0] +
                         result.axis[1] * result.axis[1] +
                         result.axis[2] * result.axis[2] );
   result.axis[0] /= divisor;
   result.axis[1] /= divisor;
   result.axis[2] /= divisor;
   return result;
}   

float US_Saxs_Util::dist( point p1, point p2) // sqrt( (p1 - p2) dot (p1 - p2) )
{
   point p = minus( p1, p2 );
   return sqrt( dot( p, p ) );
}

point US_Saxs_Util::plane( PDB_atom *a1, PDB_atom *a2, PDB_atom *a3 )
{
   point result = normal ( cross(
                                 minus(a3->coordinate, a2->coordinate),
                                 minus(a1->coordinate, a2->coordinate) ) );
   return result;
}

point US_Saxs_Util::plane( point p1, point p2, point p3 )
{
   point result = normal ( cross(
                                 minus( p3, p2 ),
                                 minus( p1, p2 )
                                 ) );
   return result;
}

point US_Saxs_Util::average( vector < point > *v )
{
   point result = (*v)[0];
   for ( unsigned int i = 1; i < v->size(); i++ )
   {
      result.axis[0] += (*v)[i].axis[0];
      result.axis[1] += (*v)[i].axis[1];
      result.axis[2] += (*v)[i].axis[2];
   }
   result.axis[0] /= (float) v->size();
   result.axis[1] /= (float) v->size();
   result.axis[2] /= (float) v->size();
   return result;
}

point US_Saxs_Util::scale( point p, float m )
{
   for ( unsigned int i = 0; i < 3; i++ )
   {
      p.axis[i] *= m;
   }
   return p;
}
   
bool US_Saxs_Util::get_quadratic_interpolation_coefficients( 
                                                            vector < double > x, 
                                                            vector < double > y, 
                                                            vector < double > &c 
                                                            )
{
   errormsg = "";
   if ( x.size() != 3 || y.size() != 3 )
   {
      errormsg = "US_Saxs_Util::get_quadratic_interpolation_coefficients vector sizes must be 3";
      return false;
   }

   point xp[3];
   point yp;

   for ( unsigned int i = 0; i < 3; i++ )
   {
      xp[0].axis[i] = 1e0;
      xp[1].axis[i] = x[i];
      xp[2].axis[i] = x[i] * x[i];
      yp.axis[i] = y[i];
   }

   // compute determinant

   float det = dot( xp[0], cross( xp[1], xp[2] ) );

   if ( det == 0.0 )
   {
      errormsg = "US_Saxs_Util::get_quadratic_interpolation_coefficients linearly dependent";
      return false;
   }

   float oneoverdet = 1.0 / det;

   point xi[3];

   xi[0] = scale( cross( xp[1], xp[2] ), oneoverdet );
   xi[1] = scale( cross( xp[2], xp[0] ), oneoverdet );
   xi[2] = scale( cross( xp[0], xp[1] ), oneoverdet );

   c.resize(3);

   for ( unsigned int i = 0; i < 3; i++ )
   {
      c[i] = dot( xi[i], yp );
   }

   return true;
}


// takes x1 on grid q1 and interpolates to x2 on use_q indexed points of q, putting the result in r (on q2)
bool US_Saxs_Util::linear_interpolate_iq_curve( 
                                               vector < double >       & /* q */,
                                               vector < unsigned int > &use_q,
                                               vector < double >       &x1, 
                                               vector < double >       &x2,
                                               vector < double >       &r
                                               )
{
   errormsg = "";
   if ( x1.size() != x2.size() )
   {
      errormsg = "US_Saxs_Util::linear_interpolate_iq_curve x1 & x2 must be the same size";
      return false;
   }

   unsigned int use_q_size = use_q.size();
   vector < double > deltax( use_q_size );

   for ( unsigned int i = 0; i < use_q_size; i++ )
   {
      unsigned int j = use_q[i];
      deltax[i] = x2[j] - x1[j];
   }

   r.resize(x1.size());

   for ( unsigned int i = 0; i < use_q_size - 1; i++ )
   {
      double one_over_merge_points = 1e0 / (double)(1 + use_q[i + 1] - use_q[i]);
      for ( unsigned int j = use_q[i]; j < use_q[i + 1]; j++ )
      {
         double x = (double)(j - use_q[i]) * one_over_merge_points;
         r[j] = x1[j] + deltax[i] * (1.0 - x) + deltax[i+1] * x;
      }
   }   
   return true;
}

// inputs x1, y1, x2, output y2

bool US_Saxs_Util::linear_interpolate( 
                                      vector < double > &x1,
                                      vector < double > &y1, 
                                      vector < double > &x2,
                                      vector < double > &y2
                                      )
{
   errormsg = "";
   if ( x1.size() != y1.size() )
   {
      errormsg = "US_Saxs_Util::linear_interpolate x1 & y1 must be the same size";
      return false;
   }

   if ( !x2.size() )
   {
      errormsg = "US_Saxs_Util::linear_interpolate target length needs at least 1 point";
      return false;
   }

   if ( x2[ 0 ] < x1[ 0 ] || x2.back() > x1.back() )
   {
      errormsg = "US_Saxs_Util::linear_interpolate can not extrapolate";
      return false;
   }
      
   y2.resize( x2.size() );

   for ( unsigned int i = 0; i < x2.size(); i++ )
   {
      bool found = false;
      for ( unsigned int j = 0; !found && j < x1.size() - 1; j++ )
      {
         if ( x2[ i ] >= x1[ j ] &&
              x2[ i ] <= x1[ j + 1 ] )
         {
            double pos = ( x2[ i ] - x1[ j ] ) / ( x1[ j + 1 ] - x1[ j ] );
            y2[ i ] = y1[ j ] * ( 1e0 - pos ) + y1[ j + 1 ] * pos;
            found = true;
         }
      }
      if ( !found )
      {
         errormsg = QString( "point %1 not found\n" ).arg( x2[ i ] );
         return false;
      }
   }
   return true;
}

// takes x1 on grid q1 and interpolates to x2 on use_q indexed points of q, putting the result in r (on q2)
bool US_Saxs_Util::quadratic_interpolate_iq_curve( 
                                                  vector < double >       &q,
                                                  vector < unsigned int > &use_q,
                                                  vector < double >       &x1, 
                                                  vector < double >       x2,
                                                  vector < double >       &r
                                                  )
{
   errormsg = "";
   if ( x1.size() != x2.size() )
   {
      errormsg = "US_Saxs_Util::quadratic_interpolate_iq_curve x1 & x2 must be the same size";
      return false;
   }

   if ( use_q.size() < 3 )
   {
      errormsg = "US_Saxs_Util::quadratic_interpolate_iq_curve needs at least 3 points";
      return false;
   }

   unsigned int use_q_size = use_q.size();
   vector < double > deltax( use_q_size );

   for ( unsigned int i = 0; i < use_q_size; i++ )
   {
      unsigned int j = use_q[i];
      deltax[i] = x2[j] - x1[j];
   }

   r.resize(x1.size());

   // special case: front end, take 1st 3 points

   // head, single quadratic
   {
      vector < double > x(3);
      vector < double > y(3);
      vector < double > c(3);

      unsigned int i = 0;
      for ( unsigned int k = 0; k < 3; k++ )
      {
         x[k] = q[use_q[i + k]];
         y[k] = deltax [i + k];
      }
      if ( !get_quadratic_interpolation_coefficients( x, y, c ) )
      {
         cout << errormsg << endl;
         cout << "resorting to linear interpolation\n";
         return linear_interpolate_iq_curve(q, use_q, x1, x2, r);
      }
      for ( unsigned int j = use_q[i]; j < use_q[i+1]; j++ )
      {
         r[j] = x1[j] + c[0] + c[1] * q[j] + c[2] * q[j] * q[j];
      }
   }

   // mid, double quadratic
   {
      vector < double > xa(3);
      vector < double > ya(3);
      vector < double > ca(3);

      vector < double > xb(3);
      vector < double > yb(3);
      vector < double > cb(3);
      
      for ( unsigned int i = 1; i < use_q_size - 2; i++ )
      {
         for ( unsigned int k = 0; k < 3; k++ )
         {
            xa[k] = q[use_q[i + k - 1]];
            ya[k] = deltax [i + k - 1];
            xb[k] = q[use_q[i + k]];
            yb[k] = deltax [i + k];
         }
         if ( 
             !get_quadratic_interpolation_coefficients( xa, ya, ca ) ||
             !get_quadratic_interpolation_coefficients( xb, yb, cb )
             )
         {
            cout << errormsg << endl;
            cout << "resorting to linear interpolation\n";
            return linear_interpolate_iq_curve(q, use_q, x1, x2, r);
         }
         for ( unsigned int j = use_q[i]; j < use_q[i+1]; j++ )
         {
            r[j] = x1[j] + 0.5 * (ca[0] + cb[0] + (ca[1] + cb[1]) * q[j] + (ca[2] + cb[2]) * q[j] * q[j]);
         }
      }
   }

   // tail, single quadratic looking-back
   {
      vector < double > x(3);
      vector < double > y(3);
      vector < double > c(3);

      unsigned int i = use_q_size - 2;
      for ( unsigned int k = 0; k < 3; k++ )
      {
         x[k] = q[use_q[i + k - 1]];
         y[k] = deltax [i + k - 1];
      }
      if ( !get_quadratic_interpolation_coefficients( x, y, c ) )
      {
         cout << errormsg << endl;
         cout << "resorting to linear interpolation\n";
         return linear_interpolate_iq_curve(q, use_q, x1, x2, r);
      }
      for ( unsigned int j = use_q[i]; j < use_q[i+1]; j++ )
      {
         r[j] = x1[j] + c[0] + c[1] * q[j] + c[2] * q[j] * q[j];
      }
   }

   return true;
}

// takes y on grid x1 and interpolates to grid x2 and puts result into y2

bool US_Saxs_Util::quadratic_interpolate( 
                                         vector < double > &x1,
                                         vector < double > &y1, 
                                         vector < double > &x2,
                                         vector < double > &y2
                                         )
{
   errormsg = "";
   if ( x1.size() != y1.size() )
   {
      errormsg = "US_Saxs_Util::quadratic_interpolate x1 & y1 must be the same size";
      return false;
   }

   if ( x1.size() < 3 )
   {
      errormsg = "US_Saxs_Util::quadratic_interpolate needs at least 3 points";
      return false;
   }

   if ( !x2.size() )
   {
      errormsg = "US_Saxs_Util::quadratic_interpolate target length needs at least 1 point";
      return false;
   }

   if ( x2[ 0 ] < x1[ 0 ] || x2.back() > x1.back() )
   {
      errormsg = "US_Saxs_Util::quadratic_interpolate can not extrapolate";
      return false;
   }
      
   y2.resize( x2.size() );

   for ( unsigned int i = 0; i < x2.size(); i++ )
   {
      bool found = false;
      for ( unsigned int j = 0; !found && j < x1.size() - 1; j++ )
      {
         if ( x2[ i ] >= x1[ j ] &&
              x2[ i ] <= x1[ j + 1 ] )
         {
            if ( j == 0 )
            {
               vector < double > x( 3 );
               vector < double > y( 3 );
               vector < double > c( 3 );

               for ( unsigned int k = 0; k < 3; k++ )
               {
                  x[ k ] = x1[ k ];
                  y[ k ] = y1[ k ];
               }
               if ( !get_quadratic_interpolation_coefficients( x, y, c ) )
               {
                  cout << errormsg << endl;
                  cout << "resorting to linear interpolation\n";
                  return linear_interpolate( x1, y1, x2, y2 );
               }
               y2[ i ] = c[ 0 ] + c[ 1 ] * x2[ i ] + c[ 2 ] * x2[ i ] * x2[ i ];
               found = true;
               continue;
            }
         
            if ( j >= x1.size() - 2 )
            {
               vector < double > x( 3 );
               vector < double > y( 3 );
               vector < double > c( 3 );

               for ( unsigned int k = 0; k < 3; k++ )
               {
                  x[ k ] = x1[ x1.size() + k - 3 ];
                  y[ k ] = y1[ x1.size() + k - 3 ];
               }
               if ( !get_quadratic_interpolation_coefficients( x, y, c ) )
               {
                  cout << errormsg << endl;
                  cout << "resorting to linear interpolation\n";
                  return linear_interpolate( x1, y1, x2, y2 );
               }
               y2[ i ] = c[ 0 ] + c[ 1 ] * x2[ i ] + c[ 2 ] * x2[ i ] * x2[ i ];
               found = true;
               continue;
            }
            {
               vector < double > xa(3);
               vector < double > ya(3);
               vector < double > ca(3);

               vector < double > xb(3);
               vector < double > yb(3);
               vector < double > cb(3);
      
               for ( unsigned int k = 0; k < 3; k++ )
               {
                  xa[k] = x1[ j + k - 1 ];
                  ya[k] = y1[ j + k - 1 ];
                  xb[k] = x1[ j + k ];
                  yb[k] = y1[ j + k ];
               }
               if ( 
                   !get_quadratic_interpolation_coefficients( xa, ya, ca ) ||
                   !get_quadratic_interpolation_coefficients( xb, yb, cb )
                   )
               {
                  cout << errormsg << endl;
                  cout << "resorting to linear interpolation\n";
                  return linear_interpolate( x1, y1, x2, y2 );
               }
               
               y2[ i ] =  0.5 * ( ca[ 0 ] + cb[ 0 ] + 
                                  ( ca[ 1 ] + cb[ 1 ] ) * x2[ i ] + 
                                  ( ca[ 2 ] + cb[ 2 ] ) * x2[ i ] * x2[ i ] );
               
               found = true;
               continue;
            }
         }
      }
      if ( !found )
      {
         errormsg = QString( "point %1 not found\n" ).arg( x2[ i ] );
         return false;
      }
   }
   return true;
}

// takes x1 on grid q1 and interpolates to x2 on use_q indexed points of q, putting the result in r (on q2)
bool US_Saxs_Util::cubic_spline_interpolate_iq_curve( 
                                                     vector < double >       &q,
                                                     vector < unsigned int > &use_q,
                                                     vector < double >       &x1, 
                                                     vector < double >       x2,
                                                     vector < double >       &r
                                                     )
{
   errormsg = "";
   if ( x1.size() != x2.size() )
   {
      errormsg = "US_Saxs_Util::linear_interpolate_iq_curve x1 & x2 must be the same size";
      return false;
   }

   unsigned int use_q_size = use_q.size();
   vector < double > deltax( use_q_size );
   vector < double > x     ( use_q_size );
   vector < double > y2    ( use_q_size );

   for ( unsigned int i = 0; i < use_q_size; i++ )
   {
      unsigned int j = use_q[i];
      x     [i]      = q[ j ];
      deltax[i]      = x2[j] - x1[j];
   }

   r.resize(x1.size());

   natural_spline( x, deltax, y2 );

   for ( unsigned int i = 0; i < q.size(); i++ )
   {
      if ( !apply_natural_spline( x, deltax, y2, q[ i ], r[ i ] ) )
      {
         return false;
      }
      r[ i ] += x1[ i ];
   }
      
   return true;
}

void US_Saxs_Util::natural_spline( vector < double > &x, 
                                   vector < double > &y,
                                   vector < double > &y2 )
{
   double  p;
   // double  qn;
   double  sig;
   // double  un;
   vector < double > u(x.size());
   
   y2.resize(x.size());
   
   y2[ 0 ] = u [ 0 ] = 0e0;
   
   for ( unsigned int i = 1; i < x.size() - 1; i++ ) 
   {
      sig     = ( x[ i ] - x[ i - 1 ] ) / ( x[ i + 1 ] - x[ i - 1 ] );
      p       = sig * y2[ i - 1 ] + 2e0;
      y2[ i ] = ( sig -1e0 ) / p;
      u [ i ] = ( y[ i + 1 ] - y[ i ] ) / ( x[ i + 1 ] - x[ i ]) - ( y[ i ] - y[ i - 1 ] ) / ( x[ i ] - x[ i - 1 ]);
      u [ i ] = ( 6e0 * u[ i ] / ( x[ i + 1 ] - x[ i - 1 ] ) - sig * u[ i - 1 ] ) / p;
   }
   
   // qn =  un = 0e0;
   
   y2[ x.size() - 1 ] = 0e0;
   
   for (unsigned int k = x.size() - 2; k >= 1 ; k-- )
   {
      y2[ k ] = y2[ k ] * y2[ k + 1 ] + u[ k ];
   }
}

void US_Saxs_Util::natural_spline( vector < double > &x, 
                                   vector < double > &y,
                                   vector < double > &y2,
                                   double            yp1,
                                   double            ypn
                                   )
{
   double  p;
   double  qn;
   double  sig;
   double  un;
   vector < double > u(x.size());

   unsigned int n = x.size();

   y2.resize(x.size());
   
   y2[ 0 ] = -0.5e0;
   u [ 0 ] = ( 3.0 / ( x[ 1 ] - x[0] ) ) * ( ( y[ 1 ] -y[ 0 ] ) / ( x[ 1 ] - x[ 0 ] ) - yp1 );
   
   for ( unsigned int i = 1; i < x.size() - 1; i++ ) 
   {
      sig     = ( x[ i ] - x[ i - 1 ] ) / ( x[ i + 1 ] - x[ i - 1 ] );
      p       = sig * y2[ i - 1 ] + 2e0;
      y2[ i ] = ( sig -1e0 ) / p;
      u [ i ] = ( y[ i + 1 ] - y[ i ] ) / ( x[ i + 1 ] - x[ i ]) - ( y[ i ] - y[ i - 1 ] ) / ( x[ i ] - x[ i - 1 ]);
      u [ i ] = ( 6e0 * u[ i ] / ( x[ i + 1 ] - x[ i - 1 ] ) - sig * u[ i - 1 ] ) / p;
   }
   
   qn = 0.5e0;
   un = ( 3.0 / ( x[ n - 1 ] - x[ n - 2 ] ) ) * ( ypn - ( y[ n - 1 ] - y[ n - 2 ] ) / ( x[ n - 1 ] - x[ n - 2 ] ) );

   y2[ n - 1 ] = ( un - qn * u[ n - 2 ] ) / ( qn * y2[ n - 2 ] + 1e0 );
   
   for (unsigned int k = x.size() - 2; k >= 1 ; k-- )
   {
      y2[ k ] = y2[ k ] * y2[ k + 1 ] + u[ k ];
   }
}


bool US_Saxs_Util::apply_natural_spline( vector < double > &xa, 
                                         vector < double > &ya,
                                         vector < double > &y2a,
                                         double            x,
                                         double            &y )
{
   unsigned int klo = 0;
   unsigned int khi = xa.size() - 1;

   while ( khi - klo > 1) {
      unsigned int k = ( khi + klo ) >> 1;
      if ( xa[ k ] > x )
      {
         khi = k;
      } else {
         klo = k;
      }
   }

   if ( khi == klo )
   {
      errormsg = "US_Saxs_Util::apply_natural_spline error finding point";
      return false;
   }

   double h = xa[ khi ] - xa[ klo ];

   if ( h <= 0e0 )
   {
      errormsg = "US_Saxs_Util::apply_natural_spline zero or negative interval";
      return false;
   }

   double a = ( xa[ khi ] - x ) / h;
   double b = ( x - xa[ klo ] ) / h;

   y = 
      a * ya[ klo ] +
      b * ya[ khi ] + ( ( a * a * a - a ) * y2a[ klo ] + 
                        ( b * b * b - b ) * y2a[ khi ] ) * ( h * h ) / 6e0;

   return true;
}


bool US_Saxs_Util::static_apply_natural_spline( vector < double > &xa, 
                                                vector < double > &ya,
                                                vector < double > &y2a,
                                                double            x,
                                                double            &y,
                                                QString           &errormsg )
{
   errormsg = "";

   unsigned int klo = 0;
   unsigned int khi = xa.size() - 1;

   while ( khi - klo > 1) {
      unsigned int k = ( khi + klo ) >> 1;
      if ( xa[ k ] > x )
      {
         khi = k;
      } else {
         klo = k;
      }
   }

   if ( khi == klo )
   {
      errormsg = "US_Saxs_Util::apply_natural_spline error finding point";
      return false;
   }

   double h = xa[ khi ] - xa[ klo ];

   if ( h <= 0e0 )
   {
      errormsg = "US_Saxs_Util::apply_natural_spline zero or negative interval";
      return false;
   }

   double a = ( xa[ khi ] - x ) / h;
   double b = ( x - xa[ klo ] ) / h;

   y = 
      a * ya[ klo ] +
      b * ya[ khi ] + ( ( a * a * a - a ) * y2a[ klo ] + 
                        ( b * b * b - b ) * y2a[ khi ] ) * ( h * h ) / 6e0;

   return true;
}

bool US_Saxs_Util::create_adaptive_grid( 
                                        vector < double >       &x,
                                        vector < double >       &y,
                                        unsigned int            n,
                                        vector < unsigned int>  &r
                                        )
{
   errormsg = "";
   noticemsg = "";

   if ( y.size() < 4 )
   {
      errormsg = "US_Saxs_Util::create_adaptive_grid() too few points";
      return false;
   }

   if ( n < 3 )
   {
      errormsg = "US_Saxs_Util::create_adaptive_grid() too few points requested points";
      return false;
   }

   vector < double > y2;
   natural_spline( x, y, y2 );

   for ( unsigned int i = 0; i < y2.size(); i++ )
   {
      y2[ i ] = y2[ i ] / y[ i ];
      // cout << QString("%1 %2\n").arg(x[i]).arg(y2[i]);
   }

   // find fluctuation points

   r.clear( );
   r.push_back(0); // add 1st point

   bool order = y2[ 1 ] > y2[ 0 ];
   for ( unsigned int i = 1; i < y2.size() - 1; i++ )
   {
      bool new_order = y2[ i ] > y2[ i - 1 ];
      if ( new_order != order ) 
      {
         r.push_back( i - 1 );
      }
      order = new_order;
   }

   r.push_back(y2.size() - 1); // add last point

#if defined(DEBUG_ADAPTIVE)
   cout << "before adding points:\n";
   for ( unsigned int i = 0; i < r.size(); i++ )
   {
      cout << QString("%1 %2 %3\n").arg(i).arg(r[i]).arg(x[r[i]]);
   }
#endif

   if ( r.size() < 2 )
   {
      errormsg = "US_Saxs_Util::create_adaptive_grid internal error, less than 2 points!";
      return false;
   }

   bool   room_left = true;
   double max_dist  = 1e99;
   double std_dist  = ( x[ x.size() - 1 ] - x[ 0 ] ) / n;

   while( max_dist > std_dist && room_left )
   {
      unsigned int max_dist_pos = 0;

      max_dist  = 0e0;
      room_left = false;

      for( unsigned int i = 1; i < r.size(); i++ )
      {
         double dist = x[ r[ i ] ] - x[ r [ i - 1 ] ];
         if ( dist > max_dist && r[ i ] != r[ i - 1 ] + 1 )
         {
            room_left    = true;
            max_dist     = dist;
            max_dist_pos = i - 1;
         }
      }

      if ( room_left )
      {
#if defined(DEBUG_ADAPTIVE)
         cout << QString("max dist %1 pos %2\n").arg(max_dist).arg(max_dist_pos);
#endif
         unsigned int centerpoint = ( r[ max_dist_pos + 1 ] + r[ max_dist_pos ] ) / 2;
#if defined(DEBUG_ADAPTIVE)
         cout << QString("r1 %1 r2 %2 centerpoint %3\n").arg(r[ max_dist_pos + 1 ]).arg(r[ max_dist_pos]).arg(centerpoint);
#endif
         if ( centerpoint == r[ max_dist_pos + 1 ] || centerpoint == r[ max_dist_pos ] )
         {
            errormsg = "US_Saxs_Util::create_adaptive_grid internal error centerpoint already present!";
            return false;
         }
#if defined(DEBUG_ADAPTIVE)
         cout << QString("max dist %1 pos %2 adding %3\n").arg(max_dist).arg(max_dist_pos).arg(centerpoint);
#endif
         vector < unsigned int > r_new = r;
         r_new.resize( max_dist_pos + 1 );
         r_new.push_back( centerpoint );
         for( unsigned int i = max_dist_pos + 1 ; i < r.size(); i++ )
         {
            r_new.push_back( r[ i ] );
         }
         r = r_new;
      }
   }

#if defined(DEBUG_ADAPTIVE)
   cout << "after adding points:\n";
   for ( unsigned int i = 0; i < r.size(); i++ )
   {
      cout << QString("%1 %2\n").arg(r[i]).arg(x[r[i]]);
   }
#endif
   int pointsleft = n - r.size();
   if ( pointsleft < 0 )
   {
      noticemsg = QString("Adaptive method forced %1 more points than the requested number (%2) for a total of %3 points")
         .arg(-pointsleft)
         .arg(n)
         .arg(r.size());
      return true;
   }

   if ( pointsleft == 0 )
   {
      noticemsg = QString("Adaptive method used exactly the requested number %1 of points")
         .arg(n);
         return true;
   }

   return true;
}

bool US_Saxs_Util::compute_rayleigh_structure_factors( 
                                                      double            radius,
                                                      double            delta_rho,
                                                      vector < double > &q,
                                                      vector < double > &F
                                                      )
{
   errormsg = "";

   if ( radius <= 0e0 )
   {
      errormsg = "US_Saxs_Util::compute_rayleigh_structure_factors radius must be greater than zero";
      return false;
   }

   F.resize( q.size() );

   double v          = ( 4.0 / 3.0 ) * M_PI * radius * radius * radius;
   double delta_rhov = delta_rho * v;
   
   for ( unsigned int i = 0; i < q.size(); i++ )
   {
      double qradius = q[ i ] * radius;

      F[ i ] = 
         delta_rhov * 
         3.0 * ( sin( qradius ) - qradius * cos( qradius ) ) / ( qradius * qradius * qradius );
   }

   return true;
}   


// takes from_data with from_errors and places on to_grid resulting in to_data & to_errors
// for systems without errors, these are assumed to be computed iq curves so a natural spline interpolation
// is performed
// for systems with errors, these are assumed to be experimental data it is only acceptible going
// to a coarser grid, otherwise an error will result with error_msg set

bool US_Saxs_Util::interpolate_iqq_by_case( vector < double > from_grid,
                                            vector < double > from_data,
                                            vector < double > from_errors,
                                            vector < double > to_grid,
                                            vector < double > &to_data,
                                            vector < double > &to_errors )
{
   errormsg  = "";
   noticemsg = "";

   if ( to_grid.size() < 3 || from_grid.size() < 3 )
   {
      errormsg = "Attempt to interpolate failed, to or from grid too few points (< 3)";
      return false;
   }

   if ( to_grid[ 0 ] + 5e-3 < from_grid[ 0 ] || 
        to_grid[ to_grid.size() - 1 ] > from_grid[ from_grid.size() - 1 ] + 5e-3 )
   {
      errormsg = "Can not extrapolate data";
      return false;
   }

   if ( !is_nonzero_vector( from_errors ) ||
        from_errors.size() != from_data.size() )
   {
      if ( is_nonzero_vector( from_errors ) &&
           from_errors.size() != from_data.size() )
      {
         cout << QString("interpolate iqq_by_case, errors size %1 does not match data size %2\n").arg(from_errors.size()).arg(from_data.size());
      }
         
      // calculated curve
      cout << "interpolate iqq_by_case, computed data so using natural spline\n";
      vector < double > y2;
      natural_spline( from_grid, from_data, y2 );
      to_data  .resize( to_grid.size() );
      to_errors.resize( to_grid.size() );

      for ( unsigned int i = 0; i < to_grid.size(); i++ )
      {
         to_errors[ i ] = 0e0;
         if ( !apply_natural_spline( from_grid, from_data, y2, to_grid[ i ], to_data[ i ] ) )
         {
            return false;
         }
      }
      return true;
   }

   // experimental data:
   cout << "interpolate iqq_by_case, experimental data so using mean of all points in range\n";

   if ( from_grid == to_grid )
   {
      cout << "identical grids, no interp needed\n";
      to_data   = from_data;
      to_errors = from_errors;
      return true;
   }
      
   if ( to_grid[ 0 ] + 5e-3 < from_grid[ 0 ] || 
        to_grid[ to_grid.size() - 1 ] > from_grid[ from_grid.size() - 1 ] )
   {
      errormsg = "Can not extrapolate experimental data";
      return false;
   }

   if ( to_grid.size() > from_grid.size() )
   {
      errormsg = "Interpolation of experimental data to a finer grid not allowed";
      return false;
   }

   // ok, we are in the range so we can interpolate

   to_data  .resize( to_grid.size() );
   to_errors.resize( to_grid.size() );

   // 1st point
   cout << QString("iibc from_grid.size() %1, to_grid.size() %2\n").arg(from_grid.size()).arg(to_grid.size());
   cout << "from grid:\n";
   for ( unsigned int i = 0; i < from_grid.size(); i++ )
   {
      cout << QString("%1, ").arg(from_grid[i]);
   }
   cout << endl;
   cout << "to grid:\n";
   for ( unsigned int i = 0; i < to_grid.size(); i++ )
   {
      cout << QString("%1, ").arg(to_grid[i]);
   }
   cout << endl;

   // we should redo this so it works in general
   // maybe build some sort of a vector < vector > for each point and
   // so on...
   // for now, lets give some tolerance
   // spacing computation

   vector < unsigned int > closest_from_point;
   for ( unsigned int i = 0; i < to_grid.size(); i++ )
   {
      double       min_deltaq    = fabs( to_grid[ i ] - from_grid[ 0 ] );
      unsigned int closest_point = 0;

      for ( unsigned int pos = 1; pos < from_grid.size(); pos++ )
      {
         double this_deltaq = fabs( to_grid[ i ] - from_grid[ pos ] );
         if ( min_deltaq > this_deltaq )
         {
            min_deltaq    = this_deltaq;
            closest_point = pos;
         }
      }
      closest_from_point.push_back( closest_point );
   }

   {
      double sum = 0e0;
      double err = 0e0;
      double tot_w = 0e0;
      unsigned int count = 0;
      
      double q_start = to_grid[ 0 ] - ( to_grid[ 1 ] - to_grid[ 0 ] ) / 2;
      double q_end   = to_grid[ 0 ] + ( to_grid[ 1 ] - to_grid[ 0 ] ) / 2;
      for ( unsigned int pos = 0; pos < from_grid.size() && q_end >= from_grid[ pos ]; pos++ )
      {
         if ( q_start <= from_grid[ pos ] )
         {
            double w = 1e0 / from_errors[ pos ];
            tot_w += w;
            sum += from_data  [ pos ] * w;
            err += from_errors[ pos ];
            count++;
         }
      }
      
      if ( !count )
      {
         errormsg = QString( "Warning: Interpolation of experimental data found no points within range %1:%2, using closest point %3\n")
            .arg( q_start ).arg( q_end ).arg( from_grid[ closest_from_point[ 0 ] ] );
         cout << errormsg;
         to_data  [ 0 ] = from_data  [ closest_from_point[ 0 ] ];
         to_errors[ 0 ] = from_errors[ closest_from_point[ 0 ] ];
      } else {
         to_data  [ 0 ] = sum / tot_w;
         to_errors[ 0 ] = err / count;
      }
      // cout << QString("q_start %1 q_end %2 ").arg( q_start ).arg( q_end );
      // cout << QString("iibc p 0 sum %1 err %2 tot_w %3 count %4\n").arg( sum ).arg( err ).arg( tot_w ).arg( count );
   }

   for ( unsigned int i = 1; i < to_grid.size() - 1; i++ )
   {
      double sum = 0e0;
      double err = 0e0;
      double tot_w = 0e0;
      unsigned int count = 0;
      
      double q_start = to_grid[ i - 1 ] + ( to_grid[ i ]     - to_grid[ i - 1 ] ) / 2;
      double q_end   = to_grid[ i ]     + ( to_grid[ i + 1 ] - to_grid[ i ]     ) / 2;

      for ( unsigned int pos = 0; pos < from_grid.size() && q_end >= from_grid[ pos ]; pos++ )
      {
         if ( q_start <= from_grid[ pos ] )
         {
            double w = 1e0 / from_errors[ pos ];
            tot_w += w;
            sum += from_data  [ pos ] * w;
            err += from_errors[ pos ];
            count++;
         }
      }

      if ( !count )
      {
         errormsg = QString( "Warning: Interpolation of experimental data found no points within range %1:%2, using closest point %3\n")
            .arg( q_start ).arg( q_end ).arg( from_grid[ closest_from_point[ i ] ] );
         cout << errormsg;
         to_data  [ i ] = from_data  [ closest_from_point[ i ] ];
         to_errors[ i ] = from_errors[ closest_from_point[ i ] ];
      } else {
         to_data  [ i ] = sum / tot_w;
         to_errors[ i ] = err / count;
      }

      // cout << QString("q_start %1 q_end %2 ").arg( q_start ).arg( q_end );
      // cout << QString("iibc p %1 sum %2 err %3 tot_w %4 count %5\n").arg( i ).arg( sum ).arg( err ).arg( tot_w ).arg( count );
   }

   // last point

   {
      double sum = 0e0;
      double err = 0e0;
      double tot_w = 0e0;
      unsigned int count = 0;
      
      double q_start = to_grid[ to_grid.size() - 2 ] + ( to_grid[ to_grid.size() - 1 ] - to_grid[ to_grid.size() - 2 ] ) / 2;
      double q_end   = to_grid[ to_grid.size() - 1 ] + ( to_grid[ to_grid.size() - 1 ] - to_grid[ to_grid.size() - 2 ] ) / 2;
      
      for ( int pos = from_grid.size() - 1; pos >= 0 && q_start <= from_grid[ pos ]; pos-- )
      {
         if ( from_grid[ pos ] <= q_end )
         {
            double w = 1e0 / from_errors[ pos ];
            tot_w += w;
            sum += from_data  [ pos ] * w;
            err += from_errors[ pos ];
            count++;
         }
      }
      
      if ( !count )
      {
         errormsg = QString( "Warning: Interpolation of experimental data found no points within range %1:%2, using closest point %3\n")
            .arg( q_start ).arg( q_end ).arg( from_grid[ closest_from_point[ to_grid.size() - 1 ] ] );
         cout << errormsg;
         to_data  [ to_grid.size() - 1 ] = from_data  [ closest_from_point[ to_grid.size() - 1 ] ];
         to_errors[ to_grid.size() - 1 ] = from_errors[ closest_from_point[ to_grid.size() - 1 ] ];
      } else {
         to_data  [ to_grid.size() - 1 ] = sum / tot_w;
         to_errors[ to_grid.size() - 1 ] = err / count;
      }

      // cout << QString("q_start %1 q_end %2 ").arg( q_start ).arg( q_end );
      // cout << QString("iibc p %1 sum %2 err %3 tot_w %4 count %5\n").arg( to_grid.size() - 1 ).arg( sum ).arg( err ).arg( tot_w ).arg( count );
   }

   return true;
}


bool US_Saxs_Util::crop( vector < double > from_grid,
                         vector < double > from_data,
                         vector < double > from_errors,
                         double            min_q,
                         double            max_q,
                         vector < double > &to_data,
                         vector < double > &to_errors )
{
   errormsg = "";
   noticemsg = "";
   to_data.clear( );
   to_errors.clear( );

   if ( from_grid.size() != from_data.size() )
   {
      errormsg = QString("Error: crop: grid size %1  does not equal data size %2").arg( from_grid.size() ).arg( from_data.size() );
      return false;
   }

   bool use_errors = ( from_errors.size() == from_data.size() );
   if ( from_errors.size() && !use_errors )
   {
      errormsg = QString("Error: crop: grid size %1  does not equal errors size %2").arg( from_grid.size() ).arg( from_errors.size() );
      errormsg = "Error: crop: grid size incompatibile";
      return false;
   }

   for ( unsigned int i = 0; i < from_grid.size(); i++ )
   {
      if ( from_grid[ i ] >= min_q &&
           from_grid[ i ] <= max_q )
      {
         to_data.push_back( from_data[ i ] );
         if ( use_errors )
         {
            to_errors.push_back( from_errors[ i ] );
         }
      }
   }
   return true;
}
