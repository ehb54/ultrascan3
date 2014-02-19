/****************************************************************************
** US_ExcludeProfile meta object code from reading C++ file 'us_exclude_profile.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_exclude_profile.h"
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

const char *US_ExcludeProfile::className() const
{
    return "US_ExcludeProfile";
}

QMetaObject *US_ExcludeProfile::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_ExcludeProfile( "US_ExcludeProfile", &US_ExcludeProfile::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_ExcludeProfile::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ExcludeProfile", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_ExcludeProfile::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ExcludeProfile", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_ExcludeProfile::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"cancel", 0, 0 };
    static const QUMethod slot_1 = {"accept", 0, 0 };
    static const QUMethod slot_2 = {"help", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_scanStart", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_scanStop", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_scanInclude", 1, param_slot_5 };
    static const QUMethod slot_6 = {"update_exclude_list", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_7 = {"closeEvent", 1, param_slot_7 };
    static const QMetaData slot_tbl[] = {
	{ "cancel()", &slot_0, QMetaData::Public },
	{ "accept()", &slot_1, QMetaData::Public },
	{ "help()", &slot_2, QMetaData::Public },
	{ "update_scanStart(double)", &slot_3, QMetaData::Public },
	{ "update_scanStop(double)", &slot_4, QMetaData::Public },
	{ "update_scanInclude(double)", &slot_5, QMetaData::Public },
	{ "update_exclude_list()", &slot_6, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_7, QMetaData::Protected }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_ptr, "vector<bool>", QUParameter::In }
    };
    static const QUMethod signal_0 = {"update_exclude_profile", 1, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ 0, &static_QUType_ptr, "vector<bool>", QUParameter::In }
    };
    static const QUMethod signal_1 = {"final_exclude_profile", 1, param_signal_1 };
    static const QMetaData signal_tbl[] = {
	{ "update_exclude_profile(vector<bool>)", &signal_0, QMetaData::Public },
	{ "final_exclude_profile(vector<bool>)", &signal_1, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_ExcludeProfile", parentObject,
	slot_tbl, 8,
	signal_tbl, 2,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_ExcludeProfile.setMetaObject( metaObj );
    return metaObj;
}

void* US_ExcludeProfile::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_ExcludeProfile" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL update_exclude_profile
void US_ExcludeProfile::update_exclude_profile( vector<bool> t0 )
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

// SIGNAL final_exclude_profile
void US_ExcludeProfile::final_exclude_profile( vector<bool> t0 )
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

bool US_ExcludeProfile::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: cancel(); break;
    case 1: accept(); break;
    case 2: help(); break;
    case 3: update_scanStart((double)static_QUType_double.get(_o+1)); break;
    case 4: update_scanStop((double)static_QUType_double.get(_o+1)); break;
    case 5: update_scanInclude((double)static_QUType_double.get(_o+1)); break;
    case 6: update_exclude_list(); break;
    case 7: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_ExcludeProfile::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: update_exclude_profile((vector<bool>)(*((vector<bool>*)static_QUType_ptr.get(_o+1)))); break;
    case 1: final_exclude_profile((vector<bool>)(*((vector<bool>*)static_QUType_ptr.get(_o+1)))); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool US_ExcludeProfile::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_ExcludeProfile::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
