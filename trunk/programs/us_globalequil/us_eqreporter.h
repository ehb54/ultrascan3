#ifndef US_EQREPORTER_H
#define US_EQREPORTER_H

#include <QtCore>
#include <QtGui>

#include "us_extern.h"
#include "us_dataIO2.h"
#include "us_globeq_data.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

class US_EqReporter : public QObject
{
	Q_OBJECT
	
	public:
		US_EqReporter( QVector< US_DataIO2::EditedData >&,
            QVector< ScanEdit >&, QVector< EqScanFit >&, EqRunFit&,
            QWidget* = 0 );

      void    scan_diagnostics();
      bool    check_scan_fit( int );
      QString fit_report( FitCtrlPar&, bool, bool, QString& );

	private:
      QVector< US_DataIO2::EditedData >&  dataList;
      QVector< ScanEdit >&                scedits;
      QVector< EqScanFit >&               scanfits;
      EqRunFit&                           runfit;
      QWidget*                            wparent;

      int      dbg_level;

      QString  asters;

   private slots:
      int     index_radius(   US_DataIO2::EditedData*, double );
      QString centerInLine(   const QString&, int, bool, const QChar );
      QString scanInfoHeader( int, int );
      int     maxLineWidth  ( QFontMetrics& fm, const QString& );
};
#endif

