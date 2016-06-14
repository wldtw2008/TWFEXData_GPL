/*
 * TaiFexFutureTick_GPL
 *
 * Authors:	WLDTW2008 <wldtw2008@googlemail.com>
 *              Please send support related email to: wldtw2008@googlemail.com
 *              Home Page: http://wldtw2008.at.3322.org
 *
 * Fixes:
 *      Fix xxxx - yourname <yourmail@yourmail.com>;
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Please read the file 'COPYING版權宣告.txt' for full GPL License.
 *
 * 本程式碼採用 GNU 組織的 GPL公開版權宣告，歡迎複製、改寫、散佈，並要求你應該以同樣的
 * 權利條款，授予後續軟體開發使用者。對 GPL 的完整條文，請查閱 'COPYING版權宣告.txt'
 * 或訪問: http://www.linux.org.tw/CLDP/OLD/doc/GPL.html
 */

#include "stdafx.h"
#include "DataClass_GPL.h"
#include <iostream>
#include <vector>
#include <algorithm>



CBarData::CBarData()
{
    vClear();
}
void CBarData::vClear()
{
	iTradeTime = 0;
	dbOpen = 0;
	dbHigh = 0;
	dbLow = 0;
	dbClose = 0;
	dbVol = 0;
}
void CBarData::vSetValueByBar(CBarData& BarData)
{
	if (dbClose == 0)
	{
		dbOpen = BarData.dbOpen;
		dbHigh = BarData.dbHigh;
		dbLow = BarData.dbLow;
		dbClose = BarData.dbClose;
		dbVol = BarData.dbVol;
	}
	else
	{
		if (BarData.dbHigh > dbHigh)
			dbHigh = BarData.dbHigh;
		if (BarData.dbLow < dbLow)
			dbLow = BarData.dbLow;
		dbClose = BarData.dbClose;
		dbVol += BarData.dbVol;
	}
}
void CBarData::vSetValueByTick(double& dbPrice, double& dbTickVol)
{
	if (dbClose == 0)
	{
		dbOpen = dbPrice;
		dbHigh = dbPrice;
		dbLow = dbPrice;
		dbClose = dbPrice;
		dbVol = dbTickVol;
	}
	else
	{
		if (dbPrice > dbHigh)
			dbHigh = dbPrice;
		if (dbPrice < dbLow)
			dbLow = dbPrice;
		dbClose = dbPrice;
		dbVol += dbTickVol;
	}
}
void CBarData::vWriteToFilePt(int iTradeDate, FILE* fp, int iTextMode)
{
	if (iTextMode == 1)
	{
		fprintf(fp, "%04d-%02d-%02d, %02d:%02d:%02d, %.4f, %.4f, %.4f, %.4f, %d\r\n",
                iTradeDate/10000, (iTradeDate/100)%100, iTradeDate%100,
                iTradeTime/1000000, (iTradeTime/10000)%100, (iTradeTime/100)%100,
                dbOpen,
                dbHigh,
                dbLow,
                dbClose,
                (int)dbVol);
	}
	else
	{
		fwrite((void*)&iTradeTime, sizeof(int), 1, fp);
		fwrite((void*)&dbOpen, sizeof(double), 1, fp);
		fwrite((void*)&dbHigh, sizeof(double), 1, fp);
		fwrite((void*)&dbLow, sizeof(double), 1, fp);
		fwrite((void*)&dbClose, sizeof(double), 1, fp);
		fwrite((void*)&dbVol, sizeof(double), 1, fp);
	}
}
bool CBarData::bReadFromBinFilePt(FILE* fp)
{
	BarDataFileStruct TaiFex1SecData;

	if (fread((void*)&TaiFex1SecData, 1, sizeof(BarDataFileStruct), fp) != NULL)
	{
		iTradeTime = TaiFex1SecData.iTradeTime;
		dbOpen = TaiFex1SecData.dbOpen;
		dbHigh = TaiFex1SecData.dbHigh;
		dbLow = TaiFex1SecData.dbLow;
		dbClose = TaiFex1SecData.dbClose;
		dbVol = TaiFex1SecData.dbVol;
		return 1;
	}
	return 0;
}
bool CBarData::operator==(CBarData& RHS)
{
    if (iTradeTime == RHS.iTradeTime)
        return true;
    else
        return false;
}
bool CBarData::operator>(CBarData& RHS)
{
    if (iTradeTime > RHS.iTradeTime)
        return true;
    else
        return false;
}
bool CBarData::operator<(CBarData& RHS)
{
    if (iTradeTime < RHS.iTradeTime)
        return true;
    else
        return false;
}

void CTaiFexTickInfo::vClear()
{
    iTradeTime = 0;
    dbPrice = 0;
    dbVol = 0;
}
CTaiFexTickInfo::CTaiFexTickInfo()
{
    vClear();
}

void CTaiFexTickInfo::vWriteToFilePt(int iTradeDate, FILE* fp, int iTextMode)
{
	if (iTextMode == 1)
	{
		fprintf(fp, "%04d-%02d-%02d, %02d:%02d:%02d, %.4f, %d\r\n",
                iTradeDate/10000, (iTradeDate/100)%100, iTradeDate%100,
                iTradeTime/1000000, (iTradeTime/10000)%100, (iTradeTime/100)%100,
				dbPrice,
				(int)dbVol);
	}
	else				
	{
		fwrite((void*)&iTradeTime, sizeof(int), 1, fp);
		fwrite((void*)&dbPrice, sizeof(double), 1, fp);
		fwrite((void*)&dbVol, sizeof(double), 1, fp);
	}
}
bool CTaiFexTickInfo::bReadFromBinFilePt(FILE* fp)
{
	TickDataFileStruct tempTickData;

	if (fread((void*)&tempTickData, 1, sizeof(tempTickData), fp) != NULL)
	{
		iTradeTime = tempTickData.iTradeTime;
		dbPrice = tempTickData.dbPrice;
		dbVol = tempTickData.dbVol;
		return 1;
	}
	return 0;
}
bool CTaiFexTickInfo::operator==(CTaiFexTickInfo& RHS)
{
    if (iTradeTime == RHS.iTradeTime)
        return true;
    else
        return false;
}
bool CTaiFexTickInfo::operator>(CTaiFexTickInfo& RHS)
{
    if (iTradeTime > RHS.iTradeTime)
        return true;
    else
        return false;
}
bool CTaiFexTickInfo::operator<(CTaiFexTickInfo& RHS)
{
    if (iTradeTime < RHS.iTradeTime)
        return true;
    else
        return false;
}


CTaiFexFeatureTick::CTaiFexFeatureTick()
{
    vClear();
}
void CTaiFexFeatureTick::vClear()
{
    iTradeDate = 0;
    szSymbol[0] = '\0';
    iMonth = 0;
    vecTickInfo.clear();
}

void vBuildNSecFromTick(int iMarketOpenTime, int iMarketCloseTime, std::vector<CTaiFexTickInfo>* pvecSource, std::vector<CBarData>* pvecBarInfo, int iNSec)
{
	//Note: iMarketOpenTime = 8:45:00 = 8*3600 + 45*60 + 0
	int iSecSize;

    //跨日商品
    if (iMarketCloseTime < iMarketOpenTime)
        iMarketCloseTime += 86400;

    if ((iMarketCloseTime - iMarketOpenTime) % iNSec == 0)
		iSecSize = (iMarketCloseTime - iMarketOpenTime) / iNSec;
	else
		iSecSize = (iMarketCloseTime - iMarketOpenTime) / iNSec + 1;

	vector<CBarData> vecBarData;
	pvecBarInfo->resize(iSecSize);
	int i;
	//先把間距的Vector取出
	for (i=0;i<iSecSize;++i)
	{
		(*pvecBarInfo)[i].iTradeTime = iMarketOpenTime + (i + 1)*iNSec;
	}
    
	int iTimeinSec;

	for (i=0;i<pvecSource->size();++i)
	{
		//做成連續秒數 86400格式
		iTimeinSec = (*pvecSource)[i].iTradeTime/1000000*3600 + (((*pvecSource)[i].iTradeTime/10000)%100)*60 + ((*pvecSource)[i].iTradeTime/100)%100;
        //跨日商品 如果交易時間小於收盤時間 那就加86400(讓TICK跨日)
        if (iMarketCloseTime > 86400 &&
            iTimeinSec < (iMarketCloseTime - 86400))
        {
            iTimeinSec += 86400;
        }

        if (iTimeinSec >= pvecBarInfo->back().iTradeTime)
		{
			pvecBarInfo->back().vSetValueByTick((*pvecSource)[i].dbPrice, (*pvecSource)[i].dbVol);
			continue;
		}
		else if ((iTimeinSec - iMarketOpenTime)/iNSec <0)
		{
			(*pvecBarInfo)[0].vSetValueByTick((*pvecSource)[i].dbPrice, (*pvecSource)[i].dbVol);
		}
        else
		{
			(*pvecBarInfo)[(iTimeinSec - iMarketOpenTime)/iNSec].vSetValueByTick((*pvecSource)[i].dbPrice, (*pvecSource)[i].dbVol);
		}
	}

	//將零值填滿
	double dbLastValue = 0;
	double dbZero = 0;
	int j;
	for (i=0;i<pvecBarInfo->size();++i)
	{
		//找到第一個有效數值
		if ((*pvecBarInfo)[i].dbClose == 0 && dbLastValue == 0) 
			continue;
		else if((*pvecBarInfo)[i].dbClose != 0 && dbLastValue == 0)
		{
			dbLastValue = (*pvecBarInfo)[i].dbClose;
			for (j=i;j>=0;--j)
				(*pvecBarInfo)[j].vSetValueByTick(dbLastValue, dbZero);
		}
		else if ((*pvecBarInfo)[i].dbClose == 0)
		{
			(*pvecBarInfo)[i].vSetValueByTick(dbLastValue, dbZero);
		}
		else
		{
			dbLastValue = (*pvecBarInfo)[i].dbClose;
		}
	}

	//還原成60 模式 ex 08:45:30 ==> 08453000
	int iTradeTime;
	for (i=0;i<pvecBarInfo->size();++i)
	{
		iTradeTime = int((*pvecBarInfo)[i].iTradeTime/3600)*1000000+
					(int((*pvecBarInfo)[i].iTradeTime/60)%60)*10000+
					(*pvecBarInfo)[i].iTradeTime%60*100;
		(*pvecBarInfo)[i].iTradeTime = iTradeTime;
	}

}
void vBuildNSecFromBar(int iMarketOpenTime, int iMarketCloseTime, std::vector<CBarData>* pvecSource, std::vector<CBarData>* pvecBarInfo, int iNSec)
{
	//Note: iMarketOpenTime = 8:45:00 = 8*3600 + 45*60 + 0
	int iSecSize;

    //跨日商品
    if (iMarketCloseTime < iMarketOpenTime)
        iMarketCloseTime += 86400;

    if ((iMarketCloseTime - iMarketOpenTime) % iNSec == 0)
		iSecSize = (iMarketCloseTime - iMarketOpenTime) / iNSec;
	else
		iSecSize = (iMarketCloseTime - iMarketOpenTime) / iNSec + 1;

	vector<CBarData> vecBarData;
	pvecBarInfo->resize(iSecSize);
	int i;
	//先把間距的Vector取出
	for (i=0;i<iSecSize;++i)
	{
		(*pvecBarInfo)[i].iTradeTime = iMarketOpenTime + (i + 1)*iNSec;
	}
    
	int iTimeinSec;

	for (i=0;i<pvecSource->size();++i)
	{
		//做成連續秒數 86400格式
		iTimeinSec = iTradeTime2DaySec((*pvecSource)[i].iTradeTime);///1000000*3600 + (((*pvecSource)[i].iTradeTime/10000)%100)*60 + ((*pvecSource)[i].iTradeTime/100)%100;
        //跨日商品 如果交易時間小於收盤時間 那就加86400(讓TICK跨日)
        if (iMarketCloseTime > 86400 &&
            iTimeinSec < (iMarketCloseTime - 86400))
        {
            iTimeinSec += 86400;
        }

        //減一的意思是 因為BAR 跟TICK不同，13:44分的那根 實際上是 13:43:00 ~ 13:43:59 
		//所以在五分鐘計算中 13:45那根 應該是 13:41 13:42 13:43 13:44 13:45 (不包含13:40那根)
		iTimeinSec--;
		if (iTimeinSec >= pvecBarInfo->back().iTradeTime)
		{
			pvecBarInfo->back().vSetValueByBar((*pvecSource)[i]);
			continue;
		}
		else if (iTimeinSec <= (*pvecBarInfo)[0].iTradeTime)
		{
			(*pvecBarInfo)[0].vSetValueByBar((*pvecSource)[i]);
			continue;
		}
		else if ((iTimeinSec - iMarketOpenTime)/iNSec <0)
		{
			(*pvecBarInfo)[0].vSetValueByBar((*pvecSource)[i]);
		}
		else
		{
			(*pvecBarInfo)[(iTimeinSec - iMarketOpenTime)/iNSec].vSetValueByBar((*pvecSource)[i]);
		}
	}

	//將零值填滿
	double dbLastValue = 0;
	double dbZero = 0;
	int j;
	for (i=0;i<pvecBarInfo->size();++i)
	{
		//找到第一個有效數值
		if ((*pvecBarInfo)[i].dbClose == 0 && dbLastValue == 0) 
			continue;
		else if((*pvecBarInfo)[i].dbClose != 0 && dbLastValue == 0)
		{
			dbLastValue = (*pvecBarInfo)[i].dbClose;
			for (j=i;j>=0;--j)
				(*pvecBarInfo)[j].vSetValueByTick(dbLastValue, dbZero);
		}
		else if ((*pvecBarInfo)[i].dbClose == 0)
		{
			(*pvecBarInfo)[i].vSetValueByTick(dbLastValue, dbZero);
		}
		else
		{
			dbLastValue = (*pvecBarInfo)[i].dbClose;
		}
	}

	//還原成60 模式 ex 08:45:30 ==> 08453000
	int iTradeTime;
	for (i=0;i<pvecBarInfo->size();++i)
	{
		iTradeTime = int((*pvecBarInfo)[i].iTradeTime/3600)*1000000+
					(int((*pvecBarInfo)[i].iTradeTime/60)%60)*10000+
					(*pvecBarInfo)[i].iTradeTime%60*100;
		(*pvecBarInfo)[i].iTradeTime = iTradeTime;
	}

}
bool bParseTaiFexFeatureTickData(char* pszData, int* piDate, 
                                 int* piTradeTime, 
                                 char* szSymbolID1, int* piMonth1, double* pdbPrice1, double* pdbVol1,
                                 char* szSymbolID2, int* piMonth2, double* pdbPrice2, double* pdbVol2)
{
    *piDate = 0;
    *piTradeTime = 0;
    szSymbolID1[0] = '\0';
    *piMonth1 = 0;
    *pdbPrice1 = 0;
    *pdbVol1 = 0;
    szSymbolID2[0] = '\0';
    *piMonth2 = 0;
    *pdbPrice2 = 0;
    *pdbVol2 = 0;

    char szTemp[32];
    char *token;
    int iTokenID = 0;
    token = strtok(pszData, ",");
    while( token != NULL )
    {
        switch(iTokenID)
        {
        //Date, symbolid, month, tradetime, price, vol(B+S), nearprice, farprice
        case 0:
            *piDate = atoi(token);
            break;
        case 1:
            strcpy(szSymbolID1, token);
            // 2008/9/3以後的代號會有空白
            if (strstr(szSymbolID1, " ") > szSymbolID1)
            {
                *(strstr(szSymbolID1, " ")) = '\0';
            }
            break;
        case 2:
            if (strstr(token, "/") != NULL)
            {
                char* pszSplit = strstr(token, "/");
                strcpy(szSymbolID2, szSymbolID1);

                szTemp[0] = '\0';
                strncat(szTemp, token, pszSplit - token);
                *piMonth1 = atoi(szTemp);
                
                szTemp[0] = '\0';
                strncat(szTemp, pszSplit + 1,  token + strlen(token) - pszSplit - 1);
                *piMonth2 = atoi(szTemp);
            }
            else
            {
                *piMonth1 = atoi(token);
            }
            break;
        case 3:
            *piTradeTime = atoi(token);
            break;
        case 4:
            if (*piMonth2 != 0)
            {
            }
            else
            {
                *pdbPrice1 = atof(token);
            }
            break;
        case 5:
			//如果是兩隻腳的TICK 那麼成交量要再除二
			if (*piMonth2 != 0)
			{
	            *pdbVol1 = atof(token)/4;
                *pdbVol2 = *pdbVol1;
			}
			else
			{
	            *pdbVol1 = atof(token)/2;
			}
            break;
        case 6://near price
            if (*piMonth2 != 0)
            {
                *pdbPrice1 = atof(token);
            }
            break;
        case 7://far price
            if (*piMonth2 != 0)
            {
                *pdbPrice2 = atof(token);
            }
            break;
        default:
            return 0;
            break;
        }
        iTokenID++;
        token = strtok(NULL, ",");
    }

    return 1;
}
unsigned int iTradeDateTime2Time_T(int iTradeDate, int iTradeTime)
{
    int iDayCount = 0;
    struct tm tmIn;
    memset(&tmIn, 0x00, sizeof(struct tm));
    iDayCount = (iTradeTime/1000000)/24;
    tmIn.tm_hour = (iTradeTime/1000000)%24;
    tmIn.tm_min = (iTradeTime/10000)%100;
    tmIn.tm_sec = (iTradeTime/100)%100;
    tmIn.tm_year = iTradeDate/10000 - 1900;
    tmIn.tm_mon = (iTradeDate/100)%100-1;
    tmIn.tm_mday = iTradeDate%100;
    time_t t = mktime(&tmIn);
    return t+iDayCount*86400;
}
int iGetAllFile(vector<CString>* pvecAllFiles, const CString& cszPath, const CString& cszFindFilter)
{
	pvecAllFiles->clear();
	CFileFind finder;
	CString cszFile;
	cszFile.Format("%s%s", cszPath, cszFindFilter);
	BOOL bWorking = finder.FindFile(cszFile);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsDots())
			continue;
		if (finder.IsDirectory())
			continue;

		pvecAllFiles->push_back(finder.GetFileName());
	}
	finder.Close();
	std::sort(pvecAllFiles->begin(),pvecAllFiles->end());

	return (pvecAllFiles->size());
}