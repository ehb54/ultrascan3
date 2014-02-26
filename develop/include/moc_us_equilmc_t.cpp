/****************************************************************************
** US_EquilMC_T meta object code from reading C++ file 'us_equilmc_t.h'
**
** Created: Wed Dec 4 19:31:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "us_equilmc_t.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <Q3Frame>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_EquilMC_T::className() const
{
    return "US_EquilMC_T";
}

QMetaObject *US_EquilMC_T::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_EquilMC_T( "US_EquilMC_T", &US_EquilMC_T::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_EquilMC_T::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_EquilMC_T", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_EquilMC_T::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_EquilMC_T", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_EquilMC_T::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"load", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ 0, &static_QUType_ptr, "unsigned int", QUParameter::In }
    };
    static const QUMethod slot_1 = {"assign_scanfit", 4, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_projectName", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out }
    };
    static const QUMethod slot_3 = {"createHtmlDir", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_limit", 1, param_slot_4 };
    static const QUMethod slot_5 = {"fitcontrol", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"loadfit", 1, param_slot_6 };
    static const QUMethod slot_7 = {"setup_vectors", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "load(const QString&)", &slot_0, QMetaData::Private },
	{ "assign_scanfit(unsigned int,unsigned int,unsigned int)", &slot_1, QMetaData::Private },
	{ "update_projectName(const QString&)", &slot_2, QMetaData::Private },
	{ "createHtmlDir()", &slot_3, QMetaData::Private },
	{ "update_limit(float)", &slot_4, QMetaData::Private },
	{ "fitcontrol()", &slot_5, QMetaData::Private },
	{ "loadfit(const QString&)", &slot_6, QMetaData::Private },
	{ "setup_vectors()", &slot_7, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_EquilMC_T", parentObject,
	slot_tbl, 8,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_EquilMC_T.setMetaObject( metaObj );
    return metaObj;
}

void* US_EquilMC_T::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_EquilMC_T" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_EquilMC_T::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: load((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: static_QUType_int.set(_o,assign_scanfit((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1))),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+2))),(unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+3))))); break;
    case 2: update_projectName((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: static_QUType_bool.set(_o,createHtmlDir()); break;
    case 4: update_limit((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 5: fitcontrol(); break;
    case 6: loadfit((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: setup_vectors(); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_EquilMC_T::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_EquilMC_T::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_EquilMC_T::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
