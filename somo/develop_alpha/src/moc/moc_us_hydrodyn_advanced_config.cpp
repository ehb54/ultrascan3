/****************************************************************************
** US_Hydrodyn_AdvancedConfig meta object code from reading C++ file 'us_hydrodyn_advanced_config.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_advanced_config.h"
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

const char *US_Hydrodyn_AdvancedConfig::className() const
{
    return "US_Hydrodyn_AdvancedConfig";
}

QMetaObject *US_Hydrodyn_AdvancedConfig::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_AdvancedConfig( "US_Hydrodyn_AdvancedConfig", &US_Hydrodyn_AdvancedConfig::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_AdvancedConfig::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_AdvancedConfig", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_AdvancedConfig::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_AdvancedConfig", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_AdvancedConfig::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUMethod slot_1 = {"set_auto_view_pdb", 0, 0 };
    static const QUMethod slot_2 = {"set_scroll_editor", 0, 0 };
    static const QUMethod slot_3 = {"set_auto_calc_somo", 0, 0 };
    static const QUMethod slot_4 = {"set_auto_show_hydro", 0, 0 };
    static const QUMethod slot_5 = {"set_pbr_broken_logic", 0, 0 };
    static const QUMethod slot_6 = {"set_use_sounds", 0, 0 };
    static const QUMethod slot_7 = {"set_expert_mode", 0, 0 };
    static const QUMethod slot_8 = {"set_experimental_threads", 0, 0 };
    static const QUMethod slot_9 = {"set_debug_1", 0, 0 };
    static const QUMethod slot_10 = {"set_debug_2", 0, 0 };
    static const QUMethod slot_11 = {"set_debug_3", 0, 0 };
    static const QUMethod slot_12 = {"set_debug_4", 0, 0 };
    static const QUMethod slot_13 = {"cancel", 0, 0 };
    static const QUMethod slot_14 = {"help", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_15 = {"closeEvent", 1, param_slot_15 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "set_auto_view_pdb()", &slot_1, QMetaData::Private },
	{ "set_scroll_editor()", &slot_2, QMetaData::Private },
	{ "set_auto_calc_somo()", &slot_3, QMetaData::Private },
	{ "set_auto_show_hydro()", &slot_4, QMetaData::Private },
	{ "set_pbr_broken_logic()", &slot_5, QMetaData::Private },
	{ "set_use_sounds()", &slot_6, QMetaData::Private },
	{ "set_expert_mode()", &slot_7, QMetaData::Private },
	{ "set_experimental_threads()", &slot_8, QMetaData::Private },
	{ "set_debug_1()", &slot_9, QMetaData::Private },
	{ "set_debug_2()", &slot_10, QMetaData::Private },
	{ "set_debug_3()", &slot_11, QMetaData::Private },
	{ "set_debug_4()", &slot_12, QMetaData::Private },
	{ "cancel()", &slot_13, QMetaData::Private },
	{ "help()", &slot_14, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_15, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_AdvancedConfig", parentObject,
	slot_tbl, 16,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_AdvancedConfig.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_AdvancedConfig::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_AdvancedConfig" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_AdvancedConfig::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: set_auto_view_pdb(); break;
    case 2: set_scroll_editor(); break;
    case 3: set_auto_calc_somo(); break;
    case 4: set_auto_show_hydro(); break;
    case 5: set_pbr_broken_logic(); break;
    case 6: set_use_sounds(); break;
    case 7: set_expert_mode(); break;
    case 8: set_experimental_threads(); break;
    case 9: set_debug_1(); break;
    case 10: set_debug_2(); break;
    case 11: set_debug_3(); break;
    case 12: set_debug_4(); break;
    case 13: cancel(); break;
    case 14: help(); break;
    case 15: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_AdvancedConfig::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_AdvancedConfig::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_AdvancedConfig::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
