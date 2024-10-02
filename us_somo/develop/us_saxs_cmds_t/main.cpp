#include "../include/us_saxs_cmds_t.h"
#include "../include/us_cmdline_app.h"
#include "../include/us_revision.h"
#include "../include/us_multi_column.h"
#include "../include/us_pm.h"
#include "us_cuda.h"
//Added by qt3to4:
#include <QTextStream>

// globals to remove dependencies on libus
// a better job could be done
QString US_Version = REVISION;
// vector < QString > modelString;


// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}


int main (int argc, char **argv)
{
   // QStringList outfiles;
   // QStringList triangles;
   // vector < double > one_over_triangles;
   // QString inputbase = "1hel_s_f0_6";
   // outfiles
   //    << "1hel_s_f0_6_01774"
   //    << "1hel_s_f0_6_02012"
   //    << "1hel_s_f0_6_02457"
   //    ;
   // for ( int i = 0; i < (int) outfiles.size(); ++i )
   // {
   //    triangles << QString( outfiles[ i ] ).replace( QRegExp( QString( "^%1_" ).arg( inputbase ) ), "" ).replace( QRegExp( "^0*" ) , "" );
   //    one_over_triangles.push_back( triangles.back().toDouble() != 0e0 ?
   //                                  1e0 / triangles.back().toDouble() : -1e0 );
   //    us_qdebug( QString( "%1 %2\n" ).arg( triangles.back() ).arg( one_over_triangles.back() ) );
   // }

   // exit( 0 );

   if (argc < 2) 
   {
      printf(
             "usage: %s command params\n"
             "Valid commands \tparams:\n"
             "avg       \toutfile infile1 infile2 {infile3 etc}"
             "          \tAverage curves\n"
             "sbg       \toutfile solutionfile bufferfile alpha"
             "          \tSubtract background wave\n"
             "scw       \toutfile solutionfile bufferfile emptycellfile alpha"
             "          \tSubtract background and cell wave\n"
             "wgsbs     \toutfile solutionfile bufferfile waxsfile alpha beta const low high"
             "          \tWAXS guided SAXS buffer subtraction\n"
             "wgsbsg    \toutfile logfile solutionfile bufferfile waxsfile alphamin alphamax alphainc betamin betamax betainc constmin constmax constinc low high\n"
             "          \tWAXS guided SAXS buffer subtraction grid search\n"
             "wgsbsgsm  \toutfile logfile solutionfile bufferfile waxsfile gsm_type max_iterations alphainit alphamin alphamax alphainc betainit betamin betamax betainc constinit constmin constmax constinc low high\n"
             "          \tWAXS guided SAXS buffer subtraction gsm search, bounded by *min,*man, starting at *init, deltas *inc\n"
             "wgsbsggsm \toutfile logfile solutionfile bufferfile waxsfile gsm_type max_iterations gsmpercent alphamin alphamax alphaincg alphaincgsm betamin betamax betaincg betaincgsm constmin constmax constincg constincgsm low high\n"
             "          \tWAXS guided SAXS buffer subtraction grid then gsm search, bounded by *min,*man, deltas *incg (grid) *incgsm (gsm), gsmpercent is the pct range delta for gsm search from min\n"
             "wgsbsnggsm \toutfile logfile solutionfile bufferfile waxsfile grids gsm_type max_iterations gsmpercent alphamin alphamax alphaincg alphaincgsm betamin betamax betaincg betaincgsm constmin constmax constincg constincgsm low high\n"
             "           \tWAXS guided SAXS buffer subtraction n grids then gsm search, bounded by *min,*man, deltas *incg (grid) *incgsm (gsm), gsmpercent is the pct range delta for subsequent grid searches and then gsm search from min\n"
             "join       \toutfile infile1 infile2 q-value\n"
             "           \tjoin two files\n"
             "guinier_plot  \toutfile infile\n"
             "              \treplace q with q*q, r with log(r), don't plot in logscale\n"
             "guinier_fit   \tlogfile infile startpos endpos minlen maxlen\n"
             "              \tcompute guinier fit\n"
             "guinier_fit2  \tlogfile infile pointsmin pointsmax sRgmaxlimit pointweightpower\n"
             "              \tcompute guinier fit\n"
             "project_init  \tprojectname\n"
             "              \tcreates a new blank project directory & template files\n"
             "project_rebuild\n"
             "              \treads the project file and produces wiki pages & pngs describing the analysis\n"
             "project_merge \toutfile { gnom }\n"
             "              \treads project files and produces a Rg/Io summary wiki page and optionally produce gnom Dmax series\n"
             "project_1d    \twikiprefix pngsplits\n"
             "              \tbuilds wiki page and computes averages for files in 1d directory\n"
             "iqqshere      \toutfile radius delta_rho minq maxq deltaq\n"
             "              \tproduces a .dat file containing the I(q) vs q curve, min,max,deltaq in 1/angstrom\n"
             "iqqspherefit  \toutfile     infile\n"
             "              \tminradius   maxradius   deltaradius\n"
             "              \tmindeltarho maxdeltarho deltadeltrarho\n"
             "              \tminq        maxq\n"
             "              \tnormalize{y|n}\n"
             "              \tbuild a collection of iqqsphere's and compute best fit & nnls distribution\n"
             "              \toutputs datafiles: outfile-radius.txt, outfile-delta_rho.txt, outfile.txt\n"
             "merge         \toutfile     infile1     weight1     infile2     weight2\n"
             "              \tproduce a weighted merge of the two files\n"
             "iq            \tcontrolfile\n"
             "              \tcompute a saxs curve (can be a .tar)\n"
#if defined( CUDA )
             "cuda          \tcuda test\n"
#endif
             "sjoin         \toutfile infile1 infile2 indep1 dep1 indep2 dep2\n"
             "              \tfind peak, compute splines and join infile2 indep2 to infile1 and output\n"
             "smooth        \toutfile infile col points\n"
             "              \t( 2 * points + 1 ) smooths col of input file\n"
             "uniquify      \toutfile infile col sumcol\n"
             "              \tmake col unique and sum up somcol\n"
             "repeak        \toutfile infile col\n"
             "              \trescales col to have max 1.0\n"
             "sort          \toutfile infile col\n"
             "              \tsorts on col\n"
             "reverse       \toutfile infile\n"
             "              \treverses row order\n"
             "range         \toutfile infile col min max\n"
             "              \nselects rows whose 'col' has values between min and max inclusive\n"
             "scale         \toutfile infile col mult\n"
             "              \tmultiplies column 'col' by multiplier\n"
             "ijoin         \toutfile infile1 infile2 col1 col2 col2add\n"
             "              \tinterpolates data in infile2 col2 to grid of infile1 col1 and joins col2add to outfile\n"
             "clip          \toutfile infile col min max\n"
             "              \tremove rows whose 'col' has values less or equal to min or greater than or equal to max\n"
             "info          \tinfile1\n"
             "              \tprints summary info about file\n"
             "pmtest        \toutfile testfile\n"
             "              \ttest parsimonious models write model to outfile\n"
             "pm            \tcontrolfile\n"
             "              \tperform pm (controlfile can be a .tar or .tgz\n"
             "fasta         \tmax_line_length outfile pdb1 {pdb2 ... }\n"
             "              \tconvert listed .pdb's to a fasta formatted outputfile\n"
             "json          \tjson\n"
             "              \tprocess commands from json provided as command line argument or as a json file\n"
             "              \tinternal json commands support: align, pmrun, hydro, pat, ssbond as top level key\n"
             , argv[0]
             );
      exit(-1);
   }
   vector < QString > cmds;
   for (int i = 1; i < argc; i++) 
   {
      cmds.push_back(argv[i]);
   }
   int errorbase = -1000;

   if (cmds[0].toLower() == "avg") 
   {
      if (cmds.size() < 4) 
      {
         printf(
                "usage: %s %s outfile infile1 infile2 {infile3 etc}\n"
                , argv[0]
                , argv[1]
                );
         exit(errorbase);
      }

      QString outfile = cmds[1];
      vector < QString > tags;

      US_Saxs_Util usu;

      for ( unsigned int i = 2; i < cmds.size(); i++ )
      {
         tags.push_back(cmds[i]);
         if ( !usu.read(cmds[i], cmds[i]) )
         {
            cout << usu.errormsg << endl;
            exit(errorbase - 1);
         }
      }


      if ( !usu.avg(outfile, tags) ||
           !usu.write(outfile, outfile) )
      {
         cout << usu.errormsg << endl;
         exit(errorbase - 2);
      }

      exit(0);
   }

   errorbase -= 1000;

   if (cmds[0].toLower() == "sbg") 
   {
      if (cmds.size() != 5) 
      {
         printf(
                "usage: %s %s outfile solutionfile bufferfile alpha\n"
                , argv[0]
                , argv[1]
                );
         exit(errorbase);
      }

      QString outfile = cmds[1];
      QString solutionfile = cmds[2];
      QString bufferfile = cmds[3];
      double alpha = cmds[4].toDouble();
      outfile.replace(QRegExp("\\.dat$"), "");
      outfile += "a" + QString("%1").arg(alpha).replace(QRegExp("\\."),"") + ".dat";

      US_Saxs_Util usu;
      if ( !usu.read(solutionfile, "sol") ||
           !usu.read(bufferfile, "buf") ||
           !usu.subbackground("out", "sol", "buf", alpha) ||
           !usu.write(outfile, "out") )
      {
         cout << usu.errormsg << endl;
         exit(errorbase - 1);
      }

      exit(0);
   }

   errorbase -= 1000;

   if (cmds[0].toLower() == "scw") 
   {
      if (cmds.size() != 6) 
      {
         printf(
                "usage: %s %s outfile solutionfile bufferfile emptycellfile alpha\n"
                , argv[0]
                , argv[1]
                );
         exit(errorbase);
      }

      QString outfile = cmds[1];
      QString solutionfile = cmds[2];
      QString bufferfile = cmds[3];
      QString emptycellfile = cmds[4];
      double alpha = cmds[5].toDouble();
      outfile.replace(QRegExp("\\.dat$"), "");
      outfile += "a" + QString("%1").arg(alpha).replace(QRegExp("\\."),"") + ".dat";

      US_Saxs_Util usu;
      if ( !usu.read(solutionfile, "sol") ||
           !usu.read(bufferfile, "buf") ||
           !usu.read(emptycellfile, "empty") ||
           !usu.subcellwave("out", "sol", "buf", "empty", alpha) ||
           !usu.write(outfile, "out") )
      {
         cout << usu.errormsg << endl;
         exit(errorbase - 1);
      }

      exit(0);
   }

   errorbase -= 1000;

   if (cmds[0].toLower() == "wgsbs") 
   {
      if (cmds.size() != 10) 
      {
         printf(
                "usage: %s %s outfile solutionfile bufferfile waxsfile alpha beta const low high\n"
                , argv[0]
                , argv[1]
                );
         exit(errorbase);
      }

      QString outfile = cmds[1];
      QString solutionfile = cmds[2];
      QString bufferfile = cmds[3];
      QString waxsfile = cmds[4];
      double alpha = cmds[5].toDouble();
      double beta = cmds[6].toDouble();
      double dconst = cmds[7].toDouble();
      double low = cmds[8].toDouble();
      double high = cmds[9].toDouble();
      outfile.replace(QRegExp("\\.dat$"), "");
      outfile += 
         "a" + QString("%1").arg(alpha).replace(QRegExp("\\."),"") +
         "b" + QString("%1").arg(beta).replace(QRegExp("\\."),"") +
         "c" + QString("%1").arg(dconst).replace(QRegExp("\\."),"") +
         ".dat";

      US_Saxs_Util usu;
      // usu.debug = 1;
      double nrmsd;
      QString waxscrop;
      QString waxscropinterp;
      if ( !usu.read(solutionfile, "sol") ||
           !usu.read(bufferfile, "buf") ||
           !usu.read(waxsfile, "waxs") ||
           !usu.waxsguidedsaxsbuffersub(nrmsd, 
                                        "out", 
                                        "sol", 
                                        "buf", 
                                        "waxs",
                                        alpha,
                                        beta,
                                        dconst,
                                        low,
                                        high,
                                        waxscrop,
                                        waxscropinterp) ||
           !usu.write(outfile, "out") )
      {
         cout << usu.errormsg << endl;
         exit(errorbase - 1);
      }
      cout << "nrmsd " << nrmsd << endl;
      exit(0);
   }

   errorbase -= 1000;

   if (cmds[0].toLower() == "wgsbsg") 
   {
      if (cmds.size() != 17) 
      {
         printf(
                "usage: %s %s outfile logfile solutionfile bufferfile waxsfile alphamin alphamax alphainc betamin betamax betainc constmin constmax constinc low high"
                , argv[0]
                , argv[1]
                );
         exit(errorbase);
      }

      int p = 1;
      QString outfile = cmds[p++];
      QString logfile = cmds[p++];
      QString solutionfile = cmds[p++];
      QString bufferfile = cmds[p++];
      QString waxsfile = cmds[p++];

      double alphalow = cmds[p++].toDouble();
      double alphahigh = cmds[p++].toDouble();
      double alphainc = cmds[p++].toDouble();
      double betalow = cmds[p++].toDouble();
      double betahigh = cmds[p++].toDouble();
      double betainc = cmds[p++].toDouble();
      double dconstlow = cmds[p++].toDouble();
      double dconsthigh = cmds[p++].toDouble();
      double dconstinc = cmds[p++].toDouble();
      double low = cmds[p++].toDouble();
      double high = cmds[p++].toDouble();

      US_Saxs_Util usu;
      usu.debug = 0;
      double nrmsd;
      double alphamin;
      double betamin;
      double dconstmin;
      QString log;

      if ( !usu.read(solutionfile, "sol") ||
           !usu.read(bufferfile, "buf") ||
           !usu.read(waxsfile, "waxs") ||
           !usu.waxsguidedsaxsbuffersubgridsearch(nrmsd, 
                                                  alphamin,
                                                  betamin,
                                                  dconstmin,
                                                  log,
                                                  "out", 
                                                  "sol", 
                                                  "buf", 
                                                  "waxs",
                                                  alphalow,
                                                  alphahigh,
                                                  alphainc,
                                                  betalow,
                                                  betahigh,
                                                  betainc,
                                                  dconstlow,
                                                  dconsthigh,
                                                  dconstinc,
                                                  low,
                                                  high) ||
           !usu.write(outfile, "out") )
      {
         cout << usu.errormsg << endl;
         exit(errorbase - 1);
      }

      cout << "nrmsd " << nrmsd << endl;

      if ( logfile == "-" )
      {
         cout << log;
      } else {
         QFile f(logfile);
         if ( f.open(QIODevice::WriteOnly ) ) 
         {
            QTextStream ts(&f);
            ts << log;
            f.close();
         } else {
            cout << "error creating " << logfile << endl;
            exit(errorbase - 2);
         }
      }
      exit(0);
   }

   errorbase -= 1000;

   if (cmds[0].toLower() == "wgsbsgsm") 
   {
      if (cmds.size() != 22) 
      {
         printf(
                "usage: %s %s outfile logfile solutionfile bufferfile waxsfile gsm_type max_iterations alphainit alphamin alphamax alphainc betainit betamin betamax betainc constinit constmin constmax constinc low high\n"
                , argv[0]
                , argv[1]
                );
         exit(errorbase);
      }

      int p = 1;
      QString outfile = cmds[p++];
      QString logfile = cmds[p++];
      QString solutionfile = cmds[p++];
      QString bufferfile = cmds[p++];
      QString waxsfile = cmds[p++];

      int gsm_type = cmds[p++].toInt();
      long max_iterations = cmds[p++].toLong();

      double alphainit = cmds[p++].toDouble();
      double alphalow = cmds[p++].toDouble();
      double alphahigh = cmds[p++].toDouble();
      double alphainc = cmds[p++].toDouble();
      double betainit = cmds[p++].toDouble();
      double betalow = cmds[p++].toDouble();
      double betahigh = cmds[p++].toDouble();
      double betainc = cmds[p++].toDouble();
      double dconstinit = cmds[p++].toDouble();
      double dconstlow = cmds[p++].toDouble();
      double dconsthigh = cmds[p++].toDouble();
      double dconstinc = cmds[p++].toDouble();
      double low = cmds[p++].toDouble();
      double high = cmds[p++].toDouble();

      US_Saxs_Util usu;
      usu.debug = 0;
      double nrmsd;
      // double alphamin;
      // double betamin;
      // double dconstmin;
      QString log;

      if ( !usu.read(solutionfile, "sol") ||
           !usu.read(bufferfile, "buf") ||
           !usu.read(waxsfile, "waxs") ||
           !usu.setup_wgsbs_gsm_f_df(
                                     "out", 
                                     "sol", 
                                     "buf", 
                                     "waxs",
                                     alphalow,
                                     alphahigh,
                                     alphainc,
                                     betalow,
                                     betahigh,
                                     betainc,
                                     dconstlow,
                                     dconsthigh,
                                     dconstinc,
                                     low,
                                     high) ||
           !usu.wgsbs_gsm(nrmsd,
                          alphainit,
                          betainit,
                          dconstinit,
                          gsm_type,
                          max_iterations) )
      {
         cout << usu.errormsg << endl;
         exit(errorbase - 1);
      }
      outfile.replace(QRegExp("\\.dat$"), "");
      outfile += 
         "a" + QString("%1").arg(alphainit).replace(QRegExp("\\."),"") +
         "b" + QString("%1").arg(betainit).replace(QRegExp("\\."),"") +
         "c" + QString("%1").arg(dconstinit).replace(QRegExp("\\."),"") +
         ".dat";

      if ( !usu.write(outfile, "out") )
      {
         cout << usu.errormsg << endl;
         exit(errorbase - 2);
      }
      cout << "nrmsd " << nrmsd << endl;
      log += 
         QString("minimum %1|%2|%3|%4|%5|%6\n")
         .arg(nrmsd)
         .arg(alphainit)
         .arg(betainit)
         .arg(dconstinit)
         .arg(low)
         .arg(high);

      if ( logfile == "-" )
      {
         cout << log;
      } else {
         QFile f(logfile);
         if ( f.open(QIODevice::WriteOnly ) ) 
         {
            QTextStream ts(&f);
            ts << log;
            f.close();
         } else {
            cout << "error creating " << logfile << endl;
            exit(errorbase - 3);
         }
      }
      exit(0);
   }

   errorbase -= 1000;

   if (cmds[0].toLower() == "wgsbsggsm") 
   {
      if (cmds.size() != 23) 
      {
         printf(
                "usage: %s %s outfile logfile solutionfile bufferfile waxsfile gsm_type max_iterations gsmpercent alphamin alphamax alphaincg alphaincgsm betamin betamax betaincg betaincgsm constmin constmax constincg constincgsm low high\n"
                , argv[0]
                , argv[1]
                );
         exit(errorbase);
      }

      int p = 1;
      QString outfile = cmds[p++];
      QString logfile = cmds[p++];
      QString solutionfile = cmds[p++];
      QString bufferfile = cmds[p++];
      QString waxsfile = cmds[p++];

      int gsm_type = cmds[p++].toInt();
      long max_iterations = cmds[p++].toLong();
      double gsmpercent = cmds[p++].toDouble();

      double alphalow = cmds[p++].toDouble();
      double alphahigh = cmds[p++].toDouble();
      double alphaincg = cmds[p++].toDouble();
      double alphaincgsm = cmds[p++].toDouble();
      double betalow = cmds[p++].toDouble();
      double betahigh = cmds[p++].toDouble();
      double betaincg = cmds[p++].toDouble();
      double betaincgsm = cmds[p++].toDouble();
      double dconstlow = cmds[p++].toDouble();
      double dconsthigh = cmds[p++].toDouble();
      double dconstincg = cmds[p++].toDouble();
      double dconstincgsm = cmds[p++].toDouble();
      double low = cmds[p++].toDouble();
      double high = cmds[p++].toDouble();

      US_Saxs_Util usu;
      usu.debug = 0;
      double nrmsd;
      double alphamin;
      double betamin;
      double dconstmin;
      QString log;

      if ( !usu.read(solutionfile, "sol") ||
           !usu.read(bufferfile, "buf") ||
           !usu.read(waxsfile, "waxs") ||
           !usu.waxsguidedsaxsbuffersubgridsearch(nrmsd, 
                                                  alphamin,
                                                  betamin,
                                                  dconstmin,
                                                  log,
                                                  "out", 
                                                  "sol", 
                                                  "buf", 
                                                  "waxs",
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
         cout << usu.errormsg << endl;
         exit(errorbase - 1);
      }

      cout << 
         QString("minimum from grid nrmsd %1 alpha %2 beta %3 dconst %4\n")
         .arg(nrmsd)
         .arg(alphamin)
         .arg(betamin)
         .arg(dconstmin);

      alphalow = alphamin * ( 1e0 - gsmpercent / 100e0 );
      alphahigh = alphamin * ( 1e0 + gsmpercent / 100e0 );
      betalow = betamin * ( 1e0 - gsmpercent / 100e0 );
      betahigh = betamin * ( 1e0 + gsmpercent / 100e0 );
      dconstlow = dconstmin * ( 1e0 - gsmpercent / 100e0 );
      dconsthigh = dconstmin * ( 1e0 + gsmpercent / 100e0 );

      cout << 
         QString("ranges for gsm alpha %1:%2 beta %3:%4 dconst %5:%6\n")
         .arg(alphalow)
         .arg(alphahigh)
         .arg(betalow)
         .arg(betahigh)
         .arg(dconstlow)
         .arg(dconsthigh);

      if ( !usu.setup_wgsbs_gsm_f_df(
                                     "out", 
                                     "sol", 
                                     "buf", 
                                     "waxs",
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
           !usu.wgsbs_gsm(nrmsd,
                          alphamin,
                          betamin,
                          dconstmin,
                          gsm_type,
                          max_iterations) )
      {
         cout << usu.errormsg << endl;
         exit(errorbase - 1);
      }
      outfile.replace(QRegExp("\\.dat$"), "");
      outfile += 
         "a" + QString("%1").arg(alphamin).replace(QRegExp("\\."),"") +
         "b" + QString("%1").arg(betamin).replace(QRegExp("\\."),"") +
         "c" + QString("%1").arg(dconstmin).replace(QRegExp("\\."),"") +
         ".dat";

      if ( !usu.write(outfile, "out") )
      {
         cout << usu.errormsg << endl;
         exit(errorbase - 2);
      }
      cout << "nrmsd " << nrmsd << endl;
      cout << log;
      log += 
         QString("minimum %1|%2|%3|%4|%5|%6\n")
         .arg(nrmsd)
         .arg(alphamin)
         .arg(betamin)
         .arg(dconstmin)
         .arg(low)
         .arg(high);

      if ( logfile == "-" )
      {
         cout << log;
      } else {
         QFile f(logfile);
         if ( f.open(QIODevice::WriteOnly ) ) 
         {
            QTextStream ts(&f);
            ts << log;
            f.close();
         } else {
            cout << "error creating " << logfile << endl;
            exit(errorbase - 3);
         }
      }
      exit(0);
   }

   errorbase -= 1000;

   if (cmds[0].toLower() == "wgsbsnggsm") 
   {
      if (cmds.size() != 24) 
      {
         printf(
                "usage: %s %s outfile logfile solutionfile bufferfile waxsfile grids gsm_type max_iterations gsmpercent alphamin alphamax alphaincg alphaincgsm betamin betamax betaincg betaincgsm constmin constmax constincg constincgsm low high\n"
                , argv[0]
                , argv[1]
                );
         exit(errorbase);
      }

      int p = 1;
      QString outfile = cmds[p++];
      QString logfile = cmds[p++];
      QString solutionfile = cmds[p++];
      QString bufferfile = cmds[p++];
      QString waxsfile = cmds[p++];

      int grids = cmds[p++].toInt();
      int gsm_type = cmds[p++].toInt();
      long max_iterations = cmds[p++].toLong();
      double gsmpercent = cmds[p++].toDouble();

      double alphalow = cmds[p++].toDouble();
      double alphahigh = cmds[p++].toDouble();
      double alphaincg = cmds[p++].toDouble();
      double alphaincgsm = cmds[p++].toDouble();
      double betalow = cmds[p++].toDouble();
      double betahigh = cmds[p++].toDouble();
      double betaincg = cmds[p++].toDouble();
      double betaincgsm = cmds[p++].toDouble();
      double dconstlow = cmds[p++].toDouble();
      double dconsthigh = cmds[p++].toDouble();
      double dconstincg = cmds[p++].toDouble();
      double dconstincgsm = cmds[p++].toDouble();
      double low = cmds[p++].toDouble();
      double high = cmds[p++].toDouble();

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
      
      US_Saxs_Util usu;
      usu.debug = 0;
      double nrmsd;
      double alphamin;
      double betamin;
      double dconstmin;
      QString log;

      if ( !usu.read(solutionfile, "sol") ||
           !usu.read(bufferfile, "buf") ||
           !usu.read(waxsfile, "waxs") )

      {
         cout << usu.errormsg << endl;
         exit(errorbase - 1);
      }

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

         if ( !usu.waxsguidedsaxsbuffersubgridsearch(nrmsd, 
                                                     alphamin,
                                                     betamin,
                                                     dconstmin,
                                                     log,
                                                     "out", 
                                                     "sol", 
                                                     "buf", 
                                                     "waxs",
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
            cout << usu.errormsg << endl;
            exit(errorbase - 2);
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
         if ( !usu.setup_wgsbs_gsm_f_df(
                                        "out", 
                                        "sol", 
                                        "buf", 
                                        "waxs",
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
              !usu.wgsbs_gsm(nrmsd,
                             alphamin,
                             betamin,
                             dconstmin,
                             gsm_type,
                             max_iterations) )
         {
            cout << usu.errormsg << endl;
            exit(errorbase - 3);
         }
      }

      outfile.replace(QRegExp("\\.dat$"), "");
      outfile += 
         "a" + QString("%1").arg(alphamin).replace(QRegExp("\\."),"") +
         "b" + QString("%1").arg(betamin).replace(QRegExp("\\."),"") +
         "c" + QString("%1").arg(dconstmin).replace(QRegExp("\\."),"") +
         ".dat";

         
      if ( !usu.write(outfile, "out") )
      {
         cout << usu.errormsg << endl;
         exit(errorbase - 4);
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
         logbest += "*********** warning alpha pegged at limit ************\n";
      }
      if ( 
          betaincg && 
          ( betamin <= betalowlimit ||
            betamin >= betahighlimit ) )
      {
         logbest += "*********** warning beta pegged at limit ************\n";
      }
      if ( 
          dconstincg && 
          ( dconstmin <= dconstlowlimit ||
            dconstmin >= dconsthighlimit ) ) 
      {
         logbest += "*********** warning const pegged at limit ************\n";
      }
           
      cout << logbest;
      log += logbest;

      if ( logfile == "-" )
      {
         cout << log;
      } else {
         QFile f(logfile);
         if ( f.open(QIODevice::WriteOnly ) ) 
         {
            QTextStream ts(&f);
            ts << log;
            f.close();
         } else {
            cout << "error creating " << logfile << endl;
            exit(errorbase - 5);
         }
      }
      exit(0);
   }

   errorbase -= 1000;

   if (cmds[0].toLower() == "join") 
   {
      if (cmds.size() != 5) 
      {
         printf(
                "usage: %s %s outfile infile1 infile2 q-value\n"
                , argv[0]
                , argv[1]
                );
         exit(errorbase);
      }

      int p = 1;
      QString outfile = cmds[p++];
      QString infile1 = cmds[p++];
      QString infile2 = cmds[p++];
      double qvalue = cmds[p++].toDouble();

      US_Saxs_Util usu;


      if ( !usu.read(infile1, infile1) ||
           !usu.read(infile2, infile2) ||
           !usu.join(outfile, infile1, infile2, qvalue) ||
           !usu.write(outfile, outfile) )
      {
         cout << usu.errormsg << endl;
         exit(errorbase - 1);
      }

      exit(0);
   }

   errorbase -= 1000;

   if (cmds[0].toLower() == "guinier_plot") 
   {
      if (cmds.size() != 3) 
      {
         printf(
                "usage: %s %s outfile infile\n"
                , argv[0]
                , argv[1]
                );
         exit(errorbase);
      }

      int p = 1;
      QString outfile = cmds[p++];
      QString infile = cmds[p++];

      US_Saxs_Util usu;


      if ( !usu.read(infile, infile) ||
           !usu.guinier_plot(outfile, infile) ||
           !usu.write(outfile, outfile) )
      {
         cout << usu.errormsg << endl;
         exit(errorbase - 1);
      }

      exit(0);
   }

   errorbase -= 1000;

   if (cmds[0].toLower() == "guinier_fit") 
   {
      if (cmds.size() != 7) 
      {
         printf(
                "usage: %s %s logfile infile startpos endpos minlen maxlen\n"
                , argv[0]
                , argv[1]
                );
         exit(errorbase);
      }

      int p = 1;
      QString logfile = cmds[p++];
      QString infile = cmds[p++];
      unsigned int startpos = cmds[p++].toUInt();
      unsigned int endpos = cmds[p++].toUInt();
      unsigned int minlen = cmds[p++].toUInt();
      unsigned int maxlen = cmds[p++].toUInt();

      US_Saxs_Util usu;

      if ( !usu.read(infile, infile) ||
           !usu.guinier_plot("guinier", infile) )
      {
         cout << usu.errormsg << endl;
         exit(errorbase - 1);
      }

      QString log = "|| startpos || endpos || s_min || s_max || sRg_min || sRg_max || Rg || Io || a || b || sigma a || sigma b || chi2 ||\n";
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
      QString logg;

      for ( unsigned int pos = startpos; pos <= endpos; pos++ )
      {
         for ( unsigned int len = minlen; len <= maxlen; len++ )
         {
            if ( !usu.guinier_fit(
                                  log,
                                  "guinier",
                                  pos,
                                  pos + len,
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
               cout << usu.errormsg << endl;
               exit(errorbase - 2);
            }
         }
      }
      
      QFile f(logfile);
      
      if ( logfile == "-" )
      {
         cout << log;
      } else {
         if ( f.open(QIODevice::WriteOnly ) )
         {
            QTextStream ts(&f);
            ts << log;
            f.close();
         } else {
            cout << log;
            cerr << "Could not open logfile " << logfile << " for writing\n";
         }
      }
         
      exit(0);
   }

   errorbase -= 1000;

   if (cmds[0].toLower() == "guinier_fit2") 
   {
      if (cmds.size() != 7) 
      {
         printf(
                "usage: %s %s logfile infile pointsmin pointsmax sRgmaxlimit pointweightpower\n"
                , argv[0]
                , argv[1]
                );
         exit(errorbase);
      }

      int p = 1;
      QString logfile = cmds[p++];
      QString infile = cmds[p++];
      unsigned int pointsmin = cmds[p++].toUInt();
      unsigned int pointsmax = cmds[p++].toUInt();
      double sRgmaxlimit = cmds[p++].toDouble();
      double pointweightpower = cmds[p++].toDouble();
      QString tag = infile;
      tag.replace(QRegExp("\\.(dat|DAT)$"), "");
      QString outfile = QString("%1g.dat").arg(tag);
      US_Saxs_Util usu;

      if ( !usu.read(infile, infile) ||
           !usu.guinier_plot(tag, infile) ||
           !usu.write(outfile, tag) )
      {
         cout << usu.errormsg << endl;
         exit(errorbase - 1);
      }

      QString log = "|| startpos || endpos || s_min || s_max || sRg_min || sRg_max || Rg || Io || a || b || sigma a || sigma b || chi2 ||\n";
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
      QString logg;
      unsigned int beststart;
      unsigned int bestend;

      if ( !usu.guinier_fit2(
                             log,
                             tag,
                             pointsmin,
                             pointsmax,
                             sRgmaxlimit,
                             pointweightpower,
                             .05,
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
                             sRgmax,
                             beststart,
                             bestend
                             ) )
      {
         cout << usu.errormsg << endl;
         exit(errorbase - 2);
      }
      
      QFile f(logfile);
      
      if ( logfile == "-" )
      {
         cout << log;
      } else {
         if ( f.open(QIODevice::WriteOnly ) )
         {
            QTextStream ts(&f);
            ts << log;
            f.close();
         } else {
            cout << log;
            cerr << "Could not open logfile " << logfile << " for writing\n";
         }
      }
         
      exit(0);
   }

   errorbase -= 1000;

   if (cmds[0].toLower() == "project_init") 
   {
      if (cmds.size() != 2) 
      {
         printf(
                "usage: %s %s projectname\n"
                , argv[0]
                , argv[1]
                );
         exit(errorbase);
      }
      errorbase--;

      int p = 1;
      QString projectname = cmds[p++];

      QString projectdir = projectname;

      projectname.replace(QRegExp("^.*((\\|/)"),"");

      // make directory
      if ( 
          !projectdir.contains(QRegExp("^(\\|/)")) // doesn't start at root
          )
      {
         projectdir = QString("%1%2%3")
            .arg(QDir::currentPath())
            .arg(QDir::separator())
            .arg(projectdir);
      }

      QString pngdir = QString("%1%2pngs")
         .arg(projectdir)
         .arg(QDir::separator());

      QString sourcewavedir = QString("%1%2wave")
         .arg(projectdir)
         .arg(QDir::separator());

      QString computedwavedir = QString("%1%2cwave")
         .arg(projectdir)
         .arg(QDir::separator());

      QString tmpdir = QString("%1%2tmp")
         .arg(projectdir)
         .arg(QDir::separator());

      QString logdir = QString("%1%2log")
         .arg(projectdir)
         .arg(QDir::separator());

      cout << "project name:                     " << projectname << endl;
      cout << "project directory:                " << projectdir << endl;
      cout << "project pngs directory:           " << pngdir << endl;
      cout << "project source waves directory:   " << sourcewavedir << endl;
      cout << "project computed waves directory: " << computedwavedir << endl;
      cout << "project temporary directory:      " << tmpdir << endl;
      cout << "project log directory:            " << logdir << endl;

      QDir pd(projectdir);
      if ( pd.exists() )
      {
         cout << "project directory already exists, not created\n";
      } else {
         if ( !pd.mkdir(projectdir) )
         {
            cerr << "error: could not create directory: " << projectdir << endl;
            exit(errorbase);
         }
      }
      errorbase--;

      if ( !pd.setCurrent(projectdir) )
      {
         cerr << "error: " << projectdir << " could not change to directory\n";
         exit(errorbase);
      }
      errorbase--;

      QDir ppd(pngdir);
      if ( ppd.exists() )
      {
         cout << "project pngs directory already exists, not created\n";
      } else {
         if ( !ppd.mkdir(pngdir) )
         {
            cerr << "error: could not create directory: " << pngdir << endl;
            exit(errorbase);
         }
      }
      errorbase--;

      QDir pswd(sourcewavedir);
      if ( pswd.exists() )
      {
         cout << "project source wave directory already exists, not created\n";
      } else {
         if ( !pswd.mkdir(sourcewavedir) )
         {
            cerr << "error: could not create directory: " << sourcewavedir << endl;
            exit(errorbase);
         }
      }
      errorbase--;

      QDir pcwd(computedwavedir);
      if ( pcwd.exists() )
      {
         cout << "project computed wave directory already exists, not created\n";
      } else {
         if ( !pcwd.mkdir(computedwavedir) )
         {
            cerr << "error: could not create directory: " << computedwavedir << endl;
            exit(errorbase);
         }
      }
      errorbase--;

      QDir ptd(tmpdir);
      if ( ptd.exists() )
      {
         cout << "project temporary directory already exists, not created\n";
      } else {
         if ( !ptd.mkdir(tmpdir) )
         {
            cerr << "error: could not create directory: " << tmpdir << endl;
            exit(errorbase);
         }
      }
      errorbase--;

      QDir pld(logdir);
      if ( pld.exists() )
      {
         cout << "project log directory already exists, not created\n";
      } else {
         if ( !pld.mkdir(logdir) )
         {
            cerr << "error: could not create directory: " << logdir << endl;
            exit(errorbase);
         }
      }
      errorbase--;

      if ( !pd.setCurrent(projectdir) )
      {
         cerr << "error: " << projectdir << " could not change to directory\n";
         exit(errorbase);
      }
      errorbase--;
         
      QFile pf("project");
      if ( pf.exists() )
      {
         cerr << "error: project file already exists, not created\n";
         exit(errorbase);
      } 
      errorbase--;

      if ( !pf.open( QIODevice::WriteOnly ) )
      {
         cerr << "error: project file can not be created.\n";
         exit(errorbase);
      }
      errorbase--;

      QTextStream ts( &pf );
      ts << 
         QString(
                 "# notes:\n"
                 "# mw in daltons\n"
                 "# conc in mg/ml\n"
                 "\n"
                 "# wiki defines the base url for the wiki\n"
                 "wiki                wiki.bcf.uthscsa.edu/embargo3\n"
                 "# wikiPrefix will be prepended to every name\n"
                 "wikiPrefix          \n"
                 "name                %1\n"
                 "description         %2\n"
                 "shortDescription    \n"
                 "# any number of comment lines ok\n"
                 "comment             \n"
                 "mw                  \n"
                 "concMultiplier      7.43e-4\n"
                 "saxsLowQ            .006\n"
                 "saxsHighQ           .24\n"
                 "waxsLowQ            .1\n"
                 "waxsHighQ           2.4\n"
                 "waxsZLowQ           .8\n"
                 "waxsZHighQ          2.4\n"
                 "overlapLowQ         .10\n"
                 "overlapHighQ        .23\n"
                 "# guinierMaxQ default is .05\n"
                 "# guinierMaxQ          .05\n"
                 "# the following paramaters for waxs guided saxs background subtraction default [0.98:1.00]\n"
                 "# alphaMin            .98\n"
                 "# alphaMax            1.00\n"
                 "# search parameters, defaults 10, 50\n"
                 "# iterationsGrid      10\n"
                 "# iterationsGSM       50\n"
                 "# join point (default .1)\n"
                 "# joinQ               .1\n"
                 "# Dmax range is for gnom runs\n"
                 "# DmaxStart           \n"
                 "# DmaxEnd             \n"
                 "# DmaxInc             \n"

                 "# waveName starts a new concentration or buffer (i.e. zero concentration) entry\n"
                 "waveName            \n"
                 "# comments here will be associated with the wave\n"
                 "comment             \n"
                 "waveFileName        \n"
                 "# waveType is saxs or waxs\n"
                 "waveType            saxs\n"
                 "exposureTime        \n"
                 "waveConc            \n"
                 "\n"
                 "waveName            \n"
                 "waveFileName        \n"
                 "# waveType is saxs or waxs\n"
                 "waveType            saxs\n"
                 "exposureTime        \n"
                 "waveConc            \n"
                 "# waveBufferName must match a previously defined wave file name\n"
                 "# multiple waveBufferNames for a wave ok, they will be averaged\n"
                 "waveBufferName      \n"
                 "# waveEmptyName must match a previously defined wave file name\n"
                 "# multiple waveEmptyNames for a wave ok, they will be averaged\n"
                 "waveEmptyName       \n"
                 "# manually set waveAlpha, Beta, Const (Beta,Const for waxs guided saxs background subtraction)\n"
                 "# waveAlpha           \n" 
                 "# waveBeta            \n" 
                 "# waveConst           \n"
                 "# manually set AlphaStart,End,Steps for waxs sample data\n"
                 "# waveAlphaStart      \n"
                 "# waveAlphaEnd        \n"
                 "# waveAlphaIncs       \n"
                 )
         .arg(projectname)
         .arg(projectname);

      pf.close();
      cout << 
         QString("project %1 created\n"
                 "now put wave files in to %2/wave and\n"
                 "edit %3/project and run \"us_saxs_cmd_t project_rebuild\" in the %4 directory\n"
                 )
         .arg(projectdir)
         .arg(projectname)
         .arg(projectname)
         .arg(projectdir);

      exit(0);
   }

   errorbase -= 1000;

   if (cmds[0].toLower() == "project_rebuild") 
   {
      if (cmds.size() != 1) 
      {
         printf(
                "usage: %s %s\n"
                , argv[0]
                , argv[1]
                );
         exit(errorbase);
      }
      errorbase--;

      US_Saxs_Util usu;
      if ( !usu.read_project() ||
           !usu.build_wiki() )
      {
         cout << usu.errormsg << endl;
         exit(errorbase);
      }

      exit(0);
   }

   errorbase -= 1000;

   if (cmds[0].toLower() == "project_merge") 
   {
      if ( !( cmds.size() == 2 ||
              ( cmds.size() == 3 && cmds[2].toLower() == "gnom" ) ) )
      {
         printf(
                "usage: %s %s outfile { gnom }\n"
                , argv[0]
                , argv[1]
                );
         exit(errorbase);
      }
      errorbase--;

      int p = 1;
      QString outfile = cmds[p++];
      bool gnom_run = ( cmds.size() == 3 && cmds[2].toLower() == "gnom" );
         
      double reference_mw_multiplier;
      QFile f("merge");
      if ( !f.exists() )
      {
         cout << 
            "You must first create a file called 'merge' which contains the reference MW multiplier\n"
            "and a list of the directories to include\n";
         exit(errorbase);
      }
      errorbase--;
         
      if ( !f.open(QIODevice::ReadOnly) )
      {
         cout << 
            "Could not open file 'merge' for reading.  Check permissions\n";
         exit(errorbase);
      }
      errorbase--;

      QTextStream ts(&f);
      reference_mw_multiplier = ts.readLine().trimmed().toDouble();
      vector < QString > projects;
      while ( !ts.atEnd() )
      {
         projects.push_back(ts.readLine().trimmed());
      }
      f.close();

      US_Saxs_Util usu;
      if ( !usu.merge_projects(
                               outfile,
                               reference_mw_multiplier,
                               projects,
                               gnom_run
                               ) )
      {
         cout << usu.errormsg << endl;
         exit(errorbase);
      }

      exit(0);
   }

   errorbase -= 1000;

   if (cmds[0].toLower() == "project_1d") 
   {
      if (cmds.size() != 3) 
      {
         printf(
                "usage: %s %s wikiprefix pngsplits\n"
                , argv[0]
                , argv[1]
                );
         exit(errorbase);
      }
      errorbase--;

      int p = 1;
      QString wikitag = cmds[p++];
      unsigned int pngsplits = cmds[p++].toUInt();

      US_Saxs_Util usu;
      if ( !usu.project_1d(wikitag,pngsplits) )
      {
         cout << usu.errormsg << endl;
         exit(errorbase - 1);
      }
      exit(0);
   }
   errorbase -= 1000;

   if ( cmds[0].toLower() == "iqqsphere" ) 
   {
      if ( cmds.size() != 7 ) 
      {
         printf(
                "usage: %s %s outfile radius delta_rho minq maxq deltaq\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString outfile   = cmds[ p++ ];
      double  radius    = cmds[ p++ ].toDouble();
      double  delta_rho = cmds[ p++ ].toDouble();
      double  minq      = cmds[ p++ ].toDouble();
      double  maxq      = cmds[ p++ ].toDouble();
      double  deltaq    = cmds[ p++ ].toDouble();

      US_Saxs_Util usu;
      if ( !usu.iqq_sphere( outfile,
                            radius,
                            delta_rho,
                            minq,
                            maxq,
                            deltaq ) )
      {
         cout << usu.errormsg << endl;
         exit( errorbase - 1 );
      }
      if ( !usu.write( outfile, outfile ) )
      {
         cout << usu.errormsg << endl;
         exit( errorbase - 2 );
      }

      exit(0);
   }
   errorbase -= 1000;

   if ( cmds[0].toLower() == "iqqspherefit" ) 
   {
      if ( cmds.size() != 12 ) 
      {
         printf(
                "usage: %s %s outfile infile minradius maxradius deltaradius mindeltarho maxdeltarho deltadeltarho minq maxq normalize{y|n}\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString outfile         = cmds[ p++ ];
      QString infile          = cmds[ p++ ];
      double  min_radius      = cmds[ p++ ].toDouble();
      double  max_radius      = cmds[ p++ ].toDouble();
      double  delta_radius    = cmds[ p++ ].toDouble();
      double  min_delta_rho   = cmds[ p++ ].toDouble();
      double  max_delta_rho   = cmds[ p++ ].toDouble();
      double  delta_delta_rho = cmds[ p++ ].toDouble();
      double  min_q           = cmds[ p++ ].toDouble();
      double  max_q           = cmds[ p++ ].toDouble();
      bool    do_normalize    = cmds[ p++ ].contains( QRegExp("^(Y|y)") );

      cout << ( do_normalize ? "yes, normalize\n" : "no don't normalize\n" );

      US_Saxs_Util usu;
      if ( !usu.iqq_sphere_fit( outfile,
                                infile,
                                min_radius,
                                max_radius,
                                delta_radius,
                                min_delta_rho,
                                max_delta_rho,
                                delta_delta_rho,
                                min_q,
                                max_q,
                                do_normalize
                                ) )
      {
         cout << usu.errormsg << endl;
         exit( errorbase - 1 );
      }
      exit(0);
   }
   errorbase -= 1000;

   if ( cmds[0].toLower() == "merge" ) 
   {
      if ( cmds.size() != 6 ) 
      {
         printf(
                "usage: %s %s outfile infile1 weight1 infile2 weight2\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString outfile         = cmds[ p++ ];
      QString infile1         = cmds[ p++ ];
      double  weight1         = cmds[ p++ ].toDouble();
      QString infile2         = cmds[ p++ ];
      double  weight2         = cmds[ p++ ].toDouble();

      US_Saxs_Util usu;
      if ( !usu.read(infile1, infile1) ||
           !usu.read(infile2, infile2) ||
           !usu.merge( outfile,
                       infile1,
                       weight1,
                       infile2,
                       weight2
                       ) ||
           !usu.write( outfile, outfile ) )
      {
         cout << usu.errormsg << endl;
         exit( errorbase - 1 );
      }
      exit(0);
   }
   errorbase -= 1000;

   if ( cmds[0].toLower() == "iq" ) 
   {
      if ( cmds.size() != 2 ) 
      {
         printf(
                "usage: %s %s controlfile\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString controlfile     = cmds[ p++ ];

      US_Saxs_Util usu;
      if ( !usu.read_control( controlfile ) )
      {
         US_Log error_log( "errors" );
         error_log.log( usu.errormsg );
         cout << usu.errormsg << endl;
         cout << usu.noticemsg << endl;
         exit( errorbase - 1 );
      }
      if ( !usu.errormsg.isEmpty() ) {
         US_Log error_log( "errors" );
         error_log.log( usu.errormsg );
      }
      exit(0);
   }
   errorbase -= 1000;

#if defined( CUDA )
   if ( cmds[0].toLower() == "cuda" ) 
   {
     cout << "cuda test\n";
     cuda_hello_world();
     exit( 0 );
   }
   errorbase -= 1000;
#endif

   if ( cmds[0].toLower() == "sjoin" ) 
   {
      if ( cmds.size() != 8 ) 
      {
         printf(
                "usage: %s %s outfile infile1 infile2 indep1 dep1 indep2 dep2\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString      outfile         = cmds[ p++ ];
      QString      infile1         = cmds[ p++ ];
      QString      infile2         = cmds[ p++ ];
      unsigned int indep1          = cmds[ p++ ].toUInt();
      unsigned int dep1            = cmds[ p++ ].toUInt();
      unsigned int indep2          = cmds[ p++ ].toUInt();
      unsigned int dep2            = cmds[ p++ ].toUInt();

      US_Multi_Column mc1( infile1 );
      US_Multi_Column mc2( infile2 );
      if ( !mc1.read() )
      {
         cout << mc1.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;
      if ( !mc2.read() )
      {
         cout << mc2.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;
      cout << mc1.info();
      cout << mc2.info();

      US_Multi_Column mc1_asc;
      US_Multi_Column mc1_des;

      US_Multi_Column mc2_asc;
      US_Multi_Column mc2_des;

      if ( !mc1.split_on_peak( mc1_asc,
                               mc1_des,
                               indep1,
                               dep1 ) ||
           !mc2.split_on_peak( mc2_asc,
                               mc2_des,
                               indep2,
                               dep2 ) )
      {
         cout << mc1.errormsg << endl;
         cout << mc2.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;
         
      cout << mc1_asc.info();
      cout << mc1_des.info();
      cout << mc2_asc.info();
      cout << mc2_des.info();

      US_Multi_Column mc1_asc_mono;
      US_Multi_Column mc1_des_mono;

      US_Multi_Column mc2_asc_mono;
      US_Multi_Column mc2_des_mono;
           
      if ( !mc1_asc.monotonize( mc1_asc_mono, dep1 ) ||
           !mc1_des.monotonize( mc1_des_mono, dep1 ) ||
           !mc2_asc.monotonize( mc2_asc_mono, dep2 ) ||
           !mc2_des.monotonize( mc2_des_mono, dep2 ) )
      {
         cout << mc1_asc.errormsg << endl;
         cout << mc1_des.errormsg << endl;
         cout << mc2_asc.errormsg << endl;
         cout << mc2_des.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;

      cout << mc1_asc_mono.info();
      cout << mc1_des_mono.info();
      cout << mc2_asc_mono.info();
      cout << mc2_des_mono.info();

      US_Multi_Column mc1_mono_joined;
      US_Multi_Column mc2_mono_joined;

      if ( !mc1_mono_joined.join( mc1_asc_mono, mc1_des_mono, "mc1_mono_joined.txt" ) ||
           !mc2_mono_joined.join( mc2_asc_mono, mc2_des_mono, "mc2_mono_joined.txt" ) )
      {
         cout << mc1_mono_joined.errormsg << endl;
         cout << mc2_mono_joined.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;

      cout << mc1_mono_joined.info();
      cout << mc2_mono_joined.info();

//       US_Multi_Column result_spline_asc;
//       US_Multi_Column result_spline_des;
//       US_Multi_Column result_spline_join;

//       if ( 
//           !result_spline_asc.spline( mc1_asc,
//                                      mc2_asc_mono,
//                                      dep1,
//                                      indep2,
//                                      dep2,
//                                      "spline-asc.txt"
//                                      ) ||
//           !result_spline_des.spline( mc1_des,
//                                      mc2_des_mono,
//                                      dep1,
//                                      indep2,
//                                      dep2,
//                                      "spline-des.txt"
//                                      ) ||
//           !result_spline_join.join( result_spline_asc, result_spline_des, outfile + "-spline.txt" ) 
//           )
//       {
//          cout << result_spline_asc.errormsg << endl;
//          cout << result_spline_des.errormsg << endl;
//          cout << result_spline_join.errormsg << endl;
//          exit( errorbase );
//       }
//      errorbase--;

//       US_Multi_Column result_quadratic_asc;
//       US_Multi_Column result_quadratic_des;
//       US_Multi_Column result_quadratic_join;

//       if ( 
//           !result_quadratic_asc.quadratic( mc1_asc,
//                                            mc2_asc_mono,
//                                            dep1,
//                                            indep2,
//                                            dep2,
//                                            "quadratic-asc.txt"
//                                            ) ||
//           !result_quadratic_des.quadratic( mc1_des,
//                                            mc2_des_mono,
//                                            dep1,
//                                            indep2,
//                                            dep2,
//                                            "quadratic-des.txt"
//                                            ) ||
//           !result_quadratic_join.join( result_quadratic_asc, result_quadratic_des, outfile + "-quadratic.txt" ) 
//           )
//       {
//          cout << result_quadratic_asc.errormsg << endl;
//          cout << result_quadratic_des.errormsg << endl;
//          cout << result_quadratic_join.errormsg << endl;
//          exit( errorbase );
//       }
//       errorbase--;

      US_Multi_Column result_linear_asc;
      US_Multi_Column result_linear_des;
      US_Multi_Column result_linear_join;

      if ( 
          !mc1_asc.write( "", true ) ||
          !mc1_des.write( "", true ) ||
          !mc2_asc.write( "", true ) ||
          !mc2_des.write( "", true ) ||
          !mc1_asc_mono.write( "", true ) ||
          !mc1_des_mono.write( "", true ) ||
          !mc2_asc_mono.write( "", true ) ||
          !mc2_des_mono.write( "", true ) ||
          !mc1_mono_joined.write( "", true ) ||
          !mc2_mono_joined.write( "", true ) )
      {
         cout << mc1.errormsg << endl;
         cout << mc2.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;

      if ( 
          !result_linear_asc.linear( mc1_asc,
                                     mc2_asc_mono,
                                     dep1,
                                     indep2,
                                     dep2,
                                     "linear-asc.txt"
                                     ) ||
          !result_linear_des.linear( mc1_des,
                                     mc2_des_mono,
                                     dep1,
                                     indep2,
                                     dep2,
                                     "linear-des.txt"
                                     ) ||
          !result_linear_join.join( result_linear_asc, result_linear_des, outfile  + "-linear.txt" ) 
          )
      {
         cout << result_linear_asc.errormsg << endl;
         cout << result_linear_des.errormsg << endl;
         cout << result_linear_join.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;

      US_Multi_Column result_linear_join_csv = result_linear_join;
      result_linear_join_csv.filename.replace( QRegExp( "\\.txt$" ), ".csv" );

      if ( 
          // !result_spline_asc.write( "", true ) ||
          // !result_spline_des.write( "", true ) ||
          // !result_spline_join.write( "", true ) ||
          // !result_quadratic_asc.write( "", true ) ||
          // !result_quadratic_des.write( "", true ) ||
          // !result_quadratic_join.write( "", true ) ||
          !result_linear_asc     .write( "", true ) ||
          !result_linear_des     .write( "", true ) ||
          !result_linear_join    .write( "", true ) ||
          !result_linear_join_csv.write( "", true )
          )
      {
         cout << mc1.errormsg << endl;
         cout << mc2.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;
      exit( 0 );
   }
   errorbase -= 1000;

   if ( cmds[0].toLower() == "smooth" ) 
   {
      if ( cmds.size() != 5 ) 
      {
         printf(
                "usage: %s %s outfile infile col points\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString      outfile         = cmds[ p++ ];
      QString      infile          = cmds[ p++ ];
      unsigned int col             = cmds[ p++ ].toUInt();
      unsigned int points          = cmds[ p++ ].toUInt();

      US_Multi_Column mc( infile );
      if ( !mc.read  () ||
           !mc.smooth( col, points ) ||
           !mc.write ( outfile, true ) )
      {
         cout << mc.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;
      cout << mc.info();
      exit( 0 );
   }
   errorbase -= 1000;

   if ( cmds[0].toLower() == "repeak" ) 
   {
      if ( cmds.size() != 4 ) 
      {
         printf(
                "usage: %s %s outfile infile col\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString      outfile         = cmds[ p++ ];
      QString      infile          = cmds[ p++ ];
      unsigned int col             = cmds[ p++ ].toUInt();

      US_Multi_Column mc( infile );
      if ( !mc.read  () ||
           !mc.repeak( col ) ||
           !mc.write ( outfile, true ) )
      {
         cout << mc.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;
      cout << mc.info();
      exit( 0 );
   }
   errorbase -= 1000;

   if ( cmds[0].toLower() == "sort" ) 
   {
      if ( cmds.size() != 4 ) 
      {
         printf(
                "usage: %s %s outfile infile col\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString      outfile         = cmds[ p++ ];
      QString      infile          = cmds[ p++ ];
      unsigned int col             = cmds[ p++ ].toUInt();

      US_Multi_Column mc( infile );
      if ( !mc.read  () ||
           !mc.sort( col ) ||
           !mc.write ( outfile, true ) )
      {
         cout << mc.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;
      cout << mc.info();
      exit( 0 );
   }
   errorbase -= 1000;

   if ( cmds[0].toLower() == "reverse" ) 
   {
      if ( cmds.size() != 3 ) 
      {
         printf(
                "usage: %s %s outfile infile\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString      outfile         = cmds[ p++ ];
      QString      infile          = cmds[ p++ ];

      US_Multi_Column mc( infile );
      if ( !mc.read   () ||
           !mc.reverse() ||
           !mc.write  ( outfile, true ) )
      {
         cout << mc.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;
      cout << mc.info();
      exit( 0 );
   }
   errorbase -= 1000;

   if ( cmds[0].toLower() == "uniquify" ) 
   {
      if ( cmds.size() != 5 ) 
      {
         printf(
                "usage: %s %s outfile infile col sumcol\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString      outfile         = cmds[ p++ ];
      QString      infile          = cmds[ p++ ];
      unsigned int col             = cmds[ p++ ].toUInt();
      unsigned int sumcol          = cmds[ p++ ].toUInt();

      US_Multi_Column mc( infile );
      if ( !mc.read    () ||
           !mc.uniquify( col, sumcol ) ||
           !mc.write   ( outfile, true ) )
      {
         cout << mc.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;
      cout << mc.info();
      exit( 0 );
   }
   errorbase -= 1000;

   if ( cmds[0].toLower() == "range" ) 
   {
      if ( cmds.size() != 6 ) 
      {
         printf(
                "usage: %s %s outfile infile col min max\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString      outfile         = cmds[ p++ ];
      QString      infile          = cmds[ p++ ];
      unsigned int col             = cmds[ p++ ].toUInt();
      double       min             = cmds[ p++ ].toDouble();
      double       max             = cmds[ p++ ].toDouble();

      US_Multi_Column mc( infile );
      if ( !mc.read  () ||
           !mc.range( col, min, max ) ||
           !mc.write ( outfile, true ) )
      {
         cout << mc.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;
      cout << mc.info();
      exit( 0 );
   }
   errorbase -= 1000;

   if ( cmds[0].toLower() == "scale" ) 
   {
      if ( cmds.size() != 5 ) 
      {
         printf(
                "usage: %s %s outfile infile col mult\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString      outfile         = cmds[ p++ ];
      QString      infile          = cmds[ p++ ];
      unsigned int col             = cmds[ p++ ].toUInt();
      double       mult            = cmds[ p++ ].toDouble();

      US_Multi_Column mc( infile );
      if ( !mc.read  () ||
           !mc.scale ( col, mult ) ||
           !mc.write ( outfile, true ) )
      {
         cout << mc.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;
      cout << mc.info();
      exit( 0 );
   }
   errorbase -= 1000;

   if ( cmds[0].toLower() == "ijoin" ) 
   {
      if ( cmds.size() != 7 ) 
      {
         printf(
                "usage: %s %s outfile infile1 infile2 col1 col2 col2add\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString      outfile         = cmds[ p++ ];
      QString      infile1         = cmds[ p++ ];
      QString      infile2         = cmds[ p++ ];
      unsigned int col1            = cmds[ p++ ].toUInt();
      unsigned int col2            = cmds[ p++ ].toUInt();
      unsigned int col2add         = cmds[ p++ ].toUInt();

      US_Multi_Column mc1( infile1 );
      US_Multi_Column mc2( infile2 );

      if ( !mc1.read() )
      {
         cout << mc1.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;
      if ( !mc2.read() )
      {
         cout << mc2.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;

      cout << mc1.info();
      cout << mc2.info();

      US_Multi_Column mco;

      if ( !mco.ijoin( mc1, mc2, col1, col2, col2add ) ||
           !mco.write( outfile, true ) )
      {
         cout << mco.errormsg << endl;
         exit( errorbase );
      }

      errorbase--;
      cout << mco.info();
      exit( 0 );
   }
   errorbase -= 1000;

   if ( cmds[0].toLower() == "info" ) 
   {
      if ( cmds.size() != 2 ) 
      {
         printf(
                "usage: %s %s infile\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString      infile          = cmds[ p++ ];

      US_Multi_Column mc( infile );
      if ( !mc.read() )
      {
         cout << mc.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;
      cout << mc.info();
      cout << mc.cinfo();
      exit( 0 );
   }
   errorbase -= 1000;

   if ( cmds[0].toLower() == "clip" ) 
   {
      if ( cmds.size() != 6 ) 
      {
         printf(
                "usage: %s %s outfile infile col min max\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString      outfile         = cmds[ p++ ];
      QString      infile          = cmds[ p++ ];
      unsigned int col             = cmds[ p++ ].toUInt();
      double       min             = cmds[ p++ ].toDouble();
      double       max             = cmds[ p++ ].toDouble();

      US_Multi_Column mc( infile );
      if ( !mc.read  () ||
           !mc.clip  ( col, min, max ) ||
           !mc.write ( outfile, true ) )
      {
         cout << mc.errormsg << endl;
         exit( errorbase );
      }
      errorbase--;
      cout << mc.info();
      exit( 0 );
   }
   errorbase -= 1000;

   if ( cmds[0].toLower() == "pmtest" ) 
   {
      if ( cmds.size() != 3 ) 
      {
         printf(
                "usage: %s %s outfile infile\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString      outfile         = cmds[ p++ ];
      QString      infile          = cmds[ p++ ];

      cout << US_PM::test( infile, outfile ).toLatin1().data() << endl;
      
      exit( 0 );
   }
   errorbase -= 1000;

   if ( cmds[0].toLower() == "pm" ) 
   {
      if ( cmds.size() != 2 ) 
      {
         printf(
                "usage: %s %s controlfile\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString      controlfile         = cmds[ p++ ];

      US_Saxs_Util usu;
      if ( !usu.run_pm( controlfile ) )
      {
         cout << usu.errormsg << endl;
         exit( errorbase );
      }
      
      exit( 0 );
   }
   errorbase -= 1000;

   if ( cmds[0].toLower() == "fasta" ) 
   {
      if ( cmds.size() <= 3 ) 
      {
         printf(
                "usage: %s %s max_line_length outfile pdb1 {pdb2 ... }\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;

      int          max_line_length = cmds[ p++ ].toInt();
      QString      outfile         = cmds[ p++ ];

      QStringList files;

      while ( (int) cmds.size() > p )
      {
         files << cmds[ p++ ];
      }

      if ( !US_Saxs_Util::pdb2fasta( outfile, files, max_line_length ) )
      {
         cout << "Errors found" << endl;
         exit( errorbase );
      }
      
      exit( 0 );
   }
   errorbase -= 1000;

   if ( cmds[0].toLower() == "json" ) 
   {
      if ( cmds.size() != 2 ) 
      {
         printf(
                "{\"errors\":\"%s %s incorrect number of arguments\"}\n"
                , argv[0]
                , argv[1]
                );
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString      json         = cmds[ p++ ];
      if ( json.right(5) == ".json" ) {
         QString contents;
         QString error;
         if ( !US_File_Util::getcontents( json, contents, error ) ) {
            QTextStream( stderr ) << error << "\n";
            exit(-3);
         }
         json = contents;
      }
      US_Saxs_Util usu;
      cout << usu.run_json( json ).toLatin1().data() << endl;
      exit( 0 );
   }
   errorbase -= 1000;

   printf("%s error: %s unknown command\n", argv[0], argv[1]);
   exit(-2);
}
