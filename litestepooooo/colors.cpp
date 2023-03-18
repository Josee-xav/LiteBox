#include "colors.h"
#include <stdlib.h>
#pragma warning(disable : 4996)
char* extract_string(char* dest, const char* src, int n)
{
	memcpy(dest, src, n);
	dest[n] = 0;
	return dest;
}

char* unquote(char* src)
{
	int l = (int)strlen(src);
	if (l >= 2 && (src[0] == '\"' || src[0] == '\'') && src[l - 1] == src[0])
		return extract_string(src, src + 1, l - 2);
	return src;
}

char* strcpy_max(char* dest, const char* src, int maxlen)
{
	int l = (int)strlen(src);
	return extract_string(dest, src, l < maxlen ? l : maxlen - 1);
}

COLORREF rgb(unsigned r, unsigned g, unsigned b)
{
	return RGB(r, g, b);
}

COLORREF switchRgb(COLORREF c)
{
	return ((c & 0x0000ff) << 16) | (c & 0x00ff00) | ((c & 0xff0000) >> 16);
}

COLORREF mixcolors(COLORREF c1, COLORREF c2, int f)
{
	int n = 255 - f;
	return RGB(
		(GetRValue(c1) * f + GetRValue(c2) * n) / 255,
		(GetGValue(c1) * f + GetGValue(c2) * n) / 255,
		(GetBValue(c1) * f + GetBValue(c2) * n) / 255
	);
}

COLORREF shadecolor(COLORREF c, int f)
{
	int r, g, b;
	r = (int)GetRValue(c) + f;
	g = (int)GetGValue(c) + f;
	b = (int)GetBValue(c) + f;
	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;
	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	return RGB(r, g, b);
}

unsigned greyvalue(COLORREF c)
{
	unsigned r, g, b;
	r = GetRValue(c);
	g = GetGValue(c);
	b = GetBValue(c);
	return (r * 79 + g * 156 + b * 21) / 256;
}

/* X-Windows color names */
static struct litcolor1 { const char* cname; COLORREF cref; } litcolor1_ary[] = {
	{ "aliceblue", RGB(240,248,255) },
	{ "beige", RGB(245,245,220) },
	{ "black", RGB(0,0,0) },
	{ "blanchedalmond", RGB(255,235,205) },
	{ "blueviolet", RGB(138,43,226) },
	{ "cornflowerblue", RGB(100,149,237) },
	{ "darkblue", RGB(0,0,139) },
	{ "darkcyan", RGB(0,139,139) },
	{ "darkgray", RGB(169,169,169) },
	{ "darkgreen", RGB(0,100,0) },
	{ "darkkhaki", RGB(189,183,107) },
	{ "darkmagenta", RGB(139,0,139) },
	{ "darkred", RGB(139,0,0) },
	{ "darksalmon", RGB(233,150,122) },
	{ "darkslateblue", RGB(72,61,139) },
	{ "darkturquoise", RGB(0,206,209) },
	{ "darkviolet", RGB(148,0,211) },
	{ "dimgray", RGB(105,105,105) },
	{ "floralwhite", RGB(255,250,240) },
	{ "forestgreen", RGB(34,139,34) },
	{ "gainsboro", RGB(220,220,220) },
	{ "ghostwhite", RGB(248,248,255) },
	{ "gray", RGB(190,190,190) },
	{ "greenyellow", RGB(173,255,47) },
	{ "lavender", RGB(230,230,250) },
	{ "lawngreen", RGB(124,252,0) },
	{ "lightcoral", RGB(240,128,128) },
	{ "lightgoldenrodyellow", RGB(250,250,210) },
	{ "lightgray", RGB(211,211,211) },
	{ "lightgreen", RGB(144,238,144) },
	{ "lightseagreen", RGB(32,178,170) },
	{ "lightslateblue", RGB(132,112,255) },
	{ "lightslategray", RGB(119,136,153) },
	{ "limegreen", RGB(50,205,50) },
	{ "linen", RGB(250,240,230) },
	{ "mediumaquamarine", RGB(102,205,170) },
	{ "mediumblue", RGB(0,0,205) },
	{ "mediumseagreen", RGB(60,179,113) },
	{ "mediumslateblue", RGB(123,104,238) },
	{ "mediumspringgreen", RGB(0,250,154) },
	{ "mediumturquoise", RGB(72,209,204) },
	{ "mediumvioletred", RGB(199,21,133) },
	{ "midnightblue", RGB(25,25,112) },
	{ "mintcream", RGB(245,255,250) },
	{ "moccasin", RGB(255,228,181) },
	{ "navy", RGB(0,0,128) },
	{ "navyblue", RGB(0,0,128) },
	{ "oldlace", RGB(253,245,230) },
	{ "palegoldenrod", RGB(238,232,170) },
	{ "papayawhip", RGB(255,239,213) },
	{ "peru", RGB(205,133,63) },
	{ "powderblue", RGB(176,224,230) },
	{ "saddlebrown", RGB(139,69,19) },
	{ "sandybrown", RGB(244,164,96) },
	{ "violet", RGB(238,130,238) },
	{ "white", RGB(255,255,255) },
	{ "whitesmoke", RGB(245,245,245) },
	{ "yellowgreen", RGB(154,205,50) },
};

static struct litcolor5 { const char* cname; COLORREF cref[5]; } litcolor5_ary[] = {

	{ "antiquewhite", { RGB(250,235,215), RGB(255,239,219), RGB(238,223,204), RGB(205,192,176), RGB(139,131,120) }},
	{ "aquamarine", { RGB(127,255,212), RGB(127,255,212), RGB(118,238,198), RGB(102,205,170), RGB(69,139,116) }},
	{ "azure", { RGB(240,255,255), RGB(240,255,255), RGB(224,238,238), RGB(193,205,205), RGB(131,139,139) }},
	{ "bisque", { RGB(255,228,196), RGB(255,228,196), RGB(238,213,183), RGB(205,183,158), RGB(139,125,107) }},
	{ "blue", { RGB(0,0,255), RGB(0,0,255), RGB(0,0,238), RGB(0,0,205), RGB(0,0,139) }},
	{ "brown", { RGB(165,42,42), RGB(255,64,64), RGB(238,59,59), RGB(205,51,51), RGB(139,35,35) }},
	{ "burlywood", { RGB(222,184,135), RGB(255,211,155), RGB(238,197,145), RGB(205,170,125), RGB(139,115,85) }},
	{ "cadetblue", { RGB(95,158,160), RGB(152,245,255), RGB(142,229,238), RGB(122,197,205), RGB(83,134,139) }},
	{ "chartreuse", { RGB(127,255,0), RGB(127,255,0), RGB(118,238,0), RGB(102,205,0), RGB(69,139,0) }},
	{ "chocolate", { RGB(210,105,30), RGB(255,127,36), RGB(238,118,33), RGB(205,102,29), RGB(139,69,19) }},
	{ "coral", { RGB(255,127,80), RGB(255,114,86), RGB(238,106,80), RGB(205,91,69), RGB(139,62,47) }},
	{ "cornsilk", { RGB(255,248,220), RGB(255,248,220), RGB(238,232,205), RGB(205,200,177), RGB(139,136,120) }},
	{ "cyan", { RGB(0,255,255), RGB(0,255,255), RGB(0,238,238), RGB(0,205,205), RGB(0,139,139) }},
	{ "darkgoldenrod", { RGB(184,134,11), RGB(255,185,15), RGB(238,173,14), RGB(205,149,12), RGB(139,101,8) }},
	{ "darkolivegreen", { RGB(85,107,47), RGB(202,255,112), RGB(188,238,104), RGB(162,205,90), RGB(110,139,61) }},
	{ "darkorange", { RGB(255,140,0), RGB(255,127,0), RGB(238,118,0), RGB(205,102,0), RGB(139,69,0) }},
	{ "darkorchid", { RGB(153,50,204), RGB(191,62,255), RGB(178,58,238), RGB(154,50,205), RGB(104,34,139) }},
	{ "darkseagreen", { RGB(143,188,143), RGB(193,255,193), RGB(180,238,180), RGB(155,205,155), RGB(105,139,105) }},
	{ "darkslategray", { RGB(47,79,79), RGB(151,255,255), RGB(141,238,238), RGB(121,205,205), RGB(82,139,139) }},
	{ "deeppink", { RGB(255,20,147), RGB(255,20,147), RGB(238,18,137), RGB(205,16,118), RGB(139,10,80) }},
	{ "deepskyblue", { RGB(0,191,255), RGB(0,191,255), RGB(0,178,238), RGB(0,154,205), RGB(0,104,139) }},
	{ "dodgerblue", { RGB(30,144,255), RGB(30,144,255), RGB(28,134,238), RGB(24,116,205), RGB(16,78,139) }},
	{ "firebrick", { RGB(178,34,34), RGB(255,48,48), RGB(238,44,44), RGB(205,38,38), RGB(139,26,26) }},
	{ "gold", { RGB(255,215,0),  RGB(255,215,0), RGB(238,201,0), RGB(205,173,0), RGB(139,117,0) }},
	{ "goldenrod", { RGB(218,165,32), RGB(255,193,37), RGB(238,180,34), RGB(205,155,29), RGB(139,105,20) }},
	{ "green", { RGB(0,255,0), RGB(0,255,0), RGB(0,238,0), RGB(0,205,0), RGB(0,139,0) }},
	{ "honeydew", { RGB(240,255,240), RGB(240,255,240), RGB(224,238,224), RGB(193,205,193), RGB(131,139,131) }},
	{ "hotpink", { RGB(255,105,180), RGB(255,110,180), RGB(238,106,167), RGB(205,96,144), RGB(139,58,98) }},
	{ "indianred", { RGB(205,92,92), RGB(255,106,106), RGB(238,99,99), RGB(205,85,85), RGB(139,58,58) }},
	{ "ivory", { RGB(255,255,240), RGB(255,255,240), RGB(238,238,224), RGB(205,205,193), RGB(139,139,131) }},
	{ "khaki", { RGB(240,230,140), RGB(255,246,143), RGB(238,230,133), RGB(205,198,115), RGB(139,134,78) }},
	{ "lavenderblush", { RGB(255,240,245), RGB(255,240,245), RGB(238,224,229), RGB(205,193,197), RGB(139,131,134) }},
	{ "lemonchiffon", { RGB(255,250,205), RGB(255,250,205), RGB(238,233,191), RGB(205,201,165), RGB(139,137,112) }},
	{ "lightblue", { RGB(173,216,230), RGB(191,239,255), RGB(178,223,238), RGB(154,192,205), RGB(104,131,139) }},
	{ "lightcyan", { RGB(224,255,255), RGB(224,255,255), RGB(209,238,238), RGB(180,205,205), RGB(122,139,139) }},
	{ "lightgoldenrod", { RGB(238,221,130), RGB(255,236,139), RGB(238,220,130), RGB(205,190,112), RGB(139,129,76) }},
	{ "lightpink", { RGB(255,182,193), RGB(255,174,185), RGB(238,162,173), RGB(205,140,149), RGB(139,95,101) }},
	{ "lightsalmon", { RGB(255,160,122), RGB(255,160,122), RGB(238,149,114), RGB(205,129,98), RGB(139,87,66) }},
	{ "lightskyblue", { RGB(135,206,250), RGB(176,226,255), RGB(164,211,238), RGB(141,182,205), RGB(96,123,139) }},
	{ "lightsteelblue", { RGB(176,196,222), RGB(202,225,255), RGB(188,210,238), RGB(162,181,205), RGB(110,123,139) }},
	{ "lightyellow", { RGB(255,255,224), RGB(255,255,224), RGB(238,238,209), RGB(205,205,180), RGB(139,139,122) }},
	{ "magenta", { RGB(255,0,255), RGB(255,0,255), RGB(238,0,238), RGB(205,0,205), RGB(139,0,139) }},
	{ "maroon", { RGB(176,48,96), RGB(255,52,179), RGB(238,48,167), RGB(205,41,144), RGB(139,28,98) }},
	{ "mediumorchid", { RGB(186,85,211), RGB(224,102,255), RGB(209,95,238), RGB(180,82,205), RGB(122,55,139) }},
	{ "mediumpurple", { RGB(147,112,219), RGB(171,130,255), RGB(159,121,238), RGB(137,104,205), RGB(93,71,139) }},
	{ "mistyrose", { RGB(255,228,225), RGB(255,228,225), RGB(238,213,210), RGB(205,183,181), RGB(139,125,123) }},
	{ "navajowhite", { RGB(255,222,173), RGB(255,222,173), RGB(238,207,161), RGB(205,179,139), RGB(139,121,94) }},
	{ "olivedrab", { RGB(107,142,35), RGB(192,255,62), RGB(179,238,58), RGB(154,205,50), RGB(105,139,34) }},
	{ "orange", { RGB(255,165,0), RGB(255,165,0), RGB(238,154,0), RGB(205,133,0), RGB(139,90,0) }},
	{ "orangered", { RGB(255,69,0), RGB(255,69,0), RGB(238,64,0), RGB(205,55,0), RGB(139,37,0) }},
	{ "orchid", { RGB(218,112,214), RGB(255,131,250), RGB(238,122,233), RGB(205,105,201), RGB(139,71,137) }},
	{ "palegreen", { RGB(152,251,152), RGB(154,255,154), RGB(144,238,144), RGB(124,205,124), RGB(84,139,84) }},
	{ "paleturquoise", { RGB(175,238,238), RGB(187,255,255), RGB(174,238,238), RGB(150,205,205), RGB(102,139,139) }},
	{ "palevioletred", { RGB(219,112,147), RGB(255,130,171), RGB(238,121,159), RGB(205,104,137), RGB(139,71,93) }},
	{ "peachpuff", { RGB(255,218,185), RGB(255,218,185), RGB(238,203,173), RGB(205,175,149), RGB(139,119,101) }},
	{ "pink", { RGB(255,192,203), RGB(255,181,197), RGB(238,169,184), RGB(205,145,158), RGB(139,99,108) }},
	{ "plum", { RGB(221,160,221), RGB(255,187,255), RGB(238,174,238), RGB(205,150,205), RGB(139,102,139) }},
	{ "purple", { RGB(160,32,240), RGB(155,48,255), RGB(145,44,238), RGB(125,38,205), RGB(85,26,139) }},
	{ "red", { RGB(255,0,0), RGB(255,0,0), RGB(238,0,0), RGB(205,0,0), RGB(139,0,0) }},
	{ "rosybrown", { RGB(188,143,143), RGB(255,193,193), RGB(238,180,180), RGB(205,155,155), RGB(139,105,105) }},
	{ "royalblue", { RGB(65,105,225), RGB(72,118,255), RGB(67,110,238), RGB(58,95,205), RGB(39,64,139) }},
	{ "salmon", { RGB(250,128,114), RGB(255,140,105), RGB(238,130,98), RGB(205,112,84), RGB(139,76,57) }},
	{ "seagreen", { RGB(46,139,87), RGB(84,255,159), RGB(78,238,148), RGB(67,205,128), RGB(46,139,87) }},
	{ "seashell", { RGB(255,245,238), RGB(255,245,238), RGB(238,229,222), RGB(205,197,191), RGB(139,134,130) }},
	{ "sienna", { RGB(160,82,45), RGB(255,130,71), RGB(238,121,66), RGB(205,104,57), RGB(139,71,38) }},
	{ "skyblue", { RGB(135,206,235), RGB(135,206,255), RGB(126,192,238), RGB(108,166,205), RGB(74,112,139) }},
	{ "slateblue", { RGB(106,90,205), RGB(131,111,255), RGB(122,103,238), RGB(105,89,205), RGB(71,60,139) }},
	{ "slategray", { RGB(112,128,144), RGB(198,226,255), RGB(185,211,238), RGB(159,182,205), RGB(108,123,139) }},
	{ "snow", { RGB(255,250,250), RGB(255,250,250), RGB(238,233,233), RGB(205,201,201), RGB(139,137,137) }},
	{ "springgreen", { RGB(0,255,127), RGB(0,255,127), RGB(0,238,118), RGB(0,205,102), RGB(0,139,69) }},
	{ "steelblue", { RGB(70,130,180), RGB(99,184,255), RGB(92,172,238), RGB(79,148,205), RGB(54,100,139) }},
	{ "tan", { RGB(210,180,140), RGB(255,165,79), RGB(238,154,73), RGB(205,133,63), RGB(139,90,43) }},
	{ "thistle", { RGB(216,191,216), RGB(255,225,255), RGB(238,210,238), RGB(205,181,205), RGB(139,123,139) }},
	{ "tomato", { RGB(255,99,71), RGB(255,99,71), RGB(238,92,66), RGB(205,79,57), RGB(139,54,38) }},
	{ "turquoise", { RGB(64,224,208), RGB(0,245,255), RGB(0,229,238), RGB(0,197,205), RGB(0,134,139) }},
	{ "violetred", { RGB(208,32,144), RGB(255,62,150), RGB(238,58,140), RGB(205,50,120), RGB(139,34,82) }},
	{ "wheat", { RGB(245,222,179), RGB(255,231,186), RGB(238,216,174), RGB(205,186,150), RGB(139,126,102) }},
	{ "yellow", { RGB(255,255,0), RGB(255,255,0), RGB(238,238,0), RGB(205,205,0), RGB(139,139,0) }}
};
int iminmax(int a, int b, int c)
{
	if (a > c) a = c;
	if (a < b) a = b;
	return a;
}
/* ------------------------------------------------------------------------- */
/* Function: ParseLiteralColor */
/* Purpose: Parses a given literal color and returns the hex value */

COLORREF ParseLiteralColor(LPCWSTR color)
{
	int i, n, s; unsigned l; wchar_t* p, c, buf[32]; const char* cp;

	l = (int)wcslen(color) + 1;
	if (l > sizeof buf)
		return (COLORREF)-1;

	memcpy(buf, color, l);
	buf[sizeof buf - 1] = '\0';

	while (NULL != (p = wcschr(buf, ' ')))
		wcscpy(p, p + 1), --l;

	if (l < 3)
		return (COLORREF)-1;

	if (NULL != (p = wcsstr(buf, L"grey")))
		p[2] = 'a';

	if (0 == memcmp(buf, "gray", 4) && (c = buf[4]) >= '0' && c <= '9') {
		i = iminmax(_wtoi(buf + 4), 0, 100);
		i = (i * 255 + 50) / 100;
		return rgb(i, i, i);
	}

	i = *(p = &buf[l - 2]) - '0';
	if (i >= 1 && i <= 4)
		*p = 0, --l;
	else
		i = 0;

	cp = (const char*)litcolor5_ary;
	n = sizeof litcolor5_ary / sizeof litcolor5_ary[0];
	s = sizeof litcolor5_ary[0];
	for (;;) {
		do {
			if (*buf <= **(const char**)cp)
				break;
		} while (cp += s, --n);
		do {
			if (*buf < **(const char**)cp)
				break;
			if (0 == memcmp(buf, *(const char**)cp, l))
				return ((struct litcolor5*)cp)->cref[i];
		} while (cp += s, --n);

		if (i || s == sizeof litcolor1_ary[0])
			return (COLORREF)-1;

		cp = (const char*)litcolor1_ary;
		n = sizeof litcolor1_ary / sizeof litcolor1_ary[0];
		s = sizeof litcolor1_ary[0];
	}
}

COLORREF readColorFromString(const wchar_t* string)
{
	wchar_t stub[32];
	wchar_t rgbstr[32];
	wchar_t* s, * d, * r, c;
	COLORREF cr;

	if (NULL == string)
		return CLR_INVALID;

	//s = _strlwr(unquote(strcpy_max(stub, string, sizeof stub)));
	wcscpy(stub, string);
	s = _wcslwr(stub);

	/* check if its an "rgb:12/ee/4c" type string */
	if (0 == memcmp(s, "rgb:", 4)) {
		int j = 3;
		s += 4, d = rgbstr, r = s;
		for (;;) {
			d[0] = *r && '/' != *r ? *r++ : '0';
			d[1] = *r && '/' != *r ? *r++ : d[0];
			d += 2;
			if (0 == --j)
				break;
			if ('/' != *r)
				goto check_hex;
			++r;
		}
		*d = 0, s = rgbstr;
	}
check_hex:
	/* check if its a valid hex number */
	if ('#' == *s)
		s++;
	for (cr = 0, d = s; (c = *d) != 0; ++d) {
		cr <<= 4;
		if (c >= '0' && c <= '9')
			cr |= c - '0';
		else
			if (c >= 'a' && c <= 'f')
				cr |= c - ('a' - 10);
			else /* must be a literal color name (or is invalid) */
				return ParseLiteralColor(s);
	}
	/* #AB4 short type colors */
	if (d - s == 3)
		cr = ((cr & 0xF00) << 12) | ((cr & 0xFF0) << 8) | ((cr & 0x0FF) << 4) | (cr & 0x00F);
	return switchRgb(cr);
}


/* ------------------------------------------------------------------------- */
