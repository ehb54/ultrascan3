/****************************************************************************
** US_Hydrodyn_Saxs_Hplc_Dctr meta object code from reading C++ file 'us_hydrodyn_saxs_hplc_dctr.h'
**
** Created: Tue Feb 25 10:33:30 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_saxs_hplc_dctr.h"
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

const char *US_Hydrodyn_Saxs_Hplc_Dctr::className() const
{
    return "US_Hydrodyn_Saxs_Hplc_Dctr";
}

QMetaObject *US_Hydrodyn_Saxs_Hplc_Dctr::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Saxs_Hplc_Dctr( "US_Hydrodyn_Saxs_Hplc_Dctr", &US_Hydrodyn_Saxs_Hplc_Dctr::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Saxs_Hplc_Dctr::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Hplc_Dctr", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Saxs_Hplc_Dctr::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Saxs_Hplc_Dctr", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Saxs_Hplc_Dctr::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"set_uv", 0, 0 };
    static const QUMethod slot_1 = {"set_ri", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"ri_conv_text", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"uv_conv_text", 1, param_slot_3 };
    static const QUMethod slot_4 = {"help", 0, 0 };
    static const QUMethod slot_5 = {"quit", 0, 0 };
    static const QUMethod slot_6 = {"keep", 0, 0 };
    static const QUMethod slot_7 = {"save", 0, 0 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_8 = {"closeEvent", 1, param_slot_8 };
    static const QMetaData slot_tbl[] = {
	{ "set_uv()", &slot_0, QMetaData::Private },
	{ "set_ri()", &slot_1, QMetaData::Private },
	{ "ri_conv_text(const QString&)", &slot_2, QMetaData::Private },
	{ "uv_conv_text(const QString&)", &slot_3, QMetaData::Private },
	{ "help()", &slot_4, QMetaData::Private },
	{ "quit()", &slot_5, QMetaData::Private },
	{ "keep()", &slot_6, QMetaData::Private },
	{ "save()", &slot_7, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_8, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Saxs_Hplc_Dctr", parentObject,
	slot_tbl, 9,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Saxs_Hplc_Dctr.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Saxs_Hplc_Dctr::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Saxs_Hplc_Dctr" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Saxs_Hplc_Dctr::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: set_uv(); break;
    case 1: set_ri(); break;
    case 2: ri_conv_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: uv_conv_text((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: help(); break;
    case 5: quit(); break;
    case 6: keep(); break;
    case 7: save(); break;
    case 8: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Saxs_Hplc_Dctr::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Saxs_Hplc_Dctr::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Saxs_Hplc_Dctr::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
