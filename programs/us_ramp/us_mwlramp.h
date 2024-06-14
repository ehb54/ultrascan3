//! \file us_mwlramp.h
#ifndef US_MWLRAMP_H
#define US_MWLRAMP_H

#include "us_extern.h"
#include "us_mwl_data.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()  //!< debug-level-conditioned qDebug()
#endif

//! Class for Ramp Data: importing from raw files or loading from vectors of rawData.
class US_mwlRamp : public QObject
{
    Q_OBJECT

public:
    US_mwlRamp( );
      
    //! Variables read from MWL-ramp files
    class RampRawData
    {
    public:
        QChar 			cell;		//! cell #
        QChar 			chan;		//! channel A or B
        QString 		description; 	//! sample names

        double 			temperature;	//! measurement temperature
        double 			radius;		//! radial position of measurement
        double 			meniscus;	//! radial position of meniscus
        double 			bottom;		//! radial position of cell bottom

        QVector <double> 	rpm;		//! rpm array
        QVector <double> 	time;		//! time array
        QVector <double> 	w2t;		//! w2t array

        QVector <int>		wl_array;	//! wavelength array
        QVector <QVector<double> > intarray;	//! 2D array of all measued values of one cellchan
						//! indexing: [wavelength] [n-th measurement]
       
    };
    
    QString  		cur_dir;  	//!< Currently selected i/p data directory
    QLineEdit*    le_status;  //!< Status report LineEdit
    int       		dbg_level;  //!< Debug level
    int           nfile;      //! #  files to read

    // xml-data read by read_runxml()
    QString   		runID;      //!< Run ID
    double        radius;     //! measured radius position as double
    QStringList		cellchans;   //! vector: cell and channel
    QStringList		samplenames; //! vector: sample name in corresponding channel
    QVector<double>	menisci;	    //! vector: solution meniscus
    QVector<double>	bottoms;	    //! vector: cell bottom

    double 		Temperature;	//! measurement temperature


    // data read from filenames
    QStringList         cells;     	//!< Input Cell strings
    
    void clear ( void );		//! clear class from memory


    //! \brief Import ramp data from a specified directory
    //! \param mwldir         Raw ramp data directory
    //! \param lestat         Status LineEdit pointer
    //! \param allData        Reference to RampRawData vector
    //! \param runID_xml_mwl  Run ID string
    //! \returns              Status of import (true->imported OK)
    bool    import_data   ( QString&, QLineEdit*,
          QVector< US_mwlRamp::RampRawData >& allData, QString & runID_xml_mwl);
 
private:
    QVector< int >               ri_wavelns;  //!< Raw input wavelengths
    QVector< QVector< int > >    ex_wavelns;  //!< Export Wavelengths, ea. cc
    QStringList         fpaths;      	// Input file paths
    QStringList         fnames;      	// Input file names
    int       ncelchn;             //!< Number of Cell/Channels
    int       nlambda;             //!< Number of wavelengths (output)
    int       nlamb_i;             //!< Number of wavelengths (raw input)
    int       curccx;              //!< Current cell/chan index
    int       slambda;             //!< Starting output lambda
    int       elambda;             //!< Ending output lambda
    QMap< QString, int >  counts;  //!< Map of counts ('File','Scan',...)

private slots:
    void   read_runxml ( QDir, QString );
//     void   mapCounts   ( void );
};
#endif
