/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_addresidue.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_addresidue.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_addresidue.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_AddResidue[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      40,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x08,
      21,   14,   14,   14, 0x08,
      29,   14,   14,   14, 0x08,
      36,   14,   14,   14, 0x08,
      55,   14,   14,   14, 0x08,
      77,   14,   14,   14, 0x08,
     104,   14,   14,   14, 0x08,
     124,   14,   14,   14, 0x08,
     143,   14,   14,   14, 0x08,
     162,   14,   14,   14, 0x08,
     185,   14,   14,   14, 0x08,
     212,   14,   14,   14, 0x08,
     229,   14,   14,   14, 0x08,
     247,   14,   14,   14, 0x08,
     264,   14,   14,   14, 0x08,
     287,   14,   14,   14, 0x08,
     298,   14,   14,   14, 0x08,
     312,   14,   14,   14, 0x08,
     326,   14,   14,   14, 0x08,
     343,   14,   14,   14, 0x08,
     364,   14,   14,   14, 0x08,
     388,   14,   14,   14, 0x08,
     411,   14,   14,   14, 0x08,
     432,   14,   14,   14, 0x08,
     452,   14,   14,   14, 0x08,
     480,   14,   14,   14, 0x08,
     510,   14,   14,   14, 0x08,
     535,   14,   14,   14, 0x08,
     559,   14,   14,   14, 0x08,
     583,   14,   14,   14, 0x08,
     602,   14,   14,   14, 0x08,
     620,   14,   14,   14, 0x08,
     636,   14,   14,   14, 0x08,
     651,   14,   14,   14, 0x08,
     667,   14,   14,   14, 0x08,
     688,   14,   14,   14, 0x08,
     711,   14,   14,   14, 0x08,
     731,   14,   14,   14, 0x08,
     751,   14,   14,   14, 0x08,
     771,   14,   14,   14, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_AddResidue[] = {
    "US_AddResidue\0\0add()\0reset()\0help()\0"
    "select_atom_file()\0select_residue_file()\0"
    "read_residue_file(QString)\0"
    "select_residue(int)\0select_r_atom(int)\0"
    "select_r_bead(int)\0select_bead_color(int)\0"
    "select_placing_method(int)\0select_type(int)\0"
    "select_beadatom()\0delete_residue()\0"
    "print_residue(residue)\0setupGUI()\0"
    "accept_bead()\0accept_atom()\0"
    "accept_residue()\0update_name(QString)\0"
    "update_comment(QString)\0update_molvol(QString)\0"
    "update_vbar(QString)\0update_asa(QString)\0"
    "update_bead_volume(QString)\0"
    "update_atom_hydration(double)\0"
    "update_hydration(double)\0"
    "update_numatoms(double)\0update_numbeads(double)\0"
    "update_hybrid(int)\0set_positioning()\0"
    "set_hydration()\0set_chain(int)\0"
    "atom_continue()\0write_residue_file()\0"
    "calc_bead_mw(residue*)\0enable_area_1(bool)\0"
    "enable_area_2(bool)\0enable_area_3(bool)\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_AddResidue::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_AddResidue *_t = static_cast<US_AddResidue *>(_o);
        switch (_id) {
        case 0: _t->add(); break;
        case 1: _t->reset(); break;
        case 2: _t->help(); break;
        case 3: _t->select_atom_file(); break;
        case 4: _t->select_residue_file(); break;
        case 5: _t->read_residue_file((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->select_residue((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->select_r_atom((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->select_r_bead((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->select_bead_color((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->select_placing_method((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->select_type((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->select_beadatom(); break;
        case 13: _t->delete_residue(); break;
        case 14: _t->print_residue((*reinterpret_cast< residue(*)>(_a[1]))); break;
        case 15: _t->setupGUI(); break;
        case 16: _t->accept_bead(); break;
        case 17: _t->accept_atom(); break;
        case 18: _t->accept_residue(); break;
        case 19: _t->update_name((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 20: _t->update_comment((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 21: _t->update_molvol((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 22: _t->update_vbar((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 23: _t->update_asa((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 24: _t->update_bead_volume((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 25: _t->update_atom_hydration((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 26: _t->update_hydration((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 27: _t->update_numatoms((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 28: _t->update_numbeads((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 29: _t->update_hybrid((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 30: _t->set_positioning(); break;
        case 31: _t->set_hydration(); break;
        case 32: _t->set_chain((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 33: _t->atom_continue(); break;
        case 34: _t->write_residue_file(); break;
        case 35: _t->calc_bead_mw((*reinterpret_cast< residue*(*)>(_a[1]))); break;
        case 36: _t->enable_area_1((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 37: _t->enable_area_2((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 38: _t->enable_area_3((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 39: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_AddResidue::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_AddResidue::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_US_AddResidue,
      qt_meta_data_US_AddResidue, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_AddResidue::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_AddResidue::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_AddResidue::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_AddResidue))
        return static_cast<void*>(const_cast< US_AddResidue*>(this));
    return QWidget::qt_metacast(_clname);
}

int US_AddResidue::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 40)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 40;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
