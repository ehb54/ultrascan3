//! \file us_sim_optics.h
#ifndef US_SIM_OPTICS_H
#define US_SIM_OPTICS_H

#include <QString>
#include <QVector>

#include "us_buffer.h"
#include "us_dataIO.h"
#include "us_extern.h"
#include "us_simparms.h"

/*! \class US_SimOptics
    What an absorbance optical system records, given a simulated absorbance.

    The simulators solve for absorbance between the meniscus and the cell
    bottom. These functions turn that solution into instrument-shaped data: a
    fixed radial window with air before the meniscus and nothing past the
    bottom, and the intensities a sample and a reference sector record.

    Random draws come from US_Math2::box_muller(), in a fixed order, so a
    simulator that seeds that stream gets reproducible output.
*/
class US_UTIL_EXTERN US_SimOptics
{
   public:
      using Intensity      = US_SimulationParameters::IntensitySettings;
      using Window         = US_SimulationParameters::WindowSettings;
      using Interference   = US_SimulationParameters::InterferenceSettings;
      using WavelengthScan = US_SimulationParameters::WavelengthScanSettings;

      //! Radius at the center of the light profile, cm.
      static constexpr double profileCenter    = 6.5;
      //! Half-width of the light profile, cm: the profile falls by
      //! profileCurvature at this distance from its center.
      static constexpr double profileHalfWidth = 0.75;
      //! Fractional fall of a curved light profile at profileHalfWidth.
      static constexpr double profileCurvature = 0.05;

      //! \brief A problem with intensity settings, or empty if they are usable.
      static QString          intensityProblem( const Intensity& settings );

      //! \brief A problem with window settings for this cell, or empty if usable.
      //! The window must hold the cell from meniscus to bottom.
      static QString          windowProblem( const Window& settings, double meniscus, double bottom );

      //! \brief The window's radii: start, start + resolution, ..., below end.
      static QVector<double>  windowRadii( const Window& settings );

      //! \brief The light source's relative output at a wavelength, 0 to 1.
      //! Weak in the far UV and near 1 above about 300 nm.
      static double           sourceFactor( double wavelength );

      //! \brief Light reaching the detector with nothing in the beam, in counts.
      static double           sourceIntensity( const Intensity& settings, double radius, double wavelength );

      //! \brief Counts noise sigma(I) = sqrt( read^2 + shot I ).
      static double           countsSigma( const Intensity& settings, double counts );

      //! \brief A recorded intensity: the true one floored at the dark level,
      //!        plus deviate times its noise sigma, never below zero.
      static double           recordedCounts( const Intensity& settings, double counts, double deviate );

      //! \brief The buffer's absorbance at a wavelength over the path length.
      //! Extinction is read as OD per cm and interpolated linearly; a buffer
      //! with no spectrum, or none covering the wavelength, absorbs nothing.
      static double           bufferOD( const US_Buffer& buffer, double wavelength, double pathlength );

      //! \brief The reference channel paired with a sample channel: A->B, C->D, E->F, G->H.
      //! \returns The letter, or 0 for a letter that is not a sample channel.
      static char             referenceChannel( char sample );

      /*! \brief Resample a liquid-region absorbance dataset onto the window.
          Before the meniscus: air, 0 plus N(0, air_sigma). At the meniscus:
          a spike a few points wide, its width and its height (at least
          meniscus_artifact times the loading) drawn from the noise stream.
          It must stand above the plateau: the GMP automatic edit takes the
          largest late-scan value near the meniscus as the meniscus. Between
          meniscus and bottom: the data, linearly interpolated, plus
          liquid_offset. Past the bottom: past_bottom. No value exceeds
          ceiling when ceiling is above 0.
          \param data          Absorbance; xvalues must be ascending. Replaced.
          \param settings      The window.
          \param meniscus      Meniscus radius, cm.
          \param bottom        Bottom radius, cm.
          \param loading       Total loading, scaling the meniscus spike.
          \param air_sigma     Random noise sigma before the meniscus.
          \param liquid_offset Added to every liquid value (buffer absorbance).
          \param past_bottom   Value past the bottom.
          \param ceiling       Largest value written; 0 for none.
      */
      static void             toWindow( US_DataIO::RawData& data,
                                        const Window&       settings,
                                        double              meniscus,
                                        double              bottom,
                                        double              loading,
                                        double              air_sigma,
                                        double              liquid_offset,
                                        double              past_bottom,
                                        double              ceiling = 0.0 );

      /*! \brief Sample and reference intensities for an absorbance dataset.
          Before the meniscus both sectors see the source. Between meniscus
          and bottom the reference sees the source through the buffer, and the
          sample through the buffer and the absorbance. Past the bottom no
          light passes. Each value then gets counts noise.
          The reference copies the sample's identity except for its channel;
          the caller assigns its raw GUID.
          \param absorbance Absorbance on any radial grid.
          \param settings   Light source and detector.
          \param meniscus   Meniscus radius, cm.
          \param bottom     Bottom radius, cm.
          \param buffer_od  Buffer absorbance.
          \param sample     Set to the sample sector's intensities.
          \param reference  Set to the reference sector's intensities.
      */
      static void             toIntensity( const US_DataIO::RawData& absorbance,
                                           const Intensity&          settings,
                                           double                    meniscus,
                                           double                    bottom,
                                           double                    buffer_od,
                                           US_DataIO::RawData&       sample,
                                           US_DataIO::RawData&       reference );

      //! \brief A problem with interference settings, or empty if they are usable.
      static QString          interferenceProblem( const Interference& settings );

      //! \brief The interference detector's radii.
      static QVector<double>  interferenceRadii( const Interference& settings );

      /*! \brief Record a liquid-region concentration as interference fringes.
          Resamples onto the interference grid, which need not reach the
          bottom: before the meniscus nothing, at the meniscus a spike, in the
          liquid the data times scale, past the bottom the ceiling (or the last
          liquid value). Then adds, in fringes, a fixed radial structure shared
          by every scan, a baseline offset per scan, occasional whole-fringe
          jumps (a whole-scan offset, or a step from a random radius outward)
          and random noise. Values above ceiling, when it is above 0, are cut.
          \param data      Signal on any ascending radial grid. Replaced.
          \param structure Set to the fixed radial structure, per grid point.
          \param offsets   Set to each scan's offset including whole-scan jumps.
      */
      static void             toInterference( US_DataIO::RawData& data,
                                              const Interference& settings,
                                              double              meniscus,
                                              double              bottom,
                                              double              loading,
                                              double              ceiling,
                                              QVector<double>&    structure,
                                              QVector<double>&    offsets );

      //! \brief A problem with wavelength-scan settings for this cell, or empty.
      static QString          wavelengthScanProblem( const WavelengthScan& settings, double meniscus, double bottom );

      //! \brief The scanned wavelengths: start, start + step, ..., up to end.
      static QVector<double>  scanWavelengths( const WavelengthScan& settings );

      //! \brief The spectrum at each wavelength, linearly interpolated.
      static QVector<double>  extinctionAt( const WavelengthScan& settings, const QVector<double>& wavelengths );

      /*! \brief Record a wavelength scan at one radius.
          Each scan's absorbance at the scan position is spread across the
          wavelengths in proportion to the spectrum (normalized to its
          maximum). The sample sees the source through that absorbance and the
          buffer, the reference through the buffer. The x-axis becomes the
          wavelengths and each scan's wavelength field the radius, as UltraScan
          stores an Optima wavelength scan.
          \param buffer_od Buffer absorbance at each scanned wavelength.
      */
      static void             toWavelengthScan( const US_DataIO::RawData& absorbance,
                                                const Intensity&          intensity,
                                                const WavelengthScan&     settings,
                                                const QVector<double>&    buffer_od,
                                                US_DataIO::RawData&       sample,
                                                US_DataIO::RawData&       reference );
};

#endif
