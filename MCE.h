#pragma once
#include <vector>
#include <string>
using namespace std;

typedef unsigned int uint;
typedef vector<string>* pPerChain;
typedef struct Ff_pairs
{
	double conv;
	double f_A;
	double F_A;
	Ff_pairs(double percent,double fA, double FA):
	conv(percent),f_A(fA), F_A(FA){}
} Ff_pairs;

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

