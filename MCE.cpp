#include "MCE.h"
#include <stdlib.h>
#include <time.h>

#define AVER_DP 50
#define MAX_CHAIN_NUMBER 100

uint CreateRandomInteger(uint nMax)
{
	if (nMax < 0 || nMax > RAND_MAX)
		return -1;
	if (0 == nMax)
		return 0;
	return rand() % nMax;	//取得[a,b]的随机整数：rand()%(b-a+1)+a, RAND_MAX = 32767
}

double CreateRandomDouble()
{
	return (rand() / (RAND_MAX + 1.0));
}

MCE::MCE(double ra, double rb, double fA) :
	rA(ra), rB(rb), m_fA(fA),
	NI_0(MAX_CHAIN_NUMBER),
	m_pAllChain(NULL),
	NA(0), NB(0)
{
	p_IA = p_IB = 0;
	p_AA = p_AB = 0;
	p_BA = p_BB = 0; 
}

void MCE::DeleteAllChain()
{
	if (NULL == m_pAllChain)
		return;
	for (auto itor = m_pAllChain->begin(); itor != m_pAllChain->end(); ++itor)
		delete* itor;
	delete 	m_pAllChain;
	m_pAllChain = NULL;
}

MCE::~MCE()
{
	DeleteAllChain();
}

void MCE::Initialization()
{
	UpdateProbability();
	DeleteAllChain();
	m_pAllChain = new vector<pPerChain>;
	for (uint i = 0; i < NI_0; i++)
		m_pAllChain->push_back(new vector<string>);
}

double MCE::Cal_F_from_rf()
{
	Initialization();
	srand((unsigned)time(0));
	double percent = 0;
	while (false == IsFinished_or_Balanced())
		Simulation();
	return (double)NA / (double)(NA + NB);
}

void MCE::Simulation()
{
	uint random_1 = CreateRandomInteger(NI_0);
	pPerChain currentChain = m_pAllChain->at(random_1);
	if (currentChain->empty())
		ChainInitiation(currentChain);
	else
		ChainPropagation(currentChain);
}

void MCE::ChainInitiation(pPerChain& currentChain)
{
	double random_2 = CreateRandomDouble();
	if (0 <= random_2 && random_2 < p_IA){
		currentChain->push_back("A");
		++NA;
	}
	else if (p_IA <= random_2 && random_2 < 1) {
		currentChain->push_back("B");
		++NB;
	}
}

void MCE::ChainPropagation(pPerChain& currentChain)
{
	if ("A" == currentChain->back()) {
		double random_3 = CreateRandomDouble();
		if (0 <= random_3 && random_3 < p_AA) {
			currentChain->back() = "A";
			++NA;
		}
		else if (p_AA <= random_3 && random_3 < 1) {
			currentChain->back() = "B";
			++NB;
		}
	}
	else if("B" == currentChain->back()) {
		double random_4 = CreateRandomDouble();
		if (0 <= random_4 && random_4 < p_BA) {
			currentChain->back() = "A";
			++NA;
		}
		else if (p_BA <= random_4 && random_4 < 1) {
			currentChain->back() = "B";
			++NB;
		}
	}
}

bool MCE::IsFinished_or_Balanced()
{
	return (NA + NB) > (NI_0 * AVER_DP) ? true : false;
}

void MCE::UpdateProbability()
{
	p_AA = rA * m_fA / (rA * m_fA + 1 - m_fA);
	p_AB = 1 - p_AA;

	p_BB = rB * (1 - m_fA) / (rB * (1 - m_fA) + m_fA);
	p_BA = 1 - p_BB;

	p_IA = p_AA + p_BA / (p_AA + p_BA + p_BB + p_AB);
	p_IB = p_BB + p_AB / (p_AA + p_BA + p_BB + p_AB);
}
