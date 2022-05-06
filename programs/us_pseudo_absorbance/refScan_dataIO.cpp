#include "refScan_dataIO.h"

int refScanDataIO::writeRefData( const QString& file, RefData& data )
{
    //define unions;
    union union16 u16a, u16b;
    union union32 u32a, u32b;
    // Open the file for writing
    QFile ff( file );
    if ( ! ff.open( QIODevice::WriteOnly ) ) return CANTOPEN;
    QDataStream ds( &ff );

    // Using quint32 to ensure same data size on all platforms
    quint32 crc = 0xffffffffUL;

    // Write magic number
    char magic[ 5 ] = "USRF";
    write( ds, magic, 4, crc );

    // Write data type
    write( ds, data.type, 2, crc );

    // Write nWavelength   
    u16a.ui = (quint16) data.nWavelength;
    qToLittleEndian(u16a.ui, u16b.c);
    write( ds, u16b.c, 2, crc );

    // Write nPoints
    u16a.ui = (quint16) data.nPoints;
    qToLittleEndian(u16a.ui, u16b.c);
    write( ds, u16b.c, 2, crc );

    // Write wavelength
    const double *wp = data.wavelength.data();
    for (int i = 0; i < data.wavelength.size(); ++i){
        u16a.ui = (quint16) wp[i] * 10;
        qToLittleEndian(u16a.ui, u16b.c);
        write( ds, u16b.c, 2, crc );
    }

    // Write xvalues
    double min_x =  data.xValues.at(0);
    double dx = data.xValues.at(1) - data.xValues.at(0);
    u32a.ui = (quint32) qRound(min_x * 1000);
    qToLittleEndian(u32a.ui, u32b.c);
    write( ds, u32b.c, 4, crc );
    u32a.ui = (quint32) qRound(dx * 1000);
    qToLittleEndian(u32a.ui, u32b.c);
    write( ds, u32b.c, 4, crc );

    const double *rp, *sp;
    // Write rvalues selected
    for (int i = 0; i < data.nWavelength; ++i){
        rp = data.rValues.at(i).data();
        for (int j = 0; j < data.nPoints; ++j){
            u32a.n = (float) rp[j];
            qToLittleEndian(u32a.ui, u32b.c);
            write( ds, u32b.c, 4, crc );
        }
    }

    // Write std selected
    for (int i = 0; i < data.nWavelength; ++i){
        sp = data.std.at(i).data();
        for (int j = 0; j < data.nPoints; ++j){
            u32a.n = (float) sp[j];
            qToLittleEndian(u32a.ui, u32b.c);
            write( ds, u32b.c, 4, crc );
        }
    }

    qToLittleEndian( crc, u32a.c ); // crc
    ds.writeRawData( u32a.c, 4 );

    ff.close();

    return OK;
}

int refScanDataIO::readRefData( const QString& file, RefData& data )
{
    //define unions;
    union union16 u16a, u16b;
    union union32 u32a, u32b;
    QFile ff( file );
    if ( ! ff.open( QIODevice::ReadOnly ) ) return CANTOPEN;
    QDataStream ds( &ff );

    int      err = OK;
    quint32  crc = 0xffffffffUL;
    data.xValues.clear();
    data.rValues.clear();
    data.std.clear();
    data.wavelength.clear();

    try
    {
        // Read magic number
        char magic[ 4 ];
        read( ds, magic, 4, crc );
        if ( strncmp( magic, "USRF", 4 ) != 0 ) throw NOT_RSDATA;

        // Read and get the file type
        char type[ 3 ];
        read( ds, type, 2, crc );
        type[ 2 ] = '\0';

        QStringList types = QStringList() << "RA" << "IP" << "RI" << "FI"
                                          << "WA" << "WI";

        if ( ! types.contains( QString( type ) ) ) throw BADTYPE;
        strncpy( data.type, type, 2 );

        // Read nWavelength
        read( ds, u16a.c, 2, crc );
        u16b.ui = qFromLittleEndian( u16a.ui );
        data.nWavelength = (int) u16b.ui;

        // Read nPoints
        read( ds, u16a.c, 2, crc );
        u16b.ui = qFromLittleEndian( u16a.ui );
        data.nPoints = (int) u16b.ui;

        // Read wavelength
        QVector<double> wavelength;
        for (int i = 0; i < data.nWavelength; ++i){
            read( ds, u16a.c, 2, crc );
            u16b.ui = qFromLittleEndian( u16a.ui );
            wavelength << u16b.ui / 10.0;
        }
        data.wavelength << wavelength;
        // Read xvalues
        double min_x, dx;
        read( ds, u32a.c, 4, crc );
        u32b.ui = qFromLittleEndian( u32a.ui );
        min_x = (double) u32b.ui / 1000.0;
        read( ds, u32a.c, 4, crc );
        u32b.ui = qFromLittleEndian( u32a.ui );
        dx = (double) u32b.ui / 1000.0;

        min_x = qRound(min_x * 1000) / 1000.0;
        dx = qRound(dx * 1000) / 1000.0;

        for (int j = 0; j < data.nPoints; ++j)
            data.xValues << min_x + dx * j;

        QVector< QVector< double > > rvalues;
        QVector<double> rval;
        QVector< QVector< double > > std;
        QVector<double> sval;

        // Read rvalues sel
        for (int i = 0; i < data.nWavelength; ++i){
            for (int j = 0; j < data.nPoints; ++j){
                read( ds, u32a.c, 4, crc );
                u32b.ui = qFromLittleEndian( u32a.ui );
                rval << (double) u32b.n;
            }
            rvalues << rval;
            rval.clear();
        }
        data.rValues << rvalues;
        rvalues.clear();

        // Read std sel
        for (int i = 0; i < data.nWavelength; ++i){
            for (int j = 0; j < data.nPoints; ++j){
                read( ds, u32a.c, 4, crc );
                u32b.ui = qFromLittleEndian( u32a.ui );
                sval << (double) u32b.n;
            }
            std << sval;
            sval.clear();
        }
        data.std << std;
        std.clear();

        // Read the crc
        quint32 read_crc;
        ds.readRawData( (char*) &read_crc , 4 );
        if ( crc != qFromLittleEndian( read_crc ) ) throw BADCRC;

    } catch( ioError error )
    {
       err = error;
    }

    ff.close();
    return err;
}

void refScanDataIO::write( QDataStream& ds, const char* c, int len, quint32& crc )
{
   ds.writeRawData( c, len );
   crc = US_Crc::crc32( crc, (unsigned char*) c, len );
}

void refScanDataIO::read( QDataStream& ds, char* cc, int len, quint32& crc )
{
   ds.readRawData( cc, len );
   crc = US_Crc::crc32( crc, (uchar*) cc, len );
}

// Compose and return an error string
QString refScanDataIO::errorString( int code )
{
   switch ( code )
   {
      case OK
         : return QObject::tr( "The operation completed successully" );
      case CANTOPEN  : return QObject::tr( "The file cannot be opened" );
      case BADCRC    : return QObject::tr( "The file was corrupted" );
      case NOT_RSDATA: return QObject::tr( "The file was not valid reference scan data" );
      case BADTYPE   : return QObject::tr( "The filetype was not recognized" );
   }

   return QObject::tr( "Unknown error code" );
}


void refScanDataIO::RefData::clear(){
    type[0] = ' ';
    type[1] = ' ';
    nWavelength = 0;
    nPoints = 0;
    wavelength.clear();
    xValues.clear();
    rValues.clear();
    std.clear();
    return;
}
