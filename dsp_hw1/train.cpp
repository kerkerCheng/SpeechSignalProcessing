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

using namespace std;

int main()
{
	HMM hmm_initial;
	HMM hmm[MD_NUM];
	loadHMM( &hmm_initial, "model_init.txt");

	ifstream seq_files[MD_NUM];
	for(int i=0; i<MD_NUM; i++)
	{
		stringstream filename;
		filename << "seq_model_0" << (i+1) << ".txt";
		string tmp = filename.str();
		seq_files[i].open(tmp.c_str());
	}

	vector< vector< vector<char> > > seq;
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
				one_line.push_back(ch);
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