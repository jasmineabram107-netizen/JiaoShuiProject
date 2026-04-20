#include "pch.h"
#include "Measure2.h"

using namespace std;

CMeasure2::CMeasure2(void)
{
	memset(&ellipse,0,sizeof(ellipse));
	B=NULL;
	BT=NULL;
	C=NULL;
	S=NULL;
}

CMeasure2::~CMeasure2(void)
{
}
//��Բ����
float CMeasure2::MeasureEllipse(dPoint* pt,int N)
{
	Memory(N);

	int index_Left=0,index_Right=0;
	FindMaxSec(& index_Left, &index_Right,pt,N); 
	ellipse.index_Left=index_Left;
	ellipse.index_Right=index_Right;

	LMEllipse(index_Left, index_Right, pt, N);

    UNMemory(N);
	return ellipse.ctheta;
}

/************************************************************/
void CMeasure2:: LMEllipse(int index_Left, int index_Right, dPoint* pt, int N)
{
    int i,j;
	for (i=0;i<N;i++)
	{
		B[i][0]=pt[i].x*pt[i].x;
		B[i][1]=pt[i].x*pt[i].y;
		B[i][2]=pt[i].y*pt[i].y;
		B[i][3]=pt[i].x;
		B[i][4]=pt[i].y;
		B[i][5]=1;
	} 
	/************************************/
	for (i=0;i<6;i++)
	{
		for (j=0;j<6;j++)
		{
			C[i][j]=0;
		}
	}
	C[0][2]=2;	C[1][1]=-1;	C[2][0]=2;
	/************************************/
    Transpose(B,N,6,BT);
	Multiply(BT,6,N, B, 6, S);
   /**********LPACK���*******************/
	char jobvl,jobvr;
	jobvl='V';
	jobvr='V';
	integer n=Z;
	integer lda=Z;
    integer ldb=Z;
	double A[Z*Z];
	double B[Z*Z];
	double alphar[Z]; 
	double alphai[Z];  
	double beta[Z];
	double wk[500];
	integer ldvl=Z;
	integer ldvr=Z;
	double vl[Z*Z];
	double vr[Z*Z];
	integer lk=100;
	integer info;

	///////////////////
	int KK=0;
	for(i=0;i<6;i++)
	{
		for(j=0;j<6;j++)
		{
		   A[KK]=S[i][j];
		   B[KK]=C[i][j];
		   KK++;
		}
	}
/////////////////////////////
    dggev_(&jobvl,&jobvr,&n,A,&lda,B,&ldb,alphar,alphai,beta,  vl,&ldvl,vr,&ldvr,wk,&lk,&info);
    
	int ind=0; //������ֵ����
	for (i=0;i<6;i++)
	{
		if (beta[i]!=0&&(alphar[i]/beta[i])>0)
		{
            break;
		}
		ind++;
	}
    //��������
    double vec[6];
	int tt=0;
	for(i=0;i<6;i++)
	{

		vec[i]=vr[i+ind*Z];
	}
   ///��Kֵ
	double k=sqrt(1.0/(4*vec[0]*vec[2]-vec[1]*vec[1]));
	for(i=0;i<6;i++)
	{
        vec[i]=k*vec[i];
	}
	///////vec��ʱΪ��Բ������////////////////
ellipse.theta=0.5*atan(vec[1]/(vec[0]-vec[2]));
double M=vec[1]*vec[1]-4*vec[0]*vec[2];
ellipse.x0=(2*vec[2]*vec[3]-vec[1]*vec[4])/M;
ellipse.y0=(2*vec[0]*vec[4]-vec[1]*vec[3])/M;
double temp1=(vec[0]*ellipse.x0*ellipse.x0+vec[2]*ellipse.y0*ellipse.y0+vec[1]*ellipse.x0*ellipse.y0-vec[5]);
double temp2=vec[0]+vec[2];
double temp3=sqrt((vec[0]-vec[2])*(vec[0]-vec[2])+vec[1]*vec[1]);
ellipse.a=sqrt(2*temp1/(temp2+temp3));
ellipse.b=sqrt(2*temp1/(temp2-temp3));
ellipse.success = (ind < 6);
//////////////������б��//������Բ����/////
double tmpk=(pt[index_Left].y-pt[index_Right].y)*1.0/(pt[index_Left].x-pt[index_Right].x);
double tm=pt[index_Left].y-tmpk*pt[index_Left].x;
double AA=vec[0]+vec[1]*tmpk+vec[2]*tmpk*tmpk;
double BB=vec[1]*tm+2*tmpk*tm*vec[2]+vec[3]+vec[4]*tmpk;
double CC=vec[2]*tm*tm+vec[4]*tm+vec[5];
double temp=sqrt(BB*BB-4*AA*CC);
double mm1=0.0,mm2=0.0;
mm1=((-1.0)*BB+temp)/(2*AA);
mm2=((-1.0)*BB-temp)/(2*AA);
if (mm1>mm2)
{
   ellipse.x_left=mm2;
   ellipse.x_right=mm1;
}
else
{
	ellipse.x_left=mm1;
	ellipse.x_right=mm2;
}
ellipse.y_left=tmpk*ellipse.x_left+tm;
ellipse.y_right=tmpk*ellipse.x_right+tm;
////////////����Բ����//////////////////////
double m1=2*vec[2]*ellipse.y_left+vec[4]+vec[1]*ellipse.x_left;
double m2=2*vec[0]*ellipse.x_left+vec[3]+vec[1]*ellipse.y_left;

double m3=2*vec[2]*ellipse.y_right+vec[4]+vec[1]*ellipse.x_right;
double m4=2*vec[0]*ellipse.x_right+vec[3]+vec[1]*ellipse.y_right;

double KL=(-1.0)*m2/m1;
double KR=(-1.0)*m4/m3;
ellipse.kL=KL;
ellipse.kR=KR;

 double t1=0.0,t2=0.0;
 if (KL<0)
 {
	 t1=-atan(KL)*180/3.1415;
 }
 if (KL>=0)
 {
	 t1=180-atan(KL)*180/3.1415;
 }
 if (KR<0)
 {
	 t2=180+atan(KR)*180/3.1415;
 }
 if (KR>=0)
 {
	 t2=atan(KR)*180/3.1415;
 }
 double v=180*atan(tmpk)/3.1415;
 t1=t1+v;
 t2=t2-v;
 ellipse.left_theta=t1;
 ellipse.right_theta=t2;
 ellipse.ctheta=(t1+t2)/2;
}
/************************************************************/
void CMeasure2::FindMaxSec(int* index_left, int* index_right, dPoint* pt,int N)
{ 
	int first=0,sec=0,t=0;
	if (pt[0].y>pt[1].y)
	{
		first=0;sec=1;
	}
	else
	{
		first=1;sec=0;
	}

	for (int i=2;i<N;i++)
	{
		if (pt[i].y>pt[sec].y)
		{
			sec=i;
			if(pt[sec].y>pt[first].y)
			{
				t= first; first= sec; sec= t;
			}
		}
	}
	if (pt[first].x<pt[sec].x)
	{
		*index_left=first;
		*index_right=sec;
	}
	else
	{
		*index_left=sec;
		*index_right=first;
	}
}

/////����ת��
void CMeasure2::Transpose(double ** a, int row, int col, double **temp)
{
	for (int i=0;i<col;i++)
	{
		for (int j=0;j<row;j++)
		{
			temp[i][j]=a[j][i];
		}
	}
}
/////����˷�
void CMeasure2::Multiply(double ** a,int a_row, int c, double **b, int b_col,double **temp)
{
	float sum;
	int i,j,k;
	for (i=0;i<a_row;i++)
	{
		for ( j=0;j<b_col;j++)
		{
			sum=0;
			for ( k=0;k<c;k++)
			{
				sum=sum+a[i][k]*b[k][j];
			}
			temp[i][j]=sum;
		}
	}
}
/////����ӷ�
void CMeasure2::Add(double ** a,int row, int col, double **b,double **temp)
{
	for (int i=0;i<row;i++)
	{
		for (int j=0;j<col;j++)
		{
			temp[i][j]=a[i][j]+b[i][j];
		}
	}			
}
/////�������
void CMeasure2::Sub(double ** a,int row, int col, double **b,double **temp)
{   
	for (int i=0;i<row;i++)
	{
		for (int j=0;j<col;j++)
		{
			temp[i][j]=a[i][j]-b[i][j];
		}
	}	
}
//�������
void CMeasure2::MultiNo(double ** a,int row, int col, double No,double **temp)
{
	for (int i=0;i<row;i++)
	{
		for (int j=0;j<col;j++)
		{
			temp[i][j]=a[i][j]*No;
		}
	}	
}
//��������
void CMeasure2::Inv(double  ** A, int N)
{
	int i,j,k;
	double d;  
	int *JS = new int[N];  
	int *IS = new int[N];  
	for (k=0;k<N;k++)  
	{  
		d=0;  
		for (i=k;i<N;i++)  
		{
			for (j=k;j<N;j++)  
			{  
				if (fabs(A[i][j])>d)  
				{  
					d=fabs(A[i][j]);  
					IS[k]=i;  
					JS[k]=j;  
				}  
			}    
		}
		if (d+1.0==1.0) return ;  
		if (IS[k]!=k)  
		{
			for (j=0;j<N;j++)  
			{
				swap(A[k][j],A[IS[k]][j]);
			}
		}
		if (JS[k]!=k)  
		{
			for (i=0;i<N;i++)  
			{
				swap(A[i][k],A[i][JS[k]]);  
			}
		}                           
		A[k][k]=1/A[k][k];  
		for (j=0;j<N;j++)  
		{
			if (j!=k) 
			{
				A[k][j]=A[k][j]*A[k][k];  
			}
		}           
		for (i=0;i<N;i++)  
		{
			if (i!=k)  
			{
				for (j=0;j<N;j++)  
				{
					if (j!=k)
					{
						A[i][j]=A[i][j]-A[i][k]*A[k][j];  
					}
				}
			}
		}
		for (i=0;i<N;i++)  
		{
			if (i!=k) 
			{
				A[i][k]=-A[i][k]*A[k][k];  
			}
		}
	}  
	for (k=N-1;k>=0;k--)  
	{  
		for (j=0;j<N;j++)  
		{ 
			if (JS[k]!=k) 
			{
				swap(A[k][j],A[JS[k]][j]);  
			}
		}	          
		for (i=0;i<N;i++)  
		{
			if (IS[k]!=k) 
			{
				swap(A[i][k],A[i][IS[k]]);  
			}
		}	          
	}  

	delete []JS;JS=NULL;
	delete []IS;IS=NULL;
} 
/************************************************************/
//���仺��
void CMeasure2::Memory(int N)//NΪȡ����
{ 
	int i;
	B= new double *[N]; 
	for( i= 0; i <N; i++)
	{  
		B[i] = new double[6];  
	} 
    /////////////////
	BT= new double *[6]; 
	for( i= 0; i <6; i++)
	{  
		BT[i] = new double[N];  
	} 
   //////////////
	 C= new double *[6]; 
	for( i= 0; i <6; i++)
	{  
		C[i] = new double[6];  
	} 
    ////////////////
	S= new double *[6]; 
	for( i= 0; i <6; i++)
	{  
		S[i] = new double[6];  
	} 
}
//�ͷŻ���
void CMeasure2::UNMemory(int N)
{
	int i,j;
	for (i=0;i<N;i++)
	{
		delete[] B[i];
		B[i]=NULL;
	}
	delete []B;
	B=NULL;////////////////////�ͷ�B����
	for (i=0;i<6;i++)
	{
		delete[] BT[i];
		BT[i]=NULL;
	}
	delete []BT;
	BT=NULL;////////////////////�ͷ�BT����
	for (i=0;i<6;i++)
	{
		delete[] C[i];	
		C[i]=NULL;
		delete[] S[i];
		S[i]=NULL;
	}
	delete []C;C=NULL;
	delete []S;S=NULL;////////////////////�ͷ�C,S����
}