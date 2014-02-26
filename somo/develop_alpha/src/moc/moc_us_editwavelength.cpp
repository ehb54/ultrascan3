/****************************************************************************
** US_EditWavelengthScan meta object code from reading C++ file 'us_editwavelength.h'
**
** Created: Wed Dec 4 19:14:44 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_editwavelength.h"
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

const char *US_EditWavelengthScan::className() const
{
    return "US_EditWavelengthScan";
}

QMetaObject *US_EditWavelengthScan::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_EditWavelengthScan( "US_EditWavelengthScan", &US_EditWavelengthScan::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_EditWavelengthScan::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_EditWavelengthScan", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_EditWavelengthScan::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_EditWavelengthScan", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_EditWavelengthScan::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_0 = {"closeEvent", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_1 = {"mouseMoved", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_2 = {"mouseReleased", 1, param_slot_2 };
    static const QUMethod slot_3 = {"resetScan", 0, 0 };
    static const QUMethod slot_4 = {"resetList", 0, 0 };
    static const QUMethod slot_5 = {"plot", 0, 0 };
    static const QUMethod slot_6 = {"help", 0, 0 };
    static const QUMethod slot_7 = {"selectScans", 0, 0 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"showScan", 1, param_slot_8 };
    static const QUMethod slot_9 = {"saveScan", 0, 0 };
    static const QUMethod slot_10 = {"selectTarget", 0, 0 };
    static const QUMethod slot_11 = {"nextScan", 0, 0 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"deleteScan", 1, param_slot_12 };
    static const QUMethod slot_13 = {"setupGUI", 0, 0 };
    static const QUMethod slot_14 = {"selectCopy", 0, 0 };
    static const QUMethod slot_15 = {"selectOverwrite", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "closeEvent(QCloseEvent*)", &slot_0, QMetaData::Protected },
	{ "mouseMoved(const QMouseEvent&)", &slot_1, QMetaData::Protected },
	{ "mouseReleased(const QMouseEvent&)", &slot_2, QMetaData::Protected },
	{ "resetScan()", &slot_3, QMetaData::Public },
	{ "resetList()", &slot_4, QMetaData::Public },
	{ "plot()", &slot_5, QMetaData::Public },
	{ "help()", &slot_6, QMetaData::Public },
	{ "selectScans()", &slot_7, QMetaData::Public },
	{ "showScan(int)", &slot_8, QMetaData::Public },
	{ "saveScan()", &slot_9, QMetaData::Public },
	{ "selectTarget()", &slot_10, QMetaData::Public },
	{ "nextScan()", &slot_11, QMetaData::Public },
	{ "deleteScan(int)", &slot_12, QMetaData::Public },
	{ "setupGUI()", &slot_13, QMetaData::Public },
	{ "selectCopy()", &slot_14, QMetaData::Public },
	{ "selectOverwrite()", &slot_15, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_EditWavelengthScan", parentObject,
	slot_tbl, 16,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_EditWavelengthScan.setMetaObject( metaObj );
    return metaObj;
}

void* US_EditWavelengthScan::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_EditWavelengthScan" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_EditWavelengthScan::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 1: mouseMoved((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 2: mouseReleased((const QMouseEvent&)*((const QMouseEvent*)static_QUType_ptr.get(_o+1))); break;
    case 3: resetScan(); break;
    case 4: resetList(); break;
    case 5: plot(); break;
    case 6: help(); break;
    case 7: selectScans(); break;
    case 8: showScan((int)static_QUType_int.get(_o+1)); break;
    case 9: saveScan(); break;
    case 10: selectTarget(); break;
    case 11: nextScan(); break;
    case 12: deleteScan((int)static_QUType_int.get(_o+1)); break;
    case 13: setupGUI(); break;
    case 14: selectCopy(); break;
    case 15: selectOverwrite(); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_EditWavelengthScan::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_EditWavelengthScan::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_EditWavelengthScan::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
