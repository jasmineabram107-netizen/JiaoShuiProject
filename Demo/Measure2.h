#pragma once
#include <math.h>
#include<algorithm>

typedef int integer;
typedef double doublereal;

extern "C" void dgesv_(const int* N, const int* nrhs, double* A, const int* lda, int
	* ipiv, double* b, const int* ldb, int* info);

extern "C" int dggev_(char* jobvl, char* jobvr, integer* n, doublereal*
	a, integer* lda, doublereal* b, integer* ldb, doublereal* alphar,
	doublereal* alphai, doublereal* beta, doublereal* vl, integer* ldvl,
	doublereal* vr, integer* ldvr, doublereal* work, integer* lwork,
	integer* info);

struct dPoint {
	double x;
	double y;
};

#define  Z  6//¾ØÕó½×Êı
class CMeasure2
{
public:
	CMeasure2(void);
public:
	~CMeasure2(void);
public:
	typedef struct ES
	{
		int index_Left;
		int index_Right;
		double x_left;
		double y_left;
		double x_right;
		double y_right;
		double kL;
		double kR;
		double left_theta;
		double right_theta;
		double ctheta;//contact angle
		//////ÍÖÔ²²ÎÊı//
		double a;
		double b;
		double x0;
		double y0;
		double theta;
		bool success;
	};
	ES ellipse;
public:
	double **B;
    double **BT;
	double **C;
	double **S;
public:
	////***********************ÍÖÔ²ÄâºÏ*********************/
	float MeasureEllipse(dPoint* pt,int N);
	void FindMaxSec(int* index_left, int* index_right, dPoint* pt,int N);//²éÕÒ±ßÔµ×óÓÒµã
	void  LMEllipse(int index_Left, int index_Right, dPoint* pt, int N);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	void Transpose(double** a, int row, int col, double**temp);
	void Multiply(double ** a,int a_row, int c, double**b, int b_col, double**temp);
	void Add(double ** a,int row, int col, double**b,double**temp);
	void Sub(double** a,int row, int col, double**b,double**temp);
	void MultiNo(double** a,int row, int col, double No,double**temp);
	void Inv(double **A, int N);
public:
	void  Memory(int N);
	void  UNMemory(int N);
};
