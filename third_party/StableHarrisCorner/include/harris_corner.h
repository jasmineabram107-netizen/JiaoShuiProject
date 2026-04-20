#ifndef __HARRIS_CORNER_H__
#define __HARRIS_CORNER_H__

#include "harris_common.h"
#include <vector>


/*
* Main function for computing Harris corners
  * It applies Gaussian smoothing, computes gradients, 
  * calculates the autocorrelation matrix, and selects corners.
  *
  * @param I: Input image.
  * @param corners: Output vector to store detected corners.
  * @param gauss: Type of Gaussian to use for smoothing.
  * @param grad: Type of gradient to compute.
  * @param measure: Measure for the discriminant function.
  * @param k: Harris constant for the measure function.
  * @param sigma_d: Standard deviation for smoothing (image denoising).
  * @param sigma_i: Standard deviation for smoothing (pixel neighbourhood).
  * @param Th: Threshold for eliminating low values in the response map.
  * @param strategy: Strategy for selecting output corners.
  * @param cells: Number of regions in the image for distributed output.
  * @param N: Number of output corners to select.
  * @param precision: Type of subpixel precision approximation.
  * @param nx: Number of columns of the image.
  * @param ny: Number of rows of the image.
  * @param verbose: Activate verbose mode (0 or 1).
*/
void harris(
    float* I,        
    std::vector<harris_corner>& corners,
    GaussianType   gauss,
    GradientType   grad,      
    CornerStyle   measure,
    float k,         
    float sigma_d,   
    float sigma_i,   
    float Th,        
    CornerSelectStrategies   strategy,
    int   cells,     
    int   N,         
    InterpolationType   precision,
    int   nx,        
    int   ny,        
    int   verbose = 0    
);


/*
* Function for computing Harris corners at multiple scales
  * It applies Gaussian smoothing, computes gradients, 
  * calculates the autocorrelation matrix, and selects stable corners.
  *
  * @param I: Input image.
  * @param corners: Output vector to store detected corners.
  * @param Nscales: Number of scales for checking the stability of corners.
  * @param gauss: Type of Gaussian to use for smoothing.
  * @param grad: Type of gradient to compute.
  * @param measure: Measure for the discriminant function.
  * @param k: Harris constant for the measure function.
  * @param sigma_d: Standard deviation for smoothing (image denoising).
  * @param sigma_i: Standard deviation for smoothing (pixel neighbourhood).
  * @param Th: Threshold for eliminating low values in the response map.
  * @param strategy: Strategy for selecting output corners.
  * @param cells: Number of regions in the image for distributed output.
  * @param N: Number of output corners to select.
  * @param precision: Type of subpixel precision approximation.
  * @param nx: Number of columns of the image.
  * @param ny: Number of rows of the image.
  * @param verbose: Activate verbose mode (0 or 1).
*/
void harris_scale(
    float* I,       
    std::vector<harris_corner>& corners, 
    int   Nscales,   
    GaussianType   gauss,
    GradientType   grad,
    CornerStyle   measure,
    float k,         
    float sigma_d,   
    float sigma_i,   
    float Th,        
    CornerSelectStrategies   strategy,
    int   cells,     
    int   N,         
    InterpolationType   precision,
    int   nx,        
    int   ny,        
    int   verbose = 0    
);



#endif//__JIAOSHUI_HARRIS_CORNER_H__