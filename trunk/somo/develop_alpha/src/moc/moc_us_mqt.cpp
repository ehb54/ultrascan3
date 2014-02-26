/****************************************************************************
** mQLabel meta object code from reading C++ file 'us_mqt.h'
**
** Created: Sat Feb 15 10:20:33 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_mqt.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QLabel>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *mQLabel::className() const
{
    return "mQLabel";
}

QMetaObject *mQLabel::metaObj = 0;
static QMetaObjectCleanUp cleanUp_mQLabel( "mQLabel", &mQLabel::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString mQLabel::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "mQLabel", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString mQLabel::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "mQLabel", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* mQLabel::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QLabel::staticMetaObject();
    static const QUMethod signal_0 = {"pressed", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "pressed()", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"mQLabel", parentObject,
	0, 0,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_mQLabel.setMetaObject( metaObj );
    return metaObj;
}

void* mQLabel::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "mQLabel" ) )
	return this;
    return QLabel::qt_cast( clname );
}

// SIGNAL pressed
void mQLabel::pressed()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool mQLabel::qt_invoke( int _id, QUObject* _o )
{
    return QLabel::qt_invoke(_id,_o);
}

bool mQLabel::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: pressed(); break;
    default:
	return QLabel::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool mQLabel::qt_property( int id, int f, QVariant* v)
{
    return QLabel::qt_property( id, f, v);
}

bool mQLabel::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *mQLineEdit::className() const
{
    return "mQLineEdit";
}

QMetaObject *mQLineEdit::metaObj = 0;
static QMetaObjectCleanUp cleanUp_mQLineEdit( "mQLineEdit", &mQLineEdit::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString mQLineEdit::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "mQLineEdit", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString mQLineEdit::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "mQLineEdit", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* mQLineEdit::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QLineEdit::staticMetaObject();
    static const QUParameter param_signal_0[] = {
	{ "hasFocus", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"focussed", 1, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "focussed(bool)", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"mQLineEdit", parentObject,
	0, 0,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_mQLineEdit.setMetaObject( metaObj );
    return metaObj;
}

void* mQLineEdit::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "mQLineEdit" ) )
	return this;
    return QLineEdit::qt_cast( clname );
}

// SIGNAL focussed
void mQLineEdit::focussed( bool t0 )
{
    activate_signal_bool( staticMetaObject()->signalOffset() + 0, t0 );
}

bool mQLineEdit::qt_invoke( int _id, QUObject* _o )
{
    return QLineEdit::qt_invoke(_id,_o);
}

bool mQLineEdit::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: focussed((bool)static_QUType_bool.get(_o+1)); break;
    default:
	return QLineEdit::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool mQLineEdit::qt_property( int id, int f, QVariant* v)
{
    return QLineEdit::qt_property( id, f, v);
}

bool mQLineEdit::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *mQPushButton::className() const
{
    return "mQPushButton";
}

QMetaObject *mQPushButton::metaObj = 0;
static QMetaObjectCleanUp cleanUp_mQPushButton( "mQPushButton", &mQPushButton::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString mQPushButton::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "mQPushButton", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString mQPushButton::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "mQPushButton", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* mQPushButton::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QPushButton::staticMetaObject();
    static const QUMethod signal_0 = {"doubleClicked", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "doubleClicked()", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"mQPushButton", parentObject,
	0, 0,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_mQPushButton.setMetaObject( metaObj );
    return metaObj;
}

void* mQPushButton::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "mQPushButton" ) )
	return this;
    return QPushButton::qt_cast( clname );
}

// SIGNAL doubleClicked
void mQPushButton::doubleClicked()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool mQPushButton::qt_invoke( int _id, QUObject* _o )
{
    return QPushButton::qt_invoke(_id,_o);
}

bool mQPushButton::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: doubleClicked(); break;
    default:
	return QPushButton::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool mQPushButton::qt_property( int id, int f, QVariant* v)
{
    return QPushButton::qt_property( id, f, v);
}

bool mQPushButton::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
