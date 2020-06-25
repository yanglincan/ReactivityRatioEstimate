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
	//扫描输入文件
	scanFile.Scan(m_strSimuFile);
	pfA_Vec = scanFile.pfA_Vec;		//获取 fA 数值
	pFA_Vec = scanFile.pFA_Vec;		//获取 FA 数值
	rA_Range = scanFile.rA_range;	//获取 rA 范围
	rB_Range = scanFile.rB_range;	//获取 rB 范围
	if (0 < scanFile.minStepSizeRA)
		minStepSizeRA = scanFile.minStepSizeRA;		//获取 rA最小步长
	if (0 < scanFile.minStepSizeRB)
		minStepSizeRB = scanFile.minStepSizeRB;		//获取 rB最小步长
}

ParaEst::~ParaEst()
{
	DeletepFA_Vec(pFcalA_Vec_ML);
	DeletepFA_Vec(pFcalA_Vec_MC);
}

//Mayo-Lewis 公式
double Cal_F_from_rf(double rA, double rB, double fA, double fB)
{
	return (rA * fA * fA + fA * fB) / (rA * fA * fA + 2 * fA * fB + rB * fB * fB);
}

//基于 Monte Carlo Estimmation 方法的计算过程
void ParaEst::CalcOn_MCE()
{
	ddPair stepSize = StepSizerArB();	//获取 rA 和 rB 的前进步长，pair中 first 为 rA 步长，second 为 rB 步长
	OnMultiThreads(stepSize);			//在多线程上执行计算，减小计算时间
	cout << "Monte Carlo Estimation Result:  " << endl;
	PostProba_YrVal(stepSize.first, stepSize.second, pFcalA_Vec_MC);	//后验概率密度 评估，以及 后验概率密度曲面 的 体积积分。
	WriteToFile(pFcalA_Vec_MC, m_strSimuFile + "_MCE.out");		//输出数据
	cout << endl;
}

//基于 Mayo-Lewis方程 的计算过程
void ParaEst::CalcOn_M_L()
{
	ddPair stepSize = StepSizerArB();		//获取 rA 和 rB 的前进步长，pair中 first 为 rA 步长，second 为 rB 步长
	Loop_M_L(stepSize);						//按照步长大小，循环执行
	cout << "Mayo-Lewis Equation Result:  " << endl;
	PostProba_YrVal(stepSize.first, stepSize.second, pFcalA_Vec_ML);	//后验概率密度 评估，以及 后验概率密度曲面 的 体积积分。
	WriteToFile(pFcalA_Vec_ML, m_strSimuFile + "_M-L.out");		//输出数据
	cout << endl;
}

//对每个 fA值 进行 Monte Carlo Estimmation 计算
void Cal_F_for_fA_Vec_MC(double rA, double rB, vector<rArBFAYr*>* pFcalA_Vec, vector<double>* pfA_Vec)
{
	rArBFAYr* pTemp = new rArBFAYr(rA, rB);
	pTemp->m_pFA_V = new vector<double>;
	for (auto itor = pfA_Vec->begin(); itor != pfA_Vec->end(); itor++) {
		MCE mce(rA, rB, *itor);		//*itor 为 fA值
		pTemp->m_pFA_V->push_back(mce.Cal_F_from_rf());		//Cal_F_from_rf 函数 执行 具体计算
	}
	pFcalA_Vec->push_back(pTemp);
}

//对每个 fA值 进行 Mayo-Lewis方程 计算
void ParaEst::Cal_F_for_fA_Vec_ML(double rA, double rB)
{
	rArBFAYr* pTemp = new rArBFAYr(rA, rB);
	pTemp->m_pFA_V = new vector<double>;
	for (auto itor = pfA_Vec->begin(); itor != pfA_Vec->end(); itor++)			//*itor 为 fA值
		pTemp->m_pFA_V->push_back(Cal_F_from_rf(rA, rB, *itor, 1 - (*itor)));	//Cal_F_from_rf 函数 执行 具体计算
	pFcalA_Vec_ML->push_back(pTemp);
}

//对每对 rA 和 rB 进行 Monte Carlo Estimmation 计算
void Loop_MCE(ddRange rA_LocalRange, ddRange rB_LocalRange, ddPair stepSize, ParaEst* pParaEst, int threadPos)
{
	for (double rA_step = rA_LocalRange.first; rA_step < rA_LocalRange.second; rA_step += stepSize.first)
		for (double rB_step = rB_LocalRange.first; rB_step < rB_LocalRange.second; rB_step += stepSize.second)
			Cal_F_for_fA_Vec_MC(rA_step, rB_step, pParaEst->GetFcalA_Vec_MC_Vec().at(threadPos), pParaEst->GetpfA_Vec());
	++threadcounting;	//全局量，统计完成的线程数量
}

//对每对 rA 和 rB 进行 Mayo-Lewis方程 计算
void ParaEst::Loop_M_L(ddPair stepSize)
{
	for (double rA_step = rA_Range.first; rA_step < rA_Range.second; rA_step += stepSize.first)
		for (double rB_step = rB_Range.first; rB_step < rB_Range.second; rB_step += stepSize.second)
			Cal_F_for_fA_Vec_ML(rA_step, rB_step);
}

//步长仲裁器
ddPair ParaEst::StepSizerArB()
{
	//默认将范围分为100份（STEPS 为 100），每一份即为步长，因此 最多 总共有 100*100 竞聚率组合（rA,rB）
	//如果范围太小，默认的100步，步数太密集，可在输入文件中，设定最小步长 minStepSize，减小步数
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
	//计算 标准差，为所有(rA,rB)下的标准差的平均值
	double aver_Sigma = 0;
	for (auto it_rArB = pFcalA_V->begin(); it_rArB != pFcalA_V->end(); it_rArB++) {
		size_t count = (*it_rArB)->m_pFA_V->size();
		//平均值
		double aver = 0;
		for (size_t pos = 0; pos < count; pos++)
			aver += (pFA_Vec->at(pos) - (*it_rArB)->m_pFA_V->at(pos));
		aver /= count;
		//单个(rA,rB)的标准差
		double sigma = 0;
		for (size_t pos = 0; pos < count; pos++)
			sigma += pow(((pFA_Vec->at(pos) - (*it_rArB)->m_pFA_V->at(pos)) - aver), 2);
		aver_Sigma += sqrt(sigma / count);		//单个(rA,rB)的标准差
	}
	aver_Sigma /= pFcalA_V->size();

	for (auto it_rArB = pFcalA_V->begin(); it_rArB != pFcalA_V->end(); it_rArB++) {
		size_t count = (*it_rArB)->m_pFA_V->size();
		//后验概率密度
		double yr = 0;
		for (size_t pos = 0; pos < count; pos++)
			yr += pow((pFA_Vec->at(pos) - (*it_rArB)->m_pFA_V->at(pos)) / aver_Sigma, 2);
		(*it_rArB)->m_YrVal = yr;
		(*it_rArB)->m_DfVal = exp(-0.5 * yr);
		//计算drA*drB，体积积分中积分单元的小底面积dxdy
		double uint = stepSize_rA * stepSize_rB;
		//体积积分累加
		forC += (*it_rArB)->m_DfVal * uint;
		//rA和rB的加权平均值
		rA_cal += (*it_rArB)->m_DfVal * (*it_rArB)->m_rA * uint;
		rB_cal += (*it_rArB)->m_DfVal * (*it_rArB)->m_rB * uint;
	}
	//计算比例系数C，归一化处理，概率密度总积分 TOTAL_PROBABILITY 为 1。
	CoefficientC = TOTAL_PROBABILITY / forC;
	cout << "Average Value :" << endl;
	cout << "\trA = " << CoefficientC * rA_cal << endl;
	cout << "\trB = " << CoefficientC * rB_cal << endl;
	//极大似然估计
	auto itor_minYrVal = pFcalA_V->begin();
	for (auto it_rArB = pFcalA_V->begin(); it_rArB != pFcalA_V->end(); it_rArB++)
		if ((*it_rArB)->m_YrVal < (*itor_minYrVal)->m_YrVal)
			itor_minYrVal = it_rArB;	//找最小值
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

//多线程计算MCE
void ParaEst::OnMultiThreads(ddPair stepSize)
{
	SplitInfo splitInfo = SplitRect(rA_Range, rB_Range, GetThreadNumber());
	_ThreadCount = splitInfo._blocks;
	ddRange rA_LocalRange;
	ddRange rB_LocalRange;
	for (int i = 0; i < _ThreadCount; i++)
		_FcalA_Vec_MC_Vec.push_back(new vector<rArBFAYr*>);
	//多线程分割区块
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
	//等待各个线程结束
	while (threadcounting < _ThreadCount)
		::this_thread::sleep_for(chrono::duration<double, std::milli>(1000));
	threadcounting = 0;
	//各个线程数据汇总
	for (auto itor = _FcalA_Vec_MC_Vec.begin(); itor != _FcalA_Vec_MC_Vec.end(); itor++)
		for (auto itor2 = (*itor)->begin(); itor2 != (*itor)->end(); itor2++)
			pFcalA_Vec_MC->push_back((*itor2));
	//内存释放
	for (auto itor = _thread_Vec.begin(); itor != _thread_Vec.end(); itor++)
		delete (*itor);
	for (auto itor = _FcalA_Vec_MC_Vec.begin(); itor != _FcalA_Vec_MC_Vec.end(); itor++)
		delete (*itor);
}
