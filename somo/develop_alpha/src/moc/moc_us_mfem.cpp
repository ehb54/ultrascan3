/****************************************************************************
** US_MovingFEM meta object code from reading C++ file 'us_mfem.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_mfem.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_MovingFEM::className() const
{
    return "US_MovingFEM";
}

QMetaObject *US_MovingFEM::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_MovingFEM( "US_MovingFEM", &US_MovingFEM::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_MovingFEM::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_MovingFEM", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_MovingFEM::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_MovingFEM", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_MovingFEM::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QObject::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "N", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"set_N", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"set_s", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"set_D", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"set_rpm", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"set_total_t", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"set_m", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"set_b", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"set_c0", 1, param_slot_7 };
    static const QUMethod slot_8 = {"stop", 0, 0 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_9 = {"get_N", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_double, 0, QUParameter::Out }
    };
    static const QUMethod slot_10 = {"get_s", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_double, 0, QUParameter::Out }
    };
    static const QUMethod slot_11 = {"get_D", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_double, 0, QUParameter::Out }
    };
    static const QUMethod slot_12 = {"get_rpm", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_double, 0, QUParameter::Out }
    };
    static const QUMethod slot_13 = {"get_total_t", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_double, 0, QUParameter::Out }
    };
    static const QUMethod slot_14 = {"get_m", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_double, 0, QUParameter::Out }
    };
    static const QUMethod slot_15 = {"get_b", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_double, 0, QUParameter::Out }
    };
    static const QUMethod slot_16 = {"get_c0", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_17 = {"run", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_charstar, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"filewrite", 2, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_ptr, "FILE", QUParameter::In }
    };
    static const QUMethod slot_19 = {"fprintparams", 1, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_ptr, "FILE", QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"fprintparams", 2, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_ptr, "FILE", QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_21 = {"fprintinitparams", 2, param_slot_21 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_ptr, "FILE", QUParameter::In },
	{ 0, &static_QUType_charstar, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"fprinterror", 2, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct mfem_data", QUParameter::In },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In },
	{ 0, &static_QUType_ptr, "double*", QUParameter::InOut }
    };
    static const QUMethod slot_23 = {"interpolate", 7, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "struct mfem_data", QUParameter::In },
	{ 0, &static_QUType_ptr, "struct mfem_data", QUParameter::In }
    };
    static const QUMethod slot_24 = {"interpolate", 3, param_slot_24 };
    static const QUMethod slot_25 = {"free_data", 0, 0 };
    static const QUMethod slot_26 = {"zero_data", 0, 0 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_27 = {"mfem", 1, param_slot_27 };
    static const QUParameter param_slot_28[] = {
	{ 0, &static_QUType_double, 0, QUParameter::Out },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In }
    };
    static const QUMethod slot_28 = {"IntConcentration", 4, param_slot_28 };
    static const QUParameter param_slot_29[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In }
    };
    static const QUMethod slot_29 = {"tridiag", 5, param_slot_29 };
    static const QMetaData slot_tbl[] = {
	{ "set_N(int)", &slot_0, QMetaData::Public },
	{ "set_s(double)", &slot_1, QMetaData::Public },
	{ "set_D(double)", &slot_2, QMetaData::Public },
	{ "set_rpm(double)", &slot_3, QMetaData::Public },
	{ "set_total_t(double)", &slot_4, QMetaData::Public },
	{ "set_m(double)", &slot_5, QMetaData::Public },
	{ "set_b(double)", &slot_6, QMetaData::Public },
	{ "set_c0(double)", &slot_7, QMetaData::Public },
	{ "stop()", &slot_8, QMetaData::Public },
	{ "get_N()", &slot_9, QMetaData::Public },
	{ "get_s()", &slot_10, QMetaData::Public },
	{ "get_D()", &slot_11, QMetaData::Public },
	{ "get_rpm()", &slot_12, QMetaData::Public },
	{ "get_total_t()", &slot_13, QMetaData::Public },
	{ "get_m()", &slot_14, QMetaData::Public },
	{ "get_b()", &slot_15, QMetaData::Public },
	{ "get_c0()", &slot_16, QMetaData::Public },
	{ "run()", &slot_17, QMetaData::Public },
	{ "filewrite(char*)", &slot_18, QMetaData::Public },
	{ "fprintparams(FILE*)", &slot_19, QMetaData::Public },
	{ "fprintparams(FILE*,int)", &slot_20, QMetaData::Public },
	{ "fprintinitparams(FILE*,int)", &slot_21, QMetaData::Public },
	{ "fprinterror(FILE*,const char*)", &slot_22, QMetaData::Public },
	{ "interpolate(struct mfem_data*,unsigned int,unsigned int,float*,double*,double**)", &slot_23, QMetaData::Public },
	{ "interpolate(struct mfem_data*,struct mfem_data*)", &slot_24, QMetaData::Public },
	{ "free_data()", &slot_25, QMetaData::Private },
	{ "zero_data()", &slot_26, QMetaData::Private },
	{ "mfem()", &slot_27, QMetaData::Private },
	{ "IntConcentration(double*,int,double*)", &slot_28, QMetaData::Private },
	{ "tridiag(int,double*,double*,double*)", &slot_29, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_ptr, "struct mfem_scan", QUParameter::In },
	{ 0, &static_QUType_ptr, "std::vector<double>", QUParameter::In }
    };
    static const QUMethod signal_0 = {"scan_updated", 2, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "scan_updated(struct mfem_scan,std::vector<double>)", &signal_0, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_MovingFEM", parentObject,
	slot_tbl, 30,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_MovingFEM.setMetaObject( metaObj );
    return metaObj;
}

void* US_MovingFEM::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_MovingFEM" ) )
	return this;
    return QObject::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL scan_updated
void US_MovingFEM::scan_updated( struct mfem_scan t0, std::vector<double> t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_ptr.set(o+1,&t0);
    static_QUType_ptr.set(o+2,&t1);
    activate_signal( clist, o );
}

bool US_MovingFEM::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: set_N((int)static_QUType_int.get(_o+1)); break;
    case 1: set_s((double)static_QUType_double.get(_o+1)); break;
    case 2: set_D((double)static_QUType_double.get(_o+1)); break;
    case 3: set_rpm((double)static_QUType_double.get(_o+1)); break;
    case 4: set_total_t((double)static_QUType_double.get(_o+1)); break;
    case 5: set_m((double)static_QUType_double.get(_o+1)); break;
    case 6: set_b((double)static_QUType_double.get(_o+1)); break;
    case 7: set_c0((double)static_QUType_double.get(_o+1)); break;
    case 8: stop(); break;
    case 9: static_QUType_int.set(_o,get_N()); break;
    case 10: static_QUType_double.set(_o,get_s()); break;
    case 11: static_QUType_double.set(_o,get_D()); break;
    case 12: static_QUType_double.set(_o,get_rpm()); break;
    case 13: static_QUType_double.set(_o,get_total_t()); break;
    case 14: static_QUType_double.set(_o,get_m()); break;
    case 15: static_QUType_double.set(_o,get_b()); break;
    case 16: static_QUType_double.set(_o,get_c0()); break;
    case 17: static_QUType_int.set(_o,run()); break;
    case 18: static_QUType_int.set(_o,filewrite((char*)static_QUType_charstar.get(_o+1))); break;
    case 19: fprintparams((FILE*)static_QUType_ptr.get(_o+1)); break;
    case 20: fprintparams((FILE*)static_QUType_ptr.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 21: fprintinitparams((FILE*)static_QUType_ptr.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 22: fprinterror((FILE*)static_QUType_ptr.get(_o+1),(const char*)static_QUType_charstar.get(_o+2)); break;
    case 23: static_QUType_int.set(_o,interpolate((struct mfem_data*)static_QUType_ptr.get(_o+1),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+2))),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+3))),(float*)static_QUType_ptr.get(_o+4),(double*)static_QUType_varptr.get(_o+5),(double**)static_QUType_ptr.get(_o+6))); break;
    case 24: static_QUType_int.set(_o,interpolate((struct mfem_data*)static_QUType_ptr.get(_o+1),(struct mfem_data*)static_QUType_ptr.get(_o+2))); break;
    case 25: free_data(); break;
    case 26: zero_data(); break;
    case 27: static_QUType_int.set(_o,mfem()); break;
    case 28: static_QUType_double.set(_o,IntConcentration((double*)static_QUType_varptr.get(_o+1),(int)static_QUType_int.get(_o+2),(double*)static_QUType_varptr.get(_o+3))); break;
    case 29: static_QUType_int.set(_o,tridiag((int)static_QUType_int.get(_o+1),(double*)static_QUType_varptr.get(_o+2),(double*)static_QUType_varptr.get(_o+3),(double*)static_QUType_varptr.get(_o+4))); break;
    default:
	return QObject::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_MovingFEM::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: scan_updated((struct mfem_scan)(*((struct mfem_scan*)static_QUType_ptr.get(_o+1))),(std::vector<double>)(*((std::vector<double>*)static_QUType_ptr.get(_o+2)))); break;
    default:
	return QObject::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_MovingFEM::qt_property( int id, int f, QVariant* v)
{
    return QObject::qt_property( id, f, v);
}

bool US_MovingFEM::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
