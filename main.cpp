//
//  main.cpp
//  SimpleDataBase
//
//  Created by qjr on 16/7/7.
//  Copyright © 2016年 qjr. All rights reserved.
//

#include <iostream>
#include "database.h"
#include <ctime>
#include <cstdlib>
#include <map>

//generate a key
string generate_key() {
    string key;
    for (int j = 0; j < (rand() % (10 - 1)) + 1; ++j)
        key.push_back(char((rand() % (122 - 48)) + 48));
    return key;
}

//generate a value
string generate_value() {
    string value;
    for (int k = 0; k < (rand() % (28 - 1)) + 1; ++k)
        value.push_back(char((rand() % (122 - 48)) + 48));
    return value;
}

//test correctness
void test_correctness() {
    cout << "TEST CORRECTNESS 1:" << endl;
    clock_t t1, t2;
    double totaltime = 0;
    
    string pathname = "/Users/qjr/Desktop/database_co";
    DBHANDLE db;
    if ((db = db_open(pathname, CREATE)) == NULL)
        cout << "OPEN FAILED" << endl;
    db_store(db, "1", "1", DB_INSERT);
    cout << db_fetch(db, "1") << endl;
    db_store(db, "1", "aaasdf", DB_INSERT);
    cout << db_fetch(db, "1") << endl;
    db_delete(db, "1");
    cout << db_fetch(db, "1") << endl;
    db_delete(db, "1");
    cout << db_fetch(db, "1") << endl;
    db_store(db, "1", "asdf", DB_REPLACE);
    cout << db_fetch(db, "1") << endl;
    map<string, string> test;
    string key;
    string value;
    t1 = clock();
    while (test.size() <= 500000) {
        key = generate_key();
        value = generate_value();
        test[key] = value;
        key.clear();
        value.clear();
    }
    t2 = clock();
    totaltime = (double)(t2 - t1)/CLOCKS_PER_SEC;
    cout << "data amount: " << test.size() << endl;
    cout << "map insertion time: " << totaltime << "s" << endl;
    
    //data base insertion and its time
    t1 = clock();
    for (map<string, string>::iterator it = test.begin(); it != test.end(); ++it)
        db_store(db, it->first, it->second, DB_STORE);
    t2 = clock();
    totaltime = (double)(t2 - t1)/CLOCKS_PER_SEC;
    cout << "database insertion time: " << totaltime << "s" << endl;
    cout << "insert time average: " << totaltime / test.size() << endl;
    
    //insertion correctness and average fetch time
    int badInsertionNum = 0;
    vector<double> fetchTime;
    string data;
    for (map<string, string>::iterator it = test.begin(); it != test.end(); ++it) {
        t1 = clock();
        data = db_fetch(db, it->first);
        t2 = clock();
        fetchTime.push_back((double)(t2 - t1)/CLOCKS_PER_SEC);
        if (it->second != data) {
            badInsertionNum++;
        }
    }
    cout << "Insertion Error: " << badInsertionNum << endl;
    
    double timeSum = 0;
    for (double d:fetchTime)
        timeSum += d;
    cout << "fetch time average: " << timeSum / fetchTime.size() << "s" << endl;
    
    //delete time and correctness
    int randomDelete = 0;
    vector<string> deletedKey;
    vector<double> deleteTime;
    double dtimeSum = 0;
    for (map<string, string>::iterator it = test.begin(); it != test.end(); ++it) {
        randomDelete = rand() % 5;
        if (randomDelete != 0) {
            t1 = clock();
            db_delete(db, it->first);
            t2 = clock();
            deleteTime.push_back((double)(t2 - t1)/CLOCKS_PER_SEC);
            deletedKey.push_back(it->first);
        }
    }
    
    int badDeleteNum = 0;
    for (int i = 0; i < deletedKey.size(); ++i) {
        if (db_fetch(db, deletedKey[i]) != "NOT FIND KEY")
            badDeleteNum++;
    }
    cout << "Deletion Error: " << badDeleteNum << endl;
    
    for (double d:deleteTime)
        dtimeSum += d;
    cout << "delete time average: " << dtimeSum / deleteTime.size() << "s" << endl;
    
    
    db_close(db);
}


void test_performance() {
    cout << "TEST PERFORMANCE:" << endl;
    clock_t t1,t2;
    double totaltime;
    t1=clock();
    
    string pathname = "/Users/qjr/Desktop/database_pr";
    DBHANDLE db;
    if ((db = db_open(pathname, CREATE)) == NULL)
        cout << "OPEN FAILED" << endl;
    srand((unsigned)time(0));
    string key;
    string value;
    map<string, string> keyMap;
    
    int nrec = 100000;
    //write nrec to database
    for(int i = 0; i < nrec; ++i) {
        key = generate_key();
        value = generate_value();
        db_store(db, key, value, DB_STORE);
        keyMap[key] = value;
    }
    
    //read nrec from database
    for (int i = 0; i < nrec; ++i)
        db_fetch(db, generate_key());
    
    //nrec * 5
    for (int i = 0; i < 5 * nrec; ++i) {
        db_fetch(db, generate_key());
        if (i % 37 == 0)
            db_delete(db, generate_key());
        if (i % 11 == 0) {
            key = generate_key();
            value = generate_value();
            db_store(db, key, value, DB_STORE);
            keyMap[key] = value;
            db_fetch(db, key);
        }
        if (i % 17 == 0) //need to reconsider
            db_store(db, generate_key(), generate_value(), DB_REPLACE);
    }
    
    //delete all the records
    for (map<string, string>::iterator it = keyMap.begin(); it != keyMap.end(); ++it) {
        db_delete(db, it->first);
        for (int i = 0; i < 10; ++i)
            db_fetch(db, generate_key());
    }
    
    t2 = clock();
    totaltime = (double)(t2 - t1)/CLOCKS_PER_SEC;
    cout << "nrec: " << nrec << endl;
    cout << "totaltime: " << totaltime << "s" << endl;
    db_close(db);
}

int main(int argc, const char * argv[]) {
    srand((unsigned)time(0));
    test_correctness();
    //test_performance();
    return 0;
}
