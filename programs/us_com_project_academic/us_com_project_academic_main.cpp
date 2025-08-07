//! \file us_experiment_main.cpp

#include "../us_com_project/us_com_project_gui.h"


int main(int argc, char *argv[]) {
   QApplication application(argc, argv);

#include "main1.inc"

   // License is OK.  Start up.

   US_ComProjectMain *w = new US_ComProjectMain("US_MODE");
   w->show();

   // Create local "server" to register applicaiton
   QString instance_socket = US_Settings::etcDir() + "/usinstance_";
   QInstances instances(instance_socket);
   bool instance_created = instances.create();
   qDebug() << "instance create returned " << is_true(instance_created);
   if (!instance_created)
      exit(-1);

   //w->check_current_stage();
   w->call_AutoflowDialogue();

   if (w->window_closed)
      return 0;

   return application.exec(); //!< \memberof QApplication
}
