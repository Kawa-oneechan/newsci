/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "NewSCI.h"

//#define DEBUG_MERGEPOLY

template<typename T> inline void SWAP(T &a, T &b) { T tmp = a; a = b; b = tmp; }
template<typename T> inline T MIN(T a, T b) { return (a < b) ? a : b; }
template<typename T> inline T MAX(T a, T b) { return (a > b) ? a : b; }
template<typename T> inline T CLIP(T v, T amin, T amax)
{
	if (v < amin) return amin; else if (v > amax) return amax; else return v;
}

namespace Common {
	// Convert radians to degrees
	// Input and Output type can be different
	// Upconvert everything to floats
	template<class InputT, class OutputT>
	inline OutputT rad2deg(InputT rad) {
		return (OutputT)((float)rad * (float)57.2957795130823); // 180.0/M_PI = 57.2957795130823
	}

	// Handle the case differently when the input type is double
	template<class OutputT>
	inline OutputT rad2deg(double rad) {
		return (OutputT)(rad * 57.2957795130823);
	}

	// Convert radians to degrees
	// Input and Output type are the same
	template<class T>
	inline T rad2deg(T rad) {
		return rad2deg<T, T>(rad);
	}

	// Convert degrees to radians
	// Input and Output type can be different
	// Upconvert everything to floats
	template<class InputT, class OutputT>
	inline OutputT deg2rad(InputT deg) {
		return (OutputT)((float)deg * (float)0.0174532925199433); // M_PI/180.0 = 0.0174532925199433
	}

	template<typename T> class List;

	namespace ListInternal {
		struct NodeBase {
			NodeBase *_prev;
			NodeBase *_next;
		};

		template<typename T>
		struct Node : public NodeBase {
			T _data;

			Node(const T &x) : _data(x) {}
		};

		template<typename T> struct ConstIterator;

		template<typename T>
		struct Iterator {
			typedef Iterator<T>	Self;
			typedef Node<T> *	NodePtr;
			typedef T &			ValueRef;
			typedef T *			ValuePtr;
			typedef T			ValueType;

			NodeBase *_node;

			Iterator() : _node(nullptr) {}
			explicit Iterator(NodeBase *node) : _node(node) {}

			// Prefix inc
			Self &operator++() {
				if (_node)
					_node = _node->_next;
				return *this;
			}
			// Postfix inc
			Self operator++(int) {
				Self tmp(_node);
				++(*this);
				return tmp;
			}
			// Prefix dec
			Self &operator--() {
				if (_node)
					_node = _node->_prev;
				return *this;
			}
			// Postfix dec
			Self operator--(int) {
				Self tmp(_node);
				--(*this);
				return tmp;
			}
			ValueRef operator*() const {
				assert(_node);
				return static_cast<NodePtr>(_node)->_data;
			}
			ValuePtr operator->() const {
				return &(operator*());
			}

			bool operator==(const Self &x) const {
				return _node == x._node;
			}

			bool operator!=(const Self &x) const {
				return _node != x._node;
			}
		};

		template<typename T>
		struct ConstIterator {
			typedef ConstIterator<T>	Self;
			typedef const Node<T> *	NodePtr;
			typedef const T &		ValueRef;
			typedef const T *		ValuePtr;

			const NodeBase *_node;

			ConstIterator() : _node(nullptr) {}
			explicit ConstIterator(const NodeBase *node) : _node(node) {}
			ConstIterator(const Iterator<T> &x) : _node(x._node) {}

			// Prefix inc
			Self &operator++() {
				if (_node)
					_node = _node->_next;
				return *this;
			}
			// Postfix inc
			Self operator++(int) {
				Self tmp(_node);
				++(*this);
				return tmp;
			}
			// Prefix dec
			Self &operator--() {
				if (_node)
					_node = _node->_prev;
				return *this;
			}
			// Postfix dec
			Self operator--(int) {
				Self tmp(_node);
				--(*this);
				return tmp;
			}
			ValueRef operator*() const {
				assert(_node);
				return static_cast<NodePtr>(_node)->_data;
			}
			ValuePtr operator->() const {
				return &(operator*());
			}

			bool operator==(const Self &x) const {
				return _node == x._node;
			}

			bool operator!=(const Self &x) const {
				return _node != x._node;
			}
		};


		template<typename T>
		bool operator==(const Iterator<T>& a, const ConstIterator<T>& b) {
			return a._node == b._node;
		}

		template<typename T>
		bool operator!=(const Iterator<T>& a, const ConstIterator<T>& b) {
			return a._node != b._node;
		}
	}

	/**
	* Simple double linked list, modeled after the list template of the standard
	* C++ library.
	*/
	template<typename t_T>
	class List {
	protected:
		typedef ListInternal::NodeBase		NodeBase;
		typedef ListInternal::Node<t_T>		Node;

		NodeBase _anchor;

	public:
		typedef ListInternal::Iterator<t_T>		iterator;
		typedef ListInternal::ConstIterator<t_T>	const_iterator;

		typedef t_T value_type;
		typedef unsigned int size_type;

	public:
		List() {
			_anchor._prev = &_anchor;
			_anchor._next = &_anchor;
		}
		List(const List<t_T> &list) {
			_anchor._prev = &_anchor;
			_anchor._next = &_anchor;

			insert(begin(), list.begin(), list.end());
		}

		~List() {
			clear();
		}

		/**
		* Inserts element before pos.
		*/
		void insert(iterator pos, const t_T &element) {
			insert(pos._node, element);
		}

		/**
		* Inserts the elements from first to last before pos.
		*/
		template<typename iterator2>
		void insert(iterator pos, iterator2 first, iterator2 last) {
			for (; first != last; ++first)
				insert(pos, *first);
		}

		/**
		* Deletes the element at location pos and returns an iterator pointing
		* to the element after the one which was deleted.
		*/
		iterator erase(iterator pos) {
			assert(pos != end());
			return iterator(erase(pos._node)._next);
		}

		/**
		* Deletes the element at location pos and returns an iterator pointing
		* to the element before the one which was deleted.
		*/
		iterator reverse_erase(iterator pos) {
			assert(pos != end());
			return iterator(erase(pos._node)._prev);
		}

		/**
		* Deletes the elements between first and last (including first but not
		* last) and returns an iterator pointing to the element after the one
		* which was deleted (i.e., last).
		*/
		iterator erase(iterator first, iterator last) {
			NodeBase *f = first._node;
			NodeBase *l = last._node;
			while (f != l)
				f = erase(f)._next;
			return last;
		}

		/**
		* Removes all elements that are equal to val from the list.
		*/
		void remove(const t_T &val) {
			NodeBase *i = _anchor._next;
			while (i != &_anchor)
				if (val == static_cast<Node *>(i)->_data)
					i = erase(i)._next;
				else
					i = i->_next;
		}

		/** Inserts element at the start of the list. */
		void push_front(const t_T &element) {
			insert(_anchor._next, element);
		}

		/** Appends element to the end of the list. */
		void push_back(const t_T &element) {
			insert(&_anchor, element);
		}

		/** Removes the first element of the list. */
		void pop_front() {
			assert(!empty());
			erase(_anchor._next);
		}

		/** Removes the last element of the list. */
		void pop_back() {
			assert(!empty());
			erase(_anchor._prev);
		}

		/** Returns a reference to the first element of the list. */
		t_T &front() {
			return static_cast<Node *>(_anchor._next)->_data;
		}

		/** Returns a reference to the first element of the list. */
		const t_T &front() const {
			return static_cast<Node *>(_anchor._next)->_data;
		}

		/** Returns a reference to the last element of the list. */
		t_T &back() {
			return static_cast<Node *>(_anchor._prev)->_data;
		}

		/** Returns a reference to the last element of the list. */
		const t_T &back() const {
			return static_cast<Node *>(_anchor._prev)->_data;
		}

		List<t_T> &operator=(const List<t_T> &list) {
			if (this != &list) {
				iterator i;
				const iterator e = end();
				const_iterator i2;
				const_iterator e2 = list.end();

				for (i = begin(), i2 = list.begin(); (i != e) && (i2 != e2); ++i, ++i2) {
					static_cast<Node *>(i._node)->_data = static_cast<const Node *>(i2._node)->_data;
				}

				if (i == e)
					insert(i, i2, e2);
				else
					erase(i, e);
			}

			return *this;
		}

		size_type size() const {
			size_type n = 0;
			for (const NodeBase *cur = _anchor._next; cur != &_anchor; cur = cur->_next)
				++n;
			return n;
		}

		void clear() {
			NodeBase *pos = _anchor._next;
			while (pos != &_anchor) {
				Node *node = static_cast<Node *>(pos);
				pos = pos->_next;
				delete node;
			}

			_anchor._prev = &_anchor;
			_anchor._next = &_anchor;
		}

		bool empty() const {
			return (&_anchor == _anchor._next);
		}


		iterator		begin() {
			return iterator(_anchor._next);
		}

		iterator		reverse_begin() {
			return iterator(_anchor._prev);
		}

		iterator		end() {
			return iterator(&_anchor);
		}

		const_iterator	begin() const {
			return const_iterator(_anchor._next);
		}

		const_iterator	reverse_begin() const {
			return const_iterator(_anchor._prev);
		}

		const_iterator	end() const {
			return const_iterator(const_cast<NodeBase *>(&_anchor));
		}

	protected:
		NodeBase erase(NodeBase *pos) {
			NodeBase n = *pos;
			Node *node = static_cast<Node *>(pos);
			n._prev->_next = n._next;
			n._next->_prev = n._prev;
			delete node;
			return n;
		}

		/**
		* Inserts element before pos.
		*/
		void insert(NodeBase *pos, const t_T &element) {
			ListInternal::NodeBase *newNode = new Node(element);
			assert(newNode);

			newNode->_next = pos;
			newNode->_prev = pos->_prev;
			newNode->_prev->_next = newNode;
			newNode->_next->_prev = newNode;
		}
	};

	struct Point {
		int16_t x;	///< The horizontal part of the point
		int16_t y;	///< The vertical part of the point

		Point() : x(0), y(0) {}
		Point(int16_t x1, int16_t y1) : x(x1), y(y1) {}
		bool  operator==(const Point &p)    const { return x == p.x && y == p.y; }
		bool  operator!=(const Point &p)    const { return x != p.x || y != p.y; }
		Point operator+(const Point &delta) const { return Point(x + delta.x, y + delta.y); }
		Point operator-(const Point &delta) const { return Point(x - delta.x, y - delta.y); }

		void operator+=(const Point &delta) {
			x += delta.x;
			y += delta.y;
		}

		void operator-=(const Point &delta) {
			x -= delta.x;
			y -= delta.y;
		}

		/**
		* Return the square of the distance between this point and the point p.
		*
		* @param p		the other point
		* @return the distance between this and p
		*/
		unsigned int sqrDist(const Point &p) const {
			int diffx = labs(p.x - x);
			if (diffx >= 0x1000)
				return 0xFFFFFF;

			int diffy = labs(p.y - y);
			if (diffy >= 0x1000)
				return 0xFFFFFF;

			return unsigned int(diffx * diffx + diffy * diffy);
		}
	};

	/**
	* Simple class for handling a rectangular zone.
	*
	* Note: This implementation is built around the assumption that (top,left) is
	* part of the rectangle, but (bottom,right) is not. This is reflected in
	* various methods, including contains(), intersects() and others.
	*
	* Another very wide spread approach to rectangle classes treats (bottom,right)
	* also as a part of the rectangle.
	*
	* Conceptually, both are sound, but the approach we use saves many intermediate
	* computations (like computing the height in our case is done by doing this:
	*   height = bottom - top;
	* while in the alternate system, it would be
	*   height = bottom - top + 1;
	*
	* When writing code using our Rect class, always keep this principle in mind!
	*/
	struct Rect {
		int16_t top, left;		///< The point at the top left of the rectangle (part of the rect).
		int16_t bottom, right;	///< The point at the bottom right of the rectangle (not part of the rect).

		Rect() : top(0), left(0), bottom(0), right(0) {}
		Rect(int16_t w, int16_t h) : top(0), left(0), bottom(h), right(w) {}
		Rect(int16_t x1, int16_t y1, int16_t x2, int16_t y2) : top(y1), left(x1), bottom(y2), right(x2) {
			assert(isValidRect());
		}
		bool operator==(const Rect &rhs) const { return equals(rhs); }
		bool operator!=(const Rect &rhs) const { return !equals(rhs); }

		int16_t width() const { return right - left; }
		int16_t height() const { return bottom - top; }

		void setWidth(int16_t aWidth) {
			right = left + aWidth;
		}

		void setHeight(int16_t aHeight) {
			bottom = top + aHeight;
		}

		/**
		* Check if given position is inside this rectangle.
		*
		* @param x the horizontal position to check
		* @param y the vertical position to check
		*
		* @return true if the given position is inside this rectangle, false otherwise
		*/
		bool contains(int16_t x, int16_t y) const {
			return (left <= x) && (x < right) && (top <= y) && (y < bottom);
		}

		/**
		* Check if given point is inside this rectangle.
		*
		* @param p the point to check
		*
		* @return true if the given point is inside this rectangle, false otherwise
		*/
		bool contains(const Point &p) const {
			return contains(p.x, p.y);
		}

		/**
		* Check if the given rect is contained inside this rectangle.
		*
		* @param r The rectangle to check
		*
		* @return true if the given rect is inside, false otherwise
		*/
		bool contains(const Rect &r) const {
			return (left <= r.left) && (r.right <= right) && (top <= r.top) && (r.bottom <= bottom);
		}

		/**
		* Check if the given rect is equal to this one.
		*
		* @param r The rectangle to check
		*
		* @return true if the given rect is equal, false otherwise
		*/
		bool equals(const Rect &r) const {
			return (left == r.left) && (right == r.right) && (top == r.top) && (bottom == r.bottom);
		}

		/**
		* Check if given rectangle intersects with this rectangle
		*
		* @param r the rectangle to check
		*
		* @return true if the given rectangle has a non-empty intersection with
		*         this rectangle, false otherwise
		*/
		bool intersects(const Rect &r) const {
			return (left < r.right) && (r.left < right) && (top < r.bottom) && (r.top < bottom);
		}

		/**
		* Find the intersecting rectangle between this rectangle and the given rectangle
		*
		* @param r the intersecting rectangle
		*
		* @return the intersection of the rectangles or an empty rectangle if not intersecting
		*/
		Rect findIntersectingRect(const Rect &r) const {
			if (!intersects(r))
				return Rect();

			return Rect(MAX(r.left, left), MAX(r.top, top), MIN(r.right, right), MIN(r.bottom, bottom));
		}

		/**
		* Extend this rectangle so that it contains r
		*
		* @param r the rectangle to extend by
		*/
		void extend(const Rect &r) {
			left = MIN(left, r.left);
			right = MAX(right, r.right);
			top = MIN(top, r.top);
			bottom = MAX(bottom, r.bottom);
		}

		/**
		* Extend this rectangle in all four directions by the given number of pixels
		*
		* @param offset the size to grow by
		*/
		void grow(int16_t offset) {
			top -= offset;
			left -= offset;
			bottom += offset;
			right += offset;
		}

		void clip(const Rect &r) {
			assert(isValidRect());
			assert(r.isValidRect());

			if (top < r.top) top = r.top;
			else if (top > r.bottom) top = r.bottom;

			if (left < r.left) left = r.left;
			else if (left > r.right) left = r.right;

			if (bottom > r.bottom) bottom = r.bottom;
			else if (bottom < r.top) bottom = r.top;

			if (right > r.right) right = r.right;
			else if (right < r.left) right = r.left;
		}

		void clip(int16_t maxw, int16_t maxh) {
			clip(Rect(0, 0, maxw, maxh));
		}

		bool isEmpty() const {
			return (left >= right || top >= bottom);
		}

		bool isValidRect() const {
			return (left <= right && top <= bottom);
		}

		void moveTo(int16_t x, int16_t y) {
			bottom += y - top;
			right += x - left;
			top = y;
			left = x;
		}

		void translate(int16_t dx, int16_t dy) {
			left += dx; right += dx;
			top += dy; bottom += dy;
		}

		void moveTo(const Point &p) {
			moveTo(p.x, p.y);
		}

		void debugPrint(int debuglevel = 0, const char *caption = "Rect:") const {
			SDL_Log("%s %d, %d, %d, %d", caption, left, top, right, bottom);
		}

		/**
		* Create a rectangle around the given center.
		* @note the center point is rounded up and left when given an odd width and height
		*/
		static Rect center(int16_t cx, int16_t cy, int16_t w, int16_t h) {
			int x = cx - w / 2, y = cy - h / 2;
			return Rect(x, y, x + w, y + h);
		}
	};
}

namespace Sci {

// TODO: Code cleanup

#define AVOIDPATH_DYNMEM_STRING "AvoidPath polyline"

#define POLY_LAST_POINT 0x7777
#define POLY_POINT_SIZE 4

// SCI-defined polygon types
enum {
	POLY_TOTAL_ACCESS = 0,
	POLY_NEAREST_ACCESS = 1,
	POLY_BARRED_ACCESS = 2,
	POLY_CONTAINED_ACCESS = 3
};

// Polygon containment types
enum {
	CONT_OUTSIDE = 0,
	CONT_ON_EDGE = 1,
	CONT_INSIDE = 2
};

#define HUGE_DISTANCE 0xFFFFFFFF

#define VERTEX_HAS_EDGES(V) ((V) != CLIST_NEXT(V))

// Error codes
enum {
	PF_OK = 0,
	PF_ERROR = -1,
	PF_FATAL = -2
};

// Floating point struct
struct FloatPoint {
	FloatPoint() : x(0), y(0) {}
	FloatPoint(float x_, float y_) : x(x_), y(y_) {}
	FloatPoint(Common::Point p) : x(p.x), y(p.y) {}

	Common::Point toPoint() {
		return Common::Point((int16_t)(x + 0.5), (int16_t)(y + 0.5));
	}

	float operator*(const FloatPoint &p) const {
		return x*p.x + y*p.y;
	}
	FloatPoint operator*(float l) const {
		return FloatPoint(l*x, l*y);
	}
	FloatPoint operator-(const FloatPoint &p) const {
		return FloatPoint(x-p.x, y-p.y);
	}
	float norm() const {
		return x*x+y*y;
	}

	float x, y;
};

struct Vertex {
	// Location
	Common::Point v;

	// Vertex circular list entry
	Vertex *_next;	// next element
	Vertex *_prev;	// previous element

	// A* cost variables
	uint32_t costF;
	uint32_t costG;

	// Previous vertex in shortest path
	Vertex *path_prev;

public:
	Vertex(const Common::Point &p) : v(p) {
		costG = HUGE_DISTANCE;
		path_prev = NULL;
	}
};

class VertexList: public Common::List<Vertex *> {
public:
	bool contains(Vertex *v) {
		for (iterator it = begin(); it != end(); ++it) {
			if (v == *it)
				return true;
		}
		return false;
	}
};

/* Circular list definitions. */

#define CLIST_FOREACH(var, head)					\
	for ((var) = (head)->first();					\
		(var);							\
		(var) = ((var)->_next == (head)->first() ?	\
		    NULL : (var)->_next))

/* Circular list access methods. */
#define CLIST_NEXT(elm)		((elm)->_next)
#define CLIST_PREV(elm)		((elm)->_prev)

class CircularVertexList {
public:
	Vertex *_head;

public:
	CircularVertexList() : _head(0) {}

	Vertex *first() const {
		return _head;
	}

	void insertAtEnd(Vertex *elm) {
		if (_head == NULL) {
			elm->_next = elm->_prev = elm;
			_head = elm;
		} else {
			elm->_next = _head;
			elm->_prev = _head->_prev;
			_head->_prev = elm;
			elm->_prev->_next = elm;
		}
	}

	void insertHead(Vertex *elm) {
		insertAtEnd(elm);
		_head = elm;
	}

	static void insertAfter(Vertex *listelm, Vertex *elm) {
		elm->_prev = listelm;
		elm->_next = listelm->_next;
		listelm->_next->_prev = elm;
		listelm->_next = elm;
	}

	void remove(Vertex *elm) {
		if (elm->_next == elm) {
			_head = NULL;
		} else {
			if (_head == elm)
				_head = elm->_next;
			elm->_prev->_next = elm->_next;
			elm->_next->_prev = elm->_prev;
		}
	}

	bool empty() const {
		return _head == NULL;
	}

	unsigned int size() const {
		int n = 0;
		Vertex *v;
		CLIST_FOREACH(v, this)
			++n;
		return n;
	}

	/**
	 * Reverse the order of the elements in this circular list.
	 */
	void reverse() {
		if (!_head)
			return;

		Vertex *elm = _head;
		do {
			SWAP(elm->_prev, elm->_next);
			elm = elm->_next;
		} while (elm != _head);
	}
};

struct Polygon {
	// SCI polygon type
	int type;

	// Circular list of vertices
	CircularVertexList vertices;

public:
	Polygon(int t) : type(t) {
	}

	~Polygon() {
		while (!vertices.empty()) {
			Vertex *vertex = vertices.first();
			vertices.remove(vertex);
			delete vertex;
		}
	}
};

typedef Common::List<Polygon *> PolygonList;

// Pathfinding state
struct PathfindingState {
	// List of all polygons
	PolygonList polygons;

	// Start and end points for pathfinding
	Vertex *vertex_start, *vertex_end;

	// Array of all vertices, used for sorting
	Vertex **vertex_index;

	// Total number of vertices
	int vertices;

	// Point to prepend and append to final path
	Common::Point *_prependPoint;
	Common::Point *_appendPoint;

	// Screen size
	int _width, _height;

	PathfindingState(int width, int height) : _width(width), _height(height) {
		vertex_start = NULL;
		vertex_end = NULL;
		vertex_index = NULL;
		_prependPoint = NULL;
		_appendPoint = NULL;
		vertices = 0;
	}

	~PathfindingState() {
		free(vertex_index);

		delete _prependPoint;
		delete _appendPoint;

		for (PolygonList::iterator it = polygons.begin(); it != polygons.end(); ++it) {
			delete *it;
		}
	}

	bool pointOnScreenBorder(const Common::Point &p);
	bool edgeOnScreenBorder(const Common::Point &p, const Common::Point &q);
	int findNearPoint(const Common::Point &p, Polygon *polygon, Common::Point *ret);
};

static Common::Point readPoint(uint16_t* list_r, int offset) {
	Common::Point point;

	point.x = list_r[offset];
	point.y = list_r[offset + 1];

	return point;
}

static void writePoint(uint16_t* ref, int offset, const Common::Point &point) {
	ref[offset] = point.x;
	ref[offset + 1] = point.y;
}

/*
static void draw_line(EngineState *s, Common::Point p1, Common::Point p2, int type, int width, int height) {
	// Colors for polygon debugging.
	// Green: Total access
	// Blue: Near-point access
	// Red : Barred access
	// Yellow: Contained access
	int poly_colors[4] = { 0, 0, 0, 0 };

	if (getSciVersion() <= SCI_VERSION_1_1) {
		poly_colors[0] = g_sci->_gfxPalette16->kernelFindColor(0, 255, 0);		// green
		poly_colors[1] = g_sci->_gfxPalette16->kernelFindColor(0, 0, 255);		// blue
		poly_colors[2] = g_sci->_gfxPalette16->kernelFindColor(255, 0, 0);		// red
		poly_colors[3] = g_sci->_gfxPalette16->kernelFindColor(255, 255, 0);	// yellow
#ifdef ENABLE_SCI32
	} else {
		poly_colors[0] = g_sci->_gfxPalette32->matchColor(0, 255, 0);			// green
		poly_colors[1] = g_sci->_gfxPalette32->matchColor(0, 0, 255);			// blue
		poly_colors[2] = g_sci->_gfxPalette32->matchColor(255, 0, 0);			// red
		poly_colors[3] = g_sci->_gfxPalette32->matchColor(255, 255, 0);			// yellow
#endif
	}

	// Clip
	// FIXME: Do proper line clipping
	p1.x = CLIP<int16_t>(p1.x, 0, width - 1);
	p1.y = CLIP<int16_t>(p1.y, 0, height - 1);
	p2.x = CLIP<int16_t>(p2.x, 0, width - 1);
	p2.y = CLIP<int16_t>(p2.y, 0, height - 1);

	assert(type >= 0 && type <= 3);

	if (getSciVersion() <= SCI_VERSION_1_1) {
		g_sci->_gfxPaint16->kernelGraphDrawLine(p1, p2, poly_colors[type], 255, 255);
#ifdef ENABLE_SCI32
	} else {
		Plane *topPlane = g_sci->_gfxFrameout->getTopVisiblePlane();
		g_sci->_gfxPaint32->kernelAddLine(topPlane->_object, p1, p2, 255, poly_colors[type], kLineStyleSolid, 0, 1);
#endif
	}
}

static void draw_point(EngineState *s, Common::Point p, int start, int width, int height) {
	// Colors for starting and end point
	// Green: End point
	// Blue: Starting point
	int point_colors[2] = { 0, 0 };

	if (getSciVersion() <= SCI_VERSION_1_1) {
		point_colors[0] = g_sci->_gfxPalette16->kernelFindColor(0, 255, 0);	// green
		point_colors[1] = g_sci->_gfxPalette16->kernelFindColor(0, 0, 255);	// blue
#ifdef ENABLE_SCI32
	} else {
		point_colors[0] = g_sci->_gfxPalette32->matchColor(0, 255, 0);		// green
		point_colors[1] = g_sci->_gfxPalette32->matchColor(0, 0, 255);		// blue
#endif
	}

	Common::Rect rect = Common::Rect(p.x - 1, p.y - 1, p.x - 1 + 3, p.y - 1 + 3);

	// Clip
	rect.top = CLIP<int16_t>(rect.top, 0, height - 1);
	rect.bottom = CLIP<int16_t>(rect.bottom, 0, height - 1);
	rect.left = CLIP<int16_t>(rect.left, 0, width - 1);
	rect.right = CLIP<int16_t>(rect.right, 0, width - 1);

	assert(start >= 0 && start <= 1);
	if (getSciVersion() <= SCI_VERSION_1_1) {
		g_sci->_gfxPaint16->kernelGraphFrameBox(rect, point_colors[start]);
#ifdef ENABLE_SCI32
	} else {
		Plane *topPlane = g_sci->_gfxFrameout->getTopVisiblePlane();
		g_sci->_gfxPaint32->kernelAddLine(topPlane->_object, Common::Point(rect.left, rect.top), Common::Point(rect.right, rect.top), 255, point_colors[start], kLineStyleSolid, 0, 1);
		g_sci->_gfxPaint32->kernelAddLine(topPlane->_object, Common::Point(rect.right, rect.top), Common::Point(rect.right, rect.bottom), 255, point_colors[start], kLineStyleSolid, 0, 1);
		g_sci->_gfxPaint32->kernelAddLine(topPlane->_object, Common::Point(rect.left, rect.bottom), Common::Point(rect.right, rect.bottom), 255, point_colors[start], kLineStyleSolid, 0, 1);
		g_sci->_gfxPaint32->kernelAddLine(topPlane->_object, Common::Point(rect.left, rect.top), Common::Point(rect.left, rect.bottom), 255, point_colors[start], kLineStyleSolid, 0, 1);
#endif
	}
}

static void draw_polygon(EngineState *s, reg_t polygon, int width, int height) {
	SegManager *segMan = s->_segMan;
	reg_t points = readSelector(segMan, polygon, SELECTOR(points));

#ifdef ENABLE_SCI32
	if (segMan->isHeapObject(points))
		points = readSelector(segMan, points, SELECTOR(data));
#endif

	int size = readSelectorValue(segMan, polygon, SELECTOR(size));
	int type = readSelectorValue(segMan, polygon, SELECTOR(type));
	Common::Point first, prev;
	int i;

	SegmentRef pointList = segMan->dereference(points);
	if (!pointList.isValid() || pointList.skipByte) {
		warning("draw_polygon: Polygon data pointer is invalid, skipping polygon");
		return;
	}

	prev = first = readPoint(pointList, 0);

	for (i = 1; i < size; i++) {
		Common::Point point = readPoint(pointList, i);
		draw_line(s, prev, point, type, width, height);
		prev = point;
	}

	draw_line(s, prev, first, type % 3, width, height);
}

static void draw_input(EngineState *s, reg_t poly_list, Common::Point start, Common::Point end, int opt, int width, int height) {
	List *list;
	Node *node;

	draw_point(s, start, 1, width, height);
	draw_point(s, end, 0, width, height);

	if (!poly_list.getSegment())
		return;

	list = s->_segMan->lookupList(poly_list);

	if (!list) {
		warning("[avoidpath] Could not obtain polygon list");
		return;
	}

	node = s->_segMan->lookupNode(list->first);

	while (node) {
		draw_polygon(s, node->value, width, height);
		node = s->_segMan->lookupNode(node->succ);
	}
}

static void print_polygon(SegManager *segMan, reg_t polygon) {
	reg_t points = readSelector(segMan, polygon, SELECTOR(points));

#ifdef ENABLE_SCI32
	if (segMan->isHeapObject(points))
		points = readSelector(segMan, points, SELECTOR(data));
#endif

	int size = readSelectorValue(segMan, polygon, SELECTOR(size));
	int type = readSelectorValue(segMan, polygon, SELECTOR(type));
	int i;
	Common::Point point;

	debugN(-1, "%i:", type);

	SegmentRef pointList = segMan->dereference(points);
	if (!pointList.isValid() || pointList.skipByte) {
		warning("print_polygon: Polygon data pointer is invalid, skipping polygon");
		return;
	}

	for (i = 0; i < size; i++) {
		point = readPoint(pointList, i);
		debugN(-1, " (%i, %i)", point.x, point.y);
	}

	point = readPoint(pointList, 0);
	debug(" (%i, %i);", point.x, point.y);
}

static void print_input(EngineState *s, reg_t poly_list, Common::Point start, Common::Point end, int opt) {
	List *list;
	Node *node;

	debug("Start point: (%i, %i)", start.x, start.y);
	debug("End point: (%i, %i)", end.x, end.y);
	debug("Optimization level: %i", opt);

	if (!poly_list.getSegment())
		return;

	list = s->_segMan->lookupList(poly_list);

	if (!list) {
		warning("[avoidpath] Could not obtain polygon list");
		return;
	}

	debug("Polygons:");
	node = s->_segMan->lookupNode(list->first);

	while (node) {
		print_polygon(s->_segMan, node->value);
		node = s->_segMan->lookupNode(node->succ);
	}
}
*/

/**
 * Computes the area of a triangle
 * Parameters: (const Common::Point &) a, b, c: The points of the triangle
 * Returns   : (int) The area multiplied by two
 */
static int area(const Common::Point &a, const Common::Point &b, const Common::Point &c) {
	return (b.x - a.x) * (a.y - c.y) - (c.x - a.x) * (a.y - b.y);
}

/**
 * Determines whether or not a point is to the left of a directed line
 * Parameters: (const Common::Point &) a, b: The directed line (a, b)
 *             (const Common::Point &) c: The query point
 * Returns   : (int) true if c is to the left of (a, b), false otherwise
 */
static bool left(const Common::Point &a, const Common::Point &b, const Common::Point &c) {
	return area(a, b, c) > 0;
}

/**
 * Determines whether or not three points are collinear
 * Parameters: (const Common::Point &) a, b, c: The three points
 * Returns   : (int) true if a, b, and c are collinear, false otherwise
 */
static bool collinear(const Common::Point &a, const Common::Point &b, const Common::Point &c) {
	return area(a, b, c) == 0;
}

/**
 * Determines whether or not a point lies on a line segment
 * Parameters: (const Common::Point &) a, b: The line segment (a, b)
 *             (const Common::Point &) c: The query point
 * Returns   : (int) true if c lies on (a, b), false otherwise
 */
static bool between(const Common::Point &a, const Common::Point &b, const Common::Point &c) {
	if (!collinear(a, b, c))
		return false;

	// Assumes a != b.
	if (a.x != b.x)
		return ((a.x <= c.x) && (c.x <= b.x)) || ((a.x >= c.x) && (c.x >= b.x));
	else
		return ((a.y <= c.y) && (c.y <= b.y)) || ((a.y >= c.y) && (c.y >= b.y));
}

/**
 * Determines whether or not two line segments properly intersect
 * Parameters: (const Common::Point &) a, b: The line segment (a, b)
 *             (const Common::Point &) c, d: The line segment (c, d)
 * Returns   : (int) true if (a, b) properly intersects (c, d), false otherwise
 */
static bool intersect_proper(const Common::Point &a, const Common::Point &b, const Common::Point &c, const Common::Point &d) {
	int ab = (left(a, b, c) && left(b, a, d)) || (left(a, b, d) && left(b, a, c));
	int cd = (left(c, d, a) && left(d, c, b)) || (left(c, d, b) && left(d, c, a));

	return ab && cd;
}

/**
 * Polygon containment test
 * Parameters: (const Common::Point &) p: The point
 *             (Polygon *) polygon: The polygon
 * Returns   : (int) CONT_INSIDE if p is strictly contained in polygon,
 *                   CONT_ON_EDGE if p lies on an edge of polygon,
 *                   CONT_OUTSIDE otherwise
 * Number of ray crossing left and right
 */
static int contained(const Common::Point &p, Polygon *polygon) {
	int lcross = 0, rcross = 0;
	Vertex *vertex;

	// Iterate over edges
	CLIST_FOREACH(vertex, &polygon->vertices) {
		const Common::Point &v1 = vertex->v;
		const Common::Point &v2 = CLIST_NEXT(vertex)->v;

		// Flags for ray straddling left and right
		int rstrad, lstrad;

		// Check if p is a vertex
		if (p == v1)
			return CONT_ON_EDGE;

		// Check if edge straddles the ray
		rstrad = (v1.y < p.y) != (v2.y < p.y);
		lstrad = (v1.y > p.y) != (v2.y > p.y);

		if (lstrad || rstrad) {
			// Compute intersection point x / xq
			int x = v2.x * v1.y - v1.x * v2.y + (v1.x - v2.x) * p.y;
			int xq = v1.y - v2.y;

			// Multiply by -1 if xq is negative (for comparison that follows)
			if (xq < 0) {
				x = -x;
				xq = -xq;
			}

			// Avoid floats by multiplying instead of dividing
			if (rstrad && (x > xq * p.x))
				rcross++;
			else if (lstrad && (x < xq * p.x))
				lcross++;
		}
	}

	// If we counted an odd number of total crossings the point is on an edge
	if ((lcross + rcross) % 2 == 1)
		return CONT_ON_EDGE;

	// If there are an odd number of crossings to one side the point is contained in the polygon
	if (rcross % 2 == 1) {
		// Invert result for contained access polygons.
		if (polygon->type == POLY_CONTAINED_ACCESS)
			return CONT_OUTSIDE;
		return CONT_INSIDE;
	}

	// Point is outside polygon. Invert result for contained access polygons
	if (polygon->type == POLY_CONTAINED_ACCESS)
		return CONT_INSIDE;

	return CONT_OUTSIDE;
}

/**
 * Computes polygon area
 * Parameters: (Polygon *) polygon: The polygon
 * Returns   : (int) The area multiplied by two
 */
static int polygon_area(Polygon *polygon) {
	Vertex *first = polygon->vertices.first();
	Vertex *v;
	int size = 0;

	v = CLIST_NEXT(first);

	while (CLIST_NEXT(v) != first) {
		size += area(first->v, v->v, CLIST_NEXT(v)->v);
		v = CLIST_NEXT(v);
	}

	return size;
}

/**
 * Fixes the vertex order of a polygon if incorrect. Contained access
 * polygons should have their vertices ordered clockwise, all other types
 * anti-clockwise
 * Parameters: (Polygon *) polygon: The polygon
 */
static void fix_vertex_order(Polygon *polygon) {
	int area = polygon_area(polygon);

	// When the polygon area is positive the vertices are ordered
	// anti-clockwise. When the area is negative the vertices are ordered
	// clockwise
	if (((area > 0) && (polygon->type == POLY_CONTAINED_ACCESS))
	        || ((area < 0) && (polygon->type != POLY_CONTAINED_ACCESS))) {

		polygon->vertices.reverse();
	}
}

/**
 * Determines whether or not a line from a point to a vertex intersects the
 * interior of the polygon, locally at that vertex
 * Parameters: (Common::Point) p: The point
 *             (Vertex *) vertex: The vertex
 * Returns   : (int) 1 if the line (p, vertex->v) intersects the interior of
 *                   the polygon, locally at the vertex. 0 otherwise
 */
static int inside(const Common::Point &p, Vertex *vertex) {
	// Check that it's not a single-vertex polygon
	if (VERTEX_HAS_EDGES(vertex)) {
		const Common::Point &prev = CLIST_PREV(vertex)->v;
		const Common::Point &next = CLIST_NEXT(vertex)->v;
		const Common::Point &cur = vertex->v;

		if (left(prev, cur, next)) {
			// Convex vertex, line (p, cur) intersects the inside
			// if p is located left of both edges
			if (left(cur, next, p) && left(prev, cur, p))
				return 1;
		} else {
			// Non-convex vertex, line (p, cur) intersects the
			// inside if p is located left of either edge
			if (left(cur, next, p) || left(prev, cur, p))
				return 1;
		}
	}

	return 0;
}

/**
 * Returns a list of all vertices that are visible from a particular vertex.
 * @param s				the pathfinding state
 * @param vertex_cur	the vertex
 * @return list of vertices that are visible from vert
 */
static VertexList *visible_vertices(PathfindingState *s, Vertex *vertex_cur) {
	VertexList *visVerts = new VertexList();

	for (int i = 0; i < s->vertices; i++) {
		Vertex *vertex = s->vertex_index[i];

		// Make sure we don't intersect a polygon locally at the vertices
		if ((vertex == vertex_cur) || (inside(vertex->v, vertex_cur)) || (inside(vertex_cur->v, vertex)))
			continue;

		// Check for intersecting edges
		int j;
		for (j = 0; j < s->vertices; j++) {
			Vertex *edge = s->vertex_index[j];
			if (VERTEX_HAS_EDGES(edge)) {
				if (between(vertex_cur->v, vertex->v, edge->v)) {
					// If we hit a vertex, make sure we can pass through it without intersecting its polygon
					if ((inside(vertex_cur->v, edge)) || (inside(vertex->v, edge)))
						break;

					// This edge won't properly intersect, so we continue
					continue;
				}

				if (intersect_proper(vertex_cur->v, vertex->v, edge->v, CLIST_NEXT(edge)->v))
					break;
			}
		}

		if (j == s->vertices)
			visVerts->push_front(vertex);
	}

	return visVerts;
}

/**
 * Determines if a point lies on the screen border
 * Parameters: (const Common::Point &) p: The point
 * Returns   : (int) true if p lies on the screen border, false otherwise
 */
bool PathfindingState::pointOnScreenBorder(const Common::Point &p) {
	return (p.x == 0) || (p.x == _width - 1) || (p.y == 0) || (p.y == _height - 1);
}

/**
 * Determines if an edge lies on the screen border
 * Parameters: (const Common::Point &) p, q: The edge (p, q)
 * Returns   : (int) true if (p, q) lies on the screen border, false otherwise
 */
bool PathfindingState::edgeOnScreenBorder(const Common::Point &p, const Common::Point &q) {
	return ((p.x == 0 && q.x == 0) || (p.y == 0 && q.y == 0)
			|| ((p.x == _width - 1) && (q.x == _width - 1))
			|| ((p.y == _height - 1) && (q.y == _height - 1)));
}

/**
 * Searches for a nearby point that is not contained in a polygon
 * Parameters: (FloatPoint) f: The pointf to search nearby
 *             (Polygon *) polygon: The polygon
 * Returns   : (int) PF_OK on success, PF_FATAL otherwise
 *             (Common::Point) *ret: The non-contained point on success
 */
static int find_free_point(FloatPoint f, Polygon *polygon, Common::Point *ret) {
	Common::Point p;

	// Try nearest point first
	p = Common::Point((int)floor(f.x + 0.5), (int)floor(f.y + 0.5));

	if (contained(p, polygon) != CONT_INSIDE) {
		*ret = p;
		return PF_OK;
	}

	p = Common::Point((int)floor(f.x), (int)floor(f.y));

	// Try (x, y), (x + 1, y), (x , y + 1) and (x + 1, y + 1)
	if (contained(p, polygon) == CONT_INSIDE) {
		p.x++;
		if (contained(p, polygon) == CONT_INSIDE) {
			p.y++;
			if (contained(p, polygon) == CONT_INSIDE) {
				p.x--;
				if (contained(p, polygon) == CONT_INSIDE)
					return PF_FATAL;
			}
		}
	}

	*ret = p;
	return PF_OK;
}

/**
 * Computes the near point of a point contained in a polygon
 * Parameters: (const Common::Point &) p: The point
 *             (Polygon *) polygon: The polygon
 * Returns   : (int) PF_OK on success, PF_FATAL otherwise
 *             (Common::Point) *ret: The near point of p in polygon on success
 */
int PathfindingState::findNearPoint(const Common::Point &p, Polygon *polygon, Common::Point *ret) {
	Vertex *vertex;
	FloatPoint near_p;
	uint32_t dist = HUGE_DISTANCE;

	CLIST_FOREACH(vertex, &polygon->vertices) {
		const Common::Point &p1 = vertex->v;
		const Common::Point &p2 = CLIST_NEXT(vertex)->v;
		float u;
		FloatPoint new_point;
		uint32_t new_dist;

		// Ignore edges on the screen border, except for contained access polygons
		if ((polygon->type != POLY_CONTAINED_ACCESS) && (edgeOnScreenBorder(p1, p2)))
			continue;

		// Compute near point
		u = ((p.x - p1.x) * (p2.x - p1.x) + (p.y - p1.y) * (p2.y - p1.y)) / (float)p1.sqrDist(p2);

		// Clip to edge
		if (u < 0.0F)
			u = 0.0F;
		if (u > 1.0F)
			u = 1.0F;

		new_point.x = p1.x + u * (p2.x - p1.x);
		new_point.y = p1.y + u * (p2.y - p1.y);

		new_dist = p.sqrDist(new_point.toPoint());

		if (new_dist < dist) {
			near_p = new_point;
			dist = new_dist;
		}
	}

	// Find point not contained in polygon
	return find_free_point(near_p, polygon, ret);
}

/**
 * Computes the intersection point of a line segment and an edge (not
 * including the vertices themselves)
 * Parameters: (const Common::Point &) a, b: The line segment (a, b)
 *             (Vertex *) vertex: The first vertex of the edge
 * Returns   : (int) PF_OK on success, PF_ERROR otherwise
 *             (FloatPoint) *ret: The intersection point
 */
static int intersection(const Common::Point &a, const Common::Point &b, const Vertex *vertex, FloatPoint *ret) {
	// Parameters of parametric equations
	float s, t;
	// Numerator and denominator of equations
	float num, denom;
	const Common::Point &c = vertex->v;
	const Common::Point &d = CLIST_NEXT(vertex)->v;

	denom = a.x * (float)(d.y - c.y) + b.x * (float)(c.y - d.y) +
	        d.x * (float)(b.y - a.y) + c.x * (float)(a.y - b.y);

	if (denom == 0.0)
		// Segments are parallel, no intersection
		return PF_ERROR;

	num = a.x * (float)(d.y - c.y) + c.x * (float)(a.y - d.y) + d.x * (float)(c.y - a.y);

	s = num / denom;

	num = -(a.x * (float)(c.y - b.y) + b.x * (float)(a.y - c.y) + c.x * (float)(b.y - a.y));

	t = num / denom;

	if ((0.0 <= s) && (s <= 1.0) && (0.0 < t) && (t < 1.0)) {
		// Intersection found
		ret->x = a.x + s * (b.x - a.x);
		ret->y = a.y + s * (b.y - a.y);
		return PF_OK;
	}

	return PF_ERROR;
}

/**
 * Computes the nearest intersection point of a line segment and the polygon
 * set. Intersection points that are reached from the inside of a polygon
 * are ignored as are improper intersections which do not obstruct
 * visibility
 * Parameters: (PathfindingState *) s: The pathfinding state
 *             (const Common::Point &) p, q: The line segment (p, q)
 * Returns   : (int) PF_OK on success, PF_ERROR when no intersections were
 *                   found, PF_FATAL otherwise
 *             (Common::Point) *ret: On success, the closest intersection point
 */
static int nearest_intersection(PathfindingState *s, const Common::Point &p, const Common::Point &q, Common::Point *ret) {
	Polygon *polygon = 0;
	FloatPoint isec;
	Polygon *ipolygon = 0;
	uint32_t dist = HUGE_DISTANCE;

	for (PolygonList::iterator it = s->polygons.begin(); it != s->polygons.end(); ++it) {
		polygon = *it;
		Vertex *vertex;

		CLIST_FOREACH(vertex, &polygon->vertices) {
			uint32_t new_dist;
			FloatPoint new_isec;

			// Check for intersection with vertex
			if (between(p, q, vertex->v)) {
				// Skip this vertex if we hit it from the
				// inside of the polygon
				if (inside(q, vertex)) {
					new_isec.x = vertex->v.x;
					new_isec.y = vertex->v.y;
				} else
					continue;
			} else {
				// Check for intersection with edges

				// Skip this edge if we hit it from the
				// inside of the polygon
				if (!left(vertex->v, CLIST_NEXT(vertex)->v, q))
					continue;

				if (intersection(p, q, vertex, &new_isec) != PF_OK)
					continue;
			}

			new_dist = p.sqrDist(new_isec.toPoint());
			if (new_dist < dist) {
				ipolygon = polygon;
				isec = new_isec;
				dist = new_dist;
			}
		}
	}

	if (dist == HUGE_DISTANCE)
		return PF_ERROR;

	// Find point not contained in polygon
	return find_free_point(isec, ipolygon, ret);
}

/**
 * Checks whether a point is nearby a contained-access polygon (distance 1 pixel)
 * @param point			the point
 * @param polygon		the contained-access polygon
 * @return true when point is nearby polygon, false otherwise
 */
static bool nearbyPolygon(const Common::Point &point, Polygon *polygon) {
	assert(polygon->type == POLY_CONTAINED_ACCESS);

	return ((contained(Common::Point(point.x, point.y + 1), polygon) != CONT_INSIDE)
			|| (contained(Common::Point(point.x, point.y - 1), polygon) != CONT_INSIDE)
			|| (contained(Common::Point(point.x + 1, point.y), polygon) != CONT_INSIDE)
			|| (contained(Common::Point(point.x - 1, point.y), polygon) != CONT_INSIDE));
}

/**
 * Checks that the start point is in a valid position, and takes appropriate action if it's not.
 * @param s				the pathfinding state
 * @param start			the start point
 * @return a valid start point on success, NULL otherwise
 */
static Common::Point *fixup_start_point(PathfindingState *s, const Common::Point &start) {
	PolygonList::iterator it = s->polygons.begin();
	Common::Point *new_start = new Common::Point(start);

	while (it != s->polygons.end()) {
		int cont = contained(start, *it);
		int type = (*it)->type;

		switch (type) {
		case POLY_TOTAL_ACCESS:
			// Remove totally accessible polygons that contain the start point
			if (cont != CONT_OUTSIDE) {
				delete *it;
				it = s->polygons.erase(it);
				continue;
			}
			break;
		case POLY_CONTAINED_ACCESS:
			// Remove contained access polygons that do not contain
			// the start point (containment test is inverted here).
			// SSCI appears to be using a small margin of error here,
			// so we do the same.
			if ((cont == CONT_INSIDE) && !nearbyPolygon(start, *it)) {
				delete *it;
				it = s->polygons.erase(it);
				continue;
			}
			// Fall through
		case POLY_BARRED_ACCESS:
		case POLY_NEAREST_ACCESS:
			if (cont != CONT_OUTSIDE) {
				if (s->_prependPoint != NULL) {
					// We shouldn't get here twice.
					// We need to break in this case, otherwise we'll end in an infinite
					// loop.
					SDL_LogWarn(SDL_LOG_CATEGORY_SYSTEM, "AvoidPath: start point is contained in multiple polygons");
					break;
				}

				if (s->findNearPoint(start, (*it), new_start) != PF_OK) {
					delete new_start;
					return NULL;
				}

				if ((type == POLY_BARRED_ACCESS) || (type == POLY_CONTAINED_ACCESS))
					SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "AvoidPath: start position at unreachable location");

				// The original start position is in an invalid location, so we
				// use the moved point and add the original one to the final path
				// later on.
				if (start != *new_start)
					s->_prependPoint = new Common::Point(start);
			}
			break;
		default:
			break;
		}

		++it;
	}

	return new_start;
}

/**
 * Checks that the end point is in a valid position, and takes appropriate action if it's not.
 * @param s				the pathfinding state
 * @param end			the end point
 * @return a valid end point on success, NULL otherwise
 */
static Common::Point *fixup_end_point(PathfindingState *s, const Common::Point &end) {
	PolygonList::iterator it = s->polygons.begin();
	Common::Point *new_end = new Common::Point(end);

	while (it != s->polygons.end()) {
		int cont = contained(end, *it);
		int type = (*it)->type;

		switch (type) {
		case POLY_TOTAL_ACCESS:
			// Remove totally accessible polygons that contain the end point
			if (cont != CONT_OUTSIDE) {
				delete *it;
				it = s->polygons.erase(it);
				continue;
			}
			break;
		case POLY_CONTAINED_ACCESS:
		case POLY_BARRED_ACCESS:
		case POLY_NEAREST_ACCESS:
			if (cont != CONT_OUTSIDE) {
				if (s->_appendPoint != NULL) {
					// We shouldn't get here twice.
					// Happens in LB2CD, inside the speakeasy when walking from the
					// speakeasy (room 310) into the bathroom (room 320), after having
					// consulted the notebook (bug #3036299).
					// We need to break in this case, otherwise we'll end in an infinite
					// loop.
					SDL_LogWarn(SDL_LOG_CATEGORY_SYSTEM, "AvoidPath: end point is contained in multiple polygons");
					break;
				}

				// The original end position is in an invalid location, so we move the point
				if (s->findNearPoint(end, (*it), new_end) != PF_OK) {
					delete new_end;
					return NULL;
				}

				// For near-point access polygons we need to add the original end point
				// to the path after pathfinding.
				if ((type == POLY_NEAREST_ACCESS) && (end != *new_end))
					s->_appendPoint = new Common::Point(end);
			}
			break;
		default:
			break;
		}

		++it;
	}

	return new_end;
}

/**
 * Merges a point into the polygon set. A new vertex is allocated for this
 * point, unless a matching vertex already exists. If the point is on an
 * already existing edge that edge is split up into two edges connected by
 * the new vertex
 * Parameters: (PathfindingState *) s: The pathfinding state
 *             (const Common::Point &) v: The point to merge
 * Returns   : (Vertex *) The vertex corresponding to v
 */
static Vertex *merge_point(PathfindingState *s, const Common::Point &v) {
	Vertex *vertex;
	Vertex *v_new;
	Polygon *polygon;

	// Check for already existing vertex
	for (PolygonList::iterator it = s->polygons.begin(); it != s->polygons.end(); ++it) {
		polygon = *it;
		CLIST_FOREACH(vertex, &polygon->vertices) {
			if (vertex->v == v)
				return vertex;
		}
	}

	v_new = new Vertex(v);

	// Check for point being on an edge
	for (PolygonList::iterator it = s->polygons.begin(); it != s->polygons.end(); ++it) {
		polygon = *it;
		// Skip single-vertex polygons
		if (VERTEX_HAS_EDGES(polygon->vertices.first())) {
			CLIST_FOREACH(vertex, &polygon->vertices) {
				Vertex *next = CLIST_NEXT(vertex);

				if (between(vertex->v, next->v, v)) {
					// Split edge by adding vertex
					polygon->vertices.insertAfter(vertex, v_new);
					return v_new;
				}
			}
		}
	}

	// Add point as single-vertex polygon
	polygon = new Polygon(POLY_BARRED_ACCESS);
	polygon->vertices.insertHead(v_new);
	s->polygons.push_front(polygon);

	return v_new;
}

/**
 * Converts an SCI polygon into a Polygon
 * Parameters: (EngineState *) s: The game state
 *             (reg_t) polygon: The SCI polygon to convert
 * Returns   : (Polygon *) The converted polygon, or NULL on error
 */
static Polygon *convert_polygon(sol::table polygon) {
	int i;
	int size = polygon.size();
	Polygon *poly = new Polygon(polygon[1].get<bool>() ? POLY_BARRED_ACCESS : POLY_CONTAINED_ACCESS);
	for (i = 2; i < size; i += 2)
	{
		Common::Point point = { polygon[i].get<int16_t>(), polygon[i + 1].get<int16_t>() };
		Vertex *vertex = new Vertex(point);
		poly->vertices.insertHead(vertex);
	}
	fix_vertex_order(poly);
	return poly;
/*
	int i;
	reg_t points = readSelector(segMan, polygon, SELECTOR(points));
	int size = readSelectorValue(segMan, polygon, SELECTOR(size));

	int skip = 0;

	Polygon *poly = new Polygon(readSelectorValue(segMan, polygon, SELECTOR(type)));

	for (i = skip; i < size; i++) {
		Vertex *vertex = new Vertex(readPoint(pointList, i));
		poly->vertices.insertHead(vertex);
	}

	fix_vertex_order(poly);
	return poly;
	*/
}

/**
 * Changes the polygon list for optimization level 0 (used for keyboard
 * support). Totally accessible polygons are removed and near-point
 * accessible polygons are changed into totally accessible polygons.
 * Parameters: (PathfindingState *) s: The pathfinding state
 */
static void change_polygons_opt_0(PathfindingState *s) {

	PolygonList::iterator it = s->polygons.begin();
	while (it != s->polygons.end()) {
		Polygon *polygon = *it;
		assert(polygon);

		if (polygon->type == POLY_TOTAL_ACCESS) {
			delete polygon;
			it = s->polygons.erase(it);
		} else {
			if (polygon->type == POLY_NEAREST_ACCESS)
				polygon->type = POLY_TOTAL_ACCESS;
			++it;
		}
	}
}

/**
 * Converts the SCI input data for pathfinding
 * Parameters: (EngineState *) s: The game state
 *             (reg_t) poly_list: Polygon list
 *             (Common::Point) start: The start point
 *             (Common::Point) end: The end point
 *             (int) opt: Optimization level (0, 1 or 2)
 * Returns   : (PathfindingState *) On success a newly allocated pathfinding state,
 *                            NULL otherwise
 */
static PathfindingState *convert_polygon_set(sol::table poly_list, Common::Point start, Common::Point end, int width, int height, int opt) {
	Polygon *polygon;
	int count = 0;
	PathfindingState *pf_s = new PathfindingState(width, height);

	// Convert all polygons
	if (poly_list) {
		for (auto &poly : poly_list)
		{
			auto p = poly.second.as<sol::table>();
			polygon = convert_polygon(p);
			pf_s->polygons.push_back(polygon);
			count += p.size();
		}
		/*
		List *list = s->_segMan->lookupList(poly_list);
		Node *node = s->_segMan->lookupNode(list->first);

		while (node) {
			// The node value might be null, in which case there's no polygon to parse.
			// Happens in LB2 floppy - refer to bug #3041232
			polygon = !node->value.isNull() ? convert_polygon(s, node->value) : NULL;

			if (polygon) {
				pf_s->polygons.push_back(polygon);
				count += readSelectorValue(segMan, node->value, SELECTOR(size));
			}

			node = s->_segMan->lookupNode(node->succ);
		}
		*/
	}

	if (opt == 0)
		change_polygons_opt_0(pf_s);

	Common::Point *new_start = fixup_start_point(pf_s, start);

	if (!new_start) {
		SDL_LogWarn(SDL_LOG_CATEGORY_SYSTEM, "AvoidPath: Couldn't fixup start position for pathfinding");
		delete pf_s;
		return NULL;
	}

	Common::Point *new_end = fixup_end_point(pf_s, end);

	if (!new_end) {
		SDL_LogWarn(SDL_LOG_CATEGORY_SYSTEM, "AvoidPath: Couldn't fixup end position for pathfinding");
		delete new_start;
		delete pf_s;
		return NULL;
	}

	if (opt == 0) {
		// Keyboard support. Only the first edge of the path we compute
		// here matches the path returned by SSCI. This is assumed to be
		// sufficient as all known use cases only use the first two
		// vertices of the returned path.
		// Pharkas uses this mode for a secondary polygon set containing
		// rectangular polygons used to block an actor's path.

		// If we have a prepended point, we do nothing here as the
		// actor is in barred territory and should be moved outside of
		// it ASAP. This matches the behavior of SSCI.
		if (!pf_s->_prependPoint) {
			// Actor position is OK, find nearest obstacle.
			int err = nearest_intersection(pf_s, start, *new_end, new_start);

			if (err == PF_FATAL) {
				SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "AvoidPath: error finding nearest intersection");
				delete new_start;
				delete new_end;
				delete pf_s;
				return NULL;
			}

			if (err == PF_OK)
				pf_s->_prependPoint = new Common::Point(start);
		}
	}

	// Merge start and end points into polygon set
	pf_s->vertex_start = merge_point(pf_s, *new_start);
	pf_s->vertex_end = merge_point(pf_s, *new_end);

	delete new_start;
	delete new_end;

	// Allocate and build vertex index
	pf_s->vertex_index = (Vertex**)malloc(sizeof(Vertex *) * (count + 2));

	count = 0;

	for (PolygonList::iterator it = pf_s->polygons.begin(); it != pf_s->polygons.end(); ++it) {
		polygon = *it;
		Vertex *vertex;

		CLIST_FOREACH(vertex, &polygon->vertices) {
			pf_s->vertex_index[count++] = vertex;
		}
	}

	pf_s->vertices = count;

	return pf_s;
}

/**
 * Computes a shortest path from vertex_start to vertex_end. The caller can
 * construct the resulting path by following the path_prev links from
 * vertex_end back to vertex_start. If no path exists vertex_end->path_prev
 * will be NULL
 * Parameters: (PathfindingState *) s: The pathfinding state
 */
static void AStar(PathfindingState *s) {
	// Vertices of which the shortest path is known
	VertexList closedSet;

	// The remaining vertices
	VertexList openSet;

	openSet.push_front(s->vertex_start);
	s->vertex_start->costG = 0;
	s->vertex_start->costF = (uint32_t)sqrt((float)s->vertex_start->v.sqrDist(s->vertex_end->v));

	while (!openSet.empty()) {
		// Find vertex in open set with lowest F cost
		VertexList::iterator vertex_min_it = openSet.end();
		Vertex *vertex_min = 0;
		uint32_t min = HUGE_DISTANCE;

		for (VertexList::iterator it = openSet.begin(); it != openSet.end(); ++it) {
			Vertex *vertex = *it;
			if (vertex->costF < min) {
				vertex_min_it = it;
				vertex_min = *vertex_min_it;
				min = vertex->costF;
			}
		}

		assert(vertex_min != 0);	// the vertex cost should never be bigger than HUGE_DISTANCE

		// Check if we are done
		if (vertex_min == s->vertex_end)
			break;

		// Move vertex from set open to set closed
		closedSet.push_front(vertex_min);
		openSet.erase(vertex_min_it);

		VertexList *visVerts = visible_vertices(s, vertex_min);

		for (VertexList::iterator it = visVerts->begin(); it != visVerts->end(); ++it) {
			uint32_t new_dist;
			Vertex *vertex = *it;

			if (closedSet.contains(vertex))
				continue;

			if (!openSet.contains(vertex))
				openSet.push_front(vertex);

			new_dist = vertex_min->costG + (uint32_t)sqrt((float)vertex_min->v.sqrDist(vertex->v));

			// When travelling to a vertex on the screen edge, we
			// add a penalty score to make this path less appealing.
			// NOTE: If an obstacle has only one vertex on a screen edge,
			// later SSCI pathfinders will treat that vertex like any
			// other, while we apply a penalty to paths traversing it.
			// This difference might lead to problems, but none are
			// known at the time of writing.

			if (s->pointOnScreenBorder(vertex->v))
				new_dist += 10000;

			if (new_dist < vertex->costG) {
				vertex->costG = new_dist;
				vertex->costF = vertex->costG + (uint32_t)sqrt((float)vertex->v.sqrDist(s->vertex_end->v));
				vertex->path_prev = vertex_min;
			}
		}

		delete visVerts;
	}

	if (openSet.empty())
		SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "AvoidPath: End point (%i, %i) is unreachable", s->vertex_end->v.x, s->vertex_end->v.y);
}

/*
static reg_t allocateOutputArray(SegManager *segMan, int size) {
	reg_t addr;

#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		SciArray *array = segMan->allocateArray(kArrayTypeInt16, size * 2, &addr);
		assert(array);
		return addr;
	}
#endif

	segMan->allocDynmem(POLY_POINT_SIZE * size, AVOIDPATH_DYNMEM_STRING, &addr);
	return addr;
}
*/

/**
 * Stores the final path in newly allocated dynmem
 * Parameters: (PathfindingState *) p: The pathfinding state
 *             (EngineState *) s: The game state
 * Returns   : (reg_t) Pointer to dynmem containing path
 */
static uint16_t* output_path(PathfindingState *p) {
	int path_len = 0;
	uint16_t* output;
	Vertex *vertex = p->vertex_end;
	int unreachable = vertex->path_prev == NULL;

	if (!unreachable) {
		while (vertex) {
			// Compute path length
			path_len++;
			vertex = vertex->path_prev;
		}
	}

	// Allocate memory for path, plus 3 extra for appended point, prepended point and sentinel
	output = (uint16_t*)malloc(((path_len + 3) * 2) * sizeof(uint16_t));

	if (unreachable) {
		// If pathfinding failed we only return the path up to vertex_start

		if (p->_prependPoint)
		{
			output[0] = p->_prependPoint->x;
			output[1] = p->_prependPoint->y;
			//writePoint(arrayRef, 0, *p->_prependPoint);
		}
		else
		{
			output[0] = p->vertex_start->v.x;
			output[1] = p->vertex_start->v.y;
			//writePoint(arrayRef, 0, p->vertex_start->v);
		}

		output[2] = p->vertex_start->v.x;
		output[3] = p->vertex_start->v.y;
		//writePoint(arrayRef, 1, p->vertex_start->v);
		output[4] = POLY_LAST_POINT;
		output[5] = POLY_LAST_POINT;
		//writePoint(arrayRef, 2, Common::Point(POLY_LAST_POINT, POLY_LAST_POINT));

		return output;
	}

	int offset = 0;

	if (p->_prependPoint) {
		output[offset++] = p->_prependPoint->x;
		output[offset++] = p->_prependPoint->y;
		//writePoint(arrayRef, offset, *p->_prependPoint);
		//offset++;
	}

	vertex = p->vertex_end;
	for (int i = path_len - 1; i >= 0; i--) {
		output[offset + 0 + (i * 2)] = vertex->v.x;
		output[offset + 1 + (i * 2)] = vertex->v.y;
		//writePoint(arrayRef, offset + i, vertex->v);
		vertex = vertex->path_prev;
	}
	offset += path_len * 2;

	if (p->_appendPoint)
	{
		output[offset++] = p->_appendPoint->x;
		output[offset++] = p->_appendPoint->y;
		//writePoint(arrayRef, offset++, *p->_appendPoint);
	}

	// Sentinel
	output[offset++] = POLY_LAST_POINT;
	output[offset++] = POLY_LAST_POINT;
	//writePoint(arrayRef, offset, Common::Point(POLY_LAST_POINT, POLY_LAST_POINT));

	return output;
}

/*
reg_t kAvoidPath(EngineState *s, int argc, reg_t *argv) {
	Common::Point start = Common::Point(argv[0].toSint16(), argv[1].toSint16());

	switch (argc) {

	case 3 : {
		reg_t retval;
		Polygon *polygon = convert_polygon(s, argv[2]);

		if (!polygon)
			return NULL_REG;

		// Override polygon type to prevent inverted result for contained access polygons
		polygon->type = POLY_BARRED_ACCESS;

		retval = make_reg(0, contained(start, polygon) != CONT_OUTSIDE);
		delete polygon;
		return retval;
	}
	case 6 :
	case 7 :
	case 8 : {
		Common::Point end = Common::Point(argv[2].toSint16(), argv[3].toSint16());
		reg_t poly_list, output;
		int width, height, opt = 1;

		if (getSciVersion() >= SCI_VERSION_2) {
			if (argc < 7)
				error("[avoidpath] Not enough arguments");

			poly_list = (!argv[4].isNull() ? readSelector(s->_segMan, argv[4], SELECTOR(elements)) : NULL_REG);
			width = argv[5].toUint16();
			height = argv[6].toUint16();
			if (argc > 7)
				opt = argv[7].toUint16();
		} else {
			// SCI1.1 and older games always ran with an internal resolution of 320x200
			poly_list = argv[4];
			width = 320;
			height = 190;
			if (argc > 6)
				opt = argv[6].toUint16();
		}

		if (DebugMan.isDebugChannelEnabled(kDebugLevelAvoidPath)) {
			debug("[avoidpath] Pathfinding input:");
			draw_point(s, start, 1, width, height);
			draw_point(s, end, 0, width, height);

			if (poly_list.getSegment()) {
				print_input(s, poly_list, start, end, opt);
				draw_input(s, poly_list, start, end, opt, width, height);
			}

			// Update the whole screen
			if (getSciVersion() <= SCI_VERSION_1_1) {
				g_sci->_gfxScreen->copyToScreen();
				g_system->updateScreen();
#ifdef ENABLE_SCI32
			} else {
				g_sci->_gfxFrameout->kernelFrameOut(true);
#endif
			}
		}

		PathfindingState *p = convert_polygon_set(s, poly_list, start, end, width, height, opt);

		if (!p) {
			warning("[avoidpath] Error: pathfinding failed for following input:\n");
			print_input(s, poly_list, start, end, opt);
			warning("[avoidpath] Returning direct path from start point to end point\n");
			output = allocateOutputArray(s->_segMan, 3);
			SegmentRef arrayRef = s->_segMan->dereference(output);
			assert(arrayRef.isValid() && !arrayRef.skipByte);

			writePoint(arrayRef, 0, start);
			writePoint(arrayRef, 1, end);
			writePoint(arrayRef, 2, Common::Point(POLY_LAST_POINT, POLY_LAST_POINT));

			return output;
		}

		// Apply Dijkstra
		AStar(p);

		output = output_path(p, s);
		delete p;

		// Memory is freed by explicit calls to Memory
		return output;
	}

	default:
		warning("Unknown AvoidPath subfunction %d", argc);
		return NULL_REG;
	}
}
*/

static bool PointInRect(const Common::Point &point, int16_t rectX1, int16_t rectY1, int16_t rectX2, int16_t rectY2) {
	int16_t top = MIN<int16_t>(rectY1, rectY2);
	int16_t left = MIN<int16_t>(rectX1, rectX2);
	int16_t bottom = MAX<int16_t>(rectY1, rectY2) + 1;
	int16_t right = MAX<int16_t>(rectX1, rectX2) + 1;

	Common::Rect rect = Common::Rect(left, top, right, bottom);
	// Add a one pixel margin of error
	rect.grow(1);

	return rect.contains(point);
}

/*
reg_t kIntersections(EngineState *s, int argc, reg_t *argv) {
	// This function computes intersection points for the "freeway pathing" in MUMG CD.
	int32 qSourceX = argv[0].toSint16();
	int32 qSourceY = argv[1].toSint16();
	int32 qDestX = argv[2].toSint16();
	int32 qDestY = argv[3].toSint16();
	uint16 startIndex = argv[5].toUint16();
	uint16 endIndex = argv[6].toUint16();
	uint16 stepSize = argv[7].toUint16();
	bool backtrack = argv[9].toUint16();

	const int32 kVertical = 0x7fffffff;

	uint16 curIndex = startIndex;
	reg_t *inpBuf = s->_segMan->derefRegPtr(argv[4], endIndex + 2);

	if (!inpBuf) {
		warning("Intersections: input buffer invalid");
		return NULL_REG;
	}

	reg_t *outBuf = s->_segMan->derefRegPtr(argv[8], (endIndex - startIndex + 2) / stepSize * 3);

	if (!outBuf) {
		warning("Intersections: output buffer invalid");
		return NULL_REG;
	}

	// Slope and y-intercept of the query line in centipixels
	int32 qIntercept;
	int32 qSlope;

	if (qSourceX != qDestX) {
		// Compute slope of the line and round to nearest centipixel
		qSlope = (1000 * (qSourceY - qDestY)) / (qSourceX - qDestX);

		if (qSlope >= 0)
			qSlope += 5;
		else
			qSlope -= 5;

		qSlope /= 10;

		// Compute y-intercept in centipixels
		qIntercept = (100 * qDestY) - (qSlope * qDestX);

		if (backtrack) {
			// If backtrack is set we extend the line from dest to source
			// until we hit a screen edge and place the source point there

			// First we try to place the source point on the left or right
			// screen edge
			if (qSourceX >= qDestX)
				qSourceX = 319;
			else
				qSourceX = 0;

			// Compute the y-coordinate
			qSourceY = ((qSlope * qSourceX) + qIntercept) / 100;

			// If the y-coordinate is off-screen, the point we want is on the
			// top or bottom edge of the screen instead
			if (qSourceY < 0 || qSourceY > 189) {
				if (qSourceY < 0)
					qSourceY = 0;
				else if (qSourceY > 189)
					qSourceY = 189;

				// Compute the x-coordinate
				qSourceX = (((((qSourceY * 100) - qIntercept) * 10) / qSlope) + 5) / 10;
			}
		}
	} else {
		// The query line is vertical
		qIntercept = qSlope = kVertical;

		if (backtrack) {
			// If backtrack is set, extend to screen edge
			if (qSourceY >= qDestY)
				qSourceY = 189;
			else
				qSourceY = 0;
		}
	}

	int32 pSourceX = inpBuf[curIndex].toSint16();
	int32 pSourceY = inpBuf[curIndex + 1].toSint16();

	// If it's a polygon, we include the first point again at the end
	int16_t doneIndex;
	if (pSourceX & (1 << 13))
		doneIndex = startIndex;
	else
		doneIndex = endIndex;

	pSourceX &= 0x1ff;

	debugCN(kDebugLevelAvoidPath, "%s: (%i, %i)[%i]",
		(doneIndex == startIndex ? "Polygon" : "Polyline"), pSourceX, pSourceY, curIndex);

	curIndex += stepSize;
	uint16 outCount = 0;

	while (1) {
		int32 pDestX = inpBuf[curIndex].toSint16() & 0x1ff;
		int32 pDestY = inpBuf[curIndex + 1].toSint16();

		if (DebugMan.isDebugChannelEnabled(kDebugLevelAvoidPath)) {
			draw_line(s, Common::Point(pSourceX, pSourceY),
				Common::Point(pDestX, pDestY), 2, 320, 190);
			debugN(-1, " (%i, %i)[%i]", pDestX, pDestY, curIndex);
		}

		// Slope and y-intercept of the polygon edge in centipixels
		int32 pIntercept;
		int32 pSlope;

		if (pSourceX != pDestX) {
			// Compute slope and y-intercept (as above)
			pSlope = ((pDestY - pSourceY) * 1000) / (pDestX - pSourceX);

			if (pSlope >= 0)
				pSlope += 5;
			else
				pSlope -= 5;

			pSlope /= 10;

			pIntercept = (pDestY * 100) - (pSlope * pDestX);
		} else {
			// Polygon edge is vertical
			pSlope = pIntercept = kVertical;
		}

		bool foundIntersection = true;
		int32 intersectionX = 0;
		int32 intersectionY = 0;

		if (qSlope == pSlope) {
			// If the lines overlap, we test the source and destination points
			// against the poly segment
			if ((pIntercept == qIntercept) && (PointInRect(Common::Point(pSourceX, pSourceY), qSourceX, qSourceY, qDestX, qDestY))) {
				intersectionX = pSourceX * 100;
				intersectionY = pSourceY * 100;
			} else if ((pIntercept == qIntercept) && PointInRect(Common::Point(qDestX, qDestY), pSourceX, pSourceY, pDestX, pDestY)) {
				intersectionX = qDestX * 100;
				intersectionY = qDestY * 100;
			} else {
				// Lines are parallel or segments don't overlap, no intersection
				foundIntersection = false;
			}
		} else {
			// Lines are not parallel
			if (qSlope == kVertical) {
				// Query segment is vertical, polygon segment is not vertical
				intersectionX = qSourceX * 100;
				intersectionY = pSlope * qSourceX + pIntercept;
			} else if (pSlope == kVertical) {
				// Polygon segment is vertical, query segment is not vertical
				intersectionX = pDestX * 100;
				intersectionY = qSlope * pDestX + qIntercept;
			} else {
				// Neither line is vertical
				intersectionX = ((pIntercept - qIntercept) * 100) / (qSlope - pSlope);
				intersectionY = ((intersectionX * pSlope) + (pIntercept * 100)) / 100;
			}
		}

		if (foundIntersection) {
			// Round back to pixels
			intersectionX = (intersectionX + 50) / 100;
			intersectionY = (intersectionY + 50) / 100;

			// If intersection point lies on both the query line segment and the poly
			// line segment, add it to the output
			if (PointInRect(Common::Point(intersectionX, intersectionY), pSourceX, pSourceY, pDestX, pDestY)
				&& PointInRect(Common::Point(intersectionX, intersectionY), qSourceX, qSourceY, qDestX, qDestY)) {
				outBuf[outCount * 3] = make_reg(0, intersectionX);
				outBuf[outCount * 3 + 1] = make_reg(0, intersectionY);
				outBuf[outCount * 3 + 2] = make_reg(0, curIndex);
				outCount++;
			}
		}

		if (curIndex == doneIndex) {
			// End of polyline/polygon reached
			if (DebugMan.isDebugChannelEnabled(kDebugLevelAvoidPath)) {
				debug(";");
				debugN(-1, "Found %i intersections", outCount);

				if (outCount) {
					debugN(-1, ":");
					for (int i = 0; i < outCount; i++) {
						Common::Point p = Common::Point(outBuf[i * 3].toSint16(), outBuf[i * 3 + 1].toSint16());
						draw_point(s, p, 0, 320, 190);
						debugN(-1, " (%i, %i)[%i]", p.x, p.y, outBuf[i * 3 + 2].toSint16());
					}
				}

				debug(";");

				g_sci->_gfxScreen->copyToScreen();
				g_system->updateScreen();
			}

			return make_reg(0, outCount);
		}

		if (curIndex != endIndex) {
			// Go to next point in polyline/polygon
			curIndex += stepSize;
		} else {
			// Wrap-around for polygon case
			curIndex = startIndex;
		}

		// Current destination point is source for the next line segment
		pSourceX = pDestX;
		pSourceY = pDestY;
	}
}
*/

// ==========================================================================
// kMergePoly utility functions

// Compute square of the distance of p to the segment a-b.
static float pointSegDistance(const Common::Point &a, const Common::Point &b,
                              const Common::Point &p) {
	FloatPoint ba(b-a);
	FloatPoint pa(p-a);
	FloatPoint bp(b-p);

	// Check if the projection of p on the line a-b lies between a and b
	if (ba * pa >= 0.0F && ba * bp >= 0.0F) {
		// If yes, return the (squared) distance of p to the line a-b:
		// translate a to origin, project p and subtract
		float linedist = (ba*((ba*pa)/(ba*ba)) - pa).norm();

		return linedist;
	} else {
		// If no, return the (squared) distance to either a or b, whichever
		// is closest.

		// distance to a:
		float adist = pa.norm();
		// distance to b:
		float bdist = FloatPoint(p-b).norm();

		return MIN(adist, bdist);
	}
}

// find intersection between edges of two polygons.
// endpoints count, except v2->_next
static bool segSegIntersect(const Vertex *v1, const Vertex *v2, Common::Point &intp) {
	const Common::Point &a = v1->v;
	const Common::Point &b = v1->_next->v;
	const Common::Point &c = v2->v;
	const Common::Point &d = v2->_next->v;

	// First handle the endpoint cases manually

	if (collinear(a, b, c) && collinear(a, b, d))
		return false;

	if (collinear(a, b, c)) {
		// a, b, c collinear
		// return true/c if c is between a and b
		intp = c;
		if (a.x != b.x) {
			if ((a.x <= c.x && c.x <= b.x) || (b.x <= c.x && c.x <= a.x))
				return true;
		} else {
			if ((a.y <= c.y && c.y <= b.y) || (b.y <= c.y && c.y <= a.y))
				return true;
		}
	}

	if (collinear(a, b, d)) {
		intp = d;
		// a, b, d collinear
		// return false/d if d is between a and b
		if (a.x != b.x) {
			if ((a.x <= d.x && d.x <= b.x) || (b.x <= d.x && d.x <= a.x))
				return false;
		} else {
			if ((a.y <= d.y && d.y <= b.y) || (b.y <= d.y && d.y <= a.y))
				return false;
		}
	}

	int len_dc = c.sqrDist(d);

	if (!len_dc) SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "zero length edge in polygon");

	if (pointSegDistance(c, d, a) <= 2.0F) {
		intp = a;
		return true;
	}

	if (pointSegDistance(c, d, b) <= 2.0F) {
		intp = b;
		return true;
	}

	// If not an endpoint, call the generic intersection function

	FloatPoint p;
	if (intersection(a, b, v2, &p) == PF_OK) {
		intp = p.toPoint();
		return true;
	} else {
		return false;
	}
}

// For intersecting polygon segments, determine if
// * the v2 edge enters polygon 1 at this intersection: positive return value
// * the v2 edge and the v1 edges are parallel: zero return value
// * the v2 edge exits polygon 1 at this intersection: negative return value
static int intersectDir(const Vertex *v1, const Vertex *v2) {
	Common::Point p1 = v1->_next->v - v1->v;
	Common::Point p2 = v2->_next->v - v2->v;
	return (p1.x * p2.y - p2.x * p1.y);
}

// Direction of edge in degrees from pos. x-axis, between -180 and 180
static int edgeDir(const Vertex *v) {
	Common::Point p = v->_next->v - v->v;
	int deg = Common::rad2deg<float,int>((float)atan2((double)p.y, (double)p.x));
	if (deg < -180) deg += 360;
	if (deg > 180) deg -= 360;
	return deg;
}

// For points p1, p2 on the polygon segment v, determine if
// * p1 lies before p2: negative return value
// * p1 and p2 are the same: zero return value
// * p1 lies after p2: positive return value
static int liesBefore(const Vertex *v, const Common::Point &p1, const Common::Point &p2) {
	return v->v.sqrDist(p1) - v->v.sqrDist(p2);
}

// Structure describing an "extension" to the work polygon following edges
// of the polygon being merged.

// The patch begins on the point intersection1, being the intersection
// of the edges starting at indexw1/vertexw1 on the work polygon, and at
// indexp1/vertexp1 on the polygon being merged.
// It ends with the point intersection2, being the analogous intersection.
struct Patch {
	uint32_t indexw1;
	uint32_t indexp1;
	const Vertex *vertexw1;
	const Vertex *vertexp1;
	Common::Point intersection1;

	uint32_t indexw2;
	uint32_t indexp2;
	const Vertex *vertexw2;
	const Vertex *vertexp2;
	Common::Point intersection2;

	bool disabled; // If true, this Patch was made superfluous by another Patch
};


// Check if the given vertex on the work polygon is bypassed by this patch.
static bool isVertexCovered(const Patch &p, uint32_t wi) {

	//         /             v       (outside)
	//  ---w1--1----p----w2--2----
	//         ^             \       (inside)
	if (wi > p.indexw1 && wi <= p.indexw2)
		return true;

	//         v             /       (outside)
	//  ---w2--2----p----w1--1----
	//         \             ^       (inside)
	if (p.indexw1 > p.indexw2 && (wi <= p.indexw2 || wi > p.indexw1))
		return true;

	//         v  /                  (outside)
	//  ---w1--2--1-------p-----
	//     w2  \  ^                  (inside)
	if (p.indexw1 == p.indexw2 && liesBefore(p.vertexw1, p.intersection1, p.intersection2) > 0)
		return true; // This patch actually covers _all_ vertices on work

	return false;
}

// Check if patch p1 makes patch p2 superfluous.
static bool isPatchCovered(const Patch &p1, const Patch &p2) {

	// Same exit and entry points
	if (p1.intersection1 == p2.intersection1 && p1.intersection2 == p2.intersection2)
		return true;

	//           /         *         v       (outside)
	//  ---p1w1--1----p2w1-1---p1w2--2----
	//           ^         *         \       (inside)
	if (p1.indexw1 < p2.indexw1 && p2.indexw1 < p1.indexw2)
		return true;
	if (p1.indexw1 > p1.indexw2 && (p2.indexw1 > p1.indexw1 || p2.indexw1 < p1.indexw2))
		return true;


	//            /         *          v       (outside)
	//  ---p1w1--11----p2w2-2---p1w2--12----
	//            ^         *          \       (inside)
	if (p1.indexw1 < p2.indexw2 && p2.indexw2 < p1.indexw2)
		return true;
	if (p1.indexw1 > p1.indexw2 && (p2.indexw2 > p1.indexw1 || p2.indexw2 < p1.indexw2))
		return true;

	// Opposite of two above situations
	if (p2.indexw1 < p1.indexw1 && p1.indexw1 < p2.indexw2)
		return false;
	if (p2.indexw1 > p2.indexw2 && (p1.indexw1 > p2.indexw1 || p1.indexw1 < p2.indexw2))
		return false;

	if (p2.indexw1 < p1.indexw2 && p1.indexw2 < p2.indexw2)
		return false;
	if (p2.indexw1 > p2.indexw2 && (p1.indexw2 > p2.indexw1 || p1.indexw2 < p2.indexw2))
		return false;


	// The above checks covered the cases where one patch covers the other and
	// the intersections of the patches are on different edges.

	// So, if we passed the above checks, we have to check the order of
	// intersections on edges.


	if (p1.indexw1 != p1.indexw2) {

		//            /    *              v       (outside)
		//  ---p1w1--11---21--------p1w2--2----
		//     p2w1   ^    *              \       (inside)
		if (p1.indexw1 == p2.indexw1)
			return (liesBefore(p1.vertexw1, p1.intersection1, p2.intersection1) < 0);

		//            /                *    v       (outside)
		//  ---p1w1--11---------p1w2--21---12----
		//            ^         p2w1   *    \       (inside)
		if (p1.indexw2 == p2.indexw1)
			return (liesBefore(p1.vertexw2, p1.intersection2, p2.intersection1) > 0);

		// If neither of the above, then the intervals of the polygon
		// covered by patch1 and patch2 are disjoint
		return false;
	}

	// p1w1 == p1w2
	// Also, p1w1/p1w2 isn't strictly between p2


	//            v   /             *      (outside)
	//  ---p1w1--12--11-------p2w1-21----
	//     p1w2   \   ^             *      (inside)

	//            v   /   /               (outside)
	//  ---p1w1--12--21--11---------
	//     p1w2   \   ^   ^               (inside)
	//     p2w1
	if (liesBefore(p1.vertexw1, p1.intersection1, p1.intersection2) > 0)
		return (p1.indexw1 != p2.indexw1);

	// CHECKME: This is meaningless if p2w1 != p2w2 ??
	if (liesBefore(p2.vertexw1, p2.intersection1, p2.intersection2) > 0)
		return false;

	// CHECKME: This is meaningless if p1w1 != p2w1 ??
	if (liesBefore(p2.vertexw1, p2.intersection1, p1.intersection1) <= 0)
		return false;

	// CHECKME: This is meaningless if p1w2 != p2w1 ??
	if (liesBefore(p2.vertexw1, p2.intersection1, p1.intersection2) >= 0)
		return false;

	return true;
}

// Merge a single polygon into the work polygon.
// If there is an intersection between work and polygon, this function
// returns true, and replaces the vertex list of work by an extended version,
// that covers polygon.
//
// NOTE: The strategy used matches qfg1new closely, and is a bit error-prone.
// A more robust strategy would be inserting all intersection points directly
// into both vertex lists as a first pass. This would make finding the merged
// polygon a much more straightforward edge-walk, and avoid cases where SSCI's
// algorithm mixes up the order of multiple intersections on a single edge.
bool mergeSinglePolygon(Polygon &work, const Polygon &polygon) {
#ifdef DEBUG_MERGEPOLY
	const Vertex *vertex;
	debugN("work:");
	CLIST_FOREACH(vertex, &(work.vertices)) {
		debugN(" (%d,%d) ", vertex->v.x, vertex->v.y);
	}
	debugN("\n");
	debugN("poly:");
	CLIST_FOREACH(vertex, &(polygon.vertices)) {
		debugN(" (%d,%d) ", vertex->v.x, vertex->v.y);
	}
	debugN("\n");
#endif
	unsigned int workSize = work.vertices.size();
	unsigned int polygonSize = polygon.vertices.size();

	int patchCount = 0;
	Patch patchList[8];

	const Vertex *workv = work.vertices._head;
	const Vertex *polyv = polygon.vertices._head;
	for (unsigned int wi = 0; wi < workSize; ++wi, workv = workv->_next) {
		for (unsigned int pi = 0; pi < polygonSize; ++pi, polyv = polyv->_next) {
			Common::Point intersection1;
			Common::Point intersection2;

			bool intersects = segSegIntersect(workv, polyv, intersection1);
			if (!intersects)
				continue;

#ifdef DEBUG_MERGEPOLY
			debug("mergePoly: intersection at work %d, poly %d", wi, pi);
#endif

			if (intersectDir(workv, polyv) >= 0)
				continue;

#ifdef DEBUG_MERGEPOLY
			debug("mergePoly: intersection in right direction");
#endif

			int angle = 0;
			int baseAngle = edgeDir(workv);

			// We now found the point where an edge of 'polygon' left 'work'.
			// Now find the re-entry point.

			// NOTE: The order in which this searches does not always work
			// properly if the correct patch would only use a single partial
			// edge of poly. Because it starts at polyv->_next, it will skip
			// the correct re-entry and proceed to the next.

			const Vertex *workv2 = workv;
			const Vertex *polyv2 = polyv->_next;

			intersects = false;

			unsigned int pi2, wi2 = 0;
			for (pi2 = 0; pi2 < polygonSize; ++pi2, polyv2 = polyv2->_next) {

				int newAngle = edgeDir(polyv2);

				int relAngle = newAngle - baseAngle;
				if (relAngle > 180) relAngle -= 360;
				if (relAngle < -180) relAngle += 360;

				angle += relAngle;
				baseAngle = newAngle;

				workv2 = workv;
				for (wi2 = 0; wi2 < workSize; ++wi2, workv2 = workv2->_next) {
					intersects = segSegIntersect(workv2, polyv2, intersection2);
					if (!intersects)
						continue;
#ifdef DEBUG_MERGEPOLY
					debug("mergePoly: re-entry intersection at work %d, poly %d", (wi + wi2) % workSize, (pi + 1 + pi2) % polygonSize);
#endif

					if (intersectDir(workv2, polyv2) > 0) {
#ifdef DEBUG_MERGEPOLY
						debug("mergePoly: re-entry intersection in right direction, angle = %d", angle);
#endif
						break; // found re-entry point
					}

				}

				if (intersects)
					break;

			}

			if (!intersects || angle < 0)
				continue;


			if (patchCount >= 8)
				SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "kMergePoly: Too many patches");

			// convert relative to absolute vertex indices
			pi2 = (pi + 1 + pi2) % polygonSize;
			wi2 = (wi + wi2) % workSize;

			Patch &newPatch = patchList[patchCount];
			newPatch.indexw1 = wi;
			newPatch.vertexw1 = workv;
			newPatch.indexp1 = pi;
			newPatch.vertexp1 = polyv;
			newPatch.intersection1 = intersection1;

			newPatch.indexw2 = wi2;
			newPatch.vertexw2 = workv2;
			newPatch.indexp2 = pi2;
			newPatch.vertexp2 = polyv2;
			newPatch.intersection2 = intersection2;
			newPatch.disabled = false;

#ifdef DEBUG_MERGEPOLY
			debug("mergePoly: adding patch at work %d, poly %d", wi, pi);
#endif

			if (patchCount == 0) {
				patchCount++;
				continue;
			}

			bool necessary = true;
			for (int i = 0; i < patchCount; ++i) {
				if (isPatchCovered(patchList[i], newPatch)) {
					necessary = false;
					break;
				}
			}

			if (!necessary)
				continue;

			patchCount++;

			if (patchCount > 1) {
				// check if this patch makes other patches superfluous
				for (int i = 0; i < patchCount-1; ++i)
					if (isPatchCovered(newPatch, patchList[i]))
						patchList[i].disabled = true;
			}
		}
	}


	if (patchCount == 0)
		return false; // nothing changed


	// Determine merged work by doing a walk over the edges
	// of work, crossing over to polygon when encountering a patch.

	Polygon output(0);

	workv = work.vertices._head;
	for (unsigned int wi = 0; wi < workSize; ++wi, workv = workv->_next) {

		bool covered = false;
		for (int p = 0; p < patchCount; ++p) {
			if (patchList[p].disabled) continue;
			if (isVertexCovered(patchList[p], wi)) {
				covered = true;
				break;
			}
		}

		if (!covered) {
			// Add vertex to output
			output.vertices.insertAtEnd(new Vertex(workv->v));
		}


		// CHECKME: Why is this the correct order in which to process
		// the patches? (What if two of them start on this line segment
		// in the opposite order?)

		for (int p = 0; p < patchCount; ++p) {

			const Patch &patch = patchList[p];
			if (patch.disabled) continue;
			if (patch.indexw1 != wi) continue;
			if (patch.intersection1 != workv->v) {
				// Add intersection point to output
				output.vertices.insertAtEnd(new Vertex(patch.intersection1));
			}

			// Add vertices from polygon between vertexp1 (excl) and vertexp2 (incl)
			for (polyv = patch.vertexp1->_next; polyv != patch.vertexp2; polyv = polyv->_next)
				output.vertices.insertAtEnd(new Vertex(polyv->v));

			output.vertices.insertAtEnd(new Vertex(patch.vertexp2->v));

			if (patch.intersection2 != patch.vertexp2->v) {
				// Add intersection point to output
				output.vertices.insertAtEnd(new Vertex(patch.intersection2));
			}

			// TODO: We could continue after the re-entry point here?
		}
	}
	// Remove last vertex if it's the same as the first vertex
	if (output.vertices._head->v == output.vertices._head->_prev->v)
		output.vertices.remove(output.vertices._head->_prev);


	// Slight hack: swap vertex lists of output and work polygons.
	SWAP(output.vertices._head, work.vertices._head);

	return true;
}


/**
 * This is a quite rare kernel function. An example of when it's called
 * is in QFG1VGA, after killing any monster.
 *
 * It takes a polygon, and extends it to also cover any polygons from the
 * input list with which it intersects. Any of those polygons so covered
 * from the input list are marked by adding 0x10 to their type field.
 */
/*
reg_t kMergePoly(EngineState *s, int argc, reg_t *argv) {
	// 3 parameters: raw polygon data, polygon list, list size
	reg_t polygonData = argv[0];
	List *list = s->_segMan->lookupList(argv[1]);

	// The size of the "work" point list SSCI uses. We use a dynamic one instead
	//reg_t listSize = argv[2];

	SegmentRef pointList = s->_segMan->dereference(polygonData);
	if (!pointList.isValid() || pointList.skipByte) {
		warning("kMergePoly: Polygon data pointer is invalid");
		return make_reg(0, 0);
	}

	Node *node;

#ifdef DEBUG_MERGEPOLY
	node = s->_segMan->lookupNode(list->first);
	while (node) {
		draw_polygon(s, node->value, 320, 190);
		node = s->_segMan->lookupNode(node->succ);
	}
	Common::Point prev, first;
	prev = first = readPoint(pointList, 0);
	for (int i = 1; readPoint(pointList, i).x != 0x7777; i++) {
		Common::Point point = readPoint(pointList, i);
		draw_line(s, prev, point, 1, 320, 190);
		prev = point;
	}
	draw_line(s, prev, first, 1, 320, 190);
	// Update the whole screen
	g_sci->_gfxScreen->copyToScreen();
	g_system->updateScreen();
	g_system->delayMillis(1000);
#endif

	// The work polygon which we're going to merge with the polygons in list
	Polygon work(0);

	for (int i = 0; true; ++i) {
		Common::Point p = readPoint(pointList, i);
		if (p.x == POLY_LAST_POINT)
			break;

		Vertex *vertex = new Vertex(p);
		work.vertices.insertAtEnd(vertex);
	}

	// TODO: Check behaviour for single-vertex polygons
	node = s->_segMan->lookupNode(list->first);
	while (node) {
		Polygon *polygon = convert_polygon(s, node->value);

		if (polygon) {
			// CHECKME: Confirm vertex order that convert_polygon and
			// fix_vertex_order output. For now, we re-reverse the order since
			// convert_polygon reads the vertices reversed, and fix up head.
			polygon->vertices.reverse();
			polygon->vertices._head = polygon->vertices._head->_next;

			// Merge this polygon into the work polygon if there is an
			// intersection.
			bool intersected = mergeSinglePolygon(work, *polygon);

			// If so, flag it
			if (intersected) {
				writeSelectorValue(s->_segMan, node->value,
				                   SELECTOR(type), polygon->type + 0x10);
#ifdef DEBUG_MERGEPOLY
				debugN("Merged polygon: ");
				// Iterate over edges
				Vertex *vertex;
				CLIST_FOREACH(vertex, &(work.vertices)) {
					debugN(" (%d,%d) ", vertex->v.x, vertex->v.y);
				}
				debugN("\n");
#endif
			}

			delete polygon;
		}

		node = s->_segMan->lookupNode(node->succ);
	}


	// Allocate output array
	reg_t output = allocateOutputArray(s->_segMan, work.vertices.size()+1);
	SegmentRef arrayRef = s->_segMan->dereference(output);

	// Copy work.vertices into arrayRef
	Vertex *vertex;
	uint32_t n = 0;
	CLIST_FOREACH(vertex, &work.vertices) {
		if (vertex == work.vertices._head || vertex->v != vertex->_prev->v) {
			writePoint(arrayRef, n, vertex->v);
			n++;
		}
	}

	writePoint(arrayRef, n, Common::Point(POLY_LAST_POINT, POLY_LAST_POINT));

#ifdef DEBUG_MERGEPOLY
	prev = first = readPoint(arrayRef, 0);
	for (int i = 1; readPoint(arrayRef, i).x != 0x7777; i++) {
		Common::Point point = readPoint(arrayRef, i);
		draw_line(s, prev, point, 3, 320, 190);
		prev = point;
	}

	draw_line(s, prev, first, 3, 320, 190);

	// Update the whole screen
	g_sci->_gfxScreen->copyToScreen();
	g_system->updateScreen();
	if (!g_sci->_gfxPaint16)
		g_system->delayMillis(1000);

	debug("kMergePoly done");
#endif

	return output;
}
*/

} // End of namespace Sci


sol::table DoGetPath(int16_t aX, int16_t aY, int16_t bX, int16_t bY, int16_t opt)
{
	uint16_t *output;
	auto solPolys = Sol["polygons"].get<sol::table>();
	auto start = Common::Point(aX, aY);
	auto end = Common::Point(bX, bY);
	auto p = Sci::convert_polygon_set(solPolys, start, end, screenWidth, screenHeight, opt);
	AStar(p);
	output = output_path(p);
	delete p;

	uint16_t* point = output;
	auto newPath = Sol.create_table();
	printf("\n\nGetPath returns: \n");
	while (true)
	{
		auto x = *point++;
		auto y = *point++;
		printf("%d,%d ", x, y);
		newPath.add(x, y);
		if (x == 0x7777)
			break;
	}
	printf("\n");

	free(output);
	return newPath;
}
