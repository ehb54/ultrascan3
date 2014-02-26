/****************************************************************************
** Mesh2MainWindowBase meta object code from reading C++ file 'mesh2mainwindowbase.h'
**
** Created: Wed Dec 4 19:14:43 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../3dplot/mesh2mainwindowbase.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *Mesh2MainWindowBase::className() const
{
    return "Mesh2MainWindowBase";
}

QMetaObject *Mesh2MainWindowBase::metaObj = 0;
static QMetaObjectCleanUp cleanUp_Mesh2MainWindowBase( "Mesh2MainWindowBase", &Mesh2MainWindowBase::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString Mesh2MainWindowBase::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "Mesh2MainWindowBase", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString Mesh2MainWindowBase::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "Mesh2MainWindowBase", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* Mesh2MainWindowBase::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3MainWindow::staticMetaObject();
    static const QUMethod slot_0 = {"languageChange", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "languageChange()", &slot_0, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"Mesh2MainWindowBase", parentObject,
	slot_tbl, 1,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_Mesh2MainWindowBase.setMetaObject( metaObj );
    return metaObj;
}

void* Mesh2MainWindowBase::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "Mesh2MainWindowBase" ) )
	return this;
    return Q3MainWindow::qt_cast( clname );
}

bool Mesh2MainWindowBase::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: languageChange(); break;
    default:
	return Q3MainWindow::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool Mesh2MainWindowBase::qt_emit( int _id, QUObject* _o )
{
    return Q3MainWindow::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool Mesh2MainWindowBase::qt_property( int id, int f, QVariant* v)
{
    return Q3MainWindow::qt_property( id, f, v);
}

bool Mesh2MainWindowBase::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
