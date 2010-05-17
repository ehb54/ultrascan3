#include "model.h"
#include <uuid/uuid.h>

//int main( int argc, char* argv[] )
int main( )
{
   US_FemGlobal::ModelSystem model;

   uchar uuid[ 16 ];
   uuid_generate( uuid );

   // Initialize model
   model.description     = "Test model description";
   model.compressibility = 0.99;
   
   US_FemGlobal::SimulationComponent b1;
   
   memcpy( b1.analyteGUID, uuid, 16 );
   b1.molar_concentration  = 1.3;
   b1.signal_concentration = 1.0;
   b1.vbar20               = 0.72;
   b1.mw                   = 1.0e4;
   b1.s                    = 0.0;
   b1.D                    = 0.0;
   b1.sigma                = 0.0;
   b1.delta                = 0.0;
   b1.f_f0                 = 2.0;
   b1.shape                = "OBLATE";
   b1.name                 = "Test Buffer";

   model.coSedSolute = b1;

   uuid_generate( uuid );

   US_FemGlobal::SimulationComponent c1;
   memcpy( c1.analyteGUID, uuid, 16 );
   c1.molar_concentration  = 1.0;
   c1.signal_concentration = 1.0;
   c1.vbar20               = 0.72;
   c1.mw                   = 1.0e5;
   c1.s                    = 0.0;
   c1.D                    = 0.0;
   c1.sigma                = 0.0;
   c1.delta                = 0.0;
   c1.f_f0                 = 1.0;
   c1.shape                = "SPHERE";
   c1.name                 = "Test Component 1";

   for ( int i = 0; i < 20; i++ )
   {
      c1.c0.radius       .push_back( 5.8  + .001 * i );
      c1.c0.concentration.push_back( 0.12 + .007 * i );
   }

   model.components.push_back( c1 );


   US_FemGlobal::Association a;
   a.keq   = 1.0;
   a.k_off = 1.0;

   a.reaction_components.push_back( 0 );
   a.reaction_components.push_back( 1 );

   a.stoichiometry.push_back( -1 );
   a.stoichiometry.push_back(  1 );

   model.associations.push_back( a );

   US_FemGlobal::write_model( "mymodel.xml", model );
   return 0;
}

int US_FemGlobal::write_model( const QString&                   filename,
                               const US_FemGlobal::ModelSystem& model )
{
   QFile f( filename );

   if ( ! f.open( QFile::WriteOnly | QFile::Text | QIODevice::Truncate ) )
   {
      qDebug() << "Could not open file for writing";
      return -1;
   }

   QTextStream ts( &f );

   QDomDocument doc( "UltraScanModel" );

   QDomElement root = doc.createElement( "model" );
   root.setAttribute( "compressibility", model.compressibility );
   root.setAttribute( "description"    , model.description );

   doc.appendChild( root );

   root.appendChild( dom_sc( model.coSedSolute, "coSedSolute", doc ) );

   US_FemGlobal::SimulationComponent sc;

   foreach( sc, model.components )
      root.appendChild( dom_sc( sc, "component", doc ) );

   Association assoc;

   foreach( assoc, model.associations )
      root.appendChild( dom_assoc( assoc, doc ) );

   const int IndentSize = 4;
   doc.save( ts, IndentSize );
   
   f.close();
   return 0;
}

QDomElement dom_sc( const US_FemGlobal::SimulationComponent& sc, 
                    const QString&                           type, 
                    QDomDocument&                            doc )
{
   QDomElement element = doc.createElement( "component" );
   element.setAttribute( "type"               , type );

   char ascii_uuid[ 37 ];
   uuid_unparse( sc.analyteGUID, ascii_uuid );
   element.setAttribute( "uuid", ascii_uuid );

   element.setAttribute( "molar_concentration" , sc.molar_concentration );
   element.setAttribute( "signal_concentration", sc.signal_concentration );
   element.setAttribute( "vbar20"              , sc.vbar20 );
   element.setAttribute( "mw"                  , sc.mw );
   element.setAttribute( "s"                   , sc.s );
   element.setAttribute( "D"                   , sc.D );
   element.setAttribute( "sigma"               , sc.sigma );
   element.setAttribute( "delta"               , sc.delta );
   element.setAttribute( "f_f0"                , sc.f_f0 );
   element.setAttribute( "shape"               , sc.shape );
   element.setAttribute( "name"                , sc.name );

   if ( sc.c0.radius.size() > 0 )
   {
      QDomElement c0 = doc.createElement( "MfemInitial" );
      
      for ( uint i = 0; i < sc.c0.radius.size(); i++ )
      {
         QDomElement value = doc.createElement( "value" );
         value.setAttribute( "radius", sc.c0.radius       [ i ] );
         value.setAttribute( "conc"  , sc.c0.concentration[ i ] );
         c0.appendChild( value );
      }

      element.appendChild( c0 );
   }

   return element;
}

QDomElement dom_assoc( const US_FemGlobal::Association& assoc, 
                       QDomDocument&                      doc )
{
   QDomElement element = doc.createElement( "association" );
   element.setAttribute( "keq"  , assoc.keq   );
   element.setAttribute( "k_off", assoc.k_off );
  
   if ( assoc.reaction_components.size() > 0 )
   {
      for ( uint i = 0; i < assoc.reaction_components.size(); i++ )
      {
         QDomElement relation = doc.createElement( "relation" );
         relation.setAttribute( "element"      , assoc.reaction_components[ i ] );
         relation.setAttribute( "stoichiometry", assoc.stoichiometry      [ i ] );
         element.appendChild( relation );
      }
   }

   return element;
}

