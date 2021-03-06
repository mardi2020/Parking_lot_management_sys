#ifndef _PARKINGLOT_H
#define _PARKINGLOT_H

#include <iostream>
#include <string.h>
#include <vector>
#include <sstream>
#include </usr/include/mysql/mysql.h>
#include "Car.h"
#define MAXSIZE 10

class Parkinglot{
public:
    // 주차장 등록용
    Parkinglot(int space, char add[], MYSQL * ptr);

    // 주차장 정보 조회용
    Parkinglot(std::string parkinglotnum, MYSQL* ptr);
    
    Parkinglot(){}

    ~Parkinglot();

    void Insert();
    void Delete(std::string pln);

    char* Getaddress(std::string pln);
    
    int* GetArray();
    //조회할때
    std::string Getspace(std::string pln);
    // db에서 얻어온 정보를 set, 조회용
    void Setspace(std::string pln);
    // 테이블에서 조회
    int GetOccupied(std::string pln);

    // 조회
    void Setoccupiedarea(std::string pln);
    // 조회용
    void Updateoccupied();
    int GetSize();
    void Print();

    void WriteData(std::string FILE);
    void ReadData(std::string FILE);
    void DataIntoVector(); 
    std::vector<MYSQL_ROW> GetVector();
private:
    std::string parkinglotNum; 
    char address[256];
    int occupied;
    int space;
    int occupiedarea[MAXSIZE];
    MYSQL* ConnPtr;
    std::vector<MYSQL_ROW> store;
};

#endif
