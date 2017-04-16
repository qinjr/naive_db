//
//  data.hpp
//  SimpleDataBase
//
//  Created by qjr on 16/7/9.
//  Copyright © 2016年 qjr. All rights reserved.
//  Definition of the class data which is used to control the data file

#ifndef data_hpp
#define data_hpp

#define MAX_REC_SPACE 30

#include <stdio.h>
#include <fstream>
#include <string>
#include <cstdlib>
#include "page.hpp"


using namespace std;

class data {
private:
    fstream fs;
public:
    data():fs(){}//default constructor
    void data_open(string pathname, int oflag);//open file
    void data_close(){fs.close();}//close file
    Offset insert(string value);//insert the data to the data file and return the offset
    Offset insert(string value, Offset writepos);//insert the data to the file according to its write position
    string showData(Offset offset);//show data
    void deleteRec(Offset offset);//delete data
    void rewind(string pathname);//clear the data file
    
};

class Bad_data{};

#endif /* data_hpp */
