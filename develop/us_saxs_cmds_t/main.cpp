#include "../include/us_cmdline_t.h"
#include <qregexp.h>
#include <map>
#include <vector>
#include <list>

#include <stdio.h>


int main (int argc, char **argv)
{
   if (argc < 2) 
   {
      printf(
             "usage: %s command params\n"
             "Valid commands \tparams:\n"
             "saxs_avg       \toutfile infile1 infile2 {infile3 etc}"
             "\tAverage curves\n"
             "saxs_sbg       \toutfile solutionfile bufferfile alpha"
             "\tSubtract background wave\n"
             "saxs_scw       \toutfile solutionfile bufferfile emptycellfile alpha"
             "\tSubtract background and cell wave\n"
             "saxs_wgsbs     \toutfile solutionfile bufferfile waxsfile alpha beta const low high"
             "\tWAXS guided SAXS buffer subtraction\n"
             "saxs_wgsbsg    \toutfile logfile solutionfile bufferfile waxsfile alphamin alphamax alphainc betamin betamax betainc constmin constmax constinc low high\n"
             "\tWAXS guided SAXS buffer subtraction grid search\n"
             "saxs_wgsbsgsm  \toutfile logfile solutionfile bufferfile waxsfile gsm_type max_iterations alphainit alphamin alphamax alphainc betainit betamin betamax betainc constinit constmin constmax constinc low high\n"
             "\tWAXS guided SAXS buffer subtraction gsm search, bounded by *min,*man, starting at *init, deltas *inc\n"
             "saxs_wgsbsggsm \toutfile logfile solutionfile bufferfile waxsfile gsm_type max_iterations gsmpercent alphamin alphamax alphaincg alphaincgsm betamin betamax betaincg betaincgsm constmin constmax constincg constincgsm low high\n"
             "\tWAXS guided SAXS buffer subtraction grid then gsm search, bounded by *min,*man, deltas *incg (grid) *incgsm (gsm), gsmpercent is the % range delta for gsm search from min\n"
             "saxs_wgsbsnggsm \toutfile logfile solutionfile bufferfile waxsfile grids gsm_type max_iterations gsmpercent alphamin alphamax alphaincg alphaincgsm betamin betamax betaincg betaincgsm constmin constmax constincg constincgsm low high\n"
             "\tWAXS guided SAXS buffer subtraction n grids then gsm search, bounded by *min,*man, deltas *incg (grid) *incgsm (gsm), gsmpercent is the % range delta for subsequent grid searches and then gsm search from min\n"
             "saxs_join       \toutfile infile1 infile2 q-value\n"
             "\tjoin two files\n"
             "saxs_guinier_plot  \toutfile infile\n"
             "\treplace q with q*q, r with log(r), don't plot in logscale\n"
             "saxs_guinier_fit   \tlogfile infile startpos endpos minlen maxlen\n"
             "\tcompute guinier fit\n"
             "saxs_guinier_fit2  \tlogfile infile pointsmin pointsmax sRgmaxlimit pointweightpower\n"
             "\tcompute guinier fit\n"
             , argv[0]
             );
      exit(-1);
   }
   vector < QString > cmds;
   for (int i = 1; i < argc; i++) 
   {
      cmds.push_back(argv[i]);
   }
   if (cmds[0].lower() == "saxs_avg") 
   {
      if (cmds.size() < 4) 
      {
         printf(
                "usage: %s %s outfile infile1 infile2 {infile3 etc}\n"
                , argv[0]
                , argv[1]
                );
         exit(-1001);
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
            exit(-1002);
         }
      }


      if ( !usu.avg(outfile, tags) ||
           !usu.write(outfile, outfile) )
      {
         cout << usu.errormsg << endl;
         exit(-1003);
      }

      exit(0);
   }

   if (cmds[0].lower() == "saxs_sbg") 
   {
      if (cmds.size() != 5) 
      {
         printf(
                "usage: %s %s outfile solutionfile bufferfile alpha\n"
                , argv[0]
                , argv[1]
                );
         exit(-1101);
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
         exit(-1102);
      }

      exit(0);
   }

   if (cmds[0].lower() == "saxs_scw") 
   {
      if (cmds.size() != 6) 
      {
         printf(
                "usage: %s %s outfile solutionfile bufferfile emptycellfile alpha\n"
                , argv[0]
                , argv[1]
                );
         exit(-1201);
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
         exit(-1202);
      }

      exit(0);
   }

   if (cmds[0].lower() == "saxs_wgsbs") 
   {
      if (cmds.size() != 10) 
      {
         printf(
                "usage: %s %s outfile solutionfile bufferfile waxsfile alpha beta const low high\n"
                , argv[0]
                , argv[1]
                );
         exit(-1301);
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
         exit(-1302);
      }
      cout << "nrmsd " << nrmsd << endl;
      exit(0);
   }

   if (cmds[0].lower() == "saxs_wgsbsg") 
   {
      if (cmds.size() != 17) 
      {
         printf(
                "usage: %s %s outfile logfile solutionfile bufferfile waxsfile alphamin alphamax alphainc betamin betamax betainc constmin constmax constinc low high"
                , argv[0]
                , argv[1]
                );
         exit(-1401);
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
         exit(-1402);
      }

      cout << "nrmsd " << nrmsd << endl;

      if ( logfile == "-" )
      {
         cout << log;
      } else {
         QFile f(logfile);
         if ( f.open(IO_WriteOnly ) ) 
         {
            QTextStream ts(&f);
            ts << log;
            f.close();
         } else {
            cout << "error creating " << logfile << endl;
            exit(-1403);
         }
      }
      exit(0);
   }

   if (cmds[0].lower() == "saxs_wgsbsgsm") 
   {
      if (cmds.size() != 22) 
      {
         printf(
                "usage: %s %s outfile logfile solutionfile bufferfile waxsfile gsm_type max_iterations alphainit alphamin alphamax alphainc betainit betamin betamax betainc constinit constmin constmax constinc low high\n"
                , argv[0]
                , argv[1]
                );
         exit(-1501);
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
      double alphamin;
      double betamin;
      double dconstmin;
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
         exit(-1502);
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
         exit(-1503);
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
         if ( f.open(IO_WriteOnly ) ) 
         {
            QTextStream ts(&f);
            ts << log;
            f.close();
         } else {
            cout << "error creating " << logfile << endl;
            exit(-1504);
         }
      }
      exit(0);
   }

   if (cmds[0].lower() == "saxs_wgsbsggsm") 
   {
      if (cmds.size() != 23) 
      {
         printf(
                "usage: %s %s outfile logfile solutionfile bufferfile waxsfile gsm_type max_iterations gsmpercent alphamin alphamax alphaincg alphaincgsm betamin betamax betaincg betaincgsm constmin constmax constincg constincgsm low high\n"
                , argv[0]
                , argv[1]
                );
         exit(-1601);
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
         exit(-1602);
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
         exit(-1602);
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
         exit(-1603);
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
         if ( f.open(IO_WriteOnly ) ) 
         {
            QTextStream ts(&f);
            ts << log;
            f.close();
         } else {
            cout << "error creating " << logfile << endl;
            exit(-1604);
         }
      }
      exit(0);
   }


   if (cmds[0].lower() == "saxs_wgsbsnggsm") 
   {
      if (cmds.size() != 24) 
      {
         printf(
                "usage: %s %s outfile logfile solutionfile bufferfile waxsfile grids gsm_type max_iterations gsmpercent alphamin alphamax alphaincg alphaincgsm betamin betamax betaincg betaincgsm constmin constmax constincg constincgsm low high\n"
                , argv[0]
                , argv[1]
                );
         exit(-1701);
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
         exit(-1702);
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
            exit(-1703);
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
            exit(-1704);
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
         exit(-1705);
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
         if ( f.open(IO_WriteOnly ) ) 
         {
            QTextStream ts(&f);
            ts << log;
            f.close();
         } else {
            cout << "error creating " << logfile << endl;
            exit(-1706);
         }
      }
      exit(0);
   }

   if (cmds[0].lower() == "saxs_join") 
   {
      if (cmds.size() != 5) 
      {
         printf(
                "usage: %s %s outfile infile1 infile2 q-value\n"
                , argv[0]
                , argv[1]
                );
         exit(-1801);
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
         exit(-1802);
      }

      exit(0);
   }

   if (cmds[0].lower() == "saxs_guinier_plot") 
   {
      if (cmds.size() != 3) 
      {
         printf(
                "usage: %s %s outfile infile\n"
                , argv[0]
                , argv[1]
                );
         exit(-1901);
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
         exit(-1902);
      }

      exit(0);
   }

   if (cmds[0].lower() == "saxs_guinier_fit") 
   {
      if (cmds.size() != 7) 
      {
         printf(
                "usage: %s %s logfile infile startpos endpos minlen maxlen\n"
                , argv[0]
                , argv[1]
                );
         exit(-2001);
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
         exit(-2002);
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
               exit(-2003);
            }
         }
      }
      
      QFile f(logfile);
      
      if ( logfile == "-" )
      {
         cout << log;
      } else {
         if ( f.open(IO_WriteOnly ) )
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

   if (cmds[0].lower() == "saxs_guinier_fit2") 
   {
      if (cmds.size() != 7) 
      {
         printf(
                "usage: %s %s logfile infile pointsmin pointsmax sRgmaxlimit pointweightpower\n"
                , argv[0]
                , argv[1]
                );
         exit(-2001);
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
         exit(-2002);
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

      if ( !usu.guinier_fit2(
                             log,
                             tag,
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
                             smin,
                             smax,
                             sRgmin,
                             sRgmax
                             ) )
      {
         cout << usu.errormsg << endl;
         exit(-2003);
      }
      
      QFile f(logfile);
      
      if ( logfile == "-" )
      {
         cout << log;
      } else {
         if ( f.open(IO_WriteOnly ) )
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

   printf("%s error: %s unknown command\n", argv[0], argv[1]);
   exit(-2);
}
