#pragma once
#include <iostream>
#include "windows.h"
#include <algorithm>
#include <string>
#include <vector>
#include <io.h> 
#include <fstream>

using namespace std;

typedef unsigned long       DWORD;
typedef pair<double, double> ddRange;
typedef pair<double, double> ddPair;

enum SPLIT_MODE { X_MODE = 0, Y_MODE };

typedef struct SplitInfo
{
	int _blocks;
	double _x_stepLength;
	double _y_stepLength;
	int _x_blocks;
	int _y_blocks;
	SplitInfo(int blocks = 0, double x_stepLength = 0, double y_stepLength = 0) :
		_blocks(blocks), _x_stepLength(x_stepLength), _y_stepLength(y_stepLength) {}
} SplitInfo;

typedef struct rArBFAYr
{
	double m_rA;
	double m_rB;
	vector<double>* m_pFA_V;
	double m_YrVal;
	double m_DfVal;
	rArBFAYr(double rA, double rB) :m_rA(rA), m_rB(rB), m_pFA_V(NULL), m_YrVal(0), m_DfVal(0) {}
} rArBFAYr;

class GetFiles
{
public:
	const vector<string>& getFiles(const string& path);
	//char *getcwd( char *buffer, int maxlen ）
	//函数能够获取当前的工作目录，具体来说，它会将当前工作目录的绝对路径复制到参数buffer所指的内存空间中,参数maxlen为buffer的空间大小。
	string GetWorkingDirectory();
private:
	vector<string> m_FilesVector;
};

class ScanFile
{
private:
	void ScanFileSection(const string& strBegin, const string& strEnd, void(*Analysis)(const string& strLine, ScanFile* pCurrent));

private:
	ifstream m_SimuFile;
public:
	ScanFile();
	~ScanFile();
	void SplitString(const string& s, vector<string>& v, const string& c);
	void Scan(const string& strSimuFile);
public:
	vector<double>* pfA_Vec;
	vector<double>* pFA_Vec;
	ddRange rA_range;
	ddRange rB_range;
	double minStepSizeRA;
	double minStepSizeRB;
};

SplitInfo SplitRect(const ddRange x_range, const ddRange y_range, const int desiredBlocks);
int GetThreadNumber();
const string GetSimuFile();
void WriteToFile(vector<rArBFAYr*>* pFcalA_V, string filename);