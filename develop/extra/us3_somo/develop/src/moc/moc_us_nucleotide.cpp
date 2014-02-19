/****************************************************************************
** US_Nucleotide meta object code from reading C++ file 'us_nucleotide.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_nucleotide.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <Q3Frame>
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Nucleotide::className() const
{
    return "US_Nucleotide";
}

QMetaObject *US_Nucleotide::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Nucleotide( "US_Nucleotide", &US_Nucleotide::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Nucleotide::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Nucleotide", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Nucleotide::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Nucleotide", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Nucleotide::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"load", 0, 0 };
    static const QUMethod slot_1 = {"download", 0, 0 };
    static const QUMethod slot_2 = {"update", 0, 0 };
    static const QUMethod slot_3 = {"update_doubleStranded", 0, 0 };
    static const QUMethod slot_4 = {"update_complement", 0, 0 };
    static const QUMethod slot_5 = {"update_3prime_oh", 0, 0 };
    static const QUMethod slot_6 = {"update_3prime_po4", 0, 0 };
    static const QUMethod slot_7 = {"update_5prime_oh", 0, 0 };
    static const QUMethod slot_8 = {"update_5prime_po4", 0, 0 };
    static const QUMethod slot_9 = {"update_DNA", 0, 0 };
    static const QUMethod slot_10 = {"update_RNA", 0, 0 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_sodium", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_potassium", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_lithium", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_calcium", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_magnesium", 1, param_slot_15 };
    static const QUMethod slot_16 = {"save", 0, 0 };
    static const QUMethod slot_17 = {"help", 0, 0 };
    static const QUMethod slot_18 = {"quit", 0, 0 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_19 = {"closeEvent", 1, param_slot_19 };
    static const QMetaData slot_tbl[] = {
	{ "load()", &slot_0, QMetaData::Public },
	{ "download()", &slot_1, QMetaData::Public },
	{ "update()", &slot_2, QMetaData::Public },
	{ "update_doubleStranded()", &slot_3, QMetaData::Public },
	{ "update_complement()", &slot_4, QMetaData::Public },
	{ "update_3prime_oh()", &slot_5, QMetaData::Public },
	{ "update_3prime_po4()", &slot_6, QMetaData::Public },
	{ "update_5prime_oh()", &slot_7, QMetaData::Public },
	{ "update_5prime_po4()", &slot_8, QMetaData::Public },
	{ "update_DNA()", &slot_9, QMetaData::Public },
	{ "update_RNA()", &slot_10, QMetaData::Public },
	{ "update_sodium(double)", &slot_11, QMetaData::Public },
	{ "update_potassium(double)", &slot_12, QMetaData::Public },
	{ "update_lithium(double)", &slot_13, QMetaData::Public },
	{ "update_calcium(double)", &slot_14, QMetaData::Public },
	{ "update_magnesium(double)", &slot_15, QMetaData::Public },
	{ "save()", &slot_16, QMetaData::Public },
	{ "help()", &slot_17, QMetaData::Public },
	{ "quit()", &slot_18, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_19, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Nucleotide", parentObject,
	slot_tbl, 20,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Nucleotide.setMetaObject( metaObj );
    return metaObj;
}

void* US_Nucleotide::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Nucleotide" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Nucleotide::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: load(); break;
    case 1: download(); break;
    case 2: update(); break;
    case 3: update_doubleStranded(); break;
    case 4: update_complement(); break;
    case 5: update_3prime_oh(); break;
    case 6: update_3prime_po4(); break;
    case 7: update_5prime_oh(); break;
    case 8: update_5prime_po4(); break;
    case 9: update_DNA(); break;
    case 10: update_RNA(); break;
    case 11: update_sodium((double)static_QUType_double.get(_o+1)); break;
    case 12: update_potassium((double)static_QUType_double.get(_o+1)); break;
    case 13: update_lithium((double)static_QUType_double.get(_o+1)); break;
    case 14: update_calcium((double)static_QUType_double.get(_o+1)); break;
    case 15: update_magnesium((double)static_QUType_double.get(_o+1)); break;
    case 16: save(); break;
    case 17: help(); break;
    case 18: quit(); break;
    case 19: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Nucleotide::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Nucleotide::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Nucleotide::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
