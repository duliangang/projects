#include <string>
#include <vector>
#include <boost/shared_array.hpp>
struct MatrixStruct
{
   boost::shared_array<int> pMatrix;
   int  row;
   int  column;
};
class MyMatrix
{
public:
	MyMatrix(int* array,int row,int column);
	MyMatrix(const MatrixStruct& matrix)
	{
		m_matrix.pMatrix.reset(new int[matrix.row][matrix.column]);

	}
	MyMatrix& operator+(const MyMatrix& other)const;
	MyMatrix& operator*(const MatrixStruct& matrix)const;
private:
	MatrixStruct m_matrix;
};