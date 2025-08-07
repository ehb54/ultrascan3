#ifndef lightingdlg_h__2004_03_07_13_35_begin_guarded_code
#define lightingdlg_h__2004_03_07_13_35_begin_guarded_code

#include <math.h>
#include <qapplication.h>
#include <qwt3d_color.h>
#include <qwt3d_enrichment.h>
#include <qwt3d_parametricsurface.h>
#include <qwt3d_surfaceplot.h>

#include "lightingdlgbase.h"

class Pointer : public Qwt3D::VertexEnrichment {
   public:
      Pointer(double rad);
      ~Pointer();

      Qwt3D::Enrichment *clone() const { return new Pointer(*this); }

      void configure(double rad);
      void drawBegin();
      void draw(Qwt3D::Triple const &) {}
      void setPos(double x, double y, double z) { pos_ = Qwt3D::Triple(x, y, z); }

   private:
      double radius_;
      Qwt3D::Triple pos_;
};

struct SColor : public Qwt3D::Color {
      Qwt3D::RGBA operator()(double x, double y, double z) const {
         // Get rid of a stupid warnign about not using x, y, z
         // Is this function ever used?
         return Qwt3D::RGBA(0.8, 0, 0, 0.5 + (x + y + z) * 0);
      }
};

typedef Qwt3D::SurfacePlot SPlot;

class Plot : public SPlot {
      Q_OBJECT

   public:
      Plot(QWidget *parent);
      Pointer *stick;
      void reset();
};

class LightingDlg : public lightingdlgbaseBase {
      Q_OBJECT

   public:
      LightingDlg(QWidget *parent = 0);
      ~LightingDlg();

      void assign(Qwt3D::Plot3D *pl);

      Plot *plot;
      Qwt3D::Plot3D *dataPlot;

   public slots:
      void setDistance(int);
      void setEmission(int);
      void setDiff(int);
      void setSpec(int);
      void setShin(int);
      void reset();
      void setRotation(double x, double y, double z);
};


#endif /* include guarded */
