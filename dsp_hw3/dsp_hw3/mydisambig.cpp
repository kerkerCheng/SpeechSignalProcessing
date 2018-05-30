#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <algorithm>
#include "Ngram.h"

using namespace std;

struct zh
{
    char zhi[2];
    
    zh() {};
    
    zh(char first_byte, char second_byte){
        zhi[0] = first_byte;
        zhi[1] = second_byte;
    }

    zh(string word){
        zhi[0] = word[0];
        zhi[1] = word[1];
    }

    bool operator==(const zh& rhs) const{
        return (zhi[0]==rhs.zhi[0]) && (zhi[1]==rhs.zhi[1]);
    }

    bool operator<(const zh& rhs) const{
        if(zhi[0] < rhs.zhi[0])
            return true;
        if(zhi[0] > rhs.zhi[0])
            return false;

        return (zhi[1] < rhs.zhi[1]);
    }
};

void print_vector(vector< vector<string> >& txt);
void print_ZB_map(map<string, vector<string> >& ZB_map);
void generate_zhuyin_list(vector<string>& zhuyin);
bool zhuyin_or_not(string in, vector<string>& zhuyin);
void read_test_data(char* filename, vector< vector<string> >& data);
void read_Zhuyin_Big5_map(char* filename, map<string, vector<string> >& ZB_map, vector<string>& zhuyin);
double getBigramProb(string w1, string w2, Vocab voc, Ngram lm);
void print_sentence(vector<string>& sen);


int main(int argc, char *argv[])
{
    vector< vector<string> > test_data;
    read_test_data(argv[2], test_data);
    vector<string> zhuyin;
    generate_zhuyin_list(zhuyin);
    
    map<string, vector<string> > ZB_map;
    read_Zhuyin_Big5_map(argv[4], ZB_map, zhuyin);


    int ngram_order = 2;
    Vocab voc;
    Ngram lm(voc, ngram_order);
    {
        File lmFile(argv[6], "r" );
        lm.read(lmFile);
        lmFile.close();
    }

    vector< vector<string> >::iterator it1;
    vector<string>::iterator it2, it3;

    for(it1 = test_data.begin(); it1 != test_data.end(); ++it1)
    {
        /* Searching All possible Zh */

        int lenth_of_sentence = (*it1).size();

        vector< vector< string > > all_possible_seq;
        vector< vector< int > > trace_back;
        vector< vector< double > > delta;

        for(it2 = (*it1).begin(); it2 != (*it1).end(); ++it2)
        {
            if(zhuyin_or_not((*it2), zhuyin))
            {
                vector<string> tmp_char;
                vector<int> tmp_int;
                vector<double> tmp_double;

                for(it3 = (ZB_map[*it2]).begin(); it3 != (ZB_map[*it2]).end(); ++it3)
                {
                    tmp_char.push_back((*it3));
                    tmp_int.push_back(0);
                    tmp_double.push_back(0.0);
                }
                all_possible_seq.push_back(tmp_char);
                trace_back.push_back(tmp_int);
                delta.push_back(tmp_double);
            }
            else
            {
                vector<string> tmp_char;
                vector<int> tmp_int;
                vector<double> tmp_double;

                tmp_char.push_back((*it2));
                tmp_int.push_back(0);
                tmp_double.push_back(0.0);
                
                all_possible_seq.push_back(tmp_char);
                trace_back.push_back(tmp_int);
                delta.push_back(tmp_double);
            }
        }

        /*Initialize Viterbi Algorithm */

        string hol = "<s>";

        for(int i=0; i<all_possible_seq[0].size(); i++)
        {
            VocabIndex wid1 = voc.getIndex(hol.c_str());
            VocabIndex wid2 = voc.getIndex(all_possible_seq[0][i].c_str());

            if(wid1 == Vocab_None)  //OOV
                wid1 = voc.getIndex(Vocab_Unknown);
            if(wid2 == Vocab_None)  //OOV
                wid2 = voc.getIndex(Vocab_Unknown);

            VocabIndex context[] = { wid1, Vocab_None };
            delta[0][i] = lm.wordProb(wid2, context);
        }
        

        /* Viterbi */

        int aaa = 0;
        for(int i=1; i<all_possible_seq.size(); i++)
        {
            for(int j=0; j<all_possible_seq[i].size(); j++)
            {
                double max_prob = LogP_Zero;
                double tmp = 0;
                for(int k=0; k<all_possible_seq[i-1].size(); k++)
                {
                    {
                        VocabIndex wid1 = voc.getIndex(all_possible_seq[i-1][k].c_str());
                        VocabIndex wid2 = voc.getIndex(all_possible_seq[i][j].c_str());

                        if(wid1 == Vocab_None)  //OOV
                            wid1 = voc.getIndex(Vocab_Unknown);
                        if(wid2 == Vocab_None)  //OOV
                            wid2 = voc.getIndex(Vocab_Unknown);

                        VocabIndex context[] = { wid1, Vocab_None };
                        tmp = lm.wordProb(wid2, context) + delta[i-1][k];
                    }
                    
                    // tmp = getBigramProb(all_possible_seq[i-1][k], all_possible_seq[i][j], voc, lm) + delta[i-1][k];
                    if(tmp > max_prob)
                    {
                        max_prob = tmp;
                        trace_back[i][j] = k;
                    }
                }
                delta[i][j] = max_prob;
            }
        }

        vector<string> ans_sentence(lenth_of_sentence);
        int pre_index = 0;

        for(int i=lenth_of_sentence-1; i>=0; i--)
        {
            ans_sentence.push_back(all_possible_seq[i].at(pre_index));
            pre_index = trace_back[i][pre_index];
        }

        ans_sentence.push_back(hol);
        reverse(ans_sentence.begin(), ans_sentence.end());
        print_sentence(ans_sentence);
    }
}

void print_vector(vector< vector<string> >& txt)
{
    for(vector< vector<string> >::iterator it1 = txt.begin(); it1 != txt.end(); ++it1)
    {
        for(vector<string>::iterator it2 = (*it1).begin(); it2 != (*it1).end(); ++it2)
        {
            cout << (*it2) << "-";
        }

        cout << endl;
    }
}

void print_ZB_map(map<string, vector<string> >& ZB_map)
{
    for(map<string, vector<string> >::iterator it1 = ZB_map.begin(); it1 != ZB_map.end(); ++it1)
    {
        cout << (it1->first) << "-";
        for(vector<string>::iterator it2 = (it1->second).begin(); it2 != (it1->second).end(); ++it2)
        {
            cout << (*it2) << " ";
        }
        cout << endl;
    }
}

void generate_zhuyin_list(vector<string>& zhuyin)
{
    string zhuyins = "£t£u£v£w£x£y£z£{£|£}£~£¡£¢£££¤£¥£¦£§£¨£©£ª£¸£¹£º£«£¬£­£®£¯£°£±£²£³£´£µ£¶£·";
    stringstream ss(zhuyins);
    string tmp;
    char t[2];

    while(ss >> t[0] >> t[1])
    {
        tmp.assign(t, 2);
        zhuyin.push_back(tmp);
    }
}

bool zhuyin_or_not(string in, vector<string>& zhuyin)
{
    for(vector<string>::iterator it = zhuyin.begin(); it != zhuyin.end(); ++it)
    {
        if(in == (*it))
            return true;
    }

    return false;
}

void read_test_data(char* filename, vector< vector<string> >& data)
{
    ifstream fin;
    fin.open(filename, ios::in);
    string line;

    while(getline(fin, line))
    {
        stringstream ss(line);

        string tmp;
        char t[2];
        vector<string> line;
        while(ss >> skipws >> t[0] >> t[1])
        {
            tmp.assign(t, 2);
            line.push_back(tmp);
        }
        string eof = "</s>";
        line.push_back(eof);
        data.push_back(line);
    }
}

void read_Zhuyin_Big5_map(char* filename, map<string, vector<string> >& ZB_map, vector<string>& zhuyin)
{
    ifstream fin;
    fin.open(filename, ios::in);
    string line;

    while(getline(fin, line))
    {
        char first[2];
        first[0] = line[0];
        first[1] = line[1];

        string key;
        key.assign(first, 2);
        if(!zhuyin_or_not(key, zhuyin))
            continue;
        
        stringstream ss(line);
        char value[2];
        string tmp;
        vector<string> values;
        ss >> skipws >> value[0] >> value[1];
        ZB_map[key] = values;

        while(ss >> skipws >> value[0] >> value[1])
        {
            tmp.assign(value, 2);
            ZB_map[key].push_back(tmp);                    
        }
    }
}

double getBigramProb(string w1, string w2, Vocab voc, Ngram lm)
{
    VocabIndex wid1 = voc.getIndex(w1.c_str());
    VocabIndex wid2 = voc.getIndex(w2.c_str());

    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);
    if(wid2 == Vocab_None)  //OOV
        wid2 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = { wid1, Vocab_None };
    return lm.wordProb(wid2, context);
}

void print_sentence(vector<string>& sen)
{
    for(vector<string>::iterator it = sen.begin(); it != sen.end(); it++)
        cout << (*it) << ' ';
    
    cout << endl;
}