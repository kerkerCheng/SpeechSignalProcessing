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

void alpha_calculate(vector< vector<double> >& alpha, vector<int>& seq, HMM *hmm);
void beta_calcaulate(vector< vector<double> >& beta, vector<int>& seq, HMM *hmm);
void epsilon_calculate(vector< vector< vector<double> > >& ep, vector< vector<double> >& alpha, vector< vector<double> >& beta, vector< vector<double> >& ijmap, vector<int>& seq, HMM* hmm);
void gamma_calculate(vector< vector<double> >& gamma, vector< vector<double> >& alpha, vector< vector<double> >& beta, vector<int>& seq);
void update_hmm(HMM *hmm, vector< vector<int> >& seq);
// void update_a(double **a_bar, double ***ep, double **ga, vector<int>& seq, HMM *hmm);
void update_pi(double **ga, HMM *hmm);
void update_observation(double **ga, HMM *hmm, vector<int>& seq);
double array_sum(double **arr, int row, int col);
double vector_sum(vector< vector<double> >& vec);
void print_vec(vector< vector<double> >& vec);
void print_arr(double **arr, int row, int col);


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
	int iteration = 20;
	cout << "iteration = " << iteration << endl;


	/* Training */
	for(int i=0; i<MD_NUM; i++)
	{
		for(int t=0; t<iteration; t++)
		{
			cout << "epoch = " << (t+1) << " ; model = " << (i+1) << endl;
			update_hmm(&hmm[i], seq[i]);
			// dumpHMM(stderr, &hmm[i]);
			// cout << "------------------------------------" << endl;
		}
	}

	/* Saving the Model */
	for(int i=0; i<MD_NUM; i++)
	{
		FILE* fp;
		stringstream filename;
		filename << "model_0" << (i+1) << ".txt";
		string tmp = filename.str();
		fp = fopen(tmp.c_str(), "w");
		dumpHMM(fp, &hmm[i]);
		fclose(fp);
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

void alpha_calculate(vector< vector<double> >& alpha, vector<int>& seq, HMM *hmm)
{
	int seq_len = seq.size();

	for(int i=0; i<STATE_NUM; i++)				// i is STATE
		alpha[i][0] = hmm->initial[i] * hmm->observation[i][seq[0]];

	
	for(int t=1; t<seq_len; t++)				// t is TIME
	{
		for(int i=0; i<STATE_NUM; i++)
		{
			double sum = 0;
			for(int j=0; j<STATE_NUM; j++)
				sum += alpha[j][t-1] * hmm->transition[j][i];
			alpha[i][t] = sum * hmm->observation[i][seq[t]];
		}
	}
}

void beta_calcaulate(vector< vector<double> >& beta, vector<int>& seq, HMM *hmm)
{
	int seq_len = seq.size();

	for(int i=0; i<STATE_NUM; i++)				//i is STATE
		beta[i][seq_len-1] = 1;

	for(int t=seq_len-2; t>=0; t--)
	{
		for(int i=0; i<STATE_NUM; i++)
		{	
			double sum = 0;
			for(int j=0; j<STATE_NUM; j++)
				sum += hmm->transition[i][j] * hmm->observation[j][seq[t+1]] * beta[j][t+1];
			beta[i][t] = sum;
		}	
	}
}

void epsilon_calculate(vector< vector< vector<double> > >& ep, vector< vector<double> >& alpha, vector< vector<double> >& beta, vector< vector<double> >& ijmap, vector<int>& seq, HMM *hmm)
{
	int seq_len = seq.size();

	for(int t=0; t<(seq_len-1); t++)
	{
		for(int i=0; i<STATE_NUM; i++)
		{
			for(int j=0; j<STATE_NUM; j++)
				ijmap[i][j] = alpha[i][t] * hmm->transition[i][j] * hmm->observation[j][seq[t+1]] * beta[j][t+1];
		}

		double deno = vector_sum(ijmap);

		for(int i=0; i<STATE_NUM; i++)
		{
			for(int j=0; j<STATE_NUM; j++)
				ep[t][i][j] = ijmap[i][j]/deno;
		}
	}
}

void gamma_calculate(vector< vector<double> >& gamma, vector< vector<double> >& alpha, vector< vector<double> >& beta, vector<int>& seq)
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

void update_hmm(HMM *hmm, vector< vector<int> >& seq)
{
	int sample_size = seq.size();
	int seq_len = seq[0].size();

	vector<vector<double> > ijmap(STATE_NUM, vector<double>(STATE_NUM));
	vector<vector<vector<double> > > alpha(sample_size, vector<vector<double> >(STATE_NUM, vector<double>(seq_len)));
	vector<vector<vector<double> > > beta(sample_size, vector<vector<double> >(STATE_NUM, vector<double>(seq_len)));
	vector<vector<vector<double> > > gamma(sample_size, vector<vector<double> >(STATE_NUM, vector<double>(seq_len)));
	vector<vector<vector<vector<double> > > > epsilon(sample_size, vector<vector<vector<double> > >((seq_len-1), vector<vector<double> >(STATE_NUM, vector<double>(STATE_NUM))));

	for(int i=0; i<sample_size; i++)
	{
		alpha_calculate(alpha[i], seq[i], hmm);
		beta_calcaulate(beta[i], seq[i], hmm);
		epsilon_calculate(epsilon[i], alpha[i], beta[i], ijmap, seq[i], hmm);
		gamma_calculate(gamma[i], alpha[i], beta[i], seq[i]);
	}

	/* UPDATE PI*/
	double sum_pi[STATE_NUM] = {0};

	for(int i=0; i<sample_size; i++)
	{
		// print_vec(gamma[i]);
		for(int state=0; state<STATE_NUM; state++)
		{
			sum_pi[state] += gamma[i][state][0];
		}
	}

	for(int state=0; state<STATE_NUM; state++)
		hmm->initial[state] = sum_pi[state]/sample_size;
	/* UPDATE PI END */


	/* UPDATE Aij */
	for(int i=0; i<STATE_NUM; i++)
	{
		for(int j=0; j<STATE_NUM; j++)
		{
			double deno=0;
			double nume=0;
			for(int s=0; s<sample_size; s++)
			{
				for(int t=0; t<(seq[s].size()-2); t++)
				{
					deno += gamma[s][i][t];
					nume += epsilon[s][t][i][j];
				}
			}
			hmm->transition[i][j] = nume/deno;
		}
	}
	/* UPDATE Aij END*/

	/* UPDATE OVSERVATION*/
	for(int i=0; i<STATE_NUM; i++)
	{
		double deno = 0;
		double nume[MAX_OBSERV] = {0};
		for(int n=0; n<sample_size; n++)
		{
			for(int t=0; t<seq[n].size(); t++)
			{
				deno += gamma[n][i][t];
				switch(seq[n][t])
				{
					case 0:
						nume[0] += gamma[n][i][t];
						break;
					case 1:
						nume[1] += gamma[n][i][t];
						break;
					case 2:
						nume[2] += gamma[n][i][t];
						break;
					case 3:
						nume[3] += gamma[n][i][t];
						break;
					case 4:
						nume[4] += gamma[n][i][t];
						break;
					case 5:
						nume[5] += gamma[n][i][t];
						break;
				}
			}
		}

		for(int j=0; j<MAX_OBSERV; j++)
			hmm->observation[i][j] = nume[j]/deno;
	}
	/* UPDATE OBSERVATION END */
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
			hmm->observation[i][j] = nume[j]/deno;
		
	}
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

double vector_sum(vector< vector<double> >& vec)
{
	double sum = 0;
	for(vector< vector<double> >::iterator it1 = vec.begin(); it1 != vec.end(); ++it1)
	{
		for(vector<double>::iterator it2 = (*it1).begin(); it2 != (*it1).end(); ++it2)
			sum += (*it2);
	}
	return sum;
}

void print_vec(vector< vector<double> >& vec)
{
	for(vector< vector<double> >::iterator it1 = vec.begin(); it1 != vec.end(); ++it1)
	{
		for(vector<double>::iterator it2 = (*it1).begin(); it2 != (*it1).end(); ++it2)
			cout << (*it2) << ' ';
		cout << endl;
	}
}

void print_arr(double **arr, int row, int col)
{
	for(int i=0; i<row; i++)
	{
		for(int j=0; j<col; j++)
			cout << arr[i][j] << ' ';
		cout << endl;
	}
}