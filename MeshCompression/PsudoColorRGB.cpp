// PsudoColorRGB.cpp: implementation of the CPsudoColorRGB class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PsudoColorRGB.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPseudoColorRGB::CPseudoColorRGB()
{
	m_dPCRampLow = 0.0;
	m_dPCRampHigh = 1.0;
}

CPseudoColorRGB::~CPseudoColorRGB( )
{

}

void CPseudoColorRGB::SetPCRamp(double low, double high)
{
	m_dPCRampLow = low;
	m_dPCRampHigh = high;
}

void CPseudoColorRGB::SetPCValueRange(double min, double max)
{
	m_dPCVMin = min;
	m_dPCVMax = max;
	m_dPCVSF = (m_dPCRampHigh-m_dPCRampLow)/(m_dPCVMax-m_dPCVMin);
}

void CPseudoColorRGB::SetPCType(PC_TYPE pct)
{
	m_PCType = pct;
	switch(m_PCType)
	{
	case PCT_JET:
		m_GetPCFuncDbl = &CPseudoColorRGB::getPseudoColorJET;
		m_GetPCFuncUbt = &CPseudoColorRGB::getPseudoColorJET;
		break;
	case PCT_COOL:
		m_GetPCFuncDbl = &CPseudoColorRGB::getPseudoColorCOOL;
		m_GetPCFuncUbt = &CPseudoColorRGB::getPseudoColorCOOL;
		break;
	case PCT_HOT:
		m_GetPCFuncDbl = &CPseudoColorRGB::getPseudoColorHOT;
		m_GetPCFuncUbt = &CPseudoColorRGB::getPseudoColorHOT;
		break;
	case PCT_BONE:
		m_GetPCFuncDbl = &CPseudoColorRGB::getPseudoColorBONE;
		m_GetPCFuncUbt = &CPseudoColorRGB::getPseudoColorBONE;
		break;
	case PCT_FLAG:
		m_GetPCFuncDbl = &CPseudoColorRGB::getPseudoColorFlag;
		m_GetPCFuncUbt = &CPseudoColorRGB::getPseudoColorFlag;
		break;
	case PCT_ZEBRA:
		m_GetPCFuncDbl = &CPseudoColorRGB::getPseudoColorZebra;
		m_GetPCFuncUbt = &CPseudoColorRGB::getPseudoColorZebra;
		break;
	case PCT_SPACE_JET:
		m_GetPCFuncDbl = &CPseudoColorRGB::getPseudoColorSpaceJet;
		m_GetPCFuncUbt = &CPseudoColorRGB::getPseudoColorSpaceJet;
		break;
	case PCT_JET_ISOLINE:
		m_GetPCFuncDbl = &CPseudoColorRGB::getPseudoColorJetIsoline;
		m_GetPCFuncUbt = &CPseudoColorRGB::getPseudoColorJetIsoline;
		break;
	case PCT_SPRING:
		m_GetPCFuncDbl = &CPseudoColorRGB::getPseudoColorSpring;
		m_GetPCFuncUbt = &CPseudoColorRGB::getPseudoColorSpring;
		break;
	case PCT_SUMMER:
		m_GetPCFuncDbl = &CPseudoColorRGB::getPseudoColorSummer;
		m_GetPCFuncUbt = &CPseudoColorRGB::getPseudoColorSummer;
		break;
	case PCT_AUTUMN:
		m_GetPCFuncDbl = &CPseudoColorRGB::getPseudoColorAutumn;
		m_GetPCFuncUbt = &CPseudoColorRGB::getPseudoColorAutumn;
		break;
	case PCT_WINTER:
		m_GetPCFuncDbl = &CPseudoColorRGB::getPseudoColorWinter;
		m_GetPCFuncUbt = &CPseudoColorRGB::getPseudoColorWinter;
		break;
	case PCT_ME:
		m_GetPCFuncDbl = &CPseudoColorRGB::getPseudoColorMe;
		m_GetPCFuncUbt = &CPseudoColorRGB::getPseudoColorMe;
		break;
	default:
		//AfxMessageBox("Error pseudo-color type!");
		m_GetPCFuncDbl = &CPseudoColorRGB::getPseudoColorJET;
		m_GetPCFuncUbt = &CPseudoColorRGB::getPseudoColorJET;
		break;
	}
}