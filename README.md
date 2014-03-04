LineBreak
=========

国内游戏对于中英文混杂的字符串在自动断行上不够完善，根据网上Unicode Line Break算法重新完善，支持中英文 数字 全角半角标点符号自动换行


####Unicode Line Breaking Algorithm实现

#####算法背景：

由于公司内游戏对国际化支持不够友好，其中文字断行方面并未够完美，仅仅支持空格以及换行符，并未考虑中文全角标点符号，英语以及数字混合等复杂情况，本算法正是为了完美解决中文 英语以及标点符号 数学表达式的自动换行。

#####算法综述：

此算法基于http://www.unicode.org/reports/tr14/规则并参考http://www.unicode.org/Public/PROGRAMS/LineBreakSampleCpp/5.2.0/实现。

由于英语换行是有明确规则，Unicode Line Breaking Algorithm上明确定义了所有英语换行规则，例如：
1 英语字母与空格一起时运行空格后面换行
2 当英语字母处于破折号-前面时不允许换行，但允许在-后面换行，同时如果-后面是数字的话则不允许换行。
3当左括号（后面接着英语字母或者数字时不允许换行，同时当英语字母或者数字紧挨着右括号）时不允许换行。
……..

因此Unicode Line Breaking Algorithm先把所有字符归类，然后根据一定的规则判断字符间是否可以换行，算法主要用途在于解析出给定字符串可以换行的地方。

######一  字符归类

因此Unicode Line Breaking Algorithm首先便利需要判断断行的字符串，并将其中的字符转换成同样的一类：
1.大小写字母为AL，数字为NU，CJK表意文字（中文 韩文 日文）则为ID，空格则为SP
2.（ “ 左括号 左引号则为OP， ） ”则为CP
………



下图则是对ANSI编码 00—7F的字符归类处理
```
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
```


######二 字符断行判断

当预处理字符后，则根据在http://www.unicode.org/reports/tr14/预先定义好的规则进行判断两个字符中间是否可以断行，例如左括号OP紧挨着字母AL时，不允许断行

核心规则表：
 
```
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
```

其中规则表里定义的动作acticon：
```
// Define some short-cuts for the table
#define oo DIRECT_BRK				// '_' break allowed
#define SS INDIRECT_BRK				// '%' only break across space (aka 'indirect break' below)
#define cc COMBINING_INDIRECT_BRK	// '#' indirect break for combining marks
#define CC COMBINING_PROHIBITED_BRK	// '@' indirect break for combining marks
#define XX PROHIBITED_BRK			// '^' no break allowed_BRK
#define xS HANGUL_SPACE_BRK			// break allowed, except when spaces are used with Hangul (not used)
``` 

OO表示可以直接断行
SS 表示如果两个字符间有空格则允许断行，否则不允许断行,例如两个英语字母之间不允许断行
XX 任何情况下都不允许断行

######三  中文CJK判断

    这部分实现主要参考 Android 源代码 StaticLayout.java中的函数
		private static final boolean isIdeographic(char c, boolean includeNonStarters)

CJK文字主要Unicode范围判断则可以参考此文档
完整的CJK Unicode范围（5.0版）


#####四 版权说明

此换行算法核心规则来源于Unicode Line Breaking Algorithm (http://www.unicode.org/unicode/reports/tr14/),
由于英语换行有明确的规则，Unicode Line Breaking Algorithm预先定义规则，并利用表格驱动方法来判断字符是否可以换行，

具体代码绝大部分借鉴于
http://www.unicode.org/Public/PROGRAMS/LineBreakSampleCpp/5.2.0/,作者仅修复其中不合理BUG并在此基础上添加对中文以及中文全角标点符号支持
中文全角字符的判断区间函数isIdeographic参考 Android 源代码 StaticLayout.java中的函数
boolean isIdeographic(char c, boolean includeNonStarters)

未经过大量测试，中英文 半角/全角符号 数学表达式换行比较完美.
