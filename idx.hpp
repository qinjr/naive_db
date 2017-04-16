//
//  idx.hpp
//  SimpleDataBase
//
//  Created by qjr on 16/7/9.
//  Copyright © 2016年 qjr. All rights reserved.
//  Definition of the class idx(index) which controls the index file

#ifndef idx_hpp
#define idx_hpp

#define MAX_KEY_LENGTH 13 //including '\0'
#define MAX_SPACE 12 + (ORDER - 1) * MAX_KEY_LENGTH + (2 * ORDER - 1) * sizeof(Offset) //max space for a page

#include <stdio.h>
#include <fstream>
#include "page.hpp"
#include <iostream>
#include <cmath>//pow
using namespace std;

class Bad_idx{};

class idx {
private:
    fstream fs;
    Offset rootPos;//offset position of the root page
    bool empty;
public:
    idx():fs(){rootPos = 0;empty = true;}//default constructor
    void idx_open(string pathname, int oflag);//open index file
    void idx_close(){fs.close();}//
    
    int insert(string key, int offset);//DB_INSERT
    int replace(string key, int offset);//DB_REPLACE
    int store(string key, int offset);//DB_STORE
    
    int deleteKey(string key, page target);//delete the key from the b tree
    
    void rewind(string pathname);//clear the index file
    
    void chgempty(bool emptyset) {empty = emptyset;}//change empty
    bool getEmpty() {return empty;}
    
    page search(string key);
    
    void solveOverflow(page target);
    void solveUnderflow(page target);
    
};


#endif /* idx_hpp */
