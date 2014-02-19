/****************************************************************************
** Extrapolation_Legend_F meta object code from reading C++ file 'us_vhwsim.h'
**
** Created: Wed Dec 4 19:14:46 2013
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../include/us_vhwsim.h"
#include <qmetaobject.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <Q3Frame>
#include <QCloseEvent>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *Extrapolation_Legend_F::className() const
{
    return "Extrapolation_Legend_F";
}

QMetaObject *Extrapolation_Legend_F::metaObj = 0;
static QMetaObjectCleanUp cleanUp_Extrapolation_Legend_F( "Extrapolation_Legend_F", &Extrapolation_Legend_F::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString Extrapolation_Legend_F::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "Extrapolation_Legend_F", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString Extrapolation_Legend_F::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "Extrapolation_Legend_F", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* Extrapolation_Legend_F::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod signal_0 = {"clicked", 0, 0 };
    static const QUParameter param_signal_1[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod signal_1 = {"valueChanged", 1, param_signal_1 };
    static const QMetaData signal_tbl[] = {
	{ "clicked()", &signal_0, QMetaData::Private },
	{ "valueChanged(double)", &signal_1, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"Extrapolation_Legend_F", parentObject,
	0, 0,
	signal_tbl, 2,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_Extrapolation_Legend_F.setMetaObject( metaObj );
    return metaObj;
}

void* Extrapolation_Legend_F::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "Extrapolation_Legend_F" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

// SIGNAL clicked
void Extrapolation_Legend_F::clicked()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

// SIGNAL valueChanged
void Extrapolation_Legend_F::valueChanged( double t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 1, t0 );
}

bool Extrapolation_Legend_F::qt_invoke( int _id, QUObject* _o )
{
    return Q3Frame::qt_invoke(_id,_o);
}

bool Extrapolation_Legend_F::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: clicked(); break;
    case 1: valueChanged((double)static_QUType_double.get(_o+1)); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool Extrapolation_Legend_F::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool Extrapolation_Legend_F::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *Distribution_Legend_F::className() const
{
    return "Distribution_Legend_F";
}

QMetaObject *Distribution_Legend_F::metaObj = 0;
static QMetaObjectCleanUp cleanUp_Distribution_Legend_F( "Distribution_Legend_F", &Distribution_Legend_F::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString Distribution_Legend_F::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "Distribution_Legend_F", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString Distribution_Legend_F::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "Distribution_Legend_F", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* Distribution_Legend_F::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Q3Frame::staticMetaObject();
    static const QUMethod signal_0 = {"clicked", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "clicked()", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"Distribution_Legend_F", parentObject,
	0, 0,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_Distribution_Legend_F.setMetaObject( metaObj );
    return metaObj;
}

void* Distribution_Legend_F::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "Distribution_Legend_F" ) )
	return this;
    return Q3Frame::qt_cast( clname );
}

// SIGNAL clicked
void Distribution_Legend_F::clicked()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool Distribution_Legend_F::qt_invoke( int _id, QUObject* _o )
{
    return Q3Frame::qt_invoke(_id,_o);
}

bool Distribution_Legend_F::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: clicked(); break;
    default:
	return Q3Frame::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool Distribution_Legend_F::qt_property( int id, int f, QVariant* v)
{
    return Q3Frame::qt_property( id, f, v);
}

bool Distribution_Legend_F::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *Distribution::className() const
{
    return "Distribution";
}

QMetaObject *Distribution::metaObj = 0;
static QMetaObjectCleanUp cleanUp_Distribution( "Distribution", &Distribution::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString Distribution::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "Distribution", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString Distribution::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "Distribution", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* Distribution::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "e", &static_QUType_ptr, "QResizeEvent", QUParameter::In }
    };
    static const QUMethod slot_0 = {"resizeEvent", 1, param_slot_0 };
    static const QUMethod slot_1 = {"expand", 0, 0 };
    static const QUMethod slot_2 = {"compress", 0, 0 };
    static const QUMethod slot_3 = {"close_dis", 0, 0 };
    static const QUMethod slot_4 = {"help", 0, 0 };
    static const QUMethod slot_5 = {"show_histogram", 0, 0 };
    static const QUMethod slot_6 = {"show_histogram_plot", 0, 0 };
    static const QUMethod slot_7 = {"update_pixmap", 0, 0 };
    static const QUMethod slot_8 = {"print_plot", 0, 0 };
    static const QUMethod slot_9 = {"show_distribution", 0, 0 };
    static const QUParameter param_slot_10[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_10 = {"closeEvent", 1, param_slot_10 };
    static const QUMethod slot_11 = {"hide_histogram", 0, 0 };
    static const QUMethod slot_12 = {"hide_envelope", 0, 0 };
    static const QUParameter param_slot_13[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"update_histogram", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"update_envelope", 1, param_slot_14 };
    static const QMetaData slot_tbl[] = {
	{ "resizeEvent(QResizeEvent*)", &slot_0, QMetaData::Protected },
	{ "expand()", &slot_1, QMetaData::Public },
	{ "compress()", &slot_2, QMetaData::Public },
	{ "close_dis()", &slot_3, QMetaData::Public },
	{ "help()", &slot_4, QMetaData::Public },
	{ "show_histogram()", &slot_5, QMetaData::Public },
	{ "show_histogram_plot()", &slot_6, QMetaData::Public },
	{ "update_pixmap()", &slot_7, QMetaData::Public },
	{ "print_plot()", &slot_8, QMetaData::Public },
	{ "show_distribution()", &slot_9, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_10, QMetaData::Public },
	{ "hide_histogram()", &slot_11, QMetaData::Public },
	{ "hide_envelope()", &slot_12, QMetaData::Public },
	{ "update_histogram(double)", &slot_13, QMetaData::Public },
	{ "update_envelope(double)", &slot_14, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"Distribution", parentObject,
	slot_tbl, 15,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_Distribution.setMetaObject( metaObj );
    return metaObj;
}

void* Distribution::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "Distribution" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool Distribution::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: resizeEvent((QResizeEvent*)static_QUType_ptr.get(_o+1)); break;
    case 1: expand(); break;
    case 2: compress(); break;
    case 3: close_dis(); break;
    case 4: help(); break;
    case 5: show_histogram(); break;
    case 6: show_histogram_plot(); break;
    case 7: update_pixmap(); break;
    case 8: print_plot(); break;
    case 9: show_distribution(); break;
    case 10: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 11: hide_histogram(); break;
    case 12: hide_envelope(); break;
    case 13: update_histogram((double)static_QUType_double.get(_o+1)); break;
    case 14: update_envelope((double)static_QUType_double.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool Distribution::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool Distribution::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool Distribution::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *vHW_Sim::className() const
{
    return "vHW_Sim";
}

QMetaObject *vHW_Sim::metaObj = 0;
static QMetaObjectCleanUp cleanUp_vHW_Sim( "vHW_Sim", &vHW_Sim::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString vHW_Sim::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "vHW_Sim", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString vHW_Sim::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "vHW_Sim", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* vHW_Sim::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "e", &static_QUType_ptr, "QResizeEvent", QUParameter::In }
    };
    static const QUMethod slot_0 = {"resizeEvent", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_1 = {"closeEvent", 1, param_slot_1 };
    static const QUMethod slot_2 = {"recalc", 0, 0 };
    static const QUMethod slot_3 = {"distrib", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"update_div", 1, param_slot_4 };
    static const QUMethod slot_5 = {"save_vhw", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_charstar, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"save_vhw", 1, param_slot_6 };
    static const QUMethod slot_7 = {"close_vhw", 0, 0 };
    static const QUMethod slot_8 = {"cleanup", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "resizeEvent(QResizeEvent*)", &slot_0, QMetaData::Protected },
	{ "closeEvent(QCloseEvent*)", &slot_1, QMetaData::Protected },
	{ "recalc()", &slot_2, QMetaData::Private },
	{ "distrib()", &slot_3, QMetaData::Private },
	{ "update_div(double)", &slot_4, QMetaData::Private },
	{ "save_vhw()", &slot_5, QMetaData::Private },
	{ "save_vhw(const char*)", &slot_6, QMetaData::Private },
	{ "close_vhw()", &slot_7, QMetaData::Private },
	{ "cleanup()", &slot_8, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"vHW_Sim", parentObject,
	slot_tbl, 9,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_vHW_Sim.setMetaObject( metaObj );
    return metaObj;
}

void* vHW_Sim::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "vHW_Sim" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool vHW_Sim::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: resizeEvent((QResizeEvent*)static_QUType_ptr.get(_o+1)); break;
    case 1: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 2: recalc(); break;
    case 3: distrib(); break;
    case 4: update_div((double)static_QUType_double.get(_o+1)); break;
    case 5: save_vhw(); break;
    case 6: save_vhw((const char*)static_QUType_charstar.get(_o+1)); break;
    case 7: close_vhw(); break;
    case 8: cleanup(); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool vHW_Sim::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool vHW_Sim::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool vHW_Sim::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
