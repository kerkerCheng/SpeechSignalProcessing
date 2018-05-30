#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <tuple>
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
double getUnigramProb(string w1, Vocab voc, Ngram lm, double unk_Prob_log);
void print_sentence(vector<string>& sen);


int main(int argc, char *argv[])
{

    vector< vector<string> > test_data;
    read_test_data("testdata/1_seg.txt", test_data);
    vector<string> zhuyin;
    generate_zhuyin_list(zhuyin);
    
    map<string, vector<string> > ZB_map;
    read_Zhuyin_Big5_map("ZhuYin-Big5.map", ZB_map, zhuyin);


    int ngram_order = 2;
    Vocab voc;
    Ngram lm(voc, ngram_order);
    {
        const char lm_filename[] = "./bigram.lm";
        File lmFile( lm_filename, "r" );
        lm.read(lmFile);
        lmFile.close();
    }

    
    VocabIndex wid = voc.getIndex(Vocab_Unknown);
    VocabIndex context[] = {Vocab_None};
    double unk_Prob_log = lm.wordProb(wid, context);

    cout << "unk_Prob_log = " << unk_Prob_log << endl;

    vector< vector<string> >::iterator it1;
    vector<string>::iterator it2, it3;
    vector< vector< tuple<string, double, int> > >::iterator it4;
    vector< tuple<string, double, int> >::iterator it5, it6;

    for(it1 = test_data.begin(); it1 != test_data.end(); ++it1)
    {
        int lenth_of_sentence = (*it1).size();
        vector< vector< tuple<string, double, int> > > all_possible_seq;
        for(it2 = (*it1).begin(); it2 != (*it1).end(); ++it2)
        {
            vector< tuple<string, double, int> > possibles;
            if(zhuyin_or_not(*it2, zhuyin))
            {
                for(it3=ZB_map[(*it2)].begin(); it3 != ZB_map[(*it2)].end(); ++it3)
                {
                    VocabIndex wid1 = voc.getIndex((*it3).c_str());
                    if(wid1 == Vocab_None)
                        continue;

                    double uni_prob = 0;

                    if(it2 == (*it1).begin())
                        uni_prob = getUnigramProb((*it3), voc, lm, unk_Prob_log);
                        

                    tuple<string, double, int> tmp((*it3), uni_prob, 0);
                    possibles.push_back(tmp);
                }
            }
            else
            {
                double uni_prob = 0;

                if(it2 == (*it1).begin())
                    uni_prob = getUnigramProb((*it2), voc, lm, unk_Prob_log);
                
                tuple<string, double, int> tmp((*it2), uni_prob, 0);
                possibles.push_back(tmp);
            }

            all_possible_seq.push_back(possibles);
        }

        cout << "·F" << endl;
        /////////////////////////////////////////////////////////////////////////
        int num_word = 1;

        for(it4 = all_possible_seq.begin(); it4 != all_possible_seq.end(); ++it4)
        {
            if(it4 == all_possible_seq.begin())
            {
                // for(it5 = (*it4).begin(); it5 != (*it4).end(); ++it5)
                // {
                //     get<1>(*it5) = getUnigramProb(get<0>(*it5), voc, lm);
                //     get<2>(*it5) = 0;
                // }
                cout << "the word num = " << num_word << endl;
            }
            else if(it4 != all_possible_seq.begin())
            {
                for(it5 = it4->begin(); it5 != it4->end(); ++it5)
                {
                    double max_log_prob = LogP_Zero;
                    
                    // get<2>(*it5) = 0;           //initialize

                    for(it6 = (it4-1)->begin(); it6 != (it4-1)->end(); ++it6)
                    {
                        double tmp = getBigramProb(get<0>(*it6), get<0>(*it5), voc, lm) + get<1>(*it6);
                        if(tmp > max_log_prob)
                        {
                            max_log_prob = tmp;
                            get<2>(*it5) = it6 - (it4-1)->begin();
                        }
                    }
                    get<1>(*it5) = max_log_prob;
                }
                cout << "the word num = " << num_word << endl;
            }
            num_word ++;
        }

        cout << "·F·F" << endl;

        ////////////////////////////////////////////////////////

        double max_last_log_prob = LogP_Zero;
        int index_last;
        vector<string> ans_sentence(all_possible_seq.size());

        for(it5 =  all_possible_seq.back().begin(); it5!=all_possible_seq.back().end(); it5++)
        {
            if(get<1>(*it5) > max_last_log_prob)
            {
                max_last_log_prob = get<1>(*it5);
                index_last = it5 - all_possible_seq.back().begin();
            }
        }

        ans_sentence.back() = get<0>(all_possible_seq.back()[index_last]);
        int pre_index = get<2>(all_possible_seq.back()[index_last]);

        for(int i=ans_sentence.size()-2; i>=0; i--)
        {
            ans_sentence[i] = get<0>(all_possible_seq[i][pre_index]);
            pre_index = get<2>(all_possible_seq[i][pre_index]);
        }

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
        cout << (it1->first) << "---------------";
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

    // if(wid1 == Vocab_None)  //OOV
    // {
    //     return LogP_Zero;
    //     wid1 = voc.getIndex(Vocab_Unknown);
    // }
    // if(wid2 == Vocab_None)  //OOV
    // {
    //     return LogP_Zero;
    //     wid2 = voc.getIndex(Vocab_Unknown);
    // }

    VocabIndex context[] = { wid1, Vocab_None };
    cout << "bigram£¸" << w1 << "£¸" << w2 << "£¸" << endl;
    return lm.wordProb(wid2, context);
}

double getUnigramProb(string w1, Vocab voc, Ngram lm, double unk_Prob_log)
{
    VocabIndex wid = voc.getIndex(w1.c_str());

    if(wid == Vocab_None)
    {
        return unk_Prob_log;
        wid = voc.getIndex(Vocab_Unknown);
    }

    VocabIndex context[] = {Vocab_None};
    // cout << "unigram£¸" << c << "£¸" <<lm.wordProb(wid, context) << endl;
    return lm.wordProb(wid, context);
}

void print_sentence(vector<string>& sen)
{
    for(vector<string>::iterator it = sen.begin(); it != sen.end(); it++)
        cout << (*it) << ' ';
    
    cout << endl;
}