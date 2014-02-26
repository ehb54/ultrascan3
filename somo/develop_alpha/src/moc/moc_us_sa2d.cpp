/****************************************************************************
** US_SA2D_W meta object code from reading C++ file 'us_sa2d.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_sa2d.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *US_SA2D_W::className() const
{
    return "US_SA2D_W";
}

QMetaObject *US_SA2D_W::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_SA2D_W( "US_SA2D_W", &US_SA2D_W::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_SA2D_W::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_SA2D_W", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_SA2D_W::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_SA2D_W", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_SA2D_W::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Data_Control_W::staticMetaObject();
    static const QUMethod slot_0 = {"sa2d_GUI", 0, 0 };
    static const QUMethod slot_1 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_2 = {"enableButtons", 0, 0 };
    static const QUMethod slot_3 = {"update_distribution", 0, 0 };
    static const QUMethod slot_4 = {"control_window", 0, 0 };
    static const QUMethod slot_5 = {"addplots", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_ptr, "struct mfem_data", QUParameter::In }
    };
    static const QUMethod slot_6 = {"clear_data", 1, param_slot_6 };
    static const QUMethod slot_7 = {"calc_residuals", 0, 0 };
    static const QUMethod slot_8 = {"write_sa2d", 0, 0 };
    static const QUMethod slot_9 = {"write_res", 0, 0 };
    static const QUMethod slot_10 = {"calc_20W_distros", 0, 0 };
    static const QUMethod slot_11 = {"calc_points", 0, 0 };
    static const QUMethod slot_12 = {"fit", 0, 0 };
    static const QUMethod slot_13 = {"loadfit", 0, 0 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"loadfit", 1, param_slot_14 };
    static const QUMethod slot_15 = {"plot_graph", 0, 0 };
    static const QUMethod slot_16 = {"update_result_vector", 0, 0 };
    static const QUMethod slot_17 = {"view", 0, 0 };
    static const QUMethod slot_18 = {"help", 0, 0 };
    static const QUMethod slot_19 = {"save", 0, 0 };
    static const QUMethod slot_20 = {"second_plot", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "sa2d_GUI()", &slot_0, QMetaData::Private },
	{ "setup_GUI()", &slot_1, QMetaData::Private },
	{ "enableButtons()", &slot_2, QMetaData::Private },
	{ "update_distribution()", &slot_3, QMetaData::Private },
	{ "control_window()", &slot_4, QMetaData::Private },
	{ "addplots()", &slot_5, QMetaData::Private },
	{ "clear_data(struct mfem_data*)", &slot_6, QMetaData::Private },
	{ "calc_residuals()", &slot_7, QMetaData::Private },
	{ "write_sa2d()", &slot_8, QMetaData::Private },
	{ "write_res()", &slot_9, QMetaData::Private },
	{ "calc_20W_distros()", &slot_10, QMetaData::Private },
	{ "calc_points()", &slot_11, QMetaData::Private },
	{ "fit()", &slot_12, QMetaData::Private },
	{ "loadfit()", &slot_13, QMetaData::Private },
	{ "loadfit(QString)", &slot_14, QMetaData::Private },
	{ "plot_graph()", &slot_15, QMetaData::Private },
	{ "update_result_vector()", &slot_16, QMetaData::Private },
	{ "view()", &slot_17, QMetaData::Private },
	{ "help()", &slot_18, QMetaData::Private },
	{ "save()", &slot_19, QMetaData::Private },
	{ "second_plot()", &slot_20, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_SA2D_W", parentObject,
	slot_tbl, 21,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_SA2D_W.setMetaObject( metaObj );
    return metaObj;
}

void* US_SA2D_W::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_SA2D_W" ) )
	return this;
    return Data_Control_W::qt_cast( clname );
}

bool US_SA2D_W::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: sa2d_GUI(); break;
    case 1: setup_GUI(); break;
    case 2: enableButtons(); break;
    case 3: update_distribution(); break;
    case 4: control_window(); break;
    case 5: addplots(); break;
    case 6: clear_data((struct mfem_data*)static_QUType_ptr.get(_o+1)); break;
    case 7: calc_residuals(); break;
    case 8: write_sa2d(); break;
    case 9: write_res(); break;
    case 10: calc_20W_distros(); break;
    case 11: calc_points(); break;
    case 12: fit(); break;
    case 13: loadfit(); break;
    case 14: loadfit((QString)static_QUType_QString.get(_o+1)); break;
    case 15: plot_graph(); break;
    case 16: update_result_vector(); break;
    case 17: view(); break;
    case 18: help(); break;
    case 19: save(); break;
    case 20: second_plot(); break;
    default:
	return Data_Control_W::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_SA2D_W::qt_emit( int _id, QUObject* _o )
{
    return Data_Control_W::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_SA2D_W::qt_property( int id, int f, QVariant* v)
{
    return Data_Control_W::qt_property( id, f, v);
}

bool US_SA2D_W::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
