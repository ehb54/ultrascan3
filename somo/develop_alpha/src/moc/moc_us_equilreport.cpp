/****************************************************************************
** US_Report_Equil meta object code from reading C++ file 'us_equilreport.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_equilreport.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Report_Equil::className() const
{
    return "US_Report_Equil";
}

QMetaObject *US_Report_Equil::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Report_Equil( "US_Report_Equil", &US_Report_Equil::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Report_Equil::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Report_Equil", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Report_Equil::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Report_Equil", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Report_Equil::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "data_control", &static_QUType_ptr, "Data_Control_W", QUParameter::In }
    };
    static const QUMethod slot_0 = {"generate", 1, param_slot_0 };
    static const QUMethod slot_1 = {"load", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"view", 1, param_slot_2 };
    static const QMetaData slot_tbl[] = {
	{ "generate(Data_Control_W*)", &slot_0, QMetaData::Public },
	{ "load()", &slot_1, QMetaData::Private },
	{ "view(QString)", &slot_2, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Report_Equil", parentObject,
	slot_tbl, 3,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Report_Equil.setMetaObject( metaObj );
    return metaObj;
}

void* US_Report_Equil::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Report_Equil" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool US_Report_Equil::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: generate((Data_Control_W*)static_QUType_ptr.get(_o+1)); break;
    case 1: load(); break;
    case 2: view((QString)static_QUType_QString.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Report_Equil::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Report_Equil::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool US_Report_Equil::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
