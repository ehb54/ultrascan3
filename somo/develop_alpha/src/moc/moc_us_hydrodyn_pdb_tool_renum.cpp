/****************************************************************************
** US_Hydrodyn_Pdb_Tool_Renum meta object code from reading C++ file 'us_hydrodyn_pdb_tool_renum.h'
**
** Created: Tue Feb 25 10:28:32 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_hydrodyn_pdb_tool_renum.h"
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

const char *US_Hydrodyn_Pdb_Tool_Renum::className() const
{
    return "US_Hydrodyn_Pdb_Tool_Renum";
}

QMetaObject *US_Hydrodyn_Pdb_Tool_Renum::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_Hydrodyn_Pdb_Tool_Renum( "US_Hydrodyn_Pdb_Tool_Renum", &US_Hydrodyn_Pdb_Tool_Renum::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_Hydrodyn_Pdb_Tool_Renum::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Pdb_Tool_Renum", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_Hydrodyn_Pdb_Tool_Renum::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_Hydrodyn_Pdb_Tool_Renum", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_Hydrodyn_Pdb_Tool_Renum::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update_inputfile", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"update_usechainlist", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"update_reseqatom", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"update_startatom", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_chainrestartatom", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"update_modelrestartatom", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"update_reseqresidue", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"update_startresidue", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"update_chainrestartresidue", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"update_modelrestartresidue", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"update_striphydrogens", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"update_itassertemplate", 1, param_slot_11 };
    static const QUMethod slot_12 = {"help", 0, 0 };
    static const QUMethod slot_13 = {"cancel", 0, 0 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_14 = {"closeEvent", 1, param_slot_14 };
    static const QMetaData slot_tbl[] = {
	{ "update_inputfile(const QString&)", &slot_0, QMetaData::Private },
	{ "update_usechainlist(const QString&)", &slot_1, QMetaData::Private },
	{ "update_reseqatom(const QString&)", &slot_2, QMetaData::Private },
	{ "update_startatom(const QString&)", &slot_3, QMetaData::Private },
	{ "update_chainrestartatom(const QString&)", &slot_4, QMetaData::Private },
	{ "update_modelrestartatom(const QString&)", &slot_5, QMetaData::Private },
	{ "update_reseqresidue(const QString&)", &slot_6, QMetaData::Private },
	{ "update_startresidue(const QString&)", &slot_7, QMetaData::Private },
	{ "update_chainrestartresidue(const QString&)", &slot_8, QMetaData::Private },
	{ "update_modelrestartresidue(const QString&)", &slot_9, QMetaData::Private },
	{ "update_striphydrogens(const QString&)", &slot_10, QMetaData::Private },
	{ "update_itassertemplate(const QString&)", &slot_11, QMetaData::Private },
	{ "help()", &slot_12, QMetaData::Private },
	{ "cancel()", &slot_13, QMetaData::Private },
	{ "closeEvent(QCloseEvent*)", &slot_14, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_Hydrodyn_Pdb_Tool_Renum", parentObject,
	slot_tbl, 15,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_Hydrodyn_Pdb_Tool_Renum.setMetaObject( metaObj );
    return metaObj;
}

void* US_Hydrodyn_Pdb_Tool_Renum::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_Hydrodyn_Pdb_Tool_Renum" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool US_Hydrodyn_Pdb_Tool_Renum::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update_inputfile((const QString&)static_QUType_QString.get(_o+1)); break;
    case 1: update_usechainlist((const QString&)static_QUType_QString.get(_o+1)); break;
    case 2: update_reseqatom((const QString&)static_QUType_QString.get(_o+1)); break;
    case 3: update_startatom((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: update_chainrestartatom((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: update_modelrestartatom((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: update_reseqresidue((const QString&)static_QUType_QString.get(_o+1)); break;
    case 7: update_startresidue((const QString&)static_QUType_QString.get(_o+1)); break;
    case 8: update_chainrestartresidue((const QString&)static_QUType_QString.get(_o+1)); break;
    case 9: update_modelrestartresidue((const QString&)static_QUType_QString.get(_o+1)); break;
    case 10: update_striphydrogens((const QString&)static_QUType_QString.get(_o+1)); break;
    case 11: update_itassertemplate((const QString&)static_QUType_QString.get(_o+1)); break;
    case 12: help(); break;
    case 13: cancel(); break;
    case 14: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_Hydrodyn_Pdb_Tool_Renum::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_Hydrodyn_Pdb_Tool_Renum::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool US_Hydrodyn_Pdb_Tool_Renum::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
