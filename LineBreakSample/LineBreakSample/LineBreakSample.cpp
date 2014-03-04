// LineBreakSample.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "LineBreak.h"
#include <string>
#include <iostream>
#include <vector>

#define MAX_CCH 256

using namespace LINE_BREAK;
using namespace std;

break_class* lbcls;
break_action* lbrks;

std::vector<UString>* strings;

void ShowLineBreaks(FILE * f, const LPTSTR pszInput, break_action *pbrk, int cch)
{

	TCHAR* pszBrkText = new TCHAR[cch * 2];

	int ichIn, ichOut;
	for (ichIn = 0, ichOut = 0; ichIn < cch; ichIn++)
	{
		if (pbrk[ichIn])
		{
			if (pbrk[ichIn] > 1)
			{
				pszBrkText[ichOut++] = pszInput[ichIn];
				//pszBrkText[ichOut++] = (TCHAR) 0xa6;

			}
			else
			{
				pszBrkText[ichOut++] = pszInput[ichIn];
				pszBrkText[ichOut++] = '?';
			}
		}
		else
		{
			pszBrkText[ichOut++] = pszInput[ichIn];
			pszBrkText[ichOut++] = '|';
		}
	}
	pszBrkText[ichOut] = 0;
	std::wcout<<L"Output:"<<pszBrkText<<std::endl;
	delete pszBrkText;
}

/*
* 实例demo，确定每个字符的宽度，需要实际情况再处理
*/
int getCharWidth(WChar c)
{
	if(isIdeographic(c))
	{
		return 2;
	} else
		return 1;
}

/*
* 示例demo，根据最大宽度字符串自动断行。
*/
void getStringArrays(UString& test, int width) 
{
	WChar* charArrays = (WChar*)test.c_str();
	int w = 0;
	int index = 0;
	int lastIndex = 0;

	while(*charArrays) 
	{
		if(*charArrays == L'\n')
		{
			strings->push_back(test.substr(lastIndex, index - lastIndex));
			lastIndex = index;
			w = 0;
			index++;
			charArrays++;
			continue;
		}

		if(w >= width) 
		{
			WChar* temp = charArrays - 1;
			int tempIndex = index - 1;
			while(*temp && tempIndex > lastIndex)
			{

				if(!lbrks[tempIndex])
					break;

				temp--;
				tempIndex--;
			}

			if(tempIndex > lastIndex) 
			{
				index = tempIndex + 1;
				strings->push_back(test.substr(lastIndex, index - lastIndex));
				lastIndex = index;
				w = 0;
				charArrays = temp + 1;
				continue;
			} 
			else 
			{
				//如果上一个断行的地方已经超出了屏幕宽度的话，则直接断行，没办法处理这种情况，比如aaaaaaaaaaaaaaaaaaa 单个英语单词就超出了屏幕运行的最大宽度
				strings->push_back(test.substr(lastIndex, index - lastIndex));
				lastIndex = index;
				w = 0;
				index++;
				charArrays++;
				continue;
			}
		}
		else
		{
			w += getCharWidth(*charArrays);
			index++;
			charArrays++;
		}
	}

	strings->push_back(test.substr(lastIndex, test.length() - lastIndex));
}

int _tmain(int argc, _TCHAR* argv[])
{
	int realArg = 0;
	int doCMInTable = 1;
	int beVerbose = 1;

	FILE* f = stdout;

	std::wstring testChinese = L"此前1月13日，中国移动公布了（4G全网）统一资费方案，包含移动数据流量、全国长市漫一口价语音以及数据业务三种资费元素，其中移动数据流量从40元400M到280元10G共7档。The incrementStock function calls the \"addProduct\" function only if the sku string given in the argument matches with a member of the \"inventory\" array (the array is of type *Product and of size 50).";

	const LPTSTR widecstr = testChinese.c_str();


	int cch = testChinese.length();
	// assign line breaking classes

	lbcls = new break_class[cch];
	lbrks = new break_action[cch];

	classifyLnBrk(widecstr, lbcls, testChinese.length());
	setlocale(LC_ALL,"chs");
	//fwprintf(f, L"Input    %2d: %s\n", widecstr);
	std::wcout<<L"input:"<<widecstr<<std::endl<<endl;

	// find the line breaks
	findLineBrk(lbcls, lbrks, cch);

	// write display string
	ShowLineBreaks(f, widecstr, lbrks,  cch); fprintf(f, "\n");

	strings = new std::vector<UString>();
	std::cout<<std::endl;
	getStringArrays(testChinese, 20);
	for(std::vector<UString>::iterator i = strings->begin(); i != strings->end(); i++)
	{
		std::wcout<<(*i).c_str()<<L"\\\\"<<std::endl;
	}


	delete lbrks;
	delete lbcls;
	delete strings;

	return 0;
}


