/****************************************************************************
** US_Config_GUI meta object code from reading C++ file 'us_config_gui.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_config_gui.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <Q3Frame>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Config_GUI::className() const
{
    return "US_Config_GUI";
}

QMetaObject *US_Config_GUI::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Config_GUI( "US_Config_GUI", &US_Config_GUI::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Config_GUI::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Config_GUI", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Config_GUI::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Config_GUI", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Config_GUI::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"help", 0, 0 };
    static const QUMethod slot_1 = {"cancel", 0, 0 };
    static const QUMethod slot_2 = {"save", 0, 0 };
    static const QUMethod slot_3 = {"update_screen", 0, 0 };
    static const QUMethod slot_4 = {"open_browser_dir", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_browser", 1, param_slot_5 };
    static const QUMethod slot_6 = {"open_tmp_dir", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_tmp_dir", 1, param_slot_7 };
    static const QUMethod slot_8 = {"open_data_dir", 0, 0 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_data_dir", 1, param_slot_9 };
    static const QUMethod slot_10 = {"open_archive_dir", 0, 0 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_archive_dir", 1, param_slot_11 };
    static const QUMethod slot_12 = {"open_root_dir", 0, 0 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_root_dir", 1, param_slot_13 };
    static const QUMethod slot_14 = {"open_result_dir", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_result_dir", 1, param_slot_15 };
    static const QUMethod slot_16 = {"open_html_dir", 0, 0 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"update_html_dir", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"update_temperature_tol", 1, param_slot_18 };
    static const QUMethod slot_19 = {"open_system_dir", 0, 0 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"update_system_dir", 1, param_slot_20 };
    static const QUMethod slot_21 = {"open_help_dir", 0, 0 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"update_help_dir", 1, param_slot_22 };
    static const QUMethod slot_23 = {"update_on_button", 0, 0 };
    static const QUMethod slot_24 = {"update_off_button", 0, 0 };
    static const QUMethod slot_25 = {"update_color", 0, 0 };
    static const QUParameter param_slot_26[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_26 = {"update_margin", 1, param_slot_26 };
    static const QUMethod slot_27 = {"update_font", 0, 0 };
    static const QUMethod slot_28 = {"update_database", 0, 0 };
    static const QUParameter param_slot_29[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_29 = {"update_numThreads", 1, param_slot_29 };
    static const QUMethod slot_30 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_31[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_31 = {"closeEvent", 1, param_slot_31 };
    static const QMetaData slot_tbl[] = {
	{ "help()", &slot_0, QMetaData::Public },
	{ "cancel()", &slot_1, QMetaData::Public },
	{ "save()", &slot_2, QMetaData::Public },
	{ "update_screen()", &slot_3, QMetaData::Public },
	{ "open_browser_dir()", &slot_4, QMetaData::Public },
	{ "update_browser(const QString&)", &slot_5, QMetaData::Public },
	{ "open_tmp_dir()", &slot_6, QMetaData::Public },
	{ "update_tmp_dir(const QString&)", &slot_7, QMetaData::Public },
	{ "open_data_dir()", &slot_8, QMetaData::Public },
	{ "update_data_dir(const QString&)", &slot_9, QMetaData::Public },
	{ "open_archive_dir()", &slot_10, QMetaData::Public },
	{ "update_archive_dir(const QString&)", &slot_11, QMetaData::Public },
	{ "open_root_dir()", &slot_12, QMetaData::Public },
	{ "update_root_dir(const QString&)", &slot_13, QMetaData::Public },
	{ "open_result_dir()", &slot_14, QMetaData::Public },
	{ "update_result_dir(const QString&)", &slot_15, QMetaData::Public },
	{ "open_html_dir()", &slot_16, QMetaData::Public },
	{ "update_html_dir(const QString&)", &slot_17, QMetaData::Public },
	{ "update_temperature_tol(const QString&)", &slot_18, QMetaData::Public },
	{ "open_system_dir()", &slot_19, QMetaData::Public },
	{ "update_system_dir(const QString&)", &slot_20, QMetaData::Public },
	{ "open_help_dir()", &slot_21, QMetaData::Public },
	{ "update_help_dir(const QString&)", &slot_22, QMetaData::Public },
	{ "update_on_button()", &slot_23, QMetaData::Public },
	{ "update_off_button()", &slot_24, QMetaData::Public },
	{ "update_color()", &slot_25, QMetaData::Public },
	{ "update_margin(int)", &slot_26, QMetaData::Public },
	{ "update_font()", &slot_27, QMetaData::Public },
	{ "update_database()", &slot_28, QMetaData::Public },
	{ "update_numThreads(double)", &slot_29, QMetaData::Public },
	{ "setup_GUI()", &slot_30, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_31, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Config_GUI", parentObject,
	slot_tbl, 32,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Config_GUI.setMetaObject( metaObj );
    return metaObj;
}

void* US_Config_GUI::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Config_GUI" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Config_GUI::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: help(); break;
    case 1: cancel(); break;
    case 2: save(); break;
    case 3: update_screen(); break;
    case 4: open_browser_dir(); break;
    case 5: update_browser((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: open_tmp_dir(); break;
    case 7: update_tmp_dir((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: open_data_dir(); break;
    case 9: update_data_dir((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: open_archive_dir(); break;
    case 11: update_archive_dir((const QString&)static_QUType_QString.get(_o+1)); break;
    case 12: open_root_dir(); break;
    case 13: update_root_dir((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: open_result_dir(); break;
    case 15: update_result_dir((const QString&)static_QUType_QString.get(_o+1)); break;
    case 16: open_html_dir(); break;
    case 17: update_html_dir((const QString&)static_QUType_QString.get(_o+1)); break;
    case 18: update_temperature_tol((const QString&)static_QUType_QString.get(_o+1)); break;
    case 19: open_system_dir(); break;
    case 20: update_system_dir((const QString&)static_QUType_QString.get(_o+1)); break;
    case 21: open_help_dir(); break;
    case 22: update_help_dir((const QString&)static_QUType_QString.get(_o+1)); break;
    case 23: update_on_button(); break;
    case 24: update_off_button(); break;
    case 25: update_color(); break;
    case 26: update_margin((int)static_QUType_int.get(_o+1)); break;
    case 27: update_font(); break;
    case 28: update_database(); break;
    case 29: update_numThreads((double)static_QUType_double.get(_o+1)); break;
    case 30: setup_GUI(); break;
    case 31: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Config_GUI::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Config_GUI::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Config_GUI::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
