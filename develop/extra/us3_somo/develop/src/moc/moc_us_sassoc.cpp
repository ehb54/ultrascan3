/****************************************************************************
** US_SassocLegend meta object code from reading C++ file 'us_sassoc.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_sassoc.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3Frame>
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_SassocLegend::className() const
{
    return "US_SassocLegend";
}

QMetaObject *US_SassocLegend::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_SassocLegend( "US_SassocLegend", &US_SassocLegend::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_SassocLegend::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_SassocLegend", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_SassocLegend::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_SassocLegend", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_SassocLegend::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"US_SassocLegend", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_SassocLegend.setMetaObject( metaObj );
    return metaObj;
}

void* US_SassocLegend::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_SassocLegend" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_SassocLegend::qt_invoke( int _id, QUObject* _o )
{
    return Q3Frame::qt_invoke(_id,_o);
}

bool US_SassocLegend::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_SassocLegend::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_SassocLegend::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *US_Sassoc::className() const
{
    return "US_Sassoc";
}

QMetaObject *US_Sassoc::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Sassoc( "US_Sassoc", &US_Sassoc::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Sassoc::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Sassoc", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Sassoc::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Sassoc", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Sassoc::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setup_GUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_1 = {"closeEvent", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_2 = {"mouseMoved", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_3 = {"mousePressed", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_4 = {"mouseReleased", 1, param_slot_4 };
    static const QUMethod slot_5 = {"print", 0, 0 };
    static const QUMethod slot_6 = {"recalc", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_7 = {"createHtmlDir", 1, param_slot_7 };
    static const QUMethod slot_8 = {"write_data", 0, 0 };
    static const QUMethod slot_9 = {"help", 0, 0 };
    static const QUMethod slot_10 = {"cancel", 0, 0 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_double, 0, QUParameter::Out },
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"monomer_root", 2, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_double, 0, QUParameter::Out },
	{ 0, &static_QUType_double, 0, QUParameter::In },
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"polynomial", 3, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_stoich1", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_stoich2", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"update_eq1", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"update_eq2", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"update_eq1Count", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"update_eq2Count", 1, param_slot_18 };
    static const QMetaData slot_tbl[] = {
	{ "setup_GUI()", &slot_0, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_1, QMetaData::Protected },
	{ "mouseMoved(const QMouseEvent&)", &slot_2, QMetaData::Protected },
	{ "mousePressed(const QMouseEvent&)", &slot_3, QMetaData::Protected },
	{ "mouseReleased(const QMouseEvent&)", &slot_4, QMetaData::Protected },
	{ "print()", &slot_5, QMetaData::Public },
	{ "recalc()", &slot_6, QMetaData::Public },
	{ "createHtmlDir()", &slot_7, QMetaData::Public },
	{ "write_data()", &slot_8, QMetaData::Public },
	{ "help()", &slot_9, QMetaData::Public },
	{ "cancel()", &slot_10, QMetaData::Public },
	{ "monomer_root(double)", &slot_11, QMetaData::Public },
	{ "polynomial(double,double)", &slot_12, QMetaData::Public },
	{ "update_stoich1(const QString&)", &slot_13, QMetaData::Public },
	{ "update_stoich2(const QString&)", &slot_14, QMetaData::Public },
	{ "update_eq1(const QString&)", &slot_15, QMetaData::Public },
	{ "update_eq2(const QString&)", &slot_16, QMetaData::Public },
	{ "update_eq1Count(double)", &slot_17, QMetaData::Public },
	{ "update_eq2Count(double)", &slot_18, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Sassoc", parentObject,
	slot_tbl, 19,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Sassoc.setMetaObject( metaObj );
    return metaObj;
}

void* US_Sassoc::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Sassoc" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Sassoc::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setup_GUI(); break;
    case 1: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 2: mouseMoved((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 3: mousePressed((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 4: mouseReleased((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 5: print(); break;
    case 6: recalc(); break;
    case 7: static_QUType_bool.set(_o,createHtmlDir()); break;
    case 8: write_data(); break;
    case 9: help(); break;
    case 10: cancel(); break;
    case 11: static_QUType_double.set(_o,monomer_root((double)static_QUType_double.get(_o+1))); break;
    case 12: static_QUType_double.set(_o,polynomial((double)static_QUType_double.get(_o+1),(double)static_QUType_double.get(_o+2))); break;
    case 13: update_stoich1((const QString&)static_QUType_QString.get(_o+1)); break;
    case 14: update_stoich2((const QString&)static_QUType_QString.get(_o+1)); break;
    case 15: update_eq1((const QString&)static_QUType_QString.get(_o+1)); break;
    case 16: update_eq2((const QString&)static_QUType_QString.get(_o+1)); break;
    case 17: update_eq1Count((double)static_QUType_double.get(_o+1)); break;
    case 18: update_eq2Count((double)static_QUType_double.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Sassoc::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Sassoc::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Sassoc::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
