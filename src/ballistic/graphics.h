/**
 * The header contains additional definitions for advanced 2D
 * and definitions for simple 3D graphics.
 */

// TODO: do base change instead of plain multiplicative transformation?

#pragma once

#include "kScience/function.h"
#include "kScience/functionDrawable.h"
#include "common.h"

#include <list>

kScienceGraphicsNSHeader()




//========================================================================
//===================== Class RealTimeFunction ===========================
//========================================================================



/**
 * The class describes a function specified by a number
 * of (x,y) pairs. These pairs are supplied dynamically.
 * 
 * The drawable automatically changes its viewport Y range
 * to display all the points of the function that fit in
 * viewport X range.
 * 
 * Linear interpolation is used to draw the function.
 */
class RealTimeFunctionDrawable : public Drawable
{

private: // Static Inner Classes =========================================

	static struct PointXY
	{
		Point x;
		double y;

		PointXY(Point x, double y) : x(x), y(y)
		{
		}

		PointXY() : PointXY(0, 0)
		{
		}
	};

private: // Fields =======================================================

	std::list<PointXY> data; // An array of points
	Viewport viewport; // The mutable viewport
	// The utility variable to be able to return to initial viewport
	// (see 'clear' method)
	Viewport _oldviewport;
	// The upper and lower bounds the viewport Y range may vary in
	double min_ymin, max_ymax;

	Gdiplus::Pen *pen;

	std::vector<Gdiplus::Point> points; // Cache for Gdiplus points

public: // Constructors ==================================================

	RealTimeFunctionDrawable
		(
		Viewport viewport,
		double min_ymin,
		double max_ymax,
		Gdiplus::Pen *pen)
		: viewport(viewport), _oldviewport(viewport), pen(pen)
	{
		this->min_ymin = min(viewport.ymin, min_ymin);
		this->max_ymax = max(viewport.ymax, max_ymax);
	}

public: // Methods (Accessors) ===========================================

	Viewport getViewport()
	{
		return viewport;
	}

	Viewport getInitialViewport()
	{
		return _oldviewport;
	}

	/**
	 * Sets the new viewport of the drawable.
	 * 
	 * @param viewport The new viewport
	 * @param overrideInitial Determines whether or not the new viewport
	 *                        is temporary or permanent
	 */
	void setVewport(const Viewport &viewport, bool overrideInitial = false)
	{
		this->viewport = viewport;
		if (overrideInitial) this->_oldviewport = viewport;
	}

public: // Methods =======================================================

	/**
	 * The method appends the new data point to this function.
	 * 
	 * If the supplied x is less than the last supplied point,
	 * debug assertion will fail.
	 * 
	 * Returns true if the point was successfully appended. False
	 * otherwise, i.e. when the last supplied value equals the
	 * given one.
	 * 
	 * If the method succeeds, it recalculates viewport of the drawable.
	 */
	bool supply(Point x, double y)
	{
		if (data.size() > 0)
		{
			PointXY tmp;
			tmp = data.back();
			ASSERT(tmp.x <= x);

			if (tmp.x == x) return false;

			tmp = data.front();
			if (tmp.x < viewport.xmin)
			{
				data.pop_front();
			}
		}

		data.push_back({ x, y });

		double delta = viewport.xmax - x.x;
		viewport.xmin -= delta;
		viewport.xmax -= delta;

		if (y > viewport.ymax) viewport.ymax = min(y + (viewport.ymax - viewport.ymin) * 0.1, max_ymax);
		if (y < viewport.ymin) viewport.ymin = max(y - (viewport.ymax - viewport.ymin) * 0.1, min_ymin);

		return true;
	}

	/**
	 * Clears the list of data points.
	 */
	void clear()
	{
		data.clear();
	}

public: // Methods (Inherited) ===========================================

	virtual void Draw(Gdiplus::Graphics *graphics, Gdiplus::Rect bounds) override
	{
		if (data.size() < 2) return;

		graphics->SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);

		if (points.size() < data.size()) points.resize(data.size() * 2);

		int i = 0;
		for (auto it = data.begin(); it != data.end(); it++, i++)
		{
			points[i].X = convert(viewport.xmin, viewport.xmax, bounds.Width, it->x.x);
			points[i].Y = bounds.Height - convert(viewport.ymin, viewport.ymax, bounds.Height, it->y);
		}

		graphics->DrawLines(pen, points.data(), data.size());
	}
};




//========================================================================
//===================== Class Transform ==================================
//========================================================================




/**
 * The class combines translation and transformation.
 * 
 * Logically, it is m4 but it would be better to abstract from such
 * low-level associations in high level API.
 */
struct Transform
{

public: // Fields ========================================================

	v3 origin;
	m3 matrix;
	bool originRelativeToOldTransform;

public: // Constructors ==================================================

	/**
	 * @see derive(const v3 &origin, const m3 &matrix,
	 *             bool originRelativeToOldTransform = false) const
	 */
	Transform
		(
		const v3 &origin,
		const m3 &matrix = identity(),
		bool originRelativeToOldTransform = true
		)
		:
		origin({}),
		matrix(identity()),
		originRelativeToOldTransform(originRelativeToOldTransform)
	{
		this->modify(origin, matrix, originRelativeToOldTransform);
	}

	Transform
		(
		const m3 &matrix = identity()
		)
		: Transform({}, matrix)
	{
	}

public: // Methods =======================================================

   /**
	* Derives the new transformation from this one.
	*
	* The method appends the given origin to the current origin
	* and modifies the current transformation by post-multiplying
	* it to the given matrix.
	* 
	* The origin given is transformed according to the following rules:
	* 
	* a) the passed origin is multiplied by the product of the current
	*    transformation and the new one (if originRelativeToOldTransform is false)
	* 
	* b) the passed origin is multiplied by just the current
	*    transformation (otherwise)
	* 
	* So the meaning of the originRelativeToOldTransform parameter is
	* that it determines whether or not the passed origin is already
	* transformed by the passed matrix.
	*
	* @param origin The offset of the origin relatively to the
	*               current origin and transform
	* @param matrix The transform to be applied to the derived transform
	* @param originRelativeToOldTransform Determines whether or not the
	*        given matrix must be applied to the given origin
	*
	* @return The new transform
	*/
	Transform derive
		(
		const v3 &origin,
		const m3 &matrix,
		bool originRelativeToOldTransform = true
		) const
	{
		m3 m = this->matrix * matrix;
		return
		{
			this->origin + (!originRelativeToOldTransform ? m : this->matrix) * origin,
			m
		};
	}

    /**
	 * @see derive(const v3 &origin, const m3 &matrix,
	 *             bool originRelativeToOldTransform = false) const
	 */
	Transform derive(const Transform &other) const
	{
		return derive(other.origin, other.matrix, other.originRelativeToOldTransform);
	}

    /**
	 * @see derive(const v3 &origin, const m3 &matrix,
	 *             bool originRelativeToOldTransform = false) const
	 * 
	 * The method is analogous to 'derive' but modifies
	 * this transform instead.
	 */
	void modify
		(
		const v3 &origin,
		const m3 &matrix,
		bool originRelativeToOldTransform = true
		)
	{
		m3 m = this->matrix * matrix;
		// Does not need to perform transformation of origin
		if (originRelativeToOldTransform)
		{
			this->origin = this->origin + this->matrix * origin;
		}
		else
		{
			this->origin = this->origin + m * origin;
		}
		this->matrix = m;

		// Does not do this as this property only specified in the constructor
		// this->originRelativeToOldTransform = originRelativeToOldTransform;
	}

   /**
	* The method applies transformation to the point and
	* then adds origin.
	*
	* @param point The point
	*
	* @return The converted point
	*/
	v3 convert(const v3 &point) const
	{
		return matrix * point + origin;
	}
};




//========================================================================
//===================== Class View =======================================
//========================================================================




/**
 * The structure determines the global viewport settings
 * as viewport size, focus, camera location and its
 * direction of view.
 * 
 * The depth given as (focus - camera) determines perspective.
 * It has 1:1 scale if z==camera.
 * 
 * Camera is placed at (0, 0, camera) point. It applies
 * transformation to the scene itself as if it was at (0, 0, 0).
 */
struct View
{
	 
public: // Fields

	// 2D viewport size (pixels)
	int width, height;

	// 3D viewport size
	Viewport viewport;
	double focus;

	// Transformation dictated by camera
	m3 transformation = identity();
	double camera;

public: // Methods

	/**
	 * The method applies camera transformation
	 * to the given point.
	 *
	 * @param point The point
	 *
	 * @return The transformed point
	 */
	v3 convert(const v3 &point) const
	{
		v3 d = transformation * point;
		d.x = (focus - camera) / (focus - d.z) * d.x;
		d.y = (focus - camera) / (focus - d.z) * d.y;
		return d;
	}

	/**
	 * Converts the given 3D point into 2D Gdiplus point.
	 * 
	 * The method DOES NOT apply any transformation
	 * to the point. It must be done manually with
	 * the 'convert' method.
	 * 
	 * @param point 3D point
	 * 
	 * @return 2D point
	 */
	Gdiplus::Point convert2d(const v3 &point) const
	{
		return Gdiplus::Point(
			::convert(viewport.xmin, viewport.xmax, width, point.x),
			::convert(viewport.ymin, viewport.ymax, height, point.y)
			);
	}

};




//========================================================================
//===================== Class Context ====================================
//========================================================================




/**
 * The class describes current offset from (0, 0, 0) point and the
 * transformation to be applied to the current object.
 */
struct Context
{

public: // Fields ========================================================

	// The current transform
	Transform transformation;

	const View &view;

public: // Constructors ==================================================

	/**
	 * Creates context from the given view, origin and transformation
	 */
	Context
		(
		const View &view,
		const Transform& transformation = {}
		)
		: view(view), transformation(transformation)
	{
	}

public: // Methods =======================================================

	/**
	 * The method transforms the given point
	 * according to the current transformation rules
	 * and view settings.
	 *
	 * @param point The point
	 *
	 * @return The transformed point
	 */
	v3 convert(const v3 &point) const
	{
		return view.convert(transformation.convert(point));
	}

	/**
	 * Derives the new context from this one. The new context
	 * is able to properly convert 3D coordinates to 2D coordinates.
	 * 
	 * @param origin The offset of the origin relatively to the
	 *               current transform
	 * @param matrix The transform to be applied to the derived context
	 * @param originRelativeToOldTransform Determines whether or not the
	 *        given matrix must be applied to the given origin
	 * 
	 * @return The new context
	 */
	Context derive(const Transform &transformation) const
	{
		return
		{
			this->view, this->transformation.derive(transformation)
		};
	}

};




//========================================================================
//===================== Class Object =====================================
//========================================================================




/**
 * The class encapsulates basic logic required for any 3D object.
 */
class Object
{

protected: // Fields =====================================================

	// The lightweight transformation
	Transform transformation;

public: // Destructors ===================================================

	virtual ~Object()
	{
	}

public: // Methods (Abstract) ============================================
	
	/**
	 * Paints the object on the given graphics using the specified context.
	 */
	virtual void draw(Gdiplus::Graphics *graphics, const Context &context) = 0;

	/**
	 * Modifies the object in heavyweight way (modifies mesh of the object).
	 */
	virtual void modify(const Transform &transformation) = 0;

public: // Methods (Concrete) ============================================

	/**
	 * Combines the current lightweight transformation with the given one
	 * (without modifying mesh).
	 */
	virtual void transform
		(
		const Transform &transformation
		)
	{
		this->transformation = this->transformation.derive(transformation);
	}

	/**
	 * Sets the lightweight transformation of the object (without modifying mesh).
	 */
	virtual void setTransform(const Transform &transformation = {})
	{
		this->transformation = transformation;
	}

	/**
	 * Sets only one aspect of the lightweight transformation
	 * of the object (without modifying mesh).
	 */
	virtual void setTransform(const m3 &transformation = identity())
	{
		this->transformation.matrix = transformation;
	}

	/**
	* Sets only one aspect of the lightweight transformation
	* of the object (without modifying mesh).
	*/
	virtual void setTransform(const v3 &transformation = {})
	{
		this->transformation.origin = transformation;
	}

	/**
	 * Returns the lightweight transformation of the object.
	 */
	virtual Transform& getTransform() // to allow to modify the transform by-hand
	{
		return this->transformation;
	}
};




//========================================================================
//===================== Class Polyline ===================================
//========================================================================




/**
 * The simple structure defines polyline as array of vertices.
 */
struct Polyline
{
	std::vector<v3> vertices;

   /**
	* Modifies points of polyline.
	*/
	void modify(const Transform &transform)
	{
		for (unsigned int i = 0; i < vertices.size(); i++)
			vertices[i] = transform.convert(vertices[i]);
	}

   /**
	* Simplifies access to vertices.
	*/
	v3& operator[](int i) { return vertices[i]; }
	int size() const { return vertices.size(); }
};




//========================================================================
//===================== Class Polylines ==================================
//========================================================================




/**
* The simple structure defines an array polylines.
*/
struct Polylines
{
	std::vector<Polyline> lines;

   /**
	* Modifies points of polylines.
	*/
	void modify(const Transform &transform)
	{
		for (unsigned int i = 0; i < lines.size(); i++)
			lines[i].modify(transform);
	}

	/**
	* Simplifies access to lines.
	*/
	Polyline& operator[](int i) { return lines[i]; }
	Polylines& append(const Polylines &other)
	{
		lines.insert(lines.end() - 1, other.lines.begin(), other.lines.end());
		return *this;
	}
	int size() const { return lines.size(); }
};




//========================================================================
//===================== Class Mesh =======================================
//========================================================================




/**
 * The class is an Object wrapper over the Polylines.
 * 
 * It adds graphical features and lightweight transformation.
 */
class Mesh : public Object
{

protected: // Fields =====================================================

	Polylines mesh;
	Gdiplus::Pen *pen; // Must be set before 'draw' method is called

public: // Constructors ==================================================

	Mesh()
	{
	}

	/**
	 * Constructs the new Mesh object.
	 * 
	 * Note that pen MUST be specified before 'draw' method is called.
	 */
	Mesh(const Polylines& mesh, Gdiplus::Pen *pen = nullptr) : mesh(mesh), pen(pen)
	{
	}

public: // Methods (Accessors) ===========================================

	Polylines& getMesh() { return this->mesh; }
	Gdiplus::Pen* getPen() { return this->pen; }
	void setPen(Gdiplus::Pen *pen) { this->pen = pen; }

public: // Methods =======================================================

	virtual void modify(const Transform &transform) override
	{
		mesh.modify(transform);
	}

	virtual void draw(Gdiplus::Graphics *graphics, const Context &context) override
	{
		Context new_context = context.derive(this->transformation);
		for (int j = 0; j < mesh.size(); j++)
		{
			Polyline &line = mesh[j];
			Gdiplus::Point *points = new Gdiplus::Point[line.size()];
			for (int k = 0; k < line.size(); k++)
			{
				points[k] = new_context.view.convert2d(new_context.convert(line[k]));
			}
			graphics->DrawLines(pen, points, line.size());
			delete[] points;
		}
	}
};




//========================================================================
//===================== Class Objects ====================================
//========================================================================




/**
 * The class defines a set of objects.
 */
class Objects : public Object
{

protected: // Fields =====================================================

	std::vector<Object *> objects;

public: // Methods (Accessors) ===========================================

	std::vector<Object *>& getObjects() { return this->objects; }

public: // Methods =======================================================

	virtual void modify(const Transform &transform) override
	{
		 for (unsigned int i = 0; i < objects.size(); i++)
			 objects[i]->modify(transform);
	}

	virtual void draw(Gdiplus::Graphics *graphics, const Context &context) override
	{
		Context new_context = context.derive(this->transformation);
		for (unsigned int j = 0; j < objects.size(); j++)
			objects[j]->draw(graphics, new_context);
	}
};




//========================================================================
//===================== Class Curve ======================================
//========================================================================




/**
 * The class implements a geometric 3D curve given parametrically.
 */
class Curve
{

public: // Methods (Abstract) ============================================
	
	/**
	 * The methods returns the {x(p), y(p), z(p)} for a given parameter p.
	 * 
	 * @param p The parameter in range 0..1
	 * 
	 * @return {x(p), y(p), z(p)}
	 */
	virtual v3 operator()(double p) = 0;

public: // Methods (Concrete) ============================================

	/**
	 * Converts the geometric continuous curve into discrete polyline.
	 * 
	 * The methods allows to apply transformation at place.
	 * 
	 * @param segments  The number of segments in the resulting polyline
	 *                  (the number of vertices is greater by 1 for unclosed lines)
	 * @param transform The additional transform
	 * 
	 * @return The Polyline
	 */
	virtual Polyline toPolyline(int segments, const Transform &transform = {})
	{
		Polyline line; line.vertices.resize(segments + 1);
		for (int i = 0; i <= segments; i++)
		{
			line.vertices[i] = transform.convert((*this)((double)i / segments));
		}
		return line;
	}
};




//========================================================================
//===================== Class CircleYZ ===================================
//========================================================================




/**
 * Circle definition. The circle has its center in (0, 0, 0)
 * and lies in YZ plane.
 */
class CircleYZ : public Curve
{

private: // Fields =======================================================

	double radius;

public: // Constructors ==================================================

	CircleYZ(double radius) : radius(radius)
	{
	}

public: // Methods =======================================================

	virtual v3 operator()(double p) override
	{
		// note that p is in range [0, 1]
		return{ 0, radius * sin((1 - 2 * p) * acos(0) * 2), radius * cos((1 - 2 * p) * acos(0) * 2) };
	}
};




//========================================================================
//===================== Factory Functions (Polylines and Objects) ========
//========================================================================



/**
 * Creates sphere using mesh of n parallels and n meridians.
 * 
 * The center of the sphere is (0, 0, 0). 90 degree parallel is
 * located at point (0, 0, +=radius).
 * 
 * The verbosity determines how much vertices the longest
 * circle has (i.e. 0 degrees parallel and all meridians).
 * The verbosity of all other circles (i.e. all parallels except
 * 0 and 90) have less number of vertices.
 * 
 * @param radius    The radius of the sphere
 * @param n         The number of parallels and meridians
 * @param verbosity The number of vertexes in the longest circle
 * 
 * @return The list of polylines
 */
Polylines create_sphere(double radius = 1, int n = 18, int verbosity = 36)
{
	n /= 2;
	CircleYZ circle(radius);
	Polylines obj;
	Transform xy(rotate_y(acos(0))), yz;
	double pin = acos(0) / n;
	for (int i = -n + 1; i <= n - 1; i++)
	{
		xy.origin = { 0, 0, radius * sin(pin * i) };
		yz.matrix = rotate_z(pin * (i + n));
		obj.lines.push_back(
			CircleYZ(radius * cos(pin * i))
			.toPolyline(static_cast<int>(verbosity * (1 + cos(pin * i))), xy)
			);
		obj.lines.push_back(circle.toPolyline(verbosity, yz));
	}
	obj.lines.push_back(circle.toPolyline(verbosity));
	return obj;
}

/**
 * Creates rectangular XY plane (with four vertices).
 *
 * The center of the plane is (0, 0, z). Its vertices
 * are (+-width/2, +-height/2, z).
 * 
 * @param width     The width of the rect
 * @param height    The height of the rect
 * @param z         Z coordinate of each point of the plane
 *
 * @return The list of polylines
 */
Polylines create_plane(double width = 1, double height = 1, double z = 0)
{
	Polylines obj;
	Polyline m1;
	m1.vertices.push_back({ -width / 2, height / 2, z });
	m1.vertices.push_back({ width / 2, height / 2, z });
	m1.vertices.push_back({ width / 2, -height / 2, z });
	m1.vertices.push_back({ -width / 2, -height / 2, z });
	m1.vertices.push_back({ -width / 2, height / 2, z });
	obj.lines.push_back(m1);
	return obj;
}

/**
 * Creates rectangular box (six rectangular planes).
 *
 * The center of the box is (0, 0, 0). Its vertices
 * are (+-width/2, +-height/2, +-depth/2).
 *
 * @param width     The width of the box
 * @param height    The height of the box
 * @param depth     The depth of the box
 *
 * @return The list of polylines
 */
Polylines create_box(double width = 1, double height = 1, double depth = 1)
{
	Polylines plane_xy1 = create_plane(width, height, -depth / 2);
	Polylines plane_xy2 = create_plane(width, height, depth / 2);

	Polylines plane_yz1 = create_plane(depth, height, -width / 2);
	plane_yz1.modify(rotate_y(acos(0)));
	Polylines plane_yz2 = create_plane(depth, height, width / 2);
	plane_yz2.modify(rotate_y(acos(0)));

	Polylines plane_xz1 = create_plane(width, depth, -height / 2);
	plane_xz1.modify(rotate_x(acos(0)));
	Polylines plane_xz2 = create_plane(width, depth, height / 2);
	plane_xz2.modify(rotate_x(acos(0)));

	plane_xy1.append(plane_xy2).append(plane_yz1).append(plane_yz2)
		.append(plane_xz1).append(plane_xz2);

	return plane_xy1;
}

/**
 * Creates XYZ orts of the specified length and color.
 *
 * Each ort starts in (0, 0, 0) and ends at (size, 0, 0) (x ort),
 * (0, size, 0) (y ort) or (0, 0, size) (z ort).
 *
 * @param size The length of each ort
 * @param x    The pen to paint x ort
 * @param y    The pen to paint y ort
 * @param z    The pen to paint z ort
 *
 * @return The object
 */
Objects create_xyz(double size, Gdiplus::Pen *x, Gdiplus::Pen *y, Gdiplus::Pen *z)
{
	Objects xyz;
	Polyline m1;

	Mesh *obj = new Mesh();
	m1.vertices.push_back({ 0, 0, 0 });
	m1.vertices.push_back({ size, 0, 0 });
	obj->getMesh().lines.push_back(m1);
	obj->setPen(x);
	xyz.getObjects().push_back(obj);
	
	obj = new Mesh();
	m1[1] = { 0, size, 0 };
	obj->getMesh().lines.push_back(m1);
	obj->setPen(y);
	xyz.getObjects().push_back(obj);

	obj = new Mesh();
	m1[1] = { 0, 0, size };
	obj->getMesh().lines.push_back(m1);
	obj->setPen(z);
	xyz.getObjects().push_back(obj);

	return xyz;
}




//========================================================================
//===================== Class SceneDrawable ==============================
//========================================================================




/**
 * The drawable to draw 3D scene.
 */
class SceneDrawable : public Drawable
{

private: // Fields =======================================================

	Viewport viewport;
	Objects &scene;
	double camera, focus;
	m3 transformation;
	bool antialiased;

public: // Constructors ==================================================

	SceneDrawable
		(
		Viewport viewport,
		Objects &scene,
		double camera,
		double focus
		)
		:
		viewport(viewport),
		scene(scene),
		camera(camera),
		focus(focus),
		transformation(identity()),
		antialiased(true)
	{
	}

public: // Methods (Accessors) ===========================================

	/**
	 * Sets the lightweight transformation of the scene.
	 */
	virtual void setTransform(const m3 &transformation = identity())
	{
		this->transformation = transformation;
	}

	/**
	 * Returns the transformation of the scene.
	 */
	virtual m3 getTransform() // to allow to modify the transform by-hand
	{
		return this->transformation;
	}

	virtual void set3DViewport(const Viewport &viewport, double camera, double focus)
	{
		this->viewport = viewport;
		this->camera = camera;
		this->focus = focus;
	}

	virtual void setAntiAliased(bool flag)
	{
		this->antialiased = flag;
	}

	virtual bool isAntiAliased()
	{
		return this->antialiased;
	}

public: // Methods =======================================================

	virtual void Draw(Gdiplus::Graphics *graphics, Gdiplus::Rect bounds) override
	{
		if (antialiased) graphics->SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
		View view;
		view.viewport = viewport;
		view.camera = camera;
		view.focus = focus;
		view.height = bounds.Height;
		view.width = bounds.Width;
		view.transformation = transformation;
		scene.draw(graphics, Context(view));
	}
};

kScienceGraphicsNSFooter()