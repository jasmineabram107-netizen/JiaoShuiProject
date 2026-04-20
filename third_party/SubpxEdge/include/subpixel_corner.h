#ifndef __SUBPIXEL_EDGE_DETECT_H__
#define __SUBPIXEL_EDGE_DETECT_H__

/**
 * @brief Chained sub-pixel edge detector based on modified Canny non-maximal suppression
 *        and modified Devernay sub-pixel correction.
 *
 * @param[out] x             Pointer to array storing sub-pixel x-coordinates of edge points.
 * @param[out] y             Pointer to array storing sub-pixel y-coordinates of edge points.
 * @param[out] N             Total number of edge points stored in x and y.
 * @param[out] curve_limits  Pointer to array storing curve segment boundaries. Each curve k satisfies:
 *                           curve_limits[k] <= i < curve_limits[k+1]
 * @param[out] M             Number of detected curves.
 * @param[in]  image         Input grayscale image as a 1D array (row-major), size X * Y.
 *                           image[x + y * X] gives pixel at coordinate (x, y).
 * @param[in]  X             Width of the image.
 * @param[in]  Y             Height of the image.
 * @param[in]  sigma         Standard deviation for Gaussian filtering. If 0, no filtering is applied.
 * @param[in]  th_h          High gradient threshold for Canny hysteresis.
 * @param[in]  th_l          Low gradient threshold for Canny hysteresis.
 *
 * @details
 * This function computes edge points at sub-pixel precision using gradient-based filtering,
 * non-maximal suppression (in horizontal/vertical directions), and chaining logic for curve grouping.
 *
 * The output (x, y, curve_limits) must be manually freed by the caller after use.
 *
 * A curve k is considered closed if:
 *   x[curve_limits[k]] == x[curve_limits[k+1] - 1] &&
 *   y[curve_limits[k]] == y[curve_limits[k+1] - 1]
 */
void devernay(
    double** x, 
    double** y, 
    int* N, 
    int** curve_limits, 
    int* M,
    double* image, 
    int X, 
    int Y,
    double sigma, 
    double th_h, 
    double th_l
);

#endif//__SUBPIXEL_EDGE_DETECT_H__