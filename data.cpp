//
//  data.cpp
//  SimpleDataBase
//
//  Created by qjr on 16/7/9.
//  Copyright © 2016年 qjr. All rights reserved.
//

#include "data.hpp"
void data::data_open(string pathname, int oflag) {
    switch (oflag) {
        case 1://read only
            fs.open(pathname, ios::in | ios::binary);
            if (!fs.is_open())//inspect whether the file is open
                throw Bad_data();
            break;
        case 2://read and write
            fs.open(pathname, ios::in | ios:: out | ios::binary);
            if (!fs.is_open())
                throw Bad_data();
            break;
        case 3://create if not exist
            fs.open(pathname, ios::in | ios::binary);
            if (!fs.is_open()){
                fs.open(pathname, ios:: out);//create
                fs.close();
                fs.open(pathname, ios::in | ios::out | ios::binary);
            }
            break;
        default:
            throw Bad_data();
            break;
    }
}

Offset data::insert(string value) {
    fs.seekp(0, ios::end);
    fs.write(value.c_str(), MAX_REC_SPACE);
    return Offset(fs.tellp()) - MAX_REC_SPACE;//this is offset
}

Offset data::insert(string value, Offset writepos) {
    fs.seekp(writepos);
    fs.write(value.c_str(), MAX_REC_SPACE);
    return Offset(fs.tellp()) - MAX_REC_SPACE;//this is offset
}

string data::showData(Offset offset) {
    fs.seekg(offset);
    char pc[MAX_REC_SPACE];
    char* ptr = pc;
    fs.read(pc, MAX_REC_SPACE);
    while (*ptr != '\0')
        ptr++;
    return string(pc, ptr);
}

void data::deleteRec(Offset offset) {
    fs.seekg(offset);
    char pc[MAX_REC_SPACE];
    fs.read(pc, MAX_REC_SPACE);
    for (int i = 0; i < MAX_REC_SPACE; ++i)
        pc[i] = '\0';
}

/*
 *func:rewind
 *usage:clear the index file
 */
void data::rewind(string pathname) {
    pathname += "(dat)";
    if (fs.is_open()) {
        fs.close();
        fs.open(pathname, ios::out);
    }
}
