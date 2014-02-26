/****************************************************************************
** US_AddSaxs meta object code from reading C++ file 'us_hydrodyn_addsaxs.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_addsaxs.h"
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

const char *US_AddSaxs::className() const
{
    return "US_AddSaxs";
}

QMetaObject *US_AddSaxs::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_AddSaxs( "US_AddSaxs", &US_AddSaxs::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_AddSaxs::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_AddSaxs", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_AddSaxs::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_AddSaxs", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_AddSaxs::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUMethod slot_0 = {"add", 0, 0 };
    static const QUMethod slot_1 = {"help", 0, 0 };
    static const QUMethod slot_2 = {"select_file", 0, 0 };
    static const QUMethod slot_3 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_saxs_name", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_a1", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_a2", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_a3", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_a4", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_b1", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_b2", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_b3", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_b4", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_c", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_volume", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"select_saxs", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_16 = {"closeEvent", 1, param_slot_16 };
    static const QMetaData slot_tbl[] = {
	{ "add()", &slot_0, QMetaData::Private },
	{ "help()", &slot_1, QMetaData::Private },
	{ "select_file()", &slot_2, QMetaData::Private },
	{ "setupGUI()", &slot_3, QMetaData::Private },
	{ "update_saxs_name(const QString&)", &slot_4, QMetaData::Private },
	{ "update_a1(const QString&)", &slot_5, QMetaData::Private },
	{ "update_a2(const QString&)", &slot_6, QMetaData::Private },
	{ "update_a3(const QString&)", &slot_7, QMetaData::Private },
	{ "update_a4(const QString&)", &slot_8, QMetaData::Private },
	{ "update_b1(const QString&)", &slot_9, QMetaData::Private },
	{ "update_b2(const QString&)", &slot_10, QMetaData::Private },
	{ "update_b3(const QString&)", &slot_11, QMetaData::Private },
	{ "update_b4(const QString&)", &slot_12, QMetaData::Private },
	{ "update_c(const QString&)", &slot_13, QMetaData::Private },
	{ "update_volume(const QString&)", &slot_14, QMetaData::Private },
	{ "select_saxs(int)", &slot_15, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_16, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_AddSaxs", parentObject,
	slot_tbl, 17,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_AddSaxs.setMetaObject( metaObj );
    return metaObj;
}

void* US_AddSaxs::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_AddSaxs" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool US_AddSaxs::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: add(); break;
    case 1: help(); break;
    case 2: select_file(); break;
    case 3: setupGUI(); break;
    case 4: update_saxs_name((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_a1((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_a2((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_a3((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update_a4((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: update_b1((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: update_b2((const QString&)static_QUType_QString.get(_o+1)); break;
    case 11: update_b3((const QString&)static_QUType_QString.get(_o+1)); break;
    case 12: update_b4((const QString&)static_QUType_QString.get(_o+1)); break;
    case 13: update_c((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: update_volume((const QString&)static_QUType_QString.get(_o+1)); break;
    case 15: select_saxs((int)static_QUType_int.get(_o+1)); break;
    case 16: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_AddSaxs::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_AddSaxs::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool US_AddSaxs::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
