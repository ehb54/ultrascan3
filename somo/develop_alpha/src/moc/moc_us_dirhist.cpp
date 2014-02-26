/****************************************************************************
** US_Dirhist meta object code from reading C++ file 'us_dirhist.h'
**
** Created: Sun Feb 16 10:02:49 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_dirhist.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Dirhist::className() const
{
    return "US_Dirhist";
}

QMetaObject *US_Dirhist::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Dirhist( "US_Dirhist", &US_Dirhist::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Dirhist::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Dirhist", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Dirhist::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Dirhist", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Dirhist::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"t_selectionChanged", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ "col", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"t_sort_column", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In },
	{ "button", &static_QUType_int, 0, QUParameter::In },
	{ "mousePos", &static_QUType_varptr, "\x0e", QUParameter::In }
    };
    static const QUMethod slot_2 = {"t_doubleClicked", 4, param_slot_2 };
    static const QUMethod slot_3 = {"del", 0, 0 };
    static const QUMethod slot_4 = {"ok", 0, 0 };
    static const QUMethod slot_5 = {"cancel", 0, 0 };
    static const QUMethod slot_6 = {"help", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_7 = {"closeEvent", 1, param_slot_7 };
    static const QMetaData slot_tbl[] = {
	{ "t_selectionChanged()", &slot_0, QMetaData::Private },
	{ "t_sort_column(int)", &slot_1, QMetaData::Private },
	{ "t_doubleClicked(int,int,int,const QPoint&)", &slot_2, QMetaData::Private },
	{ "del()", &slot_3, QMetaData::Private },
	{ "ok()", &slot_4, QMetaData::Private },
	{ "cancel()", &slot_5, QMetaData::Private },
	{ "help()", &slot_6, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_7, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Dirhist", parentObject,
	slot_tbl, 8,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Dirhist.setMetaObject( metaObj );
    return metaObj;
}

void* US_Dirhist::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Dirhist" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Dirhist::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: t_selectionChanged(); break;
    case 1: t_sort_column((int)static_QUType_int.get(_o+1)); break;
    case 2: t_doubleClicked((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(const QPoint&)*((const QPoint*)static_QUType_ptr.get(_o+4))); break;
    case 3: del(); break;
    case 4: ok(); break;
    case 5: cancel(); break;
    case 6: help(); break;
    case 7: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Dirhist::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Dirhist::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Dirhist::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
