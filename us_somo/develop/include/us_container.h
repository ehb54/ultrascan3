#ifndef US_CONTAINER_H
#define US_CONTAINER_H

// a class for managing containers

#include <qobject.h>
#include <qstring.h>

using namespace std;

class US_Container : public QObject {
   Q_OBJECT

 public:

   US_Container();
   ~US_Container();

   // bool pull( const QString & name );

   bool installed();
   bool test();
   // QStringList prepend( const QStringList & args ); // takes an argument list and prepends proper type to run
   static QString unique_name( QString basename = "" ); // convenience to name containers
   QString type();

 private:

   QString container_type; // typically "docker" or "podman"

};

#endif
