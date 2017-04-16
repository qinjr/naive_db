//
//  database.h
//  SimpleDataBase
//
//  Created by qjr on 16/7/9.
//  Copyright © 2016年 qjr. All rights reserved.
//

#ifndef database_h
#define database_h

#define DBHANDLE database*

//open flag;
#define READ_ONLY 1//read only
#define READ_WRITE 2//read and write
#define CREATE 3//if not exist, create the file

//store flag:
#define DB_INSERT 1//insert
#define DB_REPLACE 2//replace
#define DB_STORE 3//store

#include <stdio.h>
#include "data.hpp"
#include "idx.hpp"

using namespace std;

//database structure, DB_HANDLE is a pointer to database
struct database {
    idx idx;
    data data;
    string pathName;
    database(string pathname, int oflag);
};

database::database(string pathname, int oflag) {
    pathName = pathname;
    idx.idx_open(pathname + "(idx)", oflag);
    data.data_open(pathname + "(dat)", oflag);
}

DBHANDLE db_open(string pathname, int oflag);
void db_close(DBHANDLE db);
int db_store(DBHANDLE db, string key, string value, int flag);
string db_fetch(DBHANDLE db, string key);
int db_delete(DBHANDLE db, string key);
void db_rewind(DBHANDLE db);
string db_nextrec(DBHANDLE db, string key);

/*
 *func:DBHANDLE db_open
 *usage:return a ptr to database, if not success, return NULL
 */
DBHANDLE db_open(string pathname, int oflag) {//mode is used to create new file and determine the open mode
    try{
        DBHANDLE db = new database(pathname, oflag);
        return db;
    }
    catch(Bad_idx){
        return NULL;
    }
    catch(Bad_data){
        return NULL;
    }
}

/*
 *func:void db_close
 *usage:close the index and data file, release all the space
 */
void db_close(DBHANDLE db) {
    db->idx.idx_close();
    db->data.data_close();
    delete db;//release the space of the database
}

/*
 *func:db_fetch
 *usage:fetch the data according to the key
 */
string db_fetch(DBHANDLE db, string key) {
    if (db->idx.getEmpty())
        return string("NOT FIND KEY");
    page target = db->idx.search(key);
    Offset offset = -1;
    for (int i = 0; i < target.keyVec.size(); i++) {
        if (target.keyVec[i] == key) {
            offset = target.valuePosVec[i];
            break;
        }
    }
    if (offset == -1)
        return string("NOT FIND KEY");//indicate fetch failure
    else {
        string data = db->data.showData(offset);
        return data;
    }
}

/*
 *func:db_store
 *usage:add, replace record into the database
 */
int db_store(DBHANDLE db, string key, string value, int flag) {
    int offset = 0;
    if (flag == DB_INSERT) {
        if (db->idx.getEmpty()) {//if empty
            offset = db->data.insert(value);
            return db->idx.insert(key, offset);
        }
        page now = db->idx.search(key);
        if (now.hasKey(key))
            return 1;
        offset = db->data.insert(value);
        return db->idx.insert(key, offset);
    }
    if (flag == DB_REPLACE) {
        if (db->idx.getEmpty())
            return 0;//replace failed
        page now = db->idx.search(key);
        if (!now.hasKey(key))
            return 0;//replace failed
        db->data.insert(value, now.getValuePos(key));
        return 1;//replace successed
        
    }
    if (flag == DB_STORE) {
        if (db->idx.getEmpty()) {
            offset = db->data.insert(value);
            return db->idx.insert(key, offset);
        }
        page now = db->idx.search(key);
        if (now.hasKey(key))
            db->data.insert(value, now.getValuePos(key));
        else {
            offset = db->data.insert(value);
            return db->idx.insert(key, offset);
        }
        return 1;
    }
    return 0;
}

/*
 *func:db_delete
 *usage:delete the record according to key
 */
int db_delete(DBHANDLE db, string key) {
    if (db->idx.getEmpty())
        return 0;//delete failed
    page target = db->idx.search(key);
    if (!target.hasKey(key))
        return 0;//delete failed
    return db->idx.deleteKey(key, target);    
}

/*
 *func:db_rewind
 *usage:clear the whole database
 */
void db_rewind(DBHANDLE db) {
    db->idx.chgempty(true);
    db->idx.rewind(db->pathName);
    db->data.rewind(db->pathName);
}



#endif /* database_h */
