#ifndef __DROPLET_DEMO_COMMON_THREAD_H__
#define __DROPLET_DEMO_COMMON_THREAD_H__

typedef struct tagAutoTestInParam {
    DropletFitMode  dropletFitMode;
    BaseLineFitMode baselineFitMode;
    MainShapeType   mainShape;
    bool            isSubpixel;
    bool            isBoundary;
    bool            rotated;
    HorizonMode     horMode;
    bool            hasPin;    
    tagAutoTestInParam() {
		dropletFitMode = eDropletEllipseAMS;
		baselineFitMode = eBaseLineEllipseAMS;
		mainShape = eShapeUnknown;
		isSubpixel = true;
		rotated = false;
		horMode = eHmUnknow;
		hasPin = false;
        isBoundary = true;
	}
}AutoTestInParam;

typedef struct tagAutoTestOutParam {
    bool result;
    CString filePath;
    double angles[2];
    MainShapeType mainShape;
    bool hasPin;
    bool isEmpty;
    HorizonMode horMode;
    CString resultString;
    FittingInfo* pFittingInfo;
    RectangleF basePoints;
    //PointF keyPts[3];
    tagAutoTestOutParam() {
    	result = false;
		angles[0] = angles[1] = 0.0;
		mainShape = eShapeUnknown;
		hasPin = false;
		isEmpty = false;
		horMode = eHmUnknow;	
        pFittingInfo = NULL;   
    }
}AutoTestOutParam;

#endif//__DROPLET_DEMO_COMMON_THREAD_H__