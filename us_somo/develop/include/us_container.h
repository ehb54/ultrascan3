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

   bool containers_supported();

 private:

   QString type; // typically "docker" or "podman"

};

#endif
