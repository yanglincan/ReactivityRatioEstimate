#include "ParaEst.h"
#ifdef _DEBUG  
#define New   new(_NORMAL_BLOCK, __FILE__, __LINE__)  
#endif  

#define CRTDBG_MAP_ALLOC    
#include <stdlib.h>    
#include <crtdbg.h>  

using namespace std;

int main()
{
	{
		ParaEst temp(GetSimuFile());
		//ParaEst temp("C:\\Users\\YangLincan\\Desktop\\123.txt");
		temp.CalcOn_M_L();
		temp.CalcOn_MCE();
	}
	_CrtDumpMemoryLeaks();
	system("pause");
	return 0;
}