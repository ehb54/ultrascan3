/****************************************************************************
** US_Hydrodyn_Anaflex_Options meta object code from reading C++ file 'us_hydrodyn_anaflex_options.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_anaflex_options.h"
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

const char *US_Hydrodyn_Anaflex_Options::className() const
{
    return "US_Hydrodyn_Anaflex_Options";
}

QMetaObject *US_Hydrodyn_Anaflex_Options::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Anaflex_Options( "US_Hydrodyn_Anaflex_Options", &US_Hydrodyn_Anaflex_Options::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Anaflex_Options::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Anaflex_Options", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Anaflex_Options::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Anaflex_Options", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Anaflex_Options::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"set_run_anaflex", 0, 0 };
    static const QUMethod slot_1 = {"set_instprofiles", 0, 0 };
    static const QUMethod slot_2 = {"set_run_mode_1", 0, 0 };
    static const QUMethod slot_3 = {"set_run_mode_1_1", 0, 0 };
    static const QUMethod slot_4 = {"set_run_mode_1_2", 0, 0 };
    static const QUMethod slot_5 = {"set_run_mode_1_3", 0, 0 };
    static const QUMethod slot_6 = {"set_run_mode_1_4", 0, 0 };
    static const QUMethod slot_7 = {"set_run_mode_1_5", 0, 0 };
    static const QUMethod slot_8 = {"set_run_mode_1_7", 0, 0 };
    static const QUMethod slot_9 = {"set_run_mode_1_8", 0, 0 };
    static const QUMethod slot_10 = {"set_run_mode_1_12", 0, 0 };
    static const QUMethod slot_11 = {"set_run_mode_1_13", 0, 0 };
    static const QUMethod slot_12 = {"set_run_mode_1_14", 0, 0 };
    static const QUMethod slot_13 = {"set_run_mode_1_18", 0, 0 };
    static const QUMethod slot_14 = {"set_run_mode_2", 0, 0 };
    static const QUMethod slot_15 = {"set_run_mode_2_1", 0, 0 };
    static const QUMethod slot_16 = {"set_run_mode_2_2", 0, 0 };
    static const QUMethod slot_17 = {"set_run_mode_2_3", 0, 0 };
    static const QUMethod slot_18 = {"set_run_mode_2_4", 0, 0 };
    static const QUMethod slot_19 = {"set_run_mode_2_5", 0, 0 };
    static const QUMethod slot_20 = {"set_run_mode_2_7", 0, 0 };
    static const QUMethod slot_21 = {"set_run_mode_2_8", 0, 0 };
    static const QUMethod slot_22 = {"set_run_mode_2_12", 0, 0 };
    static const QUMethod slot_23 = {"set_run_mode_2_13", 0, 0 };
    static const QUMethod slot_24 = {"set_run_mode_2_14", 0, 0 };
    static const QUMethod slot_25 = {"set_run_mode_2_18", 0, 0 };
    static const QUMethod slot_26 = {"set_run_mode_3", 0, 0 };
    static const QUMethod slot_27 = {"set_run_mode_3_1", 0, 0 };
    static const QUMethod slot_28 = {"set_run_mode_3_5", 0, 0 };
    static const QUMethod slot_29 = {"set_run_mode_3_9", 0, 0 };
    static const QUMethod slot_30 = {"set_run_mode_3_10", 0, 0 };
    static const QUMethod slot_31 = {"set_run_mode_3_14", 0, 0 };
    static const QUMethod slot_32 = {"set_run_mode_3_15", 0, 0 };
    static const QUMethod slot_33 = {"set_run_mode_3_16", 0, 0 };
    static const QUMethod slot_34 = {"set_run_mode_4", 0, 0 };
    static const QUMethod slot_35 = {"set_run_mode_4_1", 0, 0 };
    static const QUMethod slot_36 = {"set_run_mode_4_6", 0, 0 };
    static const QUMethod slot_37 = {"set_run_mode_4_7", 0, 0 };
    static const QUMethod slot_38 = {"set_run_mode_4_8", 0, 0 };
    static const QUMethod slot_39 = {"set_run_mode_9", 0, 0 };
    static const QUParameter param_slot_40[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_40 = {"update_nfrec", 1, param_slot_40 };
    static const QUParameter param_slot_41[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_41 = {"update_ntimc", 1, param_slot_41 };
    static const QUParameter param_slot_42[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_42 = {"update_tmax", 1, param_slot_42 };
    static const QUParameter param_slot_43[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_43 = {"update_run_mode_3_5_iii", 1, param_slot_43 };
    static const QUParameter param_slot_44[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_44 = {"update_run_mode_3_5_jjj", 1, param_slot_44 };
    static const QUParameter param_slot_45[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_45 = {"update_run_mode_3_10_theta", 1, param_slot_45 };
    static const QUParameter param_slot_46[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_46 = {"update_run_mode_3_10_refractive_index", 1, param_slot_46 };
    static const QUParameter param_slot_47[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_47 = {"update_run_mode_3_10_lambda", 1, param_slot_47 };
    static const QUParameter param_slot_48[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_48 = {"update_run_mode_3_14_iii", 1, param_slot_48 };
    static const QUParameter param_slot_49[] = {
	{ "str", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_49 = {"update_run_mode_3_14_jjj", 1, param_slot_49 };
    static const QUMethod slot_50 = {"cancel", 0, 0 };
    static const QUMethod slot_51 = {"help", 0, 0 };
    static const QUParameter param_slot_52[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_52 = {"closeEvent", 1, param_slot_52 };
    static const QMetaData slot_tbl[] = {
	{ "set_run_anaflex()", &slot_0, QMetaData::Private },
	{ "set_instprofiles()", &slot_1, QMetaData::Private },
	{ "set_run_mode_1()", &slot_2, QMetaData::Private },
	{ "set_run_mode_1_1()", &slot_3, QMetaData::Private },
	{ "set_run_mode_1_2()", &slot_4, QMetaData::Private },
	{ "set_run_mode_1_3()", &slot_5, QMetaData::Private },
	{ "set_run_mode_1_4()", &slot_6, QMetaData::Private },
	{ "set_run_mode_1_5()", &slot_7, QMetaData::Private },
	{ "set_run_mode_1_7()", &slot_8, QMetaData::Private },
	{ "set_run_mode_1_8()", &slot_9, QMetaData::Private },
	{ "set_run_mode_1_12()", &slot_10, QMetaData::Private },
	{ "set_run_mode_1_13()", &slot_11, QMetaData::Private },
	{ "set_run_mode_1_14()", &slot_12, QMetaData::Private },
	{ "set_run_mode_1_18()", &slot_13, QMetaData::Private },
	{ "set_run_mode_2()", &slot_14, QMetaData::Private },
	{ "set_run_mode_2_1()", &slot_15, QMetaData::Private },
	{ "set_run_mode_2_2()", &slot_16, QMetaData::Private },
	{ "set_run_mode_2_3()", &slot_17, QMetaData::Private },
	{ "set_run_mode_2_4()", &slot_18, QMetaData::Private },
	{ "set_run_mode_2_5()", &slot_19, QMetaData::Private },
	{ "set_run_mode_2_7()", &slot_20, QMetaData::Private },
	{ "set_run_mode_2_8()", &slot_21, QMetaData::Private },
	{ "set_run_mode_2_12()", &slot_22, QMetaData::Private },
	{ "set_run_mode_2_13()", &slot_23, QMetaData::Private },
	{ "set_run_mode_2_14()", &slot_24, QMetaData::Private },
	{ "set_run_mode_2_18()", &slot_25, QMetaData::Private },
	{ "set_run_mode_3()", &slot_26, QMetaData::Private },
	{ "set_run_mode_3_1()", &slot_27, QMetaData::Private },
	{ "set_run_mode_3_5()", &slot_28, QMetaData::Private },
	{ "set_run_mode_3_9()", &slot_29, QMetaData::Private },
	{ "set_run_mode_3_10()", &slot_30, QMetaData::Private },
	{ "set_run_mode_3_14()", &slot_31, QMetaData::Private },
	{ "set_run_mode_3_15()", &slot_32, QMetaData::Private },
	{ "set_run_mode_3_16()", &slot_33, QMetaData::Private },
	{ "set_run_mode_4()", &slot_34, QMetaData::Private },
	{ "set_run_mode_4_1()", &slot_35, QMetaData::Private },
	{ "set_run_mode_4_6()", &slot_36, QMetaData::Private },
	{ "set_run_mode_4_7()", &slot_37, QMetaData::Private },
	{ "set_run_mode_4_8()", &slot_38, QMetaData::Private },
	{ "set_run_mode_9()", &slot_39, QMetaData::Private },
	{ "update_nfrec(const QString&)", &slot_40, QMetaData::Private },
	{ "update_ntimc(const QString&)", &slot_41, QMetaData::Private },
	{ "update_tmax(const QString&)", &slot_42, QMetaData::Private },
	{ "update_run_mode_3_5_iii(const QString&)", &slot_43, QMetaData::Private },
	{ "update_run_mode_3_5_jjj(const QString&)", &slot_44, QMetaData::Private },
	{ "update_run_mode_3_10_theta(const QString&)", &slot_45, QMetaData::Private },
	{ "update_run_mode_3_10_refractive_index(const QString&)", &slot_46, QMetaData::Private },
	{ "update_run_mode_3_10_lambda(const QString&)", &slot_47, QMetaData::Private },
	{ "update_run_mode_3_14_iii(const QString&)", &slot_48, QMetaData::Private },
	{ "update_run_mode_3_14_jjj(const QString&)", &slot_49, QMetaData::Private },
	{ "cancel()", &slot_50, QMetaData::Private },
	{ "help()", &slot_51, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_52, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Anaflex_Options", parentObject,
	slot_tbl, 53,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Anaflex_Options.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Anaflex_Options::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Anaflex_Options" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Anaflex_Options::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: set_run_anaflex(); break;
    case 1: set_instprofiles(); break;
    case 2: set_run_mode_1(); break;
    case 3: set_run_mode_1_1(); break;
    case 4: set_run_mode_1_2(); break;
    case 5: set_run_mode_1_3(); break;
    case 6: set_run_mode_1_4(); break;
    case 7: set_run_mode_1_5(); break;
    case 8: set_run_mode_1_7(); break;
    case 9: set_run_mode_1_8(); break;
    case 10: set_run_mode_1_12(); break;
    case 11: set_run_mode_1_13(); break;
    case 12: set_run_mode_1_14(); break;
    case 13: set_run_mode_1_18(); break;
    case 14: set_run_mode_2(); break;
    case 15: set_run_mode_2_1(); break;
    case 16: set_run_mode_2_2(); break;
    case 17: set_run_mode_2_3(); break;
    case 18: set_run_mode_2_4(); break;
    case 19: set_run_mode_2_5(); break;
    case 20: set_run_mode_2_7(); break;
    case 21: set_run_mode_2_8(); break;
    case 22: set_run_mode_2_12(); break;
    case 23: set_run_mode_2_13(); break;
    case 24: set_run_mode_2_14(); break;
    case 25: set_run_mode_2_18(); break;
    case 26: set_run_mode_3(); break;
    case 27: set_run_mode_3_1(); break;
    case 28: set_run_mode_3_5(); break;
    case 29: set_run_mode_3_9(); break;
    case 30: set_run_mode_3_10(); break;
    case 31: set_run_mode_3_14(); break;
    case 32: set_run_mode_3_15(); break;
    case 33: set_run_mode_3_16(); break;
    case 34: set_run_mode_4(); break;
    case 35: set_run_mode_4_1(); break;
    case 36: set_run_mode_4_6(); break;
    case 37: set_run_mode_4_7(); break;
    case 38: set_run_mode_4_8(); break;
    case 39: set_run_mode_9(); break;
    case 40: update_nfrec((const QString&)static_QUType_QString.get(_o+1)); break;
    case 41: update_ntimc((const QString&)static_QUType_QString.get(_o+1)); break;
    case 42: update_tmax((const QString&)static_QUType_QString.get(_o+1)); break;
    case 43: update_run_mode_3_5_iii((const QString&)static_QUType_QString.get(_o+1)); break;
    case 44: update_run_mode_3_5_jjj((const QString&)static_QUType_QString.get(_o+1)); break;
    case 45: update_run_mode_3_10_theta((const QString&)static_QUType_QString.get(_o+1)); break;
    case 46: update_run_mode_3_10_refractive_index((const QString&)static_QUType_QString.get(_o+1)); break;
    case 47: update_run_mode_3_10_lambda((const QString&)static_QUType_QString.get(_o+1)); break;
    case 48: update_run_mode_3_14_iii((const QString&)static_QUType_QString.get(_o+1)); break;
    case 49: update_run_mode_3_14_jjj((const QString&)static_QUType_QString.get(_o+1)); break;
    case 50: cancel(); break;
    case 51: help(); break;
    case 52: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Anaflex_Options::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Anaflex_Options::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Anaflex_Options::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
