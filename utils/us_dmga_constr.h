//! \file us_dmga_constr.h
#ifndef US_DMGA_CONSTRAINTS_H
#define US_DMGA_CONSTRAINTS_H

#include <QDebug>
#include "us_extern.h"
#include "us_model.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()  //!< debug-level-conditioned qDebug()
#endif

/*! \class US_dmGA_Constraints
   This class defines a discrete model GA constraints object consisting
   mainly of a constraints model and a definition of floating attributes
   within it.
*/

class US_UTIL_EXTERN US_dmGA_Constraints
{

   public:

      //! \brief   Attribute type codes for component or association floats
      enum AttribType
      {
         ATYPE_S,              //!< Sedimentation coefficient
         ATYPE_FF0,            //!< Frictional ratio
         ATYPE_MW,             //!< Molecular weight
         ATYPE_D,              //!< Diffusion coefficient
         ATYPE_F,              //!< Frictional coefficient
         ATYPE_VBAR,           //!< Vbar (specific density)
         ATYPE_CONC,           //!< Signal (partial) concentration
         ATYPE_EXT,            //!< Extinction
         ATYPE_KD,             //!< K_disassociation
         ATYPE_KOFF            //!< K_off rate
      };

      //! \brief Float constraint object with type and range
      class US_UTIL_EXTERN Constraint
      {
         public:
         AttribType  atype;    //!< Component/Association attribute type
         int         mcompx;   //!< Model component/association index
         double      low;      //!< Attribute low/fixed value
         double      high;     //!< Attribute high value
         bool        floats;   //!< Floats/fixed flag
         bool        logscl;   //!< Log-scale flag
      };

      //! \brief Generic constructor for the US_dmGA_Constraints class.
      US_dmGA_Constraints( US_Model* = NULL );

      //! A destructor.
      ~US_dmGA_Constraints();

      //! \brief A function to load the base model
      //! \param bmodelP A pointer to a base model to load
      void load_base_model ( US_Model* );

      //! \brief A function to load a constraints model
      //! \param cmodelP A pointer to a constraints model to load
      void load_constraints( US_Model* );

      //! \brief A function to load a constraints vector and build model
      //! \param cnsv    A reference to constraints vector to load
      void load_constraints( QVector< Constraint >& );

      //! \brief A function to update a constraints vector and rebuild model
      //! \param cnsv    A reference to constraints vector for updates
      //! \returns       Count of updated number of constraints
      int  update_constraints( QVector< Constraint >& );

      //! \brief A function to initialize the constraints vector
      void init_constraints( void );

      //! \brief A function to add a constraints vector entry
      //! \param atype   Attribute type
      //! \param mcompx  Model component/association index
      //! \param low     Low attribute value
      //! \param high    High attribute value (ignored if fixed)
      //! \param floats  Flag if attribute floats
      //! \param logscl  Flag if range scale is logarithmic
      //! \returns       Count of updated number of constraints
      int  add_constraint( AttribType, int, double, double,
                           bool, bool );

      //! \brief A function to get the current base model
      //! \param bmodelP A pointer to which to return a base model
      //! \returns       Flag if a base model exists and was returned
      bool get_base_model  ( US_Model* );

      //! \brief A function to get the current constraints model
      //! \param cmodelP A pointer to which to return a constraints model
      //! \returns       Flag if a constraints model exists and was returned
      bool get_constr_model( US_Model* );

      //! \brief A function to get the current work model
      //! \param wmodelP A pointer to which to return a work model
      //! \returns       Flag if a work model exists and was returned
      bool get_work_model  ( US_Model* );

      //! \brief A function to get specified component's constraints
      //! \param compx   Index of component for which to fetch constraints
      //! \param cnsvP   Pointer to which to return component's constraints
      //! \param kfltP   Pointer for return of component's floats count
      //! \returns       Count of constraints for this component
      int  comp_constraints( int, QVector< Constraint >* cnsvP, int* );

      //! \brief A function to get specified association's constraints
      //! \param assox   Index of association for which to fetch constraints
      //! \param cnsvP   Pointer to which to return association's constraints
      //! \param kfltP   Pointer for return of association's floats count
      //! \returns       Count of constraints for this association
      int  assoc_constraints( int, QVector< Constraint >* cnsvP, int* );

      //! \brief A function to get all the current model's float constraints
      //! \param cnsvP   Pointer to which to return float constraints
      //! \returns       Count of float constraints for the current model
      int  float_constraints( QVector< Constraint >* cnsvP );

      //! \brief A function to fetch a specified component attribute value
      //! \param sc      A reference to a component from which to fetch
      //! \param atype   The attribute type to fetch
      //! \returns       The attribute value
      double fetch_attrib( US_Model::SimulationComponent&, const AttribType );

      //! \brief A function to fetch a specified association attribute value
      //! \param as      A reference to an association from which to fetch
      //! \param atype   The attribute type to fetch
      //! \returns       The attribute value
      double fetch_attrib( US_Model::Association&,         const AttribType );

      //! \brief A function to store a specified component attribute value
      //! \param sc      A reference to a component to update
      //! \param atype   The attribute type to store
      //! \param xval    The attribute value to store
      void   store_attrib( US_Model::SimulationComponent&, const AttribType, const double );

      //! \brief A function to store a specified association attribute value
      //! \param as      A reference to an association to update
      //! \param atype   The attribute type to store
      //! \param xval    The attribute value to store
      void   store_attrib( US_Model::Association&,         const AttribType, const double );

   private:
      US_Model       bmodel;    // Base model
      US_Model       cmodel;    // Constraints model
      US_Model       wmodel;    // Work model

      QVector< Constraint >  attribs;  // Attribute-defines vector

      AttribType     x_attr;    // X model component attribute used
      AttribType     y_attr;    // Y model component attribute used
      AttribType     z_attr;    // Z (fixed?) model component attribute used

      int            nfloat;    // Number of total floats
      int            nbcomp;    // Number of base model components
      int            nccomp;    // Number of constraints components
      int            nbassoc;   // Number of base model associations
      int            ncassoc;   // Number of constraints associations
      int            nfvari;    // Number of float variations
      int            dbg_level;

      double         xfinc;     // Change-float increment (1.0/nfcomb)

   private slots:
      void constraints_from_model( void );       // Build constraints from model
      void constraints_from_base ( void );       // Build constraints from base
      void model_from_constraints( void );       // Build model from constraints
      bool base_from_cmodel      ( void );       // Build base from c. model
      int  count_comp_constraints( int, int*, int* );  // Count comp. constraints
      int  count_asso_constraints( int, int*, int* );  // Count assoc. constraints
      bool init_work_model       ( void );       // Build initial work model
};

#endif
