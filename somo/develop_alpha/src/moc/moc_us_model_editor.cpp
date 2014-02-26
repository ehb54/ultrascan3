/****************************************************************************
** US_ModelEditor meta object code from reading C++ file 'us_model_editor.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_model_editor.h"
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

const char *US_ModelEditor::className() const
{
    return "US_ModelEditor";
}

QMetaObject *US_ModelEditor::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_ModelEditor( "US_ModelEditor", &US_ModelEditor::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_ModelEditor::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ModelEditor", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_ModelEditor::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ModelEditor", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_ModelEditor::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_0 = {"closeEvent", 1, param_slot_0 };
    static const QUMethod slot_1 = {"simulate_component", 0, 0 };
    static const QUMethod slot_2 = {"update_component", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"select_component", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"select_component", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_conc", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_sed", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_diff", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_koff", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_keq", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_sigma", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_delta", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"update_mw", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_extinction", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_f_f0", 1, param_slot_14 };
    static const QUMethod slot_15 = {"select_rod", 0, 0 };
    static const QUMethod slot_16 = {"select_prolate", 0, 0 };
    static const QUMethod slot_17 = {"select_oblate", 0, 0 };
    static const QUMethod slot_18 = {"select_sphere", 0, 0 };
    static const QUMethod slot_19 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_20 = {"help", 0, 0 };
    static const QUMethod slot_21 = {"load_c0", 0, 0 };
    static const QUMethod slot_22 = {"update_sD", 0, 0 };
    static const QUMethod slot_23 = {"load_model", 0, 0 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_24 = {"get_vbar", 1, param_slot_24 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"get_f_f0", 1, param_slot_25 };
    static const QUMethod slot_26 = {"get_vbar", 0, 0 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_27 = {"printError", 1, param_slot_27 };
    static const QUMethod slot_28 = {"save_model", 0, 0 };
    static const QUMethod slot_29 = {"accept_model", 0, 0 };
    static const QUParameter param_slot_30[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_30 = {"update_vbar", 2, param_slot_30 };
    static const QUParameter param_slot_31[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_31 = {"verify_model", 1, param_slot_31 };
    static const QMetaData slot_tbl[] = {
	{ "closeEvent(QCloseEvent*)", &slot_0, QMetaData::Protected },
	{ "simulate_component()", &slot_1, QMetaData::Public },
	{ "update_component()", &slot_2, QMetaData::Public },
	{ "select_component(double)", &slot_3, QMetaData::Public },
	{ "select_component(int)", &slot_4, QMetaData::Public },
	{ "update_conc(const QString&)", &slot_5, QMetaData::Public },
	{ "update_sed(const QString&)", &slot_6, QMetaData::Public },
	{ "update_diff(const QString&)", &slot_7, QMetaData::Public },
	{ "update_koff(const QString&)", &slot_8, QMetaData::Public },
	{ "update_keq(const QString&)", &slot_9, QMetaData::Public },
	{ "update_sigma(const QString&)", &slot_10, QMetaData::Public },
	{ "update_delta(const QString&)", &slot_11, QMetaData::Public },
	{ "update_mw(const QString&)", &slot_12, QMetaData::Public },
	{ "update_extinction(const QString&)", &slot_13, QMetaData::Public },
	{ "update_f_f0(const QString&)", &slot_14, QMetaData::Public },
	{ "select_rod()", &slot_15, QMetaData::Public },
	{ "select_prolate()", &slot_16, QMetaData::Public },
	{ "select_oblate()", &slot_17, QMetaData::Public },
	{ "select_sphere()", &slot_18, QMetaData::Public },
	{ "setup_GUI()", &slot_19, QMetaData::Public },
	{ "help()", &slot_20, QMetaData::Public },
	{ "load_c0()", &slot_21, QMetaData::Public },
	{ "update_sD()", &slot_22, QMetaData::Public },
	{ "load_model()", &slot_23, QMetaData::Public },
	{ "get_vbar(const QString&)", &slot_24, QMetaData::Public },
	{ "get_f_f0(const QString&)", &slot_25, QMetaData::Public },
	{ "get_vbar()", &slot_26, QMetaData::Public },
	{ "printError(const int&)", &slot_27, QMetaData::Public },
	{ "save_model()", &slot_28, QMetaData::Public },
	{ "accept_model()", &slot_29, QMetaData::Public },
	{ "update_vbar(float,float)", &slot_30, QMetaData::Public },
	{ "verify_model()", &slot_31, QMetaData::Public }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod signal_0 = {"componentChanged", 1, param_signal_0 };
    static const QUMethod signal_1 = {"modelLoaded", 0, 0 };
    static const QUParameter param_signal_2[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod signal_2 = {"current_assoc", 1, param_signal_2 };
    static const QMetaData signal_tbl[] = {
	{ "componentChanged(unsigned int)", &signal_0, QMetaData::Public },
	{ "modelLoaded()", &signal_1, QMetaData::Public },
	{ "current_assoc(unsigned int)", &signal_2, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_ModelEditor", parentObject,
	slot_tbl, 32,
	signal_tbl, 3,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_ModelEditor.setMetaObject( metaObj );
    return metaObj;
}

void* US_ModelEditor::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_ModelEditor" ) )
	return this;
    return QDialog::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL componentChanged
void US_ModelEditor::componentChanged( unsigned int t0 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[2];
    static_QUType_ptr.set(o+1,&t0);
    activate_signal( clist, o );
}

// SIGNAL modelLoaded
void US_ModelEditor::modelLoaded()
{
    activate_signal( staticMetaObject()->signalOffset() + 1 );
}

// SIGNAL current_assoc
void US_ModelEditor::current_assoc( unsigned int t0 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 2 );
    if ( !clist )
	return;
    QUObject o[2];
    static_QUType_ptr.set(o+1,&t0);
    activate_signal( clist, o );
}

bool US_ModelEditor::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 1: simulate_component(); break;
    case 2: update_component(); break;
    case 3: select_component((double)static_QUType_double.get(_o+1)); break;
    case 4: select_component((int)static_QUType_int.get(_o+1)); break;
    case 5: update_conc((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_sed((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_diff((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update_koff((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: update_keq((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: update_sigma((const QString&)static_QUType_QString.get(_o+1)); break;
    case 11: update_delta((const QString&)static_QUType_QString.get(_o+1)); break;
    case 12: update_mw((const QString&)static_QUType_QString.get(_o+1)); break;
    case 13: update_extinction((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: update_f_f0((const QString&)static_QUType_QString.get(_o+1)); break;
    case 15: select_rod(); break;
    case 16: select_prolate(); break;
    case 17: select_oblate(); break;
    case 18: select_sphere(); break;
    case 19: setup_GUI(); break;
    case 20: help(); break;
    case 21: load_c0(); break;
    case 22: update_sD(); break;
    case 23: load_model(); break;
    case 24: get_vbar((const QString&)static_QUType_QString.get(_o+1)); break;
    case 25: get_f_f0((const QString&)static_QUType_QString.get(_o+1)); break;
    case 26: get_vbar(); break;
    case 27: printError((const int&)static_QUType_int.get(_o+1)); break;
    case 28: save_model(); break;
    case 29: accept_model(); break;
    case 30: update_vbar((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 31: static_QUType_bool.set(_o,verify_model()); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_ModelEditor::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: componentChanged((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    case 1: modelLoaded(); break;
    case 2: current_assoc((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    default:
	return QDialog::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_ModelEditor::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_ModelEditor::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
