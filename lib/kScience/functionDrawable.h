/*
Copyright(C) 2015 Alexander Vasilevsky aka kalaider

This program is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "drawable.h"
#include "function.h"

#include "sfloat.h"

#include <sstream>

kScienceGraphicsNSHeader()

using kScienceMathNS::sfloat;

struct Viewport
{
	double xmin, xmax, ymin, ymax;
};

inline int convert(double left, double right, int dim, double x) {
	return (int)(((x - left) * dim) / (right - left));
}

class FunctionDrawable :
	virtual public Drawable
{

public:
	
private:

	Function *function;
	Viewport viewport;
	int step;
	Gdiplus::Pen *pen;

public:

	void setViewport(Viewport &viewport)
	{
		this->viewport = viewport;
	}

	FunctionDrawable
		(
		Function *function,
		Viewport viewport,
		Gdiplus::Pen *pen,
		int step = 1
		) :
		function(function), viewport(viewport), pen(pen), step(step)
	{
	}

	virtual void Draw(Gdiplus::Graphics *graphics, Gdiplus::Rect bounds)
	{
		graphics->SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);

		Gdiplus::Point *points = new Gdiplus::Point[bounds.Width / step + 1];

		int i = 0, j = 0, k; double y = 0, jt;
		while (i <= bounds.Width / step)
		{
			for (k = i; k <= bounds.Width / step; k++) {
				y = (*function)(viewport.xmin + (viewport.xmax - viewport.xmin) / bounds.Width * k * step);
				jt = (((y - viewport.ymin) * bounds.Height) / (viewport.ymax - viewport.ymin));

				if (jt >= MAXSHORT || jt <= -MAXSHORT || !std::isfinite(jt))
				{
					++k;
					break;
				}
				else
				{
					j = (int)jt;
				}
				points[k - i].X = k * step; points[k - i].Y = bounds.Height - j;
			}

			if (k - i - 1 > 0)
			{
				graphics->DrawLines(pen, points, k - i - 1);
			}

			i = k;
		}

		delete[] points;
	}
};

class DataArrayDrawable :
	virtual public Drawable
{

public:

	static struct Node
	{
		Drawable *drawable;
		Gdiplus::Rect bounds;
	};

private:

	const std::map<Point, double> *data;
	Node nodeTemplate;
	Viewport viewport;

public:

	void setViewport(Viewport &viewport)
	{
		this->viewport = viewport;
	}

	DataArrayDrawable
		(
		std::map<Point, double> *data,
		Viewport viewport,
		Node nodeTemplate
		) :
		data(data), viewport(viewport), nodeTemplate(nodeTemplate)
	{
	}

	virtual void Draw(Gdiplus::Graphics *graphics, Gdiplus::Rect bounds)
	{
		double x, y;
		Gdiplus::Point point;

		for (std::map<Point, double>::const_iterator it = data->cbegin(); it != data->cend(); it++)
		{
			x = it->first.x;
			y = it->second;
			point.X = convert(viewport.xmin, viewport.xmax, bounds.Width, x);
			point.Y = bounds.Height - convert(viewport.ymin, viewport.ymax, bounds.Height, y);
			point.X -= nodeTemplate.bounds.Width / 2;
			point.Y -= nodeTemplate.bounds.Height / 2;

			Gdiplus::GraphicsState state = graphics->Save();

			graphics->TranslateTransform(static_cast<Gdiplus::REAL>(point.X), static_cast<Gdiplus::REAL>(point.Y));
			graphics->IntersectClip(nodeTemplate.bounds);
			nodeTemplate.drawable->Draw(graphics, nodeTemplate.bounds);

			graphics->Restore(state);
		}
	}
};

class FunctionViewportDrawable :
	virtual public Drawable
{

public:

	static struct Tick
	{
		std::string display;
		double value;
	};

	static class TickFactory
	{
	public:
		virtual const std::vector<Tick> * produce() = 0;
	};

private:

	typedef MultiDrawable::Layer Layer;

	MultiDrawable view;
	Viewport viewport;
	Gdiplus::Pen *pen;
	Gdiplus::Font *font;
	TickFactory *xTickFactory;
	TickFactory *yTickFactory;

	/*
	
	We use pen->GetBrush() which returns "new XXXBrush..."
	instead of expected and documented backed object

	Quote from MSDN:

	The Pen::GetBrush method gets the Brush object that
	is currently set for this Pen object.

	*/
	Gdiplus::Brush *brush; // DELETEME
	
protected:

	enum class Location {
		MIN, CENTER, MAX
	};

	static struct Grid {
		Location xLocation, yLocation;
		double x0, y0;
		int textWidth, textHeight;
		Gdiplus::Point topLeft;
		Gdiplus::Point bottomRight;

		Grid
			(
			Location xLocation, Location yLocation,
			double x0, double y0,
			int textWidth, int textHeight,
			Gdiplus::Rect &bounds
			) :
			xLocation(xLocation), yLocation(yLocation),
			x0(x0), y0(y0),
			textWidth(textWidth), textHeight(textHeight)
		{
			topLeft.X = (xLocation == Location::MIN) ? textWidth : 0;
			topLeft.Y = (yLocation == Location::MAX) ? textHeight : 0;
			bottomRight.X = bounds.Width - ((xLocation == Location::MAX) ? textWidth : 0);
			bottomRight.Y = bounds.Height - ((yLocation == Location::MIN) ? textHeight : 0);
		}
	};

	const std::vector<Tick> *xticks,  *yticks;
	Location xLocation, yLocation;
	double x0, y0;

public:

	void setViewport(Viewport viewport)
	{
		this->viewport = viewport;

		xLocation = Location::MIN;
		x0 = viewport.xmin;
		if (viewport.xmin < 0 && viewport.xmax > 0)
		{
			x0 = 0;
			xLocation = Location::CENTER;
		}
		else if (viewport.xmax <= 0)
		{
			x0 = viewport.xmax;
			xLocation = Location::MAX;
		}

		yLocation = Location::MIN;
		y0 = viewport.ymin;
		if (viewport.ymin < 0 && viewport.ymax > 0)
		{
			y0 = 0;
			yLocation = Location::CENTER;
		}
		else if (viewport.ymax <= 0)
		{
			y0 = viewport.ymax;
			yLocation = Location::MAX;
		}
	}
	
	FunctionViewportDrawable
		(
		Drawable *view,
		Viewport viewport,
		Gdiplus::Pen *pen,
		Gdiplus::Font *font,
		TickFactory *xTickFactory,
		TickFactory *yTickFactory
		) :
		viewport(viewport), pen(pen), font(font), xTickFactory(xTickFactory), yTickFactory(yTickFactory)
	{
		this->brush = pen->GetBrush();
		this->view.GetLayers().push_back(Layer(view));
		setViewport(viewport);
	}

	~FunctionViewportDrawable()
	{
		delete brush;
	}

public:

	static class BasicTickFactory :
		public TickFactory
	{

	protected:

		sfloat left, right;
		unsigned int maxTicks;
		bool modified;

		std::vector<Tick> ticks;

		static std::string toString(const sfloat &num)
		{
			std::ostringstream ss;
			ss << num.real;
			return ss.str();
		}

		static sfloat reduce_interval(const sfloat &num)
		{
			sfloat n = num;
			if (num.mantissa == 5)
			{
				n /= 5;
				n *= 2;
			}
			else if (num.mantissa == 2)
			{
				n /= 2;
			}
			else if (num.mantissa == 1)
			{
				n *= 5;
				n >>= 1;
			}
			else 
			{
				throw std::logic_error("Mantissa 1, 2 or 5 expected.");
			}
			return n;
		}

		static double plain_div(const sfloat &number, const sfloat &interval)
		{
			return number.real / interval.real;
		}

		static sfloat round_top(const sfloat &number, const sfloat &interval)
		{
			sfloat result = interval;
			return (result *= static_cast<long long>(floor(plain_div(number, interval)) + 1));
		}

		static sfloat round_bottom(const sfloat &number, const sfloat &interval)
		{
			sfloat result = interval;
			return (result *= static_cast<long long>(ceil(plain_div(number, interval)) - 1));
		}

	public:

		BasicTickFactory(const sfloat &left, const sfloat &right, int maxTicks = 10) :
			left(left), right(right), maxTicks(maxTicks), modified(true)
		{
		}

		void setLeft(const sfloat &left)
		{
			if (this->left == left) return;
			this->left = left;
			modified = true;
		}

		void setRight(const sfloat &right)
		{
			if (this->right == right) return;
			this->right = right;
			modified = true;
		}

		void setMaxTicks(unsigned int maxTicks)
		{
			if (this->maxTicks == maxTicks) return;
			this->maxTicks = maxTicks;
			modified = true;
		}

	protected:

		virtual void calculate() = 0;

	public:

		virtual const std::vector<Tick> * produce()
		{
			if (modified) calculate();
			return &ticks;
		}
	};

	static class AutoTickFactory :
		public BasicTickFactory
	{

	public:

		AutoTickFactory(const sfloat &left, const sfloat &right, int maxTicks = 10) :
			BasicTickFactory(left, right, maxTicks)
		{
		}

	protected:

		virtual void calculate()
		{
			if (left == right) {
				ticks.resize(0);
				return;
			}

			sfloat interval(1LL, max_of(left.exp(), right.exp()) + 1);
			
			sfloat top = round_top(right, interval), bottom = round_bottom(left, interval);

			unsigned int quantity = static_cast<unsigned int>(floor(abs((top.real - bottom.real)) / interval.real));

			while (true)
			{
				sfloat next_interval = reduce_interval(interval);
				sfloat next_top = round_top(right, next_interval), next_bottom = round_bottom(left, next_interval);
				unsigned int next_quantity = static_cast<unsigned int>(floor(abs((next_top.real - next_bottom.real)) / next_interval.real));
				if (next_quantity > maxTicks) break;
				quantity = next_quantity;
				interval = next_interval;
				top = next_top;
				bottom = next_bottom;
			}

			ticks.resize(quantity - 1);
			for (unsigned int i = 0; i < quantity - 1; i++)
			{
				bottom += interval;
				ticks[i] = { toString(bottom), bottom.real };
			}
		}
	};

public:

	virtual void Draw(Gdiplus::Graphics *graphics, Gdiplus::Rect bounds)
	{
		graphics->SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintSingleBitPerPixelGridFit);
		
		// =============== Calculate Grid ==================

		int textWidth = 0, textHeight = 0;

		CString out;
		Gdiplus::RectF textbounds;
		Gdiplus::PointF point(0, 0);

		xticks = xTickFactory->produce();
		yticks = yTickFactory->produce();

		for (unsigned int i = 0; i < yticks->size(); i++)
		{
			out = CString((*yticks)[i].display.c_str());
			graphics->MeasureString(out.GetBuffer(), out.GetLength(), font, point, &textbounds);
			if (textWidth < ceil(textbounds.Width)) textWidth = static_cast<int>(ceil(textbounds.Width));
		}
		textHeight = static_cast<int>(ceil(textbounds.Height));

		Grid grid
			(
			xLocation, yLocation,
			x0,
			y0,
			textWidth,
			textHeight,
			bounds
			);

		// =============== Draw View =======================

		view.GetLayers()[0].bounds = Gdiplus::Rect(grid.topLeft.X, grid.topLeft.Y, grid.bottomRight.X - grid.topLeft.X, grid.bottomRight.Y - grid.topLeft.Y);
		view.Draw(graphics, bounds);

		// =============== Draw Axises =====================

		DrawXAxis(graphics, bounds, grid);
		DrawYAxis(graphics, bounds, grid);

		DrawXAxisTicks(graphics, bounds, grid);
		DrawYAxisTicks(graphics, bounds, grid);
	}

protected:

	void DrawXAxis(Gdiplus::Graphics *graphics, Gdiplus::Rect &bounds, Grid &grid)
	{
		Gdiplus::Point start, end;

		start.Y = grid.bottomRight.Y - convert(viewport.ymin, viewport.ymax, grid.bottomRight.Y - grid.topLeft.Y, grid.y0);
		end.Y = start.Y;

		start.X = grid.topLeft.X;
		end.X = grid.bottomRight.X;

		graphics->DrawLine(pen, start, end);
	}

	void DrawXAxisTicks(Gdiplus::Graphics *graphics, Gdiplus::Rect &bounds, Grid &grid)
	{
		Gdiplus::Point start, end;
		Gdiplus::PointF text;

		start.Y = grid.bottomRight.Y - convert(viewport.ymin, viewport.ymax, grid.bottomRight.Y - grid.topLeft.Y, grid.y0);
		end.Y = start.Y;

		switch (grid.yLocation)
		{
		case Location::MIN:
			start.Y -= 5;
			text.Y = static_cast<Gdiplus::REAL>(end.Y);
			break;
		case Location::MAX:
			end.Y += 5;
			text.Y = static_cast<Gdiplus::REAL>(start.Y - grid.textHeight);
			break;
		case Location::CENTER:
			start.Y -= 5;
			end.Y += 5;
			text.Y = static_cast<Gdiplus::REAL>(end.Y);
			break;
		}

		CString out;
		Gdiplus::RectF textbounds;

		for (unsigned int i = 0; i < xticks->size(); i++)
		{
			start.X = grid.topLeft.X + convert(viewport.xmin, viewport.xmax, grid.bottomRight.X - grid.topLeft.X, (*xticks)[i].value);
			end.X = start.X;
			graphics->DrawLine(pen, start, end);

			out = CString((*xticks)[i].display.c_str());
			graphics->MeasureString(out.GetBuffer(), out.GetLength(), font, text, &textbounds);
			text.X = start.X - textbounds.Width / 2;
			graphics->DrawString(out.GetBuffer(), out.GetLength(), font, text, brush);

		}
	}

	void DrawYAxis(Gdiplus::Graphics *graphics, Gdiplus::Rect &bounds, Grid &grid)
	{
		Gdiplus::Point start, end;

		start.X = grid.topLeft.X + convert(viewport.xmin, viewport.xmax, grid.bottomRight.X - grid.topLeft.X, grid.x0);
		end.X = start.X;

		start.Y = grid.topLeft.Y;
		end.Y = grid.bottomRight.Y;

		graphics->DrawLine(pen, start, end);
	}

	void DrawYAxisTicks(Gdiplus::Graphics *graphics, Gdiplus::Rect &bounds, Grid &grid)
	{
		Gdiplus::Point start, end;
		Gdiplus::PointF text;

		start.X = grid.topLeft.X + convert(viewport.xmin, viewport.xmax, grid.bottomRight.X - grid.topLeft.X, grid.x0);
		end.X = start.X;

		switch (grid.xLocation)
		{
		case Location::MIN:
			end.X += 5;
			text.X = static_cast<Gdiplus::REAL>(start.X - grid.textWidth);
			break;
		case Location::MAX:
			start.X -= 5;
			text.X = static_cast<Gdiplus::REAL>(end.X + 1);
			break;
		case Location::CENTER:
			start.X -= 5;
			end.X += 5;
			text.X = static_cast<Gdiplus::REAL>(start.X - grid.textWidth);
			break;
		}

		CString out;

		for (unsigned int i = 0; i < yticks->size(); i++)
		{
			start.Y = grid.bottomRight.Y - convert(viewport.ymin, viewport.ymax, grid.bottomRight.Y - grid.topLeft.Y, (*yticks)[i].value);
			end.Y = start.Y;
			graphics->DrawLine(pen, start, end);

			out = CString((*yticks)[i].display.c_str());
			text.Y = static_cast<Gdiplus::REAL>(start.Y - grid.textHeight / 2);
			graphics->DrawString(out.GetBuffer(), out.GetLength(), font, text, brush);
		}
	}
};

class FunctionGridDrawable :
	virtual public Drawable
{

	typedef FunctionViewportDrawable::TickFactory TickFactory;
	typedef FunctionViewportDrawable::Tick Tick;
	typedef MultiDrawable::Layer Layer;

	Viewport viewport;
	Gdiplus::Pen *pen;
	TickFactory *xTickFactory;
	TickFactory *yTickFactory;
	const std::vector<Tick> *xticks, *yticks;

public:

	void setViewport(Viewport viewport)
	{
		this->viewport = viewport;
	}

	FunctionGridDrawable
		(
		Viewport viewport,
		Gdiplus::Pen *pen,
		TickFactory *xTickFactory,
		TickFactory *yTickFactory
		) :
		viewport(viewport), pen(pen), xTickFactory(xTickFactory), yTickFactory(yTickFactory)
	{
	}

	virtual void Draw(Gdiplus::Graphics *graphics, Gdiplus::Rect bounds)
	{
		xticks = xTickFactory->produce();
		yticks = yTickFactory->produce();
		DrawXAxisTicks(graphics, bounds);
		DrawYAxisTicks(graphics, bounds);
	}

protected:

	void DrawXAxisTicks(Gdiplus::Graphics *graphics, Gdiplus::Rect &bounds)
	{
		Gdiplus::Point start, end;
		Gdiplus::PointF text;

		start.Y = 0;
		end.Y = bounds.Height;

		for (unsigned int i = 0; i < (*xticks).size(); i++)
		{
			start.X = convert(viewport.xmin, viewport.xmax, bounds.Width, (*xticks)[i].value);
			end.X = start.X;
			graphics->DrawLine(pen, start, end);
		}
	}

	void DrawYAxisTicks(Gdiplus::Graphics *graphics, Gdiplus::Rect &bounds)
	{
		Gdiplus::Point start, end;
		Gdiplus::PointF text;

		start.X = 0;
		end.X = bounds.Width;

		for (unsigned int i = 0; i < (*yticks).size(); i++)
		{
			start.Y = bounds.Height - convert(viewport.ymin, viewport.ymax, bounds.Height, (*yticks)[i].value);
			end.Y = start.Y;
			graphics->DrawLine(pen, start, end);
		}
	}
};

kScienceGraphicsNSFooter()

//
///*
//Copyright(C) 2015 Alexander Vasilevsky aka kalaider
//
//This program is free software : you can redistribute it and / or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.If not, see <http://www.gnu.org/licenses/>.
//*/
//
//#pragma once
//
//#include "drawable.h"
//#include "function.h"
//
//#include "aputils.h"
//
//#include <sstream>
//
//kScienceGraphicsNSHeader()
//
//struct Viewport
//{
//	double xmin, xmax, ymin, ymax;
//};
//
//inline int convert(double left, double right, int dim, double x) {
//	return (int)(((x - left) * dim) / (right - left));
//}
//
//class FunctionDrawable :
//	virtual public Drawable
//{
//
//public:
//
//private:
//
//	Function *function;
//	Viewport viewport;
//	int step;
//	Gdiplus::Pen *pen;
//
//public:
//
//	void setViewport(Viewport &viewport)
//	{
//		this->viewport = viewport;
//	}
//
//	FunctionDrawable
//		(
//		Function *function,
//		Viewport viewport,
//		Gdiplus::Pen *pen,
//		int step = 1
//		) :
//		function(function), viewport(viewport), pen(pen), step(step)
//	{
//	}
//
//	virtual void Draw(Gdiplus::Graphics *graphics, Gdiplus::Rect bounds)
//	{
//		graphics->SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
//
//		Gdiplus::Point *points = new Gdiplus::Point[bounds.Width / step + 1];
//
//		int i = 0, j = 0, k; double y = 0, jt;
//		while (i <= bounds.Width / step)
//		{
//			for (k = i; k <= bounds.Width / step; k++) {
//				y = (*function)(viewport.xmin + (viewport.xmax - viewport.xmin) / bounds.Width * k * step);
//				jt = (((y - viewport.ymin) * bounds.Height) / (viewport.ymax - viewport.ymin));
//
//				if (jt >= MAXSHORT || jt <= -MAXSHORT || !std::isfinite(jt))
//				{
//					++k;
//					break;
//				}
//				else
//				{
//					j = (int)jt;
//				}
//				points[k - i].X = k * step; points[k - i].Y = bounds.Height - j;
//			}
//
//			if (k - i - 1 > 0)
//			{
//				graphics->DrawLines(pen, points, k - i - 1);
//			}
//
//			i = k;
//		}
//
//		delete[] points;
//	}
//};
//
//class DataArrayDrawable :
//	virtual public Drawable
//{
//
//public:
//
//	static struct Node
//	{
//		Drawable *drawable;
//		Gdiplus::Rect bounds;
//	};
//
//private:
//
//	const std::map<Point, double> *data;
//	Node nodeTemplate;
//	Viewport viewport;
//
//public:
//
//	void setViewport(Viewport &viewport)
//	{
//		this->viewport = viewport;
//	}
//
//	DataArrayDrawable
//		(
//		std::map<Point, double> *data,
//		Viewport viewport,
//		Node nodeTemplate
//		) :
//		data(data), viewport(viewport), nodeTemplate(nodeTemplate)
//	{
//	}
//
//	virtual void Draw(Gdiplus::Graphics *graphics, Gdiplus::Rect bounds)
//	{
//		double x, y;
//		Gdiplus::Point point;
//
//		for (std::map<Point, double>::const_iterator it = data->cbegin(); it != data->cend(); it++)
//		{
//			x = it->first.x;
//			y = it->second;
//			point.X = convert(viewport.xmin, viewport.xmax, bounds.Width, x);
//			point.Y = bounds.Height - convert(viewport.ymin, viewport.ymax, bounds.Height, y);
//			point.X -= nodeTemplate.bounds.Width / 2;
//			point.Y -= nodeTemplate.bounds.Height / 2;
//
//			Gdiplus::GraphicsState state = graphics->Save();
//
//			graphics->TranslateTransform(static_cast<Gdiplus::REAL>(point.X), static_cast<Gdiplus::REAL>(point.Y));
//			graphics->IntersectClip(nodeTemplate.bounds);
//			nodeTemplate.drawable->Draw(graphics, nodeTemplate.bounds);
//
//			graphics->Restore(state);
//		}
//	}
//};
//
//class FunctionViewportDrawable :
//	virtual public Drawable
//{
//
//public:
//
//	static struct Tick
//	{
//		std::string display;
//		double value;
//	};
//
//	static class TickFactory
//	{
//	public:
//		virtual const std::vector<Tick> * produce() = 0;
//	};
//
//private:
//
//	typedef MultiDrawable::Layer Layer;
//
//	MultiDrawable view;
//	Viewport viewport;
//	Gdiplus::Pen *pen;
//	Gdiplus::Font *font;
//	TickFactory *xTickFactory;
//	TickFactory *yTickFactory;
//
//	/*
//
//	We use pen->GetBrush() which returns "new XXXBrush..."
//	instead of expected and documented backed object
//
//	Quote from MSDN:
//
//	The Pen::GetBrush method gets the Brush object that
//	is currently set for this Pen object.
//
//	*/
//	Gdiplus::Brush *brush; // DELETEME
//
//protected:
//
//	enum class Location {
//		MIN, CENTER, MAX
//	};
//
//	static struct Grid {
//		Location xLocation, yLocation;
//		double x0, y0;
//		int textWidth, textHeight;
//		Gdiplus::Point topLeft;
//		Gdiplus::Point bottomRight;
//
//		Grid
//			(
//			Location xLocation, Location yLocation,
//			double x0, double y0,
//			int textWidth, int textHeight,
//			Gdiplus::Rect &bounds
//			) :
//			xLocation(xLocation), yLocation(yLocation),
//			x0(x0), y0(y0),
//			textWidth(textWidth), textHeight(textHeight)
//		{
//			topLeft.X = (xLocation == Location::MIN) ? textWidth : 0;
//			topLeft.Y = (yLocation == Location::MAX) ? textHeight : 0;
//			bottomRight.X = bounds.Width - ((xLocation == Location::MAX) ? textWidth : 0);
//			bottomRight.Y = bounds.Height - ((yLocation == Location::MIN) ? textHeight : 0);
//		}
//	};
//
//	const std::vector<Tick> *xticks, *yticks;
//	Location xLocation, yLocation;
//	double x0, y0;
//
//public:
//
//	void setViewport(Viewport viewport)
//	{
//		this->viewport = viewport;
//
//		xLocation = Location::MIN;
//		x0 = viewport.xmin;
//		if (viewport.xmin < 0 && viewport.xmax > 0)
//		{
//			x0 = 0;
//			xLocation = Location::CENTER;
//		}
//		else if (viewport.xmax <= 0)
//		{
//			x0 = viewport.xmax;
//			xLocation = Location::MAX;
//		}
//
//		yLocation = Location::MIN;
//		y0 = viewport.ymin;
//		if (viewport.ymin < 0 && viewport.ymax > 0)
//		{
//			y0 = 0;
//			yLocation = Location::CENTER;
//		}
//		else if (viewport.ymax <= 0)
//		{
//			y0 = viewport.ymax;
//			yLocation = Location::MAX;
//		}
//	}
//
//	FunctionViewportDrawable
//		(
//		Drawable *view,
//		Viewport viewport,
//		Gdiplus::Pen *pen,
//		Gdiplus::Font *font,
//		TickFactory *xTickFactory,
//		TickFactory *yTickFactory
//		) :
//		viewport(viewport), pen(pen), font(font), xTickFactory(xTickFactory), yTickFactory(yTickFactory)
//	{
//		this->brush = pen->GetBrush();
//		this->view.GetLayers().push_back(Layer(view));
//		setViewport(viewport);
//	}
//
//	~FunctionViewportDrawable()
//	{
//		delete brush;
//	}
//
//public:
//
//	static class BasicTickFactory :
//		public TickFactory
//	{
//
//	protected:
//
//		apfloat left, right;
//		int maxTicks;
//		bool modified;
//
//		std::vector<Tick> ticks;
//
//		// Draft for pretty printing...
//		static std::string toString(const apfloat &num)
//		{
//			std::ostringstream ss;
//			ss << pretty << num;
//			std::string sss = ss.str();
//			if (sss.find('.') != std::string::npos)
//			{
//				size_t pos = sss.find_last_not_of('0');
//				if (pos != std::string::npos)
//				{
//					sss.resize(pos + 1);
//				}
//			}
//			return sss;
//		}
//
//		static apfloat reduce_interval(const apfloat &num)
//		{
//			unsigned mantissa = kScienceApNS::getnl(num);
//
//			if (mantissa == 5)
//			{
//				return (num / 5) * 2;
//			}
//			else if (mantissa == 2)
//			{
//				return (num / 2);
//			}
//			else if (mantissa == 1)
//			{
//				return (num / 10) * 5;
//			}
//			throw std::logic_error("Mantissa 1, 2 or 5 expected.");
//		}
//
//		static apfloat plain_div(const apfloat &number, const apfloat &interval)
//		{
//			return ap2double(number.ap) / ap2double(interval.ap);
//		}
//
//		static apfloat round_top(const apfloat &number, const apfloat &interval)
//		{
//			return interval * (floor(plain_div(number, interval)) + 1);
//		}
//
//		static apfloat round_bottom(const apfloat &number, const apfloat &interval)
//		{
//			return interval * (ceil(plain_div(number, interval)) - 1);
//		}
//
//	public:
//
//		BasicTickFactory(const apfloat &left, const apfloat &right, int maxTicks = 10) :
//			left(left), right(right), maxTicks(maxTicks), modified(true)
//		{
//		}
//
//		void setLeft(const apfloat &left)
//		{
//			if (this->left == left) return;
//			this->left = left;
//			modified = true;
//		}
//
//		void setRight(const apfloat &right)
//		{
//			if (this->right == right) return;
//			this->right = right;
//			modified = true;
//		}
//
//		void setMaxTicks(int maxTicks)
//		{
//			if (this->maxTicks == maxTicks) return;
//			this->maxTicks = maxTicks;
//			modified = true;
//		}
//
//	protected:
//
//		virtual void calculate() = 0;
//
//	public:
//
//		virtual const std::vector<Tick> * produce()
//		{
//			if (modified) calculate();
//			return &ticks;
//		}
//	};
//
//	static class AutoTickFactory :
//		public BasicTickFactory
//	{
//
//	public:
//
//		AutoTickFactory(const apfloat &left, const apfloat &right, int maxTicks = 10) :
//			BasicTickFactory(left, right, maxTicks)
//		{
//		}
//
//	protected:
//
//		virtual void calculate()
//		{
//			if (left == right) {
//				ticks.resize(0);
//				return;
//			}
//
//			apfloat interval(max_of(kScienceApNS::pow10a(left), kScienceApNS::pow10a(right)));
//			interval.exp(max_of(left.exp() + Basedigits, right.exp() + Basedigits));
//
//			apfloat top = round_top(right, interval), bottom = round_bottom(left, interval);
//
//			apfloat quantity = floor(plain_div((top - bottom), interval));
//
//			while (true)
//			{
//				apfloat next_interval = reduce_interval(interval);
//				apfloat next_top = round_top(right, next_interval), next_bottom = round_bottom(left, next_interval);
//				apfloat next_quantity = floor(plain_div((next_top - next_bottom), next_interval));
//				if (next_quantity > maxTicks) break;
//				quantity = next_quantity;
//				interval = next_interval;
//				top = next_top;
//				bottom = next_bottom;
//			}
//
//			// Last datablock since there should not be anymore - it is an integer (<= maxTicks)
//			unsigned int n = kScienceApNS::toRawtype(quantity);
//
//			ticks.resize(n - 1);
//			for (unsigned int i = 0; i < n - 1; i++)
//			{
//				bottom = bottom + interval;
//				ticks[i] = { toString(bottom), kScienceApNS::toDouble(bottom) };
//			}
//		}
//	};
//
//public:
//
//	virtual void Draw(Gdiplus::Graphics *graphics, Gdiplus::Rect bounds)
//	{
//		graphics->SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintSingleBitPerPixelGridFit);
//
//		// =============== Calculate Grid ==================
//
//		int textWidth = 0, textHeight = 0;
//
//		CString out;
//		Gdiplus::RectF textbounds;
//		Gdiplus::PointF point(0, 0);
//
//		xticks = xTickFactory->produce();
//		yticks = yTickFactory->produce();
//
//		for (unsigned int i = 0; i < yticks->size(); i++)
//		{
//			out = CString((*yticks)[i].display.c_str());
//			graphics->MeasureString(out.GetBuffer(), out.GetLength(), font, point, &textbounds);
//			if (textWidth < ceil(textbounds.Width)) textWidth = static_cast<int>(ceil(textbounds.Width));
//		}
//		textHeight = static_cast<int>(ceil(textbounds.Height));
//
//		Grid grid
//			(
//			xLocation, yLocation,
//			x0,
//			y0,
//			textWidth,
//			textHeight,
//			bounds
//			);
//
//		// =============== Draw View =======================
//
//		view.GetLayers()[0].bounds = Gdiplus::Rect(grid.topLeft.X, grid.topLeft.Y, grid.bottomRight.X - grid.topLeft.X, grid.bottomRight.Y - grid.topLeft.Y);
//		view.Draw(graphics, bounds);
//
//		// =============== Draw Axises =====================
//
//		DrawXAxis(graphics, bounds, grid);
//		DrawYAxis(graphics, bounds, grid);
//
//		DrawXAxisTicks(graphics, bounds, grid);
//		DrawYAxisTicks(graphics, bounds, grid);
//	}
//
//protected:
//
//	void DrawXAxis(Gdiplus::Graphics *graphics, Gdiplus::Rect &bounds, Grid &grid)
//	{
//		Gdiplus::Point start, end;
//
//		start.Y = grid.bottomRight.Y - convert(viewport.ymin, viewport.ymax, grid.bottomRight.Y - grid.topLeft.Y, grid.y0);
//		end.Y = start.Y;
//
//		start.X = grid.topLeft.X;
//		end.X = grid.bottomRight.X;
//
//		graphics->DrawLine(pen, start, end);
//	}
//
//	void DrawXAxisTicks(Gdiplus::Graphics *graphics, Gdiplus::Rect &bounds, Grid &grid)
//	{
//		Gdiplus::Point start, end;
//		Gdiplus::PointF text;
//
//		start.Y = grid.bottomRight.Y - convert(viewport.ymin, viewport.ymax, grid.bottomRight.Y - grid.topLeft.Y, grid.y0);
//		end.Y = start.Y;
//
//		switch (grid.yLocation)
//		{
//		case Location::MIN:
//			start.Y -= 5;
//			text.Y = static_cast<Gdiplus::REAL>(end.Y);
//			break;
//		case Location::MAX:
//			end.Y += 5;
//			text.Y = static_cast<Gdiplus::REAL>(start.Y - grid.textHeight);
//			break;
//		case Location::CENTER:
//			start.Y -= 5;
//			end.Y += 5;
//			text.Y = static_cast<Gdiplus::REAL>(end.Y);
//			break;
//		}
//
//		CString out;
//		Gdiplus::RectF textbounds;
//
//		for (unsigned int i = 0; i < xticks->size(); i++)
//		{
//			start.X = grid.topLeft.X + convert(viewport.xmin, viewport.xmax, grid.bottomRight.X - grid.topLeft.X, (*xticks)[i].value);
//			end.X = start.X;
//			graphics->DrawLine(pen, start, end);
//
//			out = CString((*xticks)[i].display.c_str());
//			graphics->MeasureString(out.GetBuffer(), out.GetLength(), font, text, &textbounds);
//			text.X = start.X - textbounds.Width / 2;
//			graphics->DrawString(out.GetBuffer(), out.GetLength(), font, text, brush);
//
//		}
//	}
//
//	void DrawYAxis(Gdiplus::Graphics *graphics, Gdiplus::Rect &bounds, Grid &grid)
//	{
//		Gdiplus::Point start, end;
//
//		start.X = grid.topLeft.X + convert(viewport.xmin, viewport.xmax, grid.bottomRight.X - grid.topLeft.X, grid.x0);
//		end.X = start.X;
//
//		start.Y = grid.topLeft.Y;
//		end.Y = grid.bottomRight.Y;
//
//		graphics->DrawLine(pen, start, end);
//	}
//
//	void DrawYAxisTicks(Gdiplus::Graphics *graphics, Gdiplus::Rect &bounds, Grid &grid)
//	{
//		Gdiplus::Point start, end;
//		Gdiplus::PointF text;
//
//		start.X = grid.topLeft.X + convert(viewport.xmin, viewport.xmax, grid.bottomRight.X - grid.topLeft.X, grid.x0);
//		end.X = start.X;
//
//		switch (grid.xLocation)
//		{
//		case Location::MIN:
//			end.X += 5;
//			text.X = static_cast<Gdiplus::REAL>(start.X - grid.textWidth);
//			break;
//		case Location::MAX:
//			start.X -= 5;
//			text.X = static_cast<Gdiplus::REAL>(end.X + 1);
//			break;
//		case Location::CENTER:
//			start.X -= 5;
//			end.X += 5;
//			text.X = static_cast<Gdiplus::REAL>(start.X - grid.textWidth);
//			break;
//		}
//
//		CString out;
//
//		for (unsigned int i = 0; i < yticks->size(); i++)
//		{
//			start.Y = grid.bottomRight.Y - convert(viewport.ymin, viewport.ymax, grid.bottomRight.Y - grid.topLeft.Y, (*yticks)[i].value);
//			end.Y = start.Y;
//			graphics->DrawLine(pen, start, end);
//
//			out = CString((*yticks)[i].display.c_str());
//			text.Y = static_cast<Gdiplus::REAL>(start.Y - grid.textHeight / 2);
//			graphics->DrawString(out.GetBuffer(), out.GetLength(), font, text, brush);
//		}
//	}
//};
//
//class FunctionGridDrawable :
//	virtual public Drawable
//{
//
//	typedef FunctionViewportDrawable::TickFactory TickFactory;
//	typedef FunctionViewportDrawable::Tick Tick;
//	typedef MultiDrawable::Layer Layer;
//
//	Viewport viewport;
//	Gdiplus::Pen *pen;
//	TickFactory *xTickFactory;
//	TickFactory *yTickFactory;
//	const std::vector<Tick> *xticks, *yticks;
//
//public:
//
//	void setViewport(Viewport viewport)
//	{
//		this->viewport = viewport;
//	}
//
//	FunctionGridDrawable
//		(
//		Viewport viewport,
//		Gdiplus::Pen *pen,
//		TickFactory *xTickFactory,
//		TickFactory *yTickFactory
//		) :
//		viewport(viewport), pen(pen), xTickFactory(xTickFactory), yTickFactory(yTickFactory)
//	{
//	}
//
//	virtual void Draw(Gdiplus::Graphics *graphics, Gdiplus::Rect bounds)
//	{
//		xticks = xTickFactory->produce();
//		yticks = yTickFactory->produce();
//		DrawXAxisTicks(graphics, bounds);
//		DrawYAxisTicks(graphics, bounds);
//	}
//
//protected:
//
//	void DrawXAxisTicks(Gdiplus::Graphics *graphics, Gdiplus::Rect &bounds)
//	{
//		Gdiplus::Point start, end;
//		Gdiplus::PointF text;
//
//		start.Y = 0;
//		end.Y = bounds.Height;
//
//		for (unsigned int i = 0; i < (*xticks).size(); i++)
//		{
//			start.X = convert(viewport.xmin, viewport.xmax, bounds.Width, (*xticks)[i].value);
//			end.X = start.X;
//			graphics->DrawLine(pen, start, end);
//		}
//	}
//
//	void DrawYAxisTicks(Gdiplus::Graphics *graphics, Gdiplus::Rect &bounds)
//	{
//		Gdiplus::Point start, end;
//		Gdiplus::PointF text;
//
//		start.X = 0;
//		end.X = bounds.Width;
//
//		for (unsigned int i = 0; i < (*yticks).size(); i++)
//		{
//			start.Y = bounds.Height - convert(viewport.ymin, viewport.ymax, bounds.Height, (*yticks)[i].value);
//			end.Y = start.Y;
//			graphics->DrawLine(pen, start, end);
//		}
//	}
//};
//
//kScienceGraphicsNSFooter()