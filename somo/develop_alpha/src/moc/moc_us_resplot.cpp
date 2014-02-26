/****************************************************************************
** US_ResidualPlot meta object code from reading C++ file 'us_resplot.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_resplot.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_ResidualPlot::className() const
{
    return "US_ResidualPlot";
}

QMetaObject *US_ResidualPlot::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_ResidualPlot( "US_ResidualPlot", &US_ResidualPlot::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_ResidualPlot::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ResidualPlot", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_ResidualPlot::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ResidualPlot", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_ResidualPlot::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_ptr, "struct mfem_data", QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"setData", 3, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_ptr, "double*", QUParameter::InOut },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_1 = {"setData", 5, param_slot_1 };
    static const QMetaData slot_tbl[] = {
	{ "setData(struct mfem_data*,int,int)", &slot_0, QMetaData::Public },
	{ "setData(double**,int,int,unsigned int,unsigned int)", &slot_1, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_ResidualPlot", parentObject,
	slot_tbl, 2,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_ResidualPlot.setMetaObject( metaObj );
    return metaObj;
}

void* US_ResidualPlot::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_ResidualPlot" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool US_ResidualPlot::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setData((struct mfem_data*)static_QUType_ptr.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3)); break;
    case 1: setData((double**)static_QUType_ptr.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+4))),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+5)))); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_ResidualPlot::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_ResidualPlot::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool US_ResidualPlot::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
