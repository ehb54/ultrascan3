#ifndef US_MODEL_H
#define US_MODEL_H

#include <QtCore>
#include "us_extern.h"
#include "us_db2.h"

class US_EXTERN US_Model
{
   public:
      US_Model();

      class SimulationComponent;
      class Association;

      enum ShapeType { SPHERE, PROLATE, OBLATE, ROD };
      enum OpticsType{ ABSORBANCE, INTERFERENCE, FLUORESCENCE };
      enum ModelType { MANUAL, TWODSA, TWODSA_MW, GA, GA_MW, GA_RA, ONEDSA,
                       COFS, FE, GLOBAL };

      double     density;
      double     viscosity;
      double     compressibility;
      double     wavelength;
      double     temperature;
      QString    bufferGUID;   
      QString    bufferDesc;   
      QString    description;
      QString    guid;
      OpticsType optics;
      ModelType  type;
      //int        iterations;

      //! An index into components (-1 means none)
      int        coSedSolute;

      QVector< SimulationComponent > components;
      QVector< Association >         associations;

      //! Read a model from the disk or database
      //! \param db_access - A flag to indicate if the DB (true) or disk (false)
      //!                    should be searched for the model
      //! \param guid      - The guid of the model to be loaded
      //! \param db        - For DB access, an open database connection
      //! \returns         - The \ref US_DB2 retrun code for the operation
      int load( bool, const QString&, US_DB2* = 0 );

      //! Write a model to the disk or database
      //! \param db_access - A flag to indicate if the DB (true) or disk (false)
      //!                    should be used to save the model
      //! \param filename  - The filename (with path) where the xml file
      //!                    be written if disk access is specified
      //! \param db        - For DB access, an open database connection
      //! \returns         - The \ref US_DB2 retrun code for the operation
      int  write( bool, const QString&, US_DB2* = 0 );

      //! \param path - A reference where the path to ayalutes on the disk
      //!               drive is written
      //! \returns    - Success if the path is found or created and failure
      //!               if the path cannot be created
      static bool       model_path( QString& );

      class MfemInitial
      {
         public:
         QVector< double > radius;
         QVector< double > concentration;
      };

      class SimulationComponent
      {
         public:
         SimulationComponent();
         uchar       analyteGUID[ 16 ];    // GUID for the analyte in the MySQL DB
         double      molar_concentration;
         double      signal_concentration; // To be assigned prior to simulation
         double      vbar20;
         double      mw;
         double      s;
         double      D;
         double      f;
         double      f_f0;
         double      extinction;
         double      axial_ratio;
         double      sigma;   // Concentration dependency of s
         double      delta;   // concentration dependency of D
         int         stoichiometry;
         ShapeType   shape;
         QString     name;
         int         analyte_type;
         MfemInitial c0;      // The radius/concentration points for a user-defined
                              // initial concentration grid
      };

      class Association
      {
         public:
         Association();
         double k_eq;
         double k_off;
      
         // A list of all system components involved in this reaction
         QVector< uint > reaction_components;   

         // Stoichiometry of components in chemical equation.
         // Positive for reactant, negative for product
         QVector< int >  stoichiometry; 
      };

   private:
      int  load_db         ( const QString&, US_DB2* );
      int  load_disk       ( const QString& );
      int  read_model      ( const QString& );
      void mfem_scans      ( QXmlStreamReader&, SimulationComponent& );
      void get_associations( QXmlStreamReader&, Association& );
                           
      int  write_disk      ( const QString& );
      int  write_db        ( US_DB2* );
      void write_temp      ( QTemporaryFile& );
};
#endif
