#include "LineBreak.h"


// Line Break Character Types
// These correspond to the line break class values defined in UAX#14, Version 
// 5.0.0. In a real implementation, there would be a mapping from character
// code to line break class value. In this demo version, the mapping is from
// a pseudo alphabet to these line break classes. The actual line break algorithm
// takes as input only line break classes, so, by changing the mapping from
// pseudo alphabet to actual Unicode Characters, this demo could be adapted 
// for use in actual line breaking.

namespace LINE_BREAK
{


break_class LnBrkClassFromChar[]  =
{		
	// treat CB as BB for demo purposes
//  0	1	2	3	4	5	6	7	8	9	a	b	c	d	e	f
	AL, ZW, GL, GL, BA, GL,	AL, B2, IN, BA, LF, CB, AL, CR, NL, AL, // 00-0f
	AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, // 10-1f

//  ' '  !   "       $   %   &   '   (   )   *   +   ,   -   .    /  
	SP, EX, QU, IN, PR, PO, BB, QU, OP, CP, BA, PR, IN, HY, IN, SY, // 20-2f
//   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
	NU, NU, NU, NU, NU, NU, NU, NU, NU,	NU,	NS,	AL,	AL,	GL, AL,	EX,	// 30-3f

//   @,  A  B   C   D   E   F   G   H   I   J   K   L   M   N   O  
	CB, AL,	AL, AL,	AL, AL,	AL, AL,	AL, AL,	AL, AL,	AL, AL,	AL, AL,	// 40-4f
	AL, AL, AL,	AL, AL,	AL, AL,	AL, AL,	AL, AL,	OP,	AL,	CP,	AL,	IS,	// 50-5f ... [ \ ] ^ _ 
	CM, AL,	AL, AL,	AL, AL,	AL, AL,	AL, AL,	AL, AL,	AL, AL,	AL, AL,	// 60-6f
	AL, AL, AL,	AL, AL,	AL, AL,	AL, AL,	AL, AL,	OP,	AL,	CL,	AL,	SA,	// 70-7f ... { | } ~ DEL
//   p  q   r   s   t    u   v   w  x   y   z 
};



/*---------------------------------------------------------------------------
	Function: classify
	
    Determines the character classes for all following
	passes of the algorithm 

	This uses a pseudo alphabet as input - see the szExplain string
	above for a description. In a production version, this function
	would implement the line break property lookup for actual Unicode
	characters.

	Input: Text string
		   Character count

	Output: Array of linebreak classes	

----------------------------------------------------------------------------*/
int classifyLnBrk(const LPTSTR pszText, break_class * pcls,  int cch)
{
	int ich;
	for (ich = 0; ich < cch; ich++)
	{
		if(pszText[ich] > CHAR_FIRST_CJK)
		{
			pcls[ich] = isIdeographic(pszText[ich]) ? ID : NS;

			//对于小括号 中括号 大括号进行特殊处理
			TCHAR c = pszText[ich];
			if(c == 0xFF08 || c == 0xFF3B || c == 0xFF5B) //小 中 大括号
				pcls[ich] = OP;
			else if(c == 0xFF09 || c == 0xFF3D || c == 0xFF5D)
				pcls[ich] = CP;

			continue;
		}

		TCHAR c = pszText[ich];
		//增加中文单引号 双引号支持
		if(c == 8216 || c == 8220)
		{
			pcls[ich] = OP;
			continue;
		}

		if(c == 8217 || c == 8221)
		{
			pcls[ich] = CP;
			continue;
		}

		pcls[ich] = LBClassFromCh(pszText[ich]);

		// map unknown, and ambiguous to AL by default
		if (pcls[ich] == XX || pcls[ich] == AI)
			pcls[ich] = AL;

		// map contingent break to B2 by default
		// this saves a row/col for CB in the table
		// but only approximates rule 20
		if (pcls[ich] == CB)
			pcls[ich] = B2;

		/* If the following remapping is enabled, all tests involving 
		   NL can be removed from the main loop below.
		   
		// map NL to BK as there's no difference
		if (pcls[ich] == NL)
			pcls[ich] = BK;
		*/
	}
	return ich;
}

// mapping of special character codes to Unicode symbols for visualization
int chVisibleFromSpecial[] =
{
	/* ZW  1 chZWSP */		0x2020,	// show as dagger
	/* GL  2 chZWNBSP */	0x2021,	// show as double dagger
	/* GL  3 chNBHY */		0x00AC,	// show as not sign
	/* BA  4 chSHY */		0x00B7,	// show as dot
	/* GL  5 chNBSP */		0x2017,	// show as low line
	/* -- 6 chDummy1 */		0x203E,	// show as double low line
	/* B2 7 chEM */			0x2014,	// show as em dash
	/* IN 8 chELLIPSIS */	0x2026,	// show as ellipsis
	/* CM 9 chTB */			0x2310,	// show as not sign
	/* LF 10 chLFx */		0x2580,	// show as high square
	/* CB 11 chOBJ */		0x2302,	// show as house (delete)
	/* -- 12 chdummy2 */	0x2222,
	/* CR 13 chCRx */		0x2584,	// show as low square
	/* NL 14 chNLx */		0x258C,	// show as left half block
};

// map visible symbol to character
int CharFromVisible(int ch)
{
	for (int ich = 0; ich < sizeof chVisibleFromSpecial / sizeof (int); ich ++)
	{
		if (ch == chVisibleFromSpecial[ich])
		{
			return ich + 1;
		}
	}
	return ch;
}

break_class LBClassFromCh(TCHAR ch)
{
	ch = LINE_BREAK::CharFromVisible((int)ch);
	if (ch >= 0x7f)
		return XX;
	return LnBrkClassFromChar[ch];
}


//2 // === LINE BREAK DEFINITIONS ===================================================

// Define some short-cuts for the table
#define oo DIRECT_BRK				// '_' break allowed
#define SS INDIRECT_BRK				// '%' only break across space (aka 'indirect break' below)
#define cc COMBINING_INDIRECT_BRK	// '#' indirect break for combining marks
#define CC COMBINING_PROHIBITED_BRK	// '@' indirect break for combining marks
#define XX PROHIBITED_BRK			// '^' no break allowed_BRK
#define xS HANGUL_SPACE_BRK			// break allowed, except when spaces are used with Hangul (not used)

// xS not yet assigned in the table below

//2 // === LINE BREAK PAIR TABLE ===================================================

// Line Break Pair Table corresponding to Table 2 of UAX#14, Version 5.0.0 
// plus Korean Syllable Block extensions - for details see that document

// Additional rows added or replaced for versions 5.0.1, 5.1.0 or 5.2.0 as needed by conditional compilation
// Additional column added for version 5.2.0 (CP). In earlier versions this acts identical to col for CL.

break_action brkPairs[][JT+1]=
{   //                ---     'after'  class  ------
	//		1	2	3	4	5	6	7	8	9  10  11  12  13  14  15  16  17  18  19  20  21   22  23  24  25  26  27
	//     OP, CL, CL, QU, GL, NS, EX, SY, IS, PR, PO, NU, AL, ID, IN, HY, BA, BB, B2, ZW, CM, WJ,  H2, H3, JL, JV, JT, = after class
	/*OP*/ XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, CC, XX,  XX, XX, XX, XX, XX, // OP open
	/*CL*/ oo, XX, XX, SS, SS, XX, XX, XX, XX, SS, SS, oo, oo, oo, oo, SS, SS, oo, oo, XX, cc, XX,  oo, oo, oo, oo, oo, // CL close
	/*CP*/ oo, XX, XX, SS, SS, XX, XX, XX, XX, SS, SS, SS, SS, oo, oo, SS, SS, oo, oo, XX, cc, XX,  oo, oo, oo, oo, oo, // CL close
	/*QU*/ XX, XX, XX, SS, SS, SS, XX, XX, XX, SS, SS, SS, SS, SS, SS, SS, SS, SS, SS, XX, cc, XX,  SS, SS, SS, SS, SS, // QU quotation
	/*GL*/ SS, XX, XX, SS, SS, SS, XX, XX, XX, SS, SS, SS, SS, SS, SS, SS, SS, SS, SS, XX, cc, XX,  SS, SS, SS, SS, SS, // GL glue
	/*NS*/ oo, XX, XX, SS, SS, SS, XX, XX, XX, oo, oo, oo, oo, oo, oo, SS, SS, oo, oo, XX, cc, XX,  oo, oo, oo, oo, oo, // NS no-start
	/*EX*/ oo, XX, XX, SS, SS, SS, XX, XX, XX, oo, oo, oo, oo, oo, oo, SS, SS, oo, oo, XX, cc, XX,  oo, oo, oo, oo, oo, // EX exclamation/interrogation
	/*SY*/ oo, XX, XX, SS, SS, SS, XX, XX, XX, oo, oo, SS, oo, oo, oo, SS, SS, oo, oo, XX, cc, XX,  oo, oo, oo, oo, oo, // SY Syntax (slash)
	/*IS*/ oo, XX, XX, SS, SS, SS, XX, XX, XX, oo, oo, SS, SS, oo, oo, SS, SS, oo, oo, XX, cc, XX,  oo, oo, oo, oo, oo, // IS infix (numeric) separator
	/*PR*/ SS, XX, XX, SS, SS, SS, XX, XX, XX, oo, oo, SS, SS, SS, oo, SS, SS, oo, oo, XX, cc, XX,  SS, SS, SS, SS, SS, // PR prefix
	/*PO*/ SS, XX, XX, SS, SS, SS, XX, XX, XX, oo, oo, SS, SS, oo, oo, SS, SS, oo, oo, XX, cc, XX,  oo, oo, oo, oo, oo, // NU numeric

	// Version 5.2.0 and higher
	/*NU*/ SS, XX, XX, SS, SS, SS, XX, XX, XX, SS, SS, SS, SS, oo, SS, SS, SS, oo, oo, XX, cc, XX,  oo, oo, oo, oo, oo, // AL alphabetic
	/*AL*/ SS, XX, XX, SS, SS, SS, XX, XX, XX, oo, oo, SS, SS, oo, SS, SS, SS, oo, oo, XX, cc, XX,  oo, oo, oo, oo, oo, // AL alphabetic

	/*ID*/ oo, XX, XX, SS, SS, SS, XX, XX, XX, oo, SS, oo, oo, oo, SS, SS, SS, oo, oo, XX, cc, XX,  oo, oo, oo, oo, oo, // ID ideograph (atomic)
	/*IN*/ oo, XX, XX, SS, SS, SS, XX, XX, XX, oo, oo, oo, oo, oo, SS, SS, SS, oo, oo, XX, cc, XX,  oo, oo, oo, oo, oo, // IN inseparable

	/*HY*/ oo, XX, XX, SS, oo, SS, XX, XX, XX, oo, oo, SS, oo, oo, oo, SS, SS, oo, oo, XX, cc, XX,  oo, oo, oo, oo, oo, // HY hyphens and spaces

	/*BA*/ oo, XX, XX, SS, oo, SS, XX, XX, XX, oo, oo, oo, oo, oo, oo, SS, SS, oo, oo, XX, cc, XX,  oo, oo, oo, oo, oo, // BA break after 
	/*BB*/ SS, XX, XX, SS, SS, SS, XX, XX, XX, SS, SS, SS, SS, SS, SS, SS, SS, SS, SS, XX, cc, XX,  SS, SS, SS, SS, SS, // BB break before 
	/*B2*/ oo, XX, XX, SS, SS, SS, XX, XX, XX, oo, oo, oo, oo, oo, oo, SS, SS, oo, XX, XX, cc, XX,  oo, oo, oo, oo, oo, // B2 break either side, but not pair
	/*ZW*/ oo, oo, oo, oo, oo, oo, oo, oo, oo, oo, oo, oo, oo, oo, oo, oo, oo, oo, oo, XX, oo, oo,  oo, oo, oo, oo, oo, // ZW zero width space
	/*CM*/ oo, XX, XX, SS, SS, SS, XX, XX, XX, oo, oo, SS, SS, oo, SS, SS, SS, oo, oo, XX, cc, XX,  oo, oo, oo, oo, oo, // CM combining mark
	/*WJ*/ SS, XX, XX, SS, SS, SS, XX, XX, XX, SS, SS, SS, SS, SS, SS, SS, SS, SS, SS, XX, cc, XX,  SS, SS, SS, SS, SS, // WJ word joiner

	/*H2*/ oo, XX, XX, SS, SS, SS, XX, XX, XX, oo, SS, oo, oo, oo, SS, SS, SS, oo, oo, XX, cc, XX,  oo, oo, oo, SS, SS, // Hangul 2 Jamo syllable
	/*H3*/ oo, XX, XX, SS, SS, SS, XX, XX, XX, oo, SS, oo, oo, oo, SS, SS, SS, oo, oo, XX, cc, XX,  oo, oo, oo, oo, SS, // Hangul 3 Jamo syllable
	/*JL*/ oo, XX, XX, SS, SS, SS, XX, XX, XX, oo, SS, oo, oo, oo, SS, SS, SS, oo, oo, XX, cc, XX,  SS, SS, SS, SS, oo, // Jamo Leading Consonant
	/*JV*/ oo, XX, XX, SS, SS, SS, XX, XX, XX, oo, SS, oo, oo, oo, SS, SS, SS, oo, oo, XX, cc, XX,  oo, oo, oo, SS, SS, // Jamo Vowel
	/*JT*/ oo, XX, XX, SS, SS, SS, XX, XX, XX, oo, SS, oo, oo, oo, SS, SS, SS, oo, oo, XX, cc, XX,  oo, oo, oo, oo, SS, // Jamo Trailing Consonant

};

// handle spaces separately, all others by table
// pcls - pointer to array of line breaking classes (input)
// pbrk - pointer to array of line break opportunities (output)
// cch - number of elements in the arrays (揷ount of characters? (input)
// ich - current index into the arrays (variable) (returned value)
// cls - current resolved line break class for 'before' character (variable)
int findLineBrk(break_class *pcls, break_action *pbrk, int cch)
{
	if (cch <= 0) 
	return 0;

	break_class cls = pcls[0];

	// handle case where input starts with an LF
	if (cls == LF)
		cls = BK;

	// treat NL like BK
	if (cls == NL)
		cls = BK;

	// treat SP at start of input as if it followed WJ
	if (cls == SP)
		cls = WJ;

	// loop over all pairs in the string up to a hard break or CRLF pair
	int ich;
	for (ich = 1; (ich < cch) && (cls != BK) && (cls != CR || pcls[ich] == LF); ich++) {

		// handle spaces explicitly
		if (pcls[ich] == SP) {
			pbrk[ich-1] = PROHIBITED_BRK;   // apply rule LB 7: ?SP
			continue;                       // do not update cls
		}

		if (pcls[ich] == BK || pcls[ich] == NL || pcls[ich] == LF) {
			pbrk[ich-1] = PROHIBITED_BRK;
			cls = BK;
			continue;
		}

		if (pcls[ich] == CR)
		{
			pbrk[ich-1] = PROHIBITED_BRK;
			cls = CR;
			continue;
		}

		// handle complex scripts in a separate function
		if (cls == SA || pcls[ich] == SA) {
			ich += findComplexBreak(cls, &pcls[ich-1], &pbrk[ich-1], cch - (ich-1));
			if (ich < cch)
				cls = pcls[ich];
			continue;
		}

		if(!(cls < SP) || !(pcls[ich] < SP))
		{
			continue;
		}

		// lookup pair table information in brkPairs[before, after];
		enum break_action brk = brkPairs[cls][pcls[ich]];

		pbrk[ich-1] = brk;                              // save break action in output array

		if (brk == INDIRECT_BRK) {                      // resolve indirect break
			if (pcls[ich - 1] == SP)                    // if context is A SP * B
				pbrk[ich - 1] = DIRECT_BRK;             //       break opportunity
			else                                        // else
				pbrk[ich-1] = PROHIBITED_BRK;           //       no break opportunity
		} else if (brk == COMBINING_PROHIBITED_BRK) {   // this is the case OP SP* CM
			pbrk[ich-1] = COMBINING_PROHIBITED_BRK;     // no break allowed
			if (pcls[ich-1] != SP)
				continue;                               // apply rule 9: X CM* -> X
		} else if (brk == COMBINING_INDIRECT_BRK) {     // resolve combining mark break
			pbrk[ich-1] = PROHIBITED_BRK;               // don't break before CM
			if (pcls[ich-1] == SP){
				if (false)                        // new: SP is not a base
					pbrk[ich-1] = COMBINING_INDIRECT_BRK;    // apply rule SP ?
				else                                    
				{
					pbrk[ich-1] = PROHIBITED_BRK;		// legacy: keep SP CM together
					if (ich > 1)
						pbrk[ich-2] = ((pcls[ich - 2] == SP) ? INDIRECT_BRK : DIRECT_BRK);
				}
			} else                                      // apply rule 9: X CM * -> X
				continue;                               // don't update cls
		}
		cls = pcls[ich];                                // save cls of current character
	}
	// always break at the end
	pbrk[ich-1] = EXPLICIT_BRK;

	return ich;
}

// placeholder function for complex break analysis
// cls - last resolved line break class (this is !SA)
// pcls - pointer to array of line breaking classes with pcls[0] == SA (input)
// pbrk - pointer to array of line breaking opportunities (output)
//
int findComplexBreak(break_class cls, break_class *pcls, break_action *pbrk, int cch)
{
	if (!cch)
	return 0;

	int ich;
	for (ich = 1; ich < cch; ich++) {

		// .. do complex break analysis here
		// and report any break opportunities in pbrk ..

		pbrk[ich-1] = PROHIBITED_BRK; // by default: no break

		if (pcls[ich] != SA)
			break;
	}
	return ich;
}


 /**
     * Returns true if the specified character is one of those specified
     * as being Ideographic (class ID) by the Unicode Line Breaking Algorithm
     * (http://www.unicode.org/unicode/reports/tr14/), and is therefore OK
     * to break between a pair of.
     *
	 * @see Android Source Code StaticLayout.Java (isIdeographic)
     */
bool isIdeographic(WChar c) {
	if (c >= 0x2E80 && c <= 0x2FFF)
	{
		return true; // CJK, KANGXI RADICALS, DESCRIPTION SYMBOLS
	}
	if (c == 0x3000)
	{
		return true; // IDEOGRAPHIC SPACE
	}

	if(c >= 0x3000 && c <= 0x303f) //CJK标点符号
	{
		return false;
	}

	if (c >= 0x3040 && c <= 0x309F)		//日文平假名
	{
		switch (c) 
		{
			case 0x3041: //  # HIRAGANA LETTER SMALL A
			case 0x3043: //  # HIRAGANA LETTER SMALL I
			case 0x3045: //  # HIRAGANA LETTER SMALL U
			case 0x3047: //  # HIRAGANA LETTER SMALL E
			case 0x3049: //  # HIRAGANA LETTER SMALL O
			case 0x3063: //  # HIRAGANA LETTER SMALL TU
			case 0x3083: //  # HIRAGANA LETTER SMALL YA
			case 0x3085: //  # HIRAGANA LETTER SMALL YU
			case 0x3087: //  # HIRAGANA LETTER SMALL YO
			case 0x308E: //  # HIRAGANA LETTER SMALL WA
			case 0x3095: //  # HIRAGANA LETTER SMALL KA
			case 0x3096: //  # HIRAGANA LETTER SMALL KE
			case 0x309B: //  # KATAKANA-HIRAGANA VOICED SOUND MARK
			case 0x309C: //  # KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK
			case 0x309D: //  # HIRAGANA ITERATION MARK
			case 0x309E: //  # HIRAGANA VOICED ITERATION MARK
				return false;
				break;
			default :
				return true;
				
		}
	}

	if (c >= 0x30A0 && c <= 0x30FF)		//日文片假名
	{
		switch (c) {
			case 0x30A0: //  # KATAKANA-HIRAGANA DOUBLE HYPHEN
			case 0x30A1: //  # KATAKANA LETTER SMALL A
			case 0x30A3: //  # KATAKANA LETTER SMALL I
			case 0x30A5: //  # KATAKANA LETTER SMALL U
			case 0x30A7: //  # KATAKANA LETTER SMALL E
			case 0x30A9: //  # KATAKANA LETTER SMALL O
			case 0x30C3: //  # KATAKANA LETTER SMALL TU
			case 0x30E3: //  # KATAKANA LETTER SMALL YA
			case 0x30E5: //  # KATAKANA LETTER SMALL YU
			case 0x30E7: //  # KATAKANA LETTER SMALL YO
			case 0x30EE: //  # KATAKANA LETTER SMALL WA
			case 0x30F5: //  # KATAKANA LETTER SMALL KA
			case 0x30F6: //  # KATAKANA LETTER SMALL KE
			case 0x30FB: //  # KATAKANA MIDDLE DOT
			case 0x30FC: //  # KATAKANA-HIRAGANA PROLONGED SOUND MARK
			case 0x30FD: //  # KATAKANA ITERATION MARK
			case 0x30FE: //  # KATAKANA VOICED ITERATION MARK
				return false;
				break;
			default :
				return true;
		}
	}

	if (c >= 0x3400 && c <= 0x4DB5) {
		return true; // CJK UNIFIED IDEOGRAPHS EXTENSION A
	}
	if (c >= 0x4E00 && c <= 0x9FBB) {
		return true; // CJK UNIFIED IDEOGRAPHS
	}
	if (c >= 0xF900 && c <= 0xFAD9) {
		return true; // CJK COMPATIBILITY IDEOGRAPHS
	}
	if (c >= 0xA000 && c <= 0xA48F) {
		return true; // YI SYLLABLES
	}
	if (c >= 0xA490 && c <= 0xA4CF) {
		return true; // YI RADICALS
	}
	if (c >= 0xFE62 && c <= 0xFE66) {
		return true; // SMALL PLUS SIGN to SMALL EQUALS SIGN
	}
	if (c >= 0xFF10 && c <= 0xFF19) {
		return true; // WIDE DIGITS
	}

	if(c >= 0xFF01 && c <= 0xFF0F) {
		return false;
	}

	if(c >= 0xFF1A && c <= 0xFF20) {
		return false;
	}

	if((c >= 0xFF21 && c <= 0xFF3A) || (c >= 0xFF41 && c <= 0xFF5A)) {
		return true;	//WIDTH　Letter
	}

	return false;
}

}