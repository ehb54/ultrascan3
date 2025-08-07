//! \file us_solve_sim.cpp
#include "us_solve_sim.h"
#include "us_constants.h"
#include "us_math2.h"
#include "us_memory.h"
#include "us_settings.h"
#include "us_util.h"
//#include "us_gui_settings.h"

// Define level-conditioned debug print that includes thread/processor
#ifdef DbgLv
#undef DbgLv
//!< special definition of DbgLv(a) for threads
#define DbgLv(a) \
   if (dbg_level >= a) \
   qDebug() << "SS-w:" << thrnrank << ":"
#endif

// Define the default norm cutoff value
#define _NORM_CUTOFF_ 1.00


double zerothr = 0.020; //!< zero threshold OD value
double linethr = 0.050; //!< linear threshold OD value
double maxod = 1.50; //!< maximum OD value
double mfactor = 3.00; //!< peak multiplier value
double mfactex = 1.00; //!< peak multiplier - experiment
double minnzsc = 0.005; //!< minimum non-zero scale factor

// Create a Solve-Simulation object
US_SolveSim::US_SolveSim(QList<DataSet *> &data_sets, int thrnrank, bool signal_wanted) :
    QObject(), data_sets(data_sets), thrnrank(thrnrank), signal_wanted(signal_wanted) {
   abort = false; // Default: no abort
   dbg_level = 0; // Default: no debug prints
   dbg_timing = false; // Default: no debug timing prints
   banddthr = false; // Default: no bandform data_threshold

   // If band-forming, possibly read in threshold control values
   if (data_sets[ 0 ]->simparams.band_forming) {
      QString bfcname = US_Settings::appBaseDir() + "/etc/bandform.config";
      QFile bfcfile(bfcname);
      if (bfcfile.open(QIODevice::ReadOnly)) {
         QTextStream tsi(&bfcfile);
         // Read in, in order: zero-thresh, linear-thresh, max-od, min-nz-scale
         while (!tsi.atEnd()) {
            QString fline = tsi.readLine();
            int cmx = fline.indexOf("#") - 1;
            double val = cmx > 0 ? fline.left(cmx).simplified().toDouble() : 0.0;
            if (cmx <= 0)
               continue;
            else if (fline.contains("zero threshold"))
               zerothr = val;
            else if (fline.contains("linear threshold"))
               linethr = val;
            else if (fline.contains("maximum OD"))
               maxod = val;
            else if (fline.contains("minimum non-zero"))
               minnzsc = val;
            else if (fline.contains("sim multiply factor"))
               mfactor = val;
            else if (fline.contains("exp multiply factor"))
               mfactex = val;
         }
         bfcfile.close();

         banddthr = true;
      }

      if (thrnrank == 1)
         DbgLv(1) << "CR:zthr lthr mxod mnzc mfac mfex bthr" << zerothr << linethr << maxod << minnzsc << mfactor
                  << mfactex << banddthr;
   }
}

// Create a Simulation object
US_SolveSim::Simulation::Simulation() {
   variance = 0.0;
   xnormsq = 0.0;
   alpha = 0.0;
   variances.clear();
   ti_noise.clear();
   ri_noise.clear();
   solutes.clear();
   zsolutes.clear();
   dbg_level = 0;
   dbg_timing = false;
   noisflag = 0;
}

// Static function to check the grid size implied by data and model
bool US_SolveSim::checkGridSize(QList<DataSet *> &data_sets, double s_max, QString &smsg) {
   const long tstep_max = 600000L; // Grid time steps maximum
      //  (e.g., handles s=100 for rpm=60000)
   bool too_large = false;
   double s_show = s_max * 1.0e13;
   smsg = QString("");

   for (int dd = 0; dd < data_sets.size(); dd++) {
      US_SimulationParameters *sparams = &data_sets[ dd ]->simparams;
      QVector<US_SimulationParameters::SpeedProfile> speed_step = sparams->speed_step;
      double meniscus = sparams->meniscus;
      double bottom = sparams->bottom;
      int rpm_max = sparams->speed_step[ 0 ].rotorspeed;
      int time_max = 0;
      int rsimpts = sparams->simpoints - 1;

      for (int ss = 0; ss < speed_step.size(); ss++) {
         int duration = qRound(speed_step[ ss ].duration_hours * 3600.0 + speed_step[ ss ].duration_minutes * 60.0);
         time_max = qMax(time_max, duration);
         rpm_max = qMax(rpm_max, speed_step[ ss ].rotorspeed);
      }

      double omega_s = sq(rpm_max * M_PI / 30.0);
      double lgbmrat = log(bottom / meniscus);
      double somgfac = s_max * omega_s;
      double dt = lgbmrat / (somgfac * rsimpts);
      long tsteps = ( long ) (time_max / dt) + 1L;
      //if(thrnrank==1) DbgLv(1) << "CK: tsteps" << tsteps << "tstep_max" << tstep_max << "dt omg rpm"
      // << dt << omega_s << rpm_max << "bot men rat sfac spts tmax" << bottom << meniscus << lgbmrat
      // << somgfac << rsimpts << time_max;

      if (tsteps > tstep_max) {
         qDebug() << "CK: tsteps" << tsteps << "dt omg rpm" << dt << omega_s << rpm_max << "bot men rat sfac spts tmax"
                  << bottom << meniscus << lgbmrat << somgfac << rsimpts << time_max;
         too_large = true;
         smsg += tr("The combination of rotor speed, the ratio of sedimentation\n"
                    "over diffusion coefficient, and length of experiment\n"
                    "caused the program to exceed available memory.\n"
                    "Please sufficiently reduce any of these to bring\n"
                    "the program into a feasible range.\n\n"
                    "Related data and simulation parameters include:\n"
                    "   Maximum speed = %1 RPM ;\n"
                    "   Maximum S value = %2 x 1e-13 ;\n"
                    "   Computed grid time steps and radius points  = %3 ;\n"
                    "   Program-imposed grid time steps upper limit = %4 .\n")
                    .arg(rpm_max)
                    .arg(s_show)
                    .arg(tsteps)
                    .arg(tstep_max);
      }
   }

   return too_large;
}

// Check the grid size implied by data and model (class method)
bool US_SolveSim::check_grid_size(double s_max, QString &smsg) {
   return checkGridSize(data_sets, s_max, smsg);
}

// Do the real work of a 2dsa/ga thread/processor:  simulation from solutes set
void US_SolveSim::calc_residuals(
   int offset, int dataset_count, Simulation &sim_vals, bool padAB, QVector<double> *ASave, QVector<double> *BSave,
   QVector<double> *NSave) {
   QVector<double> sv_nnls_a;
   QVector<double> sv_nnls_b;

   dbg_level = sim_vals.dbg_level; // Debug level
   dbg_timing = sim_vals.dbg_timing; // Debug-timing flag
   noisflag = sim_vals.noisflag; // Noise-calculation flag
   int nsolutes = sim_vals.solutes.size(); // Input solutes count
   int nzsol = sim_vals.zsolutes.size(); // Input zsolutes count
   calc_ti = ((noisflag & 1) != 0); // Calculate-TI flag
   calc_ri = ((noisflag & 2) != 0); // Calculate-RI flag
   startCalc = QDateTime::currentDateTime(); // Start calc time for timings
   int ntotal = 0; // Total points count
   int ntinois = 0; // TI noise value count
   int nrinois = 0; // RI noise value count
   double alphad = sim_vals.alpha; // Alpha for diagonal
   int lim_offs = offset + dataset_count; // Offset limit
   d_offs = offset; // Initial data offset
   bool use_zsol = (nzsol > 0); // Flag use ZSolutes
   nsolutes = use_zsol ? nzsol : nsolutes; // Count of used solutes
   int npoints = data_sets[ 0 ]->run_data.pointCount();
   int nscans = data_sets[ 0 ]->run_data.scanCount();
   double norm_cut = _NORM_CUTOFF_; // Default norm_cutoff value

   // If debug text modifies norm_cut factor, apply it
   QStringList dbgtxt = US_Settings::debug_text();
   if (thrnrank < 2)
      DbgLv(1) << "CR: NCUTOFF dbgtxt count" << dbgtxt.count();

   for (int ii = 0; ii < dbgtxt.count(); ii++) { // If debug text modifies norm_cutoff, apply it
      if (dbgtxt[ ii ].startsWith("normCutoff="))
         norm_cut = QString(dbgtxt[ ii ]).section("=", 1, 1).toDouble();
      if (thrnrank < 2)
         DbgLv(1) << "CR:   NORMCUT  ii" << ii << "dbgtii" << dbgtxt[ ii ] << "norm_cut" << norm_cut;
   }
   if (thrnrank < 2)
      DbgLv(1) << "CR: NORMCUT=" << norm_cut;

   //   double norm_cs   = norm_cut;

#if 0
#ifdef NO_DB
US_Settings::set_us_debug( dbg_level );
#endif
#endif
   //-----------------------------------------------------------
   if (banddthr) {
      mfactor = data_sets[ 0 ]->simparams.cp_width;
      if (mfactor < 0.0) {
         mfactor = 1.0;
         zerothr = 0.0;
         linethr = 0.0;
         maxod = 1.0e+20;
      }
      if (thrnrank < 2)
         DbgLv(1) << "CR:zthr lthr mxod mfac" << zerothr << linethr << maxod << mfactor;
   }
   //----------------------------------------------------------

   US_DataIO::EditedData wdata;

   for (int ee = offset; ee < lim_offs; ee++) { // Count scan,point totals for all data sets
      DataSet *dset = data_sets[ ee ];
      nscans = dset->run_data.scanCount();
      npoints = dset->run_data.pointCount();
      ntotal += (nscans * npoints);
      ntinois += npoints;
      nrinois += nscans;
   }
   //----------------------------------------------------------
   bool tikreg = (sim_vals.alpha != 0.0);
   int narows = tikreg ? (ntotal + nsolutes) : ntotal;

   // Set up and clear work vectors
   int navals = narows * nsolutes; // Size of "A" matrix
   DbgLv(1) << "   CR:na nb nx" << navals << narows << nsolutes << "  nt ns" << ntotal << nsolutes;

   QVector<double> nnls_a(navals, 0.0);
   QVector<double> nnls_b(narows, 0.0);
   QVector<double> nnls_x(nsolutes, 0.0);
   QVector<double> tinvec(ntinois, 0.0);
   QVector<double> rinvec(nrinois, 0.0);

   QVector<double> sv_norm(nsolutes, 0.0);

   // Set up for a single-component model
   US_Model model;
   model.components.resize(1);

   US_Model::SimulationComponent zcomponent; // Zeroed component to init models
   zcomponent.s = 0.0;
   zcomponent.D = 0.0;
   zcomponent.mw = 0.0;
   zcomponent.f = 0.0;
   zcomponent.f_f0 = 0.0;
   //-------------------------------------------------------------
   if (banddthr) { // If band forming, hold data within thresholds
      //mfactex=mfactor;
      wdata = data_sets[ d_offs ]->run_data;
      data_threshold(&wdata, zerothr, linethr, maxod, mfactex);
   }
   //-------------------------------------------------------------
   DebugTime("BEG:calcres");
   // Populate b array with experiment data concentrations
   int kb = 0;
   int kodl = 0;
   double norm_b = 0.0;
#if 0
   double s0max  = 0.0;
#endif
   //==================================================================
   for (int ee = offset; ee < lim_offs; ee++) {
      US_DataIO::EditedData *edata = &data_sets[ ee ]->run_data;
      edata = banddthr ? &wdata : edata;
      npoints = edata->pointCount();
      nscans = edata->scanCount();
      double odlim = edata->ODlimit;

      for (int ss = 0; ss < nscans; ss++) {
         for (int rr = 0; rr < npoints; rr++) { // Fill the B matrix with experiment data (or zero)
            double evalue = edata->value(ss, rr);
            //DbgLv(1)<<"subha_OD limit for scan "<< ss << rr << evalue << odlim;

            if (evalue >= odlim) { // Where ODlimit is exceeded, substitute 0.0 and bump count
               evalue = 0.0;
               kodl++;
               DbgLv(1) << "subha_OD limit exceeded for scan no " << ss << rr << odlim << kodl;
            }
#if 0
            else if ( ss == 0 )
            {  // Find max of scan 0 values
               s0max          = qMax( s0max, evalue );
            }
#endif

            // Store the B value in a row for a dataset
            nnls_b[ kb++ ] = evalue;
            norm_b += sq(evalue);

         } //"rr"-based for loop
      } // "ss"-based for loop
   } // "ee"-based for loop
   //DebugTime("END:clcr-NB");

   norm_b = norm_b > 0.0 ? sqrt(norm_b) : 0.0;
   DbgLv(1) << "kodl= " << kodl << "norm_b" << norm_b;
   DbgLv(1) << "   CR:B fill kodl" << kodl;
#if 0
   // If needed, scale the alpha used in A-matrix appendix diagonals
   if ( tikreg )
   {
      alphad         = ( s0max == 0.0 ) ? sim_vals.alpha
                       : ( sim_vals.alpha * sqrt( s0max ) );
   }
#endif

   if (abort)
      return;

   QList<US_DataIO::RawData> simulations; // All simulations, this run
   simulations.reserve(nsolutes * dataset_count);

   // Simulate data using models, each with a single s,f/f0 component
   int increp = nsolutes / 10; // Progress report increment
   increp = (increp < 10) ? 10 : increp;
   int kstep = 0; // Progress step count
   int ka = 0; // nnls_a output index
   int ksols = 0;
   int stype = data_sets[ offset ]->solute_type;

   if (use_zsol)
      qSort(sim_vals.zsolutes);
   else
      qSort(sim_vals.solutes);

   int count_cut = 0; // Count of A columns cut by norm tolerance
   int ksolutes = nsolutes; // Saved original number of solutes (columns)
   QList<int> cutsols; // List of original solute indecies for cuts
   QList<int> usesols; // List of original solute indecies for used

   //=========================================================================
   // Build up the A matrix for one of three cases:
   //  (1) Normal:      stype=0;    s,k,constant-vbar
   //  (2) Stype mask:  stype=1|>9; any x,y,z  (e.g, s,v,constant-k)
   //  (3) Custom:      stype=2;    custom grid
   //=========================================================================

   if (stype == 0) {
      DbgLv(1) << "stype == 0 case" << "nsolutes=" << nsolutes << "offset=" << offset << "lim_offs" << lim_offs;
      //======================================================================
      // Normal case of varying f/f0 with constant vbar
      //======================================================================
      int attr_x = 0; // Default X is s
      int attr_y = 1; // Default Y is f/f0
      int attr_z = 3; // Default Z is vbar
      int smask = (attr_x << 6) | (attr_y << 3) | attr_z;
      DataSet *dset = data_sets[ 0 ];

      DbgLv(2) << "   CR:BF s20wcorr D20wcorr" << dset->s20w_correction << dset->D20w_correction << "manual"
               << dset->solution_rec.buffer.manual << "vbar20" << dset->vbar20;

      //DebugTime("BEG:clcr-NA");
      for (int cc = 0; cc < ksolutes; cc++) { // Solve for each solute
         if (abort)
            return;
         int bx = 0;
         int kacol = ka;
         double norm_a = 0.0;
#if 0
         double norm_s  = norm_cs;
         QList< double > enorms;
#endif

         //DebugTime("BEG: clcr-NA-eeloop");
         for (int ee = offset; ee < lim_offs; ee++) { // Solve for each data set
            DataSet *dset = data_sets[ ee ];
            US_DataIO::EditedData *edata = banddthr ? &wdata : &dset->run_data;
            US_DataIO::RawData simdat;
            npoints = edata->pointCount();
            nscans = edata->scanCount();
            zcomponent.vbar20 = dset->vbar20;
            //            int kasub      = ka;
            double norm_e = 0.0;

            // Set model with standard space s and k
            if (use_zsol) {
               US_ZSolute::set_mcomp_values(model.components[ 0 ], sim_vals.zsolutes[ cc ], smask);
            }
            else {
               model.components[ 0 ] = zcomponent;
               set_comp_attr(model.components[ 0 ], sim_vals.solutes[ cc ], attr_x);
               set_comp_attr(model.components[ 0 ], sim_vals.solutes[ cc ], attr_y);
            }

            // Fill in the missing component values
            model.update_coefficients();
            //DbgLv(1) << "CR:   cc" << cc << "model s,k,w,v,d,c"
            // << model.components[0].s << model.components[0].f_f0
            // << model.components[0].mw << model.components[0].vbar20
            // << model.components[0].D << model.components[0].signal_concentration;

            // Convert to experimental space
            model.components[ 0 ].s /= dset->s20w_correction;
            model.components[ 0 ].D /= dset->D20w_correction;
            //DbgLv(1) << "CR:     exp.space model s,k,w,v,d,c"
            // << model.components[0].s << model.components[0].f_f0
            // << model.components[0].mw << model.components[0].vbar20
            // << model.components[0].D << model.components[0].signal_concentration;

            // Initialize simulation data with the experiment's grid
            DbgLv(2) << "   CR:111  rss now" << US_Memory::rss_now() << "cc" << cc;
            US_AstfemMath::initSimData(simdat, *edata, 0.0);

            DbgLv(2) << "   CR:112  rss now" << US_Memory::rss_now() << "cc" << cc;
            if (dbg_level > 1 && thrnrank < 2 && cc == 0) {
               model.debug();
               dset->simparams.debug();
            }

            // Check if timestate file exists or not
            if (
               dset->simparams.tsobj == NULL
               || dset->simparams.sim_speed_prof.count() < 1) { // Simparms does not yet have a loaded timestate
               DbgLv(0) << "solve_sim_calc_residuals_1 : TSOBJ" << dset->simparams.tsobj << "ssprof count"
                        << dset->simparams.sim_speed_prof.count();
               dset->simparams.tsobj = NULL; // Insure TimeState object and
               dset->simparams.sim_speed_prof.clear(); //  speed prof are both cleared.
               QString runID = edata->runID;
#ifdef NO_DB
               QString tmst_fpath = (dset->tmst_file.isEmpty()) ? "../" + runID + ".time_state.tmst" : dset->tmst_file;
#else
               QString tmst_fpath = US_Settings::resultDir() + "/" + runID + "/" + runID + ".time_state.tmst";
#endif
               QFileInfo check_file(tmst_fpath);

               if ((check_file.exists()) && (check_file.isFile())) { // Timestate file exists
                  bool intv_1sec = US_AstfemMath::timestate_onesec(tmst_fpath, simdat);
                  DbgLv(0) << "solve_sim_calc_residuals_1 : intv_1sec" << intv_1sec;

                  if (intv_1sec) { // Timestate is already at 1-second interval, so load it
                     dset->simparams.simSpeedsFromTimeState(tmst_fpath);
                     DbgLv(0) << "solve_sim_calc_residuals_1 : sim_speed_prof count"
                              << dset->simparams.sim_speed_prof.count() << " intv_1sec" << intv_1sec;
                  }
                  DbgLv(0) << "solve_sim_calc_residuals_1 : timestate file exists" << tmst_fpath
                           << " timestateobject=" << dset->simparams.tsobj
                           << "sspknt=" << dset->simparams.sim_speed_prof.count();
                  DbgLv(0) << "solve_sim_calc_residuals_1 : ntimes" << dset->simparams.tsobj->time_count();
               }
               else { // Timestate file does not exist (will be created in astfem_rsa)
                  DbgLv(0) << "solve_sim_1: timestate file does not exist" << tmst_fpath << dset->tmst_file;
               }
            }
            else
               DbgLv(2) << "solve_sim_1: timestate object exists  sspknt=" << dset->simparams.sim_speed_prof.count()
                        << " timestateobject=" << dset->simparams.tsobj;

            //DebugTime("BEG: clcr-NA-astfem");
            DbgLv(2) << "solve_sim_2: smeni sbott" << dset->simparams.meniscus << dset->simparams.bottom;
            US_Astfem_RSA astfem_rsa(model, dset->simparams);
            DbgLv(2) << "   CR:113  rss now" << US_Memory::rss_now() << "cc" << cc;

            astfem_rsa.set_debug_flag(dbg_level);

            astfem_rsa.calculate(simdat);
            //DebugTime("END: clcr-NA-astfem");
            DbgLv(2) << "   CR:114  rss now" << US_Memory::rss_now() << "cc" << cc;
            if (abort)
               return;

            if (banddthr) { // If band forming, hold data within thresholds; skip if all-zero
               if (data_threshold(&simdat, zerothr, linethr, maxod, mfactor))
                  continue;

               ksols++;
            }

            simulations << simdat; // Save simulation (each dataset,solute)
            DbgLv(2) << "   CR:115  rss now" << US_Memory::rss_now() << "cc" << cc;

            // Populate the A matrix for the NNLS routine with simulation
            DbgLv(1) << "   CR: A fill kodl" << kodl << "bndthr ksol" << banddthr << ksols << "  kacol" << kacol;

            if (kodl == 0) { // Normal case of no ODlimit substitutions

               for (int ss = 0; ss < nscans; ss++) {
                  for (int rr = 0; rr < npoints; rr++) {
                     double sval = simdat.value(ss, rr);
                     nnls_a[ ka++ ] = sval;
                     norm_a += sq(sval);
                     norm_e += sq(sval);
                  }
               }
               DbgLv(1) << " T0O0:norm_of_the_vector" << norm_a << sim_vals.solutes[ cc ].s << sim_vals.solutes[ cc ].k;
               //------------------------------------------------

               //DbgLv(2) << "   CR:116  rss now" << US_Memory::rss_now() << "cc" << cc;
               //if(lim_offs>1&&(thrnrank==1||thrnrank==11))
               // DbgLv(1) << "CR: kacol ka" << kacol << ka
               //  << "nnA s...k" << nnls_a[ks] << nnls_a[ks+1]
               //  << nnls_a[ka-2] << nnls_a[ka-1]
               //  << "cc ee" << cc << ee;
            }

            else { // Special case where ODlimit substitutions are in B matrix
               DbgLv(1) << "stype==0 but from else part";

               for (int ss = 0; ss < nscans; ss++) {
                  for (int rr = 0; rr < npoints; rr++) { // Fill A with simulations (or zero where B has zero)
                     // x  x  x  x  x  x  x  x
                     if (nnls_b[ bx++ ] != 0.0) {
                        double sval = simdat.value(ss, rr);
                        nnls_a[ ka++ ] = sval;
                        norm_a += sq(sval);
                        norm_e += sq(sval);
                     }

                     else {
                        nnls_a[ ka++ ] = 0.0;
                     }
                  }
               }
               DbgLv(1) << " T0O1:norm_of_the_vector" << norm_a << sim_vals.solutes[ cc ].s << sim_vals.solutes[ cc ].k;
            }

#if 0
            if ( dataset_count > 1 )
            {  // Global-fit:  test each dataset portion of a column
               // Compute Norm of A sub-column:  square root of sum-of-squares
               norm_e         = norm_e > 0.0 ? sqrt( norm_e ) : 0.0;
               enorms << norm_e;
//               if ( norm_e < norm_cut )
               if ( norm_e < norm_cs )
               {  // Sub-column for dataset to be zeroed
DbgLv(0) << "CR: NORM_E" << norm_e << norm_cut << "cc ee" << cc << ee;
                  norm_s         = qMin( norm_s, norm_e );
#if 0
                  double sval    = norm_cut;
                  int ksend      = kasub + ( nscans * npoints );
                  for ( int ksa = kasub; ksa < ksend; ksa++ )
                  {
                     nnls_a[ ksa ]  = sval;
                  }
#endif
               }
            }
#endif
            DbgLv(1) << "CR: NNLS A filled ee" << ee << lim_offs;
            DbgLv(1) << "CR: NNLS  &simdat" << &simdat;
            DbgLv(1) << "CR: NNLS  &model " << &model;
            DbgLv(1) << "CR: NNLS  &nnls_a" << &nnls_a;
            DbgLv(1) << "CR: NNLS  &simulations" << &simulations;
            DbgLv(1) << "CR: NNLS  astfem_rsa" << &astfem_rsa;
            //DebugTime("END:   clcr-NA-eeiter");
         } // Each data set of constant vbar (stype=1)
         //DebugTime("END: clcr-NA-eeloop");
         DbgLv(1) << "CR: NNLS A filled lo" << lim_offs;
         //NSave = v2;
         //----------------------------------------
         if (tikreg) { // For Tikhonov Regularization append to each column
            int colx = cc - count_cut;

            for (int aa = 0; aa < ksolutes; aa++) {
               nnls_a[ ka++ ] = (aa == colx) ? alphad : 0.0;
            }
         }

         // Compute Norm of A column:  square root of sum-of-squares
         norm_a = norm_a > 0.0 ? sqrt(norm_a) : 0.0;
#if 0
         if ( norm_s < norm_cs )
         {
DbgLv(0)<< " norm_s norm_a norm_b" << norm_s << norm_a << norm_b
 << "enorms" << enorms;
            norm_a         = norm_s;
         }
#endif

         if (norm_a < norm_cut) { // Norm for A column falls below cutoff:  skip column
            ka = kacol; // Reset to replace this column
            nsolutes--; // Decrement solutes count
            count_cut++; // Bump count of cut columns
            cutsols << cc; // Save original index of cut solute
            //DbgLv(0)<< " norm is becoming zero -- norm_a" << norm_a
            // << "count_cut" << count_cut << "cc" << cc;
         }
         else {
            usesols << cc; // Save original index of used solute
         }
         DbgLv(1) << " T0:NCt: norm_of_the_vector" << norm_a << "s k" << sim_vals.solutes[ cc ].s
                  << sim_vals.solutes[ cc ].k;

         if (signal_wanted && ++kstep == increp) { // If asked for and step at increment, signal progress
            emit work_progress(increp);
            kstep = 0; // Reset step count
         }
         DbgLv(1) << "CR: NNLS A filled cc" << cc << nsolutes;
      } // Each solute of constant vbar
      //DebugTime("END:clcr-NA");
      // x  x  x  x  x  x  x  x
      //DbgLv(1) << "CR: NNLS A filled nsol ksol cutsol[n]"
      // << nsolutes << ksolutes << cutsols[cutsols.size()-1];
   } // Constant vbar

   else if (stype == 1 || stype > 9) {
      //======================================================================
      // Special case of varying vbar with constant f/f0  (or other)
      //======================================================================
      //DbgLv(1) << "stype == 1  ||  stype > 9 case :";
      int attr_x = 0; // Default X is s
      int attr_y = 3; // Default Y is vbar
      int attr_z = 1; // Default fixed is f/f0
      int smask = (attr_x << 6) | (attr_y << 3) | attr_z;

      if (stype > 9) { // Explicitly given attribute types
         attr_x = (stype >> 6) & 7;
         attr_y = (stype >> 3) & 7;
         attr_z = stype & 7;
         smask = stype;
      }
      DbgLv(1) << "CR: attr_ x,y,z" << attr_x << attr_y << attr_z << stype << smask << "use_zsol" << use_zsol;

      if (!use_zsol) {
         zcomponent.vbar20 = data_sets[ 0 ]->vbar20;
         set_comp_attr(zcomponent, sim_vals.solutes[ 0 ], attr_z);
      }

      double vbartb = data_sets[ 0 ]->vbartb; // In case Z is vbar
      double scorre = data_sets[ 0 ]->s20w_correction;
      double dcorre = data_sets[ 0 ]->D20w_correction;

      for (int cc = 0; cc < ksolutes; cc++) { // Solve for each solute
         if (abort)
            return;
         int bx = 0;
         int kacol = ka;
         double norm_a = 0.0;
         DbgLv(1) << "CR: cc" << cc << " use_zsol" << use_zsol;

         for (int ee = offset; ee < lim_offs; ee++) { // Solve for each data set
            US_Math2::SolutionData sd;
            DataSet *dset = data_sets[ ee ];
            US_DataIO::EditedData *edata = banddthr ? &wdata : &dset->run_data;
            US_DataIO::RawData simdat;
            nscans = edata->scanCount();
            npoints = edata->pointCount();
            model.components[ 0 ] = zcomponent;

            // Set model with standard space s,k,v  (or other 3 attributes)
            if (use_zsol) {
               US_ZSolute::set_mcomp_values(model.components[ 0 ], sim_vals.zsolutes[ cc ], smask);
               //DbgLv(1) << "CR:   cc" << cc << "model s,k,w,v,d,c"
               // << model.components[0].s << model.components[0].f_f0
               // << model.components[0].mw << model.components[0].vbar20
               // << model.components[0].D << model.components[0].signal_concentration;
            }
            else {
               set_comp_attr(model.components[ 0 ], sim_vals.solutes[ cc ], attr_x);
               set_comp_attr(model.components[ 0 ], sim_vals.solutes[ cc ], attr_y);
               set_comp_attr(model.components[ 0 ], sim_vals.solutes[ cc ], attr_z);
            }

            // Fill in the missing component values
            model.update_coefficients();

            // Convert to experimental space
            double avtemp = dset->temperature;
            sd.viscosity = dset->viscosity;
            sd.density = dset->density;
            sd.manual = dset->manual;
            sd.vbar20 = model.components[ 0 ].vbar20;
            if (attr_z != 3) { // Vbar not fixed:  temperature adjust vbar20
               sd.vbar = US_Math2::adjust_vbar20(sd.vbar20, avtemp);
               US_Math2::data_correction(avtemp, sd);
            }
            else { // Vbar fixed:  use already computed vbar, corrections
               sd.vbar = vbartb;
               sd.s20w_correction = scorre;
               sd.D20w_correction = dcorre;
            }

            model.components[ 0 ].s /= sd.s20w_correction;
            model.components[ 0 ].D /= sd.D20w_correction;
            //DbgLv(1) << "CR:     exp.space model s,k,w,v,d,c"
            // << model.components[0].s << model.components[0].f_f0
            // << model.components[0].mw << model.components[0].vbar20
            // << model.components[0].D << model.components[0].signal_concentration;

            // Initialize simulation data with the experiment's grid
            US_AstfemMath::initSimData(simdat, *edata, 0.0);

            if (dbg_level > 1 && thrnrank < 2 && cc == 0) {
               model.debug();
               dset->simparams.debug();
            }
            DbgLv(1) << "CR:  simdat nsc npt" << simdat.scanCount() << simdat.pointCount();

            // Calculate Astfem_RSA solution (Lamm equations)
            //
            if (
               dset->simparams.tsobj == NULL
               || dset->simparams.sim_speed_prof.count() < 1) { // Simparms does not yet have a loaded timestate
               DbgLv(0) << "solve_sim_calc_residuals_2 : TSOBJ" << dset->simparams.tsobj << "ssprof count"
                        << dset->simparams.sim_speed_prof.count();
               QString runID = edata->runID;
#ifdef NO_DB
               QString tmst_fpath = (dset->tmst_file.isEmpty()) ? "../" + runID + ".time_state.tmst" : dset->tmst_file;
#else
               QString tmst_fpath = US_Settings::resultDir() + "/" + runID + "/" + runID + ".time_state.tmst";
#endif
               QFileInfo check_file(tmst_fpath);

               if ((check_file.exists()) && (check_file.isFile())) { // Timestate file exists
                  bool intv_1sec = US_AstfemMath::timestate_onesec(tmst_fpath, simdat);
                  DbgLv(1) << "solve_sim_calc_residuals_2 : intv_1sec" << intv_1sec;

                  if (intv_1sec) {
                     dset->simparams.simSpeedsFromTimeState(tmst_fpath);
                     DbgLv(0) << "solve_sim_calc_residuals_2 : sim_speed_prof count"
                              << dset->simparams.sim_speed_prof.count() << " intv_1sec" << intv_1sec;
                  }
                  DbgLv(1) << "solve_sim_calc_residuals_2 : timestate file exists" << tmst_fpath
                           << " timestateobject=" << dset->simparams.tsobj
                           << "sspknt=" << dset->simparams.sim_speed_prof.count();
                  DbgLv(1) << "solve_sim_calc_residuals_2 : ntimes" << dset->simparams.tsobj->time_count();
               }
               else { // Timestate file does not exist (will be created in astfem_rsa)
                  DbgLv(1) << "solve_sim_2: timestate file does not exist" << tmst_fpath << dset->tmst_file;
               }
            }

            US_Astfem_RSA astfem_rsa(model, dset->simparams);

            astfem_rsa.set_debug_flag(dbg_level);

            astfem_rsa.calculate(simdat);
#if 0
int nsc=simdat.scanCount();
int npt=simdat.pointCount();
int ms=nsc/2;
int ls=nsc-1;
int mp=npt/2;
int lp=npt-1;
DbgLv(1) << "CR: edat:"
 << edata->value( 0, 0) << edata->value( 0,mp) << edata->value( 0,lp)
 << edata->value(ms, 0) << edata->value(ms,mp) << edata->value(ms,lp)
 << edata->value(ls, 0) << edata->value(ls,mp) << edata->value(ls,lp);
DbgLv(1) << "CR: sdat:"
 << simdat.value( 0, 0) << simdat.value( 0,mp) << simdat.value( 0,lp)
 << simdat.value(ms, 0) << simdat.value(ms,mp) << simdat.value(ms,lp)
 << simdat.value(ls, 0) << simdat.value(ls,mp) << simdat.value(ls,lp);
#endif
            if (abort)
               return;

            if (banddthr) { // If band forming, hold data within thresholds; skip if all-zero
               if (data_threshold(&simdat, zerothr, linethr, maxod, mfactor))
                  continue;

               ksols++;
            }

            simulations << simdat; // Save simulation (each datset,solute)

            // Populate the A matrix for the NNLS routine with simulation
            DbgLv(1) << "   CR: A-fill  bndthr" << banddthr << "kodl ksols" << kodl << ksols;
            int ks = ka;

            if (kodl == 0) { // Normal case of no ODlimit substitutions
               for (int ss = 0; ss < nscans; ss++) {
                  for (int rr = 0; rr < npoints; rr++) {
                     double sval = simdat.value(ss, rr);
                     nnls_a[ ka++ ] = sval;
                     norm_a += sq(sval);
                  }
                  // x  x  x  x  x  x  x  x
               }
            }
            else { // Special case where ODlimit substitutions are in B matrix
               for (int ss = 0; ss < nscans; ss++) {
                  for (int rr = 0; rr < npoints; rr++) { // Fill A with simulations (or zero where B has zero)
                     if (nnls_b[ bx++ ] != 0.0) {
                        double sval = simdat.value(ss, rr);
                        nnls_a[ ka++ ] = sval;
                        norm_a += sq(sval);
                     }
                     else {
                        nnls_a[ ka++ ] = 0.0;
                        // x  x  x  x  x  x  x  x
                     }
                  }
               }
            }

            //DbgLv(1) << "CR: ks ka" << ks << ka
            // << "nnA s...k" << nnls_a[ks] << nnls_a[ks+1] << nnls_a[ka-2] << nnls_a[ka-1]
            // << "cc ee" << cc << ee << "kodl" << kodl;
            DbgLv(1) << "CR: cc ee" << cc << ee << "ks ka" << ks << ka << "nnls_a sz" << nnls_a.count() << "  sz-kd"
                     << (nnls_a.count() - ka);
         } // Each data set

         if (tikreg) { // For Tikhonov Regularization append to each column
            DbgLv(1) << "CR: cc" << cc << " (PRE-tikreg)";
            int colx = cc - count_cut;

            for (int aa = 0; aa < nsolutes; aa++) {
               nnls_a[ ka++ ] = (aa == colx) ? alphad : 0.0;
            }
         }
         norm_a = norm_a > 0.0 ? sqrt(norm_a) : 0.0;

         if (norm_a < norm_cut) {
            ka = kacol;
            nsolutes--;
            count_cut++;
            cutsols << cc;
            //DbgLv(1)<< " norm is becoming zero -- norm_a" << norm_a << "count_cut" << count_cut;
         }
         else
            usesols << cc;
         if (signal_wanted && ++kstep == increp) { // If asked for and step at increment, signal progress
            emit work_progress(increp);
            kstep = 0; // Reset step count
         }
      } // Each solute
   } // Constant f/f0  (or other stype>9)

   else {
      DbgLv(1) << " Special case of custom grid";
      //======================================================================
      // Special case of custom grid
      //======================================================================
      int attr_x = 0; // Set X is s
      int attr_y = 4; // Set Y is D
      int attr_z = 3; // Set Z is vbar
      int smask = (attr_x << 6) | (attr_y << 3) | attr_z;

      for (int cc = 0; cc < ksolutes; cc++) { // Solve for each solute
         if (abort)
            return;
         int bx = 0;
         int kacol = ka;
         double norm_a = 0.0;

         for (int ee = offset; ee < lim_offs; ee++) { // Solve for each data set
            US_Math2::SolutionData sd;
            DataSet *dset = data_sets[ ee ];
            US_DataIO::EditedData *edata = banddthr ? &wdata : &dset->run_data;
            US_DataIO::RawData simdat;
            npoints = edata->pointCount();
            nscans = edata->scanCount();
            double avtemp = dset->temperature;
            zcomponent.vbar20 = dset->vbar20;
            model.components[ 0 ] = zcomponent;

            if (use_zsol) {
               US_ZSolute::set_mcomp_values(model.components[ 0 ], sim_vals.zsolutes[ cc ], smask);
            }
            else {
               set_comp_attr(model.components[ 0 ], sim_vals.solutes[ cc ], attr_x);
               set_comp_attr(model.components[ 0 ], sim_vals.solutes[ cc ], attr_y);
               set_comp_attr(model.components[ 0 ], sim_vals.solutes[ cc ], attr_z);
            }

            // Fill in the missing component values
            model.update_coefficients();

            // Convert to experimental space
            sd.viscosity = dset->viscosity;
            sd.density = dset->density;
            sd.manual = dset->manual;
            sd.vbar20 = model.components[ 0 ].vbar20;
            sd.vbar = US_Math2::adjust_vbar20(sd.vbar20, avtemp);

            US_Math2::data_correction(avtemp, sd);

            model.components[ 0 ].s /= sd.s20w_correction;
            model.components[ 0 ].D /= sd.D20w_correction;

            // Initialize simulation data with the experiment's grid
            US_AstfemMath::initSimData(simdat, *edata, 0.0);

            if (dbg_level > 1 && thrnrank < 2 && cc == 0) {
               model.debug();
               dset->simparams.debug();
            }

            // Check if timestate file exists or not
            if (
               dset->simparams.tsobj == NULL
               || dset->simparams.sim_speed_prof.count() < 1) { // Simparms does not yet have a loaded timestate
               DbgLv(0) << "solve_sim_calc_residuals_3 : TSOBJ" << dset->simparams.tsobj << "ssprof count"
                        << dset->simparams.sim_speed_prof.count();
               QString runID = edata->runID;
#ifdef NO_DB
               QString tmst_fpath = (dset->tmst_file.isEmpty()) ? "../" + runID + ".time_state.tmst" : dset->tmst_file;
#else
               QString tmst_fpath = US_Settings::resultDir() + "/" + runID + "/" + runID + ".time_state.tmst";
#endif
               QFileInfo check_file(tmst_fpath);

               if ((check_file.exists()) && (check_file.isFile())) { // Timestate file exists
                  bool intv_1sec = US_AstfemMath::timestate_onesec(tmst_fpath, simdat);
                  DbgLv(1) << "solve_sim_calc_residuals_3 : intv_1sec" << intv_1sec;

                  if (intv_1sec) {
                     dset->simparams.simSpeedsFromTimeState(tmst_fpath);
                     DbgLv(0) << "solve_sim_calc_residuals_3 : sim_speed_prof count"
                              << dset->simparams.sim_speed_prof.count() << " intv_1sec" << intv_1sec;
                  }
                  DbgLv(1) << "solve_sim_calc_residuals_3 : timestate file exists" << tmst_fpath
                           << " timestateobject=" << dset->simparams.tsobj
                           << "sspknt=" << dset->simparams.sim_speed_prof.count();
                  DbgLv(1) << "solve_sim_calc_residuals_3 : ntimes" << dset->simparams.tsobj->time_count();
               }
               else { // Timestate file does not exist (will be created in astfem_rsa)
                  DbgLv(1) << "solve_sim_3: timestate file does not exist" << tmst_fpath << dset->tmst_file;
               }
            }

            US_Astfem_RSA astfem_rsa(model, dset->simparams);

            astfem_rsa.set_debug_flag(dbg_level);

            astfem_rsa.calculate(simdat);
            if (abort)
               return;

            if (banddthr) { // If band forming, hold data within thresholds; skip if all-zero
               if (data_threshold(&simdat, zerothr, linethr, maxod, mfactor))
                  continue;

               ksols++;
            }

            simulations << simdat; // Save simulation (each datset,solute)

            /*DEBUG*/
            int ks = ka;
            /*DEBUG*/

            // Populate the A matrix for the NNLS routine with simulation
            if (kodl == 0) { // Normal case of no ODlimit substitutions
               for (int ss = 0; ss < nscans; ss++) {
                  for (int rr = 0; rr < npoints; rr++) {
                     double sval = simdat.value(ss, rr);
                     nnls_a[ ka++ ] = sval;
                     norm_a += sq(sval);
                  }
               }
            }
            else { // Special case where ODlimit substitutions are in B matrix
               for (int ss = 0; ss < nscans; ss++) {
                  for (int rr = 0; rr < npoints; rr++) { // Fill A with simulations (or zero where B has zero)
                     if (nnls_b[ bx++ ] != 0.0) {
                        double sval = simdat.value(ss, rr);
                        nnls_a[ ka++ ] = sval;
                        norm_a += sq(sval);
                     }
                     else {
                        nnls_a[ ka++ ] = 0.0;
                     }
                  }
               }
            }
            DbgLv(1) << "CR: ks ka" << ks << ka << "nnA s...k" << nnls_a[ ks ] << nnls_a[ ks + 1 ] << nnls_a[ ka - 2 ]
                     << nnls_a[ ka - 1 ] << "cc ee" << cc << ee << "kodl" << kodl;
         } // Each data set, custom grid

         if (tikreg) { // For Tikhonov Regularization append to each column
            int colx = cc - count_cut;

            for (int aa = 0; aa < nsolutes; aa++) {
               nnls_a[ ka++ ] = (aa == colx) ? alphad : 0.0;
            }
         }

         norm_a = norm_a > 0.0 ? sqrt(norm_a) : 0.0;

         if (norm_a < norm_cut) {
            ka = kacol;
            nsolutes--;
            count_cut++;
            cutsols << cc;
            //DbgLv(1)<< " norm is becoming zero -- norm_a" << norm_a << "count_cut" << count_cut;
         }
         else
            usesols << cc;
         DbgLv(1) << " norm_of_the_vector" << norm_a << "s k" << sim_vals.solutes[ cc ].s << sim_vals.solutes[ cc ].k;
         if (signal_wanted && ++kstep == increp) { // If asked for and step at increment, signal progress
            emit work_progress(increp);
            kstep = 0; // Reset step count
         }
      } // Each solute, custom grid
   } // Custom grid

   if (NSave != NULL) {
      sv_norm.resize(nsolutes);
      *NSave = sv_norm;
      DbgLv(1) << "saving_norms  size=" << sv_norm.size() << nsolutes;
      for (int ii = 0; ii < sv_norm.size(); ii++)
         DbgLv(1) << "ii" << ii << "norm_value" << sv_norm[ ii ] << (*NSave)[ ii ];
   }

   //DebugTime("BEG:clcr-so");
   if (count_cut > 0) { // Compress solutes down to those uncut by norm comparison
      int kk = 0;

      if (use_zsol) {
         for (int cc = 0; cc < ksolutes; cc++) {
            if (cutsols.contains(cc))
               continue;

            sim_vals.zsolutes[ kk++ ] = sim_vals.zsolutes[ cc ];
         }
      }

      else {
         for (int cc = 0; cc < ksolutes; cc++) {
            if (cutsols.contains(cc))
               continue;

            sim_vals.solutes[ kk++ ] = sim_vals.solutes[ cc ];
         }
      }

      if (kk != nsolutes) {
         qDebug() << "KK!=NSOLUTES:  kk" << kk << "nsolutes" << nsolutes << "ksolutes" << ksolutes << "count_cut"
                  << count_cut << cutsols.count();
      }
      DbgLv(1) << " norm cuts for #solutes=" << ksolutes << ":  count_cut" << count_cut;
   }

   if (tikreg && (nsolutes != ksolutes)) { // For regularization and norm-tolerance change in solutes, redo A
      narows = ntotal + nsolutes;
      int ja = 0;
      int ka = 0;

      for (int cc = 0; cc < nsolutes; cc++) {
         // Copy simulation point row values
         for (int aa = 0; aa < ntotal; aa++) {
            nnls_a[ ka++ ] = nnls_a[ ja++ ];
         }

         // Rebuild alpha-diagonal part of row
         for (int aa = 0; aa < nsolutes; aa++) {
            nnls_a[ ka++ ] = (aa == cc) ? alphad : 0.0;
         }

         // Bump past old alpha-diagonal rows of input
         ja += ksolutes;
      }
   }

   nsolutes = banddthr ? ksols : nsolutes;
   int ntotinoi = ntinois * nsolutes;
   int ntorinoi = nrinois * nsolutes;
   int nsolutsq = nsolutes * nsolutes;

   if (signal_wanted && kstep > 0) // If signals and steps done, report
      emit work_progress(kstep);

   if (abort)
      return;

#if 0
   /////////////////////////////////////////////////////
   // Build the nsolutes x nsolutes angles vector
   /////////////////////////////////////////////////////
   int nangles  = nsolutes * nsolutes;
   double angle = 0.0;
   QVector< double > angles( nangles, 0.0 );
   double* mav  = nnls_a.data();
   for( int ii = 0; ii < nsolutes; ii++ )
   {  // Compute angles for this column compared to ones beyond it
      double* av1  = mav + ( ii * narows );
      int aa       = ii * nsolutes + ii + 1;
      for ( int jj = ii + 1; jj < nsolutes; jj++, aa++ )
      {
         double* av2  = mav + ( jj * narows );
         angle        = angle_vectors( av1, av2, ntotal );
         angles[ aa ] = angle;
      }
   }
   // Fill in angles in lower half that are already computed
   for( int ii = 1; ii < nsolutes; ii++ )
   {  // Copy angles for columns before this one
      for ( int jj = 0; jj < ii; jj++ )
      {
         int aa       = ii * nsolutes + jj;
         int aa1      = jj * nsolutes + ii;
         angles[ aa ] = angles[ aa1 ];
      }
   }
   // Now print all the angles
   int aa       = 0;
   double afact = 180.0 / M_PI;
   for( int ii = 0; ii < nsolutes; ii++ )
   {  // Print the angles for solute ii
      qDebug() << thrnrank << "w:ANGL: row=" << ii
               << "s=" << ( sim_vals.solutes[ ii ].s * 1e+13 )
               << "k=" << sim_vals.solutes[ ii ].k
               << "nsol=" << nsolutes;
      QString arow( " angles=" );
      for ( int jj = 0; jj < nsolutes; jj++ )
      {  // Compose each angle value string for each column of the row
         double aval  = angles[ aa++ ] * afact;
         arow        += QString().sprintf( "%6.1f", aval );
      }
      qDebug() << thrnrank << "w:ANGL:" << arow;
   }
#endif


   int kstodo = nsolutes / 50; // Set steps count for NNLS
   kstodo = max(kstodo, 2);
   DbgLv(1) << "   CR:200  rss now" << US_Memory::rss_now() << "thrn" << thrnrank;
   //DebugTime("BEG:clcr-cn");
   //------------------------------------------
   //Check linear independence of nnls_a matrix
   //------------------------------------------
   DbgLv(1) << "subha_nnls_a size: " << nnls_a.size() << nscans << npoints << "nsolutes=" << nsolutes;
   //------------------------------------------

   if (calc_ti) { // Compute TI Noise (and, optionally, RI Noise)
      if (abort)
         return;
      QVector<double> a_tilde(nrinois, 0.0);
      QVector<double> a_bar(ntinois, 0.0);
      QVector<double> L_tildes(ntorinoi, 0.0);
      QVector<double> L_bars(ntotinoi, 0.0);
      QVector<double> small_a(nsolutsq, 0.0);
      QVector<double> small_b(nsolutes, 0.0);
      QVector<double> L(ntotal, 0.0);
      QVector<double> L_tilde(nrinois, 0.0);
      QVector<double> L_bar(ntinois, 0.0);

      // Compute a_tilde, the average experiment signal at each time
      if (calc_ri)
         compute_a_tilde(a_tilde, nnls_b);

      // Compute a_bar, the average experiment signal at each radius
      compute_a_bar(a_bar, a_tilde, nnls_b);

      // Compute L_tildes, the average signal at each radius (if RI noise)
      if (calc_ri)
         compute_L_tildes(nrinois, nsolutes, L_tildes, nnls_a);

      // Compute L_bars
      compute_L_bars(nsolutes, nrinois, ntinois, ntotal, L_bars, nnls_a, L_tildes);

      // Set up small_a, small_b for alternate nnls
      DbgLv(1) << "  set SMALL_A+B";
      ti_small_a_and_b(nsolutes, ntotal, ntinois, small_a, small_b, a_bar, L_bars, nnls_a, nnls_b);
      if (abort)
         return;

      // Do NNLS to compute concentrations (nnls_x)
      DbgLv(1) << "  noise small NNLS";
      US_Math2::nnls(small_a.data(), nsolutes, nsolutes, nsolutes, small_b.data(), nnls_x.data());

      if (abort)
         return;

      // This is Sum( concentration * Lamm ) for the models after NNLS
      compute_L(ntotal, nsolutes, L, nnls_a, nnls_x);

      // Now L contains the best fit sum of L equations
      // Compute L_tilde, the average model signal at each radius

      if (calc_ri)
         compute_L_tilde(L_tilde, L);

      // Compute L_bar, the average model signal at each radius
      compute_L_bar(L_bar, L, L_tilde);

      // Compute ti noise
      for (int ii = 0; ii < ntinois; ii++)
         tinvec[ ii ] = a_bar[ ii ] - L_bar[ ii ];

      if (calc_ri) { // Compute RI_noise
         for (int ii = 0; ii < nrinois; ii++)
            rinvec[ ii ] = a_tilde[ ii ] - L_tilde[ ii ];
      }

      if (signal_wanted)
         emit work_progress(kstodo); // Report noise NNLS steps done
   } // End tinoise and optional rinoise calculation

   else if (calc_ri) { // Compute RI noise (when RI only)
      if (abort)
         return;
      QVector<double> a_tilde(nrinois, 0.0);
      QVector<double> L_tildes(ntorinoi, 0.0);
      QVector<double> small_a(nsolutsq, 0.0);
      QVector<double> small_b(nsolutes, 0.0);
      QVector<double> L(ntotal, 0.0);
      QVector<double> L_tilde(nrinois, 0.0);

      // Compute a_tilde, the average experiment signal at each time
      compute_a_tilde(a_tilde, nnls_b);

      // Compute L_tildes, the average signal at each radius
      compute_L_tildes(nrinois, nsolutes, L_tildes, nnls_a);

      // Set up small_a, small_b for the nnls
      if (abort)
         return;

      ri_small_a_and_b(nsolutes, ntotal, nrinois, small_a, small_b, a_tilde, L_tildes, nnls_a, nnls_b);
      if (abort)
         return;

      US_Math2::nnls(small_a.data(), nsolutes, nsolutes, nsolutes, small_b.data(), nnls_x.data());
      if (abort)
         return;

      // This is sum( concentration * Lamm ) for the models after NNLS
      compute_L(ntotal, nsolutes, L, nnls_a, nnls_x);

      // Now L contains the best fit sum of L equations
      // Compute L_tilde, the average model signal at each radius
      compute_L_tilde(L_tilde, L);

      // Compute ri_noise  (Is this correct????)
      for (int ii = 0; ii < nrinois; ii++)
         rinvec[ ii ] = a_tilde[ ii ] - L_tilde[ ii ];

      if (signal_wanted)
         emit work_progress(kstodo); // Report noise NNLS steps done
   } // End rinoise alone calculation

   else { // No TI or RI noise
      if (abort)
         return;
      DbgLv(2) << "   CR:210  rss now" << US_Memory::rss_now() << "thrn" << thrnrank;

      if (ASave != NULL && BSave != NULL) {
         narows = tikreg ? (ntotal + nsolutes) : ntotal;
         navals = narows * nsolutes;
         nnls_a.resize(navals);
         sv_nnls_a = nnls_a;
         sv_nnls_b = nnls_b;
         DbgLv(1) << "no_ti_or_ri: CR: sv_nnls_a size" << sv_nnls_a.size() << nnls_a.size();
      }

      //DebugTime("BEG:clcr-nl");
      US_Math2::nnls(nnls_a.data(), narows, narows, nsolutes, nnls_b.data(), nnls_x.data());
      //DebugTime("END:clcr-nl");

      DbgLv(2) << "   CR:211  rss now" << US_Memory::rss_now() << "thrn" << thrnrank;
      if (lim_offs > 1 && (thrnrank == 1 || thrnrank == 11)) {
         DbgLv(1) << "CR: narows nsolutes" << narows << nsolutes;
         DbgLv(1) << "CR:  a0 a1 b0 b1" << nnls_a[ 0 ] << nnls_a[ 1 ] << nnls_b[ 0 ] << nnls_b[ 1 ];
      }

      if (abort)
         return;

      if (signal_wanted)
         emit work_progress(kstodo); // Report no-noise NNLS steps done
      // Note:  ti_noise and ri_noise are already zero

   } // End of core calculations

   sim_vals.maxrss = US_Memory::rss_max(sim_vals.maxrss);
   //DbgLv(1) << "   CR:na  rss now,max" << US_Memory::rss_now() << sim_vals.maxrss;
   DbgLv(1) << "   CR:na  rss now,max" << US_Memory::rss_now() << sim_vals.maxrss << &sim_vals;

   nnls_a.clear();
   nnls_b.clear();
   //DebugTime("END:clcr-nn");

   DbgLv(1) << "CR: kstodo" << kstodo;
   if (abort)
      return;

   // Size simulation data and initialize concentrations to zero
   int kscans = 0;
   int jscan = 0;

   for (int ee = offset; ee < lim_offs; ee++) {
      US_DataIO::RawData tdata; // Init temp sim data with edata's grid
      US_AstfemMath::initSimData(tdata, data_sets[ ee ]->run_data, 0.0);

      int nscans = tdata.scanData.size();
      kscans += nscans;
      sim_vals.sim_data.scanData.resize(kscans);

      if (ee == offset) {
         sim_vals.sim_data = tdata; // Init (first) dataset sim data
         jscan += nscans;
      }
      else {
         for (int ss = 0; ss < nscans; ss++) { // Append zeroed-scans sim_data for multiple data sets
            DbgLv(1) << "CR:     ss jscan" << ss << jscan << "ee kscans nscans" << ee << kscans << nscans;
            sim_vals.sim_data.scanData[ jscan++ ] = tdata.scanData[ ss ];
         }
      }
      DbgLv(2) << "   CR:221  rss now" << US_Memory::rss_now() << "thrn" << thrnrank;
   }

   if (lim_offs > 1 && (thrnrank == 1 || thrnrank == 11))
      DbgLv(1) << "CR:       jscan kscans" << jscan << kscans;

   // This is a little tricky.  The simulations structure was created above
   // in a loop that alternates experiments with each solute.  In the loop
   // below, the simulation structure alters that order to group all solutes
   // for each experiment together

   if (thrnrank == 1)
      DbgLv(1) << "CR: nsolutes" << nsolutes;
   if (lim_offs > 1 && (thrnrank == 1 || thrnrank == 11))
      DbgLv(1) << "CR: nsolutes" << nsolutes;

   for (int cc = 0; cc < nsolutes; cc++) {
      double soluval = nnls_x[ cc ]; // Computed concentration, this solute
      if (thrnrank == 1)
         DbgLv(1) << "CR: cc soluval" << cc << soluval;
      if (lim_offs > 1 && (thrnrank == 1 || thrnrank == 11))
         DbgLv(1) << "CR: cc soluval" << cc << soluval;

      if (soluval > 0.0) { // If concentration non-zero, need to sum in simulation data
         if (lim_offs > 1 && (thrnrank == 1 || thrnrank == 11))
            DbgLv(1) << "CR: cc soluval" << cc << soluval << "cc-old" << usesols[ cc ];
         int scnx = 0;
         int sim_ix = usesols[ cc ] * dataset_count;
         if (soluval > 1.0)
            DbgLv(1) << thrnrank << ": SOLUVAL" << soluval << "cc" << cc << "sim_ix" << sim_ix << "ksol nsol"
                     << ksolutes << nsolutes;

         for (int ee = offset; ee < lim_offs; ee++, sim_ix++) {
            // Input sims (ea.dset, ea.solute); out sims (sum.solute, ea.dset)
            //            int sim_ix  = cc * dataset_count + ee - offset;
            US_DataIO::RawData *idata = &simulations[ sim_ix ];
            US_DataIO::RawData *sdata = &sim_vals.sim_data;
            int nscans = idata->scanCount();
            int npoints = idata->pointCount();
            if (lim_offs > 1 && (thrnrank == 1 || thrnrank == 11))
               DbgLv(1) << "CR:    ee sim_ix np ns scnx" << ee << sim_ix << npoints << nscans << scnx;
            if (soluval > 1.0)
               DbgLv(1) << thrnrank << ":  SOLUVAL>1  ee nscans npoints" << ee << nscans << npoints << "idatamm"
                        << idata->value(nscans / 2, npoints / 2);

            for (int ss = 0; ss < nscans; ss++, scnx++) {
               for (int rr = 0; rr < npoints; rr++) {
                  sdata->scanData[ scnx ].rvalues[ rr ] += (soluval * idata->value(ss, rr));
               }
            }
            DbgLv(2) << "   CR:231  rss now" << US_Memory::rss_now() << "thrn" << thrnrank;
            //*DEBUG*
            int ss = nscans / 2;
            int rr = npoints / 2;
            if (thrnrank == 1) {
               DbgLv(1) << "CR:   scnx ss rr" << scnx << ss << rr;
               if (use_zsol) {
                  DbgLv(1) << "CR:     x y z" << sim_vals.zsolutes[ cc ].x * 1.0e+13 << sim_vals.zsolutes[ cc ].y
                           << sim_vals.zsolutes[ cc ].z << "sval" << soluval << "idat sdat" << idata->value(ss, rr)
                           << sdata->value(ss, rr);
                  if (soluval > 100.0) {
                     double drval = 0.0;
                     double dmax = 0.0;
                     double dsum = 0.0;
                     for (int ss = 0; ss < nscans; ss++) {
                        for (int rr = 0; rr < npoints; rr++) {
                           drval = idata->value(ss, rr);
                           dmax = qMax(dmax, drval);
                           dsum += drval;
                        }
                     }
                     DbgLv(1) << "CR:B x y" << sim_vals.zsolutes[ cc ].x * 1.0e+13 << sim_vals.zsolutes[ cc ].y
                              << "sval" << soluval << "amax asum" << dmax << dsum;
                  }
               }
               else {
                  DbgLv(1) << "CR:     s k v" << sim_vals.solutes[ cc ].s * 1.0e+13 << sim_vals.solutes[ cc ].k
                           << sim_vals.solutes[ cc ].v << "sval" << soluval << "idat sdat" << idata->value(ss, rr)
                           << sdata->value(ss, rr);
                  if (soluval > 100.0) {
                     double drval = 0.0;
                     double dmax = 0.0;
                     double dsum = 0.0;
                     for (int ss = 0; ss < nscans; ss++) {
                        for (int rr = 0; rr < npoints; rr++) {
                           drval = idata->value(ss, rr);
                           dmax = qMax(dmax, drval);
                           dsum += drval;
                        }
                     }
                     DbgLv(1) << "CR:B s k" << sim_vals.solutes[ cc ].s * 1.0e+13 << sim_vals.solutes[ cc ].k << "sval"
                              << soluval << "amax asum" << dmax << dsum;
                  }
               }
            }
            //*DEBUG*
         }
      }
   }

   double rmsds[ dataset_count ];
   int kntva[ dataset_count ];
   double variance = 0.0;
   int tinoffs = 0;
   int rinoffs = 0;
   int ktotal = 0;
   int scnx = 0;
   int kdsx = 0;
   US_DataIO::RawData *sdata = &sim_vals.sim_data;
   US_DataIO::RawData *resid = &sim_vals.residuals;
   sim_vals.residuals = sim_vals.sim_data;
   DbgLv(2) << "   CR:301  rss now" << US_Memory::rss_now() << "thrn" << thrnrank;

   // Calculate residuals and rmsd values
   for (int ee = offset; ee < lim_offs; ee++, kdsx++) {
      DataSet *dset = data_sets[ ee ];
      US_DataIO::EditedData *edata = banddthr ? &wdata : &dset->run_data;
      int npoints = edata->pointCount();
      int nscans = edata->scanCount();
      int kntcs = 0;
      double varidset = 0.0;

      for (int ss = 0; ss < nscans; ss++, scnx++) { // Create residuals dset:  exp - sim - noise(s)

         for (int rr = 0; rr < npoints; rr++) {
            double resval = edata->value(ss, rr) - sdata->value(scnx, rr) - tinvec[ rr + tinoffs ]
                            - rinvec[ ss + rinoffs ];
            resid->setValue(scnx, rr, resval);

            double r2 = sq(resval);
            variance += r2;
            varidset += r2;
            kntcs++;
         }
      }

      rmsds[ kdsx ] = varidset; // Variance for a data set
      kntva[ kdsx ] = kntcs;
      ktotal += kntcs;
      tinoffs += npoints;
      rinoffs += nscans;
      DbgLv(1) << "CR: kdsx variance" << kdsx << varidset << variance;
   }

   sim_vals.variances.resize(dataset_count);
   variance /= ( double ) ktotal; // Total sets variance
   sim_vals.variance = variance;
   int kk = 0;

   for (int ee = offset; ee < lim_offs; ee++) { // Scale variances for each data set
      sim_vals.variances[ kk ] = rmsds[ kk ] / ( double ) (kntva[ kk ]);
      DbgLv(1) << "CR:     kk variance" << sim_vals.variances[ kk ];
      kk++;
   }

   // Store solutes for return
   kk = 0;

   if (use_zsol) { // Use xyZ type solutes
      for (int cc = 0; cc < nsolutes; cc++) {
         if (nnls_x[ cc ] > 0.0) { // Store solutes with non-zero concentrations
            sim_vals.zsolutes[ cc ].c = nnls_x[ cc ];
            sim_vals.zsolutes[ kk++ ] = sim_vals.zsolutes[ cc ];
         }
      }
   }
   else { // Use old type solutes
      for (int cc = 0; cc < nsolutes; cc++) {
         if (nnls_x[ cc ] > 0.0) { // Store solutes with non-zero concentrations
            sim_vals.solutes[ cc ].c = nnls_x[ cc ];
            sim_vals.solutes[ kk++ ] = sim_vals.solutes[ cc ];
         }
      }
   }
   DbgLv(2) << "   CR:310  rss now" << US_Memory::rss_now() << "thrn" << thrnrank;

   // Truncate solutes at non-zero count
   DbgLv(1) << "CR: out solutes size" << kk;

   if (use_zsol) {
      sim_vals.zsolutes.resize(qMax(kk, 1));
      DbgLv(1) << "CR:   jj solute-c" << 0 << sim_vals.zsolutes[ 0 ].c;
      DbgLv(1) << "CR:   jj solute-c" << 1 << (kk > 1 ? sim_vals.zsolutes[ 1 ].c : 0.0);
      DbgLv(1) << "CR:   jj solute-c" << kk - 2 << (kk > 1 ? sim_vals.zsolutes[ kk - 2 ].c : 0.0);
      DbgLv(1) << "CR:   jj solute-c" << kk - 1 << (kk > 0 ? sim_vals.zsolutes[ kk - 1 ].c : 0.0);
   }
   else {
      sim_vals.solutes.resize(qMax(kk, 1));
      DbgLv(1) << "CR:   jj solute-c" << 0 << sim_vals.solutes[ 0 ].c;
      DbgLv(1) << "CR:   jj solute-c" << 1 << (kk > 1 ? sim_vals.solutes[ 1 ].c : 0.0);
      DbgLv(1) << "CR:   jj solute-c" << kk - 2 << (kk > 1 ? sim_vals.solutes[ kk - 2 ].c : 0.0);
      DbgLv(1) << "CR:   jj solute-c" << kk - 1 << (kk > 0 ? sim_vals.solutes[ kk - 1 ].c : 0.0);
   }
   if (abort)
      return;
   //double sum3 = 0.0;
   // Fill noise objects with any calculated vectors
   if (calc_ti) {
      sim_vals.ti_noise << tinvec;
      //double sum2 = 0.0,nnls_ti_norm = 0.0;
      //for ( int i = 0; i <  sim_vals.ti_noise.size(); i++ )
      //    sum2 +=  pow ( sim_vals.ti_noise[i],2.0 );
      //sum3 = sqrt ( sum2 );
      //nnls_ti_norm +=  sum3;
      //DbgLv(1)<<"nnls_ti_norm" <<  nnls_ti_norm;
   }

   if (calc_ri) {
      sim_vals.ri_noise << rinvec;
      //double sum3 = 0.0,nnls_ri_norm;
      //for ( int i = 0; i <  sim_vals.ri_noise.size(); i++ )
      //    sum3 +=  pow ( sim_vals.ri_noise[i],2.0 );
      //nnls_ri_norm = sqrt ( sum3 );
      //DbgLv(1)<<"nnls_ri_norm" <<  nnls_ri_norm;
   }

   // Compute and return the xnorm-squared value of concentrations
   nsolutes = (use_zsol) ? sim_vals.zsolutes.size() : sim_vals.solutes.size();
   DbgLv(1) << "CR:     nsolutes" << nsolutes;
   double xnorm = 0.0;
   for (int jj = 0; jj < nsolutes; jj++) {
      double cval = use_zsol ? sim_vals.zsolutes[ jj ].c : sim_vals.solutes[ jj ].c;
      xnorm += sq(cval);
   }
   DbgLv(2) << "   CR:320  rss now" << US_Memory::rss_now() << "thrn" << thrnrank;

   sim_vals.xnormsq = xnorm;
   DbgLv(1) << "CR:       xnormsq" << xnorm;

   DbgLv(1) << "ASave=" << ASave << "BSave=" << BSave;

   // If specified, return the A and B matrices (with or without padding)
   if (ASave != NULL && BSave != NULL) {
      DbgLv(1) << "entering for saving";
      if (padAB) { // Return A and B with padding for regularization
         if (tikreg) { // If regularization was used, return A and B with padding intact
            (*ASave) = sv_nnls_a;
            (*BSave) = sv_nnls_b;
         }

         else { // If no regularization, return A and B with padding added
            ASave->clear();
            BSave->clear();
            int kk = 0;

            for (int cc = 0; cc < nsolutes; cc++) { // Save and pad A matrix, a column at a time
               for (int jj = 0; jj < ntotal; jj++) { // Save an A column
                  (*ASave) << sv_nnls_a[ kk++ ];
               }

               for (int jj = 0; jj < nsolutes; jj++) {
                  (*ASave) << 0.0; // Pad an A column (zeroes for now)
               }
            }

            for (int jj = 0; jj < ntotal; jj++) { // Save the B matrix (vector)
               (*BSave) << sv_nnls_b[ jj ];
            }

            for (int cc = 0; cc < nsolutes; cc++) {
               (*BSave) << 0.0; // Pad the B vector with zeroes
            }
         }

         // If we are doing padding, it is in preparation for regularization.
         // So, if any noise was calculated, it should be added to the
         // B vector.
         if (calc_ri || calc_ti) {
            int nscans = data_sets[ 0 ]->run_data.scanCount();
            int npoints = data_sets[ 0 ]->run_data.pointCount();
            int kk = 0;

            for (int ss = 0; ss < nscans; ss++) {
               double rinoi = calc_ri ? rinvec[ ss ] : 0.0;

               for (int rr = 0; rr < npoints; rr++) {
                  double tinoi = calc_ti ? tinvec[ rr ] : 0.0;
                  (*BSave)[ kk++ ] += (tinoi + rinoi);
               }
            }
         }
      }

      else { // Return A and B without any regularization padding
         if (tikreg) { // If regularization was used, remove padding
            ASave->clear();
            BSave->clear();
            int kk = 0;

            for (int cc = 0; cc < nsolutes; cc++) {
               for (int jj = 0; jj < ntotal; jj++) { // Save an A column
                  (*ASave) << sv_nnls_a[ kk++ ];
               }

               kk += nsolutes; // Bump past regularization padding
            }

            for (int jj = 0; jj < ntotal; jj++) { // Save the B matrix (vector)
               (*BSave) << sv_nnls_b[ jj ];
            }
         }

         else { // If no regularization, return A and B as they are
            (*ASave) = sv_nnls_a;
            (*BSave) = sv_nnls_b;
            DbgLv(1) << "CR: ASv: sv_nnls_a size" << sv_nnls_a.size() << ASave->size();
         }
      }
   }
   DbgLv(2) << "   CR:777  rss now" << US_Memory::rss_now() << "thrn" << thrnrank;

   DebugTime("END:calcres");

   return;
}

// Set abort flag
void US_SolveSim::abort_work() {
   abort = true;
}

// Compute a_tilde, the average experiment signal at each time
void US_SolveSim::compute_a_tilde(QVector<double> &a_tilde, const QVector<double> &nnls_b) {
   US_DataIO::EditedData *edata = &data_sets[ d_offs ]->run_data;
   int npoints = edata->pointCount();
   int nscans = edata->scanCount();
   int jb = 0;
   double avgscale = 1.0 / ( double ) npoints;

   for (int ss = 0; ss < nscans; ss++) {
      for (int rr = 0; rr < npoints; rr++)
         a_tilde[ ss ] += nnls_b[ jb++ ];

      a_tilde[ ss ] *= avgscale;
   }
}

// Compute L_tildes, the average signal at each radius
void US_SolveSim::compute_L_tildes(int nrinois, int nsolutes, QVector<double> &L_tildes, const QVector<double> &nnls_a) {
   US_DataIO::EditedData *edata = &data_sets[ d_offs ]->run_data;
   int npoints = edata->pointCount();
   int nscans = edata->scanCount();
   double avgscale = 1.0 / ( double ) npoints;
   int a_index = 0;

   for (int cc = 0; cc < nsolutes; cc++) {
      int t_index = cc * nrinois;

      for (int ss = 0; ss < nscans; ss++) {
         for (int rr = 0; rr < npoints; rr++)
            L_tildes[ t_index ] += nnls_a[ a_index++ ];

         L_tildes[ t_index++ ] *= avgscale;
      }
   }
}

// Compute L_tilde, the average model signal at each radius
void US_SolveSim::compute_L_tilde(QVector<double> &L_tilde, const QVector<double> &L) {
   US_DataIO::EditedData *edata = &data_sets[ d_offs ]->run_data;
   int npoints = edata->pointCount();
   int nscans = edata->scanCount();
   double avgscale = 1.0 / ( double ) npoints;
   int index = 0;

   for (int ss = 0; ss < nscans; ss++) {
      for (int rr = 0; rr < npoints; rr++)
         L_tilde[ ss ] += L[ index++ ];

      L_tilde[ ss ] *= avgscale;
   }
}

void US_SolveSim::compute_L(
   int ntotal, int nsolutes, QVector<double> &L, const QVector<double> &nnls_a, const QVector<double> &nnls_x) {
   US_DataIO::EditedData *edata = &data_sets[ d_offs ]->run_data;
   int npoints = edata->pointCount();
   int nscans = edata->scanCount();

   for (int cc = 0; cc < nsolutes; cc++) {
      double concentration = nnls_x[ cc ];

      if (concentration > 0.0) {
         int r_index = cc * ntotal;
         int count = 0;

         for (int ss = 0; ss < nscans; ss++) {
            for (int rr = 0; rr < npoints; rr++) {
               L[ count++ ] += (concentration * nnls_a[ r_index++ ]);
            }
         }
      }
   }
}

void US_SolveSim::ri_small_a_and_b(
   int nsolutes, int ntotal, int nrinois, QVector<double> &small_a, QVector<double> &small_b,
   const QVector<double> &a_tilde, const QVector<double> &L_tildes, const QVector<double> &nnls_a,
   const QVector<double> &nnls_b) {
   DebugTime("BEG:ri_smab");
   US_DataIO::EditedData *edata = &data_sets[ d_offs ]->run_data;
   int npoints = edata->pointCount();
   int nscans = edata->scanCount();
   int kstodo = sq(nsolutes) / 10; // progress steps to report
   int incprg = nsolutes / 20; // increment between reports
   incprg = max(incprg, 1);
   incprg = min(incprg, 10);
   int jsols = qMax(1, nsolutes);
   int jstprg = (kstodo * incprg) / jsols; // steps for each report
   int kstep = 0; // progress counter

   for (int cc = 0; cc < nsolutes; cc++) {
      int jsa2 = cc * ntotal;
      int jst2 = cc * nrinois;
      int jjna = jsa2;
      int jjnb = 0;
      int jjlt = jst2;
      double sum_b = small_b[ cc ];

      for (int ss = 0; ss < nscans; ss++) {
         // small_b[ cc ] +=
         //    ( edata->value( ss, rr ) - a_tilde[ ss ] )
         //    *
         //    ( nnls_a[ cc * ntotal + ss * npoints + rr ]
         //      -
         //      L_tildes[ cc * nrinois + ss ] );
         double atil = a_tilde[ ss ];
         double Ltil = L_tildes[ jjlt++ ];

         for (int rr = 0; rr < npoints; rr++) {
            sum_b += ((nnls_b[ jjnb++ ] - atil) * (nnls_a[ jjna++ ] - Ltil));
         }
      }

      small_b[ cc ] = sum_b;

      for (int kk = 0; kk < nsolutes; kk++) {
         //small_a[ kk * nsolutes + cc ] +=
         //   ( nnls_a[ kk * ntotal + ss * npoints + rr ]
         //     -
         //     L_tildes[ kk * nrinois + ss  ]
         //   )
         //   *
         //   ( nnls_a[ cc * ntotal + ss * npoints + rr ]
         //     -
         //     L_tildes[ cc * nrinois + ss ] );
         int jjma = kk * nsolutes + cc;
         int jja1 = kk * ntotal;
         int jja2 = jsa2;
         int jjt1 = kk * nrinois;
         int jjt2 = jst2;
         double sum_a = small_a[ jjma ];

         for (int ss = 0; ss < nscans; ss++) {
            double Ltil1 = L_tildes[ jjt1++ ];
            double Ltil2 = L_tildes[ jjt2++ ];

            for (int rr = 0; rr < npoints; rr++) {
               sum_a += ((nnls_a[ jja1++ ] - Ltil1) * (nnls_a[ jja2++ ] - Ltil2));
            }
         }

         small_a[ jjma ] = sum_a;
      }

      if (signal_wanted && ++kstep == incprg) {
         emit work_progress(jstprg);
         kstodo -= jstprg;
         kstep = 0;
      }

      if (abort)
         return;
   }

   if (signal_wanted && kstodo > 0)
      emit work_progress(kstodo);
   DebugTime("END:ri_smab");
}

void US_SolveSim::ti_small_a_and_b(
   int nsolutes, int ntotal, int ntinois, QVector<double> &small_a, QVector<double> &small_b,
   const QVector<double> &a_bar, const QVector<double> &L_bars, const QVector<double> &nnls_a,
   const QVector<double> &nnls_b) {
   DebugTime("BEG:ti-smab");
   US_DataIO::EditedData *edata = &data_sets[ d_offs ]->run_data;
   int npoints = edata->pointCount();
   int nscans = edata->scanCount();
   int kstodo = sq(nsolutes) / 10; // progress steps to report
   int incprg = nsolutes / 20; // increment between reports
   incprg = max(incprg, 1);
   incprg = min(incprg, 10);

   DbgLv(1) << "ti_small_a_and_b: nsolutes=" << nsolutes;

   int jsols = qMax(1, nsolutes);
   int jstprg = (kstodo * incprg) / jsols; // steps for each report
   int kstep = 0; // progress counter
   //DbgLv(1)<< "ti_small_ : np ns nn nso" << npoints << nscans << ntinois << nsolutes
   // << "szb sza" << nnls_b.size() << nnls_a.size() << "nto" << ntotal;

   //int svsa=0;
   small_a.fill(0.0);
   small_b.fill(0.0);

   for (int cc = 0; cc < nsolutes; cc++) {
      int jjsa = cc;
      int jssa = cc * ntotal;
      int jssb = cc * ntinois;
      int jjna = jssa;
      int jjnb = 0;

      //small_b[ cc ] +=
      //   ( edata->value( ss, rr ) - a_bar[ rr ] )
      //     *
      //   ( nnls_a[ cc * ntotal + ss * npoints + rr ]
      //     -
      //     L_bars[ cc * ntinois + rr ] );
      double sum_b = small_b[ cc ];

      for (int ss = 0; ss < nscans; ss++) {
         int jjlb = jssb;

         for (int rr = 0; rr < npoints; rr++) {
            sum_b += ((nnls_b[ jjnb++ ] - a_bar[ rr ]) * (nnls_a[ jjna++ ] - L_bars[ jjlb++ ]));
         }
      }

      small_b[ cc ] = sum_b;

      //small_a[ kk * nsolutes + cc ] +=
      //   ( nnls_a[ kk * ntotal  + ss * npoints + rr ]
      //     -
      //     L_bars[ kk * ntinois + rr ] )
      //   *
      //   ( nnls_a[ cc * ntotal  + ss * npoints + rr ]
      //     -
      //     L_bars[ cc * ntinois + rr ] );
      for (int kk = 0; kk < nsolutes; kk++) {
         int jjna1 = kk * ntotal;
         int jjna2 = jssa;
         int jslb1 = kk * ntinois;
         int jslb2 = jssb;
         double sum_a = small_a[ jjsa ];

         for (int ss = 0; ss < nscans; ss++) {
            int jjlb1 = jslb1;
            int jjlb2 = jslb2;

            for (int rr = 0; rr < npoints; rr++) {
               sum_a += ((nnls_a[ jjna1++ ] - L_bars[ jjlb1++ ]) * (nnls_a[ jjna2++ ] - L_bars[ jjlb2++ ]));
            }
         }

         small_a[ jjsa ] = sum_a;
         //svsa=jjsa;
         jjsa += nsolutes;
      }

      if (signal_wanted && ++kstep == incprg) {
         emit work_progress(jstprg);
         kstodo -= jstprg;
         kstep = 0;
      }

      if (abort)
         return;
   }

   //DbgLv(1)<< "ti_small_:   nsb nsa" << small_b.size() << small_a.size()
   // << "jsb jsa" << nsolutes << svsa << "a0 an b0 bn"
   // << small_a[0] << small_a[svsa] << small_b[0] << small_b[nsolutes-1];

   if (signal_wanted && kstodo > 0)
      emit work_progress(kstodo);

   DebugTime("END:ti-smab");
}

void US_SolveSim::compute_L_bar(QVector<double> &L_bar, const QVector<double> &L, const QVector<double> &L_tilde) {
   US_DataIO::EditedData *edata = &data_sets[ d_offs ]->run_data;
   int npoints = edata->pointCount();
   int nscans = edata->scanCount();
   double avgscale = 1.0 / ( double ) nscans;

   for (int rr = 0; rr < npoints; rr++) {
      // Note  L_tilde is always zero when rinoise has not been requested
      for (int ss = 0; ss < nscans; ss++)
         L_bar[ rr ] += (L[ ss * npoints + rr ] - L_tilde[ ss ]);

      L_bar[ rr ] *= avgscale;
   }
}

// Calculate the average measured concentration at each radius point
void US_SolveSim::compute_a_bar(QVector<double> &a_bar, const QVector<double> &a_tilde, const QVector<double> &nnls_b) {
   US_DataIO::EditedData *edata = &data_sets[ d_offs ]->run_data;
   int npoints = edata->pointCount();
   int nscans = edata->scanCount();
   double avgscale = 1.0 / ( double ) nscans;

   for (int rr = 0; rr < npoints; rr++) {
      int jb = rr;

      // Note: a_tilde is always zero when rinoise has not been requested
      for (int ss = 0; ss < nscans; ss++) {
         a_bar[ rr ] += (nnls_b[ jb ] - a_tilde[ ss ]);
         jb += npoints;
      }

      a_bar[ rr ] *= avgscale;
   }
}

// Calculate the average simulated concentration at each radius point
void US_SolveSim::compute_L_bars(
   int nsolutes, int nrinois, int ntinois, int ntotal, QVector<double> &L_bars, const QVector<double> &nnls_a,
   const QVector<double> &L_tildes) {
   US_DataIO::EditedData *edata = &data_sets[ d_offs ]->run_data;
   int npoints = edata->pointCount();
   int nscans = edata->scanCount();
   double avgscale = 1.0 / ( double ) nscans;

   for (int cc = 0; cc < nsolutes; cc++) {
      int solute_offset = cc * ntotal;

      for (int rr = 0; rr < npoints; rr++) {
         int r_index = cc * ntinois + rr;

         for (int ss = 0; ss < nscans; ss++) {
            // Note: L_tildes is always zero when rinoise has not been
            // requested

            int n_index = solute_offset + ss * npoints + rr;
            int s_index = cc * nrinois + ss;

            L_bars[ r_index ] += (nnls_a[ n_index ] - L_tildes[ s_index ]);
         }

         L_bars[ r_index ] *= avgscale;
      }
   }
}

// Debug message with thread/processor number and elapsed time value
void US_SolveSim::DebugTime(QString mtext) {
   if (dbg_timing) {
      qDebug() << "w" << thrnrank << "TM:" << mtext << startCalc.msecsTo(QDateTime::currentDateTime()) / 1000.0;
   }
}

// Modify amplitude of data by thresholds and return flag if all-zero result
bool US_SolveSim::data_threshold(
   US_DataIO::RawData *sdata, double zerothr, double linethr, double maxod, double mfactor) {
   int nnzro = 0;
   int nzset = 0;
   int nntrp = 0;
   int nclip = 0;
   int npoints = sdata->pointCount();
   int nscans = sdata->scanCount();
   double clipout = mfactor * maxod;
   double thrfact = mfactor / ( double ) (linethr - zerothr);
   double maxs = 0.0;
   double maxsi = 0.0;

   for (int ss = 0; ss < nscans; ss++) {
      for (int rr = 0; rr < npoints; rr++) {
         double avalue = sdata->value(ss, rr);
         maxsi = qMax(maxsi, avalue);

         if (avalue < zerothr) { // Less than zero threshold:  set to zero
            avalue = 0.0;
            nzset++;
         }

         else if (avalue < linethr) { // Between zero and linear threshold:  set to interpolated value
            avalue *= ((avalue - zerothr) * thrfact);
            nntrp++;
         }

         else if (avalue < maxod) { // Under maximum OD:  set to factor times input
            avalue *= mfactor;
         }

         else { // Over maximum OD;  set to factor times maximum
            avalue = clipout;
            nclip++;
         }

         if (avalue != 0.0)
            nnzro++;
         maxs = qMax(maxs, avalue);

         sdata->setValue(ss, rr, avalue);
      }
   }

   int lownnz = qRound(minnzsc * ( double ) (nscans * npoints));
   nnzro = (nnzro < lownnz) ? 0 : nnzro;
   DbgLv(1) << "  CR:THR: nnzro zs nt cl" << nnzro << nzset << nntrp << nclip;
   //if(nnzro>0) {DbgLv(1) << "CR:THR: maxs" << maxs << maxsi << "mfact" << mfactor;}
   //else        {DbgLv(1) << "CR:THz: maxs" << maxs << nnzro << "mfact" << mfactor;}

   return (nnzro == 0);
}

// Modify amplitude by thresholds and flag if all-zero (for experiment data)
bool US_SolveSim::data_threshold(
   US_DataIO::EditedData *edata, double zerothr, double linethr, double maxod, double mfactor) {
   int nnzro = 0;
   int npoints = edata->pointCount();
   int nscans = edata->scanCount();
   double clipout = mfactor * maxod;
   double thrfact = mfactor / ( double ) (linethr - zerothr);

   for (int ss = 0; ss < nscans; ss++) {
      for (int rr = 0; rr < npoints; rr++) {
         double avalue = edata->value(ss, rr);

         if (avalue < zerothr) { // Less than zero threshold:  set to zero
            avalue = 0.0;
         }

         else if (avalue < linethr) { // Between zero and linear threshold:  set to interpolated value
            avalue *= ((avalue - zerothr) * thrfact);
         }

         else if (avalue < maxod) { // Under maximum OD:  set to factor times input
            avalue *= mfactor;
         }

         else { // Over maximum OD;  set to factor times maximum
            avalue = clipout;
         }

         if (avalue != 0.0)
            nnzro++;

         edata->setValue(ss, rr, avalue);
      }
   }

   return (nnzro == 0);
}

// Set a model component attribute value
void US_SolveSim::set_comp_attr(US_Model::SimulationComponent &component, US_Solute &solute, int attr_type) {
   switch (attr_type) {
      default:
      case ATTR_S: // Sedimentation Coefficient
         component.s = solute.s;
         break;
      case ATTR_K: // Frictional Ratio
         component.f_f0 = solute.k;
         break;
      case ATTR_W: // Molecular Weight
         component.mw = solute.d;
         break;
      case ATTR_V: // Partial Specific Volume (vbar)
         component.vbar20 = solute.v;
         break;
      case ATTR_D: // Diffusion Coefficient
         component.D = solute.d;
         break;
      case ATTR_F: // Frictional Coefficient
         component.f = solute.d;
         break;
   }
}

//-----------------------------------------
// Subroutine to find angle between vectors
//-----------------------------------------
double US_SolveSim::angle_vectors(QVector<double> &v1, QVector<double> &v2, int nn) {
   double angle = 0.0;
   double sum1 = 0.0;
   double sum2 = 0.0;
   double sum3 = 0.0;

   for (int ii = 0; ii < nn; ii++) {
      sum1 += (v1[ ii ] * v2[ ii ]);
      sum2 += (v1[ ii ] * v1[ ii ]);
      sum3 += (v2[ ii ] * v2[ ii ]);
   }

   sum1 = (sum1 != 0.0) ? sum1 : 1e-20;
   sum2 = (sum2 != 0.0) ? sum2 : 1e-20;
   angle = acos(sum1 / (sqrt(sum2) * sqrt(sum3)));
   DbgLv(1) << "angle between two vectors: " << angle << "sum1=" << sum1 << "sum2=" << sum2 << "sum3=" << sum3;
   return angle;
}

double US_SolveSim::angle_vectors(double *v1, double *v2, int nn) {
   double angle = 0.0;
   double sum1 = 0.0;
   double sum2 = 0.0;
   double sum3 = 0.0;

   for (int ii = 0; ii < nn; ii++) {
      sum1 += (v1[ ii ] * v2[ ii ]);
      sum2 += (v1[ ii ] * v1[ ii ]);
      sum3 += (v2[ ii ] * v2[ ii ]);
   }

   sum1 = (sum1 != 0.0) ? sum1 : 1e-20;
   sum2 = (sum2 != 0.0) ? sum2 : 1e-20;
   angle = acos(sum1 / (sqrt(sum2) * sqrt(sum3)));
   DbgLv(1) << "angle between two vectors: " << angle << "sum1=" << sum1 << "sum2=" << sum2 << "sum3=" << sum3;
   return angle;
}
