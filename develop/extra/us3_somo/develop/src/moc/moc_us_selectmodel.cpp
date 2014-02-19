/****************************************************************************
** US_SelectModel meta object code from reading C++ file 'us_selectmodel.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_selectmodel.h"
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

const char *US_SelectModel::className() const
{
    return "US_SelectModel";
}

QMetaObject *US_SelectModel::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_SelectModel( "US_SelectModel", &US_SelectModel::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_SelectModel::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_SelectModel", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_SelectModel::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_SelectModel", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_SelectModel::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"cancel", 0, 0 };
    static const QUMethod slot_1 = {"help", 0, 0 };
    static const QUMethod slot_2 = {"select_model", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"select_model", 1, param_slot_3 };
    static const QUMethod slot_4 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_5 = {"closeEvent", 1, param_slot_5 };
    static const QMetaData slot_tbl[] = {
	{ "cancel()", &slot_0, QMetaData::Public },
	{ "help()", &slot_1, QMetaData::Public },
	{ "select_model()", &slot_2, QMetaData::Public },
	{ "select_model(int)", &slot_3, QMetaData::Public },
	{ "setup_GUI()", &slot_4, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_5, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_SelectModel", parentObject,
	slot_tbl, 6,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_SelectModel.setMetaObject( metaObj );
    return metaObj;
}

void* US_SelectModel::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_SelectModel" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_SelectModel::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: cancel(); break;
    case 1: help(); break;
    case 2: select_model(); break;
    case 3: select_model((int)static_QUType_int.get(_o+1)); break;
    case 4: setup_GUI(); break;
    case 5: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_SelectModel::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_SelectModel::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_SelectModel::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
