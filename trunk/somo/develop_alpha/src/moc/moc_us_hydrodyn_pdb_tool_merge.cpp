/****************************************************************************
** US_Hydrodyn_Pdb_Tool_Merge meta object code from reading C++ file 'us_hydrodyn_pdb_tool_merge.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_pdb_tool_merge.h"
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

const char *US_Hydrodyn_Pdb_Tool_Merge::className() const
{
    return "US_Hydrodyn_Pdb_Tool_Merge";
}

QMetaObject *US_Hydrodyn_Pdb_Tool_Merge::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Pdb_Tool_Merge( "US_Hydrodyn_Pdb_Tool_Merge", &US_Hydrodyn_Pdb_Tool_Merge::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Pdb_Tool_Merge::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Pdb_Tool_Merge", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Pdb_Tool_Merge::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Pdb_Tool_Merge", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Pdb_Tool_Merge::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"table_value", 2, param_slot_1 };
    static const QUMethod slot_2 = {"update_enables", 0, 0 };
    static const QUMethod slot_3 = {"sel_auto", 0, 0 };
    static const QUMethod slot_4 = {"sel_from_to_merge", 0, 0 };
    static const QUMethod slot_5 = {"sel_from_to_fit", 0, 0 };
    static const QUMethod slot_6 = {"sel_to_to_fit", 0, 0 };
    static const QUMethod slot_7 = {"sel_to_to_cut", 0, 0 };
    static const QUMethod slot_8 = {"extra_chains", 0, 0 };
    static const QUMethod slot_9 = {"only_closest", 0, 0 };
    static const QUMethod slot_10 = {"delete_row", 0, 0 };
    static const QUMethod slot_11 = {"clear", 0, 0 };
    static const QUMethod slot_12 = {"load", 0, 0 };
    static const QUMethod slot_13 = {"validate", 0, 0 };
    static const QUMethod slot_14 = {"csv_save", 0, 0 };
    static const QUMethod slot_15 = {"chains_from", 0, 0 };
    static const QUMethod slot_16 = {"chains_to", 0, 0 };
    static const QUMethod slot_17 = {"target", 0, 0 };
    static const QUMethod slot_18 = {"start", 0, 0 };
    static const QUMethod slot_19 = {"trial", 0, 0 };
    static const QUMethod slot_20 = {"stop", 0, 0 };
    static const QUMethod slot_21 = {"clear_display", 0, 0 };
    static const QUMethod slot_22 = {"update_font", 0, 0 };
    static const QUMethod slot_23 = {"save", 0, 0 };
    static const QUMethod slot_24 = {"cancel", 0, 0 };
    static const QUMethod slot_25 = {"pdb_tool", 0, 0 };
    static const QUMethod slot_26 = {"help", 0, 0 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_27 = {"closeEvent", 1, param_slot_27 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "table_value(int,int)", &slot_1, QMetaData::Private },
	{ "update_enables()", &slot_2, QMetaData::Private },
	{ "sel_auto()", &slot_3, QMetaData::Private },
	{ "sel_from_to_merge()", &slot_4, QMetaData::Private },
	{ "sel_from_to_fit()", &slot_5, QMetaData::Private },
	{ "sel_to_to_fit()", &slot_6, QMetaData::Private },
	{ "sel_to_to_cut()", &slot_7, QMetaData::Private },
	{ "extra_chains()", &slot_8, QMetaData::Private },
	{ "only_closest()", &slot_9, QMetaData::Private },
	{ "delete_row()", &slot_10, QMetaData::Private },
	{ "clear()", &slot_11, QMetaData::Private },
	{ "load()", &slot_12, QMetaData::Private },
	{ "validate()", &slot_13, QMetaData::Private },
	{ "csv_save()", &slot_14, QMetaData::Private },
	{ "chains_from()", &slot_15, QMetaData::Private },
	{ "chains_to()", &slot_16, QMetaData::Private },
	{ "target()", &slot_17, QMetaData::Private },
	{ "start()", &slot_18, QMetaData::Private },
	{ "trial()", &slot_19, QMetaData::Private },
	{ "stop()", &slot_20, QMetaData::Private },
	{ "clear_display()", &slot_21, QMetaData::Private },
	{ "update_font()", &slot_22, QMetaData::Private },
	{ "save()", &slot_23, QMetaData::Private },
	{ "cancel()", &slot_24, QMetaData::Private },
	{ "pdb_tool()", &slot_25, QMetaData::Private },
	{ "help()", &slot_26, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_27, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Pdb_Tool_Merge", parentObject,
	slot_tbl, 28,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Pdb_Tool_Merge.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Pdb_Tool_Merge::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Pdb_Tool_Merge" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Pdb_Tool_Merge::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: table_value((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 2: update_enables(); break;
    case 3: sel_auto(); break;
    case 4: sel_from_to_merge(); break;
    case 5: sel_from_to_fit(); break;
    case 6: sel_to_to_fit(); break;
    case 7: sel_to_to_cut(); break;
    case 8: extra_chains(); break;
    case 9: only_closest(); break;
    case 10: delete_row(); break;
    case 11: clear(); break;
    case 12: load(); break;
    case 13: validate(); break;
    case 14: csv_save(); break;
    case 15: chains_from(); break;
    case 16: chains_to(); break;
    case 17: target(); break;
    case 18: start(); break;
    case 19: trial(); break;
    case 20: stop(); break;
    case 21: clear_display(); break;
    case 22: update_font(); break;
    case 23: save(); break;
    case 24: cancel(); break;
    case 25: pdb_tool(); break;
    case 26: help(); break;
    case 27: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Pdb_Tool_Merge::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Pdb_Tool_Merge::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Pdb_Tool_Merge::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
