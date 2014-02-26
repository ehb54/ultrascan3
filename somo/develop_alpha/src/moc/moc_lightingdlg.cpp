/****************************************************************************
** Plot meta object code from reading C++ file 'lightingdlg.h'
**
** Created: Wed Dec 4 19:14:43 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../3dplot/lightingdlg.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *Plot::className() const
{
    return "Plot";
}

QMetaObject *Plot::metaObj = 0;
static QMetaObjectCleanUp cleanUp_Plot( "Plot", &Plot::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString Plot::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "Plot", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString Plot::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "Plot", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* Plot::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = SPlot::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"Plot", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_Plot.setMetaObject( metaObj );
    return metaObj;
}

void* Plot::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "Plot" ) )
	return this;
    return SPlot::qt_cast( clname );
}

bool Plot::qt_invoke( int _id, QUObject* _o )
{
    return SPlot::qt_invoke(_id,_o);
}

bool Plot::qt_emit( int _id, QUObject* _o )
{
    return SPlot::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool Plot::qt_property( int id, int f, QVariant* v)
{
    return SPlot::qt_property( id, f, v);
}

bool Plot::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *LightingDlg::className() const
{
    return "LightingDlg";
}

QMetaObject *LightingDlg::metaObj = 0;
static QMetaObjectCleanUp cleanUp_LightingDlg( "LightingDlg", &LightingDlg::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString LightingDlg::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "LightingDlg", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString LightingDlg::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "LightingDlg", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* LightingDlg::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = lightingdlgbaseBase::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"setDistance", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"setEmission", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"setDiff", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"setSpec", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"setShin", 1, param_slot_4 };
    static const QUMethod slot_5 = {"reset", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ "x", &static_QUType_double, 0, QUParameter::In },
	{ "y", &static_QUType_double, 0, QUParameter::In },
	{ "z", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"setRotation", 3, param_slot_6 };
    static const QMetaData slot_tbl[] = {
	{ "setDistance(int)", &slot_0, QMetaData::Public },
	{ "setEmission(int)", &slot_1, QMetaData::Public },
	{ "setDiff(int)", &slot_2, QMetaData::Public },
	{ "setSpec(int)", &slot_3, QMetaData::Public },
	{ "setShin(int)", &slot_4, QMetaData::Public },
	{ "reset()", &slot_5, QMetaData::Public },
	{ "setRotation(double,double,double)", &slot_6, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"LightingDlg", parentObject,
	slot_tbl, 7,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_LightingDlg.setMetaObject( metaObj );
    return metaObj;
}

void* LightingDlg::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "LightingDlg" ) )
	return this;
    return lightingdlgbaseBase::qt_cast( clname );
}

bool LightingDlg::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setDistance((int)static_QUType_int.get(_o+1)); break;
    case 1: setEmission((int)static_QUType_int.get(_o+1)); break;
    case 2: setDiff((int)static_QUType_int.get(_o+1)); break;
    case 3: setSpec((int)static_QUType_int.get(_o+1)); break;
    case 4: setShin((int)static_QUType_int.get(_o+1)); break;
    case 5: reset(); break;
    case 6: setRotation((double)static_QUType_double.get(_o+1),(double)static_QUType_double.get(_o+2),(double)static_QUType_double.get(_o+3)); break;
    default:
	return lightingdlgbaseBase::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool LightingDlg::qt_emit( int _id, QUObject* _o )
{
    return lightingdlgbaseBase::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool LightingDlg::qt_property( int id, int f, QVariant* v)
{
    return lightingdlgbaseBase::qt_property( id, f, v);
}

bool LightingDlg::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
