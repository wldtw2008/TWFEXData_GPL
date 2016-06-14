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

// TaiFexFutureTick_GPL.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "../common/DataClass_GPL.h"
#include "../common/FileDir_GPL.h"

#include "TaiFexFutureTick_GPL.h"
#include <vector>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "../common/LiteUnzip.h"

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

int iGetZipBuffer(const char* szFileName, char* szErrMsg, const int iErrMsgLen, char** pUnZipFileName,unsigned char** pUnZipBuffer, int* piUnZipSize, const int iUnZipFileIndex)
{
	HMODULE		unzipDll;
	HUNZIP		huz;
	DWORD		result;
	DWORD		numitems = 0;
	unzipDll = (HMODULE)LoadLibrary(_T("LiteUnzip.dll"));
	if (unzipDll == NULL)
	{
		strcpy(szErrMsg, "Can't load LiteUnzip.dll!");
		return -1;
	}
	else
	{
		// Get the addresses of 5 functions in LiteUnzip.dll -- UnzipOpenFile(), UnzipGetItem()
		// UnzipItemToFile(), UnzipClose(), and UnzipFormatMessage.
		UnzipOpenFilePtr		*lpUnzipOpenFile;
		UnzipGetItemPtr			*lpUnzipGetItem;
		UnzipItemToBufferPtr	*lpUnzipItemToBuffer;
		UnzipClosePtr			*lpUnzipClose;
		UnzipFormatMessagePtr	*lpUnzipFormatMessage;
		lpUnzipOpenFile = (UnzipOpenFilePtr *)GetProcAddress(unzipDll, UNZIPOPENFILENAME);
		lpUnzipGetItem = (UnzipGetItemPtr *)GetProcAddress(unzipDll, UNZIPGETITEMNAME);
		lpUnzipItemToBuffer = (UnzipItemToBufferPtr *)GetProcAddress(unzipDll, UNZIPITEMTOBUFFERNAME);
		lpUnzipClose = (UnzipClosePtr *)GetProcAddress(unzipDll, UNZIPCLOSENAME);
		lpUnzipFormatMessage = (UnzipFormatMessagePtr *)GetProcAddress(unzipDll, UNZIPFORMATMESSAGENAME);
		result = lpUnzipOpenFile(&huz, szFileName, 0);
		if (result != 0)
		{
			strcpy(szErrMsg, "Can't load the ZIP file!");
			return -1;
		}
		else
		{
			ZIPENTRY		ze;

			// Find out how many items are in the archive.
			ze.Index = (DWORD)-1;
			result = lpUnzipGetItem(huz, &ze);
			if (result != 0) 
			{
				lpUnzipClose(huz);
				lpUnzipFormatMessage(result, szErrMsg, iErrMsgLen);
				FreeLibrary(unzipDll);
				return -1;
			}
			numitems = ze.Index;

			if (pUnZipBuffer == NULL && piUnZipSize == NULL)
			{
				lpUnzipClose(huz);
				FreeLibrary(unzipDll);
				sprintf(szErrMsg, "OK");
				return numitems;
			}

			// Unzip each item, using the name stored (in the zip) for that item.
			if (iUnZipFileIndex >= numitems)
			{
				lpUnzipClose(huz);
				FreeLibrary(unzipDll);
				sprintf(szErrMsg, "No such file index!");
				return -1;
			}
			ze.Index = iUnZipFileIndex;

			// Get info about the next item
			result = lpUnzipGetItem(huz, &ze);
			if (result != 0) 
			{
				lpUnzipClose(huz);
				lpUnzipFormatMessage(result, szErrMsg, iErrMsgLen);
				FreeLibrary(unzipDll);
				return -1;
			}
			else				
			{					
				// Allocate a memory buffer to decompress the item
				*pUnZipFileName = new char[strlen(ze.Name)+1];
				strcpy(*pUnZipFileName, ze.Name);
				*pUnZipBuffer = new unsigned char[ze.UncompressedSize];
				if (*pUnZipBuffer == NULL)
				{
					sprintf(szErrMsg, "Can't alloc memory (size=%d)!", ze.UncompressedSize);
					lpUnzipClose(huz);
					FreeLibrary(unzipDll);
					return -1;
				}
				
				// Decompress the item into our buffer
				result = lpUnzipItemToBuffer(huz, *pUnZipBuffer, ze.UncompressedSize, &ze);
				if (result != 0)
				{
					lpUnzipClose(huz);
					lpUnzipFormatMessage(result, szErrMsg, iErrMsgLen);
					FreeLibrary(unzipDll);
					return -1;
				}
				else
				{
					*piUnZipSize = ze.UncompressedSize;
				}
			}
		}

		// Done unzipping files, so close the ZIP archive.
		lpUnzipClose(huz);
		// Free the LiteUnzip.DLL
		FreeLibrary(unzipDll);
	}
	return 0;
}

int iGetIsHotData(const char* szSymbol, int iSymbolYearMonth, int iTradeDate)
{
	//int iSymbolYear = iSymbolYearMonth / 100;
	//int iSymbolMonth = iSymbolYearMonth % 100;
	int iTradeDateYear = iTradeDate / 10000;
	int iTradeDateMonth = (iTradeDate / 100) % 100;
	int iTradeDateDay = iTradeDate % 100;

	//用當天日期取出熱門月份。 方法比較簡單。
	// 1. 計算出iTradeDate當月一日是星期幾
	// 2. 計算出iTradeDate當天是第幾個星期幾
	COleDateTime tempOleDateTime;
	tempOleDateTime.SetDate(iTradeDateYear, iTradeDateMonth, 1);
	//return values range between 1 and 7, where 1=Sunday, 2=Monday, and so on.
	int iCurMonthFirstDayOfWeek = tempOleDateTime.GetDayOfWeek();

	//第一個星期三是幾號
	int i1stWedDateDay = 0;
	switch(iCurMonthFirstDayOfWeek)
	{
	case 1:
		i1stWedDateDay = 4;
		break;
	case 2:
		i1stWedDateDay = 3;
		break;
	case 3:
		i1stWedDateDay = 2;
		break;
	case 4:
		i1stWedDateDay = 1;
		break;
	case 5:
		i1stWedDateDay = 7;
		break;
	case 6:
		i1stWedDateDay = 6;
		break;
	case 7:
		i1stWedDateDay = 5;
		break;
	}
	//算出當月份第三個星期三是幾號
	int i3rdWedDateDay = i1stWedDateDay + 14;

	int iHotYearMonth;
	//如果交易日大於第三個星期三，那麼熱門月份就是當月+1
	if (iTradeDateDay > i3rdWedDateDay)
	{
		if (iTradeDateMonth == 12)
			iHotYearMonth = ((iTradeDateYear+1) * 100) + 1;
		else
			iHotYearMonth = (iTradeDateYear * 100) + (iTradeDateMonth + 1);
	}
	//如果交易日小於等於第三個星期三，那麼熱門月份就是當月
	else
	{
		iHotYearMonth = (iTradeDateYear * 100) + iTradeDateMonth;
	}

	if (iSymbolYearMonth == iHotYearMonth)
		return 1;
	else
		return 0;
}
void vSymbolTransFrom20080901(CString& cszOutput, char* pszSymbol)
{
    /* 20080901改的代號
    TX：臺股期貨
    MTX：小型臺指期貨
    TE：電子期貨
    TF：金融期貨
    GDF：黃金期貨
    T5F：臺灣50指數期貨
    GBF：十年期政府公債期貨
    CPF：三十天期利率期貨
    MSF：MSCI臺指期貨
    XIF：非金電期貨
    GTF：櫃買期貨
    TGF：新臺幣計價黃金期貨
    */
    /*//舊的
	{"WTX", "TXF", "台指期"}, //臺指期貨
	{"WMT", "MXF", "小台指"}, //小型臺指
	{"WTE", "EXF", "電子期"}, //電子期貨
	{"WTF", "FXF", "金融期"}, //金融期貨
	{"WGD", "GDF", "黃金期"}, //黃金期貨
	{"WTG", "TGF", "台幣黃金期"}, //黃金期貨
	{"WT5", "T5F", "台50期"}, //臺灣五十指數期貨
	{"WGB", "GBF", "公債期"}, //十年期公債期貨
	{"WCP", "CPF", "短利期"}, //三十天期利率期貨
	{"WMS", "MSF", "摩台期"}, //摩臺期貨
	{"WXI", "XIF", "非金電期"}, //非金電期貨
	{"WGT", "GTF", "櫃買期"}, //櫃買期貨
    */
    
    //就這四個不同
    if (strcmp(pszSymbol, "TX") == 0)
    {
        cszOutput = "TXF";
    }
    else if (strcmp(pszSymbol, "MTX") == 0)
    {
        cszOutput = "MXF";
    }
    else if (strcmp(pszSymbol, "TE") == 0)
    {
        cszOutput = "EXF";
    }
    else if (strcmp(pszSymbol, "TF") == 0)
    {
        cszOutput = "FXF";
    }
    else
    {
         cszOutput = pszSymbol;
    }
}
CString cszTaiFexAutoSymbol(char* pszSymbol)
{
    CString cszOutput;
    vSymbolTransFrom20080901(cszOutput, pszSymbol);
    return cszOutput;
}

bool bStartParseRPT(unsigned char* pszBuf, int iFileSize, int iBinaryOutputFlag,
                    int iDoTicks, int iDoSecs, int iDoMins, int iDoDailys, int iDoHotMonth)
{
    printf("Start to parse file from TaiFex Web *.rpt...\r\n");

    char szLine[512];
    int iDate;
    int iTradeTime;
    char szSymbolID1[32];
    int iMonth1;
    double dbPrice1;
    double dbVol1;
    char szSymbolID2[32];
    int iMonth2;
    double dbPrice2;
    double dbVol2;

    int iFound;
    vector<CTaiFexFeatureTick> vecTaiFexFeatureTick;
    CTaiFexFeatureTick tempTaiFexFeatureTick;
    CTaiFexTickInfo tempTaiFexTickInfo;

    unsigned char* pt = pszBuf;
    for (unsigned char* cur=pt;cur<pszBuf+iFileSize;++cur)
    {
        if (*cur == '\r' || *cur == '\n')
        {
            szLine[0] = '\0';
            strncat(szLine, (char*)pt, cur-pt);

            
           if (0 == bParseTaiFexFeatureTickData(szLine, &iDate, 
                                                 &iTradeTime, 
                                                 szSymbolID1, &iMonth1, &dbPrice1, &dbVol1,
                                                 szSymbolID2, &iMonth2, &dbPrice2, &dbVol2))
           {
                strncat(szLine, (char*)pt, cur-pt);
                printf("Parse error! >>> %s\r\n", szLine);
           }
           else if (iTradeTime == 0)
           {
                strncat(szLine, (char*)pt, cur-pt);
                printf("Parse error! >>> %s\r\n", szLine);
           }
           else 
           {
                iFound = -1;
                for (int i=0;i<vecTaiFexFeatureTick.size();++i)
                {
                    if (strcmp(vecTaiFexFeatureTick[i].szSymbol, szSymbolID1) == 0 &&
                        vecTaiFexFeatureTick[i].iMonth == iMonth1 &&
                        vecTaiFexFeatureTick[i].iTradeDate == iDate)
                    {
                        iFound = i;
                        break;
                    }
                }
                if (iFound == -1)
                {
                    tempTaiFexFeatureTick.vClear();
                    tempTaiFexFeatureTick.iMonth = iMonth1;
                    tempTaiFexFeatureTick.iTradeDate = iDate;
                    strcpy(tempTaiFexFeatureTick.szSymbol, szSymbolID1);
                    vecTaiFexFeatureTick.push_back(tempTaiFexFeatureTick);
                    iFound = vecTaiFexFeatureTick.size()-1;
                }
                tempTaiFexTickInfo.vClear();
                tempTaiFexTickInfo.dbPrice = dbPrice1;
                tempTaiFexTickInfo.dbVol = dbVol1;
                tempTaiFexTickInfo.iTradeTime = iTradeTime;
                vecTaiFexFeatureTick[iFound].vecTickInfo.push_back(tempTaiFexTickInfo);
                if (szSymbolID2[0] != '\0')
                {
                    iFound = -1;
                    for (int i=0;i<vecTaiFexFeatureTick.size();++i)
                    {
                        if (strcmp(vecTaiFexFeatureTick[i].szSymbol, szSymbolID2) == 0 &&
                            vecTaiFexFeatureTick[i].iMonth == iMonth2 &&
                            vecTaiFexFeatureTick[i].iTradeDate == iDate)
                        {
                            iFound = i;
                            break;
                        }
                    }
                    if (iFound == -1)
                    {
                        tempTaiFexFeatureTick.vClear();
                        tempTaiFexFeatureTick.iMonth = iMonth2;
                        tempTaiFexFeatureTick.iTradeDate = iDate;
                        strcpy(tempTaiFexFeatureTick.szSymbol, szSymbolID2);
                        vecTaiFexFeatureTick.push_back(tempTaiFexFeatureTick);
                        iFound = vecTaiFexFeatureTick.size()-1;
                    }
                    tempTaiFexTickInfo.vClear();
                    tempTaiFexTickInfo.dbPrice = dbPrice2;
                    tempTaiFexTickInfo.dbVol = dbVol2;
                    tempTaiFexTickInfo.iTradeTime = iTradeTime;
                    vecTaiFexFeatureTick[iFound].vecTickInfo.push_back(tempTaiFexTickInfo);
                }
           }

            while(*cur == '\r' || *cur == '\n')
            {
                cur++;
            }
            pt = cur;
        }
    }

    int i;

    if (iDoTicks == 1)
    {
        printf("Start to save tick file...\r\n");
        for (i=0;i<vecTaiFexFeatureTick.size();++i)
        {
            char szFileName[128];
            sprintf(szFileName, WLDTW_DATA_ROOT_DIR);
            CreateDirectory(szFileName, NULL);
            sprintf(szFileName, WLDTW_DATA_TICK_DIR);
            CreateDirectory(szFileName, NULL);
            sprintf(szFileName, "%s\\%s", WLDTW_DATA_TICK_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol));
            CreateDirectory(szFileName, NULL);
            sprintf(szFileName, "%s\\%s\\%s%d", WLDTW_DATA_TICK_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), vecTaiFexFeatureTick[i].iMonth);
            CreateDirectory(szFileName, NULL);
			//接續的熱門月份
            sprintf(szFileName, "%s\\%s\\%s_HOT", WLDTW_DATA_TICK_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol));
            CreateDirectory(szFileName, NULL);
        }
        for (i=0;i<vecTaiFexFeatureTick.size();++i)
        {
            FILE* fp;
            char szFileName[128];
            if (iBinaryOutputFlag == 0)
                sprintf(szFileName, "%s\\%s\\%s%d\\%08d.csv", WLDTW_DATA_TICK_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), vecTaiFexFeatureTick[i].iMonth, vecTaiFexFeatureTick[i].iTradeDate);
            else
                sprintf(szFileName, "%s\\%s\\%s%d\\%08d.tick", WLDTW_DATA_TICK_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), vecTaiFexFeatureTick[i].iMonth, vecTaiFexFeatureTick[i].iTradeDate);

            fp = fopen(szFileName, "wb");
            if (fp == NULL)
                continue;
            for (int j=0;j<vecTaiFexFeatureTick[i].vecTickInfo.size();++j)
            {
                if (iBinaryOutputFlag == 0)
                {
                    vecTaiFexFeatureTick[i].vecTickInfo[j].vWriteToFilePt(vecTaiFexFeatureTick[i].iTradeDate, fp, 1);
                }
                else
                {
                    vecTaiFexFeatureTick[i].vecTickInfo[j].vWriteToFilePt(vecTaiFexFeatureTick[i].iTradeDate, fp, 0);
                }
            }
            fclose(fp);
            printf(szFileName);
            printf(" Done!\r\n");
			if (1 == iDoHotMonth &&
				1 == iGetIsHotData(cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), vecTaiFexFeatureTick[i].iMonth, vecTaiFexFeatureTick[i].iTradeDate))
			{
				char szCopyCommand[256];
				sprintf(szCopyCommand, "copy %s %s\\%s\\%s_HOT", szFileName, WLDTW_DATA_TICK_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol));
				printf(" Hot month symbol! ==> ");
				system(szCopyCommand);
			}
        }
        printf("All tick file saved!\r\n");
    }

    if (iDoSecs == 1)
    {
	    //for 1sec file
	    std::vector<CBarData> vec1SecBarInfo;
        printf("Start to save 1sec file...\r\n");
        for (i=0;i<vecTaiFexFeatureTick.size();++i)
        {
            char szFileName[128];
            sprintf(szFileName, WLDTW_DATA_ROOT_DIR);
            CreateDirectory(szFileName, NULL);
            sprintf(szFileName, WLDTW_DATA_1SEC_DIR);
            CreateDirectory(szFileName, NULL);
            sprintf(szFileName, "%s\\%s", WLDTW_DATA_1SEC_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol));
            CreateDirectory(szFileName, NULL);
            sprintf(szFileName, "%s\\%s\\%s%d", WLDTW_DATA_1SEC_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), vecTaiFexFeatureTick[i].iMonth);
            CreateDirectory(szFileName, NULL);
			//接續的熱門月份
            sprintf(szFileName, "%s\\%s\\%s_HOT", WLDTW_DATA_1SEC_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol));
            CreateDirectory(szFileName, NULL);
        }
	    for (i=0;i<vecTaiFexFeatureTick.size();++i)
        {
            FILE* fp;
            char szFileName[128];
            if (iBinaryOutputFlag == 0)
                sprintf(szFileName, "%s\\%s\\%s%d\\%08d.csv", WLDTW_DATA_1SEC_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), vecTaiFexFeatureTick[i].iMonth, vecTaiFexFeatureTick[i].iTradeDate);
            else
                sprintf(szFileName, "%s\\%s\\%s%d\\%08d.1sec", WLDTW_DATA_1SEC_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), vecTaiFexFeatureTick[i].iMonth, vecTaiFexFeatureTick[i].iTradeDate);

            fp = fopen(szFileName, "wb");
            if (fp == NULL)
                continue;
		    
		    vec1SecBarInfo.clear();
		    vBuildNSecFromTick(MARKET_OPEN_TIME, MARKET_CLOSE_TIME, &vecTaiFexFeatureTick[i].vecTickInfo, &vec1SecBarInfo, 1);

            for (int j=0;j<vec1SecBarInfo.size();++j)
            {
			    if (iBinaryOutputFlag == 0)
                {
                    vec1SecBarInfo[j].vWriteToFilePt(vecTaiFexFeatureTick[i].iTradeDate, fp, 1);
                }
                else
                {
                    vec1SecBarInfo[j].vWriteToFilePt(vecTaiFexFeatureTick[i].iTradeDate, fp, 0);
                }
            }
            fclose(fp);
            printf(szFileName);
            printf(" Done!\r\n");
			if (1 == iDoHotMonth &&
				1 == iGetIsHotData(cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), vecTaiFexFeatureTick[i].iMonth, vecTaiFexFeatureTick[i].iTradeDate))
			{
				char szCopyCommand[256];
				sprintf(szCopyCommand, "copy %s %s\\%s\\%s_HOT", szFileName, WLDTW_DATA_1SEC_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol));
				printf(" Hot month symbol! ==> ");
				system(szCopyCommand);
			}
        }
        printf("All 1sec file saved!\r\n");
    }

	//for 1Min file
    if (iDoMins == 1)
    {
	    std::vector<CBarData> vec1MinBarInfo;
        printf("Start to save 1min file...\r\n");
        for (i=0;i<vecTaiFexFeatureTick.size();++i)
        {
            char szFileName[128];
            sprintf(szFileName, WLDTW_DATA_ROOT_DIR);
            CreateDirectory(szFileName, NULL);
            sprintf(szFileName, WLDTW_DATA_1MIN_DIR);
            CreateDirectory(szFileName, NULL);
            sprintf(szFileName, "%s\\%s", WLDTW_DATA_1MIN_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol));
            CreateDirectory(szFileName, NULL);
            sprintf(szFileName, "%s\\%s\\%s%d", WLDTW_DATA_1MIN_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), vecTaiFexFeatureTick[i].iMonth);
            CreateDirectory(szFileName, NULL);
			//接續的熱門月份
            sprintf(szFileName, "%s\\%s\\%s_HOT", WLDTW_DATA_1MIN_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol));
            CreateDirectory(szFileName, NULL);
        }
	    for (i=0;i<vecTaiFexFeatureTick.size();++i)
        {
            FILE* fp;
            char szFileName[128];
            if (iBinaryOutputFlag == 0)
                sprintf(szFileName, "%s\\%s\\%s%d\\%08d.csv", WLDTW_DATA_1MIN_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), vecTaiFexFeatureTick[i].iMonth, vecTaiFexFeatureTick[i].iTradeDate);
            else
                sprintf(szFileName, "%s\\%s\\%s%d\\%08d.1min", WLDTW_DATA_1MIN_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), vecTaiFexFeatureTick[i].iMonth, vecTaiFexFeatureTick[i].iTradeDate);

            fp = fopen(szFileName, "wb");
            if (fp == NULL)
                continue;
		    
		    vec1MinBarInfo.clear();
		    vBuildNSecFromTick(MARKET_OPEN_TIME, MARKET_CLOSE_TIME, &vecTaiFexFeatureTick[i].vecTickInfo, &vec1MinBarInfo, 60);

            for (int j=0;j<vec1MinBarInfo.size();++j)
            {
			    if (iBinaryOutputFlag == 0)
                {
                    vec1MinBarInfo[j].vWriteToFilePt(vecTaiFexFeatureTick[i].iTradeDate, fp, 1);
                }
                else
                {
                    vec1MinBarInfo[j].vWriteToFilePt(vecTaiFexFeatureTick[i].iTradeDate, fp, 0);
                }
            }
            fclose(fp);
            printf(szFileName);
            printf(" Done!\r\n");
			if (1 == iDoHotMonth &&
				1 == iGetIsHotData(cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), vecTaiFexFeatureTick[i].iMonth, vecTaiFexFeatureTick[i].iTradeDate))
			{
				char szCopyCommand[256];
				sprintf(szCopyCommand, "copy %s %s\\%s\\%s_HOT", szFileName, WLDTW_DATA_1MIN_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol));
				printf(" Hot month symbol! ==> ");
				system(szCopyCommand);
			}
        }
        printf("All 1min file saved!\r\n");
    }

	//for 1Day file
    if (iDoDailys == 1)
    {
	    std::vector<CBarData> vec1DayBarInfo;
        printf("Start to save 1Day file...\r\n");
        for (i=0;i<vecTaiFexFeatureTick.size();++i)
        {
            char szFileName[128];
            sprintf(szFileName, WLDTW_DATA_ROOT_DIR);
            CreateDirectory(szFileName, NULL);
            sprintf(szFileName, WLDTW_DATA_DAILY_DIR);
            CreateDirectory(szFileName, NULL);
            sprintf(szFileName, "%s\\%s", WLDTW_DATA_DAILY_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol));
            CreateDirectory(szFileName, NULL);
            sprintf(szFileName, "%s\\%s\\%s%d", WLDTW_DATA_DAILY_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), vecTaiFexFeatureTick[i].iMonth);
            CreateDirectory(szFileName, NULL);
			//接續的熱門月份
            sprintf(szFileName, "%s\\%s\\%s_HOT", WLDTW_DATA_DAILY_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol));
            CreateDirectory(szFileName, NULL);
        }
	    for (i=0;i<vecTaiFexFeatureTick.size();++i)
        {
            FILE* fp;
            char szFileName[128];
            if (iBinaryOutputFlag == 0)
                sprintf(szFileName, "%s\\%s\\%s%d\\%08d.csv", WLDTW_DATA_DAILY_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), vecTaiFexFeatureTick[i].iMonth, vecTaiFexFeatureTick[i].iTradeDate);
            else
                sprintf(szFileName, "%s\\%s\\%s%d\\%08d.daily", WLDTW_DATA_DAILY_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), vecTaiFexFeatureTick[i].iMonth, vecTaiFexFeatureTick[i].iTradeDate);

            fp = fopen(szFileName, "wb");
            if (fp == NULL)
                continue;
		    
		    vec1DayBarInfo.clear();
		    vBuildNSecFromTick(MARKET_OPEN_TIME, MARKET_CLOSE_TIME, &vecTaiFexFeatureTick[i].vecTickInfo, &vec1DayBarInfo, MARKET_CLOSE_TIME-MARKET_OPEN_TIME);

            for (int j=0;j<vec1DayBarInfo.size();++j)
            {
			    if (iBinaryOutputFlag == 0)
                {
                    vec1DayBarInfo[j].vWriteToFilePt(vecTaiFexFeatureTick[i].iTradeDate, fp, 1);
                }
                else
                {
                    vec1DayBarInfo[j].vWriteToFilePt(vecTaiFexFeatureTick[i].iTradeDate, fp, 0);
                }
            }
            fclose(fp);
            printf(szFileName);
            printf(" Done!\r\n");
			if (1 == iDoHotMonth &&
				1 == iGetIsHotData(cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), vecTaiFexFeatureTick[i].iMonth, vecTaiFexFeatureTick[i].iTradeDate))
			{
				char szCopyCommand[256];
				sprintf(szCopyCommand, "copy %s %s\\%s\\%s_HOT", szFileName, WLDTW_DATA_DAILY_DIR, cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol), cszTaiFexAutoSymbol(vecTaiFexFeatureTick[i].szSymbol));
				printf(" Hot month symbol! ==> ");
				system(szCopyCommand);
			}
        }
        printf("All 1Day file saved!\r\n");
    }
	return true;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	}
	else
	{
        /*
		// TODO: code your application's behavior here.
		CString strHello;
		strHello.LoadString(IDS_HELLO);
		cout << (LPCTSTR)strHello << endl;
        */
	}

	//版權宣告
	printf(pszMakeLicenseNote());
	printf("\n");

	///////////////////
    int iDoTicks = 1;
    int iDoSecs = 1;
    int iDoMins = 1;
    int iDoDailys = 1;
	int iDoHot = 0;

    int iBinaryOutputFlag = 0;
	int iZipInputFlag = 0;
    HWND hwndParent = NULL;
    CString cszCommandLine = GetCommandLine();
    cszCommandLine.MakeLower();
    if (strstr(cszCommandLine, "-f ") == NULL &&
		strstr(cszCommandLine, "-z ") == NULL)
    {
        printf("Please input *.Rpt or *.Zip\n"
			   "  ie: TaiFexFutureTick_GPL -f Daily_2008_06_12.rpt or \n"
			   "      TaiFexFutureTick_GPL -z Daily_2008_06_12.zip");
#ifdef _DEBUG
		printf("\n");
        printf("TaiFexFutureTick_GPL [-hwnd nnnn] [-t1] [-s1] [-m1] [-d1] [-hot1] [-z|-f] [-b] input filename\n");
        printf(" -hwnd nnnn: nnnn is the handle of the parent window.\n"
			   " -t1 : output ticks file.\n"
			   " -s1 : output seconds file.\n"
			   " -m1 : output minutes file.\n"
			   " -d1 : output daily file.\n"
			   " -hot1 : output hot month symbol.\n"
			   " -b : output binary file.\n");
        printf("  ie: TaiFexFutureTick_GPL -f Daily_2008_06_12.rpt <== rpt file \n"
			   "      TaiFexFutureTick_GPL -z Daily_2008_06_12.zip <== zip file \n");
#endif
        return 0;
    }

    const char* szInputFile;//= NULL; //fix for compile error in vs2010
	if (strstr(cszCommandLine, "-f ") != NULL)
	{
		szInputFile = strstr(cszCommandLine, "-f ") + 3;
		iZipInputFlag = 0;
	}
	if (strstr(cszCommandLine, "-z ") != NULL)
	{
		szInputFile = strstr(cszCommandLine, "-z ") + 3;
		iZipInputFlag = 1;
	}

	CString cszTemp = szInputFile;
	cszTemp.TrimLeft();
	cszTemp.TrimRight();
    if (cszTemp.GetLength() == 0)
    {
        printf("Please input *.Rpt or *.Zip\n"
			   "  ie: TaiFexFutureTick_GPL -f Daily_2008_06_12.rpt or \n"
			   "      TaiFexFutureTick_GPL -z Daily_2008_06_12.zip");
#ifdef _DEBUG
		printf("\n");
        printf("TaiFexFutureTick_GPL [-hwnd nnnn] [-t1] [-s1] [-m1] [-d1] [-hot1] [-z|-f] [-b] input filename\n");
        printf(" -hwnd nnnn: nnnn is the handle of the parent window.\n"
			   " -t1 : output ticks file.\n"
			   " -s1 : output seconds file.\n"
			   " -m1 : output minutes file.\n"
			   " -d1 : output daily file.\n"
			   " -hot1 : output hot month symbol.\n"
			   " -b : output binary file.\n");
        printf("  ie: TaiFexFutureTick_GPL -f Daily_2008_06_12.rpt <== rpt file \n"
			   "      TaiFexFutureTick_GPL -z Daily_2008_06_12.zip <== zip file \n");
#endif
        return 0;
    }

    if (strstr(cszCommandLine, "-b") != NULL)
    {
        iBinaryOutputFlag = 1;
    }
    
    if (strstr(cszCommandLine, "-hwnd ") != NULL)
    {
        int iHwndStart = (strstr(cszCommandLine, "-hwnd ") - cszCommandLine) + 6;
        char szHWND[20];
        szHWND[0] = '\0';
        for (int k=iHwndStart;k<strlen(cszCommandLine);++k)
        {
            if (cszCommandLine[k] == ' ')
            {
                strncat(szHWND, ((LPCTSTR)cszCommandLine)+iHwndStart, k-iHwndStart);
                break;
            }
        }
        hwndParent = (HWND)atoi(szHWND);
    }

    if (strstr(cszCommandLine, "-t0") != NULL)
        iDoTicks = 0;
    if (strstr(cszCommandLine, "-s0") != NULL)
        iDoSecs = 0;
    if (strstr(cszCommandLine, "-m0") != NULL)
        iDoMins = 0;
    if (strstr(cszCommandLine, "-d0") != NULL)
        iDoDailys = 0;
    if (strstr(cszCommandLine, "-hot1") != NULL)
        iDoHot = 1;


    unsigned char* pszBuf;
    int iFileSize;

	if (iZipInputFlag == 1)
	{
		char szErrMsg[512];
		int iTotalFile = iGetZipBuffer(szInputFile, szErrMsg, 1024, NULL, NULL, NULL, 0);
		if (iTotalFile == -1)
		{
			printf("[%s]Zip file error! %s\n",szInputFile, szErrMsg);
            system("pause");
			return 0;
		}
		for (int i=0;i<iTotalFile;++i)
		{
			char* pszUnzipFilename;
			int iRet = iGetZipBuffer(szInputFile, szErrMsg, 1024, &pszUnzipFilename, &pszBuf, &iFileSize, i);
			if (iRet == -1)
			{
				printf("[%s]Unzip file error! %s\n", szErrMsg);
                system("pause");
			}
			else
			{
				CString cszUnzipFilename = pszUnzipFilename;
				delete pszUnzipFilename;
				cszUnzipFilename.MakeLower();
				if ((cszUnzipFilename.Find(".rpt") > 0) || (cszUnzipFilename.Find(".csv") > 0)) // fix for new .csv file
				{
					bStartParseRPT(pszBuf, iFileSize, iBinaryOutputFlag, 
                                    iDoTicks, iDoSecs, iDoMins, iDoDailys, iDoHot);
				}
				else
				{
					printf("File is not *.rpt");
				}
				delete pszBuf;
			}
		}
	}
	else
	{
		FILE* fpFile;
		fpFile = fopen(szInputFile, "rb");
		if (fpFile == NULL)
		{
			printf("File not exist!");
			return 0;
		}
		if (fpFile != NULL)
		{
			fseek(fpFile, 0, SEEK_END);
			iFileSize = ftell(fpFile);
			fseek(fpFile, 0, SEEK_SET);
			pszBuf = new unsigned char[iFileSize + 1];
			fread(pszBuf, sizeof(unsigned char), iFileSize, fpFile);
			pszBuf[iFileSize] = '\0';
			fclose(fpFile);
		}
		bStartParseRPT(pszBuf, iFileSize, iBinaryOutputFlag, 
                       iDoTicks, iDoSecs, iDoMins, iDoDailys, iDoHot);
		delete pszBuf;
	}

if (hwndParent != NULL)
{
    ::UpdateWindow(hwndParent);
//    ::InvalidateRect(
}

	return nRetCode;
}


