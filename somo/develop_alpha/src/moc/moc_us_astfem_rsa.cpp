/****************************************************************************
** US_Astfem_RSA meta object code from reading C++ file 'us_astfem_rsa.h'
**
** Created: Wed Dec 4 19:14:43 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_astfem_rsa.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Astfem_RSA::className() const
{
    return "US_Astfem_RSA";
}

QMetaObject *US_Astfem_RSA::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Astfem_RSA( "US_Astfem_RSA", &US_Astfem_RSA::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Astfem_RSA::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Astfem_RSA", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Astfem_RSA::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Astfem_RSA", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Astfem_RSA::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QObject::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In },
	{ 0, &static_QUType_ptr, "struct SimulationParameters", QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<mfem_data>", QUParameter::In }
    };
    static const QUMethod slot_0 = {"calculate", 4, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In },
	{ 0, &static_QUType_ptr, "struct SimulationParameters", QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<mfem_data>", QUParameter::In },
	{ "progress", &static_QUType_varptr, "\x10", QUParameter::In }
    };
    static const QUMethod slot_1 = {"calculate", 5, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In },
	{ 0, &static_QUType_ptr, "struct SimulationParameters", QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<mfem_data>", QUParameter::In },
	{ "progress", &static_QUType_varptr, "\x10", QUParameter::In },
	{ "thread", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"calculate", 6, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct ModelSystem", QUParameter::In },
	{ 0, &static_QUType_ptr, "struct SimulationParameters", QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<mfem_data>", QUParameter::In },
	{ "progress", &static_QUType_varptr, "\x10", QUParameter::In },
	{ "thread", &static_QUType_int, 0, QUParameter::In },
	{ "rotor_list", &static_QUType_ptr, "vector<rotorInfo>", QUParameter::In }
    };
    static const QUMethod slot_3 = {"calculate", 7, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_double, 0, QUParameter::In },
	{ 0, &static_QUType_double, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "mfem_initial", QUParameter::In },
	{ 0, &static_QUType_ptr, "mfem_data", QUParameter::In },
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"calculate_ni", 6, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_double, 0, QUParameter::In },
	{ 0, &static_QUType_double, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "mfem_initial", QUParameter::In },
	{ 0, &static_QUType_ptr, "mfem_data", QUParameter::In },
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"calculate_ra2", 6, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"setTimeCorrection", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"setTimeInterpolation", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"setMovie", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"setStopFlag", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ 0, &static_QUType_ptr, "struct mfem_initial", QUParameter::In },
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"initialize_conc", 3, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_ptr, "vector<double>", QUParameter::In },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_11 = {"mesh_gen", 2, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_ptr, "vector<double>", QUParameter::In }
    };
    static const QUMethod slot_12 = {"mesh_gen_s_pos", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_ptr, "vector<double>", QUParameter::In }
    };
    static const QUMethod slot_13 = {"mesh_gen_s_neg", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"mesh_gen_RefL", 2, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_ptr, "vector<double>", QUParameter::In },
	{ 0, &static_QUType_ptr, "double*", QUParameter::InOut },
	{ 0, &static_QUType_ptr, "double*", QUParameter::InOut },
	{ 0, &static_QUType_double, 0, QUParameter::In },
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"GlobalStiff", 5, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_ptr, "vector<double>", QUParameter::In },
	{ 0, &static_QUType_ptr, "double*", QUParameter::InOut },
	{ 0, &static_QUType_ptr, "double*", QUParameter::InOut },
	{ 0, &static_QUType_double, 0, QUParameter::In },
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"GlobalStiff_ellam", 5, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In },
	{ 0, &static_QUType_double, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "double*", QUParameter::InOut },
	{ 0, &static_QUType_ptr, "double*", QUParameter::InOut }
    };
    static const QUMethod slot_17 = {"ComputeCoefMatrixFixedMesh", 4, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In },
	{ 0, &static_QUType_double, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "double*", QUParameter::InOut },
	{ 0, &static_QUType_ptr, "double*", QUParameter::InOut }
    };
    static const QUMethod slot_18 = {"ComputeCoefMatrixMovingMeshR", 4, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In },
	{ 0, &static_QUType_double, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "double*", QUParameter::InOut },
	{ 0, &static_QUType_ptr, "double*", QUParameter::InOut }
    };
    static const QUMethod slot_19 = {"ComputeCoefMatrixMovingMeshL", 4, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ 0, &static_QUType_ptr, "double*", QUParameter::InOut },
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"ReactionOneStep_Euler_imp", 3, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In }
    };
    static const QUMethod slot_21 = {"Reaction_dydt", 2, param_slot_21 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In },
	{ 0, &static_QUType_ptr, "double*", QUParameter::InOut }
    };
    static const QUMethod slot_22 = {"Reaction_dfdy", 2, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_23 = {"adjust_limits", 1, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<double>", QUParameter::In }
    };
    static const QUMethod slot_24 = {"adjust_grid", 3, param_slot_24 };
    static const QUMethod slot_25 = {"print_af", 0, 0 };
    static const QUMethod slot_26 = {"print_rg", 0, 0 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_ptr, "FILE", QUParameter::In }
    };
    static const QUMethod slot_27 = {"print_af", 1, param_slot_27 };
    static const QUMethod slot_28 = {"print_simparams", 0, 0 };
    static const QUParameter param_slot_29[] = {
	{ 0, &static_QUType_ptr, "vector<double>", QUParameter::In }
    };
    static const QUMethod slot_29 = {"print_vector", 1, param_slot_29 };
    static const QUParameter param_slot_30[] = {
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_30 = {"print_vector", 2, param_slot_30 };
    static const QUMethod slot_31 = {"initialize_rg", 0, 0 };
    static const QUMethod slot_32 = {"update_assocv", 0, 0 };
    static const QUParameter param_slot_33[] = {
	{ 0, &static_QUType_ptr, "struct mfem_initial", QUParameter::In }
    };
    static const QUMethod slot_33 = {"decompose", 1, param_slot_33 };
    static const QMetaData slot_tbl[] = {
	{ "calculate(struct ModelSystem*,struct SimulationParameters*,vector<mfem_data>*)", &slot_0, QMetaData::Public },
	{ "calculate(struct ModelSystem*,struct SimulationParameters*,vector<mfem_data>*,int*)", &slot_1, QMetaData::Public },
	{ "calculate(struct ModelSystem*,struct SimulationParameters*,vector<mfem_data>*,int*,int)", &slot_2, QMetaData::Public },
	{ "calculate(struct ModelSystem*,struct SimulationParameters*,vector<mfem_data>*,int*,int,vector<rotorInfo>*)", &slot_3, QMetaData::Public },
	{ "calculate_ni(double,double,mfem_initial*,mfem_data*,bool)", &slot_4, QMetaData::Public },
	{ "calculate_ra2(double,double,mfem_initial*,mfem_data*,bool)", &slot_5, QMetaData::Public },
	{ "setTimeCorrection(bool)", &slot_6, QMetaData::Public },
	{ "setTimeInterpolation(bool)", &slot_7, QMetaData::Public },
	{ "setMovie(bool)", &slot_8, QMetaData::Public },
	{ "setStopFlag(bool)", &slot_9, QMetaData::Public },
	{ "initialize_conc(unsigned int,struct mfem_initial*,bool)", &slot_10, QMetaData::Private },
	{ "mesh_gen(vector<double>,unsigned int)", &slot_11, QMetaData::Private },
	{ "mesh_gen_s_pos(vector<double>)", &slot_12, QMetaData::Private },
	{ "mesh_gen_s_neg(vector<double>)", &slot_13, QMetaData::Private },
	{ "mesh_gen_RefL(int,int)", &slot_14, QMetaData::Private },
	{ "GlobalStiff(vector<double>*,double**,double**,double,double)", &slot_15, QMetaData::Private },
	{ "GlobalStiff_ellam(vector<double>*,double**,double**,double,double)", &slot_16, QMetaData::Private },
	{ "ComputeCoefMatrixFixedMesh(double,double,double**,double**)", &slot_17, QMetaData::Private },
	{ "ComputeCoefMatrixMovingMeshR(double,double,double**,double**)", &slot_18, QMetaData::Private },
	{ "ComputeCoefMatrixMovingMeshL(double,double,double**,double**)", &slot_19, QMetaData::Private },
	{ "ReactionOneStep_Euler_imp(unsigned int,double**,double)", &slot_20, QMetaData::Private },
	{ "Reaction_dydt(double*,double*)", &slot_21, QMetaData::Private },
	{ "Reaction_dfdy(double*,double**)", &slot_22, QMetaData::Private },
	{ "adjust_limits(unsigned int)", &slot_23, QMetaData::Private },
	{ "adjust_grid(unsigned int,unsigned int,vector<double>*)", &slot_24, QMetaData::Private },
	{ "print_af()", &slot_25, QMetaData::Private },
	{ "print_rg()", &slot_26, QMetaData::Private },
	{ "print_af(FILE*)", &slot_27, QMetaData::Private },
	{ "print_simparams()", &slot_28, QMetaData::Private },
	{ "print_vector(vector<double>*)", &slot_29, QMetaData::Private },
	{ "print_vector(double*,unsigned int)", &slot_30, QMetaData::Private },
	{ "initialize_rg()", &slot_31, QMetaData::Private },
	{ "update_assocv()", &slot_32, QMetaData::Private },
	{ "decompose(struct mfem_initial*)", &slot_33, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_ptr, "vector<double>", QUParameter::In },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In }
    };
    static const QUMethod signal_0 = {"new_scan", 2, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_1 = {"new_time", 1, param_signal_1 };
    static const QUParameter param_signal_2[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_2 = {"current_component", 1, param_signal_2 };
    static const QUParameter param_signal_3[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod signal_3 = {"current_speed", 1, param_signal_3 };
    static const QMetaData signal_tbl[] = {
	{ "new_scan(vector<double>*,double*)", &signal_0, QMetaData::Public },
	{ "new_time(float)", &signal_1, QMetaData::Public },
	{ "current_component(int)", &signal_2, QMetaData::Public },
	{ "current_speed(unsigned int)", &signal_3, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Astfem_RSA", parentObject,
	slot_tbl, 34,
	signal_tbl, 4,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Astfem_RSA.setMetaObject( metaObj );
    return metaObj;
}

void* US_Astfem_RSA::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Astfem_RSA" ) )
	return this;
    return QObject::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL new_scan
void US_Astfem_RSA::new_scan( vector<double>* t0, double* t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_ptr.set(o+1,t0);
    static_QUType_varptr.set(o+2,t1);
    activate_signal( clist, o );
}

// SIGNAL new_time
void US_Astfem_RSA::new_time( float t0 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 1 );
    if ( !clist )
	return;
    QUObject o[2];
    static_QUType_ptr.set(o+1,&t0);
    activate_signal( clist, o );
}

// SIGNAL current_component
void US_Astfem_RSA::current_component( int t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 2, t0 );
}

// SIGNAL current_speed
void US_Astfem_RSA::current_speed( unsigned int t0 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 3 );
    if ( !clist )
	return;
    QUObject o[2];
    static_QUType_ptr.set(o+1,&t0);
    activate_signal( clist, o );
}

bool US_Astfem_RSA::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: static_QUType_int.set(_o,calculate((struct ModelSystem*)static_QUType_ptr.get(_o+1),(struct SimulationParameters*)static_QUType_ptr.get(_o+2),(vector<mfem_data>*)static_QUType_ptr.get(_o+3))); break;
    case 1: static_QUType_int.set(_o,calculate((struct ModelSystem*)static_QUType_ptr.get(_o+1),(struct SimulationParameters*)static_QUType_ptr.get(_o+2),(vector<mfem_data>*)static_QUType_ptr.get(_o+3),(int*)static_QUType_varptr.get(_o+4))); break;
    case 2: static_QUType_int.set(_o,calculate((struct ModelSystem*)static_QUType_ptr.get(_o+1),(struct SimulationParameters*)static_QUType_ptr.get(_o+2),(vector<mfem_data>*)static_QUType_ptr.get(_o+3),(int*)static_QUType_varptr.get(_o+4),(int)static_QUType_int.get(_o+5))); break;
    case 3: static_QUType_int.set(_o,calculate((struct ModelSystem*)static_QUType_ptr.get(_o+1),(struct SimulationParameters*)static_QUType_ptr.get(_o+2),(vector<mfem_data>*)static_QUType_ptr.get(_o+3),(int*)static_QUType_varptr.get(_o+4),(int)static_QUType_int.get(_o+5),(vector<rotorInfo>*)static_QUType_ptr.get(_o+6))); break;
    case 4: static_QUType_int.set(_o,calculate_ni((double)static_QUType_double.get(_o+1),(double)static_QUType_double.get(_o+2),(mfem_initial*)static_QUType_ptr.get(_o+3),(mfem_data*)static_QUType_ptr.get(_o+4),(bool)static_QUType_bool.get(_o+5))); break;
    case 5: static_QUType_int.set(_o,calculate_ra2((double)static_QUType_double.get(_o+1),(double)static_QUType_double.get(_o+2),(mfem_initial*)static_QUType_ptr.get(_o+3),(mfem_data*)static_QUType_ptr.get(_o+4),(bool)static_QUType_bool.get(_o+5))); break;
    case 6: setTimeCorrection((bool)static_QUType_bool.get(_o+1)); break;
    case 7: setTimeInterpolation((bool)static_QUType_bool.get(_o+1)); break;
    case 8: setMovie((bool)static_QUType_bool.get(_o+1)); break;
    case 9: setStopFlag((bool)static_QUType_bool.get(_o+1)); break;
    case 10: initialize_conc((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1))),(struct mfem_initial*)static_QUType_ptr.get(_o+2),(bool)static_QUType_bool.get(_o+3)); break;
    case 11: mesh_gen((vector<double>)(*((vector<double>*)static_QUType_ptr.get(_o+1))),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+2)))); break;
    case 12: mesh_gen_s_pos((vector<double>)(*((vector<double>*)static_QUType_ptr.get(_o+1)))); break;
    case 13: mesh_gen_s_neg((vector<double>)(*((vector<double>*)static_QUType_ptr.get(_o+1)))); break;
    case 14: mesh_gen_RefL((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 15: GlobalStiff((vector<double>*)static_QUType_ptr.get(_o+1),(double**)static_QUType_ptr.get(_o+2),(double**)static_QUType_ptr.get(_o+3),(double)static_QUType_double.get(_o+4),(double)static_QUType_double.get(_o+5)); break;
    case 16: GlobalStiff_ellam((vector<double>*)static_QUType_ptr.get(_o+1),(double**)static_QUType_ptr.get(_o+2),(double**)static_QUType_ptr.get(_o+3),(double)static_QUType_double.get(_o+4),(double)static_QUType_double.get(_o+5)); break;
    case 17: ComputeCoefMatrixFixedMesh((double)static_QUType_double.get(_o+1),(double)static_QUType_double.get(_o+2),(double**)static_QUType_ptr.get(_o+3),(double**)static_QUType_ptr.get(_o+4)); break;
    case 18: ComputeCoefMatrixMovingMeshR((double)static_QUType_double.get(_o+1),(double)static_QUType_double.get(_o+2),(double**)static_QUType_ptr.get(_o+3),(double**)static_QUType_ptr.get(_o+4)); break;
    case 19: ComputeCoefMatrixMovingMeshL((double)static_QUType_double.get(_o+1),(double)static_QUType_double.get(_o+2),(double**)static_QUType_ptr.get(_o+3),(double**)static_QUType_ptr.get(_o+4)); break;
    case 20: ReactionOneStep_Euler_imp((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1))),(double**)static_QUType_ptr.get(_o+2),(double)static_QUType_double.get(_o+3)); break;
    case 21: Reaction_dydt((double*)static_QUType_varptr.get(_o+1),(double*)static_QUType_varptr.get(_o+2)); break;
    case 22: Reaction_dfdy((double*)static_QUType_varptr.get(_o+1),(double**)static_QUType_ptr.get(_o+2)); break;
    case 23: adjust_limits((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    case 24: adjust_grid((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1))),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+2))),(vector<double>*)static_QUType_ptr.get(_o+3)); break;
    case 25: print_af(); break;
    case 26: print_rg(); break;
    case 27: print_af((FILE*)static_QUType_ptr.get(_o+1)); break;
    case 28: print_simparams(); break;
    case 29: print_vector((vector<double>*)static_QUType_ptr.get(_o+1)); break;
    case 30: print_vector((double*)static_QUType_varptr.get(_o+1),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+2)))); break;
    case 31: initialize_rg(); break;
    case 32: update_assocv(); break;
    case 33: decompose((struct mfem_initial*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QObject::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Astfem_RSA::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: new_scan((vector<double>*)static_QUType_ptr.get(_o+1),(double*)static_QUType_varptr.get(_o+2)); break;
    case 1: new_time((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 2: current_component((int)static_QUType_int.get(_o+1)); break;
    case 3: current_speed((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    default:
	return QObject::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_Astfem_RSA::qt_property( int id, int f, QVariant* v)
{
    return QObject::qt_property( id, f, v);
}

bool US_Astfem_RSA::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
