//! \file us_2dsa_process.cpp
#include "us_2dsa_process.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_model.h"
#include "us_sleep.h"

// Class to process 2DSA simulations
US_2dsaProcess::US_2dsaProcess( US_DataIO2::EditedData* da_exper,
      QObject* parent /*=0*/ ) : QObject( parent )
{
   edata            = da_exper;
   parentw          = parent;
   dbg_level        = US_Settings::us_debug();
}

// Start a specified 2DSA fit run
void US_2dsaProcess::start_fit( double sll, double sul, int nss,
   double kll, double kul, int nks, int ngr, int nthr, int noif )
{
DbgLv(1) << "2P(2dsaProc): start_fit()";
   slolim      = sll;
   suplim      = sul;
   nssteps     = nss;
   klolim      = kll;
   kuplim      = kul;
   nksteps     = nks;
   ngrefine    = ngr;
   nthreads    = nthr;
   noisflag    = noif;
   errMsg      = tr( "NO ERROR: start" );

DbgLv(1) << "2P: sll sul nss" << slolim << suplim << nssteps
 << " kll kul nks" << klolim << kuplim << nksteps
 << " ngref nthr noif" << ngrefine << nthreads << noisflag;

   nscans      = edata->scanData.size();
   npoints     = edata->x.size();
   gdelta_s    = ( suplim - slolim ) / (double)( nssteps - 1 );
   gdelta_k    = ( kuplim - klolim ) / (double)( nksteps - 1 );
   sdelta_s    = gdelta_s * (double)ngrefine;
   sdelta_k    = gdelta_k * (double)ngrefine;

   nsubgrid    = ngrefine * ngrefine;
   ntpsteps    = nksteps * nssteps * nsubgrid;
   kcpsteps    = 0;
   kctask      = 0;
   kstask      = 0;
   nthreads    = ( nthreads < nsubgrid ) ? nthreads : nsubgrid;
DbgLv(1) << "2P:   nscans npoints" << nscans << npoints << " gdlts gdltk"
 << gdelta_s << gdelta_k << " sdlts sdltk" << sdelta_s << sdelta_k;
DbgLv(1) << "2P:   nsubgrid ntpsteps nthreads"
 << nsubgrid << ntpsteps << nthreads;

   int ktask   = 0;
   int kthr    = 0;

   for ( int ii = 0; ii < nthreads; ii++ )
   {
      wthreads << new WorkerThread( this );
   }

   double vals = slolim;

   for ( int ii = 0; ii < ngrefine; ii++ )
   {
      double valk = klolim;

      for ( int jj = 0; jj < ngrefine; jj++ )
      {
         WorkDefine wdef;
         wdef.ll_s      = vals;
         wdef.ul_s      = suplim;
         wdef.delta_s   = sdelta_s;
         wdef.ll_k      = valk;
         wdef.ul_k      = kuplim;
         wdef.delta_k   = sdelta_k;
         wdef.thrx      = kthr;
         wdef.taskx     = ++ktask;
         wdef.edata     = edata;

         valk          += gdelta_k;
         workdefs << wdef;
      }

      vals     += gdelta_s;
   }

   // start the first threads
   for ( int ii = 0; ii < nthreads; ii++ )
   {
      WorkDefine*   wdef = &workdefs[ ii ];
      WorkerThread* wthr = wthreads[ ii ];
      wdef->thrx  = ii + 1;
      wthr->define_work( *wdef );
      wthr->start();

      connect( wthr, SIGNAL( work_complete( WorkerThread* ) ),
               this, SLOT( thread_finished( WorkerThread* ) ) );
      connect( wthr, SIGNAL( work_progress( int           ) ),
               this, SLOT( step_progress  ( int           ) ) );
   }

   kstask = nthreads;

}

void US_2dsaProcess::step_progress( int ksteps )
{
   emit progress_update( ksteps );
}

void US_2dsaProcess::thread_finished( WorkerThread* wthrd )
{
   WorkResult wresult;

   wthrd->get_result( wresult );  // get results of thread task
   int thrx   = wresult.thrx;     // thread index of task
   int taskx  = wresult.taskx;    // task index of task

   kctask++;                      // bump count of completed tasks (subgrids)
DbgLv(1) << "THR_FIN: thrx" << thrx << " taskx" << taskx << " ll_s ll_k"
 << wresult.ll_s << wresult.ll_k << " kct kst" << kctask << kstask;
   int tx = thrx - 1;             // get index into thread list
   delete wthreads[ tx ];         // destroy thread

   emit refine_complete( kctask ); 
   QString pmsg = tr( "Computations for %1 of %2 subgrids are complete" )
      .arg( kctask ).arg( nsubgrid );
   emit message_update( pmsg );

   if ( kctask >= nsubgrid )
   {  // all subgrids computed
      emit subgrids_complete();
      US_Sleep::sleep( 1L );
      emit process_complete();
      return;
   }

   if ( kstask > ( nsubgrid - 1 ) )
   {  // no more tasks need to be started
      return;
   }

   // get next task definition and create new thread
   WorkDefine*   wdef = &workdefs[ kstask ];
   WorkerThread* wthr = new WorkerThread( this );

   wthreads[ tx ]     = wthr;      // set up next thread
   wdef->thrx         = thrx;      // define its index (same as one just done)
   wthr->define_work( *wdef );     // define the work
   wthr->start();                  // start a new worker thread
   kstask++;                       // bump count of started worker threads

   connect( wthr, SIGNAL( work_complete( WorkerThread* ) ),
            this, SLOT( thread_finished( WorkerThread* ) ) );
   connect( wthr, SIGNAL( work_progress( int           ) ),
            this, SLOT( step_progress  ( int           ) ) );

}

// Get results upon completion of all refinements
bool US_2dsaProcess::get_results( US_DataIO2::RawData* da_sim,
   US_DataIO2::RawData* da_res, US_Model* da_mdl, US_Noise* da_tin,
   US_Noise* da_rin )
{
   bool all_ok = true;

   *da_sim     = sdata;
   *da_res     = rdata;
   *da_mdl     = model;

   if ( ( noisflag & 1 ) != 0  &&  da_tin != 0 )
      *da_tin     = ti_noise;

   if ( ( noisflag & 2 ) != 0  &&  da_rin != 0 )
      *da_rin     = ri_noise;

   return all_ok;
}

// Find model filename matching a given GUID
QString US_2dsaProcess::get_model_filename( QString guid )
{
   QString fname = "";
   QString path;

   if ( ! US_Model::model_path( path ) )
      return fname;

   QDir f( path );
   QStringList filter( "M???????.xml" );
   QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );
   f_names.sort();

   int         nnames  = f_names.size();
   int         newnum  = nnames + 1;
   bool        found   = false;

   for ( int ii = 0; ii < nnames; ii++ )
   {
      QString fn = f_names[ ii ];
      int     kf = fn.mid( 1, 7 ).toInt() - 1;  // expected index in file name
      fn         = path + "/" + fn;             // full path file name

      if ( kf != ii  &&  newnum > nnames )
         newnum     = kf;                       // 1st opened number slot

      QFile m_file( fn );

      if ( ! m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

      QXmlStreamReader xml( &m_file );

      while ( !xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "model" )
         {
            QXmlStreamAttributes a = xml.attributes();

            if ( a.value( "modelGUID" ).toString() == guid )
            {
               fname    = fn;                   // name of file with match
               found    = true;                 // match to guid found
               break;
            }
         }

      }

      m_file.close();

      if ( found )
         break;
   }

 
   // if no guid match found, create new file name with a numeric part from
   //   the first gap in the file list sequence or from count plus one
   if ( ! found )
      fname     = path + "/M" + QString().sprintf( "%07i", newnum ) + ".xml";

   return fname;
}

// find noise file name matching a given GUID
QString US_2dsaProcess::get_noise_filename( QString guid )
{
   QString fname = "";
   QString path  = US_Settings::dataDir() + "/noises";
   QDir    dir;

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) )
         return fname;
   }

   QDir f( path );
   QStringList filter( "N???????.xml" );
   QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );
   f_names.sort();

   int         nnames  = f_names.size();
   int         newnum  = nnames + 1;
   bool        found   = false;

   for ( int ii = 0; ii < nnames; ii++ )
   {
      QString fn = f_names[ ii ];
      int     kf = fn.mid( 1, 7 ).toInt() - 1;  // expected index in file name
      fn         = path + "/" + fn;             // full path file name

      if ( kf != ii  &&  newnum > nnames )
         newnum     = kf;                       // 1st opened number slot

      QFile m_file( fn );

      if ( ! m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

      QXmlStreamReader xml( &m_file );

      while ( !xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "noise" )
         {
            QXmlStreamAttributes a = xml.attributes();

            if ( a.value( "noiseGUID" ).toString() == guid )
            {
               fname    = fn;                   // name of file with match
               found    = true;                 // match to guid found
               break;
            }
         }

      }

      m_file.close();

      if ( found )
         break;
   }

 
   // if no guid match found, create new file name with a numeric part from
   //   the first gap in the file list sequence or from count plus one
   if ( ! found )
      fname     = path + "/N" + QString().sprintf( "%07i", newnum ) + ".xml";

   return fname;
}

WorkerThread::WorkerThread( QObject* parent )
   : QThread( parent )
{
   dbg_level        = US_Settings::us_debug();
//DbgLv(1) << "2P(WT): Thread created";
}

WorkerThread::~WorkerThread()
{
//DbgLv(1) << "2P(WT):  Thread destructor";
   //mutex.lock();
   //condition.wakeOne();
   //mutex.unlock();

   for ( int ii = 0; ii < solute_i.size(); ii++ )
      delete solute_i[ ii ];

   for ( int ii = 0; ii < solute_c.size(); ii++ )
      delete solute_c[ ii ];

   wait();
//DbgLv(1) << "2P(WT):   Thread destroyed";
}

void WorkerThread::define_work( WorkDefine& workin )
{
   llim_s      = workin.ll_s;
   ulim_s      = workin.ul_s;
   delta_s     = workin.delta_s;
   llim_k      = workin.ll_k;
   ulim_k      = workin.ul_k;
   delta_k     = workin.delta_k;
   thrx        = workin.thrx;
   taskx       = workin.taskx;
   ngls        = qRound( ( ulim_s - llim_s ) / delta_s ) + 1;
   nglk        = qRound( ( ulim_k - llim_k ) / delta_k ) + 1;
   edata       = workin.edata;

   for ( int ii = 0; ii < solute_i.size(); ii++ )
      delete solute_i[ ii ];

   solute_i.resize( ngls * nglk );

   double cval = 1.0;
   double sval = llim_s;
   int    kk   = 0;

   for ( int ii = 0; ii < ngls; ii++ )
   {
      double kval = llim_k;

      for ( int jj = 0; jj < nglk; jj++ )
      {
         Solute* soli = new Solute( sval, kval, cval );
         //DbgLv(1) << "ii jj s f/f0" << ii << jj << sval << kval;

         solute_i[ kk++ ] = soli;

         kval       += delta_k;
      }

      sval       += delta_s;
   }
}

void WorkerThread::get_result( WorkResult& workout )
{
   int ncsols = solute_c.size();
   workout.csolutes.clear();
   workout.thrx   = thrx;
   workout.taskx  = taskx;
   workout.ll_s   = llim_s;
   workout.ll_k   = llim_k;

   for ( int ii = 0; ii < ncsols; ii++ )
   {
      double  sval  = solute_c[ ii ]->s;
      double  kval  = solute_c[ ii ]->k;
      double  cval  = solute_c[ ii ]->c;
      Solute* soli = new Solute( sval, kval, cval );
      workout.csolutes << soli;
   }
}

void WorkerThread::run()
{
DbgLv(1) << "THR RUN: lls llk" << llim_s << llim_k;
   // set up for single-component model
   model.components.resize( 1 );
   US_Model::SimulationComponent zcomponent;
   zcomponent.s     = 0.0;
   zcomponent.D     = 0.0;
   zcomponent.mw    = 0.0;
   zcomponent.f     = 0.0;
   zcomponent.f_f0  = 0.0;
//DbgLv(1) << "  TR: simpars init";
//
   // populate simulation parameters based on experiment data
   US_SimulationParameters simpars;
//DbgLv(1) << "  TR:  nscan npts" << edata->scanData.size() << edata->x.size();
   simpars.initFromData( NULL, *edata );

   int ntstep       = qRound( ( ulim_s - llim_s ) / delta_s + 1.0 )
                    * qRound( ( ulim_k - llim_k ) / delta_k + 1.0 );
   int increp       = ntstep / 10;
       increp       = ( increp < 10 ) ? 10 : increp;
   int kstep        = 0;
   int lstep        = 0;

   // simulate data using models with single s,f/f0 component

   for ( double sval = llim_s; sval <= ulim_s; sval += delta_s )
   {
      for ( double kval = llim_k; kval <= ulim_k; kval += delta_k )
      {
         // set model with s,k point; update other coefficients
         model.components[ 0 ]      = zcomponent;
         model.components[ 0 ].s    = sval * 1.0e-13;
         model.components[ 0 ].f_f0 = kval;
         model.update_coefficients();
//DbgLv(1) << "  TR:   s k D" << sval << kval << model.components[0].D;

         // initialize simulation data with experiment grid
         US_AstfemMath::initSimData( sdata, *edata, 0.0 );

//DbgLv(1) << "  TR:     astfem_rsa calc";
         // calculate Astfem_RSA solution
         US_Astfem_RSA astfem_rsa( model, simpars );
         astfem_rsa.calculate( sdata );

         kstep++;

         if ( ( kstep % increp ) == 0 )
         {
            emit work_progress( increp );
            lstep = kstep;
         }
      }
   }
   emit work_progress( ntstep - lstep );

//DbgLv(1) << "  RUN call quit";
   quit();
//DbgLv(1) << "  RUN call exec";
   exec();
//DbgLv(1) << "  RUN return";

   emit work_complete( this );
}

