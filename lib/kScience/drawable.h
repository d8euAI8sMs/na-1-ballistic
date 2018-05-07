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

#include "kScience.h"
#include "graphics.h"
#include <vector>

kScienceGraphicsNSHeader()

/// <summary>
///
///     The object that can draw itself.
///
/// </summary>
class Drawable
{

public:

	/// <summary>
	///
	///     Draws itself on the given graphics inside the given bounds.
	///
	/// <param>
	///     The fact that bounds may have X and Y coordinates other that 0
	///     is rather exceptional. Plain user must not carry about it
	///     and draw as if X and Y are 0.
	/// </param>
	///
	/// <param>
	///     The most common case is that the obect is drawn by <c>MultiDrawable</c>
	///     which passes offscreen buffer instead of real graphics
	///     and sets X and Y coordinates to 0.
	/// </param>
	///
	/// </summary>
	virtual void Draw(Gdiplus::Graphics *graphics, Gdiplus::Rect bounds)
	{
	}

public:

	virtual ~Drawable()
	{
	}
};

/// <summary>
///
///     The class combines multiple drawables, or layers.
///
/// <param>
///     Each layer is drawn on its own offscreen buffer first.
///     So, the first, it doesn't have to clean up graphics object,
///     and, the second, its drawing bounds always have no X and Y (X=Y=0).
/// </param>
///
/// <param>
///     <c>MultiDrawable</c> takes a vector of <c>Layers</c>.
///     Layer is a struct with fields: <c>drawable</c>,
///     <c>bounds</c> and <c>visible</c>. If layer is invisible,
///     it will not be drawn.
/// </param>
///
/// <param>
///     There is a special case for bounds field. If it is a point
///     (width=height=0), the layer takes the same size as this
///     multidrawable and is drawn at the point (0,0).
/// </param>
///
/// </summary>
class MultiDrawable :
	virtual public Drawable
{

public:

	static struct Layer {
		Drawable *drawable;
		Gdiplus::Rect bounds;
		BOOL visible;
		Layer
			(
			Drawable *drawable,
			Gdiplus::Rect bounds = Gdiplus::Rect(0, 0, 0, 0),
			BOOL visible = true
			) :
			drawable(drawable), bounds(bounds), visible(visible)
		{
		}
	};

private:

	std::vector<Layer> layers;

public:

	std::vector<Layer> & GetLayers()
	{
		return layers;
	}

	virtual void Draw(Gdiplus::Graphics *graphics, Gdiplus::Rect bounds)
	{
		for (std::vector<Layer>::iterator it = GetLayers().begin();
			it < GetLayers().end();
			it++) {
			if ((*it).visible)
			{
				Gdiplus::Rect newrect(0, 0, (*it).bounds.Width, (*it).bounds.Height);
				// Special case of layer bounds
				if (newrect.Width == 0 || newrect.Height == 0)
				{
					newrect.Width = bounds.Width;
					newrect.Height = bounds.Height;
				}

				Gdiplus::GraphicsState state = graphics->Save();

				graphics->TranslateTransform(Gdiplus::REAL((*it).bounds.X), Gdiplus::REAL((*it).bounds.Y));
				graphics->IntersectClip(newrect);
				(*it).drawable->Draw(graphics, newrect);

				graphics->Restore(state);
			}
		}
	}
};

/// <summary>
///
///     The drawable fills background with the provided brush.
///
/// </summary>
class BackgroundDrawable :
	virtual public Drawable
{

	const Gdiplus::Brush *brush;

public:

	BackgroundDrawable(Gdiplus::Brush *brush) :
		brush(brush)
	{
	}

	virtual void Draw(Gdiplus::Graphics *graphics, Gdiplus::Rect bounds)
	{
		graphics->FillRectangle(brush, bounds);
	}
};

/// <summary>
///
///     The drawable fills elipse inside the passed bounds.
///
/// </summary>
class EllipseDrawable :
	virtual public Drawable
{

	const Gdiplus::Brush *brush;

public:

	EllipseDrawable(Gdiplus::Brush *brush) :
		brush(brush)
	{
	}

	virtual void Draw(Gdiplus::Graphics *graphics, Gdiplus::Rect bounds)
	{
		graphics->FillEllipse(brush, bounds);
	}
};

kScienceGraphicsNSFooter()