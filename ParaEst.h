#pragma once
#include "MCE.h"
#include <vector>
#include "tool.h"
#include <thread>

class ParaEst
{
public:
	ParaEst(const string& strSimuFile);
	~ParaEst();
private:
	string m_strSimuFile;
	ScanFile scanFile;
	double minStepSizeRA;
	double minStepSizeRB;
	vector<thread*> _thread_Vec;
	vector<vector<rArBFAYr*>*> _FcalA_Vec_MC_Vec;
	int _ThreadCount;
private:
	ddRange rA_Range;
	ddRange rB_Range;
	vector<double>* pfA_Vec;
	vector<double>* pFA_Vec;
	vector<rArBFAYr*>* pFcalA_Vec_ML;
	vector<rArBFAYr*>* pFcalA_Vec_MC;
public:
	void CalcOn_MCE();
	void CalcOn_M_L();
	vector<double>* GetpfA_Vec() {
		return pfA_Vec;
	}
	vector<vector<rArBFAYr*>*>& GetFcalA_Vec_MC_Vec() {
		return _FcalA_Vec_MC_Vec;
	}
	vector<rArBFAYr*>* GetFcalA_Vec_ML() {
		return pFcalA_Vec_ML;
	}
private:
	void DeletepFA_Vec(vector<rArBFAYr*>* pFA_Vec);
	void Cal_F_for_fA_Vec_ML(double rA, double rB);
	void PostProba_YrVal(double stepSize_rA, double stepSize_rB, vector<rArBFAYr*>* pFcalA_V);
	ddPair StepSizerArB();
	void OnMultiThreads(ddPair stepSize);
	void Loop_M_L(ddPair stepSize);
};

