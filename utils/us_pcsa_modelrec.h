//! \file us_pcsa_modelrec.h
#ifndef US_PCSA_MODEL_RECORD_H
#define US_PCSA_MODEL_RECORD_H

#include <QtCore>

#include "us_dataIO.h"
#include "us_extern.h"
#include "us_model.h"
#include "us_zsolute.h"

#ifndef DbgLv
#define DbgLv(a) \
   if (dbg_level >= a) \
   qDebug()
#endif
#define CTYPE_NONE 0
#define CTYPE_SL 1
#define CTYPE_IS 2
#define CTYPE_DS 4
#define CTYPE_HL 8
#define CTYPE_2O 16
#define CTYPE_ALL 7

//! \brief US_ModelRecord object

/*! \class US_ModelRecord
 *
    This class sets up a record of a model used in the set
    of models defined by curves in PCSA. A vector of such objects
    can be sorted and evaluated in analysis and display of a PCSA run.
*/
class US_UTIL_EXTERN US_ModelRecord {
   public:
      //! \brief Constructor for model record class
      US_ModelRecord();

      ~US_ModelRecord();

      int taskx; //!< Task index (submit order)
      int ctype; //!< Curve type (1/2/4/8=>SL/IS/DS/HL)
      int stype; //!< Solute type xyz mask
      int mrecID; //!< pcsa_modelrec DB ID
      int editID; //!< editedData DB ID
      int modelID; //!< best-model model DB ID (or 0)
      int v_ctype; //!< overall vector curve type (7=>All)
      double str_y; //!< Start k value
      double end_y; //!< End k value
      double par1; //!< Sigmoid/PL par1/A value
      double par2; //!< Sigmoid/PL par2/B value
      double par3; //!< Power Law  par3/C value
      double variance; //!< Variance value
      double rmsd; //!< RMSD value
      double xmin; //!< Minimum s value
      double xmax; //!< Maximum s value
      double ymin; //!< Minimum k value
      double ymax; //!< Maximum k value
      QString mrecGUID; //!< Model record GUID
      QString editGUID; //!< Model record GUID
      QString modelGUID; //!< Best Model GUID (or NULL)
      QVector<US_ZSolute> isolutes; //!< Input solutes
      QVector<US_ZSolute> csolutes; //!< Computed solutes
      QVector<double> ti_noise; //!< Computed TI noise
      QVector<double> ri_noise; //!< Computed RI noise

      US_Model model; //!< Computed model
      US_DataIO::RawData sim_data; //!< Simulation data from this fit
      US_DataIO::RawData residuals; //!< Residuals data from this fit

      //! \brief A test for ordering model descriptions. Sort by variance.
      bool operator<(const US_ModelRecord &mrec) const {
         return (variance < mrec.variance || (variance == mrec.variance && taskx < mrec.taskx));
      }

   public slots:
      //! \brief A public slot to clear data vectors (sim_data,residuals,noise)
      void clear_data(void);

      //! \brief Static public function to compute straight line model records
      //! \param xmin    X-value minimum
      //! \param xmax    X-value maximum
      //! \param ymin    Y-value minimum
      //! \param ymax    Y-value maximum
      //! \param nypts   Number of y start and end point variations
      //! \param nlpts   Number of line solute points
      //! \param parlims Parameter limits array: yslo, yshi, yelo, yehi, ...
      //! \param mrecs   Reference for generated model records vector
      //! \returns       Number of model line records generated
      static int compute_slines(
         double &, double &, double &, double &, int &, int &, double *, QVector<US_ModelRecord> &);

      //! \brief Static public function to compute sigmoid curve model records
      //! \param ctype   Curve-type flag (2/4 -> IS/DS)
      //! \param xmin    X-value minimum
      //! \param xmax    X-value maximum
      //! \param ymin    Y-value minimum
      //! \param ymax    Y-value maximum
      //! \param nypts   Number of y start and end point variations
      //! \param nlpts   Number of line solute points
      //! \param parlims Parameter limits array: yslo, yshi, yelo, yehi
      //! \param mrecs   Reference for generated model records vector
      //! \returns       Number of model line records generated
      static int compute_sigmoids(
         int &, double &, double &, double &, double &, int &, int &, double *, QVector<US_ModelRecord> &);

      //! \brief Static public function to compute horizontal line model records
      //! \param xmin    X-value minimum
      //! \param xmax    X-value maximum
      //! \param ymin    Y-value minimum
      //! \param ymax    Y-value maximum
      //! \param nypts   Number of y value variations
      //! \param nlpts   Number of line solute points
      //! \param parlims Parameter limits array: yslo, yshi, yelo, yehi
      //! \param mrecs   Reference for generated model records vector
      //! \returns       Number of model line records generated
      static int compute_hlines(
         double &, double &, double &, double &, int &, int &, double *, QVector<US_ModelRecord> &);

      //! \brief Static public function to compute 2nd-order line model records
      //! \param xmin    X-value minimum
      //! \param xmax    X-value maximum
      //! \param ymin    Y-value minimum
      //! \param ymax    Y-value maximum
      //! \param nypts   Number of y start and end point variations
      //! \param nlpts   Number of line solute points
      //! \param parlims Parameter limits array: yslo, yshi, yelo, yehi
      //! \param mrecs   Reference for generated model records vector
      //! \returns       Number of model line records generated
      static int compute_2ndorder(
         double &, double &, double &, double &, int &, int &, double *, QVector<US_ModelRecord> &);

      //! \brief Static public function to load model records from an XML stream
      //! \param xml     XML stream from which to load model records
      //! \param mrecs   Reference for generated model records vector
      //! \param descr   Ref. for model records description string
      //! \param ctype   Ref. for curve type flag: 1/2/4/7/8->SL/IS/DS/All/HL
      //! \param xmin    Ref. for x-value minimum
      //! \param xmax    Ref. for x-value maximum
      //! \param ymin    Ref. for y-value minimum
      //! \param ymax    Ref. for y-value maximum
      //! \param stype   Ref. for solute type flag (e.g., 0013 for s,k,v)
      //! \returns       Number of model line records generated
      static int load_modelrecs(
         QXmlStreamReader &xml, QVector<US_ModelRecord> &, QString &, int &, double &, double &, double &, double &,
         int &);

      //! \brief Static public function to write model records to an XML stream
      //! \param xml     XML stream to which to write model records
      //! \param mrecs   Model records vector to represent in XML
      //! \param descr   Ref. for model records description string
      //! \param ctype   Ref. for curve type flag: 1/2/4/7/8->SL/IS/DS/All/HL
      //! \param xmin    Ref. for x-value minimum
      //! \param xmax    Ref. for x-value maximum
      //! \param ymin    Ref. for y-value minimum
      //! \param ymax    Ref. for y-value maximum
      //! \param stype   Ref. for solute type flag (e.g., 0013 for s,k,v)
      //! \returns       Number of model line records generated
      static int write_modelrecs(
         QXmlStreamWriter &xml, QVector<US_ModelRecord> &, QString &, int &, double &, double &, double &, double &,
         int &);

      //! \brief Static public function to determine model records elite limits
      //! \param mrecs   Model records vector to scan
      //! \param ctype   Ref. for curve type flag: 1/2/4/7/8->SL/IS/DS/All/HL
      //! \param minyv   Ref. for y-value minimum
      //! \param maxyv   Ref. for y-value maximum
      //! \param minp1   Ref. for par1 minimum
      //! \param maxp1   Ref. for par1 maximum
      //! \param minp2   Ref. for par2 minimum
      //! \param maxp2   Ref. for par2 maximum
      //! \param minp3   Ref. for par3 minimum
      //! \param maxp3   Ref. for par3 maximum
      static void elite_limits(
         QVector<US_ModelRecord> &, int &, double &, double &, double &, double &, double &, double &, double &,
         double &);

      //! \brief Static public function to recompute mod.recs. for new iteration
      //! \param ctype   Ref. for curve type flag: 1/2/4/7/8->SL/IS/DS/All/HL
      //! \param xmin    Ref. for x-value minimum
      //! \param xmax    Ref. for x-value maximum
      //! \param ymin    Ref. for y-value minimum
      //! \param ymax    Ref. for y-value maximum
      //! \param nypts   Number of y value variations
      //! \param nlpts   Number of line solute points
      //! \param parlims Parameter limits array: yslo, yshi, yelo, yehi
      //! \param mrecs   Reference for re-created model records vector
      //! \returns       Number of model line records re-created
      static int recompute_mrecs(
         int &, double &, double &, double &, double &, int &, int &, double *, QVector<US_ModelRecord> &);

      //! \brief Static public function to return integer curve-type flag
      //!        for given text
      //! \param s_ctype String representation of curve type
      //! \returns       Integer flag representation of curve type
      static int ctype_flag(const QString);

      //! \brief Static public function to return curve-type text
      //!        for given integer flag
      //! \param i_ctype Integer flag representation of curve type
      //! \returns       String representation of curve type
      static QString ctype_text(const int);

      //! \brief Static public function to return integer solute-type flag
      //!        for given text (e.g., "013.skv")
      //! \param s_stype String representation of solute type
      //! \returns       Integer flag representation of solute type
      static int stype_flag(const QString);

      //! \brief Static public function to return solute-type text
      //!        for given integer flag (e.g., 11 == o013)
      //! \param i_stype Integer flag representation of solute type
      //! \returns       String representation of curve solute
      static QString stype_text(const int);
};
#endif
