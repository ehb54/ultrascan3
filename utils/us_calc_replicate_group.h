//! \file us_calc_replicate_group.h
//! \brief Replicate groups of a GMP report (algorithm "report_replicate_group").
//!
//! As the "Replicate Groups Averaging Results" section of the GMP reporter
//! (us_reporter_gmp.cpp:  the replicate section's assembly,
//! get_replicate_group_results and triple_exist_inMWL).  A replicate group
//! is split into one subgroup per wavelength; for each report item of the
//! subgroup's first channel (the reference), the members are the matching
//! items of every channel in the subgroup, and their mean fraction is
//! checked against the reference item's target.
#ifndef US_CALC_REPLICATE_GROUP_H
#define US_CALC_REPLICATE_GROUP_H

#include "us_calc_replicate.h"

#include <QString>
#include <QStringList>
#include <QVector>

//! GMP report rules
namespace US_Calc::Gmp
{
//! \brief A report item of one triple, as a replicate group compares it.
struct ReplicateItem
{
      QString type;
      QString method;
      double  low                 = 0.0;
      double  high                = 0.0;
      double  integration_value   = 0.0; //!< autoflowReportItem.integration_val
      double  target_percent      = 0.0;
      double  tolerance           = 0.0;
      //! The item's results on the triple's model, as distrib_info stored
      //! them; -1 (US_ReportGMP's initial value) when it was not evaluated.
      double  range_concentration = -1.0;
      double  fraction_percent    = -1.0;
};

//! \brief True when an item is the reference item of a replicate group:
//! the same type, method and bounds, and an integration value that
//! QString::number formats the same.
US_UTIL_EXTERN bool same_replicate_item( const ReplicateItem& reference, const ReplicateItem& item );

//! \brief The index of the first channel description that contains the
//! channel (e.g. "1A:UV/vis."), or -1.  A replicate member's report is the
//! one of that channel.
US_UTIL_EXTERN int  replicate_channel_index( const QStringList& channel_descriptions, const QString& channel );

//! \brief True when a triple takes part in replicate averaging:  its
//! wavelength is not in the channel's ':'-separated list of wavelengths not
//! run, and its report shows Integration Results.
US_UTIL_EXTERN bool replicate_triple_included( const QString& wavelength,
                                               const QString& wavelengths_not_run,
                                               bool           integration_results_shown );

//! \brief The triples of one wavelength in a replicate group.
struct ReplicateSubgroup
{
      QString     wavelength;
      QStringList channels; //!< e.g. "1A", in the order of the group's triples
};

//! \brief A replicate group's channels and subgroups.
struct ReplicateLayout
{
      //! The channels of the included triples, each once, in the order of
      //! their first triple (the group heading).
      QStringList                channels;
      //! One subgroup per wavelength, in the order of each wavelength's first
      //! included triple.
      QVector<ReplicateSubgroup> subgroups;
};

//! \brief Splits a replicate group's triples ("1A.260") into subgroups.
//! included[i] says whether triples[i] takes part (replicate_triple_included).
US_UTIL_EXTERN ReplicateLayout replicate_layout( const QStringList& triples, const QVector<bool>& included );

//! \brief A reference item's result over a replicate subgroup.
struct ReplicateGroupResult
{
      //! Means and population standard deviations of the members; all 0
      //! when no member is used.
      double           mean_range_concentration = 0.0;
      double           sd_range_concentration   = 0.0;
      double           mean_fraction_percent    = 0.0;
      double           sd_fraction_percent      = 0.0;
      //! The standard deviations are shown when the subgroup lists more than
      //! one triple, whether or not every triple gives a member.
      bool             show_sd                  = false;
      //! The mean fraction is within tolerance percent of the target
      //! (ToleranceRule::RelativePercentOfTarget).
      bool             passed                   = false;
      ReplicateResult  summary; //!< The members' summary, with its diagnostics
      AlgorithmVersion algorithm;
};

//! \brief Evaluates a reference item over a subgroup.
//!
//! channel_items holds the report items of each channel of the subgroup,
//! in order; a channel whose report was not found has none.  Every item
//! that is the same replicate item as the reference and was evaluated
//! (in-range concentration not negative) is a member.
US_UTIL_EXTERN ReplicateGroupResult evaluate_replicate_group( const ReplicateItem&                   reference,
                                                              const QVector<QVector<ReplicateItem>>& channel_items,
                                                              int                                    listed_triples );
} // namespace US_Calc::Gmp

#endif
