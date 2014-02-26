/****************************************************************************
** US_ImageViewer meta object code from reading C++ file 'us_imgviewer.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_imgviewer.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_ImageViewer::className() const
{
    return "US_ImageViewer";
}

QMetaObject *US_ImageViewer::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_ImageViewer( "US_ImageViewer", &US_ImageViewer::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_ImageViewer::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ImageViewer", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_ImageViewer::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_ImageViewer", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_ImageViewer::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"openFile", 1, param_slot_0 };
    static const QUMethod slot_1 = {"to1Bit", 0, 0 };
    static const QUMethod slot_2 = {"to8Bit", 0, 0 };
    static const QUMethod slot_3 = {"to32Bit", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"toBitDepth", 1, param_slot_4 };
    static const QUMethod slot_5 = {"copy", 0, 0 };
    static const QUMethod slot_6 = {"paste", 0, 0 };
    static const QUMethod slot_7 = {"hFlip", 0, 0 };
    static const QUMethod slot_8 = {"vFlip", 0, 0 };
    static const QUMethod slot_9 = {"rot180", 0, 0 };
    static const QUMethod slot_10 = {"newWindow", 0, 0 };
    static const QUMethod slot_11 = {"openFile", 0, 0 };
    static const QUParameter param_slot_12[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"saveImage", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"savePixmap", 1, param_slot_13 };
    static const QUMethod slot_14 = {"giveHelp", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"doOption", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_ptr, "US_ImageViewer", QUParameter::In }
    };
    static const QUMethod slot_16 = {"copyFrom", 1, param_slot_16 };
    static const QMetaData slot_tbl[] = {
	{ "openFile(QString)", &slot_0, QMetaData::Public },
	{ "to1Bit()", &slot_1, QMetaData::Private },
	{ "to8Bit()", &slot_2, QMetaData::Private },
	{ "to32Bit()", &slot_3, QMetaData::Private },
	{ "toBitDepth(int)", &slot_4, QMetaData::Private },
	{ "copy()", &slot_5, QMetaData::Private },
	{ "paste()", &slot_6, QMetaData::Private },
	{ "hFlip()", &slot_7, QMetaData::Private },
	{ "vFlip()", &slot_8, QMetaData::Private },
	{ "rot180()", &slot_9, QMetaData::Private },
	{ "newWindow()", &slot_10, QMetaData::Private },
	{ "openFile()", &slot_11, QMetaData::Private },
	{ "saveImage(int)", &slot_12, QMetaData::Private },
	{ "savePixmap(int)", &slot_13, QMetaData::Private },
	{ "giveHelp()", &slot_14, QMetaData::Private },
	{ "doOption(int)", &slot_15, QMetaData::Private },
	{ "copyFrom(US_ImageViewer*)", &slot_16, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_ImageViewer", parentObject,
	slot_tbl, 17,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_ImageViewer.setMetaObject( metaObj );
    return metaObj;
}

void* US_ImageViewer::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_ImageViewer" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool US_ImageViewer::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: openFile((QString)static_QUType_QString.get(_o+1)); break;
    case 1: to1Bit(); break;
    case 2: to8Bit(); break;
    case 3: to32Bit(); break;
    case 4: toBitDepth((int)static_QUType_int.get(_o+1)); break;
    case 5: copy(); break;
    case 6: paste(); break;
    case 7: hFlip(); break;
    case 8: vFlip(); break;
    case 9: rot180(); break;
    case 10: newWindow(); break;
    case 11: openFile(); break;
    case 12: saveImage((int)static_QUType_int.get(_o+1)); break;
    case 13: savePixmap((int)static_QUType_int.get(_o+1)); break;
    case 14: giveHelp(); break;
    case 15: doOption((int)static_QUType_int.get(_o+1)); break;
    case 16: copyFrom((US_ImageViewer*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_ImageViewer::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_ImageViewer::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool US_ImageViewer::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
