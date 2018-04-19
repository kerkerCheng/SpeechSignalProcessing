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

#ifndef TEST_NUM
#	define TEST_NUM		2
#endif

using namespace std;

double viterbi(double** del, HMM* hmm, vector<int>& seq);
void print_arr(double **arr, int row, int col);

int main(int argc, char *argv[])
{
	ifstream seq_file;
	seq_file.open(argv[2]);

	vector< vector<int> > seq;
	string line;

	while(getline(seq_file, line))
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
		seq.push_back(one_line);
	}
	

	HMM hmm[5];
	load_models(argv[1], hmm, 5);
	// dump_models(hmm, 5);


	/*Initialize*/
	double **del = new double*[STATE_NUM];
	for(int i=0; i<STATE_NUM; i++)
		del[i] = new double[SEQ_NUM];

	FILE* fp_result;
	fp_result = fopen(argv[3], "w");

	for(vector<vector<int> >::iterator it1 = seq.begin(); it1 != seq.end(); ++it1)
	{
		double likelihood = 0;
		int which_model;
		for(int md=0; md<MD_NUM; md++)
		{
			double tmp = viterbi(del, &hmm[md], (*it1));
			if(tmp>likelihood)
			{
				likelihood = tmp;
				which_model = md+1;
			}
		}
		fprintf(fp_result, "model_0%d.txt %e\n", which_model, likelihood);
	}
	fclose(fp_result);

	return 0;
}

double viterbi(double **del, HMM* hmm, vector<int>& seq)
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
				double tmp = del[i][t-1] * hmm->transition[i][j];
				if(tmp > max)
				{
					max = tmp;
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

void print_arr(double **arr, int row, int col)
{
	for(int i=0; i<row; i++)
	{
		for(int j=0; j<col; j++)
			cout << arr[i][j] << ' ';
		cout << endl;
	}
}