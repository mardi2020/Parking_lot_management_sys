#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <ctime>
#include </usr/include/mariadb/mysql.h>
#include "include/Car.h"
#include "include/message.h"
//#include "Parkinglot.cpp"

Car::Car(MYSQL *ConnPtr, char carnum[])
{
    this->ConnPtr = ConnPtr;
    // 주차 장소
    this->parkinglotNum = 0;
    this->occupiedNum = 0;
    strcpy(this->carnumber, carnum);
    this->toll = 0;
    InDate = GetDate();
    InTime = GetTime();
    OutDate = "";
    OutTime = "";
}

Car::~Car() {}

char* GetCarnum(){
    return this->carnumber;
}

void Car::Setlocation(int len, message_format m)
{
    int pl, c;

    for (int i = 0; i < m.len; i++)
    {
        if (0 <= i && i < 8)
        {
            continue;
        }
        else if (i == 9)
            pl = m.data[i] - 48;
        else if (i == 10)
            c = m.data[i] - 48;
    }
    this->parkinglotNum = pl;
    this->occupiedNum = c;
}
int GetParkingNum(){
    return this->parkinglotNum;
}
int GetOccupiedNum(){
    return this->occupiedNum;
}

std::string Car::GetDate()
{
    time_t t;
    tm *pt;
    time(&t);
    pt = localtime(&t);
    char buff[11];
    if (pt->tm_mon + 1 < 10)
        sprintf(buff, "%d-0%d-%d", pt->tm_year + 1900, pt->tm_mon + 1, pt->tm_mday);
    else
        sprintf(buff, "%d-%d-%d", pt->tm_year + 1900, pt->tm_mon + 1, pt->tm_mday);
    return std::string(buff);
}

std::string Car::GetTime()
{
    time_t t;
    tm *pt;
    time(&t);
    pt = localtime(&t);
    char buff2[13];
    sprintf(buff2, "%d:%d:%d", pt->tm_hour, pt->tm_min, pt->tm_sec);
    return std::string(buff2);
}

int GetToll(){
    return this->toll;
}

void Car::SettleupExpense()
{
    std::string query = "UPDATE car SET outdate = '" + OutDate + "' WHERE number = '" + std::string(carnumber) + "'";
    mysql_query(this->ConnPtr, query.c_str());

    query = "UPDATE car SET outtime = '" + OutTime + "' WHERE number = '" + std::string(carnumber) + "'";
    mysql_query(this->ConnPtr, query.c_str());

    query = "SELECT intime FROM car WHERE number = '" + std::string(carnumber) + "'";
    mysql_query(this->ConnPtr, query.c_str());

    MYSQL_RES *res;
    MYSQL_ROW row;

    res = mysql_store_result(ConnPtr);
    row = mysql_fetch_row(res);
    std::string start_time = std::string(row[0]);
    int min = (start_time[3] - 48) * 10 + (start_time[4] - 48);
    int hour = (start_time[0] - 48) * 10 + (start_time[1] - 48);

    min -= ((OutTime[3] - 48) * 10 + (OutTime[4] - 48));
    hour -= ((OutTime[0] - 48) * 10 + (OutTime[1] - 48)) * 60;
    int result = 1440 - (hour + min);

    toll = result * 100;

    query = "UPDATE car SET toll = " + std::to_string(this->toll) + " WHERE number = '" + std::string(carnumber) + "'";
    mysql_query(this->ConnPtr, query.c_str());

    query = "UPDATE car SET location_c = NULL WHERE number = '" + std::string(carnumber) + "'";
    mysql_query(this->ConnPtr, query.c_str());
    query = "UPDATE car SET location_p = NULL WHERE number = '" + std::string(carnumber) + "'";
    mysql_query(this->ConnPtr, query.c_str());
}
