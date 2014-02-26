/****************************************************************************
** US_SelectPlot meta object code from reading C++ file 'us_selectplot.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_selectplot.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_SelectPlot::className() const
{
    return "US_SelectPlot";
}

QMetaObject *US_SelectPlot::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_SelectPlot( "US_SelectPlot", &US_SelectPlot::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_SelectPlot::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_SelectPlot", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_SelectPlot::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_SelectPlot", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_SelectPlot::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"check", 0, 0 };
    static const QUMethod slot_1 = {"change_analysis", 0, 0 };
    static const QUMethod slot_2 = {"change_edit", 0, 0 };
    static const QUMethod slot_3 = {"change_color", 0, 0 };
    static const QUMethod slot_4 = {"change_bw", 0, 0 };
    static const QUMethod slot_5 = {"change_inv", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_6 = {"closeEvent", 1, param_slot_6 };
    static const QMetaData slot_tbl[] = {
	{ "check()", &slot_0, QMetaData::Public },
	{ "change_analysis()", &slot_1, QMetaData::Public },
	{ "change_edit()", &slot_2, QMetaData::Public },
	{ "change_color()", &slot_3, QMetaData::Public },
	{ "change_bw()", &slot_4, QMetaData::Public },
	{ "change_inv()", &slot_5, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_6, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_SelectPlot", parentObject,
	slot_tbl, 7,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_SelectPlot.setMetaObject( metaObj );
    return metaObj;
}

void* US_SelectPlot::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_SelectPlot" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_SelectPlot::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: check(); break;
    case 1: change_analysis(); break;
    case 2: change_edit(); break;
    case 3: change_color(); break;
    case 4: change_bw(); break;
    case 5: change_inv(); break;
    case 6: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_SelectPlot::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_SelectPlot::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_SelectPlot::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
