#include "DropletFittingBase.h"
#include "fit_util.h"

CDropletFittingBase::CDropletFittingBase(CDropletContext* pCtx)
	: m_baselineX(NULL)
	, m_baselineY(NULL)
	, m_pContext(pCtx)
{
}

CDropletFittingBase::~CDropletFittingBase()
{
	m_pContext = NULL;
	m_baselineX = NULL;
	m_baselineY = NULL;
}

void CDropletFittingBase::setParameters(
	double* baselineX,
	double* baselineY
) {

	if (m_pContext) {
		auto left = m_pContext->leftPoint();
		auto right = m_pContext->rightPoint();
		correct_base_params(left, right);
		m_pContext->SetLeftRightPoints(left, right);
	}

	m_baselineX = baselineX;
	m_baselineY = baselineY;	
}