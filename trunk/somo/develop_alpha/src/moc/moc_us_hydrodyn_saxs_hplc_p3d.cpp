/****************************************************************************
** US_Hydrodyn_Saxs_Hplc_P3d meta object code from reading C++ file 'us_hydrodyn_saxs_hplc_p3d.h'
**
** Created: Tue Feb 25 10:33:30 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_saxs_hplc_p3d.h"
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

const char *US_Hydrodyn_Saxs_Hplc_P3d::className() const
{
    return "US_Hydrodyn_Saxs_Hplc_P3d";
}

QMetaObject *US_Hydrodyn_Saxs_Hplc_P3d::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Saxs_Hplc_P3d( "US_Hydrodyn_Saxs_Hplc_P3d", &US_Hydrodyn_Saxs_Hplc_P3d::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Saxs_Hplc_P3d::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Hplc_P3d", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Saxs_Hplc_P3d::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Hplc_P3d", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Saxs_Hplc_P3d::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"plot_all", 0, 0 };
    static const QUMethod slot_1 = {"plot", 0, 0 };
    static const QUMethod slot_2 = {"quit", 0, 0 };
    static const QUMethod slot_3 = {"help", 0, 0 };
    static const QUMethod slot_4 = {"update_enables", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_5 = {"closeEvent", 1, param_slot_5 };
    static const QMetaData slot_tbl[] = {
	{ "plot_all()", &slot_0, QMetaData::Private },
	{ "plot()", &slot_1, QMetaData::Private },
	{ "quit()", &slot_2, QMetaData::Private },
	{ "help()", &slot_3, QMetaData::Private },
	{ "update_enables()", &slot_4, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_5, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Saxs_Hplc_P3d", parentObject,
	slot_tbl, 6,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Saxs_Hplc_P3d.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Saxs_Hplc_P3d::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Saxs_Hplc_P3d" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Saxs_Hplc_P3d::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: plot_all(); break;
    case 1: plot(); break;
    case 2: quit(); break;
    case 3: help(); break;
    case 4: update_enables(); break;
    case 5: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Saxs_Hplc_P3d::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Saxs_Hplc_P3d::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Saxs_Hplc_P3d::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
