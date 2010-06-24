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

      int        iterations;
      double     density;
      double     viscosity;
      double     compressibility;
      double     wavelength;
      double     temperature;
      QString    editguid;
      QString    bufferGUID;   
      QString    bufferDesc;   
      QString    description;
      QString    guid;
      OpticsType optics;
      ModelType  type;

      //! An index into components (-1 means none)
      int        coSedSolute;

      QVector< SimulationComponent > components;
      QVector< Association >         associations;

      QString message;  // Used internally for communication

      //! Read a model from the disk or database
      //! \param db_access - A flag to indicate if the DB (true) or disk (false)
      //!                    should be searched for the model
      //! \param guid      - The guid of the model to be loaded
      //! \param db        - For DB access, A pointer to an open database connection
      //! \returns         - The \ref US_DB2 retrun code for the operation
      int load( bool, const QString&, US_DB2* = 0 );

      //! An overloaded function to read a model from a database
      //! \param Database ModelID
      //! \param db        - For DB access, a pointer to an open database connection
      //! \returns         - The \ref US_DB2 retrun code for the operation
      int load( const QString&, US_DB2* ); 


      //! An overloaded function to read a model from the disk
      //! \param filename  The name, including full path, of the analyte file
      //! \returns         - The \ref US_DB2 retrun code for the operation
      int load( const QString& );  
      
      bool operator== ( const US_Model& ) const;      
      inline bool operator!= ( const US_Model& m ) const { return ! operator==(m); }
      //int load( const QString&, US_DB2* ); // guid db
      //int load( bool, const QString& );    // isFile=T, filename
                                             // isFile=F, guid

      //! Write a model to the disk or database
      //! \param db_access - A flag to indicate if the DB (true) or disk (false)
      //!                    should be used to save the model
      //! \param filename  - The filename (with path) where the xml file
      //!                    be written if disk access is specified
      //! \param db        - For DB access, a pointer to an open database connection
      //! \returns         - The \ref US_DB2 retrun code for the operation
      int write( bool, const QString&, US_DB2* = 0 );

      //! An overloaded function to write a model to the DB
      //! \param db        - A pointer to an open database connection 
      //! \returns         - The \ref US_DB2 retrun code for the operation
      int write( US_DB2* );

      //! An overloaded function to write a model to a file on disk
      //! \param filename  - The filename to write
      //! \returns         - The \ref US_DB2 retrun code for the operation
      int write( const QString& );

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
         
         bool operator== ( const SimulationComponent& ) const;      
         inline bool operator!= ( const SimulationComponent& sc ) const 
         { return ! operator==(sc); }

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
         QVector< int >  reaction_components;   

         // Stoichiometry of components in chemical equation.
         // Positive for reactant, negative for product
         QVector< int >  stoichiometry; 

         bool operator== ( const Association& ) const;      
         inline bool operator!= ( const Association& a ) const 
         { return ! operator==(a); }
      };

   private:

      int  load_db         ( const QString&, US_DB2* );
      int  load_disk       ( const QString& );
      void mfem_scans      ( QXmlStreamReader&, SimulationComponent& );
      void get_associations( QXmlStreamReader&, Association& );
                           
      void write_temp      ( QTemporaryFile& );

      void debug( void );
};
#endif
