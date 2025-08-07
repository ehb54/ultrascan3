//! \file us_ramp.h
#ifndef US_RAMP_H
#define US_RAMP_H

#include "us_dataIO.h"
#include "us_extern.h"
#include "us_mwlramp.h"
#include "us_solution.h"

//! \class US_Ramp
//!        This class provides the ability to convert raw ramp data
//!        to the file format used by US3 for ramp data.
class US_Ramp {
 public:
  //!  A constant value that identifies the format of the auc raw data.
  //!  Internal programs use this value to determine if the format of the
  //!  file is known.
  static const uint format_version = 1;

  US_Ramp();

  //! \brief   Some status codes returned by the us_convert program
  enum ioError {
    OK,           //!< Ok, no error
    CANTOPEN,     //!< The file cannot be opened
    DUP_RUNID,    //!< The given run ID already exists
    NOPERSON,     //!< The person specified doesn't exist
    NODATA,       //!< There is no data to read or write
    NODB,         //!< Connection to database cannot be made
    NOAUC,        //!< AUC File cannot be opened
    NOXML,        //!< XML data has not been entered
    BADXML,       //!< XML not formed correctly
    BADGUID,      //!< GUID read in the XML was not found in the database
    PARTIAL_XML,  //!< XML data has not been entered for all c/c/w combos
    NOT_WRITTEN   //!< Data was not written
  };

  //! \brief  Class that contains information about relevant
  //!         cell/channel/wavelength combinations
  class TripleInfo {
   public:
    int tripleID;        //!< The ID of this c/c/w combination (rawDataID) //KL:
                         //!< unsure if its useful
    QString tripleDesc;  //!< The description of this triple ( e.g., "2 / A" )
                         //!< //KL deleted the wl
    QString description;  //!< A text description of this triple
    char tripleGUID[16];  //!< The GUID of this triple
    //          QVector<tripleGUID[16]> chanGUIDs; //! The GUIDs of the triples
    //          of this channel
    QString tripleFilename;  //!< The filename of this auc file
    bool excluded;           //!< Whether triple was dropped or not
    int centerpiece;         //!< The ID of the centerpiece used
    US_Solution solution;    //!< The solution information for triple
    TripleInfo();            //!< A generic constructor
    void clear(void);        //!< Clear all triple info
    void show(void);         //!< Show triple info for debug
  };

  //! \brief Generic constructor for the US_Convert class. This
  //!        constructor establishes the dialog and its relationship to
  //!        the parent dialog.

  //! \brief Writes the converted US3 data to disk.
  //! \param rawConvertedData  A reference to a structure provided by the
  //!           calling function that already contains the US3
  //!           raw converted data.
  //! \param all_chaninfo      A reference to a structure provided by the
  //!           calling function that already contains all the different
  //!           cell/channel/wavelength combinations in the data.
  //! \param runID             The run ID of the experiment.
  //! \param dirname           Directory in which files are to be written.
  //! \returns      One of the ioError status codes, above
  static int saveToDisk(QVector<US_mwlRamp::RampRawData*>&, QList<TripleInfo>&,
                        QString, QString);

  //! \brief Writes one cellchan combo to one .ramp file
  static int writeRawData(QString file, US_mwlRamp::RampRawData& data);

  //! \brief Reads a saved Ramp run from US3 Disk
  static int readUS3Disk(QString dir,
                         QVector<US_mwlRamp::RampRawData>& rawConvertedData,
                         QList<TripleInfo>& cellchans);

  //! \brief Reads one cellchan combo from a .ramp file
  static int readRawData(const QString file, US_mwlRamp::RampRawData& data);

 private:
  static int setTriples(QList<US_DataIO::BeckmanRawScan>& rawLegacyData,
                        QList<TripleInfo>& triples, QString runType,
                        double tolerance);

  static void setCcwTriples(QList<US_DataIO::BeckmanRawScan>& rawLegacyData,
                            QList<TripleInfo>& triples, double tolerance);

  static void setCcrTriples(QList<US_DataIO::BeckmanRawScan>& rawLegacyData,
                            QList<TripleInfo>& triples, double tolerance);

  static void setInterpolated(unsigned char*, int);

  static void write(QDataStream&, const char*, int, quint32&);
  static void read(QDataStream&, char*, int, quint32&);
};
#endif
