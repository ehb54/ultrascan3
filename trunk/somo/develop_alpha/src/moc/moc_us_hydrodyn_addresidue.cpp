/****************************************************************************
** US_AddResidue meta object code from reading C++ file 'us_hydrodyn_addresidue.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_addresidue.h"
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

const char *US_AddResidue::className() const
{
    return "US_AddResidue";
}

QMetaObject *US_AddResidue::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_AddResidue( "US_AddResidue", &US_AddResidue::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_AddResidue::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_AddResidue", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_AddResidue::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_AddResidue", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_AddResidue::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUMethod slot_0 = {"add", 0, 0 };
    static const QUMethod slot_1 = {"reset", 0, 0 };
    static const QUMethod slot_2 = {"help", 0, 0 };
    static const QUMethod slot_3 = {"select_atom_file", 0, 0 };
    static const QUMethod slot_4 = {"select_residue_file", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"read_residue_file", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"select_residue", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"select_r_atom", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"select_r_bead", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"select_bead_color", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"select_placing_method", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"select_type", 1, param_slot_11 };
    static const QUMethod slot_12 = {"select_beadatom", 0, 0 };
    static const QUMethod slot_13 = {"delete_residue", 0, 0 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_ptr, "struct residue", QUParameter::In }
    };
    static const QUMethod slot_14 = {"print_residue", 1, param_slot_14 };
    static const QUMethod slot_15 = {"setupGUI", 0, 0 };
    static const QUMethod slot_16 = {"accept_bead", 0, 0 };
    static const QUMethod slot_17 = {"accept_atom", 0, 0 };
    static const QUMethod slot_18 = {"accept_residue", 0, 0 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"update_name", 1, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"update_comment", 1, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_21 = {"update_molvol", 1, param_slot_21 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"update_vbar", 1, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"update_asa", 1, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_24 = {"update_bead_volume", 1, param_slot_24 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"update_atom_hydration", 1, param_slot_25 };
    static const QUParameter param_slot_26[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_26 = {"update_hydration", 1, param_slot_26 };
    static const QUParameter param_slot_27[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_27 = {"update_numatoms", 1, param_slot_27 };
    static const QUParameter param_slot_28[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_28 = {"update_numbeads", 1, param_slot_28 };
    static const QUParameter param_slot_29[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_29 = {"update_hybrid", 1, param_slot_29 };
    static const QUMethod slot_30 = {"set_positioning", 0, 0 };
    static const QUMethod slot_31 = {"set_hydration", 0, 0 };
    static const QUParameter param_slot_32[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_32 = {"set_chain", 1, param_slot_32 };
    static const QUMethod slot_33 = {"atom_continue", 0, 0 };
    static const QUMethod slot_34 = {"write_residue_file", 0, 0 };
    static const QUParameter param_slot_35[] = {
	{ 0, &static_QUType_ptr, "struct residue", QUParameter::In }
    };
    static const QUMethod slot_35 = {"calc_bead_mw", 1, param_slot_35 };
    static const QUParameter param_slot_36[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_36 = {"enable_area_1", 1, param_slot_36 };
    static const QUParameter param_slot_37[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_37 = {"enable_area_2", 1, param_slot_37 };
    static const QUParameter param_slot_38[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_38 = {"enable_area_3", 1, param_slot_38 };
    static const QUParameter param_slot_39[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_39 = {"closeEvent", 1, param_slot_39 };
    static const QMetaData slot_tbl[] = {
	{ "add()", &slot_0, QMetaData::Private },
	{ "reset()", &slot_1, QMetaData::Private },
	{ "help()", &slot_2, QMetaData::Private },
	{ "select_atom_file()", &slot_3, QMetaData::Private },
	{ "select_residue_file()", &slot_4, QMetaData::Private },
	{ "read_residue_file(const QString&)", &slot_5, QMetaData::Private },
	{ "select_residue(int)", &slot_6, QMetaData::Private },
	{ "select_r_atom(int)", &slot_7, QMetaData::Private },
	{ "select_r_bead(int)", &slot_8, QMetaData::Private },
	{ "select_bead_color(int)", &slot_9, QMetaData::Private },
	{ "select_placing_method(int)", &slot_10, QMetaData::Private },
	{ "select_type(int)", &slot_11, QMetaData::Private },
	{ "select_beadatom()", &slot_12, QMetaData::Private },
	{ "delete_residue()", &slot_13, QMetaData::Private },
	{ "print_residue(struct residue)", &slot_14, QMetaData::Private },
	{ "setupGUI()", &slot_15, QMetaData::Private },
	{ "accept_bead()", &slot_16, QMetaData::Private },
	{ "accept_atom()", &slot_17, QMetaData::Private },
	{ "accept_residue()", &slot_18, QMetaData::Private },
	{ "update_name(const QString&)", &slot_19, QMetaData::Private },
	{ "update_comment(const QString&)", &slot_20, QMetaData::Private },
	{ "update_molvol(const QString&)", &slot_21, QMetaData::Private },
	{ "update_vbar(const QString&)", &slot_22, QMetaData::Private },
	{ "update_asa(const QString&)", &slot_23, QMetaData::Private },
	{ "update_bead_volume(const QString&)", &slot_24, QMetaData::Private },
	{ "update_atom_hydration(double)", &slot_25, QMetaData::Private },
	{ "update_hydration(double)", &slot_26, QMetaData::Private },
	{ "update_numatoms(double)", &slot_27, QMetaData::Private },
	{ "update_numbeads(double)", &slot_28, QMetaData::Private },
	{ "update_hybrid(int)", &slot_29, QMetaData::Private },
	{ "set_positioning()", &slot_30, QMetaData::Private },
	{ "set_hydration()", &slot_31, QMetaData::Private },
	{ "set_chain(int)", &slot_32, QMetaData::Private },
	{ "atom_continue()", &slot_33, QMetaData::Private },
	{ "write_residue_file()", &slot_34, QMetaData::Private },
	{ "calc_bead_mw(struct residue*)", &slot_35, QMetaData::Private },
	{ "enable_area_1(bool)", &slot_36, QMetaData::Private },
	{ "enable_area_2(bool)", &slot_37, QMetaData::Private },
	{ "enable_area_3(bool)", &slot_38, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_39, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_AddResidue", parentObject,
	slot_tbl, 40,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_AddResidue.setMetaObject( metaObj );
    return metaObj;
}

void* US_AddResidue::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_AddResidue" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool US_AddResidue::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: add(); break;
    case 1: reset(); break;
    case 2: help(); break;
    case 3: select_atom_file(); break;
    case 4: select_residue_file(); break;
    case 5: read_residue_file((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: select_residue((int)static_QUType_int.get(_o+1)); break;
    case 7: select_r_atom((int)static_QUType_int.get(_o+1)); break;
    case 8: select_r_bead((int)static_QUType_int.get(_o+1)); break;
    case 9: select_bead_color((int)static_QUType_int.get(_o+1)); break;
    case 10: select_placing_method((int)static_QUType_int.get(_o+1)); break;
    case 11: select_type((int)static_QUType_int.get(_o+1)); break;
    case 12: select_beadatom(); break;
    case 13: delete_residue(); break;
    case 14: print_residue((struct residue)(*((struct residue*)static_QUType_ptr.get(_o+1)))); break;
    case 15: setupGUI(); break;
    case 16: accept_bead(); break;
    case 17: accept_atom(); break;
    case 18: accept_residue(); break;
    case 19: update_name((const QString&)static_QUType_QString.get(_o+1)); break;
    case 20: update_comment((const QString&)static_QUType_QString.get(_o+1)); break;
    case 21: update_molvol((const QString&)static_QUType_QString.get(_o+1)); break;
    case 22: update_vbar((const QString&)static_QUType_QString.get(_o+1)); break;
    case 23: update_asa((const QString&)static_QUType_QString.get(_o+1)); break;
    case 24: update_bead_volume((const QString&)static_QUType_QString.get(_o+1)); break;
    case 25: update_atom_hydration((double)static_QUType_double.get(_o+1)); break;
    case 26: update_hydration((double)static_QUType_double.get(_o+1)); break;
    case 27: update_numatoms((double)static_QUType_double.get(_o+1)); break;
    case 28: update_numbeads((double)static_QUType_double.get(_o+1)); break;
    case 29: update_hybrid((int)static_QUType_int.get(_o+1)); break;
    case 30: set_positioning(); break;
    case 31: set_hydration(); break;
    case 32: set_chain((int)static_QUType_int.get(_o+1)); break;
    case 33: atom_continue(); break;
    case 34: write_residue_file(); break;
    case 35: calc_bead_mw((struct residue*)static_QUType_ptr.get(_o+1)); break;
    case 36: enable_area_1((bool)static_QUType_bool.get(_o+1)); break;
    case 37: enable_area_2((bool)static_QUType_bool.get(_o+1)); break;
    case 38: enable_area_3((bool)static_QUType_bool.get(_o+1)); break;
    case 39: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_AddResidue::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_AddResidue::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool US_AddResidue::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
