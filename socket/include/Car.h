#ifndef _CAR_H
#define _CAR_H
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <ctime>
#include </usr/include/mariadb/mysql.h>

class Car{
public:
    // 처음 들어갈때 
    Car(MYSQL* ConnPtr, char carnum[]);
    ~Car();
    char* GetCarnum();
    void Setlocation(int len, message_format m);
    std::string GetDate();
    std::string GetTime();
    void SettleupExpense();
    int GetParkingNum();
    int GetOccupiedNum();
    int GetToll();

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
};
#endif