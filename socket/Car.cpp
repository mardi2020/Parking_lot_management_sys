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

char* Car::GetCarnum(){
    return this->carnumber;
}
void Car::Setlocation(int len, message_format m)
{
    int c;

    for (int i = 0; i < m.len; i++)
    {
        if (0 <= i && i < 8)
        {
            continue;
        }
        else if (i == 9){
            c = m.data[i] - 48;
        }
    }
    this->occupiedNum = c;
}
void Car::SetParkingNum(){ // db에서 가져오기
    MYSQL_RES *res;
    MYSQL_ROW row;
 
    std::string query = "SELECT location_p FROM car WHERE number = '"+std::string(carnumber)+"'";
    mysql_query(ConnPtr, query.c_str());
    res = mysql_store_result(ConnPtr);
    row = mysql_fetch_row(res);
    int p = atoi(row[0]);

    this->parkinglotNum = p;

}
int Car::GetParkingNum(){
    return this->parkinglotNum;
}
int Car::GetOccupiedNum(){
    return this->occupiedNum;
}
void Car::SetOccupiedNum(){ // db에서 가져오기

    MYSQL_RES *res;
    MYSQL_ROW row;

    std::string query = "SELECT location_c FROM car WHERE number = '"+std::string(carnumber)+"'";
    mysql_query(ConnPtr, query.c_str());
    res = mysql_store_result(ConnPtr);
    row = mysql_fetch_row(res);
    int c = atoi(row[0]);
    this->occupiedNum = c;
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

int Car::GetToll(){
    return this->toll;
}

void Car::SettleupExpense()
{
    OutDate = GetDate(); OutTime = GetTime();
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

    query = "SELECT indate FROM car WHERE number = '"+std::string(carnumber)+"'";
    mysql_query(ConnPtr, query.c_str());
    res = mysql_store_result(ConnPtr);
    row = mysql_fetch_row(res);
    std::string start_date = std::string(row[0]);
    int min = 0, hour = 0;
    int min_in = (start_time[3] - 48) * 10 + (start_time[4] - 48); //35
    int hour_in = (start_time[0] - 48) * 10 + (start_time[1] - 48); //14
    int min_out = (OutTime[3] - 48) * 10 + (OutTime[4] - 48);
    int hour_out = ((OutTime[0] - 48) * 10 + (OutTime[1] - 48));
    if(OutDate != start_date){
        hour_out += 24;
        if (min_out < min_in){
            min = (min_out + 60) - min_in;
            hour = (--hour_out) - hour_in;
        }
        else{
            min = min_out - min_in;
            hour = hour_out - hour_in;
        }
        toll = (hour*60 + min) * 100;
    }
    else{ // 당일날 정산
        if(min_out >= min_in){
            min = min_out - min_in;
            hour = hour_out - hour_in;
        }
        else{
            min = (min_out + 60) - min_in;
            hour = (--hour_out) - hour_in;
        }
        toll = (hour*60 + min)*100;
    }
    query = "UPDATE car SET toll = " + std::to_string(this->toll) + " WHERE number = '" + std::string(carnumber) + "'";
    mysql_query(this->ConnPtr, query.c_str());

    query = "UPDATE car SET location_c = NULL WHERE number = '" + std::string(carnumber) + "'";
    mysql_query(this->ConnPtr, query.c_str());
    query = "UPDATE car SET location_p = NULL WHERE number = '" + std::string(carnumber) + "'";
    mysql_query(this->ConnPtr, query.c_str());
}

void Car::InsertDataInDB(){
    std::string query = "INSERT INTO car(number, indate, intime, location_c, location_p) VALUES('"+std::string(carnumber)+"','"+InDate+"','"+InTime+"',"+std::to_string(occupiedNum)+","+std::to_string(parkinglotNum)+")";
    mysql_query(ConnPtr, query.c_str());
}

void Car::SetParkinglotNum(int number){
    this->parkinglotNum = number;
}

void Car::Print(){
        std::cout<<carnumber<<std::endl;
        std::cout<<parkinglotNum<<std::endl;
        std::cout<<occupiedNum<<std::endl;
        std::cout<<InDate<<' '<<InTime<<std::endl;
    }