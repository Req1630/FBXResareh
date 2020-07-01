#include "FbxInclude.h"

DirectX::XMMATRIX FbxMatrixConvertDXMMatrix( const FbxMatrix& matrix )
{
	const double* p = matrix;
	return 
	{	
		(float)p[0],	(float)p[1],	(float)p[2],	(float)p[3],
		(float)p[4],	(float)p[5],	(float)p[6],	(float)p[7],
		(float)p[8],	(float)p[9],	(float)p[10],	(float)p[11],
		(float)p[12],	(float)p[13],	(float)p[14],	(float)p[15] 
	};
}