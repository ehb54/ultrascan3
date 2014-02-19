/****************************************************************************
** US_Spectrum meta object code from reading C++ file 'us_spectrum.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_spectrum.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3Frame>
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Spectrum::className() const
{
    return "US_Spectrum";
}

QMetaObject *US_Spectrum::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Spectrum( "US_Spectrum", &US_Spectrum::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Spectrum::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Spectrum", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Spectrum::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Spectrum", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Spectrum::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_0 = {"closeEvent", 1, param_slot_0 };
    static const QUMethod slot_1 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"printError", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_3 = {"select_basis", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"delete_basis", 1, param_slot_4 };
    static const QUMethod slot_5 = {"load_basis", 0, 0 };
    static const QUMethod slot_6 = {"load_target", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"edit_basis", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"edit_target", 1, param_slot_8 };
    static const QUMethod slot_9 = {"load_fit", 0, 0 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_ptr, "struct WavelengthProfile", QUParameter::InOut },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"load_gaussian_profile", 2, param_slot_10 };
    static const QUMethod slot_11 = {"reset_basis", 0, 0 };
    static const QUMethod slot_12 = {"fit", 0, 0 };
    static const QUMethod slot_13 = {"help", 0, 0 };
    static const QUMethod slot_14 = {"update_scale", 0, 0 };
    static const QUMethod slot_15 = {"delete_scan", 0, 0 };
    static const QUMethod slot_16 = {"save", 0, 0 };
    static const QUMethod slot_17 = {"extrapolate", 0, 0 };
    static const QUMethod slot_18 = {"overlap", 0, 0 };
    static const QUMethod slot_19 = {"difference", 0, 0 };
    static const QUMethod slot_20 = {"reset_edit_gui", 0, 0 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_ptr, "struct WavelengthProfile", QUParameter::InOut }
    };
    static const QUMethod slot_21 = {"find_amplitude", 1, param_slot_21 };
    static const QUMethod slot_22 = {"print_residuals", 0, 0 };
    static const QUMethod slot_23 = {"print_fit", 0, 0 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_ptr, "struct WavelengthProfile", QUParameter::InOut }
    };
    static const QUMethod slot_24 = {"print_profile", 1, param_slot_24 };
    static const QMetaData slot_tbl[] = {
	{ "closeEvent(QCloseEvent*)", &slot_0, QMetaData::Protected },
	{ "setup_GUI()", &slot_1, QMetaData::Private },
	{ "printError(const int&)", &slot_2, QMetaData::Private },
	{ "select_basis(const QMouseEvent&)", &slot_3, QMetaData::Private },
	{ "delete_basis(int)", &slot_4, QMetaData::Private },
	{ "load_basis()", &slot_5, QMetaData::Private },
	{ "load_target()", &slot_6, QMetaData::Private },
	{ "edit_basis(int)", &slot_7, QMetaData::Private },
	{ "edit_target(int)", &slot_8, QMetaData::Private },
	{ "load_fit()", &slot_9, QMetaData::Private },
	{ "load_gaussian_profile(struct WavelengthProfile&,const QString&)", &slot_10, QMetaData::Private },
	{ "reset_basis()", &slot_11, QMetaData::Private },
	{ "fit()", &slot_12, QMetaData::Private },
	{ "help()", &slot_13, QMetaData::Private },
	{ "update_scale()", &slot_14, QMetaData::Private },
	{ "delete_scan()", &slot_15, QMetaData::Private },
	{ "save()", &slot_16, QMetaData::Private },
	{ "extrapolate()", &slot_17, QMetaData::Private },
	{ "overlap()", &slot_18, QMetaData::Private },
	{ "difference()", &slot_19, QMetaData::Private },
	{ "reset_edit_gui()", &slot_20, QMetaData::Private },
	{ "find_amplitude(struct WavelengthProfile&)", &slot_21, QMetaData::Private },
	{ "print_residuals()", &slot_22, QMetaData::Private },
	{ "print_fit()", &slot_23, QMetaData::Private },
	{ "print_profile(struct WavelengthProfile&)", &slot_24, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Spectrum", parentObject,
	slot_tbl, 25,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Spectrum.setMetaObject( metaObj );
    return metaObj;
}

void* US_Spectrum::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Spectrum" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Spectrum::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 1: setup_GUI(); break;
    case 2: printError((const int&)static_QUType_int.get(_o+1)); break;
    case 3: select_basis((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 4: delete_basis((int)static_QUType_int.get(_o+1)); break;
    case 5: load_basis(); break;
    case 6: load_target(); break;
    case 7: edit_basis((int)static_QUType_int.get(_o+1)); break;
    case 8: edit_target((int)static_QUType_int.get(_o+1)); break;
    case 9: load_fit(); break;
    case 10: load_gaussian_profile((struct WavelengthProfile&)*((struct WavelengthProfile*)static_QUType_ptr.get(_o+1)),(const QString&)static_QUType_QString.get(_o+2)); break;
    case 11: reset_basis(); break;
    case 12: fit(); break;
    case 13: help(); break;
    case 14: update_scale(); break;
    case 15: delete_scan(); break;
    case 16: save(); break;
    case 17: extrapolate(); break;
    case 18: overlap(); break;
    case 19: difference(); break;
    case 20: reset_edit_gui(); break;
    case 21: find_amplitude((struct WavelengthProfile&)*((struct WavelengthProfile*)static_QUType_ptr.get(_o+1))); break;
    case 22: print_residuals(); break;
    case 23: print_fit(); break;
    case 24: print_profile((struct WavelengthProfile&)*((struct WavelengthProfile*)static_QUType_ptr.get(_o+1))); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Spectrum::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Spectrum::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Spectrum::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
