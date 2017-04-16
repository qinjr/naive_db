//
//  page.hpp
//  SimpleDataBase
//
//  Created by qjr on 16/7/9.
//  Copyright © 2016年 qjr. All rights reserved.
//  Definition of the class page, which is actually the node of the b tree

#ifndef page_hpp
#define page_hpp

typedef int Offset;//偏移量
#define ORDER 8 //the order of the b+ tree

#include <stdio.h>
#include <vector>
#include <string>//keys are strings
#include <cstdlib>//str functions
#include <iostream>

using namespace std;

class page {
private:
    int keyNum;//the number of keys the page has
    Offset writePos;//write position
    Offset parent;//position of the parent
    char type = 'R';//only use it in delete func
public:
    vector<string> keyVec;
    vector<Offset> valuePosVec;//value positions
    vector<Offset> dataVec;
    
    page():keyNum(0), writePos(0), parent(0), keyVec(), valuePosVec(), dataVec(){dataVec.push_back(0);}//default constructor

    int getKeyNum() {return keyNum;}
    int setKeyNum(int n) {keyNum = n;return 1;}
    void addKeyNum() {keyNum++;}
    void minusKeyNum() {keyNum--;}
    
    Offset getWritePos() {return writePos;}
    int setWritePos(Offset p) {writePos = p;return 1;}
    
    Offset getParent() {return parent;}
    int setParent(Offset p) {parent = p;return 1;}
    
    int insert(string key, Offset offset);
    
    //has key or not
    bool hasKey(string key);
    
    //get value position
    Offset getValuePos(string key);
    
    //get key's rank
    int getKeyRank(string key);
    
    //delete the element in position
    void deletePos(int rank);
    
    //remove the page's content to target
    void remove(page& target);
    
    //get type in deletion
    char getType() {return type;}
    
};


#endif /* page_hpp */
