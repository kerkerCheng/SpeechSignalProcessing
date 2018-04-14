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

void alpha_calculate();

int main()
{
	HMM hmm_initial;
	HMM hmm[MD_NUM];
	loadHMM(&hmm_initial, "model_init.txt");

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
			vector<char> one_line;
			while(iss >> ch)
			{
				int ch_int;
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

void epsilon_calculate(double ***ep, double **alpha, double **beta, double **ijmap, vector<int>& seq, HMM &hmm)
{
	int seq_len = seq.size();

	for(int t=0; t<seq_len; t++)
	{
		for(int i=0; i<STATE_NUM; i++)
		{
			for(int j=0; j<STATE_NUM; j++)
				ijmap[i][j] = alpha[i][t] * hmm.transition[i][j] * hmm.observation[j][seq[t+1]] * beta[j][t+1];
		}

		double deno = array_sum(ijmap);

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

void **a_bar_calculate(double **a_bar, double **ep, double **ga, vector<int>& seq)
{
	int seq_len = seq.size();

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
}

double array_sum(int row, int col, double **arr)
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