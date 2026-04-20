#ifndef __JIAOSHUILIB_FITTINGBASE_H__
#define __JIAOSHUILIB_FITTINGBASE_H__
#include "../DropletContext.h"

class CDropletFittingBase 
{
public:
	CDropletFittingBase(CDropletContext* pParent);
	virtual ~CDropletFittingBase();
	void setParameters(
		double* baselineX = NULL,
		double* baselineY = NULL
	);
	virtual ErrorCode process() = 0;
protected:
	CDropletContext*		m_pContext;
	double*					m_baselineX;
	double*					m_baselineY;

	//std::vector<cv::Point> m_leftPolyline;		// left points on polymonial fitting
	//std::vector<cv::Point> m_rightPolyline;		// right points on polymonial fitting
protected:
};
#endif//__JIAOSHUILIB_FITTINGBASE_H__
