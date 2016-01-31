#include "Include/Foundation/SystemInclude.h"

#include "Include/Foundation/Time.h"
using namespace std;

time_t CalculateUtcGmtDiff()
{    
    time_t gmt = std::time(nullptr);
    struct tm calendar ;
    calendar  = *std::gmtime(&gmt);
    time_t utc = std::mktime(&calendar);
    time_t diff = (time_t)difftime(utc, gmt);
    return diff;
}

/******************convert string to time_t******************/
void ConvertStrToTm(const char *str, tm &timeInfo)
{
    std::istringstream ss(str);
    ss >> std::get_time(&timeInfo, "%Y-%m-%d %H:%M:%S");  
}

/* to convert struct tm to string, use std::put_time().
   consider use std::get_time, std::time_get::get_time, std::time_get_byname::get_time 
   to impliment current function.
   exsample:
        std::tm t = {};
        std::istringstream ss("2011-5-18 23:12:34");
        ss >> std::get_time(&t, "%Y-%m-%d %H:%M:%S");

    if use sscanf() to convert string to time:
        sscanf(str,"%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);  
 */
time_t ConvertStrToTime(const char *str)  
{  
    tm timeInfo;  

    ConvertStrToTm(str, timeInfo);
    time_t ret = mktime(&timeInfo); 
   
    return ret; 
} 

void ConvertUtcToGmt(struct tm& src, struct tm& dst)
{
    time_t time = std::mktime(&src);
    dst = *std::gmtime(&time);
}

/* refer to http://www.cplusplus.com/reference/ctime/gmtime/ */
#define CCT (+8)
void ConvertGmtToUtc(struct tm& src, struct tm& dst)
{
    dst = src;
    dst.tm_hour = (src.tm_hour + CCT) % 24;
}