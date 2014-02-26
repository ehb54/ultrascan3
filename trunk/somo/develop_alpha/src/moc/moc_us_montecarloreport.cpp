/****************************************************************************
** US_Report_MonteCarlo meta object code from reading C++ file 'us_montecarloreport.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_montecarloreport.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Report_MonteCarlo::className() const
{
    return "US_Report_MonteCarlo";
}

QMetaObject *US_Report_MonteCarlo::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Report_MonteCarlo( "US_Report_MonteCarlo", &US_Report_MonteCarlo::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Report_MonteCarlo::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Report_MonteCarlo", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Report_MonteCarlo::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Report_MonteCarlo", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Report_MonteCarlo::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"US_Report_MonteCarlo", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Report_MonteCarlo.setMetaObject( metaObj );
    return metaObj;
}

void* US_Report_MonteCarlo::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Report_MonteCarlo" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool US_Report_MonteCarlo::qt_invoke( int _id, QUObject* _o )
{
    return QWidget::qt_invoke(_id,_o);
}

bool US_Report_MonteCarlo::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Report_MonteCarlo::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool US_Report_MonteCarlo::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
