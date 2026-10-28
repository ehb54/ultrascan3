#ifndef US_XPN_EXPORT_H
#define US_XPN_EXPORT_H

#include <QStringList>
#include <QVector>

struct US_XpnExportResult
{
   int files = 0;
   QStringList errors;
};

// Capture each optical system's errors before the next export resets them.
template <typename Export>
US_XpnExportResult us_xpn_export_optics( const QVector<int>& optics, Export exportData )
{
   US_XpnExportResult result;
   for ( int opticalSystem : optics )
   {
      const US_XpnExportResult current = exportData( opticalSystem );
      result.files += current.files;
      result.errors += current.errors;
   }
   return result;
}

#endif
