/****************************************************************************
** US_FemGlobal meta object code from reading C++ file 'us_femglobal.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_femglobal.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_FemGlobal::className() const
{
    return "US_FemGlobal";
}

QMetaObject *US_FemGlobal::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_FemGlobal( "US_FemGlobal", &US_FemGlobal::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_FemGlobal::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_FemGlobal", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_FemGlobal::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_FemGlobal", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_FemGlobal::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QObject::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In },
	{ 0, &static_QUType_ptr, "struct SimulationParameters", QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"read_experiment", 4, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "vector<struct ModelSystem>", QUParameter::In },
	{ 0, &static_QUType_ptr, "struct SimulationParameters", QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"read_experiment", 4, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In },
	{ 0, &static_QUType_ptr, "struct SimulationParameters", QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"write_experiment", 4, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct SimulationParameters", QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"read_simulationParameters", 3, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct SimulationParameters", QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<QString>", QUParameter::In }
    };
    static const QUMethod slot_4 = {"read_simulationParameters", 3, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct SimulationParameters", QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"write_simulationParameters", 3, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"read_modelSystem", 3, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ "flag", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"read_modelSystem", 4, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "vector<ModelSystem>", QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"read_modelSystem", 3, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<QString>", QUParameter::In }
    };
    static const QUMethod slot_9 = {"read_modelSystem", 3, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<QString>", QUParameter::In },
	{ "flag", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"read_modelSystem", 4, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<QString>", QUParameter::In },
	{ "flag", &static_QUType_bool, 0, QUParameter::In },
	{ "offset", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"read_modelSystem", 5, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"write_modelSystem", 3, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ "flag", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"write_modelSystem", 4, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In },
	{ 0, &static_QUType_ptr, "struct ModelSystemConstraints", QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"read_constraints", 4, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In },
	{ 0, &static_QUType_ptr, "struct ModelSystemConstraints", QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<QString>", QUParameter::In }
    };
    static const QUMethod slot_15 = {"read_constraints", 4, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In },
	{ 0, &static_QUType_ptr, "struct ModelSystemConstraints", QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"write_constraints", 4, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "vector<mfem_data>", QUParameter::In },
	{ "filename", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"read_model_data", 3, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "vector<mfem_data>", QUParameter::In },
	{ "filename", &static_QUType_QString, 0, QUParameter::In },
	{ "ignore_errors", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"read_model_data", 4, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "vector<mfem_data>", QUParameter::In },
	{ "filename", &static_QUType_QString, 0, QUParameter::In },
	{ "ignore_errors", &static_QUType_bool, 0, QUParameter::In },
	{ "ds", &static_QUType_ptr, "QDataStream", QUParameter::In }
    };
    static const QUMethod slot_19 = {"read_model_data", 5, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "vector<mfem_data>", QUParameter::In },
	{ "filename", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"write_model_data", 3, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "vector<mfem_data>", QUParameter::In },
	{ "filename", &static_QUType_QString, 0, QUParameter::In },
	{ "ds", &static_QUType_ptr, "QDataStream", QUParameter::In }
    };
    static const QUMethod slot_21 = {"write_model_data", 4, param_slot_21 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "infile", &static_QUType_QString, 0, QUParameter::In },
	{ "outfile", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"convert_analysis_data", 3, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "infile", &static_QUType_QString, 0, QUParameter::In },
	{ "qsl", &static_QUType_varptr, "\x04", QUParameter::In }
    };
    static const QUMethod slot_23 = {"convert_analysis_data", 3, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "vector<mfem_data>", QUParameter::In },
	{ "filename", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_24 = {"write_ascii_model_data", 3, param_slot_24 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ "accumulated_model", &static_QUType_ptr, "vector<mfem_data>", QUParameter::In },
	{ "source_model", &static_QUType_ptr, "vector<mfem_data>", QUParameter::In },
	{ "monte_carlo_iterations", &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_25 = {"accumulate_model_monte_carlo_data", 4, param_slot_25 };
    static const QUParameter param_slot_26[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "vector<mfem_data>", QUParameter::In },
	{ "filenamebase", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_26 = {"read_mwl_model_data", 3, param_slot_26 };
    static const QMetaData slot_tbl[] = {
	{ "read_experiment(struct ModelSystem*,struct SimulationParameters*,QString)", &slot_0, QMetaData::Public },
	{ "read_experiment(vector<struct ModelSystem>*,struct SimulationParameters*,QString)", &slot_1, QMetaData::Public },
	{ "write_experiment(struct ModelSystem*,struct SimulationParameters*,QString)", &slot_2, QMetaData::Public },
	{ "read_simulationParameters(struct SimulationParameters*,QString)", &slot_3, QMetaData::Public },
	{ "read_simulationParameters(struct SimulationParameters*,vector<QString>)", &slot_4, QMetaData::Public },
	{ "write_simulationParameters(struct SimulationParameters*,QString)", &slot_5, QMetaData::Public },
	{ "read_modelSystem(struct ModelSystem*,QString)", &slot_6, QMetaData::Public },
	{ "read_modelSystem(struct ModelSystem*,QString,bool)", &slot_7, QMetaData::Public },
	{ "read_modelSystem(vector<ModelSystem>*,QString)", &slot_8, QMetaData::Public },
	{ "read_modelSystem(struct ModelSystem*,vector<QString>)", &slot_9, QMetaData::Public },
	{ "read_modelSystem(struct ModelSystem*,vector<QString>,bool)", &slot_10, QMetaData::Public },
	{ "read_modelSystem(struct ModelSystem*,vector<QString>,bool,int)", &slot_11, QMetaData::Public },
	{ "write_modelSystem(struct ModelSystem*,QString)", &slot_12, QMetaData::Public },
	{ "write_modelSystem(struct ModelSystem*,QString,bool)", &slot_13, QMetaData::Public },
	{ "read_constraints(struct ModelSystem*,struct ModelSystemConstraints*,QString)", &slot_14, QMetaData::Public },
	{ "read_constraints(struct ModelSystem*,struct ModelSystemConstraints*,vector<QString>)", &slot_15, QMetaData::Public },
	{ "write_constraints(struct ModelSystem*,struct ModelSystemConstraints*,QString)", &slot_16, QMetaData::Public },
	{ "read_model_data(vector<mfem_data>*,QString)", &slot_17, QMetaData::Public },
	{ "read_model_data(vector<mfem_data>*,QString,bool)", &slot_18, QMetaData::Public },
	{ "read_model_data(vector<mfem_data>*,QString,bool,QDataStream*)", &slot_19, QMetaData::Public },
	{ "write_model_data(vector<mfem_data>*,QString)", &slot_20, QMetaData::Public },
	{ "write_model_data(vector<mfem_data>*,QString,QDataStream*)", &slot_21, QMetaData::Public },
	{ "convert_analysis_data(QString,QString)", &slot_22, QMetaData::Public },
	{ "convert_analysis_data(QString,QStringList*)", &slot_23, QMetaData::Public },
	{ "write_ascii_model_data(vector<mfem_data>*,QString)", &slot_24, QMetaData::Public },
	{ "accumulate_model_monte_carlo_data(vector<mfem_data>*,vector<mfem_data>*,unsigned int)", &slot_25, QMetaData::Public },
	{ "read_mwl_model_data(vector<mfem_data>*,QString)", &slot_26, QMetaData::Public }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"new_error", 1, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod signal_1 = {"simparams_name", 1, param_signal_1 };
    static const QUParameter param_signal_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod signal_2 = {"model_name", 1, param_signal_2 };
    static const QMetaData signal_tbl[] = {
	{ "new_error(QString)", &signal_0, QMetaData::Public },
	{ "simparams_name(QString)", &signal_1, QMetaData::Public },
	{ "model_name(QString)", &signal_2, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_FemGlobal", parentObject,
	slot_tbl, 27,
	signal_tbl, 3,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_FemGlobal.setMetaObject( metaObj );
    return metaObj;
}

void* US_FemGlobal::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_FemGlobal" ) )
	return this;
    return QObject::qt_cast( clname );
}

// SIGNAL new_error
void US_FemGlobal::new_error( QString t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 0, t0 );
}

// SIGNAL simparams_name
void US_FemGlobal::simparams_name( QString t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 1, t0 );
}

// SIGNAL model_name
void US_FemGlobal::model_name( QString t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 2, t0 );
}

bool US_FemGlobal::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: static_QUType_int.set(_o,read_experiment((struct ModelSystem*)static_QUType_ptr.get(_o+1),(struct SimulationParameters*)static_QUType_ptr.get(_o+2),(QString)static_QUType_QString.get(_o+3))); break;
    case 1: static_QUType_int.set(_o,read_experiment((vector<struct ModelSystem>*)static_QUType_ptr.get(_o+1),(struct SimulationParameters*)static_QUType_ptr.get(_o+2),(QString)static_QUType_QString.get(_o+3))); break;
    case 2: static_QUType_int.set(_o,write_experiment((struct ModelSystem*)static_QUType_ptr.get(_o+1),(struct SimulationParameters*)static_QUType_ptr.get(_o+2),(QString)static_QUType_QString.get(_o+3))); break;
    case 3: static_QUType_int.set(_o,read_simulationParameters((struct SimulationParameters*)static_QUType_ptr.get(_o+1),(QString)static_QUType_QString.get(_o+2))); break;
    case 4: static_QUType_int.set(_o,read_simulationParameters((struct SimulationParameters*)static_QUType_ptr.get(_o+1),(vector<QString>)(*((vector<QString>*)static_QUType_ptr.get(_o+2))))); break;
    case 5: static_QUType_int.set(_o,write_simulationParameters((struct SimulationParameters*)static_QUType_ptr.get(_o+1),(QString)static_QUType_QString.get(_o+2))); break;
    case 6: static_QUType_int.set(_o,read_modelSystem((struct ModelSystem*)static_QUType_ptr.get(_o+1),(QString)static_QUType_QString.get(_o+2))); break;
    case 7: static_QUType_int.set(_o,read_modelSystem((struct ModelSystem*)static_QUType_ptr.get(_o+1),(QString)static_QUType_QString.get(_o+2),(bool)static_QUType_bool.get(_o+3))); break;
    case 8: static_QUType_int.set(_o,read_modelSystem((vector<ModelSystem>*)static_QUType_ptr.get(_o+1),(QString)static_QUType_QString.get(_o+2))); break;
    case 9: static_QUType_int.set(_o,read_modelSystem((struct ModelSystem*)static_QUType_ptr.get(_o+1),(vector<QString>)(*((vector<QString>*)static_QUType_ptr.get(_o+2))))); break;
    case 10: static_QUType_int.set(_o,read_modelSystem((struct ModelSystem*)static_QUType_ptr.get(_o+1),(vector<QString>)(*((vector<QString>*)static_QUType_ptr.get(_o+2))),(bool)static_QUType_bool.get(_o+3))); break;
    case 11: static_QUType_int.set(_o,read_modelSystem((struct ModelSystem*)static_QUType_ptr.get(_o+1),(vector<QString>)(*((vector<QString>*)static_QUType_ptr.get(_o+2))),(bool)static_QUType_bool.get(_o+3),(int)static_QUType_int.get(_o+4))); break;
    case 12: static_QUType_int.set(_o,write_modelSystem((struct ModelSystem*)static_QUType_ptr.get(_o+1),(QString)static_QUType_QString.get(_o+2))); break;
    case 13: static_QUType_int.set(_o,write_modelSystem((struct ModelSystem*)static_QUType_ptr.get(_o+1),(QString)static_QUType_QString.get(_o+2),(bool)static_QUType_bool.get(_o+3))); break;
    case 14: static_QUType_int.set(_o,read_constraints((struct ModelSystem*)static_QUType_ptr.get(_o+1),(struct ModelSystemConstraints*)static_QUType_ptr.get(_o+2),(QString)static_QUType_QString.get(_o+3))); break;
    case 15: static_QUType_int.set(_o,read_constraints((struct ModelSystem*)static_QUType_ptr.get(_o+1),(struct ModelSystemConstraints*)static_QUType_ptr.get(_o+2),(vector<QString>)(*((vector<QString>*)static_QUType_ptr.get(_o+3))))); break;
    case 16: static_QUType_int.set(_o,write_constraints((struct ModelSystem*)static_QUType_ptr.get(_o+1),(struct ModelSystemConstraints*)static_QUType_ptr.get(_o+2),(QString)static_QUType_QString.get(_o+3))); break;
    case 17: static_QUType_int.set(_o,read_model_data((vector<mfem_data>*)static_QUType_ptr.get(_o+1),(QString)static_QUType_QString.get(_o+2))); break;
    case 18: static_QUType_int.set(_o,read_model_data((vector<mfem_data>*)static_QUType_ptr.get(_o+1),(QString)static_QUType_QString.get(_o+2),(bool)static_QUType_bool.get(_o+3))); break;
    case 19: static_QUType_int.set(_o,read_model_data((vector<mfem_data>*)static_QUType_ptr.get(_o+1),(QString)static_QUType_QString.get(_o+2),(bool)static_QUType_bool.get(_o+3),(QDataStream*)static_QUType_ptr.get(_o+4))); break;
    case 20: static_QUType_int.set(_o,write_model_data((vector<mfem_data>*)static_QUType_ptr.get(_o+1),(QString)static_QUType_QString.get(_o+2))); break;
    case 21: static_QUType_int.set(_o,write_model_data((vector<mfem_data>*)static_QUType_ptr.get(_o+1),(QString)static_QUType_QString.get(_o+2),(QDataStream*)static_QUType_ptr.get(_o+3))); break;
    case 22: static_QUType_int.set(_o,convert_analysis_data((QString)static_QUType_QString.get(_o+1),(QString)static_QUType_QString.get(_o+2))); break;
    case 23: static_QUType_int.set(_o,convert_analysis_data((QString)static_QUType_QString.get(_o+1),(QStringList*)static_QUType_varptr.get(_o+2))); break;
    case 24: static_QUType_int.set(_o,write_ascii_model_data((vector<mfem_data>*)static_QUType_ptr.get(_o+1),(QString)static_QUType_QString.get(_o+2))); break;
    case 25: static_QUType_int.set(_o,accumulate_model_monte_carlo_data((vector<mfem_data>*)static_QUType_ptr.get(_o+1),(vector<mfem_data>*)static_QUType_ptr.get(_o+2),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+3))))); break;
    case 26: static_QUType_int.set(_o,read_mwl_model_data((vector<mfem_data>*)static_QUType_ptr.get(_o+1),(QString)static_QUType_QString.get(_o+2))); break;
    default:
	return QObject::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_FemGlobal::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: new_error((QString)static_QUType_QString.get(_o+1)); break;
    case 1: simparams_name((QString)static_QUType_QString.get(_o+1)); break;
    case 2: model_name((QString)static_QUType_QString.get(_o+1)); break;
    default:
	return QObject::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_FemGlobal::qt_property( int id, int f, QVariant* v)
{
    return QObject::qt_property( id, f, v);
}

bool US_FemGlobal::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
