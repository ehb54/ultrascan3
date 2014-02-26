/****************************************************************************
** US_AddAtom meta object code from reading C++ file 'us_hydrodyn_addatom.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_addatom.h"
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

const char *US_AddAtom::className() const
{
    return "US_AddAtom";
}

QMetaObject *US_AddAtom::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_AddAtom( "US_AddAtom", &US_AddAtom::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_AddAtom::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_AddAtom", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_AddAtom::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_AddAtom", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_AddAtom::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUMethod slot_0 = {"add", 0, 0 };
    static const QUMethod slot_1 = {"help", 0, 0 };
    static const QUMethod slot_2 = {"select_atom_file", 0, 0 };
    static const QUMethod slot_3 = {"select_hybrid_file", 0, 0 };
    static const QUMethod slot_4 = {"select_saxs_file", 0, 0 };
    static const QUMethod slot_5 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_name", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_excl_vol", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_hybridization_name", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"select_hybrid", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"select_atom", 1, param_slot_10 };
    static const QUMethod slot_11 = {"delete_atom", 0, 0 };
    static const QUMethod slot_12 = {"write_atom_file", 0, 0 };
    static const QUMethod slot_13 = {"sort_atoms", 0, 0 };
    static const QUMethod slot_14 = {"set_excl_vol", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_15 = {"closeEvent", 1, param_slot_15 };
    static const QMetaData slot_tbl[] = {
	{ "add()", &slot_0, QMetaData::Private },
	{ "help()", &slot_1, QMetaData::Private },
	{ "select_atom_file()", &slot_2, QMetaData::Private },
	{ "select_hybrid_file()", &slot_3, QMetaData::Private },
	{ "select_saxs_file()", &slot_4, QMetaData::Private },
	{ "setupGUI()", &slot_5, QMetaData::Private },
	{ "update_name(const QString&)", &slot_6, QMetaData::Private },
	{ "update_excl_vol(const QString&)", &slot_7, QMetaData::Private },
	{ "update_hybridization_name(const QString&)", &slot_8, QMetaData::Private },
	{ "select_hybrid(int)", &slot_9, QMetaData::Private },
	{ "select_atom(int)", &slot_10, QMetaData::Private },
	{ "delete_atom()", &slot_11, QMetaData::Private },
	{ "write_atom_file()", &slot_12, QMetaData::Private },
	{ "sort_atoms()", &slot_13, QMetaData::Private },
	{ "set_excl_vol()", &slot_14, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_15, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_AddAtom", parentObject,
	slot_tbl, 16,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_AddAtom.setMetaObject( metaObj );
    return metaObj;
}

void* US_AddAtom::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_AddAtom" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool US_AddAtom::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: add(); break;
    case 1: help(); break;
    case 2: select_atom_file(); break;
    case 3: select_hybrid_file(); break;
    case 4: select_saxs_file(); break;
    case 5: setupGUI(); break;
    case 6: update_name((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_excl_vol((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update_hybridization_name((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: select_hybrid((int)static_QUType_int.get(_o+1)); break;
    case 10: select_atom((int)static_QUType_int.get(_o+1)); break;
    case 11: delete_atom(); break;
    case 12: write_atom_file(); break;
    case 13: sort_atoms(); break;
    case 14: set_excl_vol(); break;
    case 15: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_AddAtom::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_AddAtom::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool US_AddAtom::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
