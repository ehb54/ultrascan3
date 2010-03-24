// \file us_femglobal.cpp
#include "us_femglobal.h"
#include "us_defines.h"
#include "us_constants.h"

// Read a model system, and the associated constraints needed for
// initialization of the fitting process.  All associated files should start
// with the "filename" string

int US_FemGlobal::read_experiment( struct ModelSystem&          ms, 
                                   struct SimulationParameters& sp, 
                                   const  QString&              filename )
{
   QString fn;
   int     flag1;
   int     flag2;
   QFile   f;

   f.setFileName( filename );
   
   if ( filename.contains( "us_system" ) && f.open( QIODevice::ReadOnly ) )
   {
      QTextStream ts( &f );
      
      ts >> fn;
      flag1 = read_simulationParameters( sp, fn );

      if ( flag1 < 0 )
      {
         qDebug() << filename << ", couldn't read simulation parameters..." 
                  << flag1;
         f.close();
         return flag1;
      }
      
      ts >> fn;
      flag2 = read_modelSystem( ms, fn );
      
      f.close();
      
      if ( flag2 < 0 )
      {
         qDebug() << filename << ", couldn't read models..." << flag2;
         return flag2;
      }

      return 0;
   }

   else if ( filename.contains( "model" ) && f.open( QIODevice::ReadOnly ) )
   {
      QFileInfo fi( filename );
      QString s = fi.absolutePath() + "/" + fi.baseName() + "." 
                  + fi.fileName().right( 2 ) + ".simulation_parameters";

      flag1 = read_simulationParameters( sp, s );
      flag2 = read_modelSystem( ms, filename );

      if ( flag1 < 0 )
      {
         qDebug() << filename << ", couldn't read simulation parameters...";
         return flag1;
      }

      if ( flag2 < 0 )
      {
         qDebug() << filename << ", couldn't read models...";
         return flag2;
      }

      return 0;
   }

   return -200; // Can't read input file
}

int US_FemGlobal::read_experiment( vector< struct ModelSystem >&        vms, 
                                           struct SimulationParameters& sp, 
                                           const  QString&              filename)
{
   QString str;
   int     flag1;
   int     flag2;
   QFile   f;
   
   f.setFileName( filename );
   
   qDebug() << "Read exp msv trying to open " << filename;

   if ( filename.contains( "us_system" ) && f.open( QIODevice::ReadOnly ) )
   {
      QTextStream ts( &f );
      
      ts >> str;
      flag1 = read_simulationParameters( sp, str );
      
      ts >> str;
      flag2 = read_modelSystem( vms, str);

      f.close();
      
      if ( flag1 < 0 )
      {
         qDebug() << filename << ", couldn't read simulation parameters...";
         return flag1;
      }

      if ( flag2 < 0 )
      {
         qDebug()  << filename << ", couldn't read models...";
         return flag2;
      }

      return 0;
   }
   else if ( filename.contains( "model" ) && f.open( QIODevice::ReadOnly ) )
   {
      QFileInfo fi( filename );
      str = fi.absolutePath() + "/" + fi.baseName() + "." 
         + fi.fileName().right( 2 ) + ".simulation_parameters";
      
      flag1 = read_simulationParameters( sp, str );
      flag2 = read_modelSystem( vms, filename );
      
      if ( flag1 < 0 )
      {
         qDebug() << filename << ", couldn't read simulation parameters...";
         return flag1;
      }

      if ( flag2 < 0 )
      {
         qDebug() << filename << ", couldn't read models...";
         return flag2;
      }

      return 0;
   }
      
   return -200; // can't read input file
}

int US_FemGlobal::write_experiment( struct ModelSystem&          ms, 
                                    struct SimulationParameters& sp, 
                                    const  QString&              file )
{
   QFile   f;
   QString str;
   QString filename = file;
   
   if ( filename.right( 10 ) == ".us_system" )
      filename.truncate( filename.length() - 10 );

   f.setFileName( filename + ".us_system" );

   if ( f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      QTextStream ts( &f );
      
      str = filename + ".us_system.simulation_parameters";
      ts << str << " ";

      write_simulationParameters( sp, str );
      
      str.sprintf( ".us_system.model-%d.00", ms.model );
      ts << filename + str << endl;
      
      write_modelSystem( ms, str );
      f.close();
      return 0;
   }

   return -300; // Can't open output file
}

int US_FemGlobal::read_simulationParameters( 
      struct SimulationParameters& sp, 
      const  QString&              filename )
{
   QStringList qsv;
   QFile       f;
   
   f.setFileName( filename );
  
   if ( f.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QString     str;
      QTextStream ts( &f );
      
      while ( ! ( str = ts.readLine() ).isNull() )
      {
         // Removes everything from the whitespace before 
         // the first # to the end of the line
         str.replace( QRegExp( "\\s+#.*" ), "" ); 
         qsv .push_back( str );
      }

      f.close();
      
      return read_simulationParameters( sp, qsv );
   }

   return -72; // Can't open input file
}

int US_FemGlobal::read_simulationParameters( 
      struct SimulationParameters& sp, 
      const  QStringList&          qsv )
{
   int  pos  = 0;

   try
   {
      uint  ival = getInt( qsv, pos++, 51 );

      for ( uint i = 0; i < ival; i++ )
      {
         // Expand the vector as necessary
         if ( sp.speed_step.size() < i + 1 )
         {
            struct SpeedProfile profile;
            sp.speed_step .push_back( profile );
         }

         struct SpeedProfile* pro =  &sp.speed_step[ i ];

         pro->duration_hours    = getUInt  ( qsv, pos++, -52 );
         pro->duration_minutes  = getUInt  ( qsv, pos++, -53 );
         pro->delay_hours       = getUInt  ( qsv, pos++, -54 );
         pro->delay_minutes     = getDouble( qsv, pos++, -55 );
         pro->rotorspeed        = getUInt  ( qsv, pos++, -56 );
         pro->acceleration      = getUInt  ( qsv, pos++, -57 );
         pro->acceleration_flag = getInt   ( qsv, pos++, -58 );
         pro->scans             = getUInt  ( qsv, pos++, -59 );
      }

      sp.simpoints         = getUInt  ( qsv, pos++, -60 );
      sp.radial_resolution = getDouble( qsv, pos++, -61 );
      sp.meniscus          = getDouble( qsv, pos++, -62 );
      sp.bottom            = getDouble( qsv, pos++, -63 );
      sp.rnoise            = getDouble( qsv, pos++, -64 );
      sp.tinoise           = getDouble( qsv, pos++, -65 );
      sp.rinoise           = getDouble( qsv, pos++, -66 );
      sp.mesh              = getUInt  ( qsv, pos++, -67 );
      sp.moving_grid       = getInt   ( qsv, pos++, -68 );
      sp.rotor             = getInt   ( qsv, pos++, -70 );
      sp.band_forming      = ( getInt ( qsv, pos++, -71 ) == 1 ) 
         ? true : false;

      sp.band_volume       = getDouble( qsv, pos++, -71 );
   } catch ( int error )
   {
      return error;
   }

   return 0;
}

int US_FemGlobal::write_simulationParameters( 
      struct SimulationParameters& sp, 
      const  QString&              filename )
{
   QString str = QString( filename );
   QFile   f( str );
   
   if ( f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      QTextStream ts( &f );

      ts << sp.speed_step.size() 
         << "\t\t# Number of speed step profiles" << "\n";
      
      for ( uint i = 0; i< sp.speed_step.size(); i++ )
      {
         ts << sp.speed_step[ i ].duration_hours 
            << str.sprintf("\t\t# run duration hours for profile %d\n", 
                           i + 1 );
         
         ts << sp.speed_step[ i ].duration_minutes 
            << str.sprintf("\t\t# run duration minutes for profile %d\n", 
                           i + 1 );
         
         ts << sp.speed_step[ i ].delay_hours 
            << str.sprintf("\t\t# run delay hours for profile %d\n", 
                           i + 1 );
         
         ts << sp.speed_step[ i ].delay_minutes 
            << str.sprintf("\t\t# run delay minutes for profile %d\n", 
                           i + 1 );
         
         ts << sp.speed_step[ i ].rotorspeed 
            << str.sprintf("\t\t# rotor speed for profile %d\n", i + 1 );
         
         ts << sp.speed_step[ i ].acceleration 
            << str.sprintf("\t\t# acceleration profile in revs/sec for "
                           "profile %d\n", i + 1 );
         
         ts << (int) sp.speed_step[ i ].acceleration_flag 
            << str.sprintf("\t\t# flag for checking if rotor acceleration is "
                           "used for profile %d\n", i + 1 );
         
         ts << sp.speed_step[i].scans 
            << str.sprintf("\t\t# number of scans to save for profile %d\n", 
                           i + 1 );
      }

      ts << sp.simpoints 
         << "\t\t# radial discretization simulation points" << "\n";
      
      ts << sp.radial_resolution << "\t\t# radial resolution (cm)" << "\n";
      ts << sp.meniscus << "\t\t# meniscus position (cm)" << "\n";
      ts << sp.bottom << "\t\t# bottom of cell position (cm)" << "\n";
      ts << sp.rnoise << "\t\t# random noise (in percent OD)" << "\n";
      
      ts << sp.tinoise 
         << "\t\t# time invariant systematic noise (in percent OD)" << "\n";
      
      ts << sp.rinoise 
         << "\t\t# radial invariant systematic noise (in percent OD)" << "\n";
      
      ts << sp.mesh 
         << "\t\t# radial grid" << "(0 = adaptive (ASTFEM), "
            "1 = fixed (Claverie), 2 = moving hat (sedfit), "
            "3 = file: $ULTRASCAN/mesh.dat)\n";
      
      ts << sp.moving_grid 
         << "\t\t# time grid (0 = fixed (Claverie), 1 = adaptive (ASTFEM)" 
         << "\n";
      
      ts << sp.rotor << "\t\t# Rotor serial number" << "\n";
      
      if ( sp.band_forming )
      {
         ts << "1\t\t# Band-forming centerpiece is used\n";
         ts << sp.band_volume << "\t\t# Band loading volume in ml" << "\n";
      }
      else
      {
         ts << "0\t\t# Standard centerpiece is used\n";
      }
      
      ts << sp.band_volume << "\t\t# band loading volume\n";
      
      f.close();
      return 0;
   }

   return -1000;
}

int US_FemGlobal::read_modelSystem( struct ModelSystem& ms, 
                                    const  QString&     filename, 
                                    bool                flag )
{
   QFile f( filename );
   
   if ( f.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QTextStream ts( &f );
      QString     s;
      QStringList list;
      
      while ( ! ( s = ts.readLine() ).isNull() )
      {
         // Removes everything from the whitespace before the 
         // first # to the end of the line
         s.replace( QRegExp( "\\s+#.*" ), "" ); 
         list .push_back( s );
      }

      f.close();
      return( read_modelSystem( ms, list, flag ) );
   }
  
   return( -40 ); // Can't open input file
}

int US_FemGlobal::read_modelSystem( vector< ModelSystem >& modelList, 
                                     const QString&        filename )
{
   QFile       f( filename );
   
   modelList.clear();

   if ( f.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QStringList            list;
      QTextStream            ts( &f );
      vector< unsigned int > offset;
      QString                s;
      
      while ( ! ( s = ts.readLine() ).isNull() )
      {
         // Removes everything from the whitespace before 
         // the first # to the end of the line
         s.replace( QRegExp( "\\s+#.*"), "" ); 
         list .push_back( s );

         if ( s == "#!__Begin_ModelSystem__!" )
            offset .push_back( list.size() );
      }

      f.close();

      int         retval;
      ModelSystem ms;

      if ( offset.size() )
      {
         for ( uint i = 0; i < offset.size(); i++ )
         {
            retval = read_modelSystem( ms, list, false, offset[ i ] );

            if ( retval < 0 )
            {
               qDebug() << "Reading modelsystem " << i 
                        << " failed with return value: " << retval;
            }

            modelList .push_back( ms );

            if ( retval ) return retval;
         }
      }
      else
      {
         retval = read_modelSystem( ms, list, false, 0 );
         
         if ( retval < 0 )
         {
            qDebug() << "Reading modelsystem failed with return value: " 
                     << retval;
            return retval;
         }

         modelList .push_back( ms );
      }

      return( retval );
   }

   return -40; // Can't open input file
}

int US_FemGlobal::read_modelSystem( struct ModelSystem& ms, 
                                    const  QStringList& qsv, 
                                    bool                flag, 
                                    int                 offset )
{
   QString str;
   int     pos = 0;
   
   // If we are reading this model as part of a different file, we need
   // to find the beginning point first so the file is correctly offset.
   if ( flag )
   { 
      while ( str != "#!__Begin_ModelSystem__!" && pos < qsv.size() )
          str = qsv[ pos++ ];
   }

   // We are given an offset position to start in the string
   if ( offset ) pos = offset;

   pos++; // FE, SA2D, COFS, SIM or GA
   
   try
   {
      ms.description = getString( qsv, pos++, -100 );
      
      // A new model has a comment line in the second line starting with "#" 
      str = getString( qsv, pos++, -1 );

      if ( str.indexOf( "#", 0 ) == 0 )
      {
         pos++;  // UltraScan Version

         ms.model = getInt( qsv, pos++, -2 );

         uint vectors =  getInt( qsv, pos++, -3 );  
         
         for ( uint i = 0; i < vectors; i++ )
         {
            // Expand the vector as necessary
            if ( ms.component_vector.size() < i + 1 )
            {
               struct SimulationComponent sc;
               ms.component_vector .push_back( sc );
            }

            struct SimulationComponent* cv = &ms.component_vector[ i ];

            cv->name = getString( qsv, pos++, -3 ).trimmed();
            if ( cv->name.isNull() ) return -4;

            cv->concentration = getDouble( qsv, pos++, -5 );
            cv->s             = getDouble( qsv, pos++, -6 );
            cv->D             = getDouble( qsv, pos++, -7 );
            cv->sigma         = getDouble( qsv, pos++, -8 );
            cv->delta         = getDouble( qsv, pos++, -9 );
            cv->mw            = getDouble( qsv, pos++, -10 );
            cv->vbar20        = getDouble( qsv, pos++, -11 );
            
            cv->shape         = getString( qsv, pos++, -12 );
            if ( cv->shape.isNull() ) return -12;

            cv->f_f0          = getDouble    ( qsv, pos++, -13 );
            cv->extinction    = getDouble    ( qsv, pos++, -14 );
            cv->show_conc     = (bool) getInt( qsv, pos++, -15 );
            cv->show_stoich   = getInt       ( qsv, pos++, -16 );
            cv->show_keq      = (bool) getInt( qsv, pos++, -17 );
            cv->show_koff     = (bool) getInt( qsv, pos++, -18 );
            
            uint components = getInt( qsv, pos++, -19 );
            
            for ( uint j = 0; j < components; j++ )
            {
               // Expand the vector as necessary
               if ( cv->show_component.size() < j + 1 )
               {
                  uint x;;
                  cv->show_component .push_back( x );
               }

               cv->show_component[ j ] = getInt( qsv, pos++, -20 );
            }

            if ( cv->concentration < 0 )
            {
               cv->c0.radius.clear();
               cv->c0.concentration.clear();

               uint ival = getInt( qsv, pos++, -21 );

               for ( uint j = 0; j < ival; j++ )
               {
                  cv->c0.radius        .push_back( getDouble( qsv, pos++, -22 ) );
                  cv->c0.concentration .push_back( getDouble( qsv, pos++, -23 ) );
               }
            }
         }

         uint assoc_vectors = getInt( qsv, pos++, -24 );

         for ( uint i = 0; i < assoc_vectors; i++ )
         {
            // Expand the vector as necessary
            if ( ms.assoc_vector.size() < i + 1 )
            {
               struct Association a;
               ms.assoc_vector .push_back( a );
            }
            
            ms.assoc_vector[ i ].keq            = getDouble( qsv, pos++, -25 );
            ms.assoc_vector[ i ].units          = getString( qsv, pos++, -26 );
            if ( ms.assoc_vector[ i ].units.isNull() ) return -26;

            ms.assoc_vector[ i ].k_off          = getDouble( qsv, pos++, -27 );
            ms.assoc_vector[ i ].component1     = getInt   ( qsv, pos++, -28 );
            ms.assoc_vector[ i ].component2     = getInt   ( qsv, pos++, -29 );
            ms.assoc_vector[ i ].component3     = getInt   ( qsv, pos++, -30 );
            ms.assoc_vector[ i ].stoichiometry1 = getUInt  ( qsv, pos++, -31 );
            ms.assoc_vector[ i ].stoichiometry2 = getUInt  ( qsv, pos++, -32 );
            ms.assoc_vector[ i ].stoichiometry3 = getUInt  ( qsv, pos++, -33 );
         }

         return 0;
      }
      else // Load an old-style model file for noninteracting models
      {
         ms.model = str.toInt();
         
         // We can only read noninteracting models
         // Set to fixed molecular weight distribution by default
         if ( ms.model > 3) ms.model = 3; 
         
         uint components = getInt( qsv, pos++, -34 );
         
         for ( uint i = 0; i < components; i++ )
         {
            // Expand the vector as necessary
            if ( ms.component_vector.size() < i + 1 )
            {
               struct SimulationComponent sc;
               ms.component_vector .push_back( sc );
            }

            struct SimulationComponent* cv = &ms.component_vector[ i ];

            cv->concentration = getDouble( qsv, pos++, -35 );
            cv->s             = getDouble( qsv, pos++, -36 );
            cv->D             = getDouble( qsv, pos++, -37 );
            cv->sigma         = getDouble( qsv, pos++, -38 );
            cv->delta         = getDouble( qsv, pos++, -39 );

            
            cv->vbar20      = (float) 0.72;
            cv->extinction  = 1.0;
            cv->name        = "Component " + QString::number( i + 1 );
            cv->mw          = ( cv->s / cv->D ) * 
                              ( ( R * K20 ) / ( 1.0 - cv->vbar20 * DENS_20W ) );
            
            double base     = ( cv->mw * M_PI * M_PI * 3.0 * cv->vbar20 ) /
                              ( 4.0 * AVOGADRO );

            cv->f_f0        = ( ( cv->mw * ( 1.0 - cv->vbar20 * DENS_20W ) )
                                / ( cv->s * AVOGADRO ) ) /
                              ( 6 * VISC_20W * pow ( base, 1.0/3.0 ) );

            cv->show_conc   = true;
            cv->show_keq    = false;
            cv->show_koff   = false;
            cv->show_stoich = 0;
         }
      }

      return 1; // Loaded an old-style model

   } catch ( int error )
   {
      return error;
   }
}

double US_FemGlobal::getDouble( const QStringList& list, int position, int error )
{
   if ( position > list.size() || list[ position ].isNull() ) throw error;
   return list[ position ].toDouble(); 
}

QString US_FemGlobal::getString( const QStringList& list, int position, int error )
{
   if ( position > list.size() || list[ position ].isNull() ) throw error;
   return list[ position ]; 
}

int US_FemGlobal::getInt( const QStringList& list, int position, int error )
{
   if ( position > list.size() || list[ position ].isNull() ) throw error;
   return list[ position ].toInt(); 
}

quint32 US_FemGlobal::getUInt( const QStringList& list, int position, int error )
{
   if ( position > list.size() || list[ position ].isNull() ) throw error;
   return list[ position ].toUInt(); 
}


int US_FemGlobal::write_modelSystem( struct ModelSystem& ms, 
                                     const  QString&     filename, 
                                     bool                flag )
{
   QFile f( filename );

   if ( flag )
   {
      if ( ! f.open( QIODevice::WriteOnly | QIODevice::Text | 
                     QIODevice::Append ) ) return -35;
   }
   else
   {
      if ( ! f.open( QIODevice::WriteOnly | QIODevice::Text ) ) return -35;
   }

   QTextStream ts( &f );
   
   if ( flag )
      ts << "#!__Begin_ModelSystem__!\n";

   ts << "SIM" << "\n";
   ts << "Model written by US_FEMGLOBAL\n";
   ts << "# This file is computer-generated, please do not edit "
          "unless you know what you are doing\n";

   ts << US_Version << "\t\t# UltraScan Version Number\n";
   ts << ms.model   << "\t\t# model number/identifier\n";

   ts << ms.component_vector.size() 
      << "\t\t# number of components in the model\n";
   
   for ( uint i = 0; i < ms.component_vector.size(); i++ )
   {
      struct SimulationComponent* cv = &ms.component_vector[ i ];

      ts << cv->name                  << "\t\t# name of component\n";
      ts << cv->concentration         << "\t\t# concentration\n";
      ts << cv->s                     << "\t\t# sedimentation coefficient\n";
      ts << cv->D                     << "\t\t# diffusion coefficient\n";
      ts << cv->sigma                 << "\t\t# sigma\n";
      ts << cv->delta                 << "\t\t# delta\n";
      ts << cv->mw                    << "\t\t# molecular Weight \n";
      ts << cv->vbar20                << "\t\t# vbar at 20C \n";
      ts << cv->shape                 << "\t\t# shape \n";
      ts << cv->f_f0                  << "\t\t# frictional ratio \n";
      ts << cv->extinction            << "\t\t# extinction\n";
      ts << (int) cv->show_conc       << "\t\t# show concentration?\n";
      ts << cv->show_stoich           << "\t\t# show Stoichiometry?\n";
      ts << (int) cv->show_keq        << "\t\t# show k equilibrium?\n";
      ts << (int) cv->show_koff       << "\t\t# show k_off?\n";
      ts << cv->show_component.size() << "\t\t# number of linked components\n";
      
      for ( uint j = 0; j < cv->show_component.size(); j++ )
      {
         ts << cv->show_component[ j ] 
            << "\t\t# linked component (" << j + 1 << ")" << endl;;
      }

      if ( cv->concentration < 0 )
      {
         ts << cv->c0.radius.size() 
            << "\t\t# number of initial concentration points\n";
         
         for ( uint j = 0; j < cv->c0.radius.size(); j++ )
         {
            ts << cv->c0.radius       [ j ] << " "
               << cv->c0.concentration[ j ] << endl;
         }
      }
   }

   ts << ms.assoc_vector.size() 
      << "\t\t# number of association reactions in the model\n";
   
   for ( uint i = 0; i < ms.assoc_vector.size(); i++ )
   {
      struct Association* av = &ms.assoc_vector[ i ];
      
      ts << av->keq        << "\t\t# equilibrium constant\n";
      ts << av->units      << "\t\t# units for equilibrium constant\n";
      ts << av->k_off      << "\t\t# rate constant\n";
      ts << av->component1 << "\t\t# component 1 in this association\n";
      ts << av->component2 << "\t\t# component 2 in this association\n";
      ts << av->component3 << "\t\t# component 3 in this association\n";

      ts << av->stoichiometry1 
         << "\t\t# stoichiometry for component 1 in this association\n";
      
      ts << av->stoichiometry2 
         << "\t\t# stoichiometry for component 2 in this association\n";
      
      ts << av->stoichiometry3 
         << "\t\t# stoichiometry for component 3 in this association\n";
   }

   f.close();
   return 0;
}



// Read a model system, and the associated constraints needed for
// initialization of the fitting process.  All associated files should start
// with the "filename" string

int US_FemGlobal::read_constraints( struct ModelSystem&            ms, 
                                    struct ModelSystemConstraints& msc, 
                                    const  QString&                filename )
{
   QString     s;
   QStringList qsv;
   QFile       f;
   
   f.setFileName( filename );
   
   if ( f.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QTextStream ts( &f );
      
      while ( ( s = ts.readLine() ).isNull() )
      {
         // Removes everything from the whitespace before the 
         // first # to the end of the line
         s.replace( QRegExp( "\\s+#.*" ), "" ); 
         qsv .push_back( s );
      }

      f.close();
      
      return( read_constraints( ms, msc, qsv ) );
   }

   return -200; // Can't open input file
}

int US_FemGlobal::read_constraints( struct ModelSystem&            ms, 
                                    struct ModelSystemConstraints& msc, 
                                    const  QStringList&            qsv )
{
   int pos = 0;

   try
   {
      uint j = getInt( qsv, pos++, -2 );
      //msc.component_vector_constraints.resize( j );

      for ( uint i = 0; i < j; i++ )
      {
         // Expand the vector as necessary
         if ( msc.component_vector_constraints.size() < i + 1 )
         {
            struct SimulationComponentConstraints scc;
            msc.component_vector_constraints .push_back( scc );
         }

         struct SimulationComponentConstraints* cc = 
            &msc.component_vector_constraints[ i ];
         
         cc->vbar20.fit         = getUInt  ( qsv, pos++, -3 );
         cc->vbar20.low         = getDouble( qsv, pos++, -4 );
         cc->vbar20.high        = getDouble( qsv, pos++, -5 );
         cc->mw.fit             = getUInt  ( qsv, pos++, -6 );
         cc->mw.low             = getDouble( qsv, pos++, -7 );
         cc->mw.high            = getDouble( qsv, pos++, -8 );
         cc->s.fit              = getUInt  ( qsv, pos++, -9 );
         cc->s.low              = getDouble( qsv, pos++, -10 );
         cc->s.high             = getDouble( qsv, pos++, -11 );
         cc->D.fit              = getUInt  ( qsv, pos++, -12 );
         cc->D.low              = getDouble( qsv, pos++, -13 );
         cc->D.high             = getDouble( qsv, pos++, -14 );
         cc->sigma.fit          = getUInt  ( qsv, pos++, -15 );
         cc->sigma.low          = getDouble( qsv, pos++, -16 );
         cc->sigma.high         = getDouble( qsv, pos++, -17 );
         cc->delta.fit          = getUInt  ( qsv, pos++, -18 );
         cc->delta.low          = getDouble( qsv, pos++, -19 );
         cc->delta.high         = getDouble( qsv, pos++, -20 );
         cc->concentration.fit  = getUInt  ( qsv, pos++, -21 );
         cc->concentration.low  = getDouble( qsv, pos++, -22 );
         cc->concentration.high = getDouble( qsv, pos++, -23 );
         cc->f_f0.fit           = getUInt  ( qsv, pos++, -24 );
         cc->f_f0.low           = getDouble( qsv, pos++, -25 );
         cc->f_f0.high          = getDouble( qsv, pos++, -26 );
      }

      if ( pos >= qsv.size() ) return -27;
      
      // The number of reactions in this model
      uint reactions = getUInt( qsv, pos++, -27 ); 
      
      for ( uint i = 0; i < reactions; i++ )
      {
         // Expand the vector as necessary
         if ( msc.assoc_vector_constraints.size() < i + 1 )
         {
            struct AssociationConstraints assoc;
            msc.assoc_vector_constraints .push_back( assoc );
         }

         struct AssociationConstraints* ac = &msc.assoc_vector_constraints[ i ];

         ac->keq.fit   = getUInt  ( qsv, pos++, -28 );
         ac->keq.low   = getDouble( qsv, pos++, -29 );
         ac->keq.high  = getDouble( qsv, pos++, -30 );
         ac->koff.fit  = getUInt  ( qsv, pos++, -31 );;
         ac->koff.low  = getDouble( qsv, pos++, -32 );
         ac->koff.high = getDouble( qsv, pos++, -33 );
      }

      msc.simpoints   = getUInt  ( qsv, pos++, -34 );
      msc.mesh        = getUInt  ( qsv, pos++, -35 );
      msc.moving_grid = getUInt  ( qsv, pos++, -36 );
      msc.band_volume = getDouble( qsv, pos++, -37 );

   } catch( int error )
   {
      return error;
   }
   
   int flag1 = read_modelSystem( ms, qsv, true );
   if ( flag1 < 0 ) return flag1;

   return 0;
}

// Write a model system, and the associated constraints needed for
// initialization of the fitting process.  all associated files should start
// with the "filename" string

int US_FemGlobal::write_constraints( struct ModelSystem&            ms, 
                                     struct ModelSystemConstraints& msc, 
                                     const QString&                 file )
{
   QFile        f;
   QString      str;
   QString      filename = file;

   if ( filename.right( 12 ) != ".constraints" )
      filename += ".constraints";

   f.setFileName( filename );
   
   if ( f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      QTextStream ts( &f );

      ts << msc.component_vector_constraints.size() 
         << "\t\t# Number of components in the model\n";
      
      for ( uint i = 0; i < msc.component_vector_constraints.size(); i++ )
      {
         struct SimulationComponentConstraints* cvs = 
            &msc.component_vector_constraints[ i ];

         ts << cvs->vbar20.fit         << "\t\t# Is the vbar20 of component " 
                                       << i + 1 << " fitted?\n";
         ts << cvs->vbar20.low         << "\t\t# vbar20 lower limit\n";
         ts << cvs->vbar20.high        << "\t\t# vbar20 upper limit\n";
         ts << cvs->mw.fit             << "\t\t# Is molecular weight of component " 
                                       << i + 1 << " fitted?\n";
         ts << cvs->mw.low             << "\t\t# molecular weight lower limit\n";
         ts << cvs->mw.high            << "\t\t# molecular weight upper limit\n";
         ts << cvs->s.fit              << "\t\t# Is the sedimentation coefficient of component " 
                                       << i + 1 << " fitted?\n";
         ts << cvs->s.low              << "\t\t# sedimentation coefficient lower limit\n";
         ts << cvs->s.high             << "\t\t# sedimentation coefficient upper limit\n";
         ts << cvs->D.fit              << "\t\t# Is the diffusion coefficient of component " 
                                       << i + 1 << " fitted?\n";
         ts << cvs->D.low              << "\t\t# diffusion coefficient lower limit\n";
         ts << cvs->D.high             << "\t\t# diffusion coefficient upper limit\n";
         ts << cvs->sigma.fit          << "\t\t# Is concentration dependency of s of component " 
                                       << i + 1 << " fitted?\n";
         ts << cvs->sigma.low          << "\t\t# concentration dependency of s lower limit\n";
         ts << cvs->sigma.high         << "\t\t# concentration dependency of s upper limit\n";
         ts << cvs->delta.fit          << "\t\t# Is concentration dependency of D of component " 
                                       << i + 1 << " fitted?\n";
         ts << cvs->delta.low          << "\t\t# concentration dependency of D lower limit\n";
         ts << cvs->delta.high         << "\t\t# concentration dependency of D upper limit\n";
         ts << cvs->concentration.fit  << "\t\t# Is the concentration of component " 
                                       << i+1 << " fitted?\n";
         ts << cvs->concentration.low  << "\t\t# concentration lower limit\n";
         ts << cvs->concentration.high << "\t\t# concentration upper limit\n";
         ts << cvs->f_f0.fit           << "\t\t# Is the frictional ratio of component " 
                                       << i + 1 << " fitted?\n";
         ts << cvs->f_f0.low           << "\t\t# frictional ratio lower limit\n";
         ts << cvs->f_f0.high          << "\t\t# frictional ratio upper limit\n";
      }
      
      ts << msc.assoc_vector_constraints.size() 
         << "\t\t# Number of reactions in the model\n";
      
      for ( uint i = 0; i < msc.assoc_vector_constraints.size(); i++ )
      {
         struct AssociationConstraints* avc = &msc.assoc_vector_constraints[ i ];

         ts << avc->keq.fit   << "\t\t# Is the equilibrium constant of association reaction " 
                              << i + 1 << " fitted?\n";
         ts << avc->keq.low   << "\t\t# equilibrium constant lower limit\n";
         ts << avc->keq.high  << "\t\t# equilibrium constant upper limit\n";
         ts << avc->koff.fit  << "\t\t# Is the k_off rate constant of association reaction " 
                              << i + 1 << " fitted?\n";
         ts << avc->koff.low  << "\t\t# k_off rate constant lower limit\n";
         ts << avc->koff.high << "\t\t# k_off rate constant upper limit\n";
      }

      ts << msc.simpoints << "\t\t# the number of grid points\n";
      ts << msc.mesh 
         << "\t\t# the type of radial mesh used: 0 = ASTFEM, 1 = Claverie, "
            "2 = moving hat, 3 = user-selected mesh, "
            "4 = nonuniform constant mesh\n";
      
      ts << msc.moving_grid 
         << "\t\t# using moving (0) or fixed time grid (1)\n";
      
      ts << msc.band_volume 
         << "\t\t# loading volume (of lamella) in a band-forming "
            "centerpiece, if used\n";
      
      f.close();
      
      // Append the corresponding model to the constraints file
      write_modelSystem( ms, filename, true ); 
      str.sprintf( ".model-%d.00", ms.model );
      
      // Write the corresponding model to a separate file 
      write_modelSystem( ms, filename + str ); 
      
      return 0;
   }
      
   return -400; // Can't open output file
}

int US_FemGlobal::read_model_data( vector< mfem_data >& model, 
                                   const QString&       filename, 
                                   bool                 ignore_errors ) 
{
  unsigned int no_of_models;
  unsigned int no_of_radial_points;
  unsigned int no_of_scans;

  double double_val;

  QFile f( filename );

  if ( ! f.open( QIODevice::ReadOnly ) )
  {
    if ( ! ignore_errors )
    {
      qDebug() << "Could not open data file: " << filename << " for input";
      qDebug() << "Check the path, file name, and read permissions...";
    }

    return -1;
  }

  model.clear();

  struct mfem_data temp_model;
  struct mfem_scan temp_scan;
  
  vector< double > concentration;

  QDataStream ds( &f );

  ds >> no_of_models;

  qDebug() << "no of models: " << no_of_models;

  for ( quint32 i = 0; i < no_of_models; i++ )
  {
    ds >> temp_model.id;
    ds >> temp_model.cell;
    ds >> temp_model.channel;
    ds >> temp_model.wavelength;
    ds >> temp_model.meniscus;
    ds >> temp_model.bottom;
    ds >> temp_model.rpm;
    ds >> temp_model.s20w_correction;
    ds >> temp_model.D20w_correction;
    ds >> no_of_radial_points;
    
    temp_model.radius.clear();

    for ( quint32 j = 0; j < no_of_radial_points; j++ )
    {
      ds >> double_val;
      temp_model.radius .push_back( double_val );
    }
    
    ds >> no_of_scans;
    temp_model.scan.clear();
    
    for ( quint32 j = 0; j < no_of_scans; j++ )
    {
      ds >> temp_scan.time;
      ds >> temp_scan.omega_s_t;
      temp_scan.conc.clear();
      
      for ( uint k = 0; k < temp_model.radius.size(); k++ )
      {
         ds >> double_val;
         temp_scan.conc.push_back(double_val);
      }

      temp_model.scan.push_back(temp_scan);
    }

    ds >> temp_model.viscosity;
    ds >> temp_model.density;
    ds >> temp_model.vbar;
    ds >> temp_model.vbar20;
    ds >> temp_model.avg_temperature;

#if defined( DEBUG_HYDRO )

    QString s;
    s.sprintf("model time %g avg_temp %.12g vbar %.12g vbar20 %.12g "
           "visc %.12g density %.12g\n",
      temp_model.scan[ temp_model.scan.size() - 1 ].time,
      temp_model.avg_temperature,
      temp_model.vbar,
      temp_model.vbar20,
      temp_model.viscosity,
      temp_model.density );

    qDebug() < s;
#endif

    model .push_back( temp_model );
  }

  return 0;
}

int US_FemGlobal::write_model_data( vector< mfem_data >& model, 
                                    const QString&       filename) 
{
  QFile f( filename );
  
  if ( ! f.open( QIODevice::WriteOnly ) )
  {
    qDebug() << "Could not open data file: " << filename << " for output";
    qDebug() << "Please check the path, file name, and write permissions...";
    return -1;
  }

  QDataStream ds( &f );
  
  ds << (uint)model.size();
  
  for ( uint i = 0; i < model.size(); i++ )
  {
    ds << model[ i ].id;
    ds << model[ i ].cell;
    ds << model[ i ].channel;
    ds << model[ i ].wavelength;
    ds << model[ i ].meniscus;
    ds << model[ i ].bottom;
    ds << model[ i ].rpm;
    ds << model[ i ].s20w_correction;
    ds << model[ i ].D20w_correction;
    
    ds << (uint)model[ i ].radius.size();
    
    for ( uint j = 0; j < model[ i ].radius.size(); j++ )
      ds << model[ i ].radius[ j ];

    ds << (uint)model[ i ].scan.size();

    for ( uint j = 0; j < model[ i ].scan.size(); j++ )
    {
      ds << model[ i ].scan[ j ].time;
      ds << model[ i ].scan[ j ].omega_s_t;
      
      for ( uint k = 0; k < model[ i ].radius.size(); k++ )
         ds << model[ i ].scan[ j ].conc[ k ];
    }

    QString s;
    s.sprintf( "model last time %g avg_temp %g vbar %g visc %g density %g "
               "vbar %g vbar20 %g rpm %u bottom %g meniscus %g scorr %g "
               "Dcorr %g\n",
                   model[ i ].scan[ model[ i ].scan.size() - 1 ].time,
                   model[ i ].avg_temperature,
                   model[ i ].vbar,
                   model[ i ].viscosity,
                   model[ i ].density,
                   model[ i ].vbar,
                   model[ i ].vbar20,
                   model[ i ].rpm,
                   model[ i ].bottom,
                   model[ i ].meniscus,
                   model[ i ].s20w_correction,
                   model[ i ].D20w_correction);
    qDebug() << s;

    ds << model[ i ].viscosity;
    ds << model[ i ].density;
    ds << model[ i ].vbar;
    ds << model[ i ].vbar20;
    ds << model[ i ].avg_temperature;
  }

  f.close();
  return 0;
}

int US_FemGlobal::accumulate_model_monte_carlo_data(
      vector< mfem_data >& accumulated_model, 
      vector< mfem_data >& source_model, 
      unsigned int         monte_carlo_iterations )
{
  if ( ! monte_carlo_iterations )
  {
    qDebug() << "Notice: US_FemGlobal::accumulate_model_monte_carlo_data monte "
                "carlo iterations == 0, setting to 1";
    
    monte_carlo_iterations = 1;
  }

  if ( accumulated_model.size() &&
       accumulated_model.size() != source_model.size() )
  {
    qDebug() << 
       "Internal Error: US_FemGlobal::accumulate_model_monte_carlo_data "
       "model size incompability";

    return -2;
  }

  if ( ! accumulated_model.size() )
  {
    struct mfem_data temp_model;

    for ( uint i = 0; i < source_model.size(); i++ )
    {
      temp_model = source_model[ i ];

      for ( uint j = 0; j < temp_model.scan.size(); j++ )
      {
         for ( uint k = 0; k < temp_model.radius.size(); k++ )
         {
            temp_model.scan[ j ].conc[ k ] /= (double) monte_carlo_iterations;
         }
      }

      accumulated_model .push_back( temp_model );
    }

    return 0;
  }

  for ( uint i = 0; i < source_model.size(); i++ )
  {
    if ( accumulated_model[ i ].scan.size() != source_model[ i ].scan.size() )
    {
      qDebug() << 
         "Internal Error: US_FemGlobal::accumulate_model_monte_carlo_data "
         "scan size incompability";

      return ( -3 );
    }

    if ( accumulated_model[ i ].radius.size() != 
          source_model[ i ].radius.size() )
    {
      qDebug() << 
         "Internal Error: US_FemGlobal::accumulate_model_monte_carlo_data "
         "radius size incompability";

      return -4;
    }

    if ( accumulated_model[ i ].radius != source_model[ i ].radius )
    {
      qDebug() << 
         "Internal Error: US_FemGlobal::accumulate_model_monte_carlo_data "
         "radius value incompability\n";
      
      return -5;
    }

    for ( uint j = 0; j < source_model[ i ].scan.size(); j++ )
    {
      for ( uint k = 0; k < source_model[ i ].radius.size(); k++ )
      {
         accumulated_model[ i ].scan[ j ].conc[ k ] 
           += source_model[ i ].scan[ j ].conc[ k ] 
              / (double) monte_carlo_iterations;
      }
    }
  }

  return 0;
}

int US_FemGlobal::read_mwl_model_data( vector< mfem_data >& model, 
                                       const QString&       filenamebase )
{
  unsigned int no_of_models_loaded = 0;
  
  vector< mfem_data > temp_model;
  
  for ( int i = 0; i < 2048; i++ )
  {
    if ( ! read_model_data( temp_model, 
             filenamebase + QString( "-model-%1.dat" ).arg ( i ), true ) )
    {
      model .push_back( temp_model[ 0 ] );
      no_of_models_loaded++;
    }
  }

  return no_of_models_loaded;
}

