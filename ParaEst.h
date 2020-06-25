#pragma once
#include "MCE.h"
#include <vector>
#include "tool.h"
#include <thread>

#define STEPS 100
#define TOTAL_PROBABILITY 1

typedef struct rArBFAYr
{
	double m_rA;
	double m_rB;
	vector<double>* m_pFA_V;
	double m_YrVal;
	double m_DfVal;
	rArBFAYr(double rA, double rB) :m_rA(rA), m_rB(rB), m_pFA_V(NULL), m_YrVal(0), m_DfVal(0){}
} rArBFAYr;

class ParaEst
{
public:
	ParaEst(const string& strSimuFile, ScanFile& scanFile);
	~ParaEst();
	double rA_Aver, rB_Aver;
	double rA_MaxLikelihood, rB_MaxLikelihood;
private:
	string m_strSimuFile;
	//ScanFile scanFile;
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
	void WriteToFile(vector<rArBFAYr*>* pFcalA_V, string appx = "");
	ddPair StepSizerArB();
	void OnMultiThreads(ddPair stepSize);
	void Loop_M_L(ddPair stepSize);
};

