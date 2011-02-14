#include "../include/us_saxs_util.h"

void US_Saxs_Scan::clear()
{
   filename = "";
   header = "";
   header_cols.clear();
   q.clear();
   r.clear();
   s.clear();
}

US_Saxs_Util::US_Saxs_Util()
{
   debug = 0;
   wsgbs_gsm_setup = false;
   this_rank = 0;
}

void US_Saxs_Util::clear()
{
   wave.clear();
}

bool US_Saxs_Util::read(QString filename, QString tag)
{
   // read filename into wave[tag]

   wave[tag].clear();
   wave[tag].filename = filename;

   errormsg = "";

   QFile f(filename);
   
   if ( f.open(IO_ReadOnly) )
   {
      QTextStream ts(&f);
      wave[tag].header = ts.readLine();
      QRegExp rx3("^\\s*(\\S*)\\s+(\\S*)\\s+(\\S*)\\s*$");
      if ( rx3.search(wave[tag].header) == -1 )
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
         line = ts.readLine();
         if ( rx3.search(line) == -1 )
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

   if ( f.open(IO_WriteOnly ) )
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
         wave[outtag].s[j] += wave[tags[i]].r[j];
      }
   }

   for ( unsigned int j = 0; j < wave[outtag].q.size(); j++ )
   {
      wave[outtag].r[j] /= tags.size();
      wave[outtag].s[j] /= tags.size();
   }
   return true;
}

bool US_Saxs_Util::crop(QString outtag, QString fromtag, double low, double high)
{
   errormsg = "";

   wave[outtag].clear();
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

   wave[outtag].clear();
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

double US_Saxs_Util::rmsd(QString tag1, QString tag2)
{
   errormsg = "";

   if ( !compat(tag1, tag2) )
   {
      return false;
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
   wave[outtag].clear();
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

bool US_Saxs_Util::guinier_plot(QString outtag, QString tag)
{
   errormsg = "";
   wave[outtag].clear();
   wave[outtag].filename = QString("guinier_%1").arg(tag);
   wave[outtag].header = wave[tag].header;
   
   for( unsigned int i = 0; i < wave[tag].q.size(); i++ )
   {
      wave[outtag].q.push_back(wave[tag].q[i] * wave[tag].q[i]);
      wave[outtag].r.push_back(wave[tag].r[i] > 0 ? log(wave[tag].r[i]) : 0e0);
      wave[outtag].s.push_back(wave[tag].s[i]);
   }

   return true;
}

bool US_Saxs_Util::guinier_fit( 
                               QString &log,
                               QString tag,  // tag needs to be preprocessed with guinierplot
                               unsigned int startpos,
                               unsigned int endpos,
                               double &a,
                               double &b,
                               double &siga,
                               double &sigb,
                               double &chi2,
                               double &Rg,
                               double &Io,
                               double &smax,
                               double &smin,
                               double &sRgmin,
                               double &sRgmax
                               )
{
   vector < double > x;
   vector < double > y;
   errormsg = "";
   if ( startpos > endpos - 10 )
   {
      errormsg = "Guinier fit needs at least 10 points";
      return false;
   }

   if ( wave[tag].q.size() <= endpos )
   {
      errormsg = QString("end position %1 is past the end of the wave %2") 
                         .arg(endpos)
                         .arg(wave[tag].q.size());
      return false;
   }
      
   for( unsigned int i = startpos; i <= endpos; i++ )
   {
      x.push_back(wave[tag].q[i]);
      y.push_back(wave[tag].r[i]);
   }

   linear_fit(x, y, a, b, siga, sigb, chi2);
   Rg = sqrt( -3e0 * b );
   Io = exp(a);
   smin = sqrt(wave[tag].q[startpos]);
   smax = sqrt(wave[tag].q[endpos]);
   sRgmin = Rg * smin;
   sRgmax = Rg * smax;

   log += QString("").sprintf("|| %u || %u || %g || %g || %g || %g || %g || %g || %g || %g || %g || %g || %g ||\n",
                              startpos + 1,
                              endpos + 1,
                              smin,
                              smax,
                              sRgmin,
                              sRgmax,
                              Rg,
                              Io,
                              a,
                              b,
                              siga,
                              sigb,
                              chi2);
   log += QString("pnggnuplot.pl -p 1.5 -g -l points -c %1 %1 -m %1 %1 %1 %1 %1g%1_%1.png %1g.dat\n")
      .arg(wave[tag].q[startpos]* .2 )
      .arg(wave[tag].q[endpos]* 1.2 )
      .arg(a)
      .arg(b)
      .arg(smin)
      .arg(smax)
      .arg(tag)
      .arg(startpos+1)
      .arg(endpos+1)
      .arg(tag);
;
   log += QString("[[Image(htdocs:pngs/%1g%1_%1.png)]]\n")
      .arg(tag)
      .arg(startpos+1)
      .arg(endpos+1);

   return true;
}

bool US_Saxs_Util::guinier_fit2( 
                                QString &log,
                                QString tag,             // tag needs to be preprocessed with guinierplot
                                unsigned int pointsmin,  // the minimum # of points allowed typically 10
                                unsigned int pointsmax,  // the maximum # of points allowed typically 100
                                double sRgmaxlimit,      // maximum sRg allowed! typically 1.3
                                double pointweightpower, // the exponent ofnumber of points when computing the best one (3 seems to work well)
                                //                          i.e. fitness = chi2 / ( number_of_points ** pointweightpower )
                                double &a,
                                double &b,
                                double &siga,
                                double &sigb,
                                double &chi2,
                                double &Rg,
                                double &Io,
                                double &smin,
                                double &smax,
                                double &sRgmin,
                                double &sRgmax,
                                unsigned int &beststart,
                                unsigned int &bestend
                       )
{
   errormsg = "";
   double bestfitness = 9e99;
   beststart = 0;
   bestend = 0;
   double fitness;

   for ( unsigned int i = 0; i < wave[tag].q.size() - pointsmin; i++ )
   {
      for ( unsigned int j = i + pointsmin ; j < wave[tag].q.size() && j - i <= pointsmax; j++ )
      {
         QString mylog; // only keep the good ones
         if ( !guinier_fit(
                           mylog,
                           tag,
                           i,
                           j,
                           a,
                           b,
                           siga,
                           sigb,
                           chi2,
                           Rg,
                           Io,
                           smin,
                           smax,
                           sRgmin,
                           sRgmax
                           ) )
         {
            log += mylog;
            return false;
         }
         
         if ( sRgmax > sRgmaxlimit ) {
            break;
         }
         if ( isnan(Rg) )  // positive slope
         {
            continue;
         }
         // log += mylog;
         fitness = chi2 / pow(j - i, pointweightpower);
         if ( fitness < bestfitness )
         {
            bestfitness = fitness;
            beststart = i;
            bestend = j;
         }

      }
         
   }


   if ( !guinier_fit(
                     log,
                     tag,
                     beststart,
                     bestend,
                     a,
                     b,
                     siga,
                     sigb,
                     chi2,
                     Rg,
                     Io,
                     smin,
                     smax,
                     sRgmin,
                     sRgmax
                     ) )
   {
      return false;
   }
   
   return true;
}

bool US_Saxs_Util::subbackground(QString outtag, QString solutiontag, QString buffertag, double alpha)
{
   errormsg = "";

   wave[outtag].clear();
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

   wave[outtag].clear();
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

   wave[outtag].clear();
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

   wave[outcrop].clear();

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
   wsgbs_gsm_setup = true;
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
   if ( !wsgbs_gsm_setup )
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
   struct timeval tv1, tv2;
   our_vector *u, *zero;
   //  double t;
   double s1, s2, s3;
   double g_s1, g_s2, g_s3, g_s4;
   our_vector *v_s1, *v_s2, *v_s3, *v_tmp, *v_s4;
   double a, b, x;
   double s1_s2, s1_s3, s2_s3;
   double s1_2, s2_2, s3_2;
   double prev_s2, prev_g_s2;
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
      gettimeofday(&tv1, NULL);

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
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      printf("%d: start line search\n", this_rank); 
      fflush(stdout);
      gettimeofday(&tv1, NULL);
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
         prev_s2 = s2;

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
         case 8 : i->d[(6 * N_2) - 1] = 0;
         case 7 :
         case 6 : i->d[(4 * N_2) - 1] = 0;
         case 5 :
         case 4 : i->d[(2 * N_2) - 1] = 0; 
         default : break;
         }
      }
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
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
   struct timeval tv1, tv2;
   our_vector *u, *zero;
   //  double t;
   double s1, s2, s3;
   double g_s1, g_s2, g_s3, g_s4;
   our_vector *v_s1, *v_s2, *v_s3, *v_tmp, *v_s4, *v_h, *v_g;
   double a, b, x;
   double s1_s2, s1_s3, s2_s3;
   double s1_2, s2_2, s3_2;
   double prev_s2, prev_g_s2;
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

      gettimeofday(&tv1, NULL);

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

      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      printf("%d: start line search\n", this_rank); 
      fflush(stdout);
      gettimeofday(&tv1, NULL);
    
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
         prev_s2 = s2;

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
         case 8 : i->d[(6 * N_2) - 1] = 0;
         case 7 :
         case 6 : i->d[(4 * N_2) - 1] = 0;
         case 5 :
         case 4 : i->d[(2 * N_2) - 1] = 0; 
         default : break;
         }
      }

      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      printf("%d: df start\n", this_rank); 
      fflush(stdout);
      wgsbs_gsm_df(u, i);
      puts("conj dir start");
      fflush(stdout);
      gettimeofday(&tv1, NULL);
    
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
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
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
  
   struct timeval tv1, tv2;

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
   double prev_s2, prev_g_s2;
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
      gettimeofday(&tv1, NULL);

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

      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      printf("%d: start line search\n", this_rank); 
      fflush(stdout);
      gettimeofday(&tv1, NULL);

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
         prev_s2 = s2;

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
         case 8 : v_p->d[(6 * N_2) - 1] = 0;
         case 7 :
         case 6 : v_p->d[(4 * N_2) - 1] = 0;
         case 5 :
         case 4 : v_p->d[(2 * N_2) - 1] = 0; 
         default : break;
         }
      }

      /*    printf("v_dx:"); print_our_vector(v_dx); */

      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      printf("%d: df start\n", this_rank); 
      fflush(stdout);
      wgsbs_gsm_df(v_g, v_p);                 /* new gradient in v_g (old in u) */
      printf("%d: hessian start\n", this_rank);
      gettimeofday(&tv1, NULL);

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
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      fflush(stdout);

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

void US_Saxs_Util::linear_fit( 
                              vector < double > x, 
                              vector < double > y, 
                              double &a,
                              double &b,
                              double &siga,
                              double &sigb,
                              double &chi2
                              )
{
   unsigned int i;
   double t;
   double sxoss;
   double sx = 0e0;
   double sy = 0e0;
   double st2 = 0e0;
   double ss;
   double sigdat;
   unsigned int ndata = x.size();
   b = 0e0;

   for ( i = 0; i < ndata; i++ )
   {
      sx += x[i];
      sy += y[i];
   }
   ss = ndata;

   sxoss = sx / ss;

   for ( i = 0; i < ndata; i++ ) 
   {
      t = x[i] - sxoss;
      st2 += t * t;
      b += t * y[i];
   }
   b /= st2;
   a = ( sy - sx * b) / ss;
   siga = sqrt( ( 1e0 + sx * sx / ( ss * st2 ) ) / ss );
   sigb = sqrt( 1e0 /st2 );
   chi2 = 0e0;

   for ( i = 0; i < ndata; i++ ) 
   {
      chi2 += ( y[i] - a - b * x[i] ) * ( y[i] - a - b * x[i] );
   }
   sigdat = sqrt( chi2/ ( ndata - 2 ) );
   siga *= sigdat;
   sigb *= sigdat;
}

void US_Saxs_Util::clear_project()
{
   wave.clear();
   p_project = "";
   p_wiki = "";
   p_wiki_prefix = "";
   p_name = "";
   p_description = "";
   p_short_description = "";
   p_comment = "";
   p_mw = 0e0;
   p_conc_mult = 0e0;
   wave_names_vector.clear();
   wave_names.clear();
   wave_file_names.clear();
   wave_types.clear();
   wave_concs.clear();
   wave_alphas.clear();
   wave_betas.clear();
   wave_consts.clear();
   wave_exposure_times.clear();
   wave_buffer_names.clear();
   wave_empty_names.clear();
   wave_comments.clear();
   wave_Rgs.clear();
   wave_Ios.clear();
   wave_smins.clear();
   wave_smaxs.clear();
   wave_sRgmins.clear();
   wave_sRgmaxs.clear();
   wave_chi2s.clear();
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
            QString("%1file %1 does not exist")
            .arg(errormsg.isEmpty() ? "" : "\n" )
            .arg(fi.filePath());
      } else {
         if ( !fi.isReadable() )
         {
            errormsg +=
               QString("%1file %1 is not readable (check permissions)")
               .arg(errormsg.isEmpty() ? "" : "\n" )
               .arg(fi.filePath());
         }
         if ( fi.isDir() )
         {
            errormsg +=
               QString("%1file %1 is a directory!")
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
            QString("%1wave %1 has no associated buffers")
            .arg(errormsg.isEmpty() ? "" : "\n" )
            .arg(wave_names_vector[i]);
      }
   }
   return errormsg.isEmpty();
}

bool US_Saxs_Util::read_project()
{
   errormsg = "";
   QFile f("project");
   if ( !f.exists() )
   {
      errormsg = "project file does not exist";
      return false;
   }

   if ( !f.open( IO_ReadOnly ) )
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
                   "overlaplowq|"
                   "overlaphighq|"
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
                   ")$"
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

      if ( rx.search(line) == -1 )
      {
         errormsg = QString("error in project file line %1.  At least two tokens not found <%1>\n")
            .arg(linepos)
            .arg(line);
         return false;
      }

      QString token = rx.cap(1).lower();
      QString data = rx.cap(2);

      if ( !token.contains(rxvalid) )
      {
         errormsg = QString("error in project file line %1.  Unrecognized token <%1>\n")
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

      if ( token == "wavename" )
      {
         if ( wave_names.count(data) )
         {
            errormsg = QString("error in project file line %1.  duplicate waveName previously defined <%1>\n")
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
         continue;
      }

      // all wave based data setup follows

      if ( last_wave_name.isEmpty() )
      {
         errormsg = QString("error in project file line %1.  waveName must be defined first <%1>\n")
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
            errormsg = QString("error in project file line %1.  Invalid waveType <%1>\n")
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
            errormsg = QString("error in project file line %1.  waveName for buffer not previously defined <%1>\n")
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
            errormsg = QString("error in project file line %1.  waveName for empty not previously defined <%1>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
         wave_buffer_names[last_wave_name].push_back(data);
         continue;
      }

      if ( token == "wavealpha" )
      {
         wave_alphas[last_wave_name] = data.toDouble();
         continue;
      }
      if ( token == "wavebeta" )
      {
         if ( wave_types[data] != "waxs" )
         {
            errormsg = QString("error in project file line %1.  waveType must be waxs for beta assignment <%1>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
         wave_betas[last_wave_name] = data.toDouble();
         continue;
      }

      if ( token == "waveconst" )
      {
         if ( wave_types[data] != "waxs" )
         {
            errormsg = QString("error in project file line %1.  waveType must be waxs for const assignment <%1>\n")
               .arg(linepos)
               .arg(line);
            return false;
         }
         wave_consts[last_wave_name] = data.toDouble();
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

   cout << "wiki file name:" << wiki_file_name() << endl;

   return true;
}
  
bool US_Saxs_Util::build_wiki()
{
   errormsg = "";
   QFile f(wiki_file_name());
   if ( !f.open( IO_WriteOnly ) )
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
      if ( !wave_alphas[wave_names_vector[i]] )
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
      QString("cwave%1%1_%1.dat")
      .arg(QDir::separator())
      .arg(base.replace(QRegExp("\\.(dat|DAT)$"),""))
      .arg(type);
}

// for sorting by concentration
class sortable_conc_series {
public:
   QString name;
   double conc;
   bool operator < (const sortable_conc_series& objIn) const
   {
      return conc < objIn.conc;
   }
};

bool US_Saxs_Util::wiki(QString &result)
{
   errormsg = "";
   result =
      QString(
              "= %1 =\n"
              "== Overview: %1 ==\n"
              " * %1\n"
              " * MW %1 Daltons\n"
              " * concentration multiplier for standard saxs buffer subtraction: %1\n"
              " * %1\n"
              "== Project source file ==\n"
              "{{{\n%1}}}\n"
              "== Sample summary ==\n"
              "|| name || saxs or waxs || conc mg/ml || alpha || source file || comments ||\n"
              )
      .arg(p_short_description)
      .arg(p_name)
      .arg(p_description)
      .arg(p_mw)
      .arg(p_conc_mult)
      .arg(p_comment)
      .arg(p_project)
      ;

   for ( unsigned int i = 0; i < wave_names_vector.size(); i++ )
   {
      result +=
         QString(
                 "|| %1 || %1 || %1 || %1 || %1 || %1 ||\n"
                 )
         .arg(wave_names_vector[i])
         .arg(wave_types[wave_names_vector[i]])
         .arg(wave_concs[wave_names_vector[i]] ? QString("%1").arg(wave_concs[wave_names_vector[i]]) : "buffer" )
         .arg(wave_concs[wave_names_vector[i]] ? QString("%1").arg(wave_alphas[wave_names_vector[i]]) : "" )
         .arg(wave_file_names[wave_names_vector[i]])
         .arg(wave_comments[wave_names_vector[i]])
         ;
   }

   result += 
      "== SAXS standard buffer subtraction ==\n"
      ;
   for ( unsigned int i = 0; i < wave_names_vector.size(); i++ )
   {
      if ( 
          wave_types[wave_names_vector[i]] == "saxs" &&
          wave_concs[wave_names_vector[i]]
          )
      {
         // first the main sample with concentration
         result +=
            QString(
                    "=== Sample %1 ===\n"
                    "|| name || saxs or waxs || conc mg/ml || alpha || source file || comments ||\n"
                    "|| %1 || %1 || %1 || %1 || %1 || %1 ||\n"
                    )
            .arg(wave_names_vector[i])
            .arg(wave_names_vector[i])
            .arg(wave_types[wave_names_vector[i]])
            .arg(wave_concs[wave_names_vector[i]] ? QString("%1").arg(wave_concs[wave_names_vector[i]]) : "buffer" )
            .arg(wave_concs[wave_names_vector[i]] ? QString("%1").arg(wave_alphas[wave_names_vector[i]]) : "" )
            .arg(wave_file_names[wave_names_vector[i]])
            .arg(wave_comments[wave_names_vector[i]])
            ;
         // then the associated buffers
         for ( unsigned int j = 0; j < wave_buffer_names[wave_names_vector[i]].size(); j++ )
         {
            result += 
               QString(
                       "|| %1 || %1 || %1 || %1 || %1 || %1 ||\n"
                       )
               .arg(wave_buffer_names[wave_names_vector[i]][j])
               .arg(wave_types[wave_buffer_names[wave_names_vector[i]][j]])
               .arg(wave_concs[wave_buffer_names[wave_names_vector[i]][j]]? QString("%1").arg(wave_concs[wave_buffer_names[wave_names_vector[i]][j]]) : "buffer" )
               .arg(wave_concs[wave_buffer_names[wave_names_vector[i]][j]] ? QString("%1").arg(wave_alphas[wave_buffer_names[wave_names_vector[i]][j]]) : "" )
               .arg(wave_file_names[wave_buffer_names[wave_names_vector[i]][j]])
               .arg(wave_comments[wave_buffer_names[wave_names_vector[i]][j]])
               ;
         }
         QString pngfile = QString("%1_sbs_%1.png").arg(p_name).arg(wave_names_vector[i]);
         result += QString("[[Image(htdocs:pngs%1%1)]]\n").arg(QDir::separator()).arg(pngfile);
         // compute the wave now
         QString outfile = get_file_name(wave_names_vector[i],"bsub");
         if ( 
             !subbackground(outfile, wave_names_vector[i], wave_names_vector[i] + "|buffer", wave_alphas[wave_names_vector[i]]) ||
             !write(outfile, outfile) 
             )
         {
            return false;
         }
         QString cmd =
            QString(
                    "pnggnuplot.pl -c %1 %1 %1 %1 %1 %1\n"
                    )
            .arg(p_saxs_lowq)
            .arg(p_saxs_highq)
            .arg(QString("pngs%1%1").arg(QDir::separator()).arg(pngfile))
            .arg(QString("wave%1%1").arg(QDir::separator()).arg(wave_file_names[wave_names_vector[i]]))
            .arg(get_file_name(wave_names_vector[i],"buffer"))
            .arg(get_file_name(wave_names_vector[i],"bsub"))
            ;
         cout << cmd;
         system(cmd.ascii());
      }
   }

   // saxs concentration series
   // potential exposure time series merge (loop around ?)

   result += "== SAXS concentration series ==\n";

   // sort 
   sortable_conc_series tmp_conc;
   list < sortable_conc_series > conc_series;
   
   for ( unsigned int i = 0; i < wave_names_vector.size(); i++ )
   {
      if ( 
          wave_types[wave_names_vector[i]] == "saxs" &&
          wave_concs[wave_names_vector[i]]
          )
      {
         tmp_conc.name = wave_names_vector[i];
         tmp_conc.conc = wave_concs[wave_names_vector[i]];
         conc_series.push_back(tmp_conc);
      }
   }
   conc_series.sort();
   result +=
      "|| name || saxs or waxs || conc mg/ml || alpha || source file || comments ||\n";

   QString pngfile = QString("pngs%1%1_saxs_conc_series.png")
      .arg(QDir::separator())
      .arg(p_name);
      
   QString cmd = 
      QString("pnggnuplot.pl -c %1 %1 %1")
      .arg(p_saxs_lowq)
      .arg(p_saxs_highq)
      .arg(pngfile);

   vector < QString > saxs_conc_names; 

   for ( list < sortable_conc_series >::iterator it = conc_series.begin();
         it != conc_series.end();
         it++ )
   {
      it->name;
      saxs_conc_names.push_back(it->name);
      QString outfile = get_file_name(it->name,"bsub");
      cmd += " " + outfile;
      result +=
         QString(
                 "|| %1 || %1 || %1 || %1 || %1 || %1 ||\n"
                 )
         .arg(it->name)
         .arg(wave_types[it->name])
         .arg(wave_concs[it->name] ? QString("%1").arg(wave_concs[it->name]) : "buffer" )
         .arg(wave_concs[it->name] ? QString("%1").arg(wave_alphas[it->name]) : "" )
         .arg(QString("%1").arg(outfile).replace(QRegExp("^cwave."),""))
         .arg(wave_comments[it->name])
         ;
   }
   result += QString("[[Image(htdocs:%1)]]\n").arg(pngfile);
   cout << cmd << endl;
   system(cmd.ascii());
   
   // saxs Rg/Io series

   result += "== SAXS Rg/Io computations ==\n";

   QString guinier_summary = 
      "=== SAXS Guinier Summary ===\n"
      "|| name || saxs or waxs || conc mg/ml || alpha || source file || comments || Rg || Io || Io/conc || qRg min || qRg max || q min || q max || chi2 || start point || end point || points used ||\n"
      ;

   for ( unsigned int i = 0; i < saxs_conc_names.size(); i++ )
   {
      QString outfile = get_file_name(saxs_conc_names[i],"bsub");

      // compute Rg/Io
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
      double Io;
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
                        a,
                        b,
                        siga,
                        sigb,
                        chi2,
                        Rg,
                        Io,
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
      wave_Ios[saxs_conc_names[i]] = Io;
      wave_smins[saxs_conc_names[i]] = smin;
      wave_smaxs[saxs_conc_names[i]] = smax;
      wave_sRgmins[saxs_conc_names[i]] = sRgmin;
      wave_sRgmaxs[saxs_conc_names[i]] = sRgmax;
      wave_chi2s[saxs_conc_names[i]] = chi2;

      QString pngfile = QString("pngs%1%1_%1_saxs_guinier.png")
         .arg(QDir::separator())
         .arg(p_name)
         .arg(saxs_conc_names[i]);
      
      QString cmd = 
         QString("pnggnuplot.pl -p 1.5 -g -l points -c %1 %1 -m %1 %1 %1 %1 %1 %1\t\n")
         .arg(wave[guinier].q[beststart] * .2 )
         .arg(wave[guinier].q[bestend] * 1.2 )
         .arg(a)
         .arg(b)
         .arg(smin)
         .arg(smax)
         .arg(pngfile)
         .arg(guinier);
      cout << cmd.ascii();
      system(cmd.ascii());

      QString this_data_line = 
         QString("|| %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 || %1 ||\n")
         .arg(saxs_conc_names[i])
         .arg(wave_types[saxs_conc_names[i]])
         .arg(wave_concs[saxs_conc_names[i]] ? QString("%1").arg(wave_concs[saxs_conc_names[i]]) : "buffer" )
         .arg(wave_concs[saxs_conc_names[i]] ? QString("%1").arg(wave_alphas[saxs_conc_names[i]]) : "" )
         .arg(QString("%1").arg(outfile).replace(QRegExp("^cwave."),""))
         .arg(wave_comments[saxs_conc_names[i]])
         .arg(Rg)
         .arg(Io)
         .arg(Io/wave_concs[saxs_conc_names[i]])
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
                 "|| name || saxs or waxs || conc mg/ml || alpha || source file || comments || Rg || Io || Io/conc || qRg min || qRg max || q min || q max || chi2 || start point || end point || points used ||\n"
                 "%1"
                 "[[Image(htdocs:%1)]]\n"
                 )
         .arg(saxs_conc_names[i])
         .arg(this_data_line)
         .arg(pngfile)
         ;
      guinier_summary += this_data_line;
   }
   result += guinier_summary;
   return true;
}
