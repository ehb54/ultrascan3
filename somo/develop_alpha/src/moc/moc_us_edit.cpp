/****************************************************************************
** EditData_Win meta object code from reading C++ file 'us_edit.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_edit.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <Q3Frame>
#include <QMouseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *EditData_Win::className() const
{
    return "EditData_Win";
}

QMetaObject *EditData_Win::metaObj = 0;
static QMetaObjectCleanUp cleanUp_EditData_Win( "EditData_Win", &EditData_Win::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString EditData_Win::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "EditData_Win", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString EditData_Win::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "EditData_Win", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* EditData_Win::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"quitthis", 0, 0 };
    static const QUMethod slot_1 = {"details", 0, 0 };
    static const QUMethod slot_2 = {"directory", 0, 0 };
    static const QUMethod slot_3 = {"fluorescence_directory", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"newdir", 1, param_slot_4 };
    static const QUMethod slot_5 = {"edit_all", 0, 0 };
    static const QUMethod slot_6 = {"invert", 0, 0 };
    static const QUMethod slot_7 = {"load_dataset", 0, 0 };
    static const QUMethod slot_8 = {"plot_dataset", 0, 0 };
    static const QUParameter param_slot_9[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_9 = {"get_x", 1, param_slot_9 };
    static const QUMethod slot_10 = {"help", 0, 0 };
    static const QUMethod slot_11 = {"sub_baseline", 0, 0 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"set_range1", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"set_range2", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"set_editscan", 1, param_slot_14 };
    static const QUMethod slot_15 = {"exSingle", 0, 0 };
    static const QUMethod slot_16 = {"exRange", 0, 0 };
    static const QUMethod slot_17 = {"remove_spikes", 0, 0 };
    static const QUMethod slot_18 = {"edSingle", 0, 0 };
    static const QUMethod slot_19 = {"write_equil_file", 0, 0 };
    static const QUMethod slot_20 = {"set_zoom", 0, 0 };
    static const QUMethod slot_21 = {"exclude_profile", 0, 0 };
    static const QUParameter param_slot_22[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_22 = {"getMousePressed", 1, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"update_rotor", 1, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_24 = {"update_centerpiece", 1, param_slot_24 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_ptr, "vector<bool>", QUParameter::In }
    };
    static const QUMethod slot_25 = {"update_exclude_profile", 1, param_slot_25 };
    static const QUParameter param_slot_26[] = {
	{ 0, &static_QUType_ptr, "vector<bool>", QUParameter::In }
    };
    static const QUMethod slot_26 = {"final_exclude_profile", 1, param_slot_26 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_27 = {"closeEvent", 1, param_slot_27 };
    static const QUMethod slot_28 = {"cleanup_dataset", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "quitthis()", &slot_0, QMetaData::Public },
	{ "details()", &slot_1, QMetaData::Public },
	{ "directory()", &slot_2, QMetaData::Public },
	{ "fluorescence_directory()", &slot_3, QMetaData::Public },
	{ "newdir(const QString&)", &slot_4, QMetaData::Public },
	{ "edit_all()", &slot_5, QMetaData::Public },
	{ "invert()", &slot_6, QMetaData::Public },
	{ "load_dataset()", &slot_7, QMetaData::Public },
	{ "plot_dataset()", &slot_8, QMetaData::Public },
	{ "get_x(const QMouseEvent&)", &slot_9, QMetaData::Public },
	{ "help()", &slot_10, QMetaData::Public },
	{ "sub_baseline()", &slot_11, QMetaData::Public },
	{ "set_range1(double)", &slot_12, QMetaData::Public },
	{ "set_range2(double)", &slot_13, QMetaData::Public },
	{ "set_editscan(double)", &slot_14, QMetaData::Public },
	{ "exSingle()", &slot_15, QMetaData::Public },
	{ "exRange()", &slot_16, QMetaData::Public },
	{ "remove_spikes()", &slot_17, QMetaData::Public },
	{ "edSingle()", &slot_18, QMetaData::Public },
	{ "write_equil_file()", &slot_19, QMetaData::Public },
	{ "set_zoom()", &slot_20, QMetaData::Public },
	{ "exclude_profile()", &slot_21, QMetaData::Public },
	{ "getMousePressed(const QMouseEvent&)", &slot_22, QMetaData::Public },
	{ "update_rotor(int)", &slot_23, QMetaData::Public },
	{ "update_centerpiece(int)", &slot_24, QMetaData::Public },
	{ "update_exclude_profile(vector<bool>)", &slot_25, QMetaData::Public },
	{ "final_exclude_profile(vector<bool>)", &slot_26, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_27, QMetaData::Protected },
	{ "cleanup_dataset()", &slot_28, QMetaData::Protected }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"explain", 1, param_signal_0 };
    static const QUMethod signal_1 = {"clicked", 0, 0 };
    static const QUParameter param_signal_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod signal_2 = {"channel_selected", 1, param_signal_2 };
    static const QUMethod signal_3 = {"absorbance_changed", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "explain(const QString&)", &signal_0, QMetaData::Protected },
	{ "clicked()", &signal_1, QMetaData::Protected },
	{ "channel_selected(const QString&)", &signal_2, QMetaData::Protected },
	{ "absorbance_changed()", &signal_3, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"EditData_Win", parentObject,
	slot_tbl, 29,
	signal_tbl, 4,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_EditData_Win.setMetaObject( metaObj );
    return metaObj;
}

void* EditData_Win::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "EditData_Win" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

// SIGNAL explain
void EditData_Win::explain( const QString& t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 0, t0 );
}

// SIGNAL clicked
void EditData_Win::clicked()
{
    activate_signal( staticMetaObject()->signalOffset() + 1 );
}

// SIGNAL channel_selected
void EditData_Win::channel_selected( const QString& t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 2, t0 );
}

// SIGNAL absorbance_changed
void EditData_Win::absorbance_changed()
{
    activate_signal( staticMetaObject()->signalOffset() + 3 );
}

bool EditData_Win::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: quitthis(); break;
    case 1: details(); break;
    case 2: directory(); break;
    case 3: fluorescence_directory(); break;
    case 4: newdir((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: edit_all(); break;
    case 6: invert(); break;
    case 7: load_dataset(); break;
    case 8: plot_dataset(); break;
    case 9: get_x((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 10: help(); break;
    case 11: sub_baseline(); break;
    case 12: set_range1((double)static_QUType_double.get(_o+1)); break;
    case 13: set_range2((double)static_QUType_double.get(_o+1)); break;
    case 14: set_editscan((double)static_QUType_double.get(_o+1)); break;
    case 15: exSingle(); break;
    case 16: exRange(); break;
    case 17: remove_spikes(); break;
    case 18: edSingle(); break;
    case 19: write_equil_file(); break;
    case 20: set_zoom(); break;
    case 21: exclude_profile(); break;
    case 22: getMousePressed((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 23: update_rotor((int)static_QUType_int.get(_o+1)); break;
    case 24: update_centerpiece((int)static_QUType_int.get(_o+1)); break;
    case 25: update_exclude_profile((vector<bool>)(*((vector<bool>*)static_QUType_ptr.get(_o+1)))); break;
    case 26: final_exclude_profile((vector<bool>)(*((vector<bool>*)static_QUType_ptr.get(_o+1)))); break;
    case 27: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 28: cleanup_dataset(); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool EditData_Win::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: explain((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: clicked(); break;
    case 2: channel_selected((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: absorbance_changed(); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool EditData_Win::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool EditData_Win::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
