#ifndef __FITTING_DEFINES_H__
#define __FITTING_DEFINES_H__

#define M_PI 3.14159265358979323846
const double EPSILON = 1e-5;
const double TOLERANCE = 1e-6;

// Structure to store the parameters of the Young-Laplace equation
struct YoungLaplaceParams {
	double radius;    // Radius of curvature
	double height;    // Height of the droplet
	double offsetX;   // Horizontal offset
	double offsetY;   // Vertical offset
};

typedef struct EllipseParams {
	double h;	// center x
	double k;	// center y
	double a;	// semi-major axis
	double b;	// semi-minor axis
	EllipseParams() : h(0), k(0), a(0), b(0) {}
	EllipseParams(double h, double k, double a, double b) : h(h), k(k), a(a), b(b) {}
	EllipseParams(float h, float k, float a, float b) : h(h), k(k), a(a), b(b) {}
}ellipseParams;

typedef struct CircleParams {
	double h;
	double k;
	double r;
}circleParams;

#endif// __FITTING_DEFINES_H__
