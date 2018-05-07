#pragma once

/**
 * The Ballistic Missile model combining most of the logic.
 */

#include "common.h"
#include "physics.h"
#include "graphics.h"

kScienceNSHeader() namespace model
{

	using namespace phys;
	using namespace graphics;
    using namespace math;


//========================================================================
//===================== Palette ==========================================
//========================================================================



	struct palette
	{
		Gdiplus::Pen *grid_color, *potential_color, *kinetic_color,
			*summary_color, *bold_grid_color,
			*earth_color, *x_color, *y_color, *z_color,
			*source_color, *target_color, *missile_color, *trace_color;
		Gdiplus::Font *font;
		Gdiplus::FontFamily *fontFamily;

		palette()
		{
			potential_color = Gdiplus::Pen(Gdiplus::Color(255, 100, 100), 4).Clone();
			kinetic_color = Gdiplus::Pen(Gdiplus::Color(Gdiplus::Color::Green), 4).Clone();
			summary_color = Gdiplus::Pen(Gdiplus::Color(100, 100, 255), 4).Clone();
			grid_color = Gdiplus::Pen(Gdiplus::Color(185, 185, 185)).Clone();
			grid_color->SetDashStyle(Gdiplus::DashStyleDash);
			bold_grid_color = Gdiplus::Pen(Gdiplus::Color(135, 135, 135)).Clone();
			earth_color = Gdiplus::Pen(Gdiplus::Color::DeepSkyBlue).Clone();
			x_color = Gdiplus::Pen(Gdiplus::Color(255, 0, 0), 5).Clone();
			y_color = Gdiplus::Pen(Gdiplus::Color(0, 255, 0), 5).Clone();
			z_color = Gdiplus::Pen(Gdiplus::Color(0, 0, 255), 5).Clone();
			source_color = Gdiplus::Pen(Gdiplus::Color(255, 0, 0), 5).Clone();
			target_color = Gdiplus::Pen(Gdiplus::Color(Gdiplus::Color::Green), 5).Clone();
			missile_color = Gdiplus::Pen(Gdiplus::Color(0, 0, 0)).Clone();
			trace_color = Gdiplus::Pen(Gdiplus::Color(255, 0, 0), 3).Clone();
			fontFamily = Gdiplus::FontFamily(_T("Times New Roman")).Clone();
			font = Gdiplus::Font(fontFamily, 10).Clone();
		}

		~palette()
		{
			delete grid_color; delete potential_color; delete bold_grid_color;
			delete earth_color; delete x_color; delete y_color;
			delete z_color; delete font; delete source_color;
			delete target_color; delete missile_color; delete trace_color;
			delete fontFamily; delete kinetic_color; delete summary_color;
		}

	} palette;




//========================================================================
//===================== Model Classes ====================================
//========================================================================




	class Model;
	class Earth;




//========================================================================
//===================== Class Missile ====================================
//========================================================================




	class Missile
	{
		friend class Model;
		friend class Earth;

		Mesh missile;
		Objects xyz;
		Mesh trace;
		Polyline *traceLine;
		Mesh r0;

		Missile()
			:
			missile(create_sphere(R / 20, 10, 10), palette.missile_color),
			xyz(create_xyz(R / 5, palette.x_color, palette.y_color, palette.z_color))
		{
			r0.getMesh().lines.push_back(Polyline());
			r0.getMesh().lines.push_back(Polyline());
			r0.getMesh()[0].vertices.push_back({});
			r0.getMesh()[0].vertices.push_back({});
			r0.getMesh()[1].vertices.push_back({});
			r0.getMesh()[1].vertices.push_back({});
			r0.setPen(palette.missile_color);
			trace.getMesh().lines.push_back(Polyline());
			traceLine = &trace.getMesh()[0];
			trace.setPen(palette.trace_color);
			missile.setPen(palette.missile_color);
		}

		void supply(const v3 &loc)
		{
			xyz.setTransform({ loc, align_axes(identity(), 2, 1, loc, loc ^ v3{ 0, 0, 1 }) });

			missile.setTransform(loc);
			if (traceLine->size() > 0 && norm((*traceLine)[traceLine->size() - 1] - loc) < R / 100) return;
			traceLine->vertices.push_back(loc);

			r0.getMesh()[0][1] = loc;
			r0.getMesh()[1][1] = (loc ^ v3{ 0, 0, 1 }) / norm(loc ^ v3{ 0, 0, 1 }) * R;
		}

		void reset(const preconditions &state0)
		{
			traceLine->vertices.clear();
			this->supply(state0.source);
		}
	};




//========================================================================
//===================== Class Earth ======================================
//========================================================================




	class Earth
	{
		friend class Model;

		Mesh earth;
		Mesh source, target;

		Objects earthScene;

		Missile missile;

		bool rotation;

		Earth()
			:
			earth(create_sphere(R), palette.earth_color),
			source(create_sphere(R / 30, 3, 20), palette.source_color),
			target(create_sphere(R / 30, 3, 20), palette.target_color)
		{
			earthScene.getObjects().push_back(&earth);
			earthScene.getObjects().push_back(&source);
			earthScene.getObjects().push_back(&target);
			earthScene.getObjects().push_back(&missile.trace);
			earthScene.getObjects().push_back(&missile.missile);
			earthScene.getObjects().push_back(&missile.xyz);
			earthScene.getObjects().push_back(&missile.r0);
		}

		void setRotation(double zrotation)
		{
			earthScene.setTransform(rotate_z(zrotation));
		}

		void supply(const state &newState)
		{
			if (rotation) setRotation(newState.time * W);
		}

		void reset(const preconditions &state0)
		{
			setRotation(0);
			source.setTransform(state0.source);
			target.setTransform(state0.target);
			missile.reset(state0);
		}
	};




//========================================================================
//===================== Class EnergyPlot =================================
//========================================================================




	class EnergyPlot
	{

		friend class Model;

		RealTimeFunctionDrawable *potential, *kinetic, *summary;
		FunctionViewportDrawable::AutoTickFactory *xTicks, *yTicks;
		FunctionGridDrawable *grid;
		MultiDrawable *view;

		FunctionViewportDrawable *viewport;

		EnergyPlot()
		{
			view = new MultiDrawable();

			xTicks = new FunctionViewportDrawable::AutoTickFactory(sfloat(0., 16), sfloat(0., 16), 10);
			yTicks = new FunctionViewportDrawable::AutoTickFactory(sfloat(0., 16), sfloat(0., 16), 10);

			potential = new RealTimeFunctionDrawable({}, -1e300, +1e300, palette.potential_color);
			kinetic = new RealTimeFunctionDrawable({}, -1e300, +1e300, palette.kinetic_color);
			summary = new RealTimeFunctionDrawable({}, -1e300, +1e300, palette.summary_color);
			grid = new FunctionGridDrawable({}, palette.grid_color, xTicks, yTicks);

			view->GetLayers().push_back(potential);
			view->GetLayers().push_back(kinetic);
			view->GetLayers().push_back(summary);
			view->GetLayers().push_back(grid);

			viewport = new FunctionViewportDrawable(view, {}, palette.bold_grid_color, palette.font, xTicks, yTicks);
		}

		void setViewport(const Viewport &vp, bool hard = false)
		{
			potential->setVewport(vp, hard);
			kinetic->setVewport(vp, hard);
			summary->setVewport(vp, hard);

			xTicks->setLeft(sfloat(vp.xmin, 16));
			xTicks->setRight(sfloat(vp.xmax, 16));
			yTicks->setLeft(sfloat(vp.ymin, 16));
			yTicks->setRight(sfloat(vp.ymax, 16));
			grid->setViewport(vp);
			viewport->setViewport(vp);
		}

		void supply(const state &newState)
		{
			// 'supply' returns bool basing only on passed X,
			// no need to check other plots
			bool modified = potential->supply(newState.time, newState.energy.x);
			
			if (!modified) return;

			Viewport vp = potential->getViewport();

			kinetic->supply(newState.time, newState.energy.y);
			Viewport vp2 = kinetic->getViewport();
			summary->supply(newState.time, newState.energy.z);
			Viewport vp3 = summary->getViewport();

			vp.ymin = min(vp.ymin, min(vp2.ymin, vp3.ymin));
			vp.ymax = max(vp.ymax, max(vp2.ymax, vp3.ymax));

			setViewport(vp);
		}

		void reset(const preconditions &state0)
		{
			potential->clear();
			kinetic->clear();
			summary->clear();
			Viewport vp = potential->getViewport();
			vp = { 0, vp.xmax - vp.xmin, 0, 0 };
			setViewport(vp, true);
		}

		void setScale(double xmax0)
		{
			Viewport vp = potential->getViewport();
			vp.xmin = vp.xmax - xmax0;
			setViewport(vp);
		}

		~EnergyPlot()
		{
			delete potential;
			delete kinetic;
			delete summary;
			delete xTicks;
			delete yTicks;
			delete grid;
			delete view;
		}

	};




//========================================================================
//===================== Class Formatter ==================================
//========================================================================




	class Formatter
	{

	public:

		struct spreconditions
		{
			CString source;
			CString target;
			CString velocity;
		};

		struct sstate
		{
			CString time;
			CString energy;
			CString location;
			CString velocity;
		};

		preconditions parse(spreconditions input)
		{
			preconditions output;

			v3 tmp = { R, 0, 0 };
			v3 source_sp;

			_sntscanf_s(input.source.GetBuffer(), input.source.GetLength(),
				_T("la %lf lo %lf"), &tmp.y, &tmp.z);
			output.source = spherical2rect(tmp, true);

			_sntscanf_s(input.target.GetBuffer(), input.target.GetLength(),
				_T("la %lf lo %lf"), &tmp.y, &tmp.z);
			output.target = spherical2rect(tmp, true);
			source_sp = tmp; source_sp.x = 0;

			_sntscanf_s(input.velocity.GetBuffer(), input.velocity.GetLength(),
				_T("norm %lf po %lf az %lf"), &tmp.x, &tmp.y, &tmp.z);
			output.velocity = align_axes(
				identity(), 2, 1, output.source, output.source ^ v3{ 0, 0, 1 }
			) * spherical2rect(tmp, true);

			return output;
		}

		spreconditions format(preconditions input)
		{
			spreconditions output;
			v3 source_sp;

			v3 tmp = rect2spherical(input.source, true);
			output.source.Format(_T("la %.2lf lo %.2lf"), tmp.y, tmp.z);

			tmp = rect2spherical(input.target, true);
			output.target.Format(_T("la %.2lf lo %.2lf"), tmp.y, tmp.z);
			source_sp = tmp; source_sp.x = 0;

			tmp = rect2spherical(
				!align_axes(
				identity(), 2, 1, input.source, input.source ^ v3{ 0, 0, 1 }
			) * input.velocity, true);
			output.velocity.Format(_T("norm %.2lf po %.2lf az %.2lf"), tmp.x, tmp.y, tmp.z);

			return output;
		}

		sstate format(state input)
		{
			sstate output;

			output.time.Format(_T("%.2lf"), input.time);

			output.energy.Format(_T("%.2lf"), input.energy.z);

			v3 sp_loc = rect2spherical(input.r, true);
			output.location.Format(_T("h %.2lf la %.2lf lo %.2lf"), sp_loc.x - R, sp_loc.y, sp_loc.z);

			output.velocity.Format(_T("%.2f"), norm(input.v));

			return output;
		}
	};

	


//========================================================================
//===================== Class Model ======================================
//========================================================================

	


	class Model
	{

		Earth earth;
		EnergyPlot plot;

		Objects xyz;
		Objects scene;
		SceneDrawable *sceneDrawable;

		m3 standardRotation;
		Viewport standardViewport;
		double standardCamera, standardFocus;

		Formatter formatter;

	public:

		Model()
			:
			standardViewport({ -R, R, -R, R }),
			standardCamera(-R),
			standardFocus(-R),
			standardRotation(rotate_x(acos(0))),
			xyz(create_xyz(R / 3, palette.x_color, palette.y_color, palette.z_color))
		{
			sceneDrawable = new SceneDrawable(standardViewport, scene, standardCamera, standardFocus * 10);

			scene.getObjects().push_back(&xyz);
			scene.getObjects().push_back(&earth.earthScene);

			this->setRotation(0, 0);
		}

		~Model()
		{
			delete sceneDrawable;
		}

	public:

		void setRotation(double azimuthal, double polar)
		{
			sceneDrawable->setTransform(rotate_x(polar) * standardRotation * rotate_z(azimuthal));
		}

		void setDepth(double depth)
		{
			sceneDrawable->set3DViewport(standardViewport, standardCamera, standardFocus * depth);
		}

		void supply(state newState)
		{
			earth.supply(newState);
			earth.missile.supply(newState.r);
			plot.supply(newState);
		}

		void reset(const preconditions &state0)
		{
			earth.reset(state0);
			plot.reset(state0);
		}

		void setPlotScale(double xmax0)
		{
			plot.setScale(xmax0);
		}

		void setAntiAliased(bool aa)
		{
			sceneDrawable->setAntiAliased(aa);
		}

		void setEarthRotation(bool rotation)
		{
			earth.rotation = rotation;
			if (!rotation) earth.setRotation(0);
		}

		SceneDrawable & getScene()
		{
			return *this->sceneDrawable;
		}

		Drawable * getPlot()
		{
			return this->plot.viewport;
		}

		Formatter & getFormatter()
		{
			return formatter;
		}
	};

} kScienceNSFooter()
