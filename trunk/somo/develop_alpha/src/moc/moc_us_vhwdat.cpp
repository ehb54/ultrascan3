/****************************************************************************
** vhw_dat_W meta object code from reading C++ file 'us_vhwdat.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_vhwdat.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QMouseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *vhw_dat_W::className() const
{
    return "vhw_dat_W";
}

QMetaObject *vhw_dat_W::metaObj = 0;
static QMetaObjectCleanUp cleanUp_vhw_dat_W( "vhw_dat_W", &vhw_dat_W::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString vhw_dat_W::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "vhw_dat_W", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString vhw_dat_W::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "vhw_dat_W", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* vhw_dat_W::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Data_Control_W::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_1 = {"write_vhw", 0, 0 };
    static const QUMethod slot_2 = {"write_dis", 0, 0 };
    static const QUMethod slot_3 = {"write_res", 0, 0 };
    static const QUMethod slot_4 = {"select_groups", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_divisions", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_6 = {"getAnalysisPlotMouseReleased", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_7 = {"getAnalysisPlotMousePressed", 1, param_slot_7 };
    static const QUMethod slot_8 = {"cleanup", 0, 0 };
    static const QUMethod slot_9 = {"save_model", 0, 0 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_10 = {"plot_analysis", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_11 = {"setups", 1, param_slot_11 };
    static const QUMethod slot_12 = {"view", 0, 0 };
    static const QUMethod slot_13 = {"help", 0, 0 };
    static const QUMethod slot_14 = {"save", 0, 0 };
    static const QUMethod slot_15 = {"second_plot", 0, 0 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_16 = {"excludeSingleSignal", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_17 = {"excludeRangeSignal", 1, param_slot_17 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Private },
	{ "write_vhw()", &slot_1, QMetaData::Private },
	{ "write_dis()", &slot_2, QMetaData::Private },
	{ "write_res()", &slot_3, QMetaData::Private },
	{ "select_groups()", &slot_4, QMetaData::Private },
	{ "update_divisions(double)", &slot_5, QMetaData::Private },
	{ "getAnalysisPlotMouseReleased(const QMouseEvent&)", &slot_6, QMetaData::Private },
	{ "getAnalysisPlotMousePressed(const QMouseEvent&)", &slot_7, QMetaData::Private },
	{ "cleanup()", &slot_8, QMetaData::Private },
	{ "save_model()", &slot_9, QMetaData::Private },
	{ "plot_analysis()", &slot_10, QMetaData::Private },
	{ "setups()", &slot_11, QMetaData::Private },
	{ "view()", &slot_12, QMetaData::Private },
	{ "help()", &slot_13, QMetaData::Private },
	{ "save()", &slot_14, QMetaData::Private },
	{ "second_plot()", &slot_15, QMetaData::Private },
	{ "excludeSingleSignal(unsigned int)", &slot_16, QMetaData::Private },
	{ "excludeRangeSignal(unsigned int)", &slot_17, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"vhw_dat_W", parentObject,
	slot_tbl, 18,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_vhw_dat_W.setMetaObject( metaObj );
    return metaObj;
}

void* vhw_dat_W::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "vhw_dat_W" ) )
	return this;
    return Data_Control_W::qt_cast( clname );
}

bool vhw_dat_W::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: write_vhw(); break;
    case 2: write_dis(); break;
    case 3: write_res(); break;
    case 4: select_groups(); break;
    case 5: update_divisions((double)static_QUType_double.get(_o+1)); break;
    case 6: getAnalysisPlotMouseReleased((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 7: getAnalysisPlotMousePressed((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 8: cleanup(); break;
    case 9: save_model(); break;
    case 10: static_QUType_int.set(_o,plot_analysis()); break;
    case 11: static_QUType_int.set(_o,setups()); break;
    case 12: view(); break;
    case 13: help(); break;
    case 14: save(); break;
    case 15: second_plot(); break;
    case 16: excludeSingleSignal((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    case 17: excludeRangeSignal((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    default:
	return Data_Control_W::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool vhw_dat_W::qt_emit( int _id, QUObject* _o )
{
    return Data_Control_W::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool vhw_dat_W::qt_property( int id, int f, QVariant* v)
{
    return Data_Control_W::qt_property( id, f, v);
}

bool vhw_dat_W::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
