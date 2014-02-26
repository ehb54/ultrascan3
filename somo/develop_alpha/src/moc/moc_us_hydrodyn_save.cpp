/****************************************************************************
** US_Hydrodyn_Save meta object code from reading C++ file 'us_hydrodyn_save.h'
**
** Created: Tue Feb 25 10:28:33 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_save.h"
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

const char *US_Hydrodyn_Save::className() const
{
    return "US_Hydrodyn_Save";
}

QMetaObject *US_Hydrodyn_Save::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Save( "US_Hydrodyn_Save", &US_Hydrodyn_Save::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Save::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Save", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Save::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Save", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Save::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"add", 0, 0 };
    static const QUMethod slot_2 = {"remove", 0, 0 };
    static const QUMethod slot_3 = {"rebuild", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_ptr, "QWidget", QUParameter::In }
    };
    static const QUMethod slot_4 = {"tab_changed", 1, param_slot_4 };
    static const QUMethod slot_5 = {"update_enables_selected", 0, 0 };
    static const QUMethod slot_6 = {"update_enables_possible", 0, 0 };
    static const QUMethod slot_7 = {"cancel", 0, 0 };
    static const QUMethod slot_8 = {"help", 0, 0 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_9 = {"closeEvent", 1, param_slot_9 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "add()", &slot_1, QMetaData::Private },
	{ "remove()", &slot_2, QMetaData::Private },
	{ "rebuild()", &slot_3, QMetaData::Private },
	{ "tab_changed(QWidget*)", &slot_4, QMetaData::Private },
	{ "update_enables_selected()", &slot_5, QMetaData::Private },
	{ "update_enables_possible()", &slot_6, QMetaData::Private },
	{ "cancel()", &slot_7, QMetaData::Private },
	{ "help()", &slot_8, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_9, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Save", parentObject,
	slot_tbl, 10,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Save.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Save::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Save" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Save::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: add(); break;
    case 2: remove(); break;
    case 3: rebuild(); break;
    case 4: tab_changed((QWidget*)static_QUType_ptr.get(_o+1)); break;
    case 5: update_enables_selected(); break;
    case 6: update_enables_possible(); break;
    case 7: cancel(); break;
    case 8: help(); break;
    case 9: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Save::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Save::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Save::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
