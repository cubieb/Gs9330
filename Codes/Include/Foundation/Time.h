#ifndef _Time_h_
#define _Time_h_

time_t CalculateUtcGmtDiff();

/******************convert string to time_t******************/
time_t ConvertStrToTime(const char *str);
void ConvertUtcToGmt(struct tm& src, struct tm& dst);
void ConvertGmtToUtc(struct tm& src, struct tm& dst);

#endif