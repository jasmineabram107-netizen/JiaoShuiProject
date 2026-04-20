#ifndef __HARRIS_COMMON_H__
#define __HARRIS_COMMON_H__

// Gaussian convolution methods
typedef enum GaussianType {
	STD_GAUSSIAN = 0,	// Standard Gaussian convolution
	FAST_GAUSSIAN = 1,	// Fast Gaussian convolution(default)
	NO_GAUSSIAN = 2		// No Gaussian convolution
}gaussianType;

// Strategy for computing the gradient
typedef enum GradientType {
	CENTRAL_DIFFERENCES = 0, // Central differences method(default)
	SOBEL_OPERATOR = 1       // Sobel operator
}gradientType;

// Types of corner strength functions
typedef enum CornerStyle {
	HARRIS_MEASURE,				// Harris corner measure
	SHI_TOMASI_MEASURE,			// Shi-Tomasi corner measure
	HARMONIC_MEAN_MEASURE		// Harmonic mean corner measure
}cornerStyle;

// Strategy for selecting the output corners
typedef enum CornerSelectStrategies {
	ALL_CORNERS,			// All corners(default)
	ALL_CORNERS_SORTED,		// Sort all corners
	N_CORNERS,				// N corners(param: Number of corners)
	DISTRIBUTED_N_CORNERS	// Distributed N corners(param: Number of corners, Cells)
}cornerSelectStrategies;

// Strategy for subpixel accuracy
typedef enum InterpolationType {
	NO_INTERPOLATION = 0,			// No subpixel precision
	QUADRATIC_APPROXIMATION = 1,	// Quadratic approximation(default)
	QUARTIC_INTERPOLATION = 2		// Quartic interpolation
} interpolationType;

// Structure to hold Harris corner information
typedef struct harris_corner {
	float x; // position of the corner
	float y; // position of the corner
	float R; // corner strength

	harris_corner(float xx, float yy, float RR)
	{
		x = xx; y = yy; R = RR;
	}

	harris_corner()
	{
		x = 0; y = 0; R = 0;
	}
}harrisCorner;


#endif// __HARRIS_COMMON_H__
