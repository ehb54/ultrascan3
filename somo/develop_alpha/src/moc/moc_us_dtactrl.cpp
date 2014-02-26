/****************************************************************************
** Data_Control_W meta object code from reading C++ file 'us_dtactrl.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_dtactrl.h"
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

const char *Data_Control_W::className() const
{
    return "Data_Control_W";
}

QMetaObject *Data_Control_W::metaObj = 0;
static QMetaObjectCleanUp cleanUp_Data_Control_W( "Data_Control_W", &Data_Control_W::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString Data_Control_W::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "Data_Control_W", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString Data_Control_W::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "Data_Control_W", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* Data_Control_W::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"cleanup_loaded_scan", 0, 0 };
    static const QUMethod slot_1 = {"cleanup_copied_scan", 0, 0 };
    static const QUMethod slot_2 = {"cleanup_loaded_run", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"newMessage", 2, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"newMessage", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"show_cell", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"show_lambda", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"show_channel", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_8 = {"load_data", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"load_data", 2, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"load_data", 2, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"load_data", 3, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_12 = {"write_data", 1, param_slot_12 };
    static const QUMethod slot_13 = {"details", 0, 0 };
    static const QUMethod slot_14 = {"updateLabels", 0, 0 };
    static const QUMethod slot_15 = {"quit", 0, 0 };
    static const QUMethod slot_16 = {"update_density", 0, 0 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"update_density", 1, param_slot_17 };
    static const QUMethod slot_18 = {"update_viscosity", 0, 0 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"update_viscosity", 1, param_slot_19 };
    static const QUMethod slot_20 = {"read_vbar", 0, 0 };
    static const QUMethod slot_21 = {"update_vbar", 0, 0 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"update_vbar", 1, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_23 = {"update_vbar_lbl", 2, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_24 = {"update_buffer_lbl", 2, param_slot_24 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"update_smoothing", 1, param_slot_25 };
    static const QUParameter param_slot_26[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_26 = {"update_single", 1, param_slot_26 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_27 = {"update_range", 1, param_slot_27 };
    static const QUParameter param_slot_28[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_28 = {"update_boundary_range", 1, param_slot_28 };
    static const QUParameter param_slot_29[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_29 = {"update_boundary_position", 1, param_slot_29 };
    static const QUMethod slot_30 = {"ex_single", 0, 0 };
    static const QUMethod slot_31 = {"ex_range", 0, 0 };
    static const QUMethod slot_32 = {"plot_edit", 0, 0 };
    static const QUMethod slot_33 = {"update_screen", 0, 0 };
    static const QUParameter param_slot_34[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_34 = {"load_scan", 1, param_slot_34 };
    static const QUParameter param_slot_35[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_35 = {"calc_correction", 1, param_slot_35 };
    static const QUMethod slot_36 = {"find_plateaus", 0, 0 };
    static const QUMethod slot_37 = {"get_buffer", 0, 0 };
    static const QUParameter param_slot_38[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_38 = {"update_buffer_signal", 2, param_slot_38 };
    static const QUParameter param_slot_39[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_39 = {"update_vbar_signal", 2, param_slot_39 };
    static const QUParameter param_slot_40[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_40 = {"update_mw_signal", 1, param_slot_40 };
    static const QUParameter param_slot_41[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_41 = {"createHtmlDir", 1, param_slot_41 };
    static const QUParameter param_slot_42[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_42 = {"plot_analysis", 1, param_slot_42 };
    static const QUParameter param_slot_43[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_43 = {"setups", 1, param_slot_43 };
    static const QUMethod slot_44 = {"view", 0, 0 };
    static const QUMethod slot_45 = {"reset", 0, 0 };
    static const QUMethod slot_46 = {"help", 0, 0 };
    static const QUMethod slot_47 = {"save", 0, 0 };
    static const QUMethod slot_48 = {"second_plot", 0, 0 };
    static const QUParameter param_slot_49[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_49 = {"getAnalysisPlotMouseReleased", 1, param_slot_49 };
    static const QUParameter param_slot_50[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_50 = {"getAnalysisPlotMousePressed", 1, param_slot_50 };
    static const QUMethod slot_51 = {"print", 0, 0 };
    static const QUMethod slot_52 = {"updateButtons", 0, 0 };
    static const QUParameter param_slot_53[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_53 = {"closeEvent", 1, param_slot_53 };
    static const QUParameter param_slot_54[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_54 = {"excludeSingleSignal", 1, param_slot_54 };
    static const QUParameter param_slot_55[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_55 = {"excludeRangeSignal", 1, param_slot_55 };
    static const QUMethod slot_56 = {"create_GUI", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "cleanup_loaded_scan()", &slot_0, QMetaData::Public },
	{ "cleanup_copied_scan()", &slot_1, QMetaData::Public },
	{ "cleanup_loaded_run()", &slot_2, QMetaData::Public },
	{ "newMessage(QString,int)", &slot_3, QMetaData::Public },
	{ "newMessage(QString)", &slot_4, QMetaData::Public },
	{ "show_cell(int)", &slot_5, QMetaData::Public },
	{ "show_lambda(int)", &slot_6, QMetaData::Public },
	{ "show_channel(int)", &slot_7, QMetaData::Public },
	{ "load_data()", &slot_8, QMetaData::Public },
	{ "load_data(const QString&)", &slot_9, QMetaData::Public },
	{ "load_data(bool)", &slot_10, QMetaData::Public },
	{ "load_data(const QString&,bool)", &slot_11, QMetaData::Public },
	{ "write_data()", &slot_12, QMetaData::Public },
	{ "details()", &slot_13, QMetaData::Public },
	{ "updateLabels()", &slot_14, QMetaData::Public },
	{ "quit()", &slot_15, QMetaData::Public },
	{ "update_density()", &slot_16, QMetaData::Public },
	{ "update_density(const QString&)", &slot_17, QMetaData::Public },
	{ "update_viscosity()", &slot_18, QMetaData::Public },
	{ "update_viscosity(const QString&)", &slot_19, QMetaData::Public },
	{ "read_vbar()", &slot_20, QMetaData::Public },
	{ "update_vbar()", &slot_21, QMetaData::Public },
	{ "update_vbar(const QString&)", &slot_22, QMetaData::Public },
	{ "update_vbar_lbl(float,float)", &slot_23, QMetaData::Public },
	{ "update_buffer_lbl(float,float)", &slot_24, QMetaData::Public },
	{ "update_smoothing(double)", &slot_25, QMetaData::Public },
	{ "update_single(double)", &slot_26, QMetaData::Public },
	{ "update_range(double)", &slot_27, QMetaData::Public },
	{ "update_boundary_range(double)", &slot_28, QMetaData::Public },
	{ "update_boundary_position(double)", &slot_29, QMetaData::Public },
	{ "ex_single()", &slot_30, QMetaData::Public },
	{ "ex_range()", &slot_31, QMetaData::Public },
	{ "plot_edit()", &slot_32, QMetaData::Public },
	{ "update_screen()", &slot_33, QMetaData::Public },
	{ "load_scan()", &slot_34, QMetaData::Public },
	{ "calc_correction(float)", &slot_35, QMetaData::Public },
	{ "find_plateaus()", &slot_36, QMetaData::Public },
	{ "get_buffer()", &slot_37, QMetaData::Public },
	{ "update_buffer_signal(float,float)", &slot_38, QMetaData::Public },
	{ "update_vbar_signal(float,float)", &slot_39, QMetaData::Public },
	{ "update_mw_signal(float)", &slot_40, QMetaData::Public },
	{ "createHtmlDir()", &slot_41, QMetaData::Public },
	{ "plot_analysis()", &slot_42, QMetaData::Public },
	{ "setups()", &slot_43, QMetaData::Public },
	{ "view()", &slot_44, QMetaData::Public },
	{ "reset()", &slot_45, QMetaData::Public },
	{ "help()", &slot_46, QMetaData::Public },
	{ "save()", &slot_47, QMetaData::Public },
	{ "second_plot()", &slot_48, QMetaData::Public },
	{ "getAnalysisPlotMouseReleased(const QMouseEvent&)", &slot_49, QMetaData::Public },
	{ "getAnalysisPlotMousePressed(const QMouseEvent&)", &slot_50, QMetaData::Public },
	{ "print()", &slot_51, QMetaData::Public },
	{ "updateButtons()", &slot_52, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_53, QMetaData::Protected },
	{ "excludeSingleSignal(unsigned int)", &slot_54, QMetaData::Protected },
	{ "excludeRangeSignal(unsigned int)", &slot_55, QMetaData::Protected },
	{ "create_GUI()", &slot_56, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_0 = {"vbarChanged", 2, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_1 = {"mwChanged", 1, param_signal_1 };
    static const QUParameter param_signal_2[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_2 = {"bufferChanged", 2, param_signal_2 };
    static const QUMethod signal_3 = {"dataLoaded", 0, 0 };
    static const QUMethod signal_4 = {"excludeUpdated", 0, 0 };
    static const QUMethod signal_5 = {"controls_updated", 0, 0 };
    static const QUMethod signal_6 = {"datasetChanged", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "vbarChanged(float,float)", &signal_0, QMetaData::Private },
	{ "mwChanged(float)", &signal_1, QMetaData::Private },
	{ "bufferChanged(float,float)", &signal_2, QMetaData::Private },
	{ "dataLoaded()", &signal_3, QMetaData::Private },
	{ "excludeUpdated()", &signal_4, QMetaData::Private },
	{ "controls_updated()", &signal_5, QMetaData::Private },
	{ "datasetChanged()", &signal_6, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"Data_Control_W", parentObject,
	slot_tbl, 57,
	signal_tbl, 7,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_Data_Control_W.setMetaObject( metaObj );
    return metaObj;
}

void* Data_Control_W::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "Data_Control_W" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL vbarChanged
void Data_Control_W::vbarChanged( float t0, float t1 )
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

// SIGNAL mwChanged
void Data_Control_W::mwChanged( float t0 )
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

// SIGNAL bufferChanged
void Data_Control_W::bufferChanged( float t0, float t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 2 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_ptr.set(o+1,&t0);
    static_QUType_ptr.set(o+2,&t1);
    activate_signal( clist, o );
}

// SIGNAL dataLoaded
void Data_Control_W::dataLoaded()
{
    activate_signal( staticMetaObject()->signalOffset() + 3 );
}

// SIGNAL excludeUpdated
void Data_Control_W::excludeUpdated()
{
    activate_signal( staticMetaObject()->signalOffset() + 4 );
}

// SIGNAL controls_updated
void Data_Control_W::controls_updated()
{
    activate_signal( staticMetaObject()->signalOffset() + 5 );
}

// SIGNAL datasetChanged
void Data_Control_W::datasetChanged()
{
    activate_signal( staticMetaObject()->signalOffset() + 6 );
}

bool Data_Control_W::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: cleanup_loaded_scan(); break;
    case 1: cleanup_copied_scan(); break;
    case 2: cleanup_loaded_run(); break;
    case 3: newMessage((QString)static_QUType_QString.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 4: newMessage((QString)static_QUType_QString.get(_o+1)); break;
    case 5: show_cell((int)static_QUType_int.get(_o+1)); break;
    case 6: show_lambda((int)static_QUType_int.get(_o+1)); break;
    case 7: show_channel((int)static_QUType_int.get(_o+1)); break;
    case 8: static_QUType_int.set(_o,load_data()); break;
    case 9: static_QUType_int.set(_o,load_data((const QString&)static_QUType_QString.get(_o+1))); break;
    case 10: static_QUType_int.set(_o,load_data((bool)static_QUType_bool.get(_o+1))); break;
    case 11: static_QUType_int.set(_o,load_data((const QString&)static_QUType_QString.get(_o+1),(bool)static_QUType_bool.get(_o+2))); break;
    case 12: static_QUType_int.set(_o,write_data()); break;
    case 13: details(); break;
    case 14: updateLabels(); break;
    case 15: quit(); break;
    case 16: update_density(); break;
    case 17: update_density((const QString&)static_QUType_QString.get(_o+1)); break;
    case 18: update_viscosity(); break;
    case 19: update_viscosity((const QString&)static_QUType_QString.get(_o+1)); break;
    case 20: read_vbar(); break;
    case 21: update_vbar(); break;
    case 22: update_vbar((const QString&)static_QUType_QString.get(_o+1)); break;
    case 23: update_vbar_lbl((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 24: update_buffer_lbl((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 25: update_smoothing((double)static_QUType_double.get(_o+1)); break;
    case 26: update_single((double)static_QUType_double.get(_o+1)); break;
    case 27: update_range((double)static_QUType_double.get(_o+1)); break;
    case 28: update_boundary_range((double)static_QUType_double.get(_o+1)); break;
    case 29: update_boundary_position((double)static_QUType_double.get(_o+1)); break;
    case 30: ex_single(); break;
    case 31: ex_range(); break;
    case 32: plot_edit(); break;
    case 33: update_screen(); break;
    case 34: static_QUType_int.set(_o,load_scan()); break;
    case 35: calc_correction((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 36: find_plateaus(); break;
    case 37: get_buffer(); break;
    case 38: update_buffer_signal((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 39: update_vbar_signal((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 40: update_mw_signal((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 41: static_QUType_bool.set(_o,createHtmlDir()); break;
    case 42: static_QUType_int.set(_o,plot_analysis()); break;
    case 43: static_QUType_int.set(_o,setups()); break;
    case 44: view(); break;
    case 45: reset(); break;
    case 46: help(); break;
    case 47: save(); break;
    case 48: second_plot(); break;
    case 49: getAnalysisPlotMouseReleased((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 50: getAnalysisPlotMousePressed((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 51: print(); break;
    case 52: updateButtons(); break;
    case 53: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 54: excludeSingleSignal((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    case 55: excludeRangeSignal((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    case 56: create_GUI(); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool Data_Control_W::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: vbarChanged((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 1: mwChanged((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 2: bufferChanged((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 3: dataLoaded(); break;
    case 4: excludeUpdated(); break;
    case 5: controls_updated(); break;
    case 6: datasetChanged(); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool Data_Control_W::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool Data_Control_W::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
