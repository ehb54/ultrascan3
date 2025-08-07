//! \file us_ramp.cpp

#include "us_ramp.h"
#include "stdio.h"
#include "us_gui_settings.h"
#include "us_math2.h"
#include "us_settings.h"
#include "us_util.h"

#include <QDomDocument>
#include "us_crc.h"
#include "us_matrix.h"

#ifdef WIN32
#define round(x) floor((x) + 0.5)
#endif

// Generic constructor
US_Ramp::US_Ramp() {
   //      dbg_level  = US_Settings::us_debug();
   // DbgLv(0) << "MwDa: dbg_level" << dbg_level;
}


int US_Ramp::saveToDisk(
   QVector<US_mwlRamp::RampRawData *> &rawConvertedData, QList<TripleInfo> &all_chaninfo, QString runID,
   QString dirname) {
   //qDebug() << "description"<<rawConvertedData[0]->description;
   if (rawConvertedData[ 0 ]->intarray.empty())
      return NODATA;

   // Make sure directory is empty
   QDir d(dirname);
   QStringList rmvfilt("*.ramp");
   QStringList rmvfiles = d.entryList(rmvfilt, QDir::Files, QDir::Name);
   for (int ii = 0; ii < rmvfiles.size(); ii++)
      if (!d.remove(rmvfiles[ ii ]))
         //qDebug() << "Unable to remove file" << rmvfiles[ ii ];


         QString wavelnp = "";

   for (int i = 0; i < all_chaninfo.size(); i++) {
      if (all_chaninfo[ i ].excluded)
         continue;

      QString triple = all_chaninfo[ i ].tripleDesc;
      QStringList parts = triple.split(" / ");

      QString cell = parts[ 0 ];
      QString channel = parts[ 1 ];
      QString filename;

      filename = runID + "." + cell + "." + channel + "." + "ramp";
      //qDebug()<<"tripledesc_filename_cellchan_cell_chan"<<all_chaninfo[i].tripleDesc<<i<<filename;

      // Same with solutionGUID
      QRegExp rx("^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$");
      qDebug() << "_______________________________________________________________________";
      qDebug() << "all_chaninfo[ i ].solution.saveStatus" << all_chaninfo[ i ].solution.saveStatus;
      qDebug() << "US_Solution::NOT_SAVED " << US_Solution::NOT_SAVED;
      qDebug() << "!rx.exactMatch( all_chaninfo[ i ].solution.solutionGUID "
               << !rx.exactMatch(all_chaninfo[ i ].solution.solutionGUID);

      if (
         (all_chaninfo[ i ].solution.saveStatus == US_Solution::NOT_SAVED)
         || (!rx.exactMatch(all_chaninfo[ i ].solution.solutionGUID))) {
         all_chaninfo[ i ].solution.solutionGUID = US_Util::new_guid();
         all_chaninfo[ i ].solution.solutionDesc = "New Solution";
      }
      // //////////////////////////////////////////////////////////////////////////////
      // Make sure solution is saved to disk
      all_chaninfo[ i ].solution.saveToDisk();

      // Save the filename of this triple
      all_chaninfo[ i ].tripleFilename = filename;
      //size allchaninfo != size_alldata!? //KL
      //       qDebug()<<"i size_allchaninfo size_rawconverteddata"<<i<<all_chaninfo.size()<<rawConvertedData.size();
      US_mwlRamp::RampRawData savecc = *rawConvertedData[ i ];
      // Now write altered dataset
      writeRawData(dirname + filename, savecc);
   }

   return OK;
}

int US_Ramp::writeRawData(QString file, US_mwlRamp::RampRawData &data) {
   // Open the file for writing
   QFile ff(file);
   if (!ff.open(QIODevice::WriteOnly))
      return CANTOPEN;
   QDataStream ds(&ff);

   // Using quint32 to ensure same data size on all platforms
   quint32 crc = 0xffffffffUL;

   // Write magic number
   char magic[ 5 ] = "UCDA";
   write(ds, magic, 4, crc);

   // Write format version
   char fmt[ 3 ];
   sprintf(fmt, "%2.2i", format_version);
   write(ds, fmt, 2, crc);
   //qDebug()<<"format"<<format_version;

   //    // Write data type
   //    write( ds, data.type, 2, crc );

   // Write cell
   char c = data.cell.toLatin1();
   int ic = c - '0';
   write(ds, ( const char * ) &ic, 1, crc);

   // Write channel
   c = data.chan.toLatin1();
   write(ds, ( const char * ) &c, 1, crc);

   //    // Create and write a guid
   //    write( ds, data.rawGUID, 16, crc );
   //
   // Write description
   char desc[ 240 ];
   memset(desc, '\0', sizeof desc); // bzero is not defined in WIN32

   QByteArray dd = data.description.toLatin1();
   strncpy(desc, dd.data(), sizeof desc);
   write(ds, desc, sizeof desc, crc);


   ///////////////////////////////////////////////////////////////////
   // The unions below are a little tricky in order to get the
   // c++ types and architecture endian-ness right.  Floats
   // are written to uf.f (union float - float ).  Then the
   // 4 bytes of the float are treated as a 32-bit unsigned int and
   // converted to ui.u (union integer - unsigned ).  Finally
   // the ui.c type is to satify the c++ type for the write call.

   union {
         quint32 u;
         float f;
   } uf;

   union {
         char c[ 4 ];
         uchar u[ 4 ];
   } ui;

   uf.f = ( float ) data.temperature; // temperature
   qToLittleEndian(uf.u, ui.u);
   write(ds, ui.c, 4, crc);

   uf.f = ( float ) data.radius; // radius
   qToLittleEndian(uf.u, ui.u);
   write(ds, ui.c, 4, crc);

   uf.f = ( float ) data.meniscus; // meniscus
   qToLittleEndian(uf.u, ui.u);
   write(ds, ui.c, 4, crc);

   uf.f = ( float ) data.bottom; // bottom
   qToLittleEndian(uf.u, ui.u);
   write(ds, ui.c, 4, crc);

   double n = data.rpm.size(); // n * rpm/time/w2t
   uf.f = ( float ) n;
   qToLittleEndian(uf.u, ui.u);
   write(ds, ui.c, 4, crc);


   for (int i = 0; i < ( int ) n; i++) // rpm array
   {
      uf.f = ( float ) data.rpm.at(i);
      qToLittleEndian(uf.u, ui.u);
      write(ds, ui.c, 4, crc);
   }

   for (int i = 0; i < ( int ) n; i++) // time array
   {
      uf.f = ( float ) data.time.at(i);
      qToLittleEndian(uf.u, ui.u);
      write(ds, ui.c, 4, crc);
   }

   for (int i = 0; i < ( int ) n; i++) // w2t array
   {
      uf.f = ( float ) data.w2t.at(i);
      qToLittleEndian(uf.u, ui.u);
      write(ds, ui.c, 4, crc);
   }

   double n_wl = data.wl_array.size(); // n Wavelengths
   uf.f = ( float ) n_wl;
   qToLittleEndian(uf.u, ui.u);
   write(ds, ui.c, 4, crc);
   //qDebug()<<"dn_wl ui.c"<<n_wl<<ui.c;

   double wl_double;
   for (int i = 0; i < ( int ) n_wl; i++) // Wavelength array
   {
      wl_double = ( double ) data.wl_array.at(i);
      uf.f = ( float ) wl_double;
      qToLittleEndian(uf.u, ui.u);
      write(ds, ui.c, 4, crc);
   }

   data.intarray.resize(n_wl); // write measurement array
   for (int lambda = 0; lambda < n_wl; lambda++) {
      data.intarray.resize(n);
      for (int i = 0; i < n; i++) {
         uf.f = ( float ) data.intarray[ lambda ].at(i);
         qToLittleEndian(uf.u, ui.u);
         write(ds, ui.c, 4, crc);
      }
   }


   //    // Write out scan count
   //    qToLittleEndian( (quint16)data.scanData.size(), ui.u );
   //    write( ds, ui.c, 2, crc );

   //    // Loop for each scan
   //    for ( int ii = 0; ii < data.scanData.size(); ii++ )
   //       writeScan( ds, data.scanData[ ii ], crc, pp );
   //
   //    qToLittleEndian( crc, ui.u ); // crc
   //    ds.writeRawData( ui.c, 4 );


   ff.close();
   readRawData(file, data);

   return OK;
}

/*format	parameter	char[]	
  ///////////////////////////////////////
  char		magic 		4
  "%2.2i"	format		2
  int 		cell		1
  char		chan		1
  char Latin1   description	240
  float		temperature	4
  float		radius		4
  float		meniscus	4
  float		bottom		4
  float		n-measurements	4
  float		rpm-array	n*4
  float		time-array	n*4
  float		w2t-array	n*4
  float		n-wavelengths	4
  float		wl-array	n_wl*4
  float		meas.-array	n_wl*n*4
*/

int US_Ramp::readUS3Disk(
   QString dir, QVector<US_mwlRamp::RampRawData> &rawConvertedData, QList<TripleInfo> &all_chaninfo) {
   rawConvertedData.clear();

   QStringList nameFilters = QStringList("*.ramp");

   QDir d(dir);

   QStringList files = d.entryList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);

   if (files.size() == 0)
      return NODATA;

   QStringList part = files[ 0 ].split(".");

   // Set up cell / channel combinations
   all_chaninfo.clear();
   for (int i = 0; i < files.size(); i++) {
      part.clear();
      part = files[ i ].split(".");

      TripleInfo t;

      t.tripleDesc = part[ 1 ] + " / " + part[ 2 ];
      t.excluded = false;
      //qDebug()<<"t.tripleDesc"<< t.tripleDesc;
      all_chaninfo << t;
   }

   // Read all data
   QString file;
   foreach (file, files) {
      QString filename = dir + file;
      US_mwlRamp::RampRawData data;

      int result = US_Ramp::readRawData(filename, data);
      if (result != US_DataIO::OK)
         return NOAUC;

      rawConvertedData << data;
      //       data.scanData.clear();
   }

   return OK;
}

int US_Ramp::readRawData(const QString file, US_mwlRamp::RampRawData &data) {
   QFile ff(file);
   //    qDebug()<<"_________________________________________________________file"<<file;
   if (!ff.open(QIODevice::ReadOnly))
      return CANTOPEN;
   QDataStream ds(&ff);

   int err = OK;
   quint32 crc = 0xffffffffUL;

   try {
      // Read magic number
      char magic[ 4 ];
      read(ds, magic, 4, crc);
      //       qDebug() <<"readmagic"<<magic;
      if (!strncmp(magic, "UCDA", 4) != 0)
         qDebug() << "readmagic" << magic;

      // Check the version number
      unsigned char ver[ 2 ];
      read(ds, ( char * ) ver, 2, crc);
      quint32 version = ((ver[ 0 ] & 0x0f) << 8) | (ver[ 1 ] & 0x0f);
      //qDebug() <<"readmagic"<<version;
      if (version != format_version)
         qDebug() << "this ramp data version is bad!" << version; // throw BAD_VERSION;

      // Get the cell
      union {
            char c[ 4 ];
            int i;
      } cell;

      cell.i = 0;

      read(ds, cell.c, 1, crc);
      data.cell = (qFromLittleEndian(cell.i)) + '0';
      //       QChar cell3= cell2 + '0';
      //qDebug()<<"readcell"<<data.cell;

      //reset orig chan first
      data.chan = 'C';
      // Get the channel
      union {
            char c[ 4 ];
            int i;
      } chan;

      chan.i = 0;
      //       char chan[1];
      read(ds, chan.c, 1, crc);
      data.chan = qFromLittleEndian(chan.i);
      //qDebug()<<"readchan"<<data.chan;
      //
      //       // Get the guid
      //       read( ds, data.rawGUID, 16, crc );
      //
      // Get the description
      char desc[ 240 ];
      read(ds, desc, 240, crc);
      data.description = QString(desc);
      qDebug() << "readchan" << data.chan << data.description;


      // Get the parameters to expand the values
      //      union
      //      {
      //         char    c[ 2 ];
      //         quint16 I;
      //      } si;

      union {
            char c[ 4 ];
            qint32 I;
            float f;
      } u1;

      union {
            char c[ 4 ];
            qint32 I;
            float f;
      } u2;

      read(ds, u1.c, 4, crc); // temperature
      u2.I = qFromLittleEndian(u1.I);
      data.temperature = u2.f;

      read(ds, u1.c, 4, crc); // radius
      u2.I = qFromLittleEndian(u1.I);
      data.radius = u2.f;

      read(ds, u1.c, 4, crc); // meniscus
      u2.I = qFromLittleEndian(u1.I);
      data.meniscus = u2.f;

      read(ds, u1.c, 4, crc); // bottom
      u2.I = qFromLittleEndian(u1.I);
      data.bottom = u2.f;

      double dn; // n points for rpm/time/w2t
      read(ds, u1.c, 4, crc);
      u2.I = qFromLittleEndian(u1.I);
      dn = u2.f;
      int n = ( int ) dn;
      //qDebug() << "testdata.radius" <<dn<<n;


      //       data.rpm.resize(n);		// rpm array
      for (int i = 0; i < n; i++) {
         read(ds, u1.c, 4, crc);
         u2.I = qFromLittleEndian(u1.I);
         data.rpm.append(u2.f);
         // 	//qDebug()<<"rpm"<<data.rpm.at(i);
      }

      //       data.time.resize(n);		// time array
      for (int i = 0; i < n; i++) {
         read(ds, u1.c, 4, crc);
         u2.I = qFromLittleEndian(u1.I);
         data.time.append(u2.f);
         // 	//qDebug()<<"time"<<data.rpm.at(i);
      }

      //       data.w2t.resize(n);		// w2t array
      for (int i = 0; i < n; i++) {
         read(ds, u1.c, 4, crc);
         u2.I = qFromLittleEndian(u1.I);
         data.w2t.append(u2.f);
         qDebug() << "w2t" << data.w2t.at(i);
      }

      double dn_wl; // n Wavelengths
      read(ds, u1.c, 4, crc);
      u2.I = qFromLittleEndian(u1.I);
      dn_wl = u2.f;

      int n_wl = ( int ) dn_wl;


      double wl_double;
      //       data.wl_array.resize(n_wl);		// Wavelength array
      for (int i = 0; i < n_wl; i++) {
         read(ds, u1.c, 4, crc);
         u2.I = qFromLittleEndian(u1.I);
         wl_double = u2.f;
         data.wl_array.append(( int ) wl_double);
         // 	//qDebug()<<"wavelength"<<data.wl_array.at(i);//<<wl_double<<data.wl_array.at(i);
      }
      qDebug() << "size_wl_arr" << data.wl_array.size();

      data.intarray.resize(n_wl);
      for (int lambda = 0; lambda < n_wl; lambda++) {
         for (int i = 0; i < n; i++) {
            read(ds, u1.c, 4, crc);
            u2.I = qFromLittleEndian(u1.I);
            data.intarray[ lambda ].append(u2.f);
            // 	  //qDebug()<<"w2t meas_value"<<data.w2t.at(i)<<data.intarray[lambda].at(i);
         }
      }

      //       // Read the crc
      //       quint32 read_crc;
      //       ds.readRawData( (char*) &read_crc , 4 );
      //       if ( crc != qFromLittleEndian( read_crc ) ) throw BADCRC;
      //
   } catch (ioError error) {
      err = error;
   }

   ff.close();
   return err;
}

// Initializations
US_Ramp::TripleInfo::TripleInfo() {
   clear();
}

void US_Ramp::TripleInfo::clear(void) {
   tripleID = 0;
   tripleDesc = QString("");
   description = QString("");
   excluded = false;
   centerpiece = 0;
   memset(tripleGUID, 0, 16);
   tripleFilename = QString("");
   solution.clear();
}

void US_Ramp::TripleInfo::show(void) {
   QString uuidc = US_Util::uuid_unparse(( unsigned char * ) tripleGUID);

   qDebug() << "tripleID     = " << tripleID << '\n'
            << "tripleDesc   = " << tripleDesc << '\n'
            << "description  = " << description << '\n'
            << "centerpiece  = " << centerpiece << '\n'
            << "tripleGUID   = " << QString(uuidc) << '\n'
            << "tripleFilename = " << tripleFilename << '\n'
            << "solutionID   = " << QString::number(solution.solutionID) << '\n'
            << "solutionGUID = " << solution.solutionGUID << '\n'
            << "solutionDesc = " << solution.solutionDesc;

   if (excluded)
      qDebug() << "excluded";
}

void US_Ramp::write(QDataStream &ds, const char *c, int len, quint32 &crc) {
   ds.writeRawData(c, len);
   crc = US_Crc::crc32(crc, ( unsigned char * ) c, len);
}

void US_Ramp::read(QDataStream &ds, char *cc, int len, quint32 &crc) {
   ds.readRawData(cc, len);
   crc = US_Crc::crc32(crc, ( uchar * ) cc, len);
}
