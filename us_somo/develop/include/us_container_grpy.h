#ifndef US_CONTAINER_GRPY_H
#define US_CONTAINER_GRPY_H

// convenience for managing GRPY container runs

#include "../include/us_util.h" // for US_Config
#include "../include/us_arch.h"
#include "../include/us_container.h"

using namespace std;

class US_Container_Grpy {

 public:

   US_Container_Grpy( bool pull = true, bool do_not_user_container = false );
   ~US_Container_Grpy();
   
   QString     executable();
   QStringList arguments( QString directory = "" );

 private:
   US_Container      usc;
   QString           image_name;
   vector < QString> container_names;
   US_Config       * USglobal;

};

#endif
