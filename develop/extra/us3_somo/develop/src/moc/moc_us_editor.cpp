/****************************************************************************
** Meta object code from reading C++ file 'us_editor.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/us_editor.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'us_editor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_US_Editor[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x0a,
      20,   10,   10,   10, 0x0a,
      36,   27,   10,   10, 0x0a,
      50,   10,   10,   10, 0x0a,
      57,   10,   10,   10, 0x0a,
      66,   10,   10,   10, 0x0a,
      74,   10,   10,   10, 0x0a,
      88,   10,   10,   10, 0x0a,
      99,   10,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_US_Editor[] = {
    "US_Editor\0\0newDoc()\0load()\0fileName\0"
    "load(QString)\0save()\0saveAs()\0print()\0"
    "update_font()\0closeDoc()\0append(QString)\0"
};

void US_Editor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        US_Editor *_t = static_cast<US_Editor *>(_o);
        switch (_id) {
        case 0: _t->newDoc(); break;
        case 1: _t->load(); break;
        case 2: _t->load((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->save(); break;
        case 4: _t->saveAs(); break;
        case 5: _t->print(); break;
        case 6: _t->update_font(); break;
        case 7: _t->closeDoc(); break;
        case 8: _t->append((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData US_Editor::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject US_Editor::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_US_Editor,
      qt_meta_data_US_Editor, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &US_Editor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *US_Editor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *US_Editor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_US_Editor))
        return static_cast<void*>(const_cast< US_Editor*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int US_Editor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}
static const uint qt_meta_data_TextEdit[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      25,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x08,
      20,    9,    9,    9, 0x08,
      31,    9,    9,    9, 0x08,
      42,    9,    9,    9, 0x08,
      55,    9,    9,    9, 0x08,
      68,    9,    9,    9, 0x08,
      80,    9,    9,    9, 0x08,
      92,    9,    9,    9, 0x08,
     103,    9,    9,    9, 0x08,
     114,    9,    9,    9, 0x08,
     125,    9,    9,    9, 0x08,
     135,    9,    9,    9, 0x08,
     146,    9,    9,    9, 0x08,
     158,    9,    9,    9, 0x08,
     169,    9,    9,    9, 0x08,
     185,    9,    9,    9, 0x08,
     200,  198,    9,    9, 0x08,
     222,  220,    9,    9, 0x08,
     242,  240,    9,    9, 0x08,
     257,    9,    9,    9, 0x08,
     271,  269,    9,    9, 0x08,
     291,  198,    9,    9, 0x08,
     312,  310,    9,    9, 0x08,
     333,  269,    9,    9, 0x08,
     355,    9,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_TextEdit[] = {
    "TextEdit\0\0fileNew()\0fileOpen()\0"
    "fileSave()\0fileSaveAs()\0fileSaveDB()\0"
    "filePrint()\0fileClose()\0fileExit()\0"
    "editUndo()\0editRedo()\0editCut()\0"
    "editCopy()\0editPaste()\0textBold()\0"
    "textUnderline()\0textItalic()\0f\0"
    "textFamily(QString)\0p\0textSize(QString)\0"
    "s\0textStyle(int)\0textColor()\0a\0"
    "textAlign(QAction*)\0fontChanged(QFont)\0"
    "c\0colorChanged(QColor)\0alignmentChanged(int)\0"
    "editorChanged(QWidget*)\0"
};

void TextEdit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        TextEdit *_t = static_cast<TextEdit *>(_o);
        switch (_id) {
        case 0: _t->fileNew(); break;
        case 1: _t->fileOpen(); break;
        case 2: _t->fileSave(); break;
        case 3: _t->fileSaveAs(); break;
        case 4: _t->fileSaveDB(); break;
        case 5: _t->filePrint(); break;
        case 6: _t->fileClose(); break;
        case 7: _t->fileExit(); break;
        case 8: _t->editUndo(); break;
        case 9: _t->editRedo(); break;
        case 10: _t->editCut(); break;
        case 11: _t->editCopy(); break;
        case 12: _t->editPaste(); break;
        case 13: _t->textBold(); break;
        case 14: _t->textUnderline(); break;
        case 15: _t->textItalic(); break;
        case 16: _t->textFamily((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 17: _t->textSize((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 18: _t->textStyle((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 19: _t->textColor(); break;
        case 20: _t->textAlign((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 21: _t->fontChanged((*reinterpret_cast< const QFont(*)>(_a[1]))); break;
        case 22: _t->colorChanged((*reinterpret_cast< const QColor(*)>(_a[1]))); break;
        case 23: _t->alignmentChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 24: _t->editorChanged((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData TextEdit::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject TextEdit::staticMetaObject = {
    { &Q3MainWindow::staticMetaObject, qt_meta_stringdata_TextEdit,
      qt_meta_data_TextEdit, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TextEdit::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TextEdit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TextEdit::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TextEdit))
        return static_cast<void*>(const_cast< TextEdit*>(this));
    return Q3MainWindow::qt_metacast(_clname);
}

int TextEdit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3MainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 25)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 25;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
