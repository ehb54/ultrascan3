/****************************************************************************
** mQLabel meta object code from reading C++ file 'us_mqlabel.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_mqlabel.h"
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
