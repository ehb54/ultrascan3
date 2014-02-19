/****************************************************************************
** Meta object code from reading C++ file 'us_util.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_util.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_util.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Config[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      13,   11,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
      48,   10,   43,   10, 0x0a,
      55,   10,   10,   10, 0x0a,
      72,   10,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_US_Config[] = {
    "US_Config\0\0,\0errorMessage(QString,QString)\0"
    "bool\0read()\0setModelString()\0"
    "color_defaults()\0"
};

void US_Config::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Config *_t = static_cast<US_Config *>(_o);
        switch (_id) {
        case 0: _t->errorMessage((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: { bool _r = _t->read();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 2: _t->setModelString(); break;
        case 3: _t->color_defaults(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Config::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Config::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_US_Config,
      qt_meta_data_US_Config, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Config::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Config::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Config::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Config))
        return static_cast<void*>(const_cast< US_Config*>(this));
    return QObject::qt_metacast(_clname);
}

int US_Config::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void US_Config::errorMessage(QString _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_US_Help[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       9,    8,    8,    8, 0x0a,
      27,    8,    8,    8, 0x0a,
      51,    8,    8,    8, 0x0a,
      70,    8,    8,    8, 0x08,
      86,    8,    8,    8, 0x08,
     102,    8,    8,    8, 0x08,
     115,    8,    8,    8, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_US_Help[] = {
    "US_Help\0\0show_URL(QString)\0"
    "show_html_file(QString)\0show_help(QString)\0"
    "captureStdout()\0captureStderr()\0"
    "endProcess()\0openBrowser()\0"
};

void US_Help::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Help *_t = static_cast<US_Help *>(_o);
        switch (_id) {
        case 0: _t->show_URL((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->show_html_file((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->show_help((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->captureStdout(); break;
        case 4: _t->captureStderr(); break;
        case 5: _t->endProcess(); break;
        case 6: _t->openBrowser(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Help::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Help::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_US_Help,
      qt_meta_data_US_Help, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Help::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Help::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Help::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Help))
        return static_cast<void*>(const_cast< US_Help*>(this));
    return QWidget::qt_metacast(_clname);
}

int US_Help::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}
static const uint qt_meta_data_OneLiner[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
      31,    9,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_OneLiner[] = {
    "OneLiner\0\0textChanged(QString)\0"
    "update(QString)\0"
};

void OneLiner::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        OneLiner *_t = static_cast<OneLiner *>(_o);
        switch (_id) {
        case 0: _t->textChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->update((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData OneLiner::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject OneLiner::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_OneLiner,
      qt_meta_data_OneLiner, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &OneLiner::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *OneLiner::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *OneLiner::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OneLiner))
        return static_cast<void*>(const_cast< OneLiner*>(this));
    return QDialog::qt_metacast(_clname);
}

int OneLiner::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void OneLiner::textChanged(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_US_FitParameter[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x0a,
      34,   16,   16,   16, 0x0a,
      53,   16,   16,   16, 0x0a,
      72,   16,   16,   16, 0x0a,
      86,   16,   16,   16, 0x0a,
     102,   16,   16,   16, 0x0a,
     121,   16,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_US_FitParameter[] = {
    "US_FitParameter\0\0setEnabled(bool)\0"
    "updateValue(float)\0updateRange(float)\0"
    "setFloatRed()\0setFloatGreen()\0"
    "setFloatDisabled()\0setUnit(QString)\0"
};

void US_FitParameter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_FitParameter *_t = static_cast<US_FitParameter *>(_o);
        switch (_id) {
        case 0: _t->setEnabled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->updateValue((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 2: _t->updateRange((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 3: _t->setFloatRed(); break;
        case 4: _t->setFloatGreen(); break;
        case 5: _t->setFloatDisabled(); break;
        case 6: _t->setUnit((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_FitParameter::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_FitParameter::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_US_FitParameter,
      qt_meta_data_US_FitParameter, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_FitParameter::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_FitParameter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_FitParameter::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_FitParameter))
        return static_cast<void*>(const_cast< US_FitParameter*>(this));
    return QWidget::qt_metacast(_clname);
}

int US_FitParameter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
