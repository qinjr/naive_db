//
//  idx.cpp
//  SimpleDataBase
//
//  Created by qjr on 16/7/9.
//  Copyright © 2016年 qjr. All rights reserved.
//

#include "idx.hpp"


/*
 *func:intTochar
 *usage:get every byte of the int
 */
char intTochar(int integer, int no) {//32-bit integer transfer to 4-char string
    switch (no) {
        case 1:
            return char((integer & 4278190080) >> 24);
        case 2:
            return char((integer & 16711680) >> 16);//can't be that much data!
        case 3:
            return char((integer & 65280) >> 8);
        case 4:
            return char(integer & 255);
            
        default:
            return 'a';
            break;
    }
}

/*
 *func:charToint
 *usage:transfer the 4-bytes char string to an int
 */
int charToint(char* pc) {
    char* ptr = pc;
    int result = 0;
    for (int i = 1; i <= 4; ++i) {
        result += (int((unsigned char)(*ptr)) << (8 * (4 - i)));
        ++ptr;
    }
    return result;
}

/*
 *func:writePage
 *usage:write the page into index file
 *warning:consider the consistence
 */
void writePage(page& writeIn, fstream& fs) {
    //set the write position of the page, this position belongs to the page
    fs.seekp(writeIn.getWritePos());
    
    //pc is the ptr to the head address of all the content that the page has
    char pc[MAX_SPACE];
    for (int i = 0; i < MAX_SPACE; ++i)
        pc[i] = '\0';
    vector<char> content;
    
    //add keyNum and writePos and parent
    for (int i = 1; i <= 4; ++i)
        content.push_back(intTochar(writeIn.getKeyNum(), i));
    for (int i = 1; i <= 4; ++i)
        content.push_back(intTochar(writeIn.getWritePos(), i));
    for (int i = 1; i <= 4; ++i)
        content.push_back(intTochar(writeIn.getParent(), i));
    
    //deal with the 3 vectors
    for (int i = 0; i < writeIn.keyVec.size(); ++i) {
        for (int j = 0; j < writeIn.keyVec[i].size(); ++j)
            content.push_back(char(writeIn.keyVec[i][j]));
        content.push_back('\0');
    }
    
    for (int i = 0; i < writeIn.valuePosVec.size(); ++i) {
        for (int j = 1; j <= 4; ++j)
            content.push_back(intTochar(writeIn.valuePosVec[i], j));
    }
    
    for (int i = 0; i < writeIn.dataVec.size(); ++i) {
        for (int j = 1; j <= 4; ++j)
            content.push_back(intTochar(writeIn.dataVec[i], j));
    }
    
    //build pc
    for (int i = 0; i <= content.size() - 1; ++i)
        pc[i] = content[i];
    
    //write the page into the file
    fs.write(pc, sizeof(pc));
}

/*
 *func:readPage
 *usage:read a page from the index file and rebuild the page accordingly
 */
page readPage(Offset offset, fstream& fs) {
    static char pc[MAX_SPACE];//need to be static
    fs.seekg(offset);
    fs.read(pc, MAX_SPACE);
    page new_page;
    new_page.dataVec.clear();//erase the first 0 in data vector
    
    char* ptr = pc;
    
    //set keyNum and writePos and parent
    char ckeyNum[4];
    for (int i = 0; i <= 3; ++i) {
        ckeyNum[i] = *ptr;
        ++ptr;
    }
    int keyNum = charToint(ckeyNum);
    new_page.setKeyNum(keyNum);
    
    char cwritePos[4];
    for (int i = 0; i <= 3; ++i) {
        cwritePos[i] = *ptr;
        ++ptr;
    }
    int writePos = charToint(cwritePos);
    new_page.setWritePos(writePos);
    
    char cparent[4];
    for (int i = 0; i <= 3; ++i) {
        cparent[i] = *ptr;
        ++ptr;
    }
    int parent = charToint(cparent);
    new_page.setParent(parent);
    
    //set keyVec
    --ptr;
    for (int i = 1; i <= keyNum; ++i) {
        ++ptr;
        char* end = ptr;
        while (*end != '\0')
            end++;
        new_page.keyVec.push_back(string(ptr, end));
        ptr = end;
    }
    
    //set valuePosVec
    ++ptr;
    for (int i = 1; i <= keyNum; ++i) {
        char cvalue[4];
        for (int i = 0; i <= 3; ++i) {
            cvalue[i] = *ptr;
            ++ptr;
        }
        new_page.valuePosVec.push_back(charToint(cvalue));
    }
    
    //set dataVec
    for (int i = 1; i <= keyNum + 1; ++i) {
        char cdata[4];
        for (int i = 0; i <= 3; ++i) {
            cdata[i] = *ptr;
            ++ptr;
        }
        new_page.dataVec.push_back(charToint(cdata));
    }
    return new_page;
}

void idx::idx_open(string pathname, int oflag) {
    if (oflag == 1) {//read only
        fs.open(pathname, ios::in | ios::binary);
        if (!fs.is_open())//inspect if the file is opened
            throw Bad_idx();
        //find and reset rootpos
        page now = readPage(0, fs);
        while (now.getParent() != 0)
            now = readPage(now.getParent(), fs);
        rootPos = now.getWritePos();
    }
    
    else if (oflag == 2) {//read and write
        fs.open(pathname, ios::in | ios:: out | ios::binary);
        if (!fs.is_open())
            throw Bad_idx();
        //find and reset rootpos
        page now = readPage(0, fs);
        while (now.getParent() != 0)
            now = readPage(now.getParent(), fs);
        rootPos = now.getWritePos();
    }
    
    else if (oflag == 3) {//create if not exist
        fs.open(pathname, ios::in);
        if (!fs.is_open()){
            fs.open(pathname, ios:: out);//create
            fs.close();
            fs.open(pathname, ios::in | ios::out | ios::binary);
        }
    }
    
    else {throw Bad_idx();}
}

/*
 *func:findPos
 *usage:find the rank of the max element that is no more than the key
 */
int findPos(vector<string>kv, string key) {//can't be an empty page
    if (kv[0] > key)
        return -1;
    if (kv[kv.size() - 1] <= key)
        return int(kv.size() - 1);
    for (int i = 0; i < kv.size() - 1; ++i) {
        if ((kv[i] <= key) && (kv[i + 1] > key))
            return i;
    }
    return -1;
}

/*
 *func:search
 *usage:find the page which the key should be
 */
page idx::search(string key) {
    page now = readPage(rootPos, fs);//get root
    while ((now.dataVec[0] != 0) | (now.dataVec[1] != 0)) {//not a leaf page
        int rank = findPos(now.keyVec, key);
        if ((rank >= 0) && (now.keyVec[rank] == key))
            return now;
        else {
            now = readPage(now.dataVec[rank + 1], fs);
        }
    }
    return now;
}

/*
 *func:ifOverflow
 *usage:if overflow return 1, if not, return 0
 */
int ifOverflow(page& page) {
    if (page.getKeyNum() <= ORDER - 1) return 0;//means no overflow
    else return 1;
}

/*
 *func:solveOverflow
 *usage:solveOverflow
 */
void idx::solveOverflow(page lhs) {
    int s = ORDER / 2;//position to split
    page rhs;
    rhs.dataVec.clear();
    
    //seperate
    for (int i = s + 1; i < lhs.keyVec.size(); ++i) {
        rhs.keyVec.push_back(lhs.keyVec[i]);
        rhs.addKeyNum();
        rhs.valuePosVec.push_back(lhs.valuePosVec[i]);
    }
    for (int i = s + 1; i < lhs.dataVec.size(); ++i) {
        rhs.dataVec.push_back(lhs.dataVec[i]);
    }
    
    

    //set writepos
    fs.seekp(0, ios::end);
    rhs.setWritePos(Offset(fs.tellp()));
    
    
    
    //don't have parent
    if (lhs.getParent() == 0) {
        page parent;
        parent.dataVec.clear();
        parent.keyVec.push_back(lhs.keyVec[s]);
        parent.addKeyNum();
        parent.valuePosVec.push_back(lhs.valuePosVec[s]);
        
        parent.dataVec.push_back(lhs.getWritePos());
        parent.dataVec.push_back(rhs.getWritePos());

        parent.setWritePos((Offset(fs.tellp()) + MAX_SPACE));
        lhs.setParent(parent.getWritePos());
        rhs.setParent(parent.getWritePos());
        
        //set children's parent position
        if ((lhs.dataVec[0] != 0) | (lhs.dataVec[1] != 0)) {//not leaf page
            for (int i = 0; i < s + 1; ++i) {
                page kid = readPage(lhs.dataVec[i], fs);
                kid.setParent(lhs.getWritePos());
                writePage(kid, fs);
            }
            for (int i = s + 1; i < lhs.getKeyNum() + 1; ++i) {
                page kid = readPage(lhs.dataVec[i], fs);
                kid.setParent(rhs.getWritePos());
                writePage(kid, fs);
            }
        }
        
        //erase lhs
        int eraseQty = int(lhs.keyVec.size()) - s;
        while (eraseQty != 0) {
            lhs.keyVec.erase(lhs.keyVec.end() - 1);//erase the last one
            lhs.minusKeyNum();//key num - 1
            lhs.valuePosVec.erase(lhs.valuePosVec.end() - 1);
            lhs.dataVec.erase(lhs.dataVec.end() - 1);
            eraseQty--;
        }
        
        
        //update the root position
        rootPos = parent.getWritePos();
        
        //write pages
        writePage(lhs, fs);
        writePage(rhs, fs);
        writePage(parent, fs);
    }
    
    //do have parent
    else {
        if ((lhs.dataVec[0] != 0) | (lhs.dataVec[1] != 0)) {//not leaf page
            for (int i = 0; i < s + 1; ++i) {
                page kid = readPage(lhs.dataVec[i], fs);
                kid.setParent(lhs.getWritePos());
                writePage(kid, fs);
            }
            for (int i = s + 1; i < lhs.getKeyNum() + 1; ++i) {
                page kid = readPage(lhs.dataVec[i], fs);
                kid.setParent(rhs.getWritePos());
                writePage(kid, fs);
            }
        }
        
        rhs.setParent(lhs.getParent());
        Offset offset = lhs.getParent();
        page parent = readPage(offset, fs);
        
        int insertPos = parent.insert(lhs.keyVec[s], lhs.valuePosVec[s]);
        parent.dataVec[insertPos + 1] = rhs.getWritePos();
        
        //erase lhs
        int eraseQty = int(lhs.keyVec.size()) - s;
        while (eraseQty != 0) {
            lhs.keyVec.erase(lhs.keyVec.end() - 1);//erase the last one
            lhs.minusKeyNum();//key num - 1
            lhs.valuePosVec.erase(lhs.valuePosVec.end() - 1);
            lhs.dataVec.erase(lhs.dataVec.end() - 1);
            eraseQty--;
        }
        
        //write lhs, rhs
        writePage(lhs, fs);
        writePage(rhs, fs);
        
        if (ifOverflow(parent) == 1) {
            solveOverflow(parent);
        }
        else {
            writePage(parent, fs);
        }
        
    }
}

/*
 *func:insert
 *usage:insert a key to index file
 */
int idx::insert(string key, int offset) {
    if (empty) {
        page newpage;
        newpage.insert(key, offset);
        writePage(newpage, fs);
        empty = false;
        return 1;
    }
    page now = search(key);
    if ((now.dataVec[0] == 0) && (now.dataVec[1] == 0)) {//indicates its a leaf page
        now.insert(key, offset);
        if (ifOverflow(now) == 0)
            writePage(now, fs);
        else {
            solveOverflow(now);
        }
        return 1;
    }
    return 0;
}

/*
 *func:exchange
 *usage:exchange between target and its successor
 */
void exchange(page& target, page& success, int rank) {
    string tems = target.keyVec[rank];
    Offset temo = target.valuePosVec[rank];
    
    target.keyVec[rank] = success.keyVec[0];
    target.valuePosVec[rank] = success.valuePosVec[0];
    
    success.keyVec[0] = tems;
    success.valuePosVec[0] = temo;
}

/*
 *func:solveUnderflow
 *usage:solve underflow, write in pages
 */
void idx::solveUnderflow(page target) {
    if (target.getKeyNum() >= (ORDER + 1) / 2 - 1) {
        writePage(target, fs);
        return;
    }
    if (target.getParent() == 0) {//root
        if ((target.getType() == 'L') && (target.getKeyNum() == 0)) {//root and leaf
            empty = true;
            return;
        }
        else if ((target.getKeyNum() == 0) && (target.getType() != 'L')) {
            rootPos = target.dataVec[0];
            page newroot = readPage(target.dataVec[0], fs);
            newroot.setParent(0);
            writePage(newroot, fs);
        }
        else {
            writePage(target, fs);
            return;
        }
    }
    
    page parent = readPage(target.getParent(), fs);
    string tems;
    int temo;
    
    //get childNo
    int childNo = 0;
    for (int i = 0; i < parent.dataVec.size(); ++i) {
        if (parent.dataVec[i] == target.getWritePos()) {
            childNo = i;
            break;
        }
    }
    
    //only has left silbling
    if (childNo == parent.dataVec.size() - 1) {
        page lsibling = readPage(parent.dataVec[childNo - 1], fs);
        if (lsibling.getKeyNum() > (ORDER + 1) / 2 - 1) {
            tems = parent.keyVec[childNo - 1];
            temo = parent.valuePosVec[childNo - 1];
            
            parent.keyVec[childNo - 1] = lsibling.keyVec[lsibling.keyVec.size() - 1];
            parent.valuePosVec[childNo - 1] = lsibling.valuePosVec[lsibling.valuePosVec.size() - 1];
            lsibling.keyVec.erase(lsibling.keyVec.end() - 1);
            lsibling.minusKeyNum();
            lsibling.valuePosVec.erase(lsibling.valuePosVec.end() - 1);
            
            target.keyVec.insert(target.keyVec.begin(), tems);
            target.addKeyNum();
            target.valuePosVec.insert(target.valuePosVec.begin(), temo);
            
            target.dataVec.insert(target.dataVec.begin(), lsibling.dataVec[lsibling.dataVec.size() - 1]);
            lsibling.dataVec.erase(lsibling.dataVec.end() - 1);
            
            if (target.getType() != 'L') {//not a leaf
                page targetChild = readPage(target.dataVec[0], fs);
                targetChild.setParent(target.getWritePos());//set new parent position
                writePage(targetChild, fs);
            }
            writePage(lsibling, fs);
            writePage(target, fs);
            writePage(parent, fs);
            return;
        }
    }
    
    //only has right silbling
    if (childNo == 0) {
        page rsibling = readPage(parent.dataVec[childNo + 1], fs);
        if (rsibling.getKeyNum() > (ORDER + 1) / 2 - 1) {
            tems = parent.keyVec[childNo];
            temo = parent.valuePosVec[childNo];
            
            parent.keyVec[childNo] = rsibling.keyVec[0];
            parent.valuePosVec[childNo] = rsibling.valuePosVec[0];
            rsibling.keyVec.erase(rsibling.keyVec.begin());
            rsibling.minusKeyNum();
            rsibling.valuePosVec.erase(rsibling.valuePosVec.begin());
            
            target.keyVec.push_back(tems);
            target.addKeyNum();
            target.valuePosVec.push_back(temo);
            
            target.dataVec.push_back(rsibling.dataVec[0]);
            rsibling.dataVec.erase(rsibling.dataVec.begin());
            if (target.getType() != 'L') {//not a leaf
                page targetChild = readPage(target.dataVec[target.dataVec.size() - 1], fs);
                targetChild.setParent(target.getWritePos());//set new parent position
                writePage(targetChild, fs);
            }
            writePage(rsibling, fs);
            writePage(target, fs);
            writePage(parent, fs);
            return;
        }
    }
    
    //has left and right sibling
    if ((childNo >=1) && (childNo <= parent.dataVec.size() - 2)) {
        page lsibling = readPage(parent.dataVec[childNo - 1], fs);
        if (lsibling.getKeyNum() > (ORDER + 1) / 2 - 1) {
            tems = parent.keyVec[childNo - 1];
            temo = parent.valuePosVec[childNo - 1];
            
            parent.keyVec[childNo - 1] = lsibling.keyVec[lsibling.keyVec.size() - 1];
            parent.valuePosVec[childNo - 1] = lsibling.valuePosVec[lsibling.valuePosVec.size() - 1];
            lsibling.keyVec.erase(lsibling.keyVec.end() - 1);
            lsibling.minusKeyNum();
            lsibling.valuePosVec.erase(lsibling.valuePosVec.end() - 1);
            
            target.keyVec.insert(target.keyVec.begin(), tems);
            target.addKeyNum();
            target.valuePosVec.insert(target.valuePosVec.begin(), temo);
            
            target.dataVec.insert(target.dataVec.begin(), lsibling.dataVec[lsibling.dataVec.size() - 1]);
            lsibling.dataVec.erase(lsibling.dataVec.end() - 1);
            
            if (target.getType() != 'L') {//not a leaf
                page targetChild = readPage(target.dataVec[0], fs);
                targetChild.setParent(target.getWritePos());//set new parent position
                writePage(targetChild, fs);
            }
            writePage(lsibling, fs);
            writePage(target, fs);
            writePage(parent, fs);
            return;
        }
        
        page rsibling = readPage(parent.dataVec[childNo + 1], fs);
        if (rsibling.getKeyNum() > (ORDER + 1) / 2 - 1) {
            tems = parent.keyVec[childNo];
            temo = parent.valuePosVec[childNo];
            
            parent.keyVec[childNo] = rsibling.keyVec[0];
            parent.valuePosVec[childNo] = rsibling.valuePosVec[0];
            rsibling.keyVec.erase(rsibling.keyVec.begin());
            rsibling.minusKeyNum();
            rsibling.valuePosVec.erase(rsibling.valuePosVec.begin());
            
            target.keyVec.push_back(tems);
            target.addKeyNum();
            target.valuePosVec.push_back(temo);
            
            target.dataVec.push_back(rsibling.dataVec[0]);
            rsibling.dataVec.erase(rsibling.dataVec.begin());
            if (target.getType() != 'L') {//not a leaf
                page targetChild = readPage(target.dataVec[target.dataVec.size() - 1], fs);
                targetChild.setParent(target.getWritePos());//set new parent position
                writePage(targetChild, fs);
            }
            writePage(rsibling, fs);
            writePage(target, fs);
            writePage(parent, fs);
            return;
        }
    }
    
    //can't borrow from sibling
    page left, right;
    int getParentPos = 0;
    if (childNo == parent.dataVec.size() - 1) {
        getParentPos = childNo - 1;
        left = readPage(parent.dataVec[childNo - 1], fs);
        right = target;
    }
    else {
        getParentPos = childNo;
        left = target;
        right = readPage(parent.dataVec[childNo + 1], fs);
    }
    
    left.keyVec.push_back(parent.keyVec[getParentPos]);
    left.addKeyNum();
    left.valuePosVec.push_back(parent.valuePosVec[getParentPos]);
    
    parent.keyVec.erase(parent.keyVec.begin() + getParentPos);
    parent.valuePosVec.erase(parent.valuePosVec.begin() + getParentPos);
    parent.minusKeyNum();
    parent.dataVec.erase(parent.dataVec.begin() + getParentPos + 1);
    
    right.remove(left);
    
    page rightChild;
    if (right.getType() != 'L') {
        for (int i = 0; i < right.dataVec.size(); ++i) {
            rightChild = readPage(right.dataVec[i], fs);
            rightChild.setParent(left.getWritePos());
            writePage(rightChild, fs);
        }
    }
    writePage(left, fs);
    
    
    //recursive
    solveUnderflow(parent);
}

/*
 *func:deleteKey
 *usage:delete the key from the tree
 */
int idx::deleteKey(string key, page target) {
    int rank = target.getKeyRank(key);
    if ((target.dataVec[0] != 0) | (target.dataVec[1] != 0)) {//not a leaf page
        page success = readPage(target.dataVec[rank + 1], fs);
        while ((success.dataVec[0] != 0) | (success.dataVec[1] != 0))//still not a leaf page
            success = readPage(success.dataVec[0], fs);
        
        //success is the leaf page, then swap
        exchange(target, success, rank);
        //write the target, now the delete key is in success
        writePage(target, fs);
        //updata target
        target = success;
        target.deletePos(0);
        solveUnderflow(target);
        return 1;
    }
    
    else {//leaf page
        target.deletePos(rank);
        solveUnderflow(target);
        return 1;//delete success
    }
}

/*
 *func:rewind
 *usage:clear the index file
 */
void idx::rewind(string pathname) {
    pathname += "(idx)";
    if (fs.is_open()) {
        fs.close();
        fs.open(pathname, ios::out);
    }
}




