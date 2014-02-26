/****************************************************************************
** US_PlotData meta object code from reading C++ file 'us_plotdata.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_plotdata.h"
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

const char *US_PlotData::className() const
{
    return "US_PlotData";
}

QMetaObject *US_PlotData::metaObj = 0;
static QMetaObjectCleanUp cleanUp_US_PlotData( "US_PlotData", &US_PlotData::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString US_PlotData::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_PlotData", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString US_PlotData::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "US_PlotData", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* US_PlotData::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod slot_0 = {"plot_raw", 0, 0 };
    static const QUMethod slot_1 = {"plot_raw_sub_ti", 0, 0 };
    static const QUMethod slot_2 = {"plot_raw_sub_ri", 0, 0 };
    static const QUMethod slot_3 = {"plot_sim", 0, 0 };
    static const QUMethod slot_4 = {"plot_sim_add_ti", 0, 0 };
    static const QUMethod slot_5 = {"plot_sim_add_ri", 0, 0 };
    static const QUMethod slot_6 = {"plot_resid", 0, 0 };
    static const QUMethod slot_7 = {"plot_ti", 0, 0 };
    static const QUMethod slot_8 = {"plot_ri", 0, 0 };
    static const QUMethod slot_9 = {"plot_random", 0, 0 };
    static const QUMethod slot_10 = {"plot_bitmap", 0, 0 };
    static const QUMethod slot_11 = {"setup_GUI", 0, 0 };
    static const QUMethod slot_12 = {"save", 0, 0 };
    static const QUMethod slot_13 = {"cancel", 0, 0 };
    static const QUMethod slot_14 = {"update_plot", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_15 = {"closeEvent", 1, param_slot_15 };
    static const QMetaData slot_tbl[] = {
	{ "plot_raw()", &slot_0, QMetaData::Private },
	{ "plot_raw_sub_ti()", &slot_1, QMetaData::Private },
	{ "plot_raw_sub_ri()", &slot_2, QMetaData::Private },
	{ "plot_sim()", &slot_3, QMetaData::Private },
	{ "plot_sim_add_ti()", &slot_4, QMetaData::Private },
	{ "plot_sim_add_ri()", &slot_5, QMetaData::Private },
	{ "plot_resid()", &slot_6, QMetaData::Private },
	{ "plot_ti()", &slot_7, QMetaData::Private },
	{ "plot_ri()", &slot_8, QMetaData::Private },
	{ "plot_random()", &slot_9, QMetaData::Private },
	{ "plot_bitmap()", &slot_10, QMetaData::Private },
	{ "setup_GUI()", &slot_11, QMetaData::Private },
	{ "save()", &slot_12, QMetaData::Private },
	{ "cancel()", &slot_13, QMetaData::Private },
	{ "update_plot()", &slot_14, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_15, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"US_PlotData", parentObject,
	slot_tbl, 16,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_US_PlotData.setMetaObject( metaObj );
    return metaObj;
}

void* US_PlotData::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "US_PlotData" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

bool US_PlotData::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: plot_raw(); break;
    case 1: plot_raw_sub_ti(); break;
    case 2: plot_raw_sub_ri(); break;
    case 3: plot_sim(); break;
    case 4: plot_sim_add_ti(); break;
    case 5: plot_sim_add_ri(); break;
    case 6: plot_resid(); break;
    case 7: plot_ti(); break;
    case 8: plot_ri(); break;
    case 9: plot_random(); break;
    case 10: plot_bitmap(); break;
    case 11: setup_GUI(); break;
    case 12: save(); break;
    case 13: cancel(); break;
    case 14: update_plot(); break;
    case 15: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Q3Frame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool US_PlotData::qt_emit( int _id, QUObject* _o )
{
    return Q3Frame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool US_PlotData::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool US_PlotData::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
