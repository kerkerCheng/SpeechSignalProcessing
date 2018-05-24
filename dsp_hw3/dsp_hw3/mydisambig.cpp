#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <locale>
#include <codecvt>
#include "Ngram.h"

using namespace std;

struct zh
{
    char zhi[2];
};

void print_vector(vector< vector<zh> >& txt);


int main(int argc, char *argv[])
{
    // const locale big5_locale = locale(locale(), )

    ifstream fin;
    fin.open("testdata/1_seg.txt", ios::in);
    
    vector< vector<zh> > test_data;
    string line;

    while(getline(fin, line))
    {
        stringstream ss(line);

        zh tmp;
        vector<zh> line;
        while(ss >> skipws >> tmp.zhi[0] >> tmp.zhi[1])
            line.push_back(tmp);
        test_data.push_back(line);
    }

    print_vector(test_data);




    // int ngram_order = 3;
    // Vocab voc;
    // Ngram lm( voc, ngram_order );

    // {
    //     const char lm_filename[] = "./bigram.lm";
    //     File lmFile( lm_filename, "r" );
    //     lm.read(lmFile);
    //     lmFile.close();
    // }

    // char *a = "我";
    // cout << a[0] << a[1] << endl;


    // VocabIndex wid = voc.getIndex(a);
    // printf("aaaaa : %d\n", wid);
    // if(wid == Vocab_None) {
    //     printf("No word with wid = %d\n", wid);
    //     printf("where Vocab_None is %d\n", Vocab_None);
    // }

    // wid = voc.getIndex("一");
    // VocabIndex context[] = {voc.getIndex("一") , voc.getIndex("一"), Vocab_None};
    // printf("log Prob(一|一-一) = %f\n", lm.wordProb(wis, context));
}

void print_vector(vector< vector<zh> >& txt)
{
    for(vector< vector<zh> >::iterator it1 = txt.begin(); it1 != txt.end(); ++it1)
    {
        for(vector<zh>::iterator it2 = (*it1).begin(); it2 != (*it1).end(); ++it2)
        {
            cout << (*it2).zhi[0] << (*it2).zhi[1] << "-";
        }

        cout << endl;
    }
}