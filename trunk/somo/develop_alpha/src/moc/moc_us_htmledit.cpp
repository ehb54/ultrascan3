/****************************************************************************
** US_htmlEdit meta object code from reading C++ file 'us_htmledit.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_htmledit.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_htmlEdit::className() const
{
    return "US_htmlEdit";
}

QMetaObject *US_htmlEdit::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_htmlEdit( "US_htmlEdit", &US_htmlEdit::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_htmlEdit::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_htmlEdit", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_htmlEdit::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_htmlEdit", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_htmlEdit::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"cancel", 0, 0 };
    static const QUMethod slot_1 = {"save", 0, 0 };
    static const QUMethod slot_2 = {"help", 0, 0 };
    static const QUMethod slot_3 = {"quit", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_4 = {"closeEvent", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ "e", &static_QUType_ptr, "QResizeEvent", QUParameter::In }
    };
    static const QUMethod slot_5 = {"resizeEvent", 1, param_slot_5 };
    static const QMetaData slot_tbl[] = {
	{ "cancel()", &slot_0, QMetaData::Public },
	{ "save()", &slot_1, QMetaData::Public },
	{ "help()", &slot_2, QMetaData::Public },
	{ "quit()", &slot_3, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_4, QMetaData::Protected },
	{ "resizeEvent(QResizeEvent*)", &slot_5, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_htmlEdit", parentObject,
	slot_tbl, 6,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_htmlEdit.setMetaObject( metaObj );
    return metaObj;
}

void* US_htmlEdit::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_htmlEdit" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_htmlEdit::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: cancel(); break;
    case 1: save(); break;
    case 2: help(); break;
    case 3: quit(); break;
    case 4: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 5: resizeEvent((QResizeEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_htmlEdit::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_htmlEdit::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_htmlEdit::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
