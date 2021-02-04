#ifndef _CAR_H
#define _CAR_H
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <ctime>
#include </usr/include/mysql/mysql.h>
#include "message.h"

class Car{
public:
    Car(MYSQL* ConnPtr, char carnum[]);
    Car(){}
    ~Car();
    char* GetCarnum();
    void Setlocation(int len, message_format m);
    std::string GetDate();
    std::string GetTime();
    void SettleupExpense();
    int GetParkingNum();
    int GetOccupiedNum();
    int GetToll();
    void SetParkinglotNum(int number);
    void SetParkingNum();
    void SetOccupiedNum();
    void InsertDataInDB();
    void DataIntoVector();
    void Print();
    std::vector<MYSQL_ROW> GetVector();
private:
    int parkinglotNum;
    int occupiedNum;
    char carnumber[12];
    MYSQL* ConnPtr;
    int toll;
    std::string InDate;
    std::string InTime;
    std::string OutDate;
    std::string OutTime;
    std::vector<MYSQL_ROW> store;
};
#endif
