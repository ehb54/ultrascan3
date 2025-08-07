#ifndef US_MODELBUILDER_H
#define US_MODELBUILDER_H

// #include <QtGui>
#include <QApplication>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QPair>
#include <QTextStream>
#include <QTime>
#include <QVector>
#if QT_VERSION > 0x050000
#include <QtConcurrent/QtConcurrent>
#else
#include <qtconcurrentrun.h>
#endif
#include <qwt_legend.h>
#include <unistd.h>  //for getting hostname

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QThread>
#include <cmath>
#include <stdexcept>

#include "../../programs/us_astfem_sim/us_astfem_sim.h"
#include "../../programs/us_astfem_sim/us_simulationparameters.h"
#include "us_astfem_rsa.h"
#include "us_constants.h"
#include "us_dataIO.h"
#include "us_extern.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_math2.h"
#include "us_model.h"
#include "us_passwd.h"
#include "us_settings.h"
#include "us_util.h"
#include "us_widgets.h"

/*!
 */
class US_ModelBuilder : public US_Widgets {
  Q_OBJECT

 public:
  /*!
   */
  US_ModelBuilder();
  //~US_ModelBuilder();

 private:
  //   US_Help showHelp;
  //   QLineEdit* le_RMSD;
  //   QLineEdit* le_sedimentationVal;
  //   QLineEdit* le_diffusionVal;
  //   QPushButton* startButton;
  //   QPushButton* parameterButton;
  //   QwtPlot* data_plot;
  US_SimulationParameters working_simparams; /*!< TODO: describe */
  US_SimulationParameters simparams;         /*!< TODO: describe */

 private slots:
  /*!
    Opens the simparams GUI to change simulation settings
  */
  void collectParameters(void);
  /*!
    Starts the simulation processes
  */
  void startSimulation(void);
  /*!
    Updates simparams with user-specified params from collectParameters
  */
  void updateParameters(void);
  /*!
    Sets up default state for simulation parameters
  */
  void initSimparams(void);

  //   void help(void)
  //   {
  //      showHelp.show_help("manual/us_buoyancy.html");
  //   }

  /*!
   \brief Initializes a RawData object for simulation using a complete US_Model
   \param system the US_Model to use for initalization
   \return US_DataIO::RawData an initialized RawData object
  */
  US_DataIO::RawData* initRawData(US_Model* system);

  /*!
   \brief Interpolates the z-value of a point target given its x and y coords
   using 4 surrounding points on a grid via bilinear interpolation
   \param target the point to interpolate for
   \param p11 bottom left corner of square bracket
   \param p12 upper left corner of square bracket
   \param p21 upper right corner of square bracket
   \param p22 bottom right corner of square bracket
   \return double the z-value interpolated for the target
  */
  double interpolatePoint(QVector2D target, QVector3D p11, QVector3D p12,
                          QVector3D p21, QVector3D p22);

  // Declarations from rewrite below:

  /*!
   \brief Calculates the distance on the sk plane between two simulation
   components
   \param first the first point
   \param second the second point
   \param scaleS whether or not to scale s-value up by 1e13
   \return double the distance between the two points
  */
  double calculateDistance(US_Model::SimulationComponent first,
                           US_Model::SimulationComponent second, bool scaleS);

  /*!
   \brief Simulates a single simulationcomponent using ASTFEM RSA
   \param component the solute to be simulated
   \return US_DataIO::RawData a RawData object storing the simulation results
  */
  US_DataIO::RawData* simulateComponent(
      US_Model::SimulationComponent component);

  /*!
    Simulates a series of simulation components.
   \param components the QVector
   \return QVector<QPair<US_Model::SimulationComponent, US_DataIO::RawData *> >
   a list of paris of simulation components and their associated simulation
   RawDatas.
  */
  QVector<QPair<US_Model::SimulationComponent, US_DataIO::RawData*> >
  simulateBatch(QVector<US_Model::SimulationComponent> components);

  // Calculation functions, from other solute properties
  /*!
   \brief Calculates f/f0 from a sedimentation coefficient and diffusion value
   pair.
   \param s the sedimentation value
   \param D the diffusion value
   \return double the calculated frictional ratio value
  */
  double calculateFrictionalRatioSD(double s, double D);

  /*!
   \brief Calculates a diffusion value from sedimentation coefficient and
   frictional ratio
   \param s the sedimentation value
   \param k the frictional ratio
   \return double the calculated diffusion value
  */
  double calculateDiffusionSK(double s, double k);

  /*!
   \brief Calculates a diffusion value from molecular weight and frictioal ratio
   \param M the molecular weight
   \param k the frictional ratio
   \return double the calculated diffusion value
  */
  double calculateDiffusionMK(double M, double k);

  /*!
   \brief Calculates sedimentation coefficient from molecular weight and
   diffusion
   \param M the molecular weight
   \param D the diffusion
   \return double the calculate sedimentation coefficient
  */
  double calculateSedimentationMD(double M, double D);

  /*!
   \brief Creates a SimulationComponent from a sedimentation coefficient and a
   frictional ratio, calculating other values.
   \param s the sedimentaton coefficient
   \param k the frictional ratio
   \return US_Model::SimulationComponent a simulationcomponent representing a
   solute with the given s and f/f0
  */
  US_Model::SimulationComponent componentFromSK(double s, double k);

  // RMSD calculation functions and associated utilities
  /*!
   \brief Calculates the RMSD between two simulated solutes, excluding simulated
   scans that take place after either solute has pelleted.
   \param simulation1 a RawData containg the simulation of the first solute
   \param component1 a SimulationComponent describing the first solute
   \param simulation2 a RawData containg the simulation of the second solute
   \param component2 a SimulationComponent describing the second solute
   \return double the calculated RMSD value
  */
  double calculateScaledRMSD(US_DataIO::RawData* simulation1,
                             US_Model::SimulationComponent component1,
                             US_DataIO::RawData* simulation2,
                             US_Model::SimulationComponent component2);

  /*!
   \brief Wrapper method around RMSD calculation to allow RMSD to be computed
   using QPairs of SimulationComponents and RawDatas.
   \param pair1 the first pair of solute and simulation data
   \param pair2 the second pair of solute and simulation data
   \return double the RMSD between the two solutes
  */
  double calculateScaledRMSD(
      QPair<US_Model::SimulationComponent, US_DataIO::RawData*> pair1,
      QPair<US_Model::SimulationComponent, US_DataIO::RawData*> pair2);

  /*!
   \brief Calcualtes the RMSDs between every two adjacent elements in a list of
   SimulationComponents.
   \param components the list of components
   \return QVector<double> A list of RMSDs with size equal to components.size()
   - 1, storing RMSDs between each two elements in components
  */
  QVector<double> findListRMSD(
      QVector<US_Model::SimulationComponent> components);

  /*!
   \brief Calcualtes the RMSDs between adjacent elements in multiple lists of
   simulation components, with each list calculation running on a seperate
   thread.
   \param grid The 2D QVector of solutes
   \param approximateToMidpoint whether or give resultant values s, D, f/f0, and
   mw values of the average of the two contributing points
   \return QVector<QVector<double> > a list of length 5 qvectors containing s,
   D, f/f0, mw, and RMSD
  */
  QVector<QVector<double> > checkLineRMSDParalell(
      QVector<QVector<US_Model::SimulationComponent> > grid,
      bool approximateToMidpoint);

  /*!
   \brief Calculates RMSD distribution statistics on a specified grid,
   calculating along inner QVectors.
   \param processedGrid a QVector of length 5 qvectors containing s, D, f/f0,
   mw, and RMSD
   \return QVector<double> a size 3 qvector in the form: pointcount, average
   RMSD, standard deviation
  */
  QVector<double> calculateGridStatistics(
      QVector<QVector<double> > processedGrid);

  /*!
   \brief Determines RMSDs for every point on a regular grid based on the
   average RMSD to each of its four neighbors on both axes
   \param grid A 2D QVector of SimulationComponents representing the regular
   grid
   \return QVector<QPair<US_Model::SimulationComponent, double> > A list of
   pairs of SimulationComponents and the average RMSD to each neighbor
  */
  QVector<QPair<US_Model::SimulationComponent, double> > testRegularGrid(
      QVector<QVector<US_Model::SimulationComponent> > grid);

  /*!
   \brief Switches the major-minor axes of a 2D QVector containing a regular
   grid
   \param regular the regular grid representation to rotate
   \return QVector<QVector<US_Model::SimulationComponent> > a regular grid
   identical to the one passed, but with its major-minor axes switched
  */
  QVector<QVector<US_Model::SimulationComponent> > switchRegularGridOrientation(
      QVector<QVector<US_Model::SimulationComponent> > regular);

  /*!
   \brief Very inefficiently finds RMSD levels for every point in an irregularly
   spaced grid (one in which order does not provide additional information).

   Operates by simulating every single point, storing simulations in memory, and
   finding RMSDs between every point. An arbitrary threshold, numNeighbors, then
   determines how many neighboring points to include in the RMSD average.
   Generally, this method uses RMSD as a measure of what points are closest to
   each other. As a consequence, passing large grids to this method will require
   large amounts of memory and extremely large amounts of time to produce
   results.

   \param grid A 2D QVector containing all the points in the irregular grid
   \param numNeighbors The number of neighbors to use when calculating RMSD
   average
   \return QVector<QPair<US_Model::SimulationComponent, double> > A list of
   pairs of SimulationComponents and the average RMSD to each neighbor
  */
  QVector<QPair<US_Model::SimulationComponent, double> >
  calculateIrregularGridRMSD(
      QVector<QVector<US_Model::SimulationComponent> > grid, int numNeighbors);

  // Grid generation functions for particular grid-types and planes
  /*!
   \brief Generates a Faxen grid over the specified area, with a particular size
   parameter.

   Notably, the grids parameter does not directly set the number of points to
   generate; instead, it

   \param minS The minimum sedimentation value for the grid.
   \param maxS The maximum sedimentation value for the grid.
   \param minK The minimum frictional ratio value for the grid.
   \param maxK The maximum frictional ratio value for the grid.
   \param grids The gridsize parameter, which indirectly controls the number of
   points produced.
   \return QVector<QVector<US_Model::SimulationComponent> > A 2D QVector
   representing the points in the Faxen grid, along constant f/f0 lines
  */
  QVector<QVector<US_Model::SimulationComponent> > generateFaxenGrid(
      double sMin, double maxS, double kMin, double kMax, int grids);

  /*!
   \brief Generates a regularly spaced solute grid on the
   sedimentation-frictional ratio plane.

   \param minS The minimum sedimentation value for the grid.
   \param maxS The maximum sedimentation value for the grid.
   \param minK The minimum frictional ratio value for the grid.
   \param maxK The maximum frictional ratio value for the grid.
   \param sDim The number of distinct sedimentaion lines to include in the grid
   \param kDim The number of distinct frictional ratio lines to include in the
   grid
   \param constantAxis The first axis to be iterated down (top level QVector
   iterates over this, internal QVectors have constant value). Must be either
   's' or 'k'.
   \return QVector<QVector<US_Model::SimulationComponent> > A 2D QVector
   containing the grid, oriented based on the selected constantAxis
  */
  QVector<QVector<US_Model::SimulationComponent> > generateSKGrid(
      double sMin, double sMax, double kMin, double kMax, int sDim, int kDim,
      QChar constantAxis);

  /*!
   \brief Generates a regularly spaced solute grid on the
   sedimentation-diffusion plane

   \param sMin The minimum molecular weight for the grid.
   \param sMax The maximum molecular weight for the grid.
   \param dMin The minimum diffusion value for the grid.
   \param dMax The maximum diffusion value for the grid.
   \param sDim The number of distinct sedimentaion lines to include in the grid.
   \param dDim The number of distinct diffusion lines to include in the grid.
   \param constantAxis The first axis to be iterated down. Must be either 's' or
   'd'.
   \return QVector<QVector<US_Model::SimulationComponent> > A 2D QVector
   containing the grid, oriented based on the selected constantAxis.
  */
  QVector<QVector<US_Model::SimulationComponent> > generateSDGrid(
      double sMin, double sMax, double dMin, double dMax, int sDim, int dDim,
      QChar constantAxis);

  /*!
   \brief Generates a regularly spaced solute grid on the molecular
   weight-frictional ratio plane.

   \param mMin The minimum molecular weight for the grid.
   \param mMax The maximum molecular weight for the grid.
   \param minK The minimum frictional ratio value for the grid.
   \param maxK The maximum frictional ratio value for the grid.
   \param mDim The number of distinct molecular weight lines to include in the
   grid.
   \param kDim The number of distinct frictional ratio lines to include in the
   grid.
   \param constantAxis The first axis to be iterated down. Must be either 'm' or
   'k'.
   \return QVector<QVector<US_Model::SimulationComponent> > A 2D QVector
   containing the grid, oriented based on the selected constantAxis.
  */
  QVector<QVector<US_Model::SimulationComponent> > generateMKGrid(
      double mMin, double mMax, double kMin, double kMax, int mDim, int kDim,
      QChar constantAxis);

  // Same for numerical grids

  /*!
   \brief Numerically produces a line of constant-RMSD solute points on a given
   line.

   Uses a form of binary search over a line, testing points and halving step
   distances until a candidate is reached, then using the selected point as a
   comparison for the next. The if the target RMSD is too high, only the
   starting coordinate will be returned.

   \param tolerance The RMSD margin to allow when selecting points. Decreasing
   this value will raise precision and compute time.
   \param targetRMSD The RMSD value to iterate by while traversing the line.
   \param startCoord The starting point on the line, in terms of sedimentation
   and f/f0.
   \param endCoord The end point on the line.
   \return QVector<US_Model::SimulationComponent> A QVector of solutes, starting
   with the startCoord, and proceeding to points that differ from the previous
   by the specified RMSD.
  */
  QVector<US_Model::SimulationComponent> calculateLine(double tolerance,
                                                       double targetRMSD,
                                                       QVector2D startCoord,
                                                       QVector2D endCoord);

  /*!
   \brief Generates a solute grid of either constant sedimentation or f/f0 lines
   with constant RMSD between adjacent points.

   Although this method does space points such that a constant RMSD is
   maintained along the major axis, this is only done for a single line. All
   other points are placed along the inital set of sedimentation or f/f0 lines,
   which is inconsistent with the way that RMSD changes over the
   sedimetation-f/f0 plane. This method operates by using the calculateLine
   function once on its major axis, and then along the minor axis for each point
   produced.

   \param tolerance The RMSD margin to pass into calculateLine. Decreasing this
   value will raise precision and compute time.
   \param targetRMSD The target RMSD level.
   \param minS The minimum sedimentation value for the grid.
   \param maxS The maximum sedimentation value for the grid.
   \param minK The minimum frictional ratio value for the grid.
   \param maxK The maximum frictional ratio value for the grid.
   \param majorAxis The axis to initialize constant perpendicular lines over.
   Must be 's' or 'k'.
   \return QVector<QVector<US_Model::SimulationComponent> > A QVector containing
   each line produced for the grid.
  */
  QVector<QVector<US_Model::SimulationComponent> > createNumericalGrid(
      double tolerance, double targetRMSD, double minS, double maxS,
      double minK, double maxK, QChar majorAxis);

  // iso-RMSD components

  /*!
   \brief Generates a unit vector in the direction specified.

   \param degree A degree representation of the desired direction.
   \return QPair<double, double> A double-precision representation of the unit
   vector.
  */
  QPair<double, double> convertPolar(double degree);

  /*!
   \brief Finds a solute point with a specified RMSD to a specified origin
   solute point.

   Utilizes a golden section search to identify points.
   NOTE: Tolerance, in this context does not directly refer to RMSD deviation
   from the target; use 0.01 as value for most use cases

   \param origin A representation of the origin solute point.
   \param originSim A pointer to a simulation RawData of the origin point.
   \param target The RMSD level to search for.
   \param tolerance A tolerance level to be passed into the golden section
   search.
   \param degree The polar direction in which to search along from the origin.
   \return US_Model::SimulationComponent
  */
  US_Model::SimulationComponent findConstantRMSDPointInDirection(
      US_Model::SimulationComponent origin, US_DataIO::RawData* originSim,
      double target, double tolerance, double degree);

  /*!
   \brief Finds points with a specified RMSD level to a sedimentation-f/f0
   origin in all directions.

   Uses the golden section search implemented in
   findConstantRMSDPointInDirection.

   \param s The sedimentaiton coefficient of the origin point.
   \param k The frictional ratio value of the origin point.
   \param degreeIncrement The number of degrees to step between each search.
   Determines the number of searches performed (360/degreeIncrement).
   \param target The target RMSD level.
   \return QVector<US_Model::SimulationComponent> A list of SimulationComponent
   representations of all identified points.
  */
  QVector<US_Model::SimulationComponent> findConstantRMSDNeighbors(
      double s, double k, double degreeIncrement, double target);

  /*!
   \brief Finds a point in rectangular coordinates given an origin and a polar
   coordinate step distance.

   \param originX The origin point's X component.
   \param originY The origin point's Y component.
   \param distance The distance to step by.
   \param degree The direction to step in.
   \return QPair<double, double> A double-precision representation of the point.
  */
  QPair<double, double> projectPolar(double originX, double originY,
                                     double distance, double degree);

  /*!
   \brief Generates a SimulationComponent from a QPair of sedimentation and
   f/f0.

   \param sk The QPair containing both sedimentation and f/f0 values.
   \param scaleS Whether or not to scale the sedimentation value down by 1e13
   (to order e-13).
   \return US_Model::SimulationComponent The resulting SimulationComponent.
  */
  US_Model::SimulationComponent componentFromSKPair(QPair<double, double> sk,
                                                    bool scaleS);

  /*!
   \brief Generates a set of sample points in a given direction from an origin
   point for use in line fitting.

   \param origin A representation of the initial point.
   \param originSim A pointer to a simulation RawData of the origin point.
   \param numSamples The number of samples to collect.
   \param degree The direciton in which to do sampling.
   \return QVector<QPair<double, double> > A list of QPairs containing,
   respectively, the absolute distance along the specified direction from the
   origin and the RMSD relative to the origin.
  */
  QVector<QPair<double, double> > getSamplePoints(
      US_Model::SimulationComponent origin, US_DataIO::RawData* originSim,
      int numSamples, double degree);

  /*!
   \brief Gets RMSD level sets around a specified point.

   \param center The point around which level sets are to be found.
   \param targets A list of RMSD levels for which level sets will be found.
   \param degreeIncrement The degree step between directions sampled for each
   level set.
   \return QPair<QVector<QVector<US_Model::SimulationComponent> >,
   QVector<QPair<double, double> > > A QPair containing a paralell QVector to
   targets, each index of which contains a level set for its corresponding RMSD
   level, and a QVector of QPairs containing each degree-slope pair.
  */
  QPair<QVector<QVector<US_Model::SimulationComponent> >,
        QVector<QPair<double, double> > >
  getLevelSets(US_Model::SimulationComponent center, QVector<double> targets,
               double degreeIncrement);

  /*!
   \brief Produces a set of isoRMSD level sets for a list of solute points, then
   writes them to a file.

   Also writes a JSON file containing the slopes used to generate each point's
   level sets. Creates files and directories if they do not already exist.

   \param points The points for which level sets should be generated.
   \param rmsdTargets The RMSD levels for the level sets.
   \param increment The degree step between directions sampled for each level
   set.
   \param outputname The desired output file suffix.
  */
  void runIsoRMSDTest(QVector<US_Model::SimulationComponent> points,
                      QVector<double> rmsdTargets, double increment,
                      QString outputname);

  /*!
   \brief Generates level sets for a set of points contained in a JSON produced
   by runIsoRMSDTest.

   \param slopeJson A QJsonArray extracted from the written JSON file.
   \param targets The RMSD targets for each level set.
   \return QPair<QVector<QVector<QVector<US_Model::SimulationComponent> > >,
   QVector<US_Model::SimulationComponent> > A QPair containing a QVector
   structure that contains each centerpoint's corresponding level sets (in the
   format used by getLevelSets) and a paralell QVector containing each
   centerpoint.
  */
  QPair<QVector<QVector<QVector<US_Model::SimulationComponent> > >,
        QVector<US_Model::SimulationComponent> >
  levelSetFromJson(QJsonArray slopeJson, QVector<double> targets);

  /*!
   \brief Writes JSON-derived level sets to disk in the format used by
   runIsoRMSDTest.

   Creates files and directories if they do not already exist.

   \param sets The level sets produced by levelSetFromJson.
   \param centerpoints The list of centerpoints produced by levelSetFromJson.
   \param targets The list of RMSD targets passed to levelSetFromJson.
   \param outputname The desired output filename suffix.
  */
  void processJsonSlopes(
      QVector<QVector<QVector<US_Model::SimulationComponent> > > sets,
      QVector<US_Model::SimulationComponent> centerpoints,
      QVector<double> targets, QString outputname);

  /*!
   * \brief Performs a linefit without intercept, yeilding a slope for a fitted
   * line passing through the origin. Uses ordinary least squares regression.
   * \param points A QVector of QPairs representing the set of points to be
   * fitted to; for each QPair the first element represents the x value, and the
   * second represents the y value of the point
   * \return a double value representing the slope of the line
   */
  QVector<double> linefitThroughOrigin(QVector<QPair<double, double> > points);
};

#endif
