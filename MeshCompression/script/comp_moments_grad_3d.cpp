 // ----------------------------------------------------------------------- -
 //  copyright : (c)2014 - 2015 Przemyslaw Musialski
 //  email     : pm@cg.tuwien.ac.at
 //  created   : 11 / 06 / 2014
 //  edited	   : 04 / 26 / 2015
 //------------------------------------------------------------------------ -
 //
 //  This code implements the computation of mass moments and their gradiens as 
 //  described in the appendix of the paper:
 //
 //  P.Musialski, T.Auzinger, M.Birsak, M.Wimmer, and L.Kobbelt,
 //  Reduced - Order Shape Optimization Using Offset Surfaces,
 //  ACM Trans.Graph. (Proc.ACM SIGGRAPH 2015), vol. 34, no. 4, pp. 102 : 1–102 : 9, Jul. 2015.
 //
 //  See https://www.cg.tuwien.ac.at/research/publications/2015/musialski-2015-souos/musialski-2015-souos-additional.pdf
 //  for further details. 
 //
 //  If you use the code or parts of it in your academic work you need to cite the paper :
 //
 //  @article{ Musialski2015a,
 //  author = { Musialski, Przemyslaw and Auzinger, Thomas and Birsak, Michael and Wimmer, Michael and Kobbelt, Leif },
 //  title = { { Reduced-Order Shape Optimization Using Offset Surfaces } },
 //  journal = { ACM Transactions on Graphics (Proc.ACM SIGGRAPH 2015) },
 //  volume = { 34 },
 //  year = { 2015 }
 //  doi = { 10.1145/2766955 },
 //  issn = { 07300301 },
 //  month = jul,
 //  number = { 4 },
 //  pages = { 102:1--102 : 9 },
 //  url = { http://www.cg.tuwien.ac.at/research/publications/2015/musialski-2015-souos/ http://dl.acm.org/citation.cfm?doid=2809654.2766955},
 //  }
 //
 // ------------------------------------------------------------------------ -
 //
 //  This program is free software : you can redistribute it and / or modify
 //  it under the terms of the GNU General Public License as published by
 //  the Free Software Foundation, either version 3 of the License, or
 //  (at your option) any later version.
 //
 //  This program is distributed in the hope that it will be useful,
 //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 //  GNU General Public License for more details.
 //
 //  You should have received a copy of the GNU General Public License
 //  along with this program.If not, see <http://www.gnu.org/licenses/>.
 //
 // ------------------------------------------------------------------------ -

#include <math.h>
#include <matrix.h>
#include <mex.h>

#define Mat(row, col, cstride) ( (col) * (cstride) + (row) )

#define oneDiv6   (1.0 / 6.0)
#define oneDiv24  (1.0 / 24.0)
#define oneDiv60  (1.0 / 60.0)
#define oneDiv120 (1.0 / 120.0)

#define  S__ 0
#define  SX_ 1
#define  SY_ 2
#define  SZ_ 3
#define  SXX 4
#define  SYY 5
#define  SZZ 6
#define  SXY 7
#define  SYZ 8
#define  SZX 9
#define  X_ 0
#define  Y_ 1
#define  Z_ 2


inline double Dsdx(double sx, double s, double dsx, double ds)
{
	return (-sx*ds + s*dsx) / (s*s);
}

inline double Dsdxx(
	double sy, double sz, double s,
	double dsyy, double dszz, double dsy, double dsz, double ds)
{
	double s2  = s*s;	
	double sy2 = sy*sy;
	double sz2 = sz*sz;
	return (sy2*ds + sz2*ds - 2*s*sy*dsy - 2*s*sz*dsz + s2*(dsyy + dszz)) / s2;	
}

inline double Dsdxy(
        double sx, double sy, double s, 
        double dsxy, double dsx, double dsy, double ds )
{
	return (s*(sy*dsx - s*dsxy) + sx*(-sy*ds + s*dsy)) / (s*s);
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	/* pointers to input & output matrices*/
	double *V1 = mxGetPr(prhs[0]); /* 1 input matrix: points per triangle NTx3 */
	double *V2 = mxGetPr(prhs[1]); /* 2 input matrix: points per triangle NTx3 */
	double *V3 = mxGetPr(prhs[2]); /* 3 input matrix: points per triangle NTx3 */
	double *VI = mxGetPr(prhs[3]); /* 4 input matrix: triangle vertex indices: NTx3 */
	double *N0 = mxGetPr(prhs[4]); /* 5 input matrix: normals per vertex: Nx3 */

	/* check dimensions of input matrices */
	size_t mrows = mxGetM(prhs[0]);
	size_t ncols = mxGetN(prhs[0]);
	size_t mrows1 = mxGetM(prhs[1]);
	size_t ncols1 = mxGetN(prhs[1]);
	size_t mrows2 = mxGetM(prhs[2]);
	size_t ncols2 = mxGetN(prhs[2]);
	size_t mrows3 = mxGetM(prhs[3]);
	size_t ncols3 = mxGetN(prhs[3]);
	size_t mrows4 = mxGetM(prhs[4]);
	size_t ncols4 = mxGetN(prhs[4]);

	if (ncols != 3 ||
		ncols1 != 3 ||
		ncols2 != 3 ||
		ncols3 != 3 ||
		ncols4 != 3)
	{
		mexErrMsgIdAndTxt("MATLAB:matchdims",
			"All matrices must have 3 columns.");
	}

	if (mrows != mrows1 || ncols != ncols1 ||
		mrows != mrows2 || ncols != ncols2 ||
		mrows != mrows3 || ncols != ncols3)
	{
		mexErrMsgIdAndTxt("MATLAB:matchdims",
			"Dimensions of matices do not match.");
	}

	/* number of normals == number of points */
	int Np = (int)mrows4;
	int Nt = (int)mrows;
	int Nm = 10; // number moments

	/* create output matrices */
	plhs[0] = mxCreateDoubleMatrix(Nm, 1, mxREAL);
	plhs[1] = mxCreateDoubleMatrix(Nm, 3 * Np, mxREAL);
	plhs[2] = mxCreateDoubleMatrix(Nm, Np, mxREAL);
	double *M = mxGetPr(plhs[0]);
	double *DmDx = mxGetPr(plhs[1]);
	double *DmDd = mxGetPr(plhs[2]);
	//	double *ds = new double[Nm * 3 * Np];


	const bool bodyCoords = true;
	double mass, cmx, cmy, cmz;

	// order:  1, x, y, z, x^2, y^2, z^2, xy, yz, zx
	double intg[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	double s[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


	for (int i = 0; i < Nm; i++) { M[i] = 0; }
	for (int i = 0; i < Nm * 3 * Np; i++) { DmDx[i] = 0; }
	for (int i = 0; i < Nm * Np; i++) { DmDd[i] = 0; }


	for (int j = 0; j < Nt; j++)
	{
		// get vertices of triangle i
		double x0 = V1[Mat(j, X_, Nt)]; //  Mat(row,col,cstride) = (col*cstride+row)
		double y0 = V1[Mat(j, Y_, Nt)];
		double z0 = V1[Mat(j, Z_, Nt)];

		double x1 = V2[Mat(j, X_, Nt)];
		double y1 = V2[Mat(j, Y_, Nt)];
		double z1 = V2[Mat(j, Z_, Nt)];

		double x2 = V3[Mat(j, X_, Nt)];
		double y2 = V3[Mat(j, Y_, Nt)];
		double z2 = V3[Mat(j, Z_, Nt)];

		// get edges and cross product of edges
		double a1 = x1 - x0;
		double b1 = y1 - y0;
		double c1 = z1 - z0;
		double a2 = x2 - x0;
		double b2 = y2 - y0;
		double c2 = z2 - z0;
		double d0 = b1*c2 - b2*c1;
		double d1 = a2*c1 - a1*c2;
		double d2 = a1*b2 - a2*b1;

		// compute gradient						
		// particular terms were automatically generated with Mathematica
		// code could be optimized for better performance

		double x0_2 = x0*x0;
		double x0_3 = x0*x0*x0;
		double x1_2 = x1*x1;
		double x1_3 = x1*x1*x1;
		double x2_2 = x2*x2;
		double x2_3 = x2*x2*x2;

		double y0_2 = y0*y0;
		double y0_3 = y0*y0*y0;
		double y1_2 = y1*y1;
		double y1_3 = y1*y1*y1;
		double y2_2 = y2*y2;
		double y2_3 = y2*y2*y2;

		double z0_2 = z0*z0;
		double z0_3 = z0*z0*z0;
		double z1_2 = z1*z1;
		double z1_3 = z1*z1*z1;
		double z2_2 = z2*z2;
		double z2_3 = z2*z2*z2;

		// get integrals (include all constants)
		s[0] += ((x0 + x1 + x2)*(-((-y0 + y2)*(-z0 + z1)) + (-y0 + y1)*(-z0 + z2))) / 6.;
		s[1] += ((x0_2 + x0*x1 + x1_2 + x0*x2 + x1*x2 + x2_2)*(-((-y0 + y2)*(-z0 + z1)) + (-y0 + y1)*(-z0 + z2))) / 24.;
		s[2] += ((y0_2 + y0*y1 + y1_2 + y0*y2 + y1*y2 + y2_2)*((-x0 + x2)*(-z0 + z1) - (-x0 + x1)*(-z0 + z2))) / 24.;
		s[3] += ((-((-x0 + x2)*(-y0 + y1)) + (-x0 + x1)*(-y0 + y2))*(z0_2 + z0*z1 + z1_2 + z0*z2 + z1*z2 + z2_2)) / 24.;
		s[4] += ((x0_3 + x1_3 + x1_2*x2 + x1*x2_2 + x2_3 + x0_2*(x1 + x2) + x0*(x1_2 + x1*x2 + x2_2))*(y2*(z0 - z1) + y0*(z1 - z2) + y1*(-z0 + z2))) / 60.;
		s[5] += ((y0_3 + y1_3 + y1_2*y2 + y1*y2_2 + y2_3 + y0_2*(y1 + y2) + y0*(y1_2 + y1*y2 + y2_2))*(x2*(-z0 + z1) + x1*(z0 - z2) + x0*(-z1 + z2))) / 60.;
		s[6] += ((x2*(y0 - y1) + x0*(y1 - y2) + x1*(-y0 + y2))*(z0_3 + z1_3 + z1_2*z2 + z1*z2_2 + z2_3 + z0_2*(z1 + z2) + z0*(z1_2 + z1*z2 + z2_2))) / 60.;
		s[7] += ((x0_2*(3 * y0 + y1 + y2) + x1_2*(y0 + 3 * y1 + y2) + x2_2*(y0 + y1 + 3 * y2) + x1*x2*(y0 + 2 * (y1 + y2)) + x0*(x1*(2 * y0 + 2 * y1 + y2) + x2*(2 * y0 + y1 + 2 * y2)))*(y2*(z0 - z1) + y0*(z1 - z2) + y1*(-z0 + z2))) / 120.;
		s[8] += ((x2*(-z0 + z1) + x1*(z0 - z2) + x0*(-z1 + z2))*(y0_2*(3 * z0 + z1 + z2) + y1_2*(z0 + 3 * z1 + z2) + y2_2*(z0 + z1 + 3 * z2) + y1*y2*(z0 + 2 * (z1 + z2)) + y0*(y1*(2 * z0 + 2 * z1 + z2) + y2*(2 * z0 + z1 + 2 * z2)))) / 120.;
		s[9] += ((x2*(y0 - y1) + x0*(y1 - y2) + x1*(-y0 + y2))*(x1*(z0_2 + 2 * z0*z1 + 3 * z1_2 + z0*z2 + 2 * z1*z2 + z2_2) + x2*(z0_2 + z0*z1 + z1_2 + 2 * z0*z2 + 2 * z1*z2 + 3 * z2_2) + x0*(3 * z0_2 + z1_2 + z1*z2 + z2_2 + 2 * z0*(z1 + z2)))) / 120.;

		// get vertex-indices of the current triangle vertices
		int i0 = (int)(3 * (VI[Mat(j, 0, Nt)] - 1));
		int i1 = (int)(3 * (VI[Mat(j, 1, Nt)] - 1));
		int i2 = (int)(3 * (VI[Mat(j, 2, Nt)] - 1));

		// get all dertivatives (also include all constants)
		//ds1/d{xi,yi,zi}
		DmDx[Mat(0, i0 + 0, Nm)] += (y2*(z0 - z1) + y0*(z1 - z2) + y1*(-z0 + z2)) / 6.;
		DmDx[Mat(0, i1 + 0, Nm)] += (y2*(z0 - z1) + y0*(z1 - z2) + y1*(-z0 + z2)) / 6.;
		DmDx[Mat(0, i2 + 0, Nm)] += (y2*(z0 - z1) + y0*(z1 - z2) + y1*(-z0 + z2)) / 6.;
		DmDx[Mat(0, i0 + 1, Nm)] += ((x0 + x1 + x2)*(z1 - z2)) / 6.;
		DmDx[Mat(0, i1 + 1, Nm)] += ((x0 + x1 + x2)*(-z0 + z2)) / 6.;
		DmDx[Mat(0, i2 + 1, Nm)] += ((x0 + x1 + x2)*(z0 - z1)) / 6.;
		DmDx[Mat(0, i0 + 2, Nm)] += ((x0 + x1 + x2)*(-y1 + y2)) / 6.;
		DmDx[Mat(0, i1 + 2, Nm)] += ((x0 + x1 + x2)*(y0 - y2)) / 6.;
		DmDx[Mat(0, i2 + 2, Nm)] += ((x0 + x1 + x2)*(-y0 + y1)) / 6.;
		//dsx/d{xi,yi,zi}
		DmDx[Mat(1, i0 + 0, Nm)] += ((2 * x0 + x1 + x2)*(y2*(z0 - z1) + y0*(z1 - z2) + y1*(-z0 + z2))) / 24.;
		DmDx[Mat(1, i1 + 0, Nm)] += ((x0 + 2 * x1 + x2)*(y2*(z0 - z1) + y0*(z1 - z2) + y1*(-z0 + z2))) / 24.;
		DmDx[Mat(1, i2 + 0, Nm)] += ((x0 + x1 + 2 * x2)*(y2*(z0 - z1) + y0*(z1 - z2) + y1*(-z0 + z2))) / 24.;
		DmDx[Mat(1, i0 + 1, Nm)] += ((x0_2 + x1_2 + x1*x2 + x2_2 + x0*(x1 + x2))*(z1 - z2)) / 24.;
		DmDx[Mat(1, i1 + 1, Nm)] += ((x0_2 + x1_2 + x1*x2 + x2_2 + x0*(x1 + x2))*(-z0 + z2)) / 24.;
		DmDx[Mat(1, i2 + 1, Nm)] += ((x0_2 + x1_2 + x1*x2 + x2_2 + x0*(x1 + x2))*(z0 - z1)) / 24.;
		DmDx[Mat(1, i0 + 2, Nm)] += ((x0_2 + x1_2 + x1*x2 + x2_2 + x0*(x1 + x2))*(-y1 + y2)) / 24.;
		DmDx[Mat(1, i1 + 2, Nm)] += ((x0_2 + x1_2 + x1*x2 + x2_2 + x0*(x1 + x2))*(y0 - y2)) / 24.;
		DmDx[Mat(1, i2 + 2, Nm)] += ((x0_2 + x1_2 + x1*x2 + x2_2 + x0*(x1 + x2))*(-y0 + y1)) / 24.;
		//dsy/d{xi,yi,zi}
		DmDx[Mat(2, i0 + 0, Nm)] += ((y0_2 + y1_2 + y1*y2 + y2_2 + y0*(y1 + y2))*(-z1 + z2)) / 24.;
		DmDx[Mat(2, i1 + 0, Nm)] += ((y0_2 + y1_2 + y1*y2 + y2_2 + y0*(y1 + y2))*(z0 - z2)) / 24.;
		DmDx[Mat(2, i2 + 0, Nm)] += ((y0_2 + y1_2 + y1*y2 + y2_2 + y0*(y1 + y2))*(-z0 + z1)) / 24.;
		DmDx[Mat(2, i0 + 1, Nm)] += ((2 * y0 + y1 + y2)*(x2*(-z0 + z1) + x1*(z0 - z2) + x0*(-z1 + z2))) / 24.;
		DmDx[Mat(2, i1 + 1, Nm)] += ((y0 + 2 * y1 + y2)*(x2*(-z0 + z1) + x1*(z0 - z2) + x0*(-z1 + z2))) / 24.;
		DmDx[Mat(2, i2 + 1, Nm)] += ((y0 + y1 + 2 * y2)*(x2*(-z0 + z1) + x1*(z0 - z2) + x0*(-z1 + z2))) / 24.;
		DmDx[Mat(2, i0 + 2, Nm)] += ((x1 - x2)*(y0_2 + y1_2 + y1*y2 + y2_2 + y0*(y1 + y2))) / 24.;
		DmDx[Mat(2, i1 + 2, Nm)] += ((-x0 + x2)*(y0_2 + y1_2 + y1*y2 + y2_2 + y0*(y1 + y2))) / 24.;
		DmDx[Mat(2, i2 + 2, Nm)] += ((x0 - x1)*(y0_2 + y1_2 + y1*y2 + y2_2 + y0*(y1 + y2))) / 24.;
		//dsz/d{xi,yi,zi}
		DmDx[Mat(3, i0 + 0, Nm)] += ((y1 - y2)*(z0_2 + z1_2 + z1*z2 + z2_2 + z0*(z1 + z2))) / 24.;
		DmDx[Mat(3, i1 + 0, Nm)] += ((-y0 + y2)*(z0_2 + z1_2 + z1*z2 + z2_2 + z0*(z1 + z2))) / 24.;
		DmDx[Mat(3, i2 + 0, Nm)] += ((y0 - y1)*(z0_2 + z1_2 + z1*z2 + z2_2 + z0*(z1 + z2))) / 24.;
		DmDx[Mat(3, i0 + 1, Nm)] += ((-x1 + x2)*(z0_2 + z1_2 + z1*z2 + z2_2 + z0*(z1 + z2))) / 24.;
		DmDx[Mat(3, i1 + 1, Nm)] += ((x0 - x2)*(z0_2 + z1_2 + z1*z2 + z2_2 + z0*(z1 + z2))) / 24.;
		DmDx[Mat(3, i2 + 1, Nm)] += ((-x0 + x1)*(z0_2 + z1_2 + z1*z2 + z2_2 + z0*(z1 + z2))) / 24.;
		DmDx[Mat(3, i0 + 2, Nm)] += ((x2*(y0 - y1) + x0*(y1 - y2) + x1*(-y0 + y2))*(2 * z0 + z1 + z2)) / 24.;
		DmDx[Mat(3, i1 + 2, Nm)] += ((x2*(y0 - y1) + x0*(y1 - y2) + x1*(-y0 + y2))*(z0 + 2 * z1 + z2)) / 24.;
		DmDx[Mat(3, i2 + 2, Nm)] += ((x2*(y0 - y1) + x0*(y1 - y2) + x1*(-y0 + y2))*(z0 + z1 + 2 * z2)) / 24.;
		//dsxx/d{xi,yi,zi}
		DmDx[Mat(4, i0 + 0, Nm)] += ((3 * x0_2 + x1_2 + x1*x2 + x2_2 + 2 * x0*(x1 + x2))*(y2*(z0 - z1) + y0*(z1 - z2) + y1*(-z0 + z2))) / 60.;
		DmDx[Mat(4, i1 + 0, Nm)] += ((x0_2 + 3 * x1_2 + 2 * x1*x2 + x2_2 + x0*(2 * x1 + x2))*(y2*(z0 - z1) + y0*(z1 - z2) + y1*(-z0 + z2))) / 60.;
		DmDx[Mat(4, i2 + 0, Nm)] += ((x0_2 + x0*x1 + x1_2 + 2 * (x0 + x1)*x2 + 3 * x2_2)*(y2*(z0 - z1) + y0*(z1 - z2) + y1*(-z0 + z2))) / 60.;
		DmDx[Mat(4, i0 + 1, Nm)] += ((x0_3 + x0_2*(x1 + x2) + (x1 + x2)*(x1_2 + x2_2) + x0*(x1_2 + x1*x2 + x2_2))*(z1 - z2)) / 60.;
		DmDx[Mat(4, i1 + 1, Nm)] += -((x0_3 + x0_2*(x1 + x2) + (x1 + x2)*(x1_2 + x2_2) + x0*(x1_2 + x1*x2 + x2_2))*(z0 - z2)) / 60.;
		DmDx[Mat(4, i2 + 1, Nm)] += ((x0_3 + x0_2*(x1 + x2) + (x1 + x2)*(x1_2 + x2_2) + x0*(x1_2 + x1*x2 + x2_2))*(z0 - z1)) / 60.;
		DmDx[Mat(4, i0 + 2, Nm)] += -((x0_3 + x0_2*(x1 + x2) + (x1 + x2)*(x1_2 + x2_2) + x0*(x1_2 + x1*x2 + x2_2))*(y1 - y2)) / 60.;
		DmDx[Mat(4, i1 + 2, Nm)] += ((x0_3 + x0_2*(x1 + x2) + (x1 + x2)*(x1_2 + x2_2) + x0*(x1_2 + x1*x2 + x2_2))*(y0 - y2)) / 60.;
		DmDx[Mat(4, i2 + 2, Nm)] += -((x0_3 + x0_2*(x1 + x2) + (x1 + x2)*(x1_2 + x2_2) + x0*(x1_2 + x1*x2 + x2_2))*(y0 - y1)) / 60.;
		//dsyy/d{xi,yi,zi}
		DmDx[Mat(5, i0 + 0, Nm)] += -((y0_3 + y0_2*(y1 + y2) + (y1 + y2)*(y1_2 + y2_2) + y0*(y1_2 + y1*y2 + y2_2))*(z1 - z2)) / 60.;
		DmDx[Mat(5, i1 + 0, Nm)] += ((y0_3 + y0_2*(y1 + y2) + (y1 + y2)*(y1_2 + y2_2) + y0*(y1_2 + y1*y2 + y2_2))*(z0 - z2)) / 60.;
		DmDx[Mat(5, i2 + 0, Nm)] += -((y0_3 + y0_2*(y1 + y2) + (y1 + y2)*(y1_2 + y2_2) + y0*(y1_2 + y1*y2 + y2_2))*(z0 - z1)) / 60.;
		DmDx[Mat(5, i0 + 1, Nm)] += ((3 * y0_2 + y1_2 + y1*y2 + y2_2 + 2 * y0*(y1 + y2))*(x2*(-z0 + z1) + x1*(z0 - z2) + x0*(-z1 + z2))) / 60.;
		DmDx[Mat(5, i1 + 1, Nm)] += ((y0_2 + 3 * y1_2 + 2 * y1*y2 + y2_2 + y0*(2 * y1 + y2))*(x2*(-z0 + z1) + x1*(z0 - z2) + x0*(-z1 + z2))) / 60.;
		DmDx[Mat(5, i2 + 1, Nm)] += ((y0_2 + y0*y1 + y1_2 + 2 * (y0 + y1)*y2 + 3 * y2_2)*(x2*(-z0 + z1) + x1*(z0 - z2) + x0*(-z1 + z2))) / 60.;
		DmDx[Mat(5, i0 + 2, Nm)] += ((x1 - x2)*(y0_3 + y0_2*(y1 + y2) + (y1 + y2)*(y1_2 + y2_2) + y0*(y1_2 + y1*y2 + y2_2))) / 60.;
		DmDx[Mat(5, i1 + 2, Nm)] += -((x0 - x2)*(y0_3 + y0_2*(y1 + y2) + (y1 + y2)*(y1_2 + y2_2) + y0*(y1_2 + y1*y2 + y2_2))) / 60.;
		DmDx[Mat(5, i2 + 2, Nm)] += ((x0 - x1)*(y0_3 + y0_2*(y1 + y2) + (y1 + y2)*(y1_2 + y2_2) + y0*(y1_2 + y1*y2 + y2_2))) / 60.;
		//dszz/d{xi,yi,zi}
		DmDx[Mat(6, i0 + 0, Nm)] += ((y1 - y2)*(z0_3 + z0_2*(z1 + z2) + (z1 + z2)*(z1_2 + z2_2) + z0*(z1_2 + z1*z2 + z2_2))) / 60.;
		DmDx[Mat(6, i1 + 0, Nm)] += -((y0 - y2)*(z0_3 + z0_2*(z1 + z2) + (z1 + z2)*(z1_2 + z2_2) + z0*(z1_2 + z1*z2 + z2_2))) / 60.;
		DmDx[Mat(6, i2 + 0, Nm)] += ((y0 - y1)*(z0_3 + z0_2*(z1 + z2) + (z1 + z2)*(z1_2 + z2_2) + z0*(z1_2 + z1*z2 + z2_2))) / 60.;
		DmDx[Mat(6, i0 + 1, Nm)] += -((x1 - x2)*(z0_3 + z0_2*(z1 + z2) + (z1 + z2)*(z1_2 + z2_2) + z0*(z1_2 + z1*z2 + z2_2))) / 60.;
		DmDx[Mat(6, i1 + 1, Nm)] += ((x0 - x2)*(z0_3 + z0_2*(z1 + z2) + (z1 + z2)*(z1_2 + z2_2) + z0*(z1_2 + z1*z2 + z2_2))) / 60.;
		DmDx[Mat(6, i2 + 1, Nm)] += -((x0 - x1)*(z0_3 + z0_2*(z1 + z2) + (z1 + z2)*(z1_2 + z2_2) + z0*(z1_2 + z1*z2 + z2_2))) / 60.;
		DmDx[Mat(6, i0 + 2, Nm)] += ((x2*(y0 - y1) + x0*(y1 - y2) + x1*(-y0 + y2))*(3 * z0_2 + z1_2 + z1*z2 + z2_2 + 2 * z0*(z1 + z2))) / 60.;
		DmDx[Mat(6, i1 + 2, Nm)] += ((x2*(y0 - y1) + x0*(y1 - y2) + x1*(-y0 + y2))*(z0_2 + 3 * z1_2 + 2 * z1*z2 + z2_2 + z0*(2 * z1 + z2))) / 60.;
		DmDx[Mat(6, i2 + 2, Nm)] += ((x2*(y0 - y1) + x0*(y1 - y2) + x1*(-y0 + y2))*(z0_2 + z0*z1 + z1_2 + 2 * (z0 + z1)*z2 + 3 * z2_2)) / 60.;
		//dsxy/d{xi,yi,zi}
		DmDx[Mat(7, i0 + 0, Nm)] += ((2 * x0*(3 * y0 + y1 + y2) + x1*(2 * (y0 + y1) + y2) + x2*(2 * y0 + y1 + 2 * y2))*(y2*(z0 - z1) + y0*(z1 - z2) + y1*(-z0 + z2))) / 120.;
		DmDx[Mat(7, i1 + 0, Nm)] += ((2 * x1*(y0 + 3 * y1 + y2) + x0*(2 * (y0 + y1) + y2) + x2*(y0 + 2 * (y1 + y2)))*(y2*(z0 - z1) + y0*(z1 - z2) + y1*(-z0 + z2))) / 120.;
		DmDx[Mat(7, i2 + 0, Nm)] += ((x0*(2 * y0 + y1 + 2 * y2) + 2 * x2*(y0 + y1 + 3 * y2) + x1*(y0 + 2 * (y1 + y2)))*(y2*(z0 - z1) + y0*(z1 - z2) + y1*(-z0 + z2))) / 120.;
		DmDx[Mat(7, i0 + 1, Nm)] += ((x0_2*(3 * y0 + y1 + y2) + x1_2*(y0 + 3 * y1 + y2) + x2_2*(y0 + y1 + 3 * y2) + x1*x2*(y0 + 2 * (y1 + y2)) + x0*(x1*(2 * (y0 + y1) + y2) + x2*(2 * y0 + y1 + 2 * y2)))*(z1 - z2) + (3 * x0_2 + x1_2 + x1*x2 + x2_2 + 2 * x0*(x1 + x2))*(y2*(z0 - z1) + y0*(z1 - z2) + y1*(-z0 + z2))) / 120.;
		DmDx[Mat(7, i1 + 1, Nm)] += ((x0_2*(3 * y0 + y1 + y2) + x1_2*(y0 + 3 * y1 + y2) + x2_2*(y0 + y1 + 3 * y2) + x1*x2*(y0 + 2 * (y1 + y2)) + x0*(x1*(2 * (y0 + y1) + y2) + x2*(2 * y0 + y1 + 2 * y2)))*(-z0 + z2) + (x0_2 + 3 * x1_2 + 2 * x1*x2 + x2_2 + x0*(2 * x1 + x2))*(y2*(z0 - z1) + y0*(z1 - z2) + y1*(-z0 + z2))) / 120.;
		DmDx[Mat(7, i2 + 1, Nm)] += ((x0_2*(3 * y0 + y1 + y2) + x1_2*(y0 + 3 * y1 + y2) + x2_2*(y0 + y1 + 3 * y2) + x1*x2*(y0 + 2 * (y1 + y2)) + x0*(x1*(2 * (y0 + y1) + y2) + x2*(2 * y0 + y1 + 2 * y2)))*(z0 - z1) + (x0_2 + x0*x1 + x1_2 + 2 * (x0 + x1)*x2 + 3 * x2_2)*(y2*(z0 - z1) + y0*(z1 - z2) + y1*(-z0 + z2))) / 120.;
		DmDx[Mat(7, i0 + 2, Nm)] += ((-y1 + y2)*(x0_2*(3 * y0 + y1 + y2) + x1_2*(y0 + 3 * y1 + y2) + x2_2*(y0 + y1 + 3 * y2) + x1*x2*(y0 + 2 * (y1 + y2)) + x0*(x1*(2 * (y0 + y1) + y2) + x2*(2 * y0 + y1 + 2 * y2)))) / 120.;
		DmDx[Mat(7, i1 + 2, Nm)] += ((y0 - y2)*(x0_2*(3 * y0 + y1 + y2) + x1_2*(y0 + 3 * y1 + y2) + x2_2*(y0 + y1 + 3 * y2) + x1*x2*(y0 + 2 * (y1 + y2)) + x0*(x1*(2 * (y0 + y1) + y2) + x2*(2 * y0 + y1 + 2 * y2)))) / 120.;
		DmDx[Mat(7, i2 + 2, Nm)] += ((-y0 + y1)*(x0_2*(3 * y0 + y1 + y2) + x1_2*(y0 + 3 * y1 + y2) + x2_2*(y0 + y1 + 3 * y2) + x1*x2*(y0 + 2 * (y1 + y2)) + x0*(x1*(2 * (y0 + y1) + y2) + x2*(2 * y0 + y1 + 2 * y2)))) / 120.;
		//dsyz/d{xi,yi,zi}
		DmDx[Mat(8, i0 + 0, Nm)] += ((-z1 + z2)*(y0_2*(3 * z0 + z1 + z2) + y1_2*(z0 + 3 * z1 + z2) + y2_2*(z0 + z1 + 3 * z2) + y1*y2*(z0 + 2 * (z1 + z2)) + y0*(y1*(2 * (z0 + z1) + z2) + y2*(2 * z0 + z1 + 2 * z2)))) / 120.;
		DmDx[Mat(8, i1 + 0, Nm)] += ((z0 - z2)*(y0_2*(3 * z0 + z1 + z2) + y1_2*(z0 + 3 * z1 + z2) + y2_2*(z0 + z1 + 3 * z2) + y1*y2*(z0 + 2 * (z1 + z2)) + y0*(y1*(2 * (z0 + z1) + z2) + y2*(2 * z0 + z1 + 2 * z2)))) / 120.;
		DmDx[Mat(8, i2 + 0, Nm)] += ((-z0 + z1)*(y0_2*(3 * z0 + z1 + z2) + y1_2*(z0 + 3 * z1 + z2) + y2_2*(z0 + z1 + 3 * z2) + y1*y2*(z0 + 2 * (z1 + z2)) + y0*(y1*(2 * (z0 + z1) + z2) + y2*(2 * z0 + z1 + 2 * z2)))) / 120.;
		DmDx[Mat(8, i0 + 1, Nm)] += ((x2*(-z0 + z1) + x1*(z0 - z2) + x0*(-z1 + z2))*(2 * y0*(3 * z0 + z1 + z2) + y1*(2 * (z0 + z1) + z2) + y2*(2 * z0 + z1 + 2 * z2))) / 120.;
		DmDx[Mat(8, i1 + 1, Nm)] += ((x2*(-z0 + z1) + x1*(z0 - z2) + x0*(-z1 + z2))*(2 * y1*(z0 + 3 * z1 + z2) + y0*(2 * (z0 + z1) + z2) + y2*(z0 + 2 * (z1 + z2)))) / 120.;
		DmDx[Mat(8, i2 + 1, Nm)] += ((x2*(-z0 + z1) + x1*(z0 - z2) + x0*(-z1 + z2))*(y0*(2 * z0 + z1 + 2 * z2) + 2 * y2*(z0 + z1 + 3 * z2) + y1*(z0 + 2 * (z1 + z2)))) / 120.;
		DmDx[Mat(8, i0 + 2, Nm)] += ((3 * y0_2 + y1_2 + y1*y2 + y2_2 + 2 * y0*(y1 + y2))*(x2*(-z0 + z1) + x1*(z0 - z2) + x0*(-z1 + z2)) + (x1 - x2)*(y0_2*(3 * z0 + z1 + z2) + y1_2*(z0 + 3 * z1 + z2) + y2_2*(z0 + z1 + 3 * z2) + y1*y2*(z0 + 2 * (z1 + z2)) + y0*(y1*(2 * (z0 + z1) + z2) + y2*(2 * z0 + z1 + 2 * z2)))) / 120.;
		DmDx[Mat(8, i1 + 2, Nm)] += ((y0_2 + 3 * y1_2 + 2 * y1*y2 + y2_2 + y0*(2 * y1 + y2))*(x2*(-z0 + z1) + x1*(z0 - z2) + x0*(-z1 + z2)) + (-x0 + x2)*(y0_2*(3 * z0 + z1 + z2) + y1_2*(z0 + 3 * z1 + z2) + y2_2*(z0 + z1 + 3 * z2) + y1*y2*(z0 + 2 * (z1 + z2)) + y0*(y1*(2 * (z0 + z1) + z2) + y2*(2 * z0 + z1 + 2 * z2)))) / 120.;
		DmDx[Mat(8, i2 + 2, Nm)] += ((y0_2 + y0*y1 + y1_2 + 2 * (y0 + y1)*y2 + 3 * y2_2)*(x2*(-z0 + z1) + x1*(z0 - z2) + x0*(-z1 + z2)) + (x0 - x1)*(y0_2*(3 * z0 + z1 + z2) + y1_2*(z0 + 3 * z1 + z2) + y2_2*(z0 + z1 + 3 * z2) + y1*y2*(z0 + 2 * (z1 + z2)) + y0*(y1*(2 * (z0 + z1) + z2) + y2*(2 * z0 + z1 + 2 * z2)))) / 120.;
		//dszx/d{xi,yi,zi}
		DmDx[Mat(9, i0 + 0, Nm)] += ((x2*(y0 - y1) + x0*(y1 - y2) + x1*(-y0 + y2))*(3 * z0_2 + z1_2 + z1*z2 + z2_2 + 2 * z0*(z1 + z2)) + (y1 - y2)*(x1*(z0_2 + 2 * z0*z1 + 3 * z1_2 + z0*z2 + 2 * z1*z2 + z2_2) + x2*(z0_2 + z0*z1 + z1_2 + 2 * (z0 + z1)*z2 + 3 * z2_2) + x0*(3 * z0_2 + z1_2 + z1*z2 + z2_2 + 2 * z0*(z1 + z2)))) / 120.;
		DmDx[Mat(9, i1 + 0, Nm)] += ((x2*(y0 - y1) + x0*(y1 - y2) + x1*(-y0 + y2))*(z0_2 + 3 * z1_2 + 2 * z1*z2 + z2_2 + z0*(2 * z1 + z2)) + (-y0 + y2)*(x1*(z0_2 + 2 * z0*z1 + 3 * z1_2 + z0*z2 + 2 * z1*z2 + z2_2) + x2*(z0_2 + z0*z1 + z1_2 + 2 * (z0 + z1)*z2 + 3 * z2_2) + x0*(3 * z0_2 + z1_2 + z1*z2 + z2_2 + 2 * z0*(z1 + z2)))) / 120.;
		DmDx[Mat(9, i2 + 0, Nm)] += ((x2*(y0 - y1) + x0*(y1 - y2) + x1*(-y0 + y2))*(z0_2 + z0*z1 + z1_2 + 2 * (z0 + z1)*z2 + 3 * z2_2) + (y0 - y1)*(x1*(z0_2 + 2 * z0*z1 + 3 * z1_2 + z0*z2 + 2 * z1*z2 + z2_2) + x2*(z0_2 + z0*z1 + z1_2 + 2 * (z0 + z1)*z2 + 3 * z2_2) + x0*(3 * z0_2 + z1_2 + z1*z2 + z2_2 + 2 * z0*(z1 + z2)))) / 120.;
		DmDx[Mat(9, i0 + 1, Nm)] += -((x1 - x2)*(x1*(z0_2 + 2 * z0*z1 + 3 * z1_2 + z0*z2 + 2 * z1*z2 + z2_2) + x2*(z0_2 + z0*z1 + z1_2 + 2 * (z0 + z1)*z2 + 3 * z2_2) + x0*(3 * z0_2 + z1_2 + z1*z2 + z2_2 + 2 * z0*(z1 + z2)))) / 120.;
		DmDx[Mat(9, i1 + 1, Nm)] += ((x0 - x2)*(x1*(z0_2 + 2 * z0*z1 + 3 * z1_2 + z0*z2 + 2 * z1*z2 + z2_2) + x2*(z0_2 + z0*z1 + z1_2 + 2 * (z0 + z1)*z2 + 3 * z2_2) + x0*(3 * z0_2 + z1_2 + z1*z2 + z2_2 + 2 * z0*(z1 + z2)))) / 120.;
		DmDx[Mat(9, i2 + 1, Nm)] += -((x0 - x1)*(x1*(z0_2 + 2 * z0*z1 + 3 * z1_2 + z0*z2 + 2 * z1*z2 + z2_2) + x2*(z0_2 + z0*z1 + z1_2 + 2 * (z0 + z1)*z2 + 3 * z2_2) + x0*(3 * z0_2 + z1_2 + z1*z2 + z2_2 + 2 * z0*(z1 + z2)))) / 120.;
		DmDx[Mat(9, i0 + 2, Nm)] += ((x2*(y0 - y1) + x0*(y1 - y2) + x1*(-y0 + y2))*(2 * x0*(3 * z0 + z1 + z2) + x1*(2 * (z0 + z1) + z2) + x2*(2 * z0 + z1 + 2 * z2))) / 120.;
		DmDx[Mat(9, i1 + 2, Nm)] += ((x2*(y0 - y1) + x0*(y1 - y2) + x1*(-y0 + y2))*(2 * x1*(z0 + 3 * z1 + z2) + x0*(2 * (z0 + z1) + z2) + x2*(z0 + 2 * (z1 + z2)))) / 120.;
		DmDx[Mat(9, i2 + 2, Nm)] += ((x2*(y0 - y1) + x0*(y1 - y2) + x1*(-y0 + y2))*(x0*(2 * z0 + z1 + 2 * z2) + 2 * x2*(z0 + z1 + 3 * z2) + x1*(z0 + 2 * (z1 + z2)))) / 120.;
		//ds-end		
	} // end for all triangles


	// derivate DmDx/D{x,y,z} for all points
	for (int i = 0; i < 3 * Np; i += 3) // for each vertex
	{
		// Ds/D{x,y,z}
		//double dsdx = DmDx[Mat(0, i + X_, Nm)];
		//double dsdy = DmDx[Mat(0, i + Y_, Nm)];
		//double dsdz = DmDx[Mat(0, i + Z_, Nm)];

		// Dsx/D{x,y,z}
		double dsxdx = Dsdx(s[SX_], s[S__], DmDx[Mat(SX_, i + X_, Nm)], DmDx[Mat(S__, i + X_, Nm)]);
		double dsxdy = Dsdx(s[SX_], s[S__], DmDx[Mat(SX_, i + Y_, Nm)], DmDx[Mat(S__, i + Y_, Nm)]);
		double dsxdz = Dsdx(s[SX_], s[S__], DmDx[Mat(SX_, i + Z_, Nm)], DmDx[Mat(S__, i + Z_, Nm)]);
		// Dsy/D{x,y,z}				    			  				  							 
		double dsydx = Dsdx(s[SY_], s[S__], DmDx[Mat(SY_, i + X_, Nm)], DmDx[Mat(S__, i + X_, Nm)]);
		double dsydy = Dsdx(s[SY_], s[S__], DmDx[Mat(SY_, i + Y_, Nm)], DmDx[Mat(S__, i + Y_, Nm)]);
		double dsydz = Dsdx(s[SY_], s[S__], DmDx[Mat(SY_, i + Z_, Nm)], DmDx[Mat(S__, i + Z_, Nm)]);
		// Dsz/D{x,y,z}					 			   											 
		double dszdx = Dsdx(s[SZ_], s[S__], DmDx[Mat(SZ_, i + X_, Nm)], DmDx[Mat(S__, i + X_, Nm)]);
		double dszdy = Dsdx(s[SZ_], s[S__], DmDx[Mat(SZ_, i + Y_, Nm)], DmDx[Mat(S__, i + Y_, Nm)]);
		double dszdz = Dsdx(s[SZ_], s[S__], DmDx[Mat(SZ_, i + Z_, Nm)], DmDx[Mat(S__, i + Z_, Nm)]);

		//inline double Dsdxx(
		//	double sy, double sz, double s,
		//	double dsyy, double dszz, double dsy, double dsz, double ds)
		// Dsxx/D{x,y,z}
		double dsxxdx = Dsdxx(s[SY_], s[SZ_], s[S__], DmDx[Mat(SYY, i + X_, Nm)], DmDx[Mat(SZZ, i + X_, Nm)], DmDx[Mat(SY_, i + X_, Nm)], DmDx[Mat(SZ_, i + X_, Nm)], DmDx[Mat(S__, i + X_, Nm)]);
		double dsxxdy = Dsdxx(s[SY_], s[SZ_], s[S__], DmDx[Mat(SYY, i + Y_, Nm)], DmDx[Mat(SZZ, i + Y_, Nm)], DmDx[Mat(SY_, i + Y_, Nm)], DmDx[Mat(SZ_, i + Y_, Nm)], DmDx[Mat(S__, i + Y_, Nm)]);
		double dsxxdz = Dsdxx(s[SY_], s[SZ_], s[S__], DmDx[Mat(SYY, i + Z_, Nm)], DmDx[Mat(SZZ, i + Z_, Nm)], DmDx[Mat(SY_, i + Z_, Nm)], DmDx[Mat(SZ_, i + Z_, Nm)], DmDx[Mat(S__, i + Z_, Nm)]);
		// Dsyy/D{x,y,z}	   					   
		double dsyydx = Dsdxx(s[SX_], s[SZ_], s[S__], DmDx[Mat(SXX, i + X_, Nm)], DmDx[Mat(SZZ, i + X_, Nm)], DmDx[Mat(SX_, i + X_, Nm)], DmDx[Mat(SZ_, i + X_, Nm)], DmDx[Mat(S__, i + X_, Nm)]);
		double dsyydy = Dsdxx(s[SX_], s[SZ_], s[S__], DmDx[Mat(SXX, i + Y_, Nm)], DmDx[Mat(SZZ, i + Y_, Nm)], DmDx[Mat(SX_, i + Y_, Nm)], DmDx[Mat(SZ_, i + Y_, Nm)], DmDx[Mat(S__, i + Y_, Nm)]);
		double dsyydz = Dsdxx(s[SX_], s[SZ_], s[S__], DmDx[Mat(SXX, i + Z_, Nm)], DmDx[Mat(SZZ, i + Z_, Nm)], DmDx[Mat(SX_, i + Z_, Nm)], DmDx[Mat(SZ_, i + Z_, Nm)], DmDx[Mat(S__, i + Z_, Nm)]);
		// Dszz/D{x,y,z}	   					   
		double dszzdx = Dsdxx(s[SX_], s[SY_], s[S__], DmDx[Mat(SXX, i + X_, Nm)], DmDx[Mat(SYY, i + X_, Nm)], DmDx[Mat(SX_, i + X_, Nm)], DmDx[Mat(SY_, i + X_, Nm)], DmDx[Mat(S__, i + X_, Nm)]);
		double dszzdy = Dsdxx(s[SX_], s[SY_], s[S__], DmDx[Mat(SXX, i + Y_, Nm)], DmDx[Mat(SYY, i + Y_, Nm)], DmDx[Mat(SX_, i + Y_, Nm)], DmDx[Mat(SY_, i + Y_, Nm)], DmDx[Mat(S__, i + Y_, Nm)]);
		double dszzdz = Dsdxx(s[SX_], s[SY_], s[S__], DmDx[Mat(SXX, i + Z_, Nm)], DmDx[Mat(SYY, i + Z_, Nm)], DmDx[Mat(SX_, i + Z_, Nm)], DmDx[Mat(SY_, i + Z_, Nm)], DmDx[Mat(S__, i + Z_, Nm)]);

		//inline double Dsdxy(
		//	double sx, double sy, double s,
		//	double dsxy, double dsx, double dsy, double ds)
		// Dsxy/D{x,y,z}
		double dsxydx = Dsdxy(s[SX_], s[SY_], s[S__], DmDx[Mat(SXY, i + X_, Nm)], DmDx[Mat(SX_, i + X_, Nm)], DmDx[Mat(SY_, i + X_, Nm)], DmDx[Mat(S__, i + X_, Nm)]);
		double dsxydy = Dsdxy(s[SX_], s[SY_], s[S__], DmDx[Mat(SXY, i + Y_, Nm)], DmDx[Mat(SX_, i + Y_, Nm)], DmDx[Mat(SY_, i + Y_, Nm)], DmDx[Mat(S__, i + Y_, Nm)]);
		double dsxydz = Dsdxy(s[SX_], s[SY_], s[S__], DmDx[Mat(SXY, i + Z_, Nm)], DmDx[Mat(SX_, i + Z_, Nm)], DmDx[Mat(SY_, i + Z_, Nm)], DmDx[Mat(S__, i + Z_, Nm)]);
		// Dsyz/D{x,y,z}
		double dsyzdx = Dsdxy(s[SY_], s[SZ_], s[S__], DmDx[Mat(SYZ, i + X_, Nm)], DmDx[Mat(SY_, i + X_, Nm)], DmDx[Mat(SZ_, i + X_, Nm)], DmDx[Mat(S__, i + X_, Nm)]);
		double dsyzdy = Dsdxy(s[SY_], s[SZ_], s[S__], DmDx[Mat(SYZ, i + Y_, Nm)], DmDx[Mat(SY_, i + Y_, Nm)], DmDx[Mat(SZ_, i + Y_, Nm)], DmDx[Mat(S__, i + Y_, Nm)]);
		double dsyzdz = Dsdxy(s[SY_], s[SZ_], s[S__], DmDx[Mat(SYZ, i + Z_, Nm)], DmDx[Mat(SY_, i + Z_, Nm)], DmDx[Mat(SZ_, i + Z_, Nm)], DmDx[Mat(S__, i + Z_, Nm)]);
		// Dszx/D{x,y,z}
		double dszxdx = Dsdxy(s[SZ_], s[SX_], s[S__], DmDx[Mat(SZX, i + X_, Nm)], DmDx[Mat(SZ_, i + X_, Nm)], DmDx[Mat(SX_, i + X_, Nm)], DmDx[Mat(S__, i + X_, Nm)]);
		double dszxdy = Dsdxy(s[SZ_], s[SX_], s[S__], DmDx[Mat(SZX, i + Y_, Nm)], DmDx[Mat(SZ_, i + Y_, Nm)], DmDx[Mat(SX_, i + Y_, Nm)], DmDx[Mat(S__, i + Y_, Nm)]);
		double dszxdz = Dsdxy(s[SZ_], s[SX_], s[S__], DmDx[Mat(SZX, i + Z_, Nm)], DmDx[Mat(SZ_, i + Z_, Nm)], DmDx[Mat(SX_, i + Z_, Nm)], DmDx[Mat(S__, i + Z_, Nm)]);

		DmDx[Mat(SX_, i + X_, Nm)] = dsxdx;
		DmDx[Mat(SX_, i + Y_, Nm)] = dsxdy;
		DmDx[Mat(SX_, i + Z_, Nm)] = dsxdz;

		DmDx[Mat(SY_, i + X_, Nm)] = dsydx;
		DmDx[Mat(SY_, i + Y_, Nm)] = dsydy;
		DmDx[Mat(SY_, i + Z_, Nm)] = dsydz;

		DmDx[Mat(SZ_, i + X_, Nm)] = dszdx;
		DmDx[Mat(SZ_, i + Y_, Nm)] = dszdy;
		DmDx[Mat(SZ_, i + Z_, Nm)] = dszdz;

		DmDx[Mat(SXX, i + X_, Nm)] = dsxxdx;
		DmDx[Mat(SXX, i + Y_, Nm)] = dsxxdy;
		DmDx[Mat(SXX, i + Z_, Nm)] = dsxxdz;

		DmDx[Mat(SYY, i + X_, Nm)] = dsyydx;
		DmDx[Mat(SYY, i + Y_, Nm)] = dsyydy;
		DmDx[Mat(SYY, i + Z_, Nm)] = dsyydz;

		DmDx[Mat(SZZ, i + X_, Nm)] = dszzdx;
		DmDx[Mat(SZZ, i + Y_, Nm)] = dszzdy;
		DmDx[Mat(SZZ, i + Z_, Nm)] = dszzdz;

		DmDx[Mat(SXY, i + X_, Nm)] = dsxydx;
		DmDx[Mat(SXY, i + Y_, Nm)] = dsxydy;
		DmDx[Mat(SXY, i + Z_, Nm)] = dsxydz;

		DmDx[Mat(SYZ, i + X_, Nm)] = dsyzdx;
		DmDx[Mat(SYZ, i + Y_, Nm)] = dsyzdy;
		DmDx[Mat(SYZ, i + Z_, Nm)] = dsyzdz;

		DmDx[Mat(SZX, i + X_, Nm)] = dszxdx;
		DmDx[Mat(SZX, i + Y_, Nm)] = dszxdy;
		DmDx[Mat(SZX, i + Z_, Nm)] = dszxdz;
	} //end for each vertex


	// for each vertex
	// compute the derivatives Dm/Dd = Dm/Dx * Dx/Dd
	for (int i = 0; i < Np; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			int ii = 3 * i;

			DmDd[Mat(j, i, Nm)] = 0.0;
			DmDd[Mat(j, i, Nm)] += DmDx[Mat(j, ii + X_, Nm)] * N0[Mat(i, X_, Np)]; //x
			DmDd[Mat(j, i, Nm)] += DmDx[Mat(j, ii + Y_, Nm)] * N0[Mat(i, Y_, Np)]; //y
			DmDd[Mat(j, i, Nm)] += DmDx[Mat(j, ii + Z_, Nm)] * N0[Mat(i, Z_, Np)]; //z						
		}
	}

	// mass
	M[0] = mass = s[0];

	// center of mass
	M[1] = cmx = s[1] / s[0];
	M[2] = cmy = s[2] / s[0];
	M[3] = cmz = s[3] / s[0];

	if (bodyCoords)
	{
		// inertia relative to center of mass
		M[4] = s[5] + s[6] - mass*(cmy*cmy + cmz*cmz);
		M[5] = s[4] + s[6] - mass*(cmz*cmz + cmx*cmx);
		M[6] = s[4] + s[5] - mass*(cmx*cmx + cmy*cmy);
		M[7] = -(s[7] - mass*cmx*cmy);
		M[8] = -(s[8] - mass*cmy*cmz);
		M[9] = -(s[9] - mass*cmz*cmx);
	}
	else
	{
		// inertia relative to world origin
		M[4] = s[5] + s[6]; // Ixx
		M[5] = s[4] + s[6]; // Iyy
		M[6] = s[4] + s[5]; // Izz

		M[7] = -s[7]; // Ixy
		M[8] = -s[8]; // Iyz
		M[9] = -s[9]; // Izx
	}
}