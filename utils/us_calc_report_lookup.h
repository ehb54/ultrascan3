//! \file us_calc_report_lookup.h
//! \brief Which inputs a GMP report uses for a triple's model (algorithm
//! "report_lookup").
//!
//! As distrib_info in us_reporter_gmp.cpp:  the triple, cell, wavelength and
//! model name read out of the model description by position; the channel
//! whose report and analysis-profile values apply; the intensity that
//! applies; and the rows of the comparison and integration sections the
//! report mask shows.
#ifndef US_CALC_REPORT_LOOKUP_H
#define US_CALC_REPORT_LOOKUP_H

#include "us_extern.h"

#include <QMap>
#include <QString>
#include <QStringList>

//! GMP report rules
namespace US_Calc::Gmp
{
//! \brief The parts of a model description the report uses, e.g. for
//! "fixA.2A660.ee1_a2611090000_2DSA-IT_000001_i01.model":  triple "2A660",
//! cell "2", wavelength "660", model name "2DSA-IT".
struct ModelDescriptionParts
{
      //! False when the description has no second '.' field or its analysis
      //! label has no second '_' field; the other parts are then empty.
      bool    ok = false;
      QString triple;     //!< Second '.' field
      QString cell;       //!< First character of the triple
      QString wavelength; //!< Characters 3-5 of the triple
      QString analysis;   //!< analysis_label of the description
      //! Second '_' field of the analysis label; any PCSA method is "PCSA"
      //! and "-CG" is removed, as the report mask names models.
      QString model_name;
};

//! \brief Reads the parts of a model description by position.
US_UTIL_EXTERN ModelDescriptionParts parse_model_description( const QString& description );

//! \brief The index of the channel whose report and analysis-profile values
//! apply to a triple ("2A660"), or -1 when none does.
//!
//! The first channel description whose channel name (before the first ':')
//! the triple contains.  For an interference triple only interference
//! channels ("Interf") are considered.  For simulated absorbance data the
//! channel name's "A" is read as "S", as the simulated triples are named.
US_UTIL_EXTERN int                   report_channel_index( const QStringList& channel_descriptions,
                                                           const QString&     triple,
                                                           bool               interference_triple,
                                                           bool               simulated_absorbance );

//! \brief The triple as the intensity and the report mask name it:  for
//! simulated absorbance data every "S" is read as "A".
US_UTIL_EXTERN QString               intensity_triple( const QString& triple, bool simulated_absorbance );

//! \brief The average intensity that applies to a triple (named as
//! intensity_triple gives it).
//!
//! With one wavelength its value, whatever the key; otherwise the value of
//! the first key, in key order, that the triple contains; NaN when none.
US_UTIL_EXTERN double                triple_intensity( const QMap<QString, QString>& intensities, const QString& triple );

//! \brief The triple as the report mask names it:  for an interference
//! triple "660" is read as "Interference".
US_UTIL_EXTERN QString               mask_triple( const QString& triple, bool interference_triple );

//! \brief The rows the report mask shows for a triple's model.
struct MaskRows
{
      bool total_concentration = false;
      bool rmsd                = false;
      bool experiment_duration = false;
      bool minimum_intensity   = false;
      bool loading_volume      = false;
      bool integration_results = false;

      //! The comparison section is shown when any of its rows is.
      bool comparison_section() const
      {
         return total_concentration || rmsd || experiment_duration || minimum_intensity || loading_volume;
      }
};

//! \brief The rows of a model's mask entries (name -> "0"/"1"):  a row is
//! shown when an entry whose name contains its keyword ("Concentration",
//! "RMSD", "Duration", "Intensity", "Loading", "Integration") has a nonzero
//! integer value.  A missing triple or model has no entries and shows none.
US_UTIL_EXTERN MaskRows mask_rows( const QMap<QString, QString>& model_parts );
} // namespace US_Calc::Gmp

#endif
