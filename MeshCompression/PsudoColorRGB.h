// PsudoColorRGB.h: interface for the CPsudoColorRGB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PSUDOCOLORRGB_H__7A69032F_2B09_4FE3_809D_80705C4013B8__INCLUDED_)
#define AFX_PSUDOCOLORRGB_H__7A69032F_2B09_4FE3_809D_80705C4013B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//#include <gl\glut.h>

#define ASSERT(b) {if(!(b)) printf("assertion fail.\n");}

typedef enum _PC_TYPE { PCT_JET=0, PCT_HOT, PCT_COOL, 
						PCT_BONE, PCT_FLAG, PCT_ZEBRA,
						PCT_SPACE_JET, PCT_JET_ISOLINE,
						PCT_SPRING, PCT_SUMMER, PCT_AUTUMN,
						PCT_WINTER,PCT_ME} PC_TYPE;

class CPseudoColorRGB
{
public:
	typedef void (CPseudoColorRGB::*GETPCFUNC_DOUBLE) (double* rgb, double v, bool bFullRamp);
	typedef void (CPseudoColorRGB::*GETPCFUNC_UBYTE) (GLubyte* rgb, double v, bool bFullRamp);

	CPseudoColorRGB();
	virtual ~CPseudoColorRGB();

	void SetPCRamp(double low, double high);
	void SetPCValueRange(double min, double max);
	void GetPC(double* rgb, double v, bool bFullRamp);
	void GetPC(GLubyte* rgb, double v, bool bFullRamp);
	void SetPCType(PC_TYPE pct);
	PC_TYPE GetPCType(void) {return m_PCType;};
	
private:
	void getPseudoColorJET(double* rgb, double v, bool bFullRamp);
	void getPseudoColorCOOL(double* rgb, double v, bool bFullRamp);
	void getPseudoColorHOT(double* rgb, double v, bool bFullRamp);
	void getPseudoColorBONE(double* rgb, double v, bool bFullRamp);
	void getPseudoColorFlag(double* rgb, double v, bool bFullRamp);
	void getPseudoColorSpring(double* rgb, double v, bool bFullRamp);
	void getPseudoColorSummer(double* rgb, double v, bool bFullRamp);
	void getPseudoColorAutumn(double* rgb, double v, bool bFullRamp);
	void getPseudoColorWinter(double* rgb, double v, bool bFullRamp);
	void getPseudoColorZebra(double* rgb, double v, bool bFullRamp);
	void getPseudoColorSpaceJet(double* rgb, double v, bool bFullRamp);
	void getPseudoColorJetIsoline(double* rgb, double v, bool bFullRamp);
	void getPseudoColorMe(double* rgb, double v, bool bFullRamp);


	void getPseudoColorJET(GLubyte* rgb, double v, bool bFullRamp);
	void getPseudoColorCOOL(GLubyte* rgb, double v, bool bFullRamp);
	void getPseudoColorHOT(GLubyte* rgb, double v, bool bFullRamp);
	void getPseudoColorBONE(GLubyte* rgb, double v, bool bFullRamp);
	void getPseudoColorFlag(GLubyte* rgb, double v, bool bFullRamp);
	void getPseudoColorSpring(GLubyte* rgb, double v, bool bFullRamp);
	void getPseudoColorSummer(GLubyte* rgb, double v, bool bFullRamp);
	void getPseudoColorAutumn(GLubyte* rgb, double v, bool bFullRamp);
	void getPseudoColorWinter(GLubyte* rgb, double v, bool bFullRamp);
	void getPseudoColorZebra(GLubyte* rgb, double v, bool bFullRamp);
	void getPseudoColorSpaceJet(GLubyte* rgb, double v, bool bFullRamp);
	void getPseudoColorJetIsoline(GLubyte* rgb, double v, bool bFullRamp);
	void getPseudoColorMe(GLubyte* rgb, double v, bool bFullRamp);


	GETPCFUNC_DOUBLE	m_GetPCFuncDbl;
	GETPCFUNC_UBYTE		m_GetPCFuncUbt;
	double m_dPCRampLow;	// min value of pseudo-color scale
	double m_dPCRampHigh;	// max value of pseudo-color scale
	double m_dPCVMin;		// minimum input value
	double m_dPCVMax;		// maximum input value
	double m_dPCVSF;		// scale factor of input value: (high-low)/(max-min) 
	PC_TYPE m_PCType;
};

inline void CPseudoColorRGB::GetPC(double* rgb, double v, bool bFullRamp=false)
{
	if (m_GetPCFuncDbl != NULL) {
		(this->*m_GetPCFuncDbl)(rgb, v, bFullRamp);
	} else {
		rgb[0] = rgb[1] = rgb[2] = 0.4;
	}
}

inline void CPseudoColorRGB::GetPC(GLubyte* rgb, double v, bool bFullRamp=false)
{
	if (m_GetPCFuncUbt != NULL) {
		(this->*m_GetPCFuncUbt)(rgb, v, bFullRamp);
	} else {
		rgb[0] = rgb[1] = rgb[2] = 102;
	}
}

inline void CPseudoColorRGB::getPseudoColorJET(double* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	if (rs < 0.25) {
		rgb[0] = 0.0;
		rgb[1] = rs * 4.0;
		rgb[2] = 1.0;
	} else if (rs < 0.5) {
		rgb[0] = 0.0;
		rgb[1] = 1.0;
		rgb[2] = (0.5 - rs) * 4.0;
	} else if (rs < 0.75) {
		rgb[0] = (rs - 0.5) * 4.0;
		rgb[1] = 1.0;
		rgb[2] = 0.0;
	} else {
		rgb[0] = 1.0;
		rgb[1] = (1.0 - rs) * 4.0;
		rgb[2] = 0.0;
	}
}

inline void CPseudoColorRGB::getPseudoColorJET(GLubyte* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	if (rs < 0.25) {
		rgb[0] = 0;
		rgb[1] = rs * 1020;
		rgb[2] = 255;
	} else if (rs < 0.5) {
		rgb[0] = 0;
		rgb[1] = 255;
		rgb[2] = (0.5 - rs) * 1020;
	} else if (rs < 0.75) {
		rgb[0] = (rs - 0.5) * 1020;
		rgb[1] = 255;
		rgb[2] = 0;
	} else {
		rgb[0] = 255;
		rgb[1] = (1.0 - rs) * 1020;
		rgb[2] = 0;
	}
}

inline void CPseudoColorRGB::getPseudoColorCOOL(double* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	rgb[0] = rs;
	rgb[1] = 1.0 - rs;
	rgb[2] = 1.0;
}

inline void CPseudoColorRGB::getPseudoColorCOOL(GLubyte* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	rgb[0] = rs * 255;
	rgb[1] = 255 - rgb[0];
	rgb[2] = 255;
}

inline void CPseudoColorRGB::getPseudoColorHOT(double* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	if (rs < 0.375) {
		rgb[0] = rs * 2.666667;
		rgb[1] = 0.0;
		rgb[2] = 0.0;
	} else if (rs < 0.75) {
		rgb[0] = 1.0;
		rgb[1] = (rs - 0.375) * 2.666667;
		rgb[2] = 0.0;
	} else {
		rgb[0] = 1.0;
		rgb[1] = 1.0;
		rgb[2] = (rs - 0.75) * 4.0;
	}
}

inline void CPseudoColorRGB::getPseudoColorHOT(GLubyte* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	if (rs < 0.375) {
		rgb[0] = rs * 680;
		rgb[1] = 0;
		rgb[2] = 0;
	} else if (rs < 0.75) {
		rgb[0] = 255;
		rgb[1] = (rs - 0.375) * 680;
		rgb[2] = 0;
	} else {
		rgb[0] = 255;
		rgb[1] = 255;
		rgb[2] = (rs - 0.75) * 1020;
	}
}

inline void CPseudoColorRGB::getPseudoColorBONE(double* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	if (rs < 0.375) {
		rgb[0] = rs * 0.865;
		rgb[1] = rs * 0.865;
		rgb[2] = rs * 1.225;
	} else if (rs < 0.75) {
		rgb[0] = rs * 0.865;
		rgb[1] = 0.324375 + (rs - 0.375) * 1.225;
		rgb[2] = 0.459375 + (rs - 0.375) * 0.865;
	} else {
		rgb[0] = 0.65 + (rs - 0.75) * 1.4;
		rgb[1] = 0.78375 + (rs - 0.75) * 0.865;
		rgb[2] = 0.78375 + (rs - 0.75) * 0.865;
	}
}

inline void CPseudoColorRGB::getPseudoColorBONE(GLubyte* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	if (rs < 0.375) {
		rgb[0] = rs * 221;
		rgb[1] = rgb[0];
		rgb[2] = rs * 312;
	} else if (rs < 0.75) {
		rgb[0] = rs * 221;
		rgb[1] = 83 + (rs - 0.375) * 312;
		rgb[2] = 117 + (rs - 0.375) * 221;
	} else {
		rgb[0] = 166 + (rs - 0.75) * 357;
		rgb[1] = 200 + (rs - 0.75) * 221;
		rgb[2] = 200 + (rs - 0.75) * 221;
	}
}

inline void CPseudoColorRGB::getPseudoColorFlag(double* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	int c = (int)(rs * 64);
	switch(c%4)
	{
	case 0:
		rgb[0] = 1.0;
		rgb[1] = 0.0;
		rgb[2] = 0.0;
		break;
	case 1:
		rgb[0] = 1.0;
		rgb[1] = 1.0;
		rgb[2] = 1.0;
		break;
	case 2:
		rgb[0] = 0.0;
		rgb[1] = 0.0;
		rgb[2] = 1.0;
	    break;
	case 3:
		rgb[0] = 0.0;
		rgb[1] = 0.0;
		rgb[2] = 0.0;
	    break;
	default:
	    break;
	}
}

inline void CPseudoColorRGB::getPseudoColorFlag(GLubyte* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	int c = (int)(rs * 64);
	switch(c%4)
	{
	case 0:
		rgb[0] = 255;
		rgb[1] = 0;
		rgb[2] = 0;
		break;
	case 1:
		rgb[0] = 255;
		rgb[1] = 255;
		rgb[2] = 255;
		break;
	case 2:
		rgb[0] = 0;
		rgb[1] = 0;
		rgb[2] = 255;
		break;
	case 3:
		rgb[0] = 0;
		rgb[1] = 0;
		rgb[2] = 0;
		break;
	default:
		break;
	}
}

inline void CPseudoColorRGB::getPseudoColorSpring(double* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	rgb[0] = 1.0;
	rgb[1] = rs;
	rgb[2] = 1.0 - rs;
}

inline void CPseudoColorRGB::getPseudoColorSpring(GLubyte* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	rgb[0] = 255;
	rgb[1] = rs * 255;
	rgb[2] = 255 - rgb[1];
}

inline void CPseudoColorRGB::getPseudoColorSummer(double* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	rgb[0] = rs;
	rgb[1] = 0.5 + rs * 0.5;
	rgb[2] = 0.4;
}

inline void CPseudoColorRGB::getPseudoColorSummer(GLubyte* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	rgb[0] = rs * 255;
	rgb[1] = 127 + rs * 128;
	rgb[2] = 102;
}

inline void CPseudoColorRGB::getPseudoColorAutumn(double* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	rgb[0] = 1.0;
	rgb[1] = rs;
	rgb[2] = 0.0;
}

inline void CPseudoColorRGB::getPseudoColorAutumn(GLubyte* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	rgb[0] = 255;
	rgb[1] = rs * 255;
	rgb[2] = 0;
}

inline void CPseudoColorRGB::getPseudoColorWinter(double* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	rgb[0] = 0.0;
	rgb[1] = rs;
	rgb[2] = 1.0 - rs * 0.5;
}

inline void CPseudoColorRGB::getPseudoColorWinter(GLubyte* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	rgb[0] = 0;
	rgb[1] = rs * 255;
	rgb[2] = 255 - rs * 128;
}

inline void CPseudoColorRGB::getPseudoColorZebra(double* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	int c = (int)(rs * 64);
	switch(c%2)
	{
	case 0:
		rgb[0] = 0.8;
		rgb[1] = 0.6;
		rgb[2] = 0.1;
		break;
	case 1:
		rgb[0] = 0.1;
		rgb[1] = 0.1;
		rgb[2] = 0.1;
		break;
	default:
		break;
	}
}

inline void CPseudoColorRGB::getPseudoColorZebra(GLubyte* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	int c = (int)(rs * 90);
	switch(c%2)
	{
	case 0:
		rgb[0] = 204;
		rgb[1] = 153;
		rgb[2] = 20;
		break;
	case 1:
		rgb[0] = 20;
		rgb[1] = 20;
		rgb[2] = 20;
		break;
	default:
		break;
	}
}

inline void CPseudoColorRGB::getPseudoColorSpaceJet(double* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	int c = (int)(rs * 64);
	switch(c%2)
	{
	case 0:
		rgb[0] = 0.8;
		rgb[1] = 0.6;
		rgb[2] = 0.1;
		break;
	case 1:
		if (rs < 0.25) {
			rgb[0] = 0.0;
			rgb[1] = rs * 4.0;
			rgb[2] = 1.0;
		} else if (rs < 0.5) {
			rgb[0] = 0.0;
			rgb[1] = 1.0;
			rgb[2] = (0.5 - rs) * 4.0;
		} else if (rs < 0.75) {
			rgb[0] = (rs - 0.5) * 4.0;
			rgb[1] = 1.0;
			rgb[2] = 0.0;
		} else {
			rgb[0] = 1.0;
			rgb[1] = (1.0 - rs) * 4.0;
			rgb[2] = 0.0;
		}
		break;
	default:
		break;
	}
}

inline void CPseudoColorRGB::getPseudoColorSpaceJet(GLubyte* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	int c = (int)(rs * 100);
	switch(c%2)
	{
	case 0:
		if (rs < 0.25) {
			rgb[0] = 0;
			rgb[1] = rs * 1020;
			rgb[2] = 255;
		} else if (rs < 0.5) {
			rgb[0] = 0;
			rgb[1] = 255;
			rgb[2] = (0.5 - rs) * 1020;
		} else if (rs < 0.75) {
			rgb[0] = (rs - 0.5) * 1020;
			rgb[1] = 255;
			rgb[2] = 0;
		} else {
			rgb[0] = 255;
			rgb[1] = (1.0 - rs) * 1020;
			rgb[2] = 0;
		}
		break;
	case 1:
		rgb[0] = 100;
		rgb[1] = 100;
		rgb[2] = 100;
		break;
	default:
		break;
	}
}

inline void CPseudoColorRGB::getPseudoColorJetIsoline(double* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	int c = (int)(rs * 200);
	if (c%10 == 0) {
		rgb[0] = 0.1;
		rgb[1] = 0.1;
		rgb[2] = 0.1;
	} else {
		if (rs < 0.25) {
			rgb[0] = 0.0;
			rgb[1] = rs * 4.0;
			rgb[2] = 1.0;
		} else if (rs < 0.5) {
			rgb[0] = 0.0;
			rgb[1] = 1.0;
			rgb[2] = (0.5 - rs) * 4.0;
		} else if (rs < 0.75) {
			rgb[0] = (rs - 0.5) * 4.0;
			rgb[1] = 1.0;
			rgb[2] = 0.0;
		} else {
			rgb[0] = 1.0;
			rgb[1] = (1.0 - rs) * 4.0;
			rgb[2] = 0.0;
		}
	}
}

inline void CPseudoColorRGB::getPseudoColorJetIsoline(GLubyte* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (bFullRamp) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	int c = (int)(rs * 200);
	if (c%10 == 0) {
		rgb[0] = 20;
		rgb[1] = 20;
		rgb[2] = 20;
	} else {
		if (rs < 0.25) {
			rgb[0] = 0;
			rgb[1] = rs * 1020;
			rgb[2] = 255;
		} else if (rs < 0.5) {
			rgb[0] = 0;
			rgb[1] = 255;
			rgb[2] = (0.5 - rs) * 1020;
		} else if (rs < 0.75) {
			rgb[0] = (rs - 0.5) * 1020;
			rgb[1] = 255;
			rgb[2] = 0;
		} else {
			rgb[0] = 255;
			rgb[1] = (1.0 - rs) * 1020;
			rgb[2] = 0;
		}
	}
}

inline void CPseudoColorRGB::getPseudoColorMe(GLubyte* rgb, double v, bool bFullRamp)
{
	ASSERT(rgb != NULL);
	double rs;
	if (1) {
		rs = (v-m_dPCVMin)/(m_dPCVMax-m_dPCVMin);
	} else {
		rs = m_dPCRampLow+(v-m_dPCVMin)*m_dPCVSF;
	}
	rgb[0] = 49+181*rs;
	rgb[1] = 149-149*rs;
	rgb[2] = 191-173*rs;
}

inline void CPseudoColorRGB::getPseudoColorMe(double* rgb, double v, bool bFullRamp)
{

}

#endif // !defined(AFX_PSUDOCOLORRGB_H__7A69032F_2B09_4FE3_809D_80705C4013B8__INCLUDED_)
