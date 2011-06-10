//! \file us_data_model.cpp

#include "us_data_model.h"
#include "us_data_process.h"
#include "us_data_tree.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_editor.h"

// scan the database and local disk for R/E/M/N data sets
US_DataModel::US_DataModel( QWidget* parwidg /*=0*/ )
{
   parentw    = parwidg;   // parent (main manage_data) widget

   ddescs.clear();         // db descriptions
   ldescs.clear();         // local descriptions
   adescs.clear();         // all descriptions

   dbg_level  = US_Settings::us_debug();
}

// set database related pointers
void US_DataModel::setDatabase( US_DB2* a_db, QString a_invtxt )
{
   db         = a_db;      // pointer to opened db connection
   investig   = a_invtxt;  // investigator text
   invID      = investig.section( ":", 0, 0 ).simplified();
}

// set progress bar related pointers
void US_DataModel::setProgress( QProgressBar* a_progr, QLabel* a_lbstat )
{
   progress   = a_progr;   // pointer to progress bar
   lb_status  = a_lbstat;  // pointer to status label
}

// set sibling classes pointers
void US_DataModel::setSiblings( QObject* a_proc, QObject* a_tree )
{
   ob_process = a_proc;    // pointer to sister DataProcess object
   ob_tree    = a_tree;    // pointer to sister DataTree object
}

// get database pointer
US_DB2* US_DataModel::dbase()
{
   return db;
}

// get investigator text
QString US_DataModel::invtext()
{
   return investig;
}

// get progress bar pointer
QProgressBar* US_DataModel::progrBar()
{
   return progress;
}

// get status label pointer
QLabel* US_DataModel::statlab()
{
   return lb_status;
}

// get us_data_process object pointer
QObject* US_DataModel::procobj()
{
   return ob_process;
}

// get us_data_tree object pointer
QObject* US_DataModel::treeobj()
{
   return ob_tree;
}

// scan the database and local disk for R/E/M/N data sets
void US_DataModel::scan_data()
{
   ddescs.clear();         // db descriptions
   ldescs.clear();         // local descriptions
   adescs.clear();         // all descriptions

   scan_dbase( );          // read db to build db descriptions

   sort_descs( ddescs  );  // sort db descriptions

   scan_local( );          // read files to build local descriptions

   sort_descs( ldescs  );  // sort local descriptions

   merge_dblocal();        // merge database and local descriptions
}

// get pointer to data description object at specified row
US_DataModel::DataDesc US_DataModel::row_datadesc( int irow )
{
   return adescs.at( irow );
}

// get pointer to current data description object
US_DataModel::DataDesc US_DataModel::current_datadesc( )
{
   return cdesc;
}

// set pointer to current data description object
void US_DataModel::setCurrent( int irow )
{
   cdesc   = adescs.at( irow );
}

// get count of total data records
int US_DataModel::recCount()
{
   return adescs.size();
}

// get count of DB data records
int US_DataModel::recCountDB()
{
   return ddescs.size();
}

// get count of local data records
int US_DataModel::recCountLoc()
{
   return ldescs.size();
}

// scan the database for R/E/M/N data sets
void US_DataModel::scan_dbase( )
{
   QStringList rawIDs;
   QStringList rawGUIDs;
   QStringList edtIDs;
   QStringList modIDs;
   QStringList noiIDs;
   QStringList query;
   QMap< QString, int > edtMap;
   QString     dmyGUID  = "00000000-0000-0000-0000-000000000000";
   QString     recID;
   QString     rawGUID;
   QString     contents;
   int         irecID;
   int         nraws = 0;
   int         nedts = 0;
   int         nmods = 0;
   int         nnois = 0;
   int         nstep = 20;
   int         istep = 0;

   lb_status->setText( tr( "Reading DataBase Data..." ) );
   progress->setMaximum( nstep );
   ddescs.clear();

   query.clear();
   query << "get_experiment_desc" << invID;
   db->query( query );
   QStringList expIDs;

   while ( db->next() )
   {
      QString expID = db->value( 0 ).toString();
      QString runID = db->value( 1 ).toString();
      QString etype = db->value( 2 ).toString();
      DbgLv(1) << " expID runID type" << expID << runID << etype;
      expIDs << expID;
   }

   DbgLv(1) << "  expID expGUID runID label comment date";

   for ( int ii = 0; ii < expIDs.size(); ii++ )
   {
      QString expID = expIDs[ ii ];
      query.clear();
      query << "get_experiment_info" << expID;
      db->query( query );
      db->next();
      QString expGUID = db->value( 0 ).toString();
      QString runID   = db->value( 2 ).toString();
      QString label   = db->value( 9 ).toString();
      QString comment = db->value( 10 ).toString();
      QString date    = US_Util::toUTCDatetimeText( db->value( 12 )
                        .toDateTime().toString( Qt::ISODate ), true );
      DbgLv(1) << "  " << expID << expGUID << runID << label << comment << date;
   }
//*DEBUG*
QString labID = db->value( 3 ).toString();
QStringList rotorIDs;
QStringList locNames;
query.clear();
query << "get_rotor_names" << labID;
db->query( query );
while ( db->next() )
{
  rotorIDs << db->value(0).toString();
  locNames << db->value(1).toString();
}
DbgLv(2) << "ROTOR ID lname name serial absname stretch omeg2t GUID";
for ( int ii=0; ii<rotorIDs.size(); ii++ )
{
  QString rotorID = rotorIDs.at(ii);
  QString locName = locNames.at(ii);
  query.clear();
  query << "get_rotor_info" << rotorID;
  db->query( query );
  if ( db->next() )
  {
  QString rotorGUID = db->value(0).toString();
  QString rotorName = db->value(1).toString();
  QString serialNum = db->value(2).toString();
  QString stretchF  = db->value(3).toString();
  QString omega2t   = db->value(4).toString();
  QString abrotName = db->value(5).toString();
  DbgLv(2) << "   " << rotorID << locName << rotorName << serialNum
     << abrotName << stretchF << omega2t << rotorGUID;
  }
  else
    DbgLv(2) << "   *** get_rotor_info *ERROR* rotorID" << rotorID;
}
//*DEBUG*

   // get raw data IDs
   query.clear();
   query << "all_rawDataIDs" << invID;
   db->query( query );

   while ( db->next() )
   {
      rawIDs << db->value( 0 ).toString();
DbgLv(2) << "BrDb: RAW id" << db->value(0).toString()
 << " expID" << db->value(3).toString() << " solID" << db->value(4).toString();
   }
   progress->setValue( ++istep );

   // get edited data IDs
   query.clear();
   query << "all_editedDataIDs" << invID;
   db->query( query );

   while ( db->next() )
   {
      edtIDs << db->value( 0 ).toString();
DbgLv(2) << "BrDb: EDT id" << db->value(0).toString()
 << " raID" << db->value(3).toString() << " expID" << db->value(4).toString();
   }
   progress->setValue( ++istep );

   // get model IDs
   query.clear();
   query << "get_model_desc" << invID;
   db->query( query );

   while ( db->next() )
   {
      modIDs << db->value( 0 ).toString();
DbgLv(2) << "BrDb: MOD id" << db->value(0).toString()
 << " edID" << db->value(6).toString() << " edGID" << db->value(5).toString();
   }
   progress->setValue( ++istep );

   // get noise IDs
   query.clear();
   query << "get_noise_desc" << invID;
   db->query( query );

   while ( db->next() )
   {
      noiIDs << db->value( 0 ).toString();
DbgLv(2) << "BrDb: NOI id" << db->value(0).toString()
 << " edID" << db->value(2).toString() << " moID" << db->value(3).toString();
   }
   progress->setValue( ++istep );
   nraws = rawIDs.size();
   nedts = edtIDs.size();
   nmods = modIDs.size();
   nnois = noiIDs.size();
   nstep = istep + ( nraws * 5 ) + ( nedts * 5 ) + nmods + nnois;
   progress->setMaximum( nstep );
DbgLv(1) << "BrDb: kr ke km kn"
 << rawIDs.size() << edtIDs.size() << modIDs.size() << noiIDs.size();

   for ( int ii = 0; ii < nraws; ii++ )
   {  // get raw data information from DB
      recID             = rawIDs.at( ii );
      irecID            = recID.toInt();

      query.clear();
      query << "get_rawData" << recID;
      db->query( query );
      db->next();

              rawGUID   = db->value( 0 ).toString();
      QString label     = db->value( 1 ).toString();
      QString filename  = db->value( 2 ).toString().replace( "\\", "/" );
      QString filebase  = filename.section( "/", -1, -1 );
      QString comment   = db->value( 3 ).toString();
      QString experID   = db->value( 4 ).toString();
      QString date      = US_Util::toUTCDatetimeText( db->value( 7 )
                          .toDateTime().toString( Qt::ISODate ), true );
      QString cksum     = db->value( 8 ).toString();
      QString recsize   = db->value( 9 ).toString();
      QString runID     = filebase.section( ".", 0, 0 );
      QString subType   = "";
      contents          = cksum + " " + recsize;

      if ( comment.isEmpty() )
         comment        = filename.section( ".", 0, -2 );

      if ( ! label.contains( "." ) )
         label          = filename.section( ".", 0, -2 );

      rawGUIDs << rawGUID;

      query.clear();
      query << "get_experiment_info" << experID;
      db->query( query );
      db->next();

      QString expGUID   = db->value( 0 ).toString();
DbgLv(2) << "BrDb:     raw expGid" << expGUID;
DbgLv(2) << "BrDb:      label filename comment" << label << filename << comment;

//DbgLv(2) << "BrDb:       (R)contents" << contents;

      cdesc.recordID    = irecID;
      cdesc.recType     = 1;
      cdesc.subType     = subType;
      cdesc.recState    = REC_DB;
      cdesc.dataGUID    = rawGUID.simplified();
      cdesc.parentGUID  = expGUID.simplified();
      cdesc.parentID    = experID.toInt();
      cdesc.filename    = filename;
      cdesc.contents    = contents;
      cdesc.label       = label;
      cdesc.description = comment;
      cdesc.lastmodDate = date;

      if ( cdesc.dataGUID.length() != 36  ||  cdesc.dataGUID == dmyGUID )
         cdesc.dataGUID    = US_Util::new_guid();

      cdesc.parentGUID  = cdesc.parentGUID.length() == 36 ?
                          cdesc.parentGUID : dmyGUID;

      ddescs << cdesc;
      progress->setValue( ( istep += 5 ) );
   }

   for ( int ii = 0; ii < nedts; ii++ )
   {  // get edited data information from DB
      recID             = edtIDs.at( ii );
      irecID            = recID.toInt();

      query.clear();
      query << "get_editedData" << recID;
      db->query( query );
      db->next();

      QString rawID     = db->value( 0 ).toString();
      QString editGUID  = db->value( 1 ).toString();
      QString label     = db->value( 2 ).toString();
      QString filename  = db->value( 3 ).toString().replace( "\\", "/" );
      QString filebase  = filename.section( "/", -1, -1 );
      QString comment   = db->value( 4 ).toString();
      QString date      = US_Util::toUTCDatetimeText( db->value( 5 )
                          .toDateTime().toString( Qt::ISODate ), true );
      QString cksum     = db->value( 6 ).toString();
      QString recsize   = db->value( 7 ).toString();
      QString subType   = filebase.section( ".", 2, 2 );
      contents          = cksum + " " + recsize;

              rawGUID   = rawGUIDs.at( rawIDs.indexOf( rawID ) );
DbgLv(2) << "BrDb:     edt ii id eGID rGID label date"
 << ii << irecID << editGUID << rawGUID << label << date;
//DbgLv(2) << "BrDb:       (E)contents" << contents;

      if ( ! filename.contains( "/" ) )
         filename          = US_Settings::resultDir() + "/"
                             + filename.section( ".", 0, 0 ) + "/"
                             + filename;
//DbgLv(2) << "BrDb:       fname" << filename;

      cdesc.recordID    = irecID;
      cdesc.recType     = 2;
      cdesc.subType     = subType;
      cdesc.recState    = REC_DB;
      cdesc.dataGUID    = editGUID.simplified();
      cdesc.parentGUID  = rawGUID.simplified();
      cdesc.parentID    = rawID.toInt();
      cdesc.filename    = filename;
      cdesc.contents    = contents;
      cdesc.description = ( comment.isEmpty() ) ?
                          filebase.section( ".", 0, 2 ) :
                          comment;
      cdesc.label       = cdesc.description;
      cdesc.lastmodDate = date;

      if ( cdesc.dataGUID.length() != 36  ||  cdesc.dataGUID == dmyGUID )
         cdesc.dataGUID    = US_Util::new_guid();

      cdesc.parentGUID  = cdesc.parentGUID.simplified().length() == 36 ?
                          cdesc.parentGUID.simplified() : dmyGUID;

      ddescs << cdesc;
      progress->setValue( ( istep += 5 ) );

      edtMap[ cdesc.dataGUID ] = cdesc.recordID;    // save edit ID for GUID
   }

   for ( int ii = 0; ii < nmods; ii++ )
   {  // get model information from DB
      recID             = modIDs.at( ii );
      irecID            = recID.toInt();

      query.clear();
      query << "get_model_info" << recID;
      db->query( query );
      db->next();

      QString modelGUID = db->value( 0 ).toString();
      QString descript  = db->value( 1 ).toString();
              contents  = db->value( 2 ).toString();
      QString date      = US_Util::toUTCDatetimeText( db->value( 6 )
                          .toDateTime().toString( Qt::ISODate ), true );
      QString cksum     = db->value( 7 ).toString();
      QString recsize   = db->value( 8 ).toString();
      QString label     = descript.section( ".", 0, -2 );

      if ( label.length() > 40 )
         label = label.left( 13 ) + "..." + label.right( 24 );

      QString subType   = model_type( contents );
      int     jj        = contents.indexOf( "editGUID=" );
      QString editGUID  = ( jj < 1 ) ? "" :
                          contents.mid( jj ).section( QChar( '"' ), 1, 1 );
      contents          = cksum + " " + recsize;
              editGUID  = editGUID.simplified();

//DbgLv(2) << "BrDb:         det: cont" << contents;

      cdesc.recordID    = irecID;
      cdesc.recType     = 3;
      cdesc.subType     = subType;
      cdesc.recState    = REC_DB;
      cdesc.dataGUID    = modelGUID.simplified();
      cdesc.parentGUID  = editGUID;
      cdesc.parentID    = edtMap[ editGUID ];
      cdesc.filename    = "";
      cdesc.contents    = contents;
      cdesc.label       = label;
      cdesc.description = descript;
      cdesc.lastmodDate = date;

      if ( cdesc.dataGUID.length() != 36  ||  cdesc.dataGUID == dmyGUID )
         cdesc.dataGUID    = US_Util::new_guid();

      cdesc.parentGUID  = cdesc.parentGUID.simplified().length() == 36 ?
                          cdesc.parentGUID.simplified() : dmyGUID;

      ddescs << cdesc;
      progress->setValue( ++istep );
   }

   for ( int ii = 0; ii < nnois; ii++ )
   {  // get noise information from DB
      recID             = noiIDs.at( ii );
      irecID            = recID.toInt();

      query.clear();
      query << "get_noise_info" << recID;
      db->query( query );
      db->next();

      QString noiseGUID = db->value( 0 ).toString();
      QString editID    = db->value( 1 ).toString();
      QString modelID   = db->value( 2 ).toString();
      QString modelGUID = db->value( 3 ).toString();
      QString noiseType = db->value( 4 ).toString();
              contents  = db->value( 5 ).toString();
      int     idescr    = contents.indexOf( "description=" ) + 10;
      QString descript  = contents.mid( idescr, idescr+100 )
                          .section( "\"", 1, 1 );
      QString date      = US_Util::toUTCDatetimeText( db->value( 6 )
                          .toDateTime().toString( Qt::ISODate ), true );
      QString cksum     = db->value( 7 ).toString();
      QString recsize   = db->value( 8 ).toString();
//DbgLv(3) << "BrDb: contents================================================";
//DbgLv(3) << contents.left( 200 );
//DbgLv(3) << "BrDb: contents================================================";

      contents          = cksum + " " + recsize;
      QString label     = descript.section( ".", 0, -2 );

      if ( label.length() > 40 )
         label = label.left( 13 ) + "..." + label.right( 24 );

      cdesc.recordID    = irecID;
      cdesc.recType     = 4;
      cdesc.subType     = ( noiseType == "ti_noise" ) ? "TI" : "RI";
      cdesc.recState    = REC_DB;
      cdesc.dataGUID    = noiseGUID.simplified();
      cdesc.parentGUID  = modelGUID.simplified();
      cdesc.parentID    = modelID.toInt();
      cdesc.filename    = "";
      cdesc.contents    = contents;
      cdesc.label       = label;
      cdesc.description = descript;
      cdesc.lastmodDate = date;
DbgLv(2) << "BrDb:       noi ii id nGID dsc typ noityp"
   << ii << irecID << noiseGUID << descript << cdesc.subType << noiseType;

      if ( cdesc.dataGUID.length() != 36  ||  cdesc.dataGUID == dmyGUID )
         cdesc.dataGUID    = US_Util::new_guid();

      cdesc.parentGUID  = cdesc.parentGUID.simplified().length() == 36 ?
                          cdesc.parentGUID.simplified() : dmyGUID;

      ddescs << cdesc;
      progress->setValue( ++istep );
   }

   progress->setValue( nstep );
   lb_status->setText( tr( "Database Review Complete" ) );
}

// scan the local disk for R/E/M/N data sets
void US_DataModel::scan_local( )
{
   // start with AUC (raw) and edit files in directories of resultDir
   QString     rdir     = US_Settings::resultDir();
   QString     ddir     = US_Settings::dataDir();
   QString     dirm     = ddir + "/models";
   QString     dirn     = ddir + "/noises";
   QString     contents = "";
   QString     dmyGUID  = "00000000-0000-0000-0000-000000000000";
   QStringList aucdirs  = QDir( rdir )
      .entryList( QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name );
   QStringList aucfilt;
   QStringList edtfilt;
   QStringList modfilt( "M*xml" );
   QStringList noifilt( "N*xml" );
   QStringList modfils = QDir( dirm )
      .entryList( modfilt, QDir::Files, QDir::Name );
   QStringList noifils = QDir( dirn )
      .entryList( noifilt, QDir::Files, QDir::Name );
   int         ktask   = 0;
   int         naucd   = aucdirs.size();
   int         nmodf   = modfils.size();
   int         nnoif   = noifils.size();
   int         nstep   = naucd * 4 + nmodf + nnoif;
DbgLv(1) << "BrLoc:  nau nmo nno nst" << naucd << nmodf << nnoif << nstep;
   aucfilt << "*.auc";
   edtfilt << "*.xml";
   rdir    = rdir + "/";
   lb_status->setText( tr( "Reading Local-Disk Data..." ) );
   progress->setMaximum( nstep );

   for ( int ii = 0; ii < naucd; ii++ )
   {  // loop thru potential data directories
      QString     subdir   = rdir + aucdirs.at( ii );
      QStringList aucfiles = QDir( subdir )
         .entryList( aucfilt, QDir::Files, QDir::Name );
      int         naucf    = aucfiles.size();
      US_DataIO2::RawData    rdata;
      US_DataIO2::EditValues edval;

      for ( int jj = 0; jj < naucf; jj++ )
      {  // loop thru .auc files found in a directory
         QString fname    = aucfiles.at( jj );
         QString runid    = fname.section( ".", 0, 0 );
         QString tripl    = fname.section( ".", -5, -2 );
         QString aucfile  = subdir + "/" + fname;
         QString descr    = "";
         QString expGUID  = expGUIDauc( aucfile );
DbgLv(2) << "BrLoc: ii jj file" << ii << jj << aucfile;

         // read in the raw data and build description record
         US_DataIO2::readRawData( aucfile, rdata );

         contents         = US_Util::md5sum_file( aucfile );
DbgLv(2) << "BrLoc:      contents" << contents;

         QString uuid      = US_Util::uuid_unparse( (uchar*)rdata.rawGUID );
         QString rawGUID   = uuid;

         cdesc.recordID    = -1;
         cdesc.recType     = 1;
         cdesc.subType     = "";
         cdesc.recState    = REC_LO;
         cdesc.dataGUID    = rawGUID.simplified();
         cdesc.parentGUID  = expGUID.simplified();
         cdesc.parentID    = -1;
         cdesc.filename    = aucfile;
         cdesc.contents    = contents;
         cdesc.label       = runid + "." + tripl;
         cdesc.description = rdata.description;
         cdesc.lastmodDate = US_Util::toUTCDatetimeText( QFileInfo( aucfile )
                             .lastModified().toUTC().toString( Qt::ISODate )
                             , true );

         if ( cdesc.dataGUID.length() != 36  ||  cdesc.dataGUID == dmyGUID )
            cdesc.dataGUID    = US_Util::new_guid();

         cdesc.parentGUID  = cdesc.parentGUID.simplified().length() == 36 ?
                             cdesc.parentGUID.simplified() : dmyGUID;

         ldescs << cdesc;

         // now load edit files associated with this auc file
         edtfilt.clear();
         edtfilt << runid + ".*." + tripl + ".xml";
DbgLv(2) << "BrLoc:  edtfilt" << edtfilt;

         QStringList edtfiles = QDir( subdir )
            .entryList( edtfilt, QDir::Files, QDir::Name );

         for ( int kk = 0; kk < edtfiles.size(); kk++ )
         {
            QString efname   = edtfiles.at( kk );
            QString editid   = efname.section( ".", 1, 3 );
            QString edtfile  = subdir + "/" + efname;
                    contents = "";
//DbgLv(2) << "BrLoc:    kk file" << kk << edtfile;

            // read EditValues for the edit data and build description record
            US_DataIO2::readEdits( edtfile, edval );

            contents          = US_Util::md5sum_file( edtfile );
//DbgLv(2) << "BrLoc:      (E)contents edtfile" << contents << edtfile;

            cdesc.recordID    = -1;
            cdesc.recType     = 2;
            cdesc.subType     = efname.section( ".", 2, 2 );
            cdesc.recState    = REC_LO;
            cdesc.dataGUID    = edval.editGUID.simplified();
            cdesc.parentGUID  = edval.dataGUID.simplified();
            cdesc.parentID    = -1;
            cdesc.filename    = edtfile;
            cdesc.contents    = contents;
            cdesc.label       = runid + "." + editid;
            cdesc.description = efname.section( ".", 0, -2 );
            cdesc.lastmodDate = US_Util::toUTCDatetimeText( QFileInfo( edtfile )
                                .lastModified().toUTC().toString( Qt::ISODate )
                                , true );

            if ( cdesc.dataGUID.length() != 36  ||  cdesc.dataGUID == dmyGUID )
               cdesc.dataGUID    = US_Util::new_guid();

            cdesc.parentGUID  = cdesc.parentGUID.simplified().length() == 36 ?
                                cdesc.parentGUID.simplified() : dmyGUID;

            ldescs << cdesc;
         }
         if ( ii == ( naucd / 2 )  &&  jj == ( naucf / 2 ) )
            progress->setValue( ++ktask );
      }
      progress->setValue( ++ktask );
   }
   progress->setValue( ++ktask );

   for ( int ii = 0; ii < nmodf; ii++ )
   {  // loop thru potential model files
      US_Model    model;
      QString     modfil   = dirm + "/" + modfils.at( ii );
                  contents = "";

      model.load( modfil );

      contents          = US_Util::md5sum_file( modfil );

      cdesc.recordID    = -1;
      cdesc.recType     = 3;
      cdesc.subType     = model_type( model );
      cdesc.recState    = REC_LO;
      cdesc.dataGUID    = model.modelGUID.simplified();
      cdesc.parentGUID  = model.editGUID.simplified();
      cdesc.parentID    = -1;
      cdesc.filename    = modfil;
      cdesc.contents    = contents;
      cdesc.description = model.description;
      cdesc.lastmodDate = US_Util::toUTCDatetimeText( QFileInfo( modfil )
                          .lastModified().toUTC().toString( Qt::ISODate )
                          , true );

      if ( cdesc.dataGUID.length() != 36  ||  cdesc.dataGUID == dmyGUID )
         cdesc.dataGUID    = US_Util::new_guid();

      cdesc.parentGUID  = cdesc.parentGUID.simplified().length() == 36 ?
                          cdesc.parentGUID.simplified() : dmyGUID;
      QString label     = model.description.section( ".", 0, -2 );
      cdesc.label       = ( label.length() < 41 ) ? label :
                          ( label.left( 13 ) + "..." + label.right( 24 ) );

      ldescs << cdesc;

      progress->setValue( ++ktask );
   }

   for ( int ii = 0; ii < nnoif; ii++ )
   {  // loop thru potential noise files
      US_Noise    noise;
      QString     noifil   = dirn + "/" + noifils.at( ii );

      noise.load( noifil );

      contents          = US_Util::md5sum_file( noifil );

      cdesc.recordID    = -1;
      cdesc.recType     = 4;
      cdesc.subType     = ( noise.type == US_Noise::RI ) ? "RI" : "TI";
      cdesc.recState    = REC_LO;
      cdesc.dataGUID    = noise.noiseGUID.simplified();
      cdesc.parentGUID  = noise.modelGUID.simplified();
      cdesc.parentID    = -1;
      cdesc.filename    = noifil;
      cdesc.contents    = contents;
      cdesc.description = noise.description;
      cdesc.lastmodDate = US_Util::toUTCDatetimeText( QFileInfo( noifil )
                          .lastModified().toUTC().toString( Qt::ISODate )
                          , true );

      if ( cdesc.dataGUID.length() != 36  ||  cdesc.dataGUID == dmyGUID )
         cdesc.dataGUID    = US_Util::new_guid();

      cdesc.parentGUID  = cdesc.parentGUID.simplified().length() == 36 ?
                          cdesc.parentGUID.simplified() : dmyGUID;
      QString label     = noise.description;
      cdesc.label       = ( label.length() < 41 ) ? label :
                          ( label.left( 9 ) + "..." + label.right( 28 ) );

      ldescs << cdesc;

      progress->setValue( ++ktask );
   }

   progress->setValue( nstep );
   lb_status->setText( tr( "Local Data Review Complete" ) );
}

// merge the database and local description vectors into a single combined
void US_DataModel::merge_dblocal( )
{
   int nddes = ddescs.size();
   int nldes = ldescs.size();
   int nstep = ( ( nddes + nldes ) * 5 ) / 8;

   int jdr   = 0;
   int jlr   = 0;
   int kar   = 1;

   DataDesc  descd = ( nddes > 0 ) ? ddescs.at( 0 ) : DataDesc();
   DataDesc  descl = ( nldes > 0 ) ? ldescs.at( 0 ) : DataDesc();
DbgLv(2) << "MERGE: nd nl dlab llab"
 << nddes << nldes << descd.label << descl.label;

   lb_status->setText( tr( "Merging Data ..." ) );
   progress->setMaximum( nstep );

   while ( jdr < nddes  &&  jlr < nldes )
   {  // main loop to merge records until one is exhausted

      progress->setValue( kar );           // report progress

      if ( kar > nstep )
      {  // if count beyond max, bump max by one eighth
         nstep = ( kar * 9 ) / 8;
         progress->setMaximum( nstep );
      }

      while ( descd.dataGUID == descl.dataGUID )
      {  // records match in GUID:  merge them into one
         descd.recState    |= descl.recState;     // OR states
         descd.filename     = descl.filename;     // filename from local
         descd.lastmodDate  = descl.lastmodDate;  // last mod date from local
         descd.description  = descl.description;  // description from local
//if ( descl.recType == 3 && descd.contents != descl.contents ) {
// US_Model modell;
// US_Model modeld;
// modell.load( descl.filename );
// modeld.load( QString::number(descd.recordID), db );
// DbgLv(1) << " ++LOCAL Model:";
// modell.debug();
// DbgLv(1) << " ++DB Model:";
// modeld.debug(); }
         descd.contents     = descd.contents + " " + descl.contents;

         adescs << descd;                  // output combo record
//DbgLv(2) << "MERGE:  kar jdr jlr (1)GID" << kar << jdr << jlr << descd.dataGUID;
         kar++;

         if ( ++jdr < nddes )              // bump db count and test if done
            descd = ddescs.at( jdr );      // get next db record

         else
         {
            if ( ++jlr < nldes )
               descl = ldescs.at( jlr );   // get next local record
            break;
         }


         if ( ++jlr < nldes )              // bump local count and test if done
            descl = ldescs.at( jlr );      // get next local record
         else
            break;
      }

      if ( jdr >= nddes  ||  jlr >= nldes )
         break;

      while ( descd.recType > descl.recType )
      {  // output db records that are left-over children
         adescs << descd;
//DbgLv(2) << "MERGE:  kar jdr jlr (2)GID" << kar << jdr << jlr << descd.dataGUID;
         kar++;

         if ( ++jdr < nddes )
            descd = ddescs.at( jdr );
         else
            break;
      }

      if ( jdr >= nddes  ||  jlr >= nldes )
         break;

      while ( descl.recType > descd.recType )
      {  // output local records that are left-over children
         adescs << descl;
//DbgLv(2) << "MERGE:  kar jdr jlr (3)GID" << kar << jdr << jlr << descl.dataGUID;
         kar++;

         if ( ++jlr < nldes )
            descl = ldescs.at( jlr );
         else
            break;
      }

      if ( jdr >= nddes  ||  jlr >= nldes )
         break;

      // If we've reached another matching pair or if we are not at
      // the same level, go back up to the start of the main loop.
      if ( descd.dataGUID == descl.dataGUID  ||
           descd.recType  != descl.recType  )
         continue;

      // If we are here, we have records at the same level,
      // but with different GUIDs. Output one of them, based on
      // an alphanumeric comparison of label values.

      QString dlabel = descd.label;
      QString llabel = descl.label;
      if ( descd.recType > 2 )
      {
         dlabel = descd.description;
         llabel = descl.description;
      }

      if ( dlabel < llabel )
      {  // output db record first based on alphabetic label sort
         adescs << descd;
//DbgLv(2) << "MERGE:  kar jdr jlr (4)GID" << kar << jdr << jlr << descd.dataGUID;
         kar++;

         if ( ++jdr < nddes )
            descd = ddescs.at( jdr );
         else
            break;
      }

      else
      {  // output local record first based on alphabetic label sort
         adescs << descl;
//DbgLv(2) << "MERGE:  kar jdr jlr (5)GID" << kar << jdr << jlr << descl.dataGUID;
         kar++;

         if ( ++jlr < nldes )
            descl = ldescs.at( jlr );
         else
            break;
      }

   }  // end of main merge loop;

   // after breaking from main loop, output any records left from one
   // source (db/local) or the other.
   nstep += ( nddes - jdr + nldes - jlr );
   progress->setMaximum( nstep );

   while ( jdr < nddes )
   {
      adescs << ddescs.at( jdr++ );
//descd=ddescs.at(jlr-1);
//DbgLv(2) << "MERGE:  kar jdr jlr (8)GID" << kar << jdr << jlr << descd.dataGUID;
      kar++;
      progress->setValue( kar );
   }

   while ( jlr < nldes )
   {
      adescs << ldescs.at( jlr++ );
//descl=ldescs.at(jlr-1);
//DbgLv(2) << "MERGE:  kar jdr jlr (9)GID" << kar << jdr << jlr << descl.dataGUID;
      kar++;
      progress->setValue( kar );
   }

//DbgLv(2) << "MERGE: nddes nldes kar" << nddes << nldes << --kar;
//DbgLv(2) << " a/d/l sizes" << adescs.size() << ddescs.size() << ldescs.size();

   progress->setValue( nstep );
   lb_status->setText( tr( "Data Merge Complete" ) );
}

// sort a data-set description vector
void US_DataModel::sort_descs( QVector< DataDesc >& descs )
{
   QVector< DataDesc > tdess;                 // temporary descr. vector
   DataDesc            desct;                 // temporary descr. entry
   QStringList         sortr;                 // sort string lists
   QStringList         sorte;
   QStringList         sortm;
   QStringList         sortn;
   int                 nrecs = descs.size();  // number of descr. records

DbgLv(1) << "sort_desc: nrecs" << nrecs;
   if ( nrecs == 0 )
      return;

   tdess.resize( nrecs );
   // Determine maximum description string length
   maxdlen = 0;
   for ( int ii = 0; ii < nrecs; ii++ )
      maxdlen = qMax( maxdlen, descs[ ii ].description.length() );

   for ( int ii = 0; ii < nrecs; ii++ )
   {  // build sort strings for Raw,Edit,Model,Noise; copy unsorted vector
      desct        = descs[ ii ];

      if (      desct.recType == 1 )
         sortr << sort_string( desct, ii );

      else if ( desct.recType == 2 )
         sorte << sort_string( desct, ii );

      else if ( desct.recType == 3 )
         sortm << sort_string( desct, ii );

      else if ( desct.recType == 4 )
         sortn << sort_string( desct, ii );

      tdess[ ii ]  = desct;
   }

   // sort the string lists for each type
   sortr.sort();
   sorte.sort();
   sortm.sort();
   sortn.sort();

   // review each type for duplicate GUIDs
   if ( review_descs( sortr, tdess ) )
      return;
   if ( review_descs( sorte, tdess ) )
      return;
   if ( review_descs( sortm, tdess ) )
      return;
   if ( review_descs( sortn, tdess ) )
      return;

   // create list of noise,model,edit orphans
   QStringList orphn = list_orphans( sortn, sortm );
   QStringList orphm = list_orphans( sortm, sorte );
   QStringList orphe = list_orphans( sorte, sortr );

   QString dmyGUID = "00000000-0000-0000-0000-000000000000";
   QString dsorts;
   QString dlabel;
   QString dindex;
   QString ddGUID;
   QString dpGUID;
   QString ppGUID;
   int kndx = tdess.size();
   int jndx;
   int kk;
   int ndmy = 0;     // flag of duplicate dummies

   // create dummy records to parent each orphan

   for ( int ii = 0; ii < orphn.size(); ii++ )
   {  // for each orphan noise, create a dummy model
      dsorts = orphn.at( ii );
      dlabel = dsorts.section( ":", 0, 0 );
      dindex = dsorts.section( ":", 1, 1 );
      ddGUID = dsorts.section( ":", 2, 2 ).simplified();
      dpGUID = dsorts.section( ":", 3, 3 ).simplified();
      jndx   = dindex.toInt();
      cdesc  = tdess[ jndx ];

      if ( dpGUID.length() < 2 )
      { // handle case where there is no valid parentGUID
         if ( ndmy == 0 )      // first time:  create one
            dpGUID = dmyGUID;
         else
            dpGUID = ppGUID;   // afterwards:  re-use same parent

         kk     = sortn.indexOf( dsorts );  // find index in full list
         dsorts = dlabel + ":" + dindex + ":" + ddGUID + ":" + dpGUID;

         if ( kk >= 0 )
         {  // replace present record for new parentGUID
            sortn.replace( kk, dsorts );
            cdesc.parentGUID  = dpGUID;
            tdess[ jndx ]     = cdesc;
         }

         if ( ndmy > 0 )       // after 1st time, skip creating new parent
            continue;

         ndmy++;               // flag that we have a parent for invalid ones
         ppGUID = dpGUID;      // save the GUID for new dummy parent
      }

      // if this record is no longer an orphan, skip creating new parent
      if ( index_substring( dpGUID, 2, sortm ) >= 0 )
         continue;

      if ( dpGUID == dmyGUID )
         cdesc.label       = "Dummy-Model-for-Orphans";

      cdesc.parentID    = cdesc.recordID;
      cdesc.recordID    = -1;
      cdesc.recType     = 3;
      cdesc.subType     = "";
      cdesc.recState    = NOSTAT;
      cdesc.dataGUID    = dpGUID;
      cdesc.parentGUID  = dmyGUID;
      cdesc.parentID    = -1;
      cdesc.filename    = "";
      cdesc.contents    = "";
      cdesc.label       = cdesc.label.section( ".", 0, 0 );
      cdesc.description = cdesc.label + "--ARTIFICIAL-RECORD";
      cdesc.lastmodDate = US_Util::toUTCDatetimeText(
                          QDateTime::currentDateTime().toUTC()
                          .toString( Qt::ISODate ), true );

      dlabel = dlabel.section( ".", 0, 0 );
      dindex = QString().sprintf( "%4.4d", kndx++ );
      ddGUID = cdesc.dataGUID;
      dpGUID = cdesc.parentGUID;
      dsorts = dlabel + ":" + dindex + ":" + ddGUID + ":" + dpGUID;

      sortm << dsorts;
      orphm << dsorts;
      tdess.append( cdesc );
//DbgLv(2) << "N orphan:" << orphn.at( ii );
//DbgLv(2) << "  M dummy:" << dsorts;
   }

   ndmy   = 0;

   for ( int ii = 0; ii < orphm.size(); ii++ )
   {  // for each orphan model, create a dummy edit
      dsorts = orphm.at( ii );
      dlabel = dsorts.section( ":", 0, 0 );
      dindex = dsorts.section( ":", 1, 1 );
      ddGUID = dsorts.section( ":", 2, 2 ).simplified();
      dpGUID = dsorts.section( ":", 3, 3 ).simplified();
      jndx   = dindex.toInt();
      cdesc  = tdess[ jndx ];

      if ( dpGUID.length() < 16 )
      { // handle case where there is no valid parentGUID
         if ( ndmy == 0 )      // first time:  create one
            dpGUID = dmyGUID;
         else
            dpGUID = ppGUID;   // afterwards:  re-use same parent

         kk     = sortm.indexOf( dsorts );  // find index in full list
         dsorts = dlabel + ":" + dindex + ":" + ddGUID + ":" + dpGUID;

         if ( kk >= 0 )
         {  // replace present record for new parentGUID
            sortm.replace( kk, dsorts );
            cdesc.parentGUID  = dpGUID;
            tdess[ jndx ]     = cdesc;
         }

         if ( ndmy > 0 )       // after 1st time, skip creating new parent
            continue;

         ndmy++;               // flag that we have a parent for invalid ones
         ppGUID = dpGUID;      // save the GUID for new dummy parent
      }

      // if this record is no longer an orphan, skip creating new parent
      if ( index_substring( dpGUID, 2, sorte ) >= 0 )
         continue;

      if ( dpGUID == dmyGUID )
         cdesc.label       = "Dummy-Edit-for-Orphans";

      cdesc.parentID    = cdesc.recordID;
      cdesc.recordID    = -1;
      cdesc.recType     = 2;
      cdesc.subType     = "";
      cdesc.recState    = NOSTAT;
      cdesc.dataGUID    = dpGUID;
      cdesc.parentGUID  = dmyGUID;
      cdesc.parentID    = -1;
      cdesc.filename    = "";
      cdesc.contents    = "";
      cdesc.label       = cdesc.label.section( ".", 0, 0 );
      cdesc.description = cdesc.label + "--ARTIFICIAL-RECORD";
      cdesc.lastmodDate = US_Util::toUTCDatetimeText(
                          QDateTime::currentDateTime().toUTC()
                          .toString( Qt::ISODate ), true );

      dlabel = dlabel.section( ".", 0, 0 );
      dindex = QString().sprintf( "%4.4d", kndx++ );
      ddGUID = cdesc.dataGUID;
      dpGUID = cdesc.parentGUID;
      dsorts = dlabel + ":" + dindex + ":" + ddGUID + ":" + dpGUID;

      sorte << dsorts;
      orphe << dsorts;
      tdess.append( cdesc );
//DbgLv(2) << "M orphan:" << orphm.at( ii );
//DbgLv(2) << "  E dummy:" << dsorts;
   }

   ndmy   = 0;

   for ( int ii = 0; ii < orphe.size(); ii++ )
   {  // for each orphan edit, create a dummy raw
      dsorts = orphe.at( ii );
      dlabel = dsorts.section( ":", 0, 0 );
      dindex = dsorts.section( ":", 1, 1 );
      ddGUID = dsorts.section( ":", 2, 2 ).simplified();
      dpGUID = dsorts.section( ":", 3, 3 ).simplified();
      jndx   = dindex.toInt();
      cdesc  = tdess[ jndx ];

      if ( dpGUID.length() < 2 )
      { // handle case where there is no valid parentGUID
         if ( ndmy == 0 )      // first time:  create one
            dpGUID = dmyGUID;
         else
            dpGUID = ppGUID;   // afterwards:  re-use same parent

         kk     = sorte.indexOf( dsorts );  // find index in full list
         dsorts = dlabel + ":" + dindex + ":" + ddGUID + ":" + dpGUID;

         if ( kk >= 0 )
         {  // replace present record for new parentGUID
            sorte.replace( kk, dsorts );
            cdesc.parentGUID  = dpGUID;
            tdess[ jndx ]     = cdesc;
         }

         if ( ndmy > 0 )       // after 1st time, skip creating new parent
            continue;

         ndmy++;               // flag that we have a parent for invalid ones
         ppGUID = dpGUID;      // save the GUID for new dummy parent
      }

      // if this record is no longer an orphan, skip creating new parent
      if ( index_substring( dpGUID, 2, sortr ) >= 0 )
         continue;

      if ( dpGUID == dmyGUID )
         cdesc.label       = "Dummy-Raw-for-Orphans";

      cdesc.parentID    = cdesc.recordID;
      cdesc.recordID    = -1;
      cdesc.recType     = 1;
      cdesc.subType     = "";
      cdesc.recState    = NOSTAT;
      cdesc.dataGUID    = dpGUID;
      cdesc.parentGUID  = dmyGUID;
      cdesc.parentID    = -1;
      cdesc.filename    = "";
      cdesc.contents    = "";
      cdesc.label       = cdesc.label.section( ".", 0, 0 );
      cdesc.description = cdesc.label + "--ARTIFICIAL-RECORD";
      cdesc.lastmodDate = US_Util::toUTCDatetimeText(
                          QDateTime::currentDateTime().toUTC()
                          .toString( Qt::ISODate ), true );

      dlabel = dlabel.section( ".", 0, 0 );
      dindex = QString().sprintf( "%4.4d", kndx++ );
      ddGUID = cdesc.dataGUID;
      dpGUID = cdesc.parentGUID;
      dsorts = dlabel + ":" + dindex + ":" + ddGUID + ":" + dpGUID;

      sortr << dsorts;
      tdess.append( cdesc );
DbgLv(2) << "E orphan:" << orphe.at( ii );
DbgLv(2) << "  R dummy:" << dsorts;
   }

//for ( int ii = 0; ii < sortr.size(); ii++ )
// DbgLv(2) << "R entry:" << sortr.at( ii );
   int countR = sortr.size();    // count of each kind in sorted lists
   int countE = sorte.size();
   int countM = sortm.size();
   int countN = sortn.size();

   sortr.sort();                 // re-sort for dummy additions
   sorte.sort();
   sortm.sort();
   sortn.sort();
DbgLv(1) << "sort/dumy: count REMN" << countR << countE << countM << countN;
//for(int ii=0;ii<countM;ii++) DbgLv(2) << "sm" << ii << "++ " << sortm[ii];

   int noutR  = 0;               // count of each kind in hierarchical output
   int noutE  = 0;
   int noutM  = 0;
   int noutN  = 0;
   int indx;
   int pstate = REC_LO | PAR_LO;

   descs.clear();                // reset input vector to become sorted output

   // rebuild the description vector with sorted trees

   for ( int ii = 0; ii < countR; ii++ )
   {  // loop to output sorted Raw records
      QString recr = sortr[ ii ];
      QString didr = recr.section( ":", 2, 2 );
      QString pidr = recr.section( ":", 3, 3 );
      indx         = recr.section( ":", 1, 1 ).toInt();
      cdesc        = tdess.at( indx );

      // set up a default parent state flag
      pstate = cdesc.recState;
      pstate = ( pstate & REC_DB ) != 0 ? ( pstate | PAR_DB ) : pstate;
      pstate = ( pstate & REC_LO ) != 0 ? ( pstate | PAR_LO ) : pstate;

      // new state is the default,  or NOSTAT if this is a dummy record
      cdesc.recState = record_state_flag( cdesc, pstate );

      descs << cdesc;                   // output Raw rec
      noutR++;

      // set up parent state for children to follow
      int rpstate    = cdesc.recState;

      for ( int jj = 0; jj < countE; jj++ )
      {  // loop to output sorted Edit records for the above Raw
         QString rece   = sorte[ jj ];
         QString pide   = rece.section( ":", 3, 3 );

         if ( pide != didr )            // skip if current Raw not parent
            continue;

         QString dide   = rece.section( ":", 2, 2 );
         indx           = rece.section( ":", 1, 1 ).toInt();
         cdesc          = tdess.at( indx );
         cdesc.recState = record_state_flag( cdesc, rpstate );

         descs << cdesc;                // output Edit rec
         noutE++;

         // set up parent state for children to follow
         int epstate    = cdesc.recState;

         for ( int mm = 0; mm < countM; mm++ )
         {  // loop to output sorted Model records for above Edit
            QString recm   = sortm[ mm ];
            QString pidm   = recm.section( ":", 3, 3 );

            if ( pidm != dide )         // skip if current Edit not parent
               continue;

            QString didm   = recm.section( ":", 2, 2 );
            indx           = recm.section( ":", 1, 1 ).toInt();
            cdesc          = tdess.at( indx );
            cdesc.recState = record_state_flag( cdesc, epstate );

            descs << cdesc;             // output Model rec

            noutM++;

            // set up parent state for children to follow
            int mpstate    = cdesc.recState;

            for ( int nn = 0; nn < countN; nn++ )
            {  // loop to output sorted Noise records for above Model
               QString recn   = sortn[ nn ];
               QString pidn   = recn.section( ":", 3, 3 );

               if ( pidn != didm )      // skip if current Model not parent
                  continue;

               indx           = recn.section( ":", 1, 1 ).toInt();
               cdesc          = tdess.at( indx );
               cdesc.recState = record_state_flag( cdesc, mpstate );

               descs << cdesc;          // output Noise rec

               noutN++;
            }
         }
      }
   }

   if ( noutR != countR  ||  noutE != countE  ||
        noutM != countM  ||  noutN != countN )
   {  // not all accounted for, so we will need some dummy parents
      DbgLv(1) << "sort_desc: count REMN"
         << countR << countE << countM << countN;
      DbgLv(1) << "sort_desc: nout REMN"
         << noutR << noutE << noutM << noutN;
   }
}

// review sorted string lists for duplicate GUIDs
bool US_DataModel::review_descs( QStringList& sorts,
      QVector< DataDesc >& descv )
{
   bool           abort = false;
   int            nrecs = sorts.size();
   int            nmult = 0;
   int            kmult = 0;
   int            ityp;
   QString        cGUID;
   QString        pGUID;
   QString        rtyp;
   QVector< int > multis;
   const char* rtyps[] = { "RawData", "EditedData", "Model", "Noise" };

   if ( nrecs < 1 )
      return abort;

   int ii = sorts[ 0 ].section( ":", 1, 1 ).toInt();
   ityp   = descv[ ii ].recType;
   rtyp   = QString( rtyps[ ityp - 1 ] );

   if ( descv[ ii ].recordID >= 0 )
      rtyp   = "DB " + rtyp;
   else
      rtyp   = "Local " + rtyp;
DbgLv(2) << "RvwD: ii ityp rtyp nrecs" << ii << ityp << rtyp << nrecs;

   for ( int ii = 1; ii < nrecs; ii++ )
   {  // do a pass to determine if there are duplicate GUIDs
      cGUID    = sorts[ ii ].section( ":", 2, 2 );     // current rec GUID
      kmult    = 0;                                    // flag no multiples yet

      for ( int jj = 0; jj < ii; jj++ )
      {  // review all the records preceeding this one
         pGUID    = sorts[ jj ].section( ":", 2, 2 );  // a previous GUID

         if ( pGUID == cGUID )
         {  // found a duplicate
            kmult++;

            if ( ! multis.contains( jj ) )
            {  // not yet marked, so mark previous as multiple
               multis << jj;    // save index
               nmult++;         // bump count
            }

            else  // if it was marked, we can quit the inner loop
               break;
         }
      }

      if ( kmult > 0 )
      {  // this pass found a duplicate:  save the index and bump count
         multis << ii;
         nmult++;
      }
//DbgLv(2) << "RvwD:   ii kmult nmult" << ii << kmult << nmult;
   }

DbgLv(1) << "RvwD:      nmult" << nmult;
   if ( nmult > 0 )
   {  // there were multiple instances of the same GUID
      QMessageBox msgBox;
      QString     msg;

      // format a message for the warning pop-up
      msg  =
         tr( "There are %1 %2 records that have\n" ).arg( nmult ).arg( rtyp ) +
         tr( "the same GUID as another.\n" ) +
         tr( "You should correct the situation before proceeding.\n" ) +
         tr( "  Click \"Ok\" to see details, then abort.\n" ) +
         tr( "  Click \"Ignore\" to proceed to further review.\n" );
      msgBox.setWindowTitle( tr( "Duplicate %1 Records" ).arg( rtyp ) );
      msgBox.setText( msg );
      msgBox.setStandardButtons( QMessageBox::Ok | QMessageBox::Ignore );
      msgBox.setDefaultButton( QMessageBox::Ok );

      if ( msgBox.exec() == QMessageBox::Ok )
      {  // user wants details, so display them
         QString fileexts = tr( "Text,Log files (*.txt *.log);;" )
            + tr( "All files (*)" );
         QString pGUID = "";
         QString cGUID;
         QString label;

         msg =
            tr( "Review the details below on duplicate records.\n" ) +
            tr( "Save or Print the contents of this message.\n" ) +
            tr( "Decide which of the duplicates should be removed.\n" ) +
            tr( "Close the main US_DataModel window after exiting here.\n" ) +
            tr( "\nSummary of Duplicates:\n\n" );

         for ( int ii = 0; ii < nmult; ii++ )
         {  // add summary lines on duplicates
            int jj = multis.at( ii );
            cGUID  = sorts.at( jj ).section( ":", 2, 2 );
            label  = sorts.at( jj ).section( ":", 0, 0 );

            if ( cGUID != pGUID )
            {  // first instance of this GUID:  show GUID
               msg  += tr( "GUID:  " ) + cGUID + "\n";
               pGUID = cGUID;
            }

            // one label line for each multiple
            msg  += tr( "  Label:  " ) + label + "\n";
         }

         msg += tr( "\nDetails of Duplicates:\n\n" );

         for ( int ii = 0; ii < nmult; ii++ )
         {  // add detail lines
            int jj = multis.at( ii );
            cGUID  = sorts.at( jj ).section( ":", 2, 2 );
            pGUID  = sorts.at( jj ).section( ":", 3, 3 );
            label  = sorts.at( jj ).section( ":", 0, 0 );
            int kk = sorts.at( jj ).section( ":", 1, 1 ).toInt();
            cdesc  = descv[ kk ];

            msg   += tr( "GUID:  " ) + cGUID + "\n" +
               tr( "  ParentGUID:  " ) + pGUID + "\n" +
               tr( "  Label:  " ) + label + "\n" +
               tr( "  Description:  " ) + cdesc.description + "\n" +
               tr( "  DB record ID:  %1" ).arg( cdesc.recordID ) + "\n" +
               tr( "  File Directory:  " ) +
               cdesc.filename.section( "/",  0, -2 ) + "\n" +
               tr( "  File Name:  " ) +
               cdesc.filename.section( "/", -1, -1 ) + "\n" +
               tr( "  Last Mod Date:  " ) +
               cdesc.lastmodDate + "\n";
         }

         // pop up text dialog
         US_Editor* editd = new US_Editor( US_Editor::LOAD, true, fileexts );
         editd->setWindowTitle( tr( "Data Set Duplicate GUID Details" ) );
         editd->move( QCursor::pos() + QPoint( 200, 200 ) );
         editd->resize( 600, 500 );
         editd->e->setFont( QFont( "monospace", US_GuiSettings::fontSize() ) );
         editd->e->setText( msg );
         editd->show();

         abort = true;      // tell caller to abort data tree build
      }

      else
      {
         abort = false;     // signal to proceed with data tree build
      }
   }
DbgLv(1) << "review_descs   abort" << abort;

   return abort;
}

// find index of substring at given position in strings of string list
int US_DataModel::index_substring( QString ss, int ixs, QStringList& sl )
{
   QString sexp = "XXX";
   QRegExp rexp;

   if ( ixs == 0 )
      sexp = ss + ":*";        // label at beginning of strings in list

   else if ( ixs == 1  ||  ixs == 2 )
      sexp = "*:" + ss + ":*"; // RecIndex/recGUID in middle of list strings

   else if ( ixs == 3 )
      sexp = "*:" + ss;        // parentGUID at end of strings in list

   rexp = QRegExp( sexp, Qt::CaseSensitive, QRegExp::Wildcard );

   return sl.indexOf( rexp );
}

// get sublist from string list of substring matches at a given string position
QStringList US_DataModel::filter_substring( QString ss, int ixs,
   QStringList& sl )
{
   QStringList subl;

   if ( ixs == 0 )
      // match label at beginning of strings in list
      subl = sl.filter( QRegExp( "^" + ss + ":" ) );

   else if ( ixs == 1  ||  ixs == 2 )
      // match RecIndex or recGUID in middle of strings in list
      subl = sl.filter( ":" + ss + ":" );

   else if ( ixs == 3 )
      // match parentGUID at end of strings in list
      subl = sl.filter( QRegExp( ":" + ss + "$" ) );

   return subl;
}

// list orphans of a record type (in rec list, no tie to parent list)
QStringList US_DataModel::list_orphans( QStringList& rlist,
   QStringList& plist )
{
   QStringList olist;

   for ( int ii = 0; ii < rlist.size(); ii++ )
   {  // examine parentGUID for each record in the list
      QString pGUID = rlist.at( ii ).section( ":", 3, 3 );

      // see if it is the recordGUID of any in the potential parent list
      if ( index_substring( pGUID, 2, plist ) < 0 )
         olist << rlist.at( ii ); // no parent found, so add to the orphan list
   }

   return olist;
}

// return a record state flag with parent state ORed in
int US_DataModel::record_state_flag( DataDesc descr, int pstate )
{
   int state = descr.recState;

   if ( descr.recState == NOSTAT  ||
        descr.description.contains( "-ARTIFICIAL" ) )
      state = NOSTAT;                    // mark a dummy record

   else
   {  // detect and mark parentage of non-dummy
      if ( ( pstate & REC_DB ) != 0 )
         state = state | PAR_DB;         // mark a record with db parent

      if ( ( pstate & REC_LO ) != 0 )
         state = state | PAR_LO;         // mark a record with local parent
   }

   return state;
}

// compose concatenation on which to sort (label:index:dataGUID:parentGUID)
QString US_DataModel::sort_string( DataDesc ddesc, int indx )
{  // create string for ascending sort on label
   QString label  = ( ddesc.recType < 3 ) ? ddesc.label : ddesc.description;
   int     lablen = label.length();
   if ( lablen < maxdlen )
      label = label.leftJustified( maxdlen, ' ' );

   QString ostr  = label                              // label to sort on
      + ":"      + QString().sprintf( "%4.4d", indx ) // index in desc. vector
      + ":"      + ddesc.dataGUID                     // data GUID
      + ":"      + ddesc.parentGUID;                  // parent GUID
   return ostr;
}

// compose string describing model type
QString US_DataModel::model_type( int imtype, int nassoc, int gtype, bool isMC )
{
   QString mtype;

   // format the base model type string
   switch ( imtype )
   {
      default:
      case (int)US_Model::MANUAL:
         mtype = "MANUAL";
         break;
      case (int)US_Model::TWODSA:
         mtype = "2DSA";
         break;
      case (int)US_Model::TWODSA_MW:
         mtype = "2DSA-MW";
         break;
      case (int)US_Model::GA:
         mtype = "GA";
         break;
      case (int)US_Model::GA_MW:
         mtype = "GA-MW";
         break;
      case (int)US_Model::COFS:
         mtype = "COFS";
         break;
      case (int)US_Model::FE:
         mtype = "FE";
         break;
      case (int)US_Model::ONEDSA:
         mtype = "1DSA";
         break;
   }

   // add RA for Reversible Associations (if associations count > 1)
   if ( nassoc > 1 )
      mtype = mtype + "-RA";

   // add FM | GL | SG for Fit-Meniscus|GLobal|SuperGlobal
   if ( gtype == (int)US_Model::MENISCUS )
      mtype = mtype + "-FM";

   else if ( gtype == (int)US_Model::GLOBAL )
      mtype = mtype + "-GL";

   else if ( gtype == (int)US_Model::SUPERGLOBAL )
      mtype = mtype + "-SG";

   // add MC for Monte Carlo
   if ( isMC )
      mtype = mtype + "-MC";

   return mtype;
}

// compose string describing model type
QString US_DataModel::model_type( US_Model model )
{
   // return model type string based on flags in the model object
   return model_type( (int)model.analysis, model.associations.size(),
                      (int)model.global,   model.monteCarlo );
}

// compose string describing model type
QString US_DataModel::model_type( QString modxml )
{
   QChar quo( '"' );
   int   jj;
   int   imtype;
   int   nassoc;
   int   gtype;
   bool  isMC;

   // model type number from type attribute
   jj       = modxml.indexOf( " analysisType=" );
   imtype   = ( jj < 1 ) ? 0 : modxml.mid( jj ).section( quo, 1, 1 ).toInt();

   // count of associations is count of k_eq attributes present
   nassoc   = modxml.count( "k_eq=" );

   // global type number from type attribute
   jj       = modxml.indexOf( " globalType=" );
   gtype    = ( jj < 1 ) ? 0 : modxml.mid( jj ).section( quo, 1, 1 ).toInt();

   // flag if MonteCarlo
   jj       = modxml.indexOf( " MonteCarlo=\"1" );
   isMC     = ( jj > 0 );

   // return model type string based on integer flags
   return model_type( imtype, nassoc, gtype, isMC );
}

void US_DataModel::dummy_data()
{
   adescs.clear();
   ddescs.clear();
   ldescs.clear();

   cdesc.recType        = 1;
   cdesc.recState       = REC_DB | PAR_DB;
   cdesc.subType        = "";
   cdesc.label          = "item_1_2";
   cdesc.description    = "demo1_veloc";
   cdesc.dataGUID       = "demo1_veloc";
   cdesc.recordID       = 1;
   cdesc.filename       = "";
   adescs<<cdesc;
   ddescs<<cdesc;

   cdesc.recType        = 2;
   cdesc.recState       = REC_DB | REC_LO | PAR_DB | PAR_LO;
   cdesc.subType        = "RA";
   cdesc.label          = "item_2_2";
   cdesc.description    = "demo1_veloc";
   cdesc.contents       = "AA 12 AA 12";
   cdesc.recordID       = 2;
   cdesc.filename       = "demo1_veloc_edit.xml";
   adescs<<cdesc;
   ddescs<<cdesc;
   ldescs<<cdesc;

   cdesc.recType        = 3;
   cdesc.recState       = REC_LO | PAR_LO;
   cdesc.subType        = "2DSA";
   cdesc.label          = "item_3_2";
   cdesc.description    = "demo1_veloc.sa2d.model.11";
   cdesc.recordID       = -1;
   cdesc.filename       = "demo1_veloc_model.xml";
   adescs<<cdesc;
   ldescs<<cdesc;

   cdesc.recType        = 4;
   cdesc.recState       = REC_DB | REC_LO | PAR_DB | PAR_LO;
   cdesc.subType        = "TI";
   cdesc.label          = "item_4_2";
   cdesc.description    = "demo1_veloc.ti_noise";
   cdesc.contents       = "BB 12 AA 13";
   cdesc.recordID       = 3;
   cdesc.filename       = "demo1_veloc_model.xml";
   adescs<<cdesc;
   ddescs<<cdesc;
   ldescs<<cdesc;

   cdesc.recType        = 2;
   cdesc.recState       = NOSTAT;
   cdesc.subType        = "RA";
   cdesc.label          = "item_5_2";
   cdesc.description    = "demo1_veloc";
   cdesc.recordID       = -1;
   cdesc.filename       = "";
   adescs<<cdesc;
}

QString US_DataModel::expGUIDauc( QString aucfile )
{
   QString expGUID = "00000000-0000-0000-0000-000000000000";
   QString expfnam = aucfile.section( "/", -1, -1 )
                            .section( ".", 0, 1 ) + ".xml";
   QString expfile = aucfile.section( "/", 0, -2 ) + "/" + expfnam;

   QFile file( expfile );

   if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QXmlStreamReader xml( &file );

      while( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "experiment" )
         {
            QXmlStreamAttributes a = xml.attributes();
            expGUID  = a.value( "guid" ).toString();
            break;
         }
      }

      file.close();
   }

   return expGUID;
}


