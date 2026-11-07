//! \file us_sim_schedule.h
#ifndef US_SIM_SCHEDULE_H
#define US_SIM_SCHEDULE_H

#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>

#include "us_dataIO.h"
#include "us_extern.h"
#include "us_simparms.h"

/*! \class US_SimSchedule
    When a run's scans happen, and what its telemetry records.

    An instrument scans a run's cells in turn: each cell takes a slot, and
    the slots repeat in cycles. Every channel of a run is simulated with the
    same slots and its own slot index, so each one can compute the whole
    run's scan times and write the same TimeState as every other channel.

    Telemetry is what the TimeState and the scan headers record: the speed
    jitters about the set speed and the temperature wanders about the set
    temperature. The sedimentation is solved at the set values; telemetry
    never feeds back into it. Its random stream is seeded from the settings
    alone, so the channels of a run record identical telemetry.
*/
class US_UTIL_EXTERN US_SimSchedule
{
   public:
      using Schedule  = US_SimulationParameters::ScheduleSettings;
      using Telemetry = US_SimulationParameters::TelemetrySettings;

      //! Correlation time of the recorded temperature's wander, seconds.
      static constexpr double temperatureCorrelationSeconds = 300.0;

      //! \brief One TimeState record.
      struct Row
      {
            int    time        = 0;   //!< Seconds since the start of the run
            double raw_speed   = 0.0; //!< Recorded rotor speed, rpm
            int    set_speed   = 0;   //!< Set speed, rpm
            double omega2t     = 0.0; //!< Integral of omega squared to this second
            double temperature = 0.0; //!< Recorded temperature, degrees C
            int    step        = 0;   //!< Speed step number
            int    scan        = 0;   //!< Scan number at this second, 0 for none
      };

      //! \brief A problem with schedule settings, or empty if they are usable.
      //! \param wavelengths The channel's wavelength count, which must fit its slot.
      static QString        scheduleProblem( const Schedule& settings, int wavelengths = 1 );

      //! \brief A problem with telemetry settings, or empty if they are usable.
      static QString        telemetryProblem( const Telemetry& settings );

      //! \brief Seconds between cycle starts.
      static double         cycleSeconds( const Schedule& settings );

      //! \brief Seconds from a cycle's start to a slot's start.
      static double         slotOffset( const Schedule& settings, int slot );

      //! \brief When one scan happens, in whole seconds.
      //! \param first      Time of the first cycle's start in this speed step.
      //! \param scan       Scan index within the step, from 0.
      //! \param wavelength Wavelength index within a multi-wavelength slot.
      //! \param slot       Slot; -1 for the settings' own slot.
      static int            scanTime( const Schedule& settings, double first, int scan, int wavelength = 0, int slot = -1 );

      //! \brief Every slot's scan start in the run, mapped to its cycle number.
      //! Cycle numbers count from 1 across the run's speed steps.
      //! \param firsts First cycle start of each speed step.
      //! \param counts Scans in each speed step.
      static QMap<int, int> runMarkers( const Schedule& settings, const QVector<double>& firsts, const QVector<int>& counts );

      //! \brief A copy of data with one scan, like its first, at each marker's time.
      //! Written through US_AstfemMath::writetimestate, it marks the run's
      //! scans in a time state. data must hold at least one scan.
      static US_DataIO::RawData markedData( const US_DataIO::RawData& data, const QMap<int, int>& markers );

      //! \brief Whether two speed-step lists describe the same run: speeds,
      //! scan counts, durations, delays and accelerations.
      static bool               sameSpeedSteps( const QVector<US_SimulationParameters::SpeedProfile>& one,
                                                const QVector<US_SimulationParameters::SpeedProfile>& two );

      //! \brief Replace every record's scan number with the run's markers.
      static void           applyMarkers( QVector<Row>& rows, const QMap<int, int>& markers );

      //! \brief Add recorded speed jitter and temperature variation.
      //! Speed jitters only where the set speed holds; omega2t is
      //! re-integrated from the recorded speed.
      static void           applyTelemetry( QVector<Row>& rows, const Telemetry& settings );

      //! \brief Read a TimeState's records and its field keys and formats.
      static bool           readRows( const QString& tmst_path, QVector<Row>& rows, QStringList& keys, QStringList& formats );

      //! \brief Write records as a TimeState with the given keys and formats.
      static bool           writeRows( const QString&      tmst_path,
                                       const QVector<Row>& rows,
                                       const QStringList&  keys,
                                       const QStringList&  formats );

      /*! \brief Rewrite a TimeState as the run's recorded one.
          Replaces its scan markers with the run's, when given, and adds the
          recorded speed and temperature variation, when telemetry is given.
          \param rows Set to the records written.
          \returns An error message, or empty on success.
      */
      static QString        recordRun( const QString&        tmst_path,
                                       const QMap<int, int>* markers,
                                       const Telemetry&      telemetry,
                                       QVector<Row>&         rows );

      //! \brief Set each scan's rpm, omega2t and temperature from the record at its time.
      //! \returns False if a scan's time has no record.
      static bool           recordScans( US_DataIO::RawData& data, const QVector<Row>& rows );
};

#endif
