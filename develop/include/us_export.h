#ifndef US_EXPORT_H
#define US_EXPORT_H

#include "us.h"

#include "us_editor.h"
#include "us_util.h"
#include "us_pixmap.h"
#include "us_dtactrl.h"

#include <qprogbar.h>

class US_EXTERN US_Export_Veloc : public Data_Control_W
{
   Q_OBJECT
   
   public:
   
      US_Export_Veloc(QWidget *p = 0, const char *name = 0);
      ~US_Export_Veloc();

   private:
      US_Pixmap *pm;
      void help();
      void save();
      
   protected:
         
      void setup_GUI();
};

class US_EXTERN US_Export_Equil : public Data_Control_W
{
   Q_OBJECT
   
   public:
   
      US_Export_Equil(QWidget *p = 0, const char *name = 0);
      ~US_Export_Equil();

   private:
   
      US_Pixmap *pm;
      void help();
      void save();

   protected:         
      
      void setup_GUI();
};

class US_EXTERN US_Print_Veloc : public Data_Control_W
{
   Q_OBJECT
   
   public:
   
      US_Print_Veloc(QWidget *p = 0, const char *name = 0);
      ~US_Print_Veloc();

   private:
   
      void help();

   protected:
   
      void setup_GUI();
};

class US_EXTERN US_Print_Equil : public Data_Control_W
{
   Q_OBJECT
   
   public:
   
      US_Print_Equil(QWidget *p = 0, const char *name = 0);
      ~US_Print_Equil();

   private:
   
      void help();

   protected:
   
      void setup_GUI();
};
#endif

