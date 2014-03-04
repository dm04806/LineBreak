#ifndef _LINEBRRK_H_
#define _LINEBRK_H_

/*
	此换行算法核心规则来源于Unicode Line Breaking Algorithm (http://www.unicode.org/unicode/reports/tr14/),
由于英语换行有明确的规则，Unicode Line Breaking Algorithm预先定义规则，并利用表格驱动方法来判断字符是否可以换行，

	具体代码绝大部分借鉴于http://www.unicode.org/Public/PROGRAMS/LineBreakSampleCpp/5.2.0/
	作者仅修复其中不合理BUG并在此基础上添加对中文以及中文全角标点符号支持
	中文全角字符的判断区间函数isIdeographic参考 Android 源代码 StaticLayout.java中的函数
		private static final boolean isIdeographic(char c, boolean includeNonStarters)


	未经过大量测试，中英文 半角/全角符号 数学表达式换行比较完美，对于同样是CJK表意文字的韩语 日文支持良好
	@author marlonlu
	@date: 2014.2.28
*/

#include <stdio.h>
#include <string>

namespace LINE_BREAK
{
	typedef unsigned long		u32;


	typedef std::wstring							UString;
	typedef wchar_t									WChar;

	#define TCHAR  WChar
	#define LPTSTR WChar*

	#define CHAR_FIRST_CJK 0x2E80

	enum break_class
	{
		// input types
		OP = 0,	// open
		CL,	// closing punctuation
		CP,	// closing parentheses (from 5.2.0) (before 5.2.0 treat like CL)
		QU,	// quotation
		GL,	// glue
		NS,	// no-start
		EX,	// exclamation/interrogation
		SY,	// Syntax (slash)
		IS,	// infix (numeric) separator
		PR,	// prefix
		PO,	// postfix
		NU,	// numeric
		AL,	// alphabetic
		ID,	// ideograph (atomic)
		IN,	// inseparable
		HY,	// hyphen
		BA,	// break after
		BB,	// break before
		B2,	// break both
		ZW,	// ZW space
		CM,	// combining mark
		WJ, // word joiner

		// used for Korean Syllable Block pair table
		H2, // Hamgul 2 Jamo Syllable
		H3, // Hangul 3 Jamo Syllable
		JL, // Jamo leading consonant
		JV, // Jamo vowel
		JT, // Jamo trailing consonant

		// these are not handled in the pair tables
		SA, // South (East) Asian
		SP,	// space
		PS,	// paragraph and line separators
		BK,	// hard break (newline)
		CR, // carriage return
		LF, // line feed
		NL, // next line
		CB, // contingent break opportunity
		SG, // surrogate
		AI, // ambiguous
		XX, // unknown
	}; 


	// Break actions are the types of break opportunities that may occur at a particular
	// point in the input. Values for these are also needed in the UI portion of the code
	// so they are already defined here - for explanation see below in the line break
	// section.
	enum break_action
	{
		DIRECT_BRK,
		INDIRECT_BRK, 		
		COMBINING_INDIRECT_BRK, 	
		COMBINING_PROHIBITED_BRK, 	
		PROHIBITED_BRK,
		EXPLICIT_BRK,
		HANGUL_SPACE_BRK,
	};

	int classifyLnBrk(const LPTSTR pszText, break_class * pcls,  int cch);
	int findLineBrk(break_class *pcls, break_action *pbrk, int cch);
	int findComplexBreak(break_class cls, break_class *pcls, break_action *pbrk, int cch);

	 break_class LBClassFromCh(TCHAR ch);
	int CharFromVisible(int ch);
	bool isIdeographic(TCHAR c);

}

#endif // ifndef _LINEBRK_H_
