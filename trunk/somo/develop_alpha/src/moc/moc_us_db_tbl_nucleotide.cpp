/****************************************************************************
** US_Nucleotide_DB meta object code from reading C++ file 'us_db_tbl_nucleotide.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_db_tbl_nucleotide.h"
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

const char *US_Nucleotide_DB::className() const
{
    return "US_Nucleotide_DB";
}

QMetaObject *US_Nucleotide_DB::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Nucleotide_DB( "US_Nucleotide_DB", &US_Nucleotide_DB::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Nucleotide_DB::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Nucleotide_DB", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Nucleotide_DB::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Nucleotide_DB", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Nucleotide_DB::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_DB::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"retrieve_DNA", 1, param_slot_0 };
    static const QUMethod slot_1 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_2 = {"load", 0, 0 };
    static const QUMethod slot_3 = {"download", 0, 0 };
    static const QUMethod slot_4 = {"update", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_vbar", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_e280", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_e260", 1, param_slot_7 };
    static const QUMethod slot_8 = {"update_doubleStranded", 0, 0 };
    static const QUMethod slot_9 = {"update_complement", 0, 0 };
    static const QUMethod slot_10 = {"update_3prime_oh", 0, 0 };
    static const QUMethod slot_11 = {"update_3prime_po4", 0, 0 };
    static const QUMethod slot_12 = {"update_5prime_oh", 0, 0 };
    static const QUMethod slot_13 = {"update_5prime_po4", 0, 0 };
    static const QUMethod slot_14 = {"update_DNA", 0, 0 };
    static const QUMethod slot_15 = {"update_RNA", 0, 0 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"update_sodium", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"update_potassium", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"update_lithium", 1, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"update_calcium", 1, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"update_magnesium", 1, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ "item", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_21 = {"select_DNA", 1, param_slot_21 };
    static const QUMethod slot_22 = {"sel_investigator", 0, 0 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"update_investigator_lbl", 2, param_slot_23 };
    static const QUMethod slot_24 = {"save", 0, 0 };
    static const QUMethod slot_25 = {"help", 0, 0 };
    static const QUMethod slot_26 = {"read_db", 0, 0 };
    static const QUMethod slot_27 = {"enter_DNA", 0, 0 };
    static const QUMethod slot_28 = {"check_permission", 0, 0 };
    static const QUParameter param_slot_29[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_29 = {"delete_db", 1, param_slot_29 };
    static const QUMethod slot_30 = {"clear", 0, 0 };
    static const QUMethod slot_31 = {"reset", 0, 0 };
    static const QUMethod slot_32 = {"quit", 0, 0 };
    static const QUParameter param_slot_33[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_33 = {"closeEvent", 1, param_slot_33 };
    static const QMetaData slot_tbl[] = {
	{ "retrieve_DNA(int)", &slot_0, QMetaData::Public },
	{ "setup_GUI()", &slot_1, QMetaData::Private },
	{ "load()", &slot_2, QMetaData::Private },
	{ "download()", &slot_3, QMetaData::Private },
	{ "update()", &slot_4, QMetaData::Private },
	{ "update_vbar(const QString&)", &slot_5, QMetaData::Private },
	{ "update_e280(const QString&)", &slot_6, QMetaData::Private },
	{ "update_e260(const QString&)", &slot_7, QMetaData::Private },
	{ "update_doubleStranded()", &slot_8, QMetaData::Private },
	{ "update_complement()", &slot_9, QMetaData::Private },
	{ "update_3prime_oh()", &slot_10, QMetaData::Private },
	{ "update_3prime_po4()", &slot_11, QMetaData::Private },
	{ "update_5prime_oh()", &slot_12, QMetaData::Private },
	{ "update_5prime_po4()", &slot_13, QMetaData::Private },
	{ "update_DNA()", &slot_14, QMetaData::Private },
	{ "update_RNA()", &slot_15, QMetaData::Private },
	{ "update_sodium(double)", &slot_16, QMetaData::Private },
	{ "update_potassium(double)", &slot_17, QMetaData::Private },
	{ "update_lithium(double)", &slot_18, QMetaData::Private },
	{ "update_calcium(double)", &slot_19, QMetaData::Private },
	{ "update_magnesium(double)", &slot_20, QMetaData::Private },
	{ "select_DNA(int)", &slot_21, QMetaData::Private },
	{ "sel_investigator()", &slot_22, QMetaData::Private },
	{ "update_investigator_lbl(QString,int)", &slot_23, QMetaData::Private },
	{ "save()", &slot_24, QMetaData::Private },
	{ "help()", &slot_25, QMetaData::Private },
	{ "read_db()", &slot_26, QMetaData::Private },
	{ "enter_DNA()", &slot_27, QMetaData::Private },
	{ "check_permission()", &slot_28, QMetaData::Private },
	{ "delete_db(bool)", &slot_29, QMetaData::Private },
	{ "clear()", &slot_30, QMetaData::Private },
	{ "reset()", &slot_31, QMetaData::Private },
	{ "quit()", &slot_32, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_33, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ "DNAID", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"IdChanged", 1, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "IdChanged(int)", &signal_0, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Nucleotide_DB", parentObject,
	slot_tbl, 34,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Nucleotide_DB.setMetaObject( metaObj );
    return metaObj;
}

void* US_Nucleotide_DB::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Nucleotide_DB" ) )
	return this;
    return US_DB::qt_cast( clname );
}

// SIGNAL IdChanged
void US_Nucleotide_DB::IdChanged( int t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 0, t0 );
}

bool US_Nucleotide_DB::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: retrieve_DNA((int)static_QUType_int.get(_o+1)); break;
    case 1: setup_GUI(); break;
    case 2: load(); break;
    case 3: download(); break;
    case 4: update(); break;
    case 5: update_vbar((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_e280((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_e260((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update_doubleStranded(); break;
    case 9: update_complement(); break;
    case 10: update_3prime_oh(); break;
    case 11: update_3prime_po4(); break;
    case 12: update_5prime_oh(); break;
    case 13: update_5prime_po4(); break;
    case 14: update_DNA(); break;
    case 15: update_RNA(); break;
    case 16: update_sodium((double)static_QUType_double.get(_o+1)); break;
    case 17: update_potassium((double)static_QUType_double.get(_o+1)); break;
    case 18: update_lithium((double)static_QUType_double.get(_o+1)); break;
    case 19: update_calcium((double)static_QUType_double.get(_o+1)); break;
    case 20: update_magnesium((double)static_QUType_double.get(_o+1)); break;
    case 21: select_DNA((int)static_QUType_int.get(_o+1)); break;
    case 22: sel_investigator(); break;
    case 23: update_investigator_lbl((QString)static_QUType_QString.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 24: save(); break;
    case 25: help(); break;
    case 26: read_db(); break;
    case 27: enter_DNA(); break;
    case 28: check_permission(); break;
    case 29: delete_db((bool)static_QUType_bool.get(_o+1)); break;
    case 30: clear(); break;
    case 31: reset(); break;
    case 32: quit(); break;
    case 33: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return US_DB::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Nucleotide_DB::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: IdChanged((int)static_QUType_int.get(_o+1)); break;
    default:
	return US_DB::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_Nucleotide_DB::qt_property( int id, int f, QVariant* v)
{
    return US_DB::qt_property( id, f, v);
}

bool US_Nucleotide_DB::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
