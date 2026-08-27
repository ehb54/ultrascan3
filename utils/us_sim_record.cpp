//! \file us_sim_record.cpp

#include "us_sim_record.h"
#include "us_settings.h"
#include "us_util.h"

#include <QUuid>

QString US_SimRecord::guid( const QString& seed, const QString& key )
{
   if ( seed.isEmpty() )
      return US_Util::new_guid();

   // A fixed namespace of our own, so these identities cannot collide with
   // version 5 UUIDs minted elsewhere from the same seed text.
   static const QUuid ns( QString( "{cb2a4256-450e-4202-8e7f-dcb34d66b142}" ) );

   return QUuid::createUuidV5( ns, ( seed + "/" + key ).toUtf8() )
          .toString().mid( 1, 36 );
}

US_Experiment US_SimRecord::experiment( const US_Rotor::Rotor&         rotor,
                                        const US_SimulationParameters& simparams,
                                        const QString&                 runID,
                                        const QString&                 runType,
                                        const QString&                 guidSeed )
{
   US_Experiment experiment;
   experiment.clear();

   // clear() has already set the investigator from settings, blanked every
   // GUID and date, and cleared the project, so only what a simulation
   // actually determines is set here.
   experiment.expGUID             = guid( guidSeed, "experiment." + runID );
   experiment.expType             = "velocity";
   experiment.runID               = runID;
   experiment.project.projectDesc = "Simulation";

   // US_Experiment::saveToDisk() writes the experiment XML before it saves the
   // project, so an unset GUID here is serialized empty and the project the
   // save then mints is one nothing references.  The key is not qualified by
   // runID: the speed-specific runs of one multi-speed channel are the same
   // simulated project.
   experiment.project.projectGUID = guid( guidSeed, "project" );

   // No real lab or instrument stands behind a simulated run; these are the
   // first row of each, which is what a fresh LIMS instance seeds.
   experiment.labID               = 1;
   experiment.instrumentID        = 1;

   experiment.rotorID             = rotor.ID;
   experiment.rotorGUID           = rotor.GUID;
   experiment.rotorSerial         = rotor.serialNumber;
   experiment.rotorName           = rotor.name;
   experiment.calibrationID       = simparams.rotorCalID.toInt();
   experiment.rotorCoeff1         = simparams.rotorcoeffs[ 0 ];
   experiment.rotorCoeff2         = simparams.rotorcoeffs[ 1 ];

   // A simulated run has no real calibration. Fixed rather than taken from
   // the clock, so re-running the same inputs changes only the GUIDs.
   experiment.rotorUpdated        = QDate( 2019, 1, 1 );

   experiment.opticalSystem       = runType.toLatin1();
   experiment.runTemp             = QString::number( simparams.temperature );
   experiment.label               = runID;
   experiment.comments            = "Auto exported";

   return experiment;
}

US_Solution US_SimRecord::solution( const QList< US_Model >& models,
                                    const QList< double >&   wavelengths,
                                    const US_Buffer&         buffer )
{
   US_Solution solution;

   if ( models.isEmpty()  ||  models.size() != wavelengths.size() )
      return solution;

   const US_Model& first = models[ 0 ];
   QString         desc  = "";

   solution.buffer = buffer;
   solution.analyteInfo.clear();

   for ( int ic = 0; ic < first.components.size(); ic++ )
   {
      const US_Model::SimulationComponent& comp = first.components[ ic ];

      US_Analyte analyte;
      analyte.extinction.clear();
      analyte.type        = static_cast< US_Analyte::analyte_t >( comp.analyte_type );
      analyte.vbar20      = comp.vbar20;
      analyte.mw          = comp.mw;
      analyte.description = comp.name;
      analyte.analyteGUID = comp.analyteGUID;
      // analyte.sequence has no corresponding SimulationComponent property
      analyte.grad_form   = ( first.coSedSolute == ic );

      // One entry per model, so a run of several wavelengths yields a
      // spectrum and a run of one yields a single reading.
      for ( int jm = 0; jm < models.size(); jm++ )
      {
         if ( ic >= models[ jm ].components.size() )
            continue;

         analyte.extinction[ wavelengths[ jm ] ]
            = models[ jm ].components[ ic ].extinction;
      }

      US_Solution::AnalyteInfo analyteInfo;
      analyteInfo.analyte = analyte;
      analyteInfo.amount  = comp.signal_concentration;

      if ( ic > 0 )
         desc += " ";
      desc += comp.name;

      solution.analyteInfo.append( analyteInfo );
   }

   desc                   += " | " + buffer.description;
   solution.solutionDesc   = desc;

   return solution;
}

double US_SimRecord::maxOD( US_DataIO::RawData& data )
{
   double maxc    = 0.0;
   int    nscans  = data.scanCount();
   int    npoints = data.pointCount();

   for ( int ii = 0; ii < nscans; ii++ )
   {
      for ( int kk = 0; kk < npoints; kk++ )
         maxc = qMax( maxc, data.value( ii, kk ) );
   }

   return maxc;
}
