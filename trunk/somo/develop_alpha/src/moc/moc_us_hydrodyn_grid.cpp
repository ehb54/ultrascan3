/****************************************************************************
** US_Hydrodyn_Grid meta object code from reading C++ file 'us_hydrodyn_grid.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_grid.h"
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

const char *US_Hydrodyn_Grid::className() const
{
    return "US_Hydrodyn_Grid";
}

QMetaObject *US_Hydrodyn_Grid::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Grid( "US_Hydrodyn_Grid", &US_Hydrodyn_Grid::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Grid::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Grid", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Grid::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Grid", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Grid::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_cube_side", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"select_center", 1, param_slot_2 };
    static const QUMethod slot_3 = {"set_hydrate", 0, 0 };
    static const QUMethod slot_4 = {"set_tangency", 0, 0 };
    static const QUMethod slot_5 = {"set_enable_asa", 0, 0 };
    static const QUMethod slot_6 = {"set_cubic", 0, 0 };
    static const QUMethod slot_7 = {"set_create_nmr_bead_pdb", 0, 0 };
    static const QUMethod slot_8 = {"set_equalize_radii_constant_volume", 0, 0 };
    static const QUMethod slot_9 = {"overlaps", 0, 0 };
    static const QUMethod slot_10 = {"cancel", 0, 0 };
    static const QUMethod slot_11 = {"help", 0, 0 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_12 = {"closeEvent", 1, param_slot_12 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "update_cube_side(double)", &slot_1, QMetaData::Private },
	{ "select_center(int)", &slot_2, QMetaData::Private },
	{ "set_hydrate()", &slot_3, QMetaData::Private },
	{ "set_tangency()", &slot_4, QMetaData::Private },
	{ "set_enable_asa()", &slot_5, QMetaData::Private },
	{ "set_cubic()", &slot_6, QMetaData::Private },
	{ "set_create_nmr_bead_pdb()", &slot_7, QMetaData::Private },
	{ "set_equalize_radii_constant_volume()", &slot_8, QMetaData::Private },
	{ "overlaps()", &slot_9, QMetaData::Private },
	{ "cancel()", &slot_10, QMetaData::Private },
	{ "help()", &slot_11, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_12, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Grid", parentObject,
	slot_tbl, 13,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Grid.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Grid::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Grid" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_Grid::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: update_cube_side((double)static_QUType_double.get(_o+1)); break;
    case 2: select_center((int)static_QUType_int.get(_o+1)); break;
    case 3: set_hydrate(); break;
    case 4: set_tangency(); break;
    case 5: set_enable_asa(); break;
    case 6: set_cubic(); break;
    case 7: set_create_nmr_bead_pdb(); break;
    case 8: set_equalize_radii_constant_volume(); break;
    case 9: overlaps(); break;
    case 10: cancel(); break;
    case 11: help(); break;
    case 12: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Grid::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Grid::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_Grid::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
