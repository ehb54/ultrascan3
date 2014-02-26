/****************************************************************************
** US_GAModelEditor meta object code from reading C++ file 'us_ga_model_editor.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_ga_model_editor.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_GAModelEditor::className() const
{
    return "US_GAModelEditor";
}

QMetaObject *US_GAModelEditor::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_GAModelEditor( "US_GAModelEditor", &US_GAModelEditor::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_GAModelEditor::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_GAModelEditor", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_GAModelEditor::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_GAModelEditor", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_GAModelEditor::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = US_ModelEditor::staticMetaObject();
    static const QUMethod slot_0 = {"load_constraints", 0, 0 };
    static const QUMethod slot_1 = {"save_constraints", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_constraints", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_3 = {"verify_constraints", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_radialGrid", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_timeGrid", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ "val", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_simpoints", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ "val", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_band_volume", 1, param_slot_7 };
    static const QUMethod slot_8 = {"select_model", 0, 0 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_ptr, "struct constraint", QUParameter::In }
    };
    static const QUMethod slot_9 = {"mw_constraintChanged", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_ptr, "struct constraint", QUParameter::In }
    };
    static const QUMethod slot_10 = {"f_f0_constraintChanged", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_ptr, "struct constraint", QUParameter::In }
    };
    static const QUMethod slot_11 = {"conc_constraintChanged", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_ptr, "struct constraint", QUParameter::In }
    };
    static const QUMethod slot_12 = {"keq_constraintChanged", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_ptr, "struct constraint", QUParameter::In }
    };
    static const QUMethod slot_13 = {"koff_constraintChanged", 1, param_slot_13 };
    static const QUMethod slot_14 = {"initialize_msc", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_current_assoc", 1, param_slot_15 };
    static const QUMethod slot_16 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_17 = {"help", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "load_constraints()", &slot_0, QMetaData::Private },
	{ "save_constraints()", &slot_1, QMetaData::Private },
	{ "update_constraints(unsigned int)", &slot_2, QMetaData::Private },
	{ "verify_constraints()", &slot_3, QMetaData::Private },
	{ "update_radialGrid(int)", &slot_4, QMetaData::Private },
	{ "update_timeGrid(int)", &slot_5, QMetaData::Private },
	{ "update_simpoints(double)", &slot_6, QMetaData::Private },
	{ "update_band_volume(double)", &slot_7, QMetaData::Private },
	{ "select_model()", &slot_8, QMetaData::Private },
	{ "mw_constraintChanged(struct constraint)", &slot_9, QMetaData::Private },
	{ "f_f0_constraintChanged(struct constraint)", &slot_10, QMetaData::Private },
	{ "conc_constraintChanged(struct constraint)", &slot_11, QMetaData::Private },
	{ "keq_constraintChanged(struct constraint)", &slot_12, QMetaData::Private },
	{ "koff_constraintChanged(struct constraint)", &slot_13, QMetaData::Private },
	{ "initialize_msc()", &slot_14, QMetaData::Private },
	{ "update_current_assoc(unsigned int)", &slot_15, QMetaData::Private },
	{ "setup_GUI()", &slot_16, QMetaData::Private },
	{ "help()", &slot_17, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_GAModelEditor", parentObject,
	slot_tbl, 18,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_GAModelEditor.setMetaObject( metaObj );
    return metaObj;
}

void* US_GAModelEditor::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_GAModelEditor" ) )
	return this;
    return US_ModelEditor::qt_cast( clname );
}

bool US_GAModelEditor::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: load_constraints(); break;
    case 1: save_constraints(); break;
    case 2: update_constraints((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    case 3: static_QUType_bool.set(_o,verify_constraints()); break;
    case 4: update_radialGrid((int)static_QUType_int.get(_o+1)); break;
    case 5: update_timeGrid((int)static_QUType_int.get(_o+1)); break;
    case 6: update_simpoints((double)static_QUType_double.get(_o+1)); break;
    case 7: update_band_volume((double)static_QUType_double.get(_o+1)); break;
    case 8: select_model(); break;
    case 9: mw_constraintChanged((struct constraint)(*((struct constraint*)static_QUType_ptr.get(_o+1)))); break;
    case 10: f_f0_constraintChanged((struct constraint)(*((struct constraint*)static_QUType_ptr.get(_o+1)))); break;
    case 11: conc_constraintChanged((struct constraint)(*((struct constraint*)static_QUType_ptr.get(_o+1)))); break;
    case 12: keq_constraintChanged((struct constraint)(*((struct constraint*)static_QUType_ptr.get(_o+1)))); break;
    case 13: koff_constraintChanged((struct constraint)(*((struct constraint*)static_QUType_ptr.get(_o+1)))); break;
    case 14: initialize_msc(); break;
    case 15: update_current_assoc((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1)))); break;
    case 16: setup_GUI(); break;
    case 17: help(); break;
    default:
	return US_ModelEditor::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_GAModelEditor::qt_emit( int _id, QUObject* _o )
{
    return US_ModelEditor::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_GAModelEditor::qt_property( int id, int f, QVariant* v)
{
    return US_ModelEditor::qt_property( id, f, v);
}

bool US_GAModelEditor::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
