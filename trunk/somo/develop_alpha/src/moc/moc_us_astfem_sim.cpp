/****************************************************************************
** US_Astfem_Sim meta object code from reading C++ file 'us_astfem_sim.h'
**
** Created: Wed Dec 4 19:14:43 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_astfem_sim.h"
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

const char *US_Astfem_Sim::className() const
{
    return "US_Astfem_Sim";
}

QMetaObject *US_Astfem_Sim::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Astfem_Sim( "US_Astfem_Sim", &US_Astfem_Sim::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Astfem_Sim::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Astfem_Sim", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Astfem_Sim::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Astfem_Sim", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Astfem_Sim::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_0 = {"closeEvent", 1, param_slot_0 };
    static const QUMethod slot_1 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_2 = {"new_model", 0, 0 };
    static const QUMethod slot_3 = {"assign_model", 0, 0 };
    static const QUMethod slot_4 = {"load_model", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"load_model", 1, param_slot_5 };
    static const QUMethod slot_6 = {"load_system", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"load_system", 1, param_slot_7 };
    static const QUMethod slot_8 = {"save_system", 0, 0 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"save_system", 1, param_slot_9 };
    static const QUMethod slot_10 = {"change_model", 0, 0 };
    static const QUMethod slot_11 = {"simulation_parameters", 0, 0 };
    static const QUMethod slot_12 = {"start_simulation", 0, 0 };
    static const QUMethod slot_13 = {"stop_simulation", 0, 0 };
    static const QUMethod slot_14 = {"dcdt_window", 0, 0 };
    static const QUMethod slot_15 = {"save_scans", 0, 0 };
    static const QUMethod slot_16 = {"help", 0, 0 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_ptr, "vector<double>", QUParameter::In },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In }
    };
    static const QUMethod slot_17 = {"update_movie_plot", 2, param_slot_17 };
    static const QUMethod slot_18 = {"update_movieFlag", 0, 0 };
    static const QUMethod slot_19 = {"update_savemovie", 0, 0 };
    static const QUMethod slot_20 = {"update_time_correctionFlag", 0, 0 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_21 = {"update_time", 1, param_slot_21 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"update_progress", 1, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_23 = {"update_speed", 1, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_24 = {"printError", 1, param_slot_24 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"save_xla", 1, param_slot_25 };
    static const QUParameter param_slot_26[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_26 = {"save_ultrascan", 1, param_slot_26 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_27 = {"update_simparams_name", 1, param_slot_27 };
    static const QUParameter param_slot_28[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_28 = {"update_model_name", 1, param_slot_28 };
    static const QMetaData slot_tbl[] = {
	{ "closeEvent(QCloseEvent*)", &slot_0, QMetaData::Protected },
	{ "setup_GUI()", &slot_1, QMetaData::Private },
	{ "new_model()", &slot_2, QMetaData::Private },
	{ "assign_model()", &slot_3, QMetaData::Private },
	{ "load_model()", &slot_4, QMetaData::Private },
	{ "load_model(const QString&)", &slot_5, QMetaData::Private },
	{ "load_system()", &slot_6, QMetaData::Private },
	{ "load_system(const QString&)", &slot_7, QMetaData::Private },
	{ "save_system()", &slot_8, QMetaData::Private },
	{ "save_system(const QString&)", &slot_9, QMetaData::Private },
	{ "change_model()", &slot_10, QMetaData::Private },
	{ "simulation_parameters()", &slot_11, QMetaData::Private },
	{ "start_simulation()", &slot_12, QMetaData::Private },
	{ "stop_simulation()", &slot_13, QMetaData::Private },
	{ "dcdt_window()", &slot_14, QMetaData::Private },
	{ "save_scans()", &slot_15, QMetaData::Private },
	{ "help()", &slot_16, QMetaData::Private },
	{ "update_movie_plot(vector<double>*,double*)", &slot_17, QMetaData::Private },
	{ "update_movieFlag()", &slot_18, QMetaData::Private },
	{ "update_savemovie()", &slot_19, QMetaData::Private },
	{ "update_time_correctionFlag()", &slot_20, QMetaData::Private },
	{ "update_time(float)", &slot_21, QMetaData::Private },
	{ "update_progress(int)", &slot_22, QMetaData::Private },
	{ "update_speed(unsigned int)", &slot_23, QMetaData::Private },
	{ "printError(const int&)", &slot_24, QMetaData::Private },
	{ "save_xla(const QString&)", &slot_25, QMetaData::Private },
	{ "save_ultrascan(const QString&)", &slot_26, QMetaData::Private },
	{ "update_simparams_name(QString)", &slot_27, QMetaData::Private },
	{ "update_model_name(QString)", &slot_28, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Astfem_Sim", parentObject,
	slot_tbl, 29,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Astfem_Sim.setMetaObject( metaObj );
    return metaObj;
}

void* US_Astfem_Sim::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Astfem_Sim" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Astfem_Sim::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 1: setup_GUI(); break;
    case 2: new_model(); break;
    case 3: assign_model(); break;
    case 4: load_model(); break;
    case 5: load_model((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: load_system(); break;
    case 7: load_system((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: save_system(); break;
    case 9: save_system((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: change_model(); break;
    case 11: simulation_parameters(); break;
    case 12: start_simulation(); break;
    case 13: stop_simulation(); break;
    case 14: dcdt_window(); break;
    case 15: save_scans(); break;
    case 16: help(); break;
    case 17: update_movie_plot((vector<double>*)static_QUType_ptr.get(_o+1),(double*)static_QUType_varptr.get(_o+2)); break;
    case 18: update_movieFlag(); break;
    case 19: update_savemovie(); break;
    case 20: update_time_correctionFlag(); break;
    case 21: update_time((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 22: update_progress((int)static_QUType_int.get(_o+1)); break;
    case 23: update_speed((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    case 24: printError((const int&)static_QUType_int.get(_o+1)); break;
    case 25: save_xla((const QString&)static_QUType_QString.get(_o+1)); break;
    case 26: save_ultrascan((const QString&)static_QUType_QString.get(_o+1)); break;
    case 27: update_simparams_name((QString)static_QUType_QString.get(_o+1)); break;
    case 28: update_model_name((QString)static_QUType_QString.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Astfem_Sim::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Astfem_Sim::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Astfem_Sim::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
