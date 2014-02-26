/****************************************************************************
** Mesh2MainWindow meta object code from reading C++ file 'mesh2mainwindow.h'
**
** Created: Tue Feb 25 10:28:27 2014
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../3dplot/mesh2mainwindow.h"
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

const char *Mesh2MainWindow::className() const
{
    return "Mesh2MainWindow";
}

QMetaObject *Mesh2MainWindow::metaObj = 0;
static QMetaObjectCleanUp cleanUp_Mesh2MainWindow( "Mesh2MainWindow", &Mesh2MainWindow::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString Mesh2MainWindow::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "Mesh2MainWindow", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString Mesh2MainWindow::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "Mesh2MainWindow", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* Mesh2MainWindow::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = Mesh2MainWindowBase::staticMetaObject();
    static const QUMethod slot_0 = {"open", 0, 0 };
    static const QUMethod slot_1 = {"openMesh", 0, 0 };
    static const QUMethod slot_2 = {"createFunction", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ "name", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"setFileType", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_ptr, "QAction", QUParameter::In }
    };
    static const QUMethod slot_4 = {"pickCoordSystem", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_ptr, "QAction", QUParameter::In }
    };
    static const QUMethod slot_5 = {"pickPlotStyle", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_ptr, "QAction", QUParameter::In }
    };
    static const QUMethod slot_6 = {"pickFloorStyle", 1, param_slot_6 };
    static const QUMethod slot_7 = {"pickAxesColor", 0, 0 };
    static const QUMethod slot_8 = {"pickBgColor", 0, 0 };
    static const QUMethod slot_9 = {"pickMeshColor", 0, 0 };
    static const QUMethod slot_10 = {"pickNumberColor", 0, 0 };
    static const QUMethod slot_11 = {"pickLabelColor", 0, 0 };
    static const QUMethod slot_12 = {"pickTitleColor", 0, 0 };
    static const QUMethod slot_13 = {"pickDataColor", 0, 0 };
    static const QUMethod slot_14 = {"pickLighting", 0, 0 };
    static const QUMethod slot_15 = {"resetColors", 0, 0 };
    static const QUMethod slot_16 = {"pickNumberFont", 0, 0 };
    static const QUMethod slot_17 = {"pickLabelFont", 0, 0 };
    static const QUMethod slot_18 = {"pickTitleFont", 0, 0 };
    static const QUMethod slot_19 = {"resetFonts", 0, 0 };
    static const QUMethod slot_20 = {"setStandardView", 0, 0 };
    static const QUMethod slot_21 = {"dumpImage", 0, 0 };
    static const QUParameter param_slot_22[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"dumpImage", 2, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"toggleAnimation", 1, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_24 = {"toggleProjectionMode", 1, param_slot_24 };
    static const QUParameter param_slot_25[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"toggleColorLegend", 1, param_slot_25 };
    static const QUParameter param_slot_26[] = {
	{ "val", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_26 = {"toggleAutoScale", 1, param_slot_26 };
    static const QUParameter param_slot_27[] = {
	{ "val", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_27 = {"toggleShader", 1, param_slot_27 };
    static const QUMethod slot_28 = {"rotate", 0, 0 };
    static const QUParameter param_slot_29[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_29 = {"setPolygonOffset", 1, param_slot_29 };
    static const QUParameter param_slot_30[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<struct element_3D>", QUParameter::In },
	{ 0, &static_QUType_ptr, "SA2d_control_variables", QUParameter::In }
    };
    static const QUMethod slot_30 = {"setParameters", 4, param_slot_30 };
    static const QUParameter param_slot_31[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_QString, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "double*", QUParameter::InOut },
	{ 0, &static_QUType_ptr, "SA2d_control_variables", QUParameter::In }
    };
    static const QUMethod slot_31 = {"setParameters", 5, param_slot_31 };
    static const QUParameter param_slot_32[] = {
	{ "title_3d", &static_QUType_QString, 0, QUParameter::In },
	{ "xtitle", &static_QUType_QString, 0, QUParameter::In },
	{ "ytitle", &static_QUType_QString, 0, QUParameter::In },
	{ "ztitle", &static_QUType_QString, 0, QUParameter::In },
	{ "data", &static_QUType_ptr, "double*", QUParameter::InOut },
	{ "sa2d_ctrl_vars", &static_QUType_ptr, "SA2d_control_variables", QUParameter::In }
    };
    static const QUMethod slot_32 = {"setParameters", 6, param_slot_32 };
    static const QUParameter param_slot_33[] = {
	{ "x", &static_QUType_double, 0, QUParameter::In },
	{ "y", &static_QUType_double, 0, QUParameter::In },
	{ "z", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_33 = {"showRotate", 3, param_slot_33 };
    static const QUParameter param_slot_34[] = {
	{ "x", &static_QUType_double, 0, QUParameter::In },
	{ "y", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_34 = {"showShift", 2, param_slot_34 };
    static const QUParameter param_slot_35[] = {
	{ "x", &static_QUType_double, 0, QUParameter::In },
	{ "y", &static_QUType_double, 0, QUParameter::In },
	{ "z", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_35 = {"showScale", 3, param_slot_35 };
    static const QUParameter param_slot_36[] = {
	{ "z", &static_QUType_double, 0, QUParameter::In }
    };
    static const QUMethod slot_36 = {"showZoom", 1, param_slot_36 };
    static const QUParameter param_slot_37[] = {
	{ "val", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_37 = {"showNormals", 1, param_slot_37 };
    static const QUParameter param_slot_38[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_38 = {"setNormalQuality", 1, param_slot_38 };
    static const QUParameter param_slot_39[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_39 = {"setNormalLength", 1, param_slot_39 };
    static const QUParameter param_slot_40[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::Out },
	{ "cv", &static_QUType_ptr, "Qwt3D::ColorVector", QUParameter::InOut },
	{ "fname", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_40 = {"openColorMap", 3, param_slot_40 };
    static const QUParameter param_slot_41[] = {
	{ 0, &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_41 = {"adaptDataColors", 1, param_slot_41 };
    static const QUParameter param_slot_42[] = {
	{ "majors", &static_QUType_int, 0, QUParameter::In },
	{ "minors", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_42 = {"updateColorLegend", 2, param_slot_42 };
    static const QUParameter param_slot_43[] = {
	{ "b", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_43 = {"setLeftGrid", 1, param_slot_43 };
    static const QUParameter param_slot_44[] = {
	{ "b", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_44 = {"setRightGrid", 1, param_slot_44 };
    static const QUParameter param_slot_45[] = {
	{ "b", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_45 = {"setCeilGrid", 1, param_slot_45 };
    static const QUParameter param_slot_46[] = {
	{ "b", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_46 = {"setFloorGrid", 1, param_slot_46 };
    static const QUParameter param_slot_47[] = {
	{ "b", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_47 = {"setFrontGrid", 1, param_slot_47 };
    static const QUParameter param_slot_48[] = {
	{ "b", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_48 = {"setBackGrid", 1, param_slot_48 };
    static const QUParameter param_slot_49[] = {
	{ 0, &static_QUType_ptr, "Qwt3D::SIDE", QUParameter::In },
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_49 = {"setGrid", 2, param_slot_49 };
    static const QUParameter param_slot_50[] = {
	{ "val", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_50 = {"enableLighting", 1, param_slot_50 };
    static const QUParameter param_slot_51[] = {
	{ "e", &static_QUType_ptr, "QCloseEvent", QUParameter::In }
    };
    static const QUMethod slot_51 = {"closeEvent", 1, param_slot_51 };
    static const QMetaData slot_tbl[] = {
	{ "open()", &slot_0, QMetaData::Public },
	{ "openMesh()", &slot_1, QMetaData::Public },
	{ "createFunction()", &slot_2, QMetaData::Public },
	{ "setFileType(const QString&)", &slot_3, QMetaData::Public },
	{ "pickCoordSystem(QAction*)", &slot_4, QMetaData::Public },
	{ "pickPlotStyle(QAction*)", &slot_5, QMetaData::Public },
	{ "pickFloorStyle(QAction*)", &slot_6, QMetaData::Public },
	{ "pickAxesColor()", &slot_7, QMetaData::Public },
	{ "pickBgColor()", &slot_8, QMetaData::Public },
	{ "pickMeshColor()", &slot_9, QMetaData::Public },
	{ "pickNumberColor()", &slot_10, QMetaData::Public },
	{ "pickLabelColor()", &slot_11, QMetaData::Public },
	{ "pickTitleColor()", &slot_12, QMetaData::Public },
	{ "pickDataColor()", &slot_13, QMetaData::Public },
	{ "pickLighting()", &slot_14, QMetaData::Public },
	{ "resetColors()", &slot_15, QMetaData::Public },
	{ "pickNumberFont()", &slot_16, QMetaData::Public },
	{ "pickLabelFont()", &slot_17, QMetaData::Public },
	{ "pickTitleFont()", &slot_18, QMetaData::Public },
	{ "resetFonts()", &slot_19, QMetaData::Public },
	{ "setStandardView()", &slot_20, QMetaData::Public },
	{ "dumpImage()", &slot_21, QMetaData::Public },
	{ "dumpImage(QString,QString)", &slot_22, QMetaData::Public },
	{ "toggleAnimation(bool)", &slot_23, QMetaData::Public },
	{ "toggleProjectionMode(bool)", &slot_24, QMetaData::Public },
	{ "toggleColorLegend(bool)", &slot_25, QMetaData::Public },
	{ "toggleAutoScale(bool)", &slot_26, QMetaData::Public },
	{ "toggleShader(bool)", &slot_27, QMetaData::Public },
	{ "rotate()", &slot_28, QMetaData::Public },
	{ "setPolygonOffset(int)", &slot_29, QMetaData::Public },
	{ "setParameters(QString,QString,vector<struct element_3D>,SA2d_control_variables*)", &slot_30, QMetaData::Public },
	{ "setParameters(QString,QString,QString,double**,SA2d_control_variables*)", &slot_31, QMetaData::Public },
	{ "setParameters(QString,QString,QString,QString,double**,SA2d_control_variables*)", &slot_32, QMetaData::Public },
	{ "showRotate(double,double,double)", &slot_33, QMetaData::Public },
	{ "showShift(double,double)", &slot_34, QMetaData::Public },
	{ "showScale(double,double,double)", &slot_35, QMetaData::Public },
	{ "showZoom(double)", &slot_36, QMetaData::Public },
	{ "showNormals(bool)", &slot_37, QMetaData::Public },
	{ "setNormalQuality(int)", &slot_38, QMetaData::Public },
	{ "setNormalLength(int)", &slot_39, QMetaData::Public },
	{ "openColorMap(Qwt3D::ColorVector&,QString)", &slot_40, QMetaData::Public },
	{ "adaptDataColors(const QString&)", &slot_41, QMetaData::Public },
	{ "updateColorLegend(int,int)", &slot_42, QMetaData::Public },
	{ "setLeftGrid(bool)", &slot_43, QMetaData::Public },
	{ "setRightGrid(bool)", &slot_44, QMetaData::Public },
	{ "setCeilGrid(bool)", &slot_45, QMetaData::Public },
	{ "setFloorGrid(bool)", &slot_46, QMetaData::Public },
	{ "setFrontGrid(bool)", &slot_47, QMetaData::Public },
	{ "setBackGrid(bool)", &slot_48, QMetaData::Public },
	{ "setGrid(Qwt3D::SIDE,bool)", &slot_49, QMetaData::Public },
	{ "enableLighting(bool)", &slot_50, QMetaData::Public },
	{ "closeEvent(QCloseEvent*)", &slot_51, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"Mesh2MainWindow", parentObject,
	slot_tbl, 52,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_Mesh2MainWindow.setMetaObject( metaObj );
    return metaObj;
}

void* Mesh2MainWindow::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "Mesh2MainWindow" ) )
	return this;
    return Mesh2MainWindowBase::qt_cast( clname );
}

bool Mesh2MainWindow::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: open(); break;
    case 1: openMesh(); break;
    case 2: createFunction(); break;
    case 3: setFileType((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: pickCoordSystem((QAction*)static_QUType_ptr.get(_o+1)); break;
    case 5: pickPlotStyle((QAction*)static_QUType_ptr.get(_o+1)); break;
    case 6: pickFloorStyle((QAction*)static_QUType_ptr.get(_o+1)); break;
    case 7: pickAxesColor(); break;
    case 8: pickBgColor(); break;
    case 9: pickMeshColor(); break;
    case 10: pickNumberColor(); break;
    case 11: pickLabelColor(); break;
    case 12: pickTitleColor(); break;
    case 13: pickDataColor(); break;
    case 14: pickLighting(); break;
    case 15: resetColors(); break;
    case 16: pickNumberFont(); break;
    case 17: pickLabelFont(); break;
    case 18: pickTitleFont(); break;
    case 19: resetFonts(); break;
    case 20: setStandardView(); break;
    case 21: dumpImage(); break;
    case 22: dumpImage((QString)static_QUType_QString.get(_o+1),(QString)static_QUType_QString.get(_o+2)); break;
    case 23: toggleAnimation((bool)static_QUType_bool.get(_o+1)); break;
    case 24: toggleProjectionMode((bool)static_QUType_bool.get(_o+1)); break;
    case 25: toggleColorLegend((bool)static_QUType_bool.get(_o+1)); break;
    case 26: toggleAutoScale((bool)static_QUType_bool.get(_o+1)); break;
    case 27: toggleShader((bool)static_QUType_bool.get(_o+1)); break;
    case 28: rotate(); break;
    case 29: setPolygonOffset((int)static_QUType_int.get(_o+1)); break;
    case 30: setParameters((QString)static_QUType_QString.get(_o+1),(QString)static_QUType_QString.get(_o+2),(vector<struct element_3D>)(*((vector<struct element_3D>*)static_QUType_ptr.get(_o+3))),(SA2d_control_variables*)static_QUType_ptr.get(_o+4)); break;
    case 31: setParameters((QString)static_QUType_QString.get(_o+1),(QString)static_QUType_QString.get(_o+2),(QString)static_QUType_QString.get(_o+3),(double**)static_QUType_ptr.get(_o+4),(SA2d_control_variables*)static_QUType_ptr.get(_o+5)); break;
    case 32: setParameters((QString)static_QUType_QString.get(_o+1),(QString)static_QUType_QString.get(_o+2),(QString)static_QUType_QString.get(_o+3),(QString)static_QUType_QString.get(_o+4),(double**)static_QUType_ptr.get(_o+5),(SA2d_control_variables*)static_QUType_ptr.get(_o+6)); break;
    case 33: showRotate((double)static_QUType_double.get(_o+1),(double)static_QUType_double.get(_o+2),(double)static_QUType_double.get(_o+3)); break;
    case 34: showShift((double)static_QUType_double.get(_o+1),(double)static_QUType_double.get(_o+2)); break;
    case 35: showScale((double)static_QUType_double.get(_o+1),(double)static_QUType_double.get(_o+2),(double)static_QUType_double.get(_o+3)); break;
    case 36: showZoom((double)static_QUType_double.get(_o+1)); break;
    case 37: showNormals((bool)static_QUType_bool.get(_o+1)); break;
    case 38: setNormalQuality((int)static_QUType_int.get(_o+1)); break;
    case 39: setNormalLength((int)static_QUType_int.get(_o+1)); break;
    case 40: static_QUType_bool.set(_o,openColorMap((Qwt3D::ColorVector&)*((Qwt3D::ColorVector*)static_QUType_ptr.get(_o+1)),(QString)static_QUType_QString.get(_o+2))); break;
    case 41: adaptDataColors((const QString&)static_QUType_QString.get(_o+1)); break;
    case 42: updateColorLegend((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 43: setLeftGrid((bool)static_QUType_bool.get(_o+1)); break;
    case 44: setRightGrid((bool)static_QUType_bool.get(_o+1)); break;
    case 45: setCeilGrid((bool)static_QUType_bool.get(_o+1)); break;
    case 46: setFloorGrid((bool)static_QUType_bool.get(_o+1)); break;
    case 47: setFrontGrid((bool)static_QUType_bool.get(_o+1)); break;
    case 48: setBackGrid((bool)static_QUType_bool.get(_o+1)); break;
    case 49: setGrid((Qwt3D::SIDE)(*((Qwt3D::SIDE*)static_QUType_ptr.get(_o+1))),(bool)static_QUType_bool.get(_o+2)); break;
    case 50: enableLighting((bool)static_QUType_bool.get(_o+1)); break;
    case 51: closeEvent((QCloseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return Mesh2MainWindowBase::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool Mesh2MainWindow::qt_emit( int _id, QUObject* _o )
{
    return Mesh2MainWindowBase::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool Mesh2MainWindow::qt_property( int id, int f, QVariant* v)
{
    return Mesh2MainWindowBase::qt_property( id, f, v);
}

bool Mesh2MainWindow::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
