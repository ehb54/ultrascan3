/****************************************************************************
** US_License meta object code from reading C++ file 'us_license.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_license.h"
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

const char *US_License::className() const
{
    return "US_License";
}

QMetaObject *US_License::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_License( "US_License", &US_License::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_License::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_License", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_License::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_License", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_License::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"help", 0, 0 };
    static const QUMethod slot_1 = {"cancel", 0, 0 };
    static const QUMethod slot_2 = {"save", 0, 0 };
    static const QUMethod slot_3 = {"import", 0, 0 };
    static const QUMethod slot_4 = {"request", 0, 0 };
    static const QUMethod slot_5 = {"display", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_firstname", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_lastname", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_institution", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_address", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_city", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_state", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_zip", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_phone", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_email", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_version", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"update_licensetype", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"update_code", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"update_expiration", 1, param_slot_18 };
    static const QUMethod slot_19 = {"update_opteron_rb", 0, 0 };
    static const QUMethod slot_20 = {"update_intel_rb", 0, 0 };
    static const QUMethod slot_21 = {"update_sparc_rb", 0, 0 };
    static const QUMethod slot_22 = {"update_mac_rb", 0, 0 };
    static const QUMethod slot_23 = {"update_sgi_rb", 0, 0 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_24 = {"update_os", 1, param_slot_24 };
    static const QUMethod slot_25 = {"captureStdout", 0, 0 };
    static const QUMethod slot_26 = {"captureStderr", 0, 0 };
    static const QUMethod slot_27 = {"endProcess", 0, 0 };
    static const QUParameter param_slot_28[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_28 = {"closeEvent", 1, param_slot_28 };
    static const QMetaData slot_tbl[] = {
	{ "help()", &slot_0, QMetaData::Public },
	{ "cancel()", &slot_1, QMetaData::Public },
	{ "save()", &slot_2, QMetaData::Public },
	{ "import()", &slot_3, QMetaData::Public },
	{ "request()", &slot_4, QMetaData::Public },
	{ "display()", &slot_5, QMetaData::Public },
	{ "update_firstname(const QString&)", &slot_6, QMetaData::Public },
	{ "update_lastname(const QString&)", &slot_7, QMetaData::Public },
	{ "update_institution(const QString&)", &slot_8, QMetaData::Public },
	{ "update_address(const QString&)", &slot_9, QMetaData::Public },
	{ "update_city(const QString&)", &slot_10, QMetaData::Public },
	{ "update_state(int)", &slot_11, QMetaData::Public },
	{ "update_zip(const QString&)", &slot_12, QMetaData::Public },
	{ "update_phone(const QString&)", &slot_13, QMetaData::Public },
	{ "update_email(const QString&)", &slot_14, QMetaData::Public },
	{ "update_version(int)", &slot_15, QMetaData::Public },
	{ "update_licensetype(int)", &slot_16, QMetaData::Public },
	{ "update_code(const QString&)", &slot_17, QMetaData::Public },
	{ "update_expiration(const QString&)", &slot_18, QMetaData::Public },
	{ "update_opteron_rb()", &slot_19, QMetaData::Public },
	{ "update_intel_rb()", &slot_20, QMetaData::Public },
	{ "update_sparc_rb()", &slot_21, QMetaData::Public },
	{ "update_mac_rb()", &slot_22, QMetaData::Public },
	{ "update_sgi_rb()", &slot_23, QMetaData::Public },
	{ "update_os(int)", &slot_24, QMetaData::Public },
	{ "captureStdout()", &slot_25, QMetaData::Public },
	{ "captureStderr()", &slot_26, QMetaData::Public },
	{ "endProcess()", &slot_27, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_28, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_License", parentObject,
	slot_tbl, 29,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_License.setMetaObject( metaObj );
    return metaObj;
}

void* US_License::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_License" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_License::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: help(); break;
    case 1: cancel(); break;
    case 2: save(); break;
    case 3: import(); break;
    case 4: request(); break;
    case 5: display(); break;
    case 6: update_firstname((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_lastname((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update_institution((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: update_address((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: update_city((const QString&)static_QUType_QString.get(_o+1)); break;
    case 11: update_state((int)static_QUType_int.get(_o+1)); break;
    case 12: update_zip((const QString&)static_QUType_QString.get(_o+1)); break;
    case 13: update_phone((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: update_email((const QString&)static_QUType_QString.get(_o+1)); break;
    case 15: update_version((int)static_QUType_int.get(_o+1)); break;
    case 16: update_licensetype((int)static_QUType_int.get(_o+1)); break;
    case 17: update_code((const QString&)static_QUType_QString.get(_o+1)); break;
    case 18: update_expiration((const QString&)static_QUType_QString.get(_o+1)); break;
    case 19: update_opteron_rb(); break;
    case 20: update_intel_rb(); break;
    case 21: update_sparc_rb(); break;
    case 22: update_mac_rb(); break;
    case 23: update_sgi_rb(); break;
    case 24: update_os((int)static_QUType_int.get(_o+1)); break;
    case 25: captureStdout(); break;
    case 26: captureStderr(); break;
    case 27: endProcess(); break;
    case 28: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_License::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_License::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_License::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
