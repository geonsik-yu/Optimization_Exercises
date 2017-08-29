# include <cmath>
# include "Jacobi.h"
using namespace std;
//////////Reference #1 : JACOBI_EIGENVALUE _ Eigenvalues and Eigenvectors of a Symmetric Matrix
//////////				from <the GNU LGPL>
//////////Reference #2 : en.wikipedia.org/wiki/Jacobi_eigenvalue_algorithm
////////// �����ߴµ�, ��ġ ������� ���� ������ ���� �Ϻκ� ���ذ� ���ڶ� �κ��� �ֽ��ϴ�;;

void jacobi_eigenvalue(int matSize, int** A, double** eigVec, double* eigVal){
	//// double type�� ��ķ� ��ȯ�Ͽ� �����ϱ� ���� �ӽ� ��� a(���� A�� int ���)
	double** a = new double*[matSize];
	for(int i = 0; i < matSize; i++)
		a[i] = new double[matSize];
	for(int i = 0; i < matSize; i++)
		for(int j = 0; j < matSize; j++)
			a[i][j] = (double)A[i][j];
	//// End of ��ȯ

	double c;	double g;	double h;
	double s;	double t;
	double tau;	double term;
	double termp;	double termq;
	double theta;

	double gapq;
	identityFunc(matSize, eigVec); //// ���̰պ��͵��� �����ϴ� ��������� ������ķ� ä��
	getDiag(matSize, a, eigVal);   //// ���̰պ������� �����ϴ� ��������� ���a�� �밢���ҷ� ä��
	
	double* bw = new double[matSize];
	double* zw = new double[matSize];
	
	for(int i = 0; i < matSize; i++){
		bw[i] = eigVal[i];		//�ӽ� ������� bw�� ���̰պ����� ����
		zw[i] = 0.0;			//�ӽ� ������� zw�� 0.0���� ä��
	}
	
	int it_num = 0; // ���ͷ��̼� ��
	while( it_num < 100 ){	
		// ��ǥ�ϴ� ��Ȯ���� �Ҽ��� 2��° �ڸ��� �ſ� �����Ƿ�,
		// 100ȸ ������ ���ͷ��̼����� �����Ͽ��� ��ǥ�ϴ� ���� �ٻ��ϴµ� ������ ���� ������ �����ߴ�.
		it_num = it_num + 1;
	    double thresh = 0.0;
		for(int i = 0; i < matSize; i++){
			for(int j = 0; j < i; j++){	//lower triangle �κ��� Ž���ϸ鼭, thresh�� ������ �������� ����.
				thresh = thresh + a[i][j] * a[i][j];
			}
		}
		thresh = sqrt(thresh)/(double)(4*matSize);
		if( thresh == 0.0 )//�ݺ� �ߴ� ����.
			break;

		for(int p = 0; p < matSize; p++){
			for(int q = p + 1; q < matSize; q++){
				gapq = 10.0 * fabs(a[q][p]);
				termp = gapq + fabs(eigVal[p]);
				termq = gapq + fabs(eigVal[q]);
		
				if (4<it_num && termp== fabs(eigVal[p]) && termq == fabs(eigVal[q]))
					a[q][p] = 0.0;
				else if(thresh <= fabs(a[q][p])){
					h = eigVal[q] - eigVal[p];
					term = fabs(h) + gapq;

					if(term == fabs(h))
						t = a[q][p]/h;
					else{
						theta = 0.5 * h / a[q][p];
						t = 1.0/(fabs(theta)+sqrt(1.0+theta*theta));
						if (theta < 0.0)
							t = - t;
					}
					c = 1.0/sqrt(1.0 + t*t);
					s = t * c;
					tau = s / ( 1.0 + c );
					h = t * a[q][p];

					zw[p] = zw[p] - h;                 
					zw[q] = zw[q] + h;
					eigVal[p] = eigVal[p] - h;
					eigVal[q] = eigVal[q] + h;
					a[q][p] = 0.0;

					for(int j = 0; j < p; j++){
						g = a[p][j];
						h = a[q][j];
						a[p][j] = g - s * ( h + g * tau );
						a[q][j] = h + s * ( g - h * tau );
					}

					for(int j = p+1; j < q; j++){
						g = a[j][p];
						h = a[q][j];
						a[j][p] = g - s * ( h + g * tau );
						a[q][j] = h + s * ( g - h * tau );
					}

					for(int j = q+1; j < matSize; j++){
						g = a[j][p];
						h = a[j][q];
						a[j][p] = g - s * ( h + g * tau );
						a[j][q] = h + s * ( g - h * tau );
					}

					for(int j = 0; j < matSize; j++){
						g = eigVec[p][j];
						h = eigVec[q][j];
						eigVec[p][j] = g - s * ( h + g * tau );
						eigVec[q][j] = h + s * ( g - h * tau );
					}
				}
			}
		}
		
		///bw������Ʈ, ���̰պ��� ������Ʈ, zw�ʱ�ȭ.
		for(int i = 0; i < matSize; i++){
			bw[i] = bw[i] + zw[i];
			eigVal[i] = bw[i];
			zw[i] = 0.0;
		}
	}

	//��ﰢ ��� ����.
	for(int j = 0; j < matSize; j++){
		for(int i = 0; i < j; i++){
			a[j][i] = a[i][j];
		}
	}
	
	///�̻� �Լ����� ����ߴ� dynamic array ����
	delete[] bw;
	delete[] zw;

	for(int i = 0; i < matSize; i++)
		delete[] a[i];
	delete[] a;
	return;
};


//// ��Ʈ���� ����� Ȯ���ϰ� ��� a�� �밢���Ҹ� ���� v�� ����.
void getDiag(int matSize, double** a, double* v){
	for(int i = 0; i < matSize; i++)
		v[i] = a[i][i];
  return;
};

//// ��Ʈ���� ����� Ȯ���ϰ� ��� a�� ������ķ� �����.
void identityFunc(int matSize, double** a)
{
	for(int i = 0; i < matSize; i++)
		for(int j = 0; j < matSize; j++)
			if( i == j ) a[i][j] = 1;
			else		 a[i][j] = 0;
  return;
};

//// ����� Norm�� ����Ѵ�.
double norm ( int matSize, int** a ){
	double value = 0.0;
	for (int i = 0; i < matSize; i++ ){
		for (int j = 0; j < matSize; j++ ){
			value = value + a[i][j]*a[i][j];
		}
	}
	value = sqrt( value );
	return value;
};