//! \file us_dmga_constr.h
#ifndef US_DMGA_CONSTRAINTS_H
#define US_DMGA_CONSTRAINTS_H

#include "us_extern.h"
#include "us_model.h"

/*! \class US_dmGA_Constraints
   This class defines a discrete model GA constraints object consisting of
   a constraints model and a definition of floating attributes within it.
*/

class US_UTIL_EXTERN US_dmGA_Constraints
{

   public:

      //! \brief   Attribute type codes for component or association floats
      enum AttribType
      {
         ATYPE_S,              //!< Sedimentation coefficient
         ATYPE_D,              //!< Diffusion coefficient
         ATYPE_MW,             //!< Molecular weight
         ATYPE_FF0,            //!< Frictional ratio
         ATYPE_F,              //!< Frictional coefficient
         ATYPE_VBAR,           //!< Vbar (specific density)
         ATYPE_CONC,           //!< Signal (partial) concentration
         ATYPE_EXT,            //!< Extinction
         ATYPE_KD,             //!< K_disassociation
         ATYPE_KOFF            //!< K_off rate
      };

      //! \brief Float constraint object with type and range
      class Constraint
      {
         public:
         AttribType  atype;    //!< Component/Association attribute type
         int         mcompx;   //!< Model component/association index
         double      low;      //!< Attribute low/fixed value
         double      high;     //!< Attribute high value
         bool        fixed;    //!< Fixed/float flag
         bool        logscl;   //!< Log-scale flag
      };

      //! \brief Generic constructor for the US_dmGA_Constraints class.
      US_dmGA_Constraints( US_Model* );

      //! A destructor.
      ~US_dmGA_Constraints();

      //! \brief A function to load the base model
      //! \param bmodelP A pointer to a base model to load
      void load_base_model ( US_Model* );

      //! \brief A function to load a constraints model
      //! \param cmodelP A pointer to a constraints model to load
      void load_constraints( US_Model* );

      //! \brief A function to get the current base model
      //! \param bmodelP A pointer to which to return a base model
      //! \returns       Flag if a base model exists and was returned
      bool get_base_model  ( US_Model* );

      //! \brief A function to get the current constraints model
      //! \param cmodelP A pointer to which to return a constraints model
      //! \returns       Flag if a constraints model exists and was returned
      bool get_constraints ( US_Model* );

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

      double         xfinc;     // Change-float increment (1.0/nfcomb)

   private slots:

      void constraints_from_model( void );
      void constraints_from_base ( void );
};

#endif
