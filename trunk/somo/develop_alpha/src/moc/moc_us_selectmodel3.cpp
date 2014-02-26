/****************************************************************************
** US_SelectModel3 meta object code from reading C++ file 'us_selectmodel3.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_selectmodel3.h"
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

const char *US_SelectModel3::className() const
{
    return "US_SelectModel3";
}

QMetaObject *US_SelectModel3::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_SelectModel3( "US_SelectModel3", &US_SelectModel3::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_SelectModel3::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_SelectModel3", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_SelectModel3::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_SelectModel3", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_SelectModel3::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"cancel", 0, 0 };
    static const QUMethod slot_1 = {"check", 0, 0 };
    static const QUMethod slot_2 = {"help", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_mwSlots", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_mwUpperLimit", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_mwLowerLimit", 1, param_slot_5 };
    static const QUMethod slot_6 = {"change_commonVbar", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_vbar", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_vbar_lbl", 2, param_slot_8 };
    static const QUMethod slot_9 = {"read_vbar", 0, 0 };
    static const QUMethod slot_10 = {"resize_me", 0, 0 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_11 = {"closeEvent", 1, param_slot_11 };
    static const QMetaData slot_tbl[] = {
	{ "cancel()", &slot_0, QMetaData::Public },
	{ "check()", &slot_1, QMetaData::Public },
	{ "help()", &slot_2, QMetaData::Public },
	{ "update_mwSlots(const QString&)", &slot_3, QMetaData::Public },
	{ "update_mwUpperLimit(const QString&)", &slot_4, QMetaData::Public },
	{ "update_mwLowerLimit(const QString&)", &slot_5, QMetaData::Public },
	{ "change_commonVbar()", &slot_6, QMetaData::Public },
	{ "update_vbar(const QString&)", &slot_7, QMetaData::Public },
	{ "update_vbar_lbl(float,float)", &slot_8, QMetaData::Public },
	{ "read_vbar()", &slot_9, QMetaData::Public },
	{ "resize_me()", &slot_10, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_11, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_SelectModel3", parentObject,
	slot_tbl, 12,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_SelectModel3.setMetaObject( metaObj );
    return metaObj;
}

void* US_SelectModel3::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_SelectModel3" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_SelectModel3::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: cancel(); break;
    case 1: check(); break;
    case 2: help(); break;
    case 3: update_mwSlots((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_mwUpperLimit((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_mwLowerLimit((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: change_commonVbar(); break;
    case 7: update_vbar((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update_vbar_lbl((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 9: read_vbar(); break;
    case 10: resize_me(); break;
    case 11: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_SelectModel3::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_SelectModel3::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_SelectModel3::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
