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

void print_vector(vector< vector<zh> >& txt);
void print_ZB_map(map<zh, vector<zh> >& ZB_map);
void generate_zhuyin_list(vector<zh>& zhuyin);
bool zhuyin_or_not(zh in, vector<zh>& zhuyin);
void read_test_data(char* filename, vector< vector<zh> >& data);
void read_Zhuyin_Big5_map(char* filename, map<zh, vector<zh> >& ZB_map, vector<zh>& zhuyin);
double getBigramProb(zh w1, zh w2, Vocab voc, Ngram lm);
double getUnigramProb(zh w1, Vocab voc, Ngram lm, double unk_Prob_log);
void print_sentence(vector<zh>& sen);


int main(int argc, char *argv[])
{ 

    vector< vector<zh> > test_data;
    read_test_data("testdata/1_seg.txt", test_data);
    vector<zh> zhuyin;
    generate_zhuyin_list(zhuyin);
    
    map<zh, vector<zh> > ZB_map;
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

    vector< vector<zh> >::iterator it1;
    vector<zh>::iterator it2, it3;
    vector< vector< tuple<zh, double, int> > >::iterator it4;
    vector< tuple<zh, double, int> >::iterator it5, it6;

    for(it1 = test_data.begin(); it1 != test_data.end(); ++it1)
    {
        int lenth_of_sentence = (*it1).size();
        vector< vector< tuple<zh, double, int> > > all_possible_seq;
        for(it2 = (*it1).begin(); it2 != (*it1).end(); ++it2)
        {
            vector< tuple<zh, double, int> > possibles;
            if(zhuyin_or_not(*it2, zhuyin))
            {
                for(it3=ZB_map[(*it2)].begin(); it3 != ZB_map[(*it2)].end(); ++it3)
                {
                    double uni_prob = getUnigramProb((*it3), voc, lm, unk_Prob_log);
                    if(uni_prob == unk_Prob_log)
                        continue;

                    tuple<zh, double, int> tmp;
                    get<0>(tmp) = (*it3);
                    get<1>(tmp) = uni_prob;
                    possibles.push_back(tmp);
                }
            }
            else
            {
                tuple<zh, double, int> tmp;
                get<0>(tmp) = (*it2);
                get<1>(tmp) = getUnigramProb((*it2), voc, lm, unk_Prob_log);
                possibles.push_back(tmp);
            }

            all_possible_seq.push_back(possibles);
        }

        cout << "·F" << endl;
        /////////////////////////////////////////////////////////////////////////

        for(it4 = all_possible_seq.begin(); it4 != all_possible_seq.end(); ++it4)
        {
            if(it4 == all_possible_seq.begin())
            {
                for(it5 = (*it4).begin(); it5 != (*it4).end(); ++it5)
                {
                    // get<1>(*it5) = getUnigramProb(get<0>(*it5), voc, lm);
                    get<2>(*it5) = 0;
                }
            }
            else if(it4 != all_possible_seq.begin())
            {
                for(it5 = (*it4).begin(); it5 != (*it4).end(); ++it5)
                {
                    double max_log_prob = LogP_Zero;
                    
                    get<2>(*it5) = 0;           //initialize

                    for(it6 = (*(it4-1)).begin(); it6 != (*(it4-1)).end(); ++it6)
                    {
                        double tmp = get<1>(*it6) + getBigramProb(get<0>(*it6), get<0>(*it5), voc, lm);
                        if(tmp > max_log_prob)
                        {
                            max_log_prob = tmp;
                            int index_pre = it6 - (*(it4-1)).begin();
                            get<2>(*it5) = index_pre;
                        }
                    }
                    get<1>(*it5) = max_log_prob + get<1>(*it5);
                }
            }
        }

        ////////////////////////////////////////////////////////

        // double max_last_log_prob = LogP_Zero;
        // int index_last;
        // vector<zh> ans_sentence(all_possible_seq.size());

        // for(it5 =  all_possible_seq.back().begin(); it5!=all_possible_seq.back().end(); it5++)
        // {
        //     if(get<1>(*it5) > max_last_log_prob)
        //     {
        //         max_last_log_prob = get<1>(*it5);
        //         index_last = it5 - all_possible_seq.back().begin();
        //     }
        // }

        // ans_sentence.back() = get<0>(all_possible_seq.back()[index_last]);
        // int pre_index = get<2>(all_possible_seq.back()[index_last]);

        // for(int i=ans_sentence.size()-2; i>=0; i--)
        // {
        //     ans_sentence[i] = get<0>(all_possible_seq[i][pre_index]);
        //     pre_index = get<2>(all_possible_seq[i][pre_index]);
        // }

        // print_sentence(ans_sentence);

    }








    // print_vector(test_data);
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

void print_ZB_map(map<zh, vector<zh> >& ZB_map)
{
    for(map<zh, vector<zh> >::iterator it1 = ZB_map.begin(); it1 != ZB_map.end(); ++it1)
    {
        cout << (it1->first).zhi[0] << (it1->first).zhi[1] << "---------------";
        for(vector<zh>::iterator it2 = (it1->second).begin(); it2 != (it1->second).end(); ++it2)
        {
            cout << (*it2).zhi[0] << (*it2).zhi[1] << " ";
        }
        cout << endl;
    }
}

void generate_zhuyin_list(vector<zh>& zhuyin)
{
    string zhuyins = "£t£u£v£w£x£y£z£{£|£}£~£¡£¢£££¤£¥£¦£§£¨£©£ª£¸£¹£º£«£¬£­£®£¯£°£±£²£³£´£µ£¶£·";
    stringstream ss(zhuyins);
    zh tmp;

    while(ss >> tmp.zhi[0] >> tmp.zhi[1])
        zhuyin.push_back(tmp);
}

bool zhuyin_or_not(zh in, vector<zh>& zhuyin)
{
    for(vector<zh>::iterator it = zhuyin.begin(); it != zhuyin.end(); ++it)
    {
        if(in == (*it))
            return true;
    }

    return false;
}

void read_test_data(char* filename, vector< vector<zh> >& data)
{
    ifstream fin;
    fin.open(filename, ios::in);
    string line;

    while(getline(fin, line))
    {
        stringstream ss(line);

        zh tmp;
        vector<zh> line;
        while(ss >> skipws >> tmp.zhi[0] >> tmp.zhi[1])
            line.push_back(tmp);
        data.push_back(line);
    }
}

void read_Zhuyin_Big5_map(char* filename, map<zh, vector<zh> >& ZB_map, vector<zh>& zhuyin)
{
    ifstream fin;
    fin.open(filename, ios::in);
    string line;

    while(getline(fin, line))
    {
        zh first(line[0], line[1]);
        if(!zhuyin_or_not(first, zhuyin))
            continue;
        
        stringstream ss(line);
        zh key, tmp;
        vector<zh> line;
        ss >> skipws >> key.zhi[0] >> key.zhi[1];
        ZB_map[key] = line;

        while(ss >> skipws >> tmp.zhi[0] >> tmp.zhi[1])
            ZB_map[key].push_back(tmp);                       
    }
}

double getBigramProb(zh w1, zh w2, Vocab voc, Ngram lm)
{
    char a[2];
    a[0] = w1.zhi[0];
    a[1] = w1.zhi[1];

    char b[2];
    b[0] = w2.zhi[0];
    b[1] = w2.zhi[1];

    string c,d;
    c.assign(a, 2);
    d.assign(b, 2);

    const char *e = c.c_str();
    const char *f = d.c_str();
    
    VocabIndex wid1 = voc.getIndex(e);
    VocabIndex wid2 = voc.getIndex(f);

    if(wid1 == Vocab_None)  //OOV
    {
        return LogP_Zero;
        wid1 = voc.getIndex(Vocab_Unknown);
    }
    if(wid2 == Vocab_None)  //OOV
    {
        return LogP_Zero;
        wid2 = voc.getIndex(Vocab_Unknown);
    }

    VocabIndex context[] = { wid1, Vocab_None };
    // cout << "bigram£¸" << e << "£¸" << f << "£¸" << lm.wordProb(wid2, context) << endl;
    return lm.wordProb( wid2, context);
}

double getUnigramProb(zh w1, Vocab voc, Ngram lm, double unk_Prob_log)
{
    char a[2];
    a[0] = w1.zhi[0];
    a[1] = w1.zhi[1];

    string b;
    b.assign(a, 2);

    const char* c = b.c_str();

    VocabIndex wid = voc.getIndex(c);

    if(wid == Vocab_None)
    {
        return unk_Prob_log;
        wid = voc.getIndex(Vocab_Unknown);
    }

    VocabIndex context[] = {Vocab_None};
    // cout << "unigram£¸" << c << "£¸" <<lm.wordProb(wid, context) << endl;
    return lm.wordProb(wid, context);
}

void print_sentence(vector<zh>& sen)
{
    for(vector<zh>::iterator it = sen.begin(); it != sen.end(); it++)
        cout << (*it).zhi << ' ';
    
    cout << endl;
}