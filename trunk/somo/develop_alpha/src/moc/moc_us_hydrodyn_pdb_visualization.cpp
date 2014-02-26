/****************************************************************************
** US_Hydrodyn_PDB_Visualization meta object code from reading C++ file 'us_hydrodyn_pdb_visualization.h'
**
** Created: Wed Dec 4 19:14:45 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_pdb_visualization.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <Q3Frame>
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_Hydrodyn_PDB_Visualization::className() const
{
    return "US_Hydrodyn_PDB_Visualization";
}

QMetaObject *US_Hydrodyn_PDB_Visualization::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_PDB_Visualization( "US_Hydrodyn_PDB_Visualization", &US_Hydrodyn_PDB_Visualization::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_PDB_Visualization::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_PDB_Visualization", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_PDB_Visualization::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_PDB_Visualization", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_PDB_Visualization::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"setupGUI", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"select_option", 1, param_slot_1 };
    static const QUMethod slot_2 = {"select_filename", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_filename", 1, param_slot_3 };
    static const QUMethod slot_4 = {"cancel", 0, 0 };
    static const QUMethod slot_5 = {"help", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_6 = {"closeEvent", 1, param_slot_6 };
    static const QMetaData slot_tbl[] = {
	{ "setupGUI()", &slot_0, QMetaData::Private },
	{ "select_option(int)", &slot_1, QMetaData::Private },
	{ "select_filename()", &slot_2, QMetaData::Private },
	{ "update_filename(const QString&)", &slot_3, QMetaData::Private },
	{ "cancel()", &slot_4, QMetaData::Private },
	{ "help()", &slot_5, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_6, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_PDB_Visualization", parentObject,
	slot_tbl, 7,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_PDB_Visualization.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_PDB_Visualization::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_PDB_Visualization" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_Hydrodyn_PDB_Visualization::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setupGUI(); break;
    case 1: select_option((int)static_QUType_int.get(_o+1)); break;
    case 2: select_filename(); break;
    case 3: update_filename((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: cancel(); break;
    case 5: help(); break;
    case 6: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_PDB_Visualization::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_PDB_Visualization::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_Hydrodyn_PDB_Visualization::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
