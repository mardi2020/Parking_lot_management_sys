#include <iostream>
#include <string.h>
#include <vector>
#include <sstream>
#include "Car.h"
#include "Parkinglot.h"
#define MAXSIZE 10

Parkinglot::Parkinglot(int space, char add[], MYSQL * ptr)
{
    this->ConnPtr = ptr;
    this->space = space;
    this->occupied = 0;
    memset(occupiedarea, 0x00, sizeof(occupiedarea));
    strcpy(this->address, add);
}
Parkinglot::Parkinglot(std::string parkinglotnum, MYSQL *ptr)
{
    this->ConnPtr = ptr;
    this->parkinglotNum = parkinglotnum;
    memset(occupiedarea, 0x00, sizeof(occupiedarea));
    this->Getaddress(parkinglotnum); 
    Setspace(parkinglotnum);
    this->occupied = GetOccupied(parkinglotnum);
}
Parkinglot::~Parkinglot() {}

void Parkinglot::Insert()
{
    std::string query = "INSERT INTO parkinglot(address, space) VALUES('" + std::string(this->address) + "','" + std::to_string(space) + "')";
    mysql_query(this->ConnPtr, query.c_str());
}
void Parkinglot::Delete(std::string pln)
{
    std::string query = "DELETE FROM parkinglot WHERE number = " + pln;
    mysql_query(this->ConnPtr, query.c_str());
}

char *Parkinglot::Getaddress(std::string pln)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    std::string query = "SELECT address FROM parkinglot WHERE number = " + pln;
    if (this->ConnPtr == NULL) {
        std::cout << "mysql connnect ERROR!\n";
    }
    mysql_query(this->ConnPtr, query.c_str()); 
    res = mysql_store_result(this->ConnPtr);
    row = mysql_fetch_row(res);

    return row[0];
}

int* Parkinglot::GetArray()
{
    return this->occupiedarea;
}
int Parkinglot::GetSize()
{
    return sizeof(occupiedarea);
}
void Parkinglot::Print()
{   
    std::cout<<parkinglotNum<<std::endl;
    std::cout<<address<<std::endl;
    std::cout<<occupied<<' '<<space<<std::endl;
    for(int i = 0;i < this->space;i++)
        std::cout<<occupiedarea[i]<<' ';
    std::cout<<std::endl;
}
//조회할때
std::string Parkinglot::Getspace(std::string pln)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    std::string query = "SELECT space FROM parkinglot WHERE number = " + pln;
    mysql_query(this->ConnPtr, query.c_str());
    res = mysql_store_result(this->ConnPtr);
    row = mysql_fetch_row(res);
    // 총 주차 공간
    std::string space = std::string(row[0]);
    return space;
}
// db에서 얻어온 정보를 set, 조회용
void Parkinglot::Setspace(std::string pln)
{
    std::string str = Getspace(pln);
    int i = 0;
    std::stringstream ssInt(str);
    ssInt >> i;
    this->space = i;
}
// 테이블에서 조회
int Parkinglot::GetOccupied(std::string pln)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    std::string query = "SELECT COUNT(*) FROM car WHERE location_p = " + pln;
    mysql_query(this->ConnPtr, query.c_str());
    res = mysql_store_result(this->ConnPtr);
    row = mysql_fetch_row(res);
    std::string occupied = std::string(row[0]);
    int i = 0;
    std::stringstream ssInt(occupied);
    ssInt >> i;
    return i;
}

// 조회
void Parkinglot::Setoccupiedarea(std::string pln)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    std::vector<MYSQL_ROW> v1;
    std::string space = Getspace(pln);
    std::string query = "SELECT location_c FROM car WHERE location_p = " + pln + " and NOT location_p is NULL";

    mysql_query(this->ConnPtr, query.c_str());
    res = mysql_store_result(this->ConnPtr);

    while ((row = mysql_fetch_row(res)) != NULL)
        v1.push_back(row);
    std::stringstream ss(space);
    int n;
    ss >> n;

    memset(this->occupiedarea, 0x00, sizeof(int) * n);
    for (int i = 0; i < v1.size(); i++) //
    {
        this->occupiedarea[atoi(v1[i][0]) - 1] = 1;
    }
}
// 조회용
void Parkinglot::Updateoccupied()
{
    std::string query = "UPDATE parkinglot SET occupied=(SELECT COUNT(*) FROM car WHERE location_p = " + this->parkinglotNum + ") WHERE number = " + this->parkinglotNum;
    mysql_query(this->ConnPtr, query.c_str());
}


void Parkinglot::WriteData(std::string FILE){
    std::string query = "SELECT * INTO OUTFILE '"+FILE+"'"+" FIELDS TERMINATED BY ',' ENCLOSED BY '\"' ESCAPED BY '\\' LINES TERMINATED BY '\n' FROM parkinglot";
    mysql_query(ConnPtr, query.c_str());
}

void Parkinglot::ReadData(std::string FILE){
    std::string query = "LOAD DATA LOCAL INFILE '" + FILE + "' "+"INTO TABLE parkinglot CHARACTER SET UTF8 FIELDS TERMINATED BY ','";
    mysql_query(ConnPtr, query.c_str());
}

void Parkinglot::DataIntoVector(){
	std::string query = "SELECT * FROM parkinglot";
	mysql_query(ConnPtr, query.c_str());
	MYSQL_RES *res;
        MYSQL_ROW row;
	res = mysql_store_result(ConnPtr);
	while((row = mysql_fetch_row(res)) != NULL){
		this->store.push_back(row);
	}
	
}
std::vector<MYSQL_ROW> Parkinglot::GetVector(){
	return this->store;
}
