/****************************************************************************
** Meta object code from reading C++ file 'us_hydrodyn_advanced_config.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_hydrodyn_advanced_config.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_hydrodyn_advanced_config.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Hydrodyn_AdvancedConfig[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      28,   27,   27,   27, 0x08,
      39,   27,   27,   27, 0x08,
      59,   27,   27,   27, 0x08,
      79,   27,   27,   27, 0x08,
     100,   27,   27,   27, 0x08,
     122,   27,   27,   27, 0x08,
     145,   27,   27,   27, 0x08,
     162,   27,   27,   27, 0x08,
     180,   27,   27,   27, 0x08,
     207,   27,   27,   27, 0x08,
     221,   27,   27,   27, 0x08,
     235,   27,   27,   27, 0x08,
     249,   27,   27,   27, 0x08,
     263,   27,   27,   27, 0x08,
     272,   27,   27,   27, 0x08,
     279,   27,   27,   27, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_US_Hydrodyn_AdvancedConfig[] = {
    "US_Hydrodyn_AdvancedConfig\0\0setupGUI()\0"
    "set_auto_view_pdb()\0set_scroll_editor()\0"
    "set_auto_calc_somo()\0set_auto_show_hydro()\0"
    "set_pbr_broken_logic()\0set_use_sounds()\0"
    "set_expert_mode()\0set_experimental_threads()\0"
    "set_debug_1()\0set_debug_2()\0set_debug_3()\0"
    "set_debug_4()\0cancel()\0help()\0"
    "closeEvent(QCloseEvent*)\0"
};

void US_Hydrodyn_AdvancedConfig::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Hydrodyn_AdvancedConfig *_t = static_cast<US_Hydrodyn_AdvancedConfig *>(_o);
        switch (_id) {
        case 0: _t->setupGUI(); break;
        case 1: _t->set_auto_view_pdb(); break;
        case 2: _t->set_scroll_editor(); break;
        case 3: _t->set_auto_calc_somo(); break;
        case 4: _t->set_auto_show_hydro(); break;
        case 5: _t->set_pbr_broken_logic(); break;
        case 6: _t->set_use_sounds(); break;
        case 7: _t->set_expert_mode(); break;
        case 8: _t->set_experimental_threads(); break;
        case 9: _t->set_debug_1(); break;
        case 10: _t->set_debug_2(); break;
        case 11: _t->set_debug_3(); break;
        case 12: _t->set_debug_4(); break;
        case 13: _t->cancel(); break;
        case 14: _t->help(); break;
        case 15: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Hydrodyn_AdvancedConfig::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Hydrodyn_AdvancedConfig::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Hydrodyn_AdvancedConfig,
      qt_meta_data_US_Hydrodyn_AdvancedConfig, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Hydrodyn_AdvancedConfig::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Hydrodyn_AdvancedConfig::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Hydrodyn_AdvancedConfig::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Hydrodyn_AdvancedConfig))
        return static_cast<void*>(const_cast< US_Hydrodyn_AdvancedConfig*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Hydrodyn_AdvancedConfig::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
