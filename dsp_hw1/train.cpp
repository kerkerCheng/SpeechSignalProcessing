#include "hmm.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

#ifndef SEQ_NUM
#	define SEQ_NUM	50
#endif

#ifndef MD_NUM
#	define MD_NUM	5
#endif

#ifndef STATE_NUM
#	define STATE_NUM	6
#endif

using namespace std;

void alpha_calculate(double **alpha, vector<int>& seq, HMM &hmm);
void beta_calcaulate(double **beta, vector<int>& seq, HMM &hmm);
void epsilon_calculate(double ***ep, double **alpha, double **beta, double **ijmap, vector<int>& seq, HMM* hmm);
void gamma_calculate(double **gamma, double **alpha, double **beta, vector<int>& seq);
void update_a(double **a_bar, double ***ep, double **ga, vector<int>& seq, HMM *hmm);
void update_pi(double **ga, HMM *hmm);
void update_observation(double **ga, HMM *hmm, vector<int>& seq);
double viterbi(double** del, HMM* hmm, vector<int>& seq);
double array_sum(double **arr, int row, int col);


int main()
{
	HMM hmm_initial;
	HMM hmm[MD_NUM];
	loadHMM(&hmm_initial, "model_init.txt");

	for(int i=0; i<5; i++)
		hmm[i] = hmm_initial;

	ifstream seq_files[MD_NUM];
	for(int i=0; i<MD_NUM; i++)
	{
		stringstream filename;
		filename << "seq_model_0" << (i+1) << ".txt";
		string tmp = filename.str();
		seq_files[i].open(tmp.c_str());
	}

	vector< vector< vector<int> > > seq;
	string line;
	seq.resize(MD_NUM);

	for(int i=0; i<MD_NUM; i++)
	{
		seq[i].reserve(9000);
		while(getline(seq_files[i], line))
		{
			istringstream iss(line);
			char ch;
			vector<int> one_line;
			while(iss >> ch)
			{
				int ch_int=0;
				switch(ch)
				{
					case 'A':
						ch_int=0;
						break;
					case 'B':
						ch_int=1;
						break;
					case 'C':
						ch_int=2;
						break;
					case 'D':
						ch_int=3;
						break;
					case 'E':
						ch_int=4;
						break;
					case 'F':
						ch_int=5;
						break;
				}
				one_line.push_back(ch_int);
			}
			seq[i].push_back(one_line);
		}
	}

	/* Initialize */
	double **alpha = new double*[STATE_NUM];
	double **beta = new double*[STATE_NUM];
	double **gamma = new double*[STATE_NUM];
	double **ijmap = new double*[STATE_NUM];
	double **a_bar = new double*[STATE_NUM];
	double ***epsilon = new double**[SEQ_NUM-1];

	for(int i=0; i<(SEQ_NUM-1); i++)
	{
		epsilon[i] = new double*[STATE_NUM];
		for(int j=0; j<STATE_NUM; j++)
			epsilon[i][j] = new double[STATE_NUM];
	}

	for(int i=0; i<STATE_NUM; i++)
	{
		alpha[i] = new double[SEQ_NUM];
		beta[i] = new double[SEQ_NUM];
		gamma[i] = new double[SEQ_NUM];
		ijmap[i] = new double[STATE_NUM];
		a_bar[i] = new double[STATE_NUM];
	}

	/* Training */
	for(int i=0; i<5; i++)
	{
		for(vector< vector<int> >::iterator it1 = seq[i].begin(); it1 != seq[i].end(); ++it1)
		{
			alpha_calculate(alpha, *it1, hmm[i]);
			beta_calcaulate(beta, *it1, hmm[i]);
			epsilon_calculate(epsilon, alpha, beta, ijmap, *it1, &hmm[i]);
			gamma_calculate(gamma, alpha, beta, *it1);

			update_a(a_bar, epsilon, gamma, *it1, &hmm[i]);
			update_pi(gamma, &hmm[i]);
			update_observation(gamma, &hmm[i], *it1);
		}
	}
	


	
	// FILE *f;
	// f = fopen("test.txt", "w");
	// for(vector< vector<char> >::iterator it=seq[0].begin(); it != seq[0].end(); ++it)
	// {
	// 	for(vector<char>::iterator it2=(*it).begin(); it2 != (*it).end(); ++it2)
	// 		fprintf(f, "%c ", *it2);
	// 	fprintf(f, "\n");
	// }

}

void alpha_calculate(double **alpha, vector<int>& seq, HMM &hmm)
{
	int seq_len = seq.size();

	for(int i=0; i<STATE_NUM; i++)				// i is STATE
		alpha[i][0] = hmm.initial[i] * hmm.observation[i][seq[0]];

	
	for(int t=1; t<seq_len; t++)				// t is TIME
	{
		for(int i=0; i<STATE_NUM; i++)
		{
			int sum = 0;
			for(int j=0; j<STATE_NUM; j++)
				sum += alpha[j][t-1] * hmm.transition[j][i];
			alpha[i][t] = sum * hmm.observation[i][seq[t]];
		}
	}
}

void beta_calcaulate(double **beta, vector<int>& seq, HMM &hmm)
{
	int seq_len = seq.size();

	for(int i=0; i<STATE_NUM; i++)				//i is STATE
		beta[i][seq_len-1] = 1;

	for(int t=seq_len-2; t>=0; t--)
	{
		for(int i=0; i<STATE_NUM; i++)
		{	
			int sum = 0;
			for(int j=0; j<STATE_NUM; j++)
				sum += hmm.transition[i][j] * hmm.observation[j][seq[t+1]] * beta[j][t+1];
			beta[i][t] = sum;
		}	
	}
}

void epsilon_calculate(double ***ep, double **alpha, double **beta, double **ijmap, vector<int>& seq, HMM* hmm)
{
	int seq_len = seq.size();

	for(int t=0; t<(seq_len-1); t++)
	{
		for(int i=0; i<STATE_NUM; i++)
		{
			for(int j=0; j<STATE_NUM; j++)
				ijmap[i][j] = alpha[i][t] * hmm->transition[i][j] * hmm->observation[j][seq[t+1]] * beta[j][t+1];
		}

		double deno = array_sum(ijmap, STATE_NUM, STATE_NUM);

		for(int i=0; i<STATE_NUM; i++)
		{
			for(int j=0; j<STATE_NUM; j++)
				ep[t][i][j] = ijmap[i][j]/deno;
		}
	}
}

void gamma_calculate(double **gamma, double **alpha, double **beta, vector<int>& seq)
{
	int seq_len = seq.size();
	for(int t=0; t<seq_len; t++)
	{
		double sum = 0;
		for(int i=0; i<STATE_NUM; i++)
			gamma[i][t] = alpha[i][t] * beta[i][t];
		
		for(int n=0; n<STATE_NUM; n++)
			sum += gamma[n][t];

		for(int i=0; i<STATE_NUM; i++)
			gamma[i][t] = gamma[i][t] / sum;
	}
}

void update_a(double **a_bar, double ***ep, double **ga, vector<int>& seq, HMM *hmm)
{
	int sample_size = seq.size();

	for(int i=0; i<STATE_NUM; i++)
	{
		for(int j=0; j<STATE_NUM; j++)
		{
			double deno=0;
			double nume=0;
			for(int t=0; t<(seq_len-1); t++)
			{
				deno += ga[i][t];
				nume += ep[t][i][j];
			}

			a_bar[i][j] = nume/deno;
		}
	}

	for(int i=0; i<STATE_NUM; i++)
	{
		for(int j=0; j<STATE_NUM; j++)
			hmm->transition[i][j] = a_bar[i][j];
	}
}

void update_pi(double **ga, HMM *hmm)
{
	for(int i=0; i<STATE_NUM; i++)
		hmm->initial[i] = ga[i][0];
}

void update_observation(double **ga, HMM *hmm, vector<int>& seq)
{
	int seq_len = seq.size();
	for(int i=0; i<STATE_NUM; i++)
	{
		double deno = 0;
		double nume[STATE_NUM] = {0};
		for(int t=0; t<seq_len; t++)
		{
			deno += ga[i][t];
			switch(seq[t])
			{
				case 0:
					nume[0] += ga[i][t];
					break;
				case 1:
					nume[1] += ga[i][t];
					break;
				case 2:
					nume[2] += ga[i][t];
					break;
				case 3:
					nume[3] += ga[i][t];
					break;
				case 4:
					nume[4] += ga[i][t];
					break;
				case 5:
					nume[5] += ga[i][t];
					break;
			}
		}

		for(int j=0; j<MAX_OBSERV; j++)
		{
			hmm->observation[i][j] = nume[j]/deno;
		}
	}
}

double viterbi(double** del, HMM* hmm, vector<int>& seq)
{
	int seq_len = seq.size();
	for(int i=0; i<STATE_NUM; i++)
		del[i][0] = hmm->initial[i] * hmm->observation[i][seq[0]];

	for(int t=1; t<seq_len; t++)
	{
		for(int j=0; j<STATE_NUM; j++)
		{
			double max = 0;
			int max_state = 0;
			for(int i=0; i<STATE_NUM; i++)
			{
				double t = del[i][t-1] * hmm->transition[i][j];
				if(t > max)
				{
					max = t;
					max_state = i;
				}
			}
			del[j][t] = max * hmm->observation[j][seq[t]];
		}
	}

	double p_star = 0;
	for(int i=0; i<STATE_NUM; i++)
	{
		if(del[i][seq_len-1] > p_star)
			p_star = del[i][seq_len-1];
	}
	
	return p_star;
}

double array_sum(double **arr, int row, int col)
{
	double sum = 0;
	for(int i=0; i<row; i++)
	{
		for(int j=0; j<col; j++)
		{
			sum += arr[i][j];
		}
	}

	return sum;
}