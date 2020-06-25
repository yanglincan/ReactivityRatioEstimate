#pragma once
#include <vector>
#include <string>

using namespace std;

typedef unsigned int uint;
typedef vector<string>* pPerChain;

class MCE
{
public:
	MCE(double ra = 0, double rb = 0, double fA = 0);
	~MCE();
	double Cal_F_from_rf();
private:
	double rA, rB;
	unsigned int NI_0;
	unsigned int NA, NB;
	double p_IA, p_IB;
	double p_AA, p_AB;
	double p_BA, p_BB;
	double m_fA;
private:
	vector<pPerChain>* m_pAllChain;
private:
	void DeleteAllChain();
	void Initialization();
	void Simulation();
	void ChainInitiation(pPerChain& currentChain);
	void ChainPropagation(pPerChain& currentChain);
	void UpdateProbability();
	bool IsFinished_or_Balanced();
};

