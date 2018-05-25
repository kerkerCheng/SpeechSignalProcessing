#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "Ngram.h"
#include "map.h"

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
void read_lm();


int main(int argc, char *argv[])
{ 

    vector< vector<zh> > test_data;
    read_test_data("testdata/1_seg.txt", test_data);
    vector<zh> zhuyin;
    generate_zhuyin_list(zhuyin);
    
    map<zh, vector<zh> > ZB_map;
    read_Zhuyin_Big5_map("ZhuYin-Big5.map", ZB_map, zhuyin);






    // print_vector(test_data);




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
    string zhuyins = "ㄅㄆㄇㄈㄉㄊㄋㄌㄍㄎㄏㄐㄑㄒㄓㄔㄕㄖㄗㄘㄙㄧㄨㄩㄚㄛㄜㄝㄞㄟㄠㄡㄢㄣㄤㄥㄦ";
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

void read_lm()
{

}