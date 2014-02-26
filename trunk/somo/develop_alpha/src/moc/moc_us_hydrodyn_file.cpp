/****************************************************************************
** US_Hydrodyn_File meta object code from reading C++ file 'us_hydrodyn_file.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_file.h"
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

const char *US_Hydrodyn_File::className() const
{
    return "US_Hydrodyn_File";
}

QMetaObject *US_Hydrodyn_File::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_File( "US_Hydrodyn_File", &US_Hydrodyn_File::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_File::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_File", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_File::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_File", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_File::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_base", 1, param_slot_0 };
    static const QUMethod slot_1 = {"overwrite", 0, 0 };
    static const QUMethod slot_2 = {"auto_inc", 0, 0 };
    static const QUMethod slot_3 = {"try_again", 0, 0 };
    static const QUMethod slot_4 = {"help", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_5 = {"closeEvent", 1, param_slot_5 };
    static const QMetaData slot_tbl[] = {
	{ "update_base(const QString&)", &slot_0, QMetaData::Private },
	{ "overwrite()", &slot_1, QMetaData::Private },
	{ "auto_inc()", &slot_2, QMetaData::Private },
	{ "try_again()", &slot_3, QMetaData::Private },
	{ "help()", &slot_4, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_5, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_File", parentObject,
	slot_tbl, 6,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_File.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_File::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_File" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_File::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_base((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: overwrite(); break;
    case 2: auto_inc(); break;
    case 3: try_again(); break;
    case 4: help(); break;
    case 5: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_File::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_File::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_File::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
