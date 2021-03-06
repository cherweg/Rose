#ifndef POINT_H_
#define POINT_H_

#include "basics/serializer.h"
#include "basics/types.h"

enum CoordDimType { X=0, Y=1, Z=2, W=3 }; // index values used for various dimensions in an array. W is intended for quaternions

class Point  : public Serializable {
	 friend ostream& operator<<(ostream&, const Point&);
	public:
		Point();
		Point(const Point& p);
		virtual ~Point();
		Point(const HomogeneousVector& p);
		Point(int args, const valarray<float>& vec);
		Point(millimeter xP,millimeter yP, millimeter zP);
		Point(millimeter xP,millimeter yP) : Point(xP, yP, 0) {};

		void translate(const Point& pPoint);
		Point  getTranslated(const Point& pPoint);

		void mirrorAt(const Point& pPoint, double scale);
		void mirrorAt(const Point& pPoint);
		void set(millimeter pX, millimeter pY,millimeter pZ);

		void null();
		bool isNull() const;

		Point getRotatedAroundZ(angle_rad alpha) const;
		Point getRotatedAroundY(angle_rad alpha) const;
		Point getRotatedAroundX(angle_rad alpha) const;

		Point& operator= (const Point& p);
		void operator= (const HomogeneousVector& p);
		void operator+= (const Point& p);
		void operator-= (const Point& p);
		void operator*= (const double f);

		void operator/= (const double f);
		Point operator- (const Point& p) const;
		Point operator+ (const Point& p) const;
		Point operator/ (const double f) const;
		Point operator* (const double f) const;

		bool operator==(const Point& pos);
		bool operator!=(const Point& pos);
		bool operator==(const Point& pos) const ;
		bool operator!=(const Point& pos) const;

		double& operator[] (int idx);
		double operator[] (int idx)  const;

		double distance(const Point& p) const;
		double distanceSqr(const Point& p) const;

		void moveTo(const Point& b, double dT, double maxSpeed);
		double length() const;
		double lengthSqr() const;

		double angleToDegree(const Point& pPoint) const;
		double scalarProduct(const Point& pPoint) const;
		Point orthogonalProjection(const Point& pLine) const;
		Point orthogonalProjection(const Point& pLineA, const Point &pLineB) const;
		Point getPointOfLine(double ratio, const Point& target);

		// returns as homogenous vector, i.e. a 4-dimensional vector with 1.0 as last dimension
		HomogeneousVector getHomVector() const;

		// return as vector , i.e. a 3-dimensional vector
		Vector getVector() const ;
		void limit(const Point&min, const Point &max);

		virtual std::ostream& serialize(std::ostream &out) const;
		virtual std::istream& deserialize(std::istream &in, bool& ok);

		millimeter x;
		millimeter y;
		millimeter z;
};

#endif
