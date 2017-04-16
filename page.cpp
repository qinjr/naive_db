//
//  page.cpp
//  SimpleDataBase
//
//  Created by qjr on 16/7/9.
//  Copyright © 2016年 qjr. All rights reserved.
//

#include "page.hpp"

int findInsertPos(string key, vector<string>& keyVec) {
    if (keyVec.size() == 0) {
        return 0;
    }
    else if (keyVec.size() == 1) {
        if (keyVec[0] < key)
            return 1;
        else if (keyVec[0] > key)
            return 0;
        else
            return -1;//don't insert
    }
    else {
        int pos = 0;//default insert position
        for (int i = 0; i < keyVec.size(); ++i) {
            if (keyVec[i] < key)
                pos++;
            if (keyVec[i] > key)
                break;
            if (keyVec[i] == key)
                pos = -1;//don't insert
        }
        return pos;
    }
}


int page::insert(string key, Offset offset) {
    int insertPos = findInsertPos(key, keyVec);
    if (insertPos >= 0) {
        keyVec.insert(keyVec.begin() + insertPos, key);
        keyNum += 1;
        valuePosVec.insert(valuePosVec.begin() + insertPos, offset);
        dataVec.insert(dataVec.begin() + insertPos + 1, 0);
        return insertPos;
    }
    else {return 0;}
}

bool page::hasKey(string key) {
    for (int i = 0; i < keyVec.size(); ++i) {
        if (key == keyVec[i])
            return true;
    }
    return false;
}

Offset page::getValuePos(string key) {
    for (int i = 0; i < keyVec.size(); ++i) {
        if (key == keyVec[i])
            return valuePosVec[i];
    }
    return 0;
}

int page::getKeyRank(string key) {
    for (int i = 0; i < keyNum; ++i) {
        if (keyVec[i] == key)
            return i;
    }
    return -1;
}

void page::deletePos(int rank) {
    if ((dataVec[0] == 0) && (dataVec[1] == 0))
        type = 'L';
    keyVec.erase(keyVec.begin() + rank);
    minusKeyNum();
    valuePosVec.erase(valuePosVec.begin() + rank);
    dataVec.erase(dataVec.begin() + rank);
}

void page::remove(page& target) {
    for (int i = 0; i < keyVec.size(); ++i) {
        target.keyVec.push_back(keyVec[i]);
        target.addKeyNum();
        target.valuePosVec.push_back(valuePosVec[i]);
    }
    for (int i = 0; i < dataVec.size(); ++i) {
        target.dataVec.push_back(dataVec[i]);
    }
}

