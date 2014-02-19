/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_pdb_tool_renum.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_pdb_tool_renum.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_pdb_tool_renum.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_Pdb_Tool_Renum[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      28,   27,   27,   27, 0x08,
      54,   27,   27,   27, 0x08,
      83,   27,   27,   27, 0x08,
     109,   27,   27,   27, 0x08,
     135,   27,   27,   27, 0x08,
     168,   27,   27,   27, 0x08,
     201,   27,   27,   27, 0x08,
     230,   27,   27,   27, 0x08,
     259,   27,   27,   27, 0x08,
     295,   27,   27,   27, 0x08,
     331,   27,   27,   27, 0x08,
     362,   27,   27,   27, 0x08,
     394,   27,   27,   27, 0x08,
     401,   27,   27,   27, 0x08,
     410,   27,   27,   27, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_Pdb_Tool_Renum[] = {
    "US_Hydrodyn_Pdb_Tool_Renum\0\0"
    "update_inputfile(QString)\0"
    "update_usechainlist(QString)\0"
    "update_reseqatom(QString)\0"
    "update_startatom(QString)\0"
    "update_chainrestartatom(QString)\0"
    "update_modelrestartatom(QString)\0"
    "update_reseqresidue(QString)\0"
    "update_startresidue(QString)\0"
    "update_chainrestartresidue(QString)\0"
    "update_modelrestartresidue(QString)\0"
    "update_striphydrogens(QString)\0"
    "update_itassertemplate(QString)\0help()\0"
    "cancel()\0closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_Pdb_Tool_Renum::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_Pdb_Tool_Renum *_t = static_cast<US_Hydrodyn_Pdb_Tool_Renum *>(_o);
        switch (_id) {
        case 0: _t->update_inputfile((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->update_usechainlist((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->update_reseqatom((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->update_startatom((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->update_chainrestartatom((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->update_modelrestartatom((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->update_reseqresidue((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->update_startresidue((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->update_chainrestartresidue((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->update_modelrestartresidue((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->update_striphydrogens((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: _t->update_itassertemplate((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 12: _t->help(); break;
        case 13: _t->cancel(); break;
        case 14: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_Pdb_Tool_Renum::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_Pdb_Tool_Renum::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_Pdb_Tool_Renum,
      qt_meta_data_US_Hydrodyn_Pdb_Tool_Renum, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_Pdb_Tool_Renum::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_Pdb_Tool_Renum::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_Pdb_Tool_Renum::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_Pdb_Tool_Renum))
        return static_cast<void*>(const_cast< US_Hydrodyn_Pdb_Tool_Renum*>(this));
    return QDialog::qt_metacast(_clname);
}

int US_Hydrodyn_Pdb_Tool_Renum::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
