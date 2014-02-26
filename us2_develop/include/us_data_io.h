#ifndef US_DATA_IO_H
#define US_DATA_IO_H

#include <qstring.h>
#include <qdatastream.h>
#include <qobject.h>
#include "us_rundetails.h"
#include "us.h"
#include "us_util.h"
#include "us_buffer.h"
#include "us_vbar.h"
#include "us_extern.h"
#include "us_femglobal.h"

struct channelData
{
   vector <float> radius;
   vector <vector <float> > absorbance;
};

struct hydrodynamicData
{
   float Density[8][4];
   float Viscosity[8][4];
   float   Vbar[8][4][3];
   float Vbar20[8][4][3];
};

class US_EXTERN US_Data_IO : public QObject
{
   Q_OBJECT

   public:

      US_Data_IO(struct runinfo *, bool, QObject * parent=0, const char * name=0);
      virtual ~US_Data_IO();
      US_Config *USglobal;
      US_DB_T *terminalDB;
      float temperature;
      bool baseline_flag;
      struct runinfo *run_inf;
      struct peptideDetails Vbar_info;
      struct BufferData buff_info;
      US_Buffer *BUFFER;
      US_Vbar *VBAR;

      int load_run(QString,    //file name
      int,                     // run type
      bool *,                   // has_data[8]
      vector <struct centerpieceInfo> *);
      int write_run(QString,    //file name
      int,                     // run type
      bool *,                   // has_data[8]
      vector <struct centerpieceInfo> *);
      int load_veloc_scan(struct channelData *, unsigned int, unsigned int, unsigned int);
      int load_hydrodynamics(struct US_DatabaseLogin, struct hydrodynamicData *);
      int load_hydrodynamics(struct hydrodynamicData *);
      struct US_buffer export_buffer(int);
      struct peptideDetails export_vbar(int id);
      void calc_correction(struct correctionTerms * /*defined in us_util.h*/);
      void assign_simparams(struct SimulationParameters *, unsigned int, unsigned int, unsigned int);

   signals:

      void newMessage(QString, int);
};

bool check_dbname(QString /*dbname*/, QString * /*defaultdb*/);
#endif
