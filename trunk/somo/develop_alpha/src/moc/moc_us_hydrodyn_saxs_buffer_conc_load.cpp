/****************************************************************************
** US_Hydrodyn_Saxs_Buffer_Conc_Load meta object code from reading C++ file 'us_hydrodyn_saxs_buffer_conc_load.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_saxs_buffer_conc_load.h"
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

const char *US_Hydrodyn_Saxs_Buffer_Conc_Load::className() const
{
    return "US_Hydrodyn_Saxs_Buffer_Conc_Load";
}

QMetaObject *US_Hydrodyn_Saxs_Buffer_Conc_Load::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Saxs_Buffer_Conc_Load( "US_Hydrodyn_Saxs_Buffer_Conc_Load", &US_Hydrodyn_Saxs_Buffer_Conc_Load::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Saxs_Buffer_Conc_Load::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Buffer_Conc_Load", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Saxs_Buffer_Conc_Load::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Buffer_Conc_Load", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Saxs_Buffer_Conc_Load::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"update_enables", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ "col", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"col_header_released", 1, param_slot_1 };
    static const QUMethod slot_2 = {"del_row", 0, 0 };
    static const QUMethod slot_3 = {"set_name", 0, 0 };
    static const QUMethod slot_4 = {"set_conc", 0, 0 };
    static const QUMethod slot_5 = {"adjust", 0, 0 };
    static const QUMethod slot_6 = {"trial", 0, 0 };
    static const QUMethod slot_7 = {"cancel", 0, 0 };
    static const QUMethod slot_8 = {"help", 0, 0 };
    static const QUMethod slot_9 = {"set_ok", 0, 0 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_10 = {"closeEvent", 1, param_slot_10 };
    static const QMetaData slot_tbl[] = {
	{ "update_enables()", &slot_0, QMetaData::Private },
	{ "col_header_released(int)", &slot_1, QMetaData::Private },
	{ "del_row()", &slot_2, QMetaData::Private },
	{ "set_name()", &slot_3, QMetaData::Private },
	{ "set_conc()", &slot_4, QMetaData::Private },
	{ "adjust()", &slot_5, QMetaData::Private },
	{ "trial()", &slot_6, QMetaData::Private },
	{ "cancel()", &slot_7, QMetaData::Private },
	{ "help()", &slot_8, QMetaData::Private },
	{ "set_ok()", &slot_9, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_10, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Saxs_Buffer_Conc_Load", parentObject,
	slot_tbl, 11,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Saxs_Buffer_Conc_Load.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Saxs_Buffer_Conc_Load::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Saxs_Buffer_Conc_Load" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Saxs_Buffer_Conc_Load::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_enables(); break;
    case 1: col_header_released((int)static_QUType_int.get(_o+1)); break;
    case 2: del_row(); break;
    case 3: set_name(); break;
    case 4: set_conc(); break;
    case 5: adjust(); break;
    case 6: trial(); break;
    case 7: cancel(); break;
    case 8: help(); break;
    case 9: set_ok(); break;
    case 10: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Saxs_Buffer_Conc_Load::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Saxs_Buffer_Conc_Load::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Saxs_Buffer_Conc_Load::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
