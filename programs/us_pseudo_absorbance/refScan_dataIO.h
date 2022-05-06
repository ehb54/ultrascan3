#ifndef REFSCAN_DATAIO_H
#define REFSCAN_DATAIO_H

#include <QtCore>
#include <us_crc.h>

class refScanDataIO
{
public:
    enum ioError { OK, CANTOPEN, BADCRC, NOT_RSDATA, BADTYPE };
    class RefData
          {
             public:
             char    type[ 2 ];         //!< Data type: "RA"|"IP"|"RI"|"FI"|"WA"|"WI"
             int     nWavelength;
             int     nPoints;
             QVector< double > wavelength;
             QVector< double > xValues;
             QVector< QVector< double > > rValues;
             QVector< QVector< double > > std;
             void clear();
          };
    static int     writeRefData( const QString&, RefData& );
    static int     readRefData ( const QString&, RefData& );
    static QString errorString ( int );

private:
    union union16{
        quint16  ui;
        char   c[2];
    };
    union union32{
        float     n;
        quint32  ui;
        char   c[4];
    };
    union union64 {
        double    n;
        quint64  ui;
        char   c[8];
    };

    static void write      ( QDataStream&, const char*, int, quint32& );
    static void read       ( QDataStream&,       char*, int, quint32& );

};

#endif // REFSCAN_DATAIO_H
