//! \file us_dmga_mc_stats.h
#ifndef US_DMGA_MC_STATS_H
#define US_DMGA_MC_STATS_H

#include <QtCore>

#include "us_extern.h"
#include "us_model.h"

//! \brief DMGA-MC Statistics and Processor object

//! \class US_DmgaMcStats
//!  This class consists of static functions for manipulation
//!  of a DMGA-MC model, including producing an iteration models vector
//!  and statistics related to those models.
//!  Statistics for each attribute are:
//!    [  0 ] = minimum
//!    [  1 ] = maximum
//!    [  2 ] = mean
//!    [  3 ] = median
//!    [  4 ] = skew
//!    [  5 ] = kurtosis
//!    [  6 ] = lower mode
//!    [  7 ] = upper mode
//!    [  8 ] = mode center
//!    [  9 ] = 95% confidence interval low
//!    [ 10 ] = 95% confidence interval high
//!    [ 11 ] = 99% confidence interval low
//!    [ 12 ] = 99% confidence interval high
//!    [ 13 ] = standard deviation
//!    [ 14 ] = standard error
//!    [ 15 ] = variance
//!    [ 16 ] = correlation coefficient
//!    [ 17 ] = number of bins
//!    [ 18 ] = distribution area
//!    [ 19 ] = 95% confidence limit low
//!    [ 20 ] = 95% confidence limit high
//!    [ 21 ] = 99% confidence limit low
//!    [ 22 ] = 99% confidence limit high

class US_DmgaMcStats {
 public:
  //! \brief Build a vector of iteration models from a DMGA-MC model
  //! \param model   Input DMGA-MC model
  //! \param imodels Output vector of iteration models
  //! \return        Number of MC iterations and vector elements
  static int build_imodels(US_Model&, QVector<US_Model>&);

  //! \brief Build RMSD statistics from iteration models
  //! \param niters  Number of MC iterations
  //! \param imodels Vector of iteration models
  //! \param rstats  Vector of doubles holding iteration statistics
  static void build_rmsd_stats(int, QVector<US_Model>&, QVector<double>&);

  //! \brief Build Model attribute statistics from iteration models
  //! \param niters  Number of MC iterations
  //! \param imodels Vector of iteration models
  //! \param astats  Vector of vectors of iteration attribute statistics
  //! \return        Number of total attributes analyzed
  static int build_model_stats(int, QVector<US_Model>&,
                               QVector<QVector<double> >&);

  //! \brief Compute the statistical values for a vector of values
  //! \param nvals   Number of values to analyze
  //! \param vals    Vector of values to analyze
  //! \param concs   Vector of concentrations (weights) for each value
  //! \param stats   Vector of output statistics values (min,max,mean,...)
  //! \returns       Boolean flag if values are fixed (all equal)
  static bool compute_statistics(int, QVector<double>&, QVector<double>&,
                                 QVector<double>&);

  //! \brief Build used model of specified type from iteration models
  //! \param smtype  Used model type ("model","mean","median","mode")
  //! \param iter    Iteration number (1-niters) if "model" (iter.) used
  //! \param imodels Vector of iteration models
  //! \param umodel  Returned composed used model
  //! \return        Statistics index used (-iter,2,3,8)
  static int build_used_model(const QString, const int, QVector<US_Model>&,
                              US_Model&);
};
#endif
