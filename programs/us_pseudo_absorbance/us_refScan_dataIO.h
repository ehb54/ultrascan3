#ifndef US_REFSCAN_DATAIO_H
#define US_REFSCAN_DATAIO_H

#include <QtCore>
#include <us_crc.h>

class US_RefScanDataIO
{
public:
    enum ioError { OK, CANTOPEN, BADCRC, NOT_RSDATA, BADTYPE };
    class RefData
          {
             public:
             char    type[ 2 ];         //!< Data type: "RA"|"IP"|"RI"|"FI"|"WA"|"WI"
             int     nWavelength;
             int     nPoints;
             bool    CAState;          // chromatic aberration correction
             QVector< double > wavelength;
             QVector< double > CAValues;
             QVector< double > xValues;
             QVector< QVector< double > > rValues;
             QVector< QVector< double > > std;
             void clear();
             QVector< QVector< double > > get_CA_corrected(bool&);
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

#endif // US_REFSCAN_DATAIO_H
