#include "tool.h"

SplitInfo SplitRect(const ddRange x_range, const ddRange y_range, const int desiredBlocks)
{
	if (x_range.second <= x_range.first || y_range.second <= y_range.first)
		return SplitInfo();

	SplitInfo info;
	SplitInfo info_out;

	info._x_stepLength = x_range.second - x_range.first;
	info._y_stepLength = y_range.second - y_range.first;

	SPLIT_MODE mode = (info._x_stepLength >= info._y_stepLength) ? X_MODE : Y_MODE;
	
	while (info._blocks < desiredBlocks) {
		if (X_MODE == mode) 
			info._x_stepLength /= 2;
		else if (Y_MODE == mode) 
			info._y_stepLength /= 2;

		int x_blocks = -1;
		int y_blocks = -1;
		for (double x_walk = x_range.first; x_walk <= x_range.second; x_walk += info._x_stepLength)
			++x_blocks;
		for (double y_walk = y_range.first; y_walk <= y_range.second; y_walk += info._y_stepLength)
			++y_blocks;
		info._blocks = x_blocks * y_blocks;
		info._x_blocks = x_blocks;
		info._y_blocks = y_blocks;

		if (info._blocks <= desiredBlocks)
			info_out = info;

		if (X_MODE == mode)
			mode = Y_MODE;
		else if (Y_MODE == mode)
			mode = X_MODE;
	}
	return info_out;
}

int GetThreadNumber()
{
	SYSTEM_INFO si;
	memset(&si, 0, sizeof(SYSTEM_INFO));
	GetSystemInfo(&si);
	DWORD dwNum = si.dwNumberOfProcessors;
	return (int)dwNum;
}

const string GetSimuFile()
{
	cout << endl << "-> Place the simulation file into the foldor that stores this program. <-" << endl << endl;
	GetFiles filename;
	const vector<string>& v = filename.getFiles(filename.GetWorkingDirectory());
	for_each(v.begin(), v.end(), [](string filename) {static int i = 1; cout << "#" << i++ << ": " << filename << endl;	});
	cout << endl << "-> Input and select [e.g. ""1 + enter""] <-" << endl << endl;
	size_t select = 1;
	cin >> select;
	return v.at(select - 1);
}

const vector<string>& GetFiles::getFiles(const string& path)
{
	//文件句柄  
	intptr_t hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1){
		do{
			//如果是目录,迭代之  
			//如果不是,加入列表  
			if ((fileinfo.attrib & _A_SUBDIR)){
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name));
			}
			else
				m_FilesVector.push_back(p.assign(path).append("\\").append(fileinfo.name));
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
	return m_FilesVector;
}

#include <direct.h>
string GetFiles::GetWorkingDirectory()
{
	char buffer[80];
	_getcwd(buffer, 80);
	return string(buffer);
}

ScanFile::ScanFile() :
	pfA_Vec(NULL), pFA_Vec(NULL), 
	minStepSizeRA(0), minStepSizeRB(0)
{
	pfA_Vec = new vector<double>;
	pFA_Vec = new vector<double>;
	rA_range = make_pair(0, 0);
	rB_range = make_pair(0, 0);
}

ScanFile::~ScanFile()
{
	if (pfA_Vec)
		delete pfA_Vec;
	if (pFA_Vec)
		delete pFA_Vec;
	pfA_Vec = NULL;
	pFA_Vec = NULL;
}

void ScanFile::ScanFileSection(const string& strBegin, const string& strEnd, void(*Analysis)(const string& strLine, ScanFile* pCurrent))
{
	char buffer[256] = { '\0' };
	m_SimuFile.seekg(0, std::ios::beg);
	while (!m_SimuFile.eof()){
		m_SimuFile.getline(buffer, 100);
		string strLine(buffer);
		if (string::npos != strLine.find(strBegin)){
			while (!m_SimuFile.eof()){
				m_SimuFile.getline(buffer, 100);
				strLine = buffer;
				if (string::npos != strLine.find(strEnd))
					return;
				Analysis(strLine, this);
			}
		}
	}
}

void ScanFile::SplitString(const string& s, vector<string>& v, const string& c)
{
	string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (string::npos != pos2){
		v.push_back(s.substr(pos1, pos2 - pos1));
		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}

void ScanfAFA(const string& strLine, ScanFile* pCurrent)
{
#define fF_VECTOR_SIZE 2
#define LOWER_BOUNDARY 0
#define UPPER_BOUNDARY 1
	if (strLine.empty())
		return;
	vector<string> vectorStr;
	pCurrent->SplitString(strLine, vectorStr, "\t");
	if (vectorStr.size() < fF_VECTOR_SIZE) {
		vectorStr.clear();
		return;
	}
	double f_value = atof(vectorStr[0].data());
	double F_value = atof(vectorStr[1].data());
	if (f_value <= LOWER_BOUNDARY || F_value <= LOWER_BOUNDARY ||
		f_value >= UPPER_BOUNDARY || F_value >= UPPER_BOUNDARY)
		return;
	pCurrent->pfA_Vec->push_back(f_value);
	pCurrent->pFA_Vec->push_back(F_value);
	vectorStr.clear();
}

void Scan_rArB(const string& strLine, ScanFile* pCurrent)
{
#define RARB_VECTOR_SIZE 4
#define LOWER_BOUNDARY 0
	if (strLine.empty())
		return;
	vector<string> vectorStr;
	pCurrent->SplitString(strLine, vectorStr, "\t");
	if (vectorStr.size() < RARB_VECTOR_SIZE) {
		vectorStr.clear();
		return;
	}
	double left = atof(vectorStr[1].data());
	double right = atof(vectorStr[2].data());
	double stepLength = atof(vectorStr[3].data());
	if (left < LOWER_BOUNDARY || left >= right || stepLength < 0)
		return;
	if ("rA" == vectorStr[0]) {
		pCurrent->rA_range = make_pair(left, right);
		pCurrent->minStepSizeRA = stepLength;
	}
	else if ("rB" == vectorStr[0]) {
		pCurrent->rB_range = make_pair(left, right);
		pCurrent->minStepSizeRB = stepLength;
	}
	vectorStr.clear();
}

void ScanFile::Scan(const string& strSimuFile)
{
	m_SimuFile.open(strSimuFile, ios::in);
	if (!m_SimuFile.is_open())
		exit(0);
	ScanFileSection("fA_FA", "End_fA_FA", ScanfAFA);
	ScanFileSection("rA_rB", "End_rA_rB", Scan_rArB);
}

void WriteToFile(vector<rArBFAYr*>* pFcalA_V, string filename)
{
	ofstream outFile;
	//string stringOutFile = m_strSimuFile + appx + ".out";
	outFile.open(filename.data(), ios::app);
	outFile << "rA\trB\tYR\tDf" << endl;
	for (auto it_rArB = pFcalA_V->begin(); it_rArB != pFcalA_V->end(); it_rArB++)
		outFile << (*it_rArB)->m_rA << "\t" << (*it_rArB)->m_rB
		<< "\t" << (*it_rArB)->m_YrVal << "\t" << (*it_rArB)->m_DfVal << endl;
	outFile.close();
}
