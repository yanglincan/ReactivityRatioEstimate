#include "ParaEst.h"
#include <fstream>
#include <iostream>

using namespace std;

#define STEPS 100
#define TOTAL_PROBABILITY 1
static int threadcounting = 0;

ParaEst::ParaEst(const string& strSimuFile) :
	rA_Range(ddRange(0, 0)), rB_Range(ddRange(0, 0)), 
	minStepSizeRA(0), minStepSizeRB(0),
	pFcalA_Vec_ML(NULL), pFcalA_Vec_MC(NULL),
	m_strSimuFile(strSimuFile),
	_ThreadCount(0)
{
	pFcalA_Vec_ML = new vector<rArBFAYr*>;
	pFcalA_Vec_MC = new vector<rArBFAYr*>;
	//ɨ�������ļ�
	scanFile.Scan(m_strSimuFile);
	pfA_Vec = scanFile.pfA_Vec;		//��ȡ fA ��ֵ
	pFA_Vec = scanFile.pFA_Vec;		//��ȡ FA ��ֵ
	rA_Range = scanFile.rA_range;	//��ȡ rA ��Χ
	rB_Range = scanFile.rB_range;	//��ȡ rB ��Χ
	if (0 < scanFile.minStepSizeRA)
		minStepSizeRA = scanFile.minStepSizeRA;		//��ȡ rA��С����
	if (0 < scanFile.minStepSizeRB)
		minStepSizeRB = scanFile.minStepSizeRB;		//��ȡ rB��С����
}

ParaEst::~ParaEst()
{
	DeletepFA_Vec(pFcalA_Vec_ML);
	DeletepFA_Vec(pFcalA_Vec_MC);
}

//Mayo-Lewis ��ʽ
double Cal_F_from_rf(double rA, double rB, double fA, double fB)
{
	return (rA * fA * fA + fA * fB) / (rA * fA * fA + 2 * fA * fB + rB * fB * fB);
}

//���� Monte Carlo Estimmation �����ļ������
void ParaEst::CalcOn_MCE()
{
	ddPair stepSize = StepSizerArB();	//��ȡ rA �� rB ��ǰ��������pair�� first Ϊ rA ������second Ϊ rB ����
	OnMultiThreads(stepSize);			//�ڶ��߳���ִ�м��㣬��С����ʱ��
	cout << "Monte Carlo Estimation Result:  " << endl;
	PostProba_YrVal(stepSize.first, stepSize.second, pFcalA_Vec_MC);	//��������ܶ� �������Լ� ��������ܶ����� �� ������֡�
	WriteToFile(pFcalA_Vec_MC, m_strSimuFile + "_MCE.out");		//�������
	cout << endl;
}

//���� Mayo-Lewis���� �ļ������
void ParaEst::CalcOn_M_L()
{
	ddPair stepSize = StepSizerArB();		//��ȡ rA �� rB ��ǰ��������pair�� first Ϊ rA ������second Ϊ rB ����
	Loop_M_L(stepSize);						//���ղ�����С��ѭ��ִ��
	cout << "Mayo-Lewis Equation Result:  " << endl;
	PostProba_YrVal(stepSize.first, stepSize.second, pFcalA_Vec_ML);	//��������ܶ� �������Լ� ��������ܶ����� �� ������֡�
	WriteToFile(pFcalA_Vec_ML, m_strSimuFile + "_M-L.out");		//�������
	cout << endl;
}

//��ÿ�� fAֵ ���� Monte Carlo Estimmation ����
void Cal_F_for_fA_Vec_MC(double rA, double rB, vector<rArBFAYr*>* pFcalA_Vec, vector<double>* pfA_Vec)
{
	rArBFAYr* pTemp = new rArBFAYr(rA, rB);
	pTemp->m_pFA_V = new vector<double>;
	for (auto itor = pfA_Vec->begin(); itor != pfA_Vec->end(); itor++) {
		MCE mce(rA, rB, *itor);		//*itor Ϊ fAֵ
		pTemp->m_pFA_V->push_back(mce.Cal_F_from_rf());		//Cal_F_from_rf ���� ִ�� �������
	}
	pFcalA_Vec->push_back(pTemp);
}

//��ÿ�� fAֵ ���� Mayo-Lewis���� ����
void ParaEst::Cal_F_for_fA_Vec_ML(double rA, double rB)
{
	rArBFAYr* pTemp = new rArBFAYr(rA, rB);
	pTemp->m_pFA_V = new vector<double>;
	for (auto itor = pfA_Vec->begin(); itor != pfA_Vec->end(); itor++)			//*itor Ϊ fAֵ
		pTemp->m_pFA_V->push_back(Cal_F_from_rf(rA, rB, *itor, 1 - (*itor)));	//Cal_F_from_rf ���� ִ�� �������
	pFcalA_Vec_ML->push_back(pTemp);
}

//��ÿ�� rA �� rB ���� Monte Carlo Estimmation ����
void Loop_MCE(ddRange rA_LocalRange, ddRange rB_LocalRange, ddPair stepSize, ParaEst* pParaEst, int threadPos)
{
	for (double rA_step = rA_LocalRange.first; rA_step < rA_LocalRange.second; rA_step += stepSize.first)
		for (double rB_step = rB_LocalRange.first; rB_step < rB_LocalRange.second; rB_step += stepSize.second)
			Cal_F_for_fA_Vec_MC(rA_step, rB_step, pParaEst->GetFcalA_Vec_MC_Vec().at(threadPos), pParaEst->GetpfA_Vec());
	++threadcounting;	//ȫ������ͳ����ɵ��߳�����
}

//��ÿ�� rA �� rB ���� Mayo-Lewis���� ����
void ParaEst::Loop_M_L(ddPair stepSize)
{
	for (double rA_step = rA_Range.first; rA_step < rA_Range.second; rA_step += stepSize.first)
		for (double rB_step = rB_Range.first; rB_step < rB_Range.second; rB_step += stepSize.second)
			Cal_F_for_fA_Vec_ML(rA_step, rB_step);
}

//�����ٲ���
ddPair ParaEst::StepSizerArB()
{
	//Ĭ�Ͻ���Χ��Ϊ100�ݣ�STEPS Ϊ 100����ÿһ�ݼ�Ϊ��������� ��� �ܹ��� 100*100 ��������ϣ�rA,rB��
	//�����Χ̫С��Ĭ�ϵ�100��������̫�ܼ������������ļ��У��趨��С���� minStepSize����С����
	if (rA_Range.second <= rA_Range.first || rB_Range.second <= rB_Range.first)
		exit(0);
	double stepSize_rA = (rA_Range.second - rA_Range.first) / STEPS < minStepSizeRA ? minStepSizeRA : (rA_Range.second - rA_Range.first) / STEPS;
	double stepSize_rB = (rB_Range.second - rB_Range.first) / STEPS < minStepSizeRB ? minStepSizeRB : (rB_Range.second - rB_Range.first) / STEPS;
	return make_pair(stepSize_rA, stepSize_rB);
}

void ParaEst::PostProba_YrVal(double stepSize_rA, double stepSize_rB, vector<rArBFAYr*>* pFcalA_V)
{
	if (pFcalA_V->empty())
		return;
	double rA_cal = 0;
	double rB_cal = 0;
	double forC = 0;
	double CoefficientC = 0;
	//���� ��׼�Ϊ����(rA,rB)�µı�׼���ƽ��ֵ
	double aver_Sigma = 0;
	for (auto it_rArB = pFcalA_V->begin(); it_rArB != pFcalA_V->end(); it_rArB++) {
		size_t count = (*it_rArB)->m_pFA_V->size();
		//ƽ��ֵ
		double aver = 0;
		for (size_t pos = 0; pos < count; pos++)
			aver += (pFA_Vec->at(pos) - (*it_rArB)->m_pFA_V->at(pos));
		aver /= count;
		//����(rA,rB)�ı�׼��
		double sigma = 0;
		for (size_t pos = 0; pos < count; pos++)
			sigma += pow(((pFA_Vec->at(pos) - (*it_rArB)->m_pFA_V->at(pos)) - aver), 2);
		aver_Sigma += sqrt(sigma / count);		//����(rA,rB)�ı�׼��
	}
	aver_Sigma /= pFcalA_V->size();

	for (auto it_rArB = pFcalA_V->begin(); it_rArB != pFcalA_V->end(); it_rArB++) {
		size_t count = (*it_rArB)->m_pFA_V->size();
		//��������ܶ�
		double yr = 0;
		for (size_t pos = 0; pos < count; pos++)
			yr += pow((pFA_Vec->at(pos) - (*it_rArB)->m_pFA_V->at(pos)) / aver_Sigma, 2);
		(*it_rArB)->m_YrVal = yr;
		(*it_rArB)->m_DfVal = exp(-0.5 * yr);
		//����drA*drB����������л��ֵ�Ԫ��С�����dxdy
		double uint = stepSize_rA * stepSize_rB;
		//��������ۼ�
		forC += (*it_rArB)->m_DfVal * uint;
		//rA��rB�ļ�Ȩƽ��ֵ
		rA_cal += (*it_rArB)->m_DfVal * (*it_rArB)->m_rA * uint;
		rB_cal += (*it_rArB)->m_DfVal * (*it_rArB)->m_rB * uint;
	}
	//�������ϵ��C����һ�����������ܶ��ܻ��� TOTAL_PROBABILITY Ϊ 1��
	CoefficientC = TOTAL_PROBABILITY / forC;
	cout << "Average Value :" << endl;
	cout << "\trA = " << CoefficientC * rA_cal << endl;
	cout << "\trB = " << CoefficientC * rB_cal << endl;
	//������Ȼ����
	auto itor_minYrVal = pFcalA_V->begin();
	for (auto it_rArB = pFcalA_V->begin(); it_rArB != pFcalA_V->end(); it_rArB++)
		if ((*it_rArB)->m_YrVal < (*itor_minYrVal)->m_YrVal)
			itor_minYrVal = it_rArB;	//����Сֵ
	cout << "Max. Likelihood:" << endl;
	cout << "\trA = " << (*itor_minYrVal)->m_rA << endl;
	cout << "\trB = " << (*itor_minYrVal)->m_rB << endl;
}

void ParaEst::DeletepFA_Vec(vector<rArBFAYr*>* pFA_Vec)
{
	if (NULL == pFA_Vec)
		return;
	for (auto itor = pFA_Vec->begin(); itor != pFA_Vec->end(); ++itor) {
		delete (*itor)->m_pFA_V;
		delete* itor;
	}
	delete 	pFA_Vec;
	pFA_Vec = NULL;
}

//���̼߳���MCE
void ParaEst::OnMultiThreads(ddPair stepSize)
{
	SplitInfo splitInfo = SplitRect(rA_Range, rB_Range, GetThreadNumber());
	_ThreadCount = splitInfo._blocks;
	ddRange rA_LocalRange;
	ddRange rB_LocalRange;
	for (int i = 0; i < _ThreadCount; i++)
		_FcalA_Vec_MC_Vec.push_back(new vector<rArBFAYr*>);
	//���̷ָ߳�����
	int threadPos = 0;
	for (int y_step = 0; y_step < splitInfo._y_blocks; y_step++)
		for (int x_step = 0; x_step < splitInfo._x_blocks; x_step++) {
			rA_LocalRange = make_pair(rA_LocalRange.first = rA_Range.first + x_step * splitInfo._x_stepLength,
				rA_LocalRange.second = rA_Range.first + (x_step + 1) * splitInfo._x_stepLength);
			if (rA_LocalRange.second > rA_Range.second)
				rA_LocalRange.second = rA_Range.second;
			rB_LocalRange = make_pair(rB_LocalRange.first = rB_Range.first + y_step * splitInfo._y_stepLength,
				rB_LocalRange.second = rB_Range.first + (y_step + 1) * splitInfo._y_stepLength);
			if (rB_LocalRange.second > rB_Range.second)
				rB_LocalRange.second = rB_Range.second;
			thread* pThread = new thread(Loop_MCE, rA_LocalRange, rB_LocalRange, stepSize, this, threadPos);
			pThread->detach();
			_thread_Vec.push_back(pThread);
			++threadPos;
		}
	//�ȴ������߳̽���
	while (threadcounting < _ThreadCount)
		::this_thread::sleep_for(chrono::duration<double, std::milli>(1000));
	threadcounting = 0;
	//�����߳����ݻ���
	for (auto itor = _FcalA_Vec_MC_Vec.begin(); itor != _FcalA_Vec_MC_Vec.end(); itor++)
		for (auto itor2 = (*itor)->begin(); itor2 != (*itor)->end(); itor2++)
			pFcalA_Vec_MC->push_back((*itor2));
	//�ڴ��ͷ�
	for (auto itor = _thread_Vec.begin(); itor != _thread_Vec.end(); itor++)
		delete (*itor);
	for (auto itor = _FcalA_Vec_MC_Vec.begin(); itor != _FcalA_Vec_MC_Vec.end(); itor++)
		delete (*itor);
}
