/****************************************************************************
** US_Editor meta object code from reading C++ file 'us_editor.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_editor.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <Q3Frame>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Editor::className() const
{
    return "US_Editor";
}

QMetaObject *US_Editor::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Editor( "US_Editor", &US_Editor::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Editor::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Editor", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Editor::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Editor", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Editor::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"newDoc", 0, 0 };
    static const QUMethod slot_1 = {"load", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ "fileName", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"load", 1, param_slot_2 };
    static const QUMethod slot_3 = {"save", 0, 0 };
    static const QUMethod slot_4 = {"saveAs", 0, 0 };
    static const QUMethod slot_5 = {"print", 0, 0 };
    static const QUMethod slot_6 = {"update_font", 0, 0 };
    static const QUMethod slot_7 = {"closeDoc", 0, 0 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"append", 1, param_slot_8 };
    static const QMetaData slot_tbl[] = {
	{ "newDoc()", &slot_0, QMetaData::Public },
	{ "load()", &slot_1, QMetaData::Public },
	{ "load(const QString&)", &slot_2, QMetaData::Public },
	{ "save()", &slot_3, QMetaData::Public },
	{ "saveAs()", &slot_4, QMetaData::Public },
	{ "print()", &slot_5, QMetaData::Public },
	{ "update_font()", &slot_6, QMetaData::Public },
	{ "closeDoc()", &slot_7, QMetaData::Public },
	{ "append(const QString&)", &slot_8, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Editor", parentObject,
	slot_tbl, 9,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Editor.setMetaObject( metaObj );
    return metaObj;
}

void* US_Editor::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Editor" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Editor::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: newDoc(); break;
    case 1: load(); break;
    case 2: load((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: save(); break;
    case 4: saveAs(); break;
    case 5: print(); break;
    case 6: update_font(); break;
    case 7: closeDoc(); break;
    case 8: append((const QString&)static_QUType_QString.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Editor::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Editor::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Editor::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *TextEdit::className() const
{
    return "TextEdit";
}

QMetaObject *TextEdit::metaObj = 0;
static QMetaObjectCleanUp cleanUp_TextEdit( "TextEdit", &TextEdit::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString TextEdit::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "TextEdit", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString TextEdit::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "TextEdit", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* TextEdit::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3MainWindow::staticMetaObject();
    static const QUMethod slot_0 = {"fileNew", 0, 0 };
    static const QUMethod slot_1 = {"fileOpen", 0, 0 };
    static const QUMethod slot_2 = {"fileSave", 0, 0 };
    static const QUMethod slot_3 = {"fileSaveAs", 0, 0 };
    static const QUMethod slot_4 = {"fileSaveDB", 0, 0 };
    static const QUMethod slot_5 = {"filePrint", 0, 0 };
    static const QUMethod slot_6 = {"fileClose", 0, 0 };
    static const QUMethod slot_7 = {"fileExit", 0, 0 };
    static const QUMethod slot_8 = {"editUndo", 0, 0 };
    static const QUMethod slot_9 = {"editRedo", 0, 0 };
    static const QUMethod slot_10 = {"editCut", 0, 0 };
    static const QUMethod slot_11 = {"editCopy", 0, 0 };
    static const QUMethod slot_12 = {"editPaste", 0, 0 };
    static const QUMethod slot_13 = {"textBold", 0, 0 };
    static const QUMethod slot_14 = {"textUnderline", 0, 0 };
    static const QUMethod slot_15 = {"textItalic", 0, 0 };
    static const QUParameter param_slot_16[] = {
	{ "f", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"textFamily", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ "p", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"textSize", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ "s", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"textStyle", 1, param_slot_18 };
    static const QUMethod slot_19 = {"textColor", 0, 0 };
    static const QUParameter param_slot_20[] = {
	{ "a", &static_QUType_ptr, "QAction", QUParameter::In }
    };
    static const QUMethod slot_20 = {"textAlign", 1, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ "f", &static_QUType_varptr, "\x05", QUParameter::In }
    };
    static const QUMethod slot_21 = {"fontChanged", 1, param_slot_21 };
    static const QUParameter param_slot_22[] = {
	{ "c", &static_QUType_varptr, "\x0a", QUParameter::In }
    };
    static const QUMethod slot_22 = {"colorChanged", 1, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ "a", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"alignmentChanged", 1, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_ptr, "QWidget", QUParameter::In }
    };
    static const QUMethod slot_24 = {"editorChanged", 1, param_slot_24 };
    static const QMetaData slot_tbl[] = {
	{ "fileNew()", &slot_0, QMetaData::Private },
	{ "fileOpen()", &slot_1, QMetaData::Private },
	{ "fileSave()", &slot_2, QMetaData::Private },
	{ "fileSaveAs()", &slot_3, QMetaData::Private },
	{ "fileSaveDB()", &slot_4, QMetaData::Private },
	{ "filePrint()", &slot_5, QMetaData::Private },
	{ "fileClose()", &slot_6, QMetaData::Private },
	{ "fileExit()", &slot_7, QMetaData::Private },
	{ "editUndo()", &slot_8, QMetaData::Private },
	{ "editRedo()", &slot_9, QMetaData::Private },
	{ "editCut()", &slot_10, QMetaData::Private },
	{ "editCopy()", &slot_11, QMetaData::Private },
	{ "editPaste()", &slot_12, QMetaData::Private },
	{ "textBold()", &slot_13, QMetaData::Private },
	{ "textUnderline()", &slot_14, QMetaData::Private },
	{ "textItalic()", &slot_15, QMetaData::Private },
	{ "textFamily(const QString&)", &slot_16, QMetaData::Private },
	{ "textSize(const QString&)", &slot_17, QMetaData::Private },
	{ "textStyle(int)", &slot_18, QMetaData::Private },
	{ "textColor()", &slot_19, QMetaData::Private },
	{ "textAlign(QAction*)", &slot_20, QMetaData::Private },
	{ "fontChanged(const QFont&)", &slot_21, QMetaData::Private },
	{ "colorChanged(const QColor&)", &slot_22, QMetaData::Private },
	{ "alignmentChanged(int)", &slot_23, QMetaData::Private },
	{ "editorChanged(QWidget*)", &slot_24, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"TextEdit", parentObject,
	slot_tbl, 25,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_TextEdit.setMetaObject( metaObj );
    return metaObj;
}

void* TextEdit::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "TextEdit" ) )
	return this;
    return Q3MainWindow::qt_cast( clname );
}

bool TextEdit::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: fileNew(); break;
    case 1: fileOpen(); break;
    case 2: fileSave(); break;
    case 3: fileSaveAs(); break;
    case 4: fileSaveDB(); break;
    case 5: filePrint(); break;
    case 6: fileClose(); break;
    case 7: fileExit(); break;
    case 8: editUndo(); break;
    case 9: editRedo(); break;
    case 10: editCut(); break;
    case 11: editCopy(); break;
    case 12: editPaste(); break;
    case 13: textBold(); break;
    case 14: textUnderline(); break;
    case 15: textItalic(); break;
    case 16: textFamily((const QString&)static_QUType_QString.get(_o+1)); break;
    case 17: textSize((const QString&)static_QUType_QString.get(_o+1)); break;
    case 18: textStyle((int)static_QUType_int.get(_o+1)); break;
    case 19: textColor(); break;
    case 20: textAlign((QAction*)static_QUType_ptr.get(_o+1)); break;
    case 21: fontChanged((const QFont&)*((const QFont*)static_QUType_ptr.get(_o+1))); break;
    case 22: colorChanged((const QColor&)*((const QColor*)static_QUType_ptr.get(_o+1))); break;
    case 23: alignmentChanged((int)static_QUType_int.get(_o+1)); break;
    case 24: editorChanged((QWidget*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3MainWindow::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool TextEdit::qt_emit( int _id, QUObject* _o )
{
    return Q3MainWindow::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool TextEdit::qt_property( int id, int f, QVariant* v)
{
    return Q3MainWindow::qt_property( id, f, v);
}

bool TextEdit::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
