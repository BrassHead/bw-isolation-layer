// Main program to excercise string routines
//

#include <iostream>

#include <bw/bwassert.h>
#include <bw/ustring.h>

using namespace bw;

using std::cout;
using std::endl;

int main(int, char**)
{
	cout << "size of wchar is: " << sizeof(wchar_t) << endl;

	// Constructors
	UString str1;
	UString str2(42);
	UString str3 = L"StrinG 3 and 4";
	UString str4 = str3;
	bwverify( str3==str4 );

	UString str5 = L"abc";
	UString str6 = L"def";
	UString str7;
	UString str8(14), str9(15), str10(16), str11(17);

	// Member functions
	int i = str3.length();
	bwverify( i==14 );
	str10.append( str11 );
	bwverify( str10.length()==0 );
	str10 += str11;
	bwverify( str10.length()==0 );
	str9.append( L"654" );
	bwverify( str9==L"654" );
	str9 += L"321";
	bwverify( str9==L"654321" );
	str9.append( L'0' );
	bwverify( str9.length()==7 );
	bwverify( str9==L"6543210" );
	(str10 += L'0') += L'1';
	bwverify( str10==L"01" );
	str3.ensureCapacity( 1000 );
	for (i=0; i<10; i++) {
		str3.append( str4 );
		str8 += str9;
	}
	int max3=str3.capacity();
	bwverify( max3>=140 );
	int max4=str8.capacity();
	bwverify( max4>=70 );

	UString s(L"abc");
	UString ss(L"aaa");
	UString se(L"abc");
	UString sb(L"ccc");
	UString sei(L"ABC");
	UString snei(L"aBd");
	UString slb(L"b");
	const wchar_t* ps=L"aaa";
	const wchar_t* pe=L"abc";
	const wchar_t* pb=L"ccc";
	const wchar_t* pei=L"AbC";
	const wchar_t* pnei=L"AbD";
	int comp;

	comp = s.compareTo(ss);
	bwverify( comp > 0 );
	comp = s.compareTo(se);
	bwverify( comp == 0 );
	comp = s.compareTo(sb);
	bwverify( comp < 0 );
	comp = s.compareTo(sei);
	bwverify( comp > 0 );

	comp = s.compareTo(ps);
	bwverify( comp > 0 );
	comp = s.compareTo(pe);
	bwverify( comp == 0 );
	comp = s.compareTo(pb);
	bwverify( comp < 0 );
	comp = s.compareTo(pei);
	bwverify( comp > 0 );

	bwverify( ! s.equals(ss) );
	bwverify(   s.equals(se) );
	bwverify( ! s.equals(sb) );
	bwverify( ! s.equals(sei) );

	bwverify( ! s.equals(ps) );
	bwverify(   s.equals(pe) );
	bwverify( ! s.equals(pb) );
	bwverify( ! s.equals(pei) );

	bwverify( ! s.equalsIgnoreCase(ss) );
	bwverify(   s.equalsIgnoreCase(se) );
	bwverify( ! s.equalsIgnoreCase(sb) );
	bwverify(   s.equalsIgnoreCase(sei) );
	bwverify( ! s.equalsIgnoreCase(snei) );

	bwverify( ! s.equalsIgnoreCase(ps) );
	bwverify(   s.equalsIgnoreCase(pe) );
	bwverify( ! s.equalsIgnoreCase(pb) );
	bwverify(   s.equalsIgnoreCase(pei) );
	bwverify( ! s.equalsIgnoreCase(pnei) );

	bwverify( ! s.startsWith(ss) );
	bwverify(   s.startsWith(se) );
	bwverify( ! s.startsWith(ps) );
	bwverify(   s.startsWith(pe) );
	bwverify(   s.startsWith(L"ab") );

	bwverify(   (s.indexOf(L'b')==1) );
	bwverify(   (s.indexOf(L'a')==0) );
	bwverify(   (s.indexOf(L'A')==-1) );
	bwverify(   (s.indexOf(L'\0')==3) );

	bwverify(   (s.indexOf(slb)==1) );
	bwverify(   (s.indexOf(se)==0) );
	bwverify(   (s.indexOf(ss)==-1) );

	bwverify(   (s.lastIndexOf(L'b')==1) );
	bwverify(   (s.lastIndexOf(L'a')==0) );
	bwverify(   (s.lastIndexOf(L'A')==-1) );
	bwverify(   (s.lastIndexOf(L'\0')==3) );

	bwverify(   (s.lastIndexOf(slb)==1) );
	bwverify(   (s.lastIndexOf(se)==0) );
	bwverify(   (s.lastIndexOf(ss)==-1) );
	bwverify(   (s.lastIndexOf(L"ab")==0) );
	bwverify(   (s.lastIndexOf(L"c")==2) );

	str1 = str4.substring(7);
	str2 = str4.substring(0,7);

	str1.toUpperCase();
	str2.toLowerCase();

	// Operators
	str7 = str5;
	str7 = str7;
	str7 = (const wchar_t*)str7;
	str8 = str5 + str6;
	str9 = str5 + L"abcdefghijklmnopqrstuvwxyz";
	str11 = L"xyz" + str5;
	str4[3] = L'a';
	wchar_t ch = str4[7];
	bwverify( ch==L'3' );
	wchar_t ch2 = str4[13];
	bwverify( ch2==L'4' );
	const wchar_t* foo=str8;
	bwverify( foo==str8 );

	bwverify(   (s==s) );
	bwverify( ! (s==ss) );
	bwverify(   (s==se) );
	bwverify( ! (s==sb) );
	bwverify( ! (s==ps) );
	bwverify(   (s==pe) );
	bwverify( ! (s==pb) );
	bwverify( ! (ps==s) );
	bwverify(   (pe==s) );
	bwverify( ! (pb==s) );

	bwverify( ! (s!=s) );
	bwverify(   (s!=ss) );
	bwverify( ! (s!=se) );
	bwverify(   (s!=sb) );
	bwverify(   (s!=ps) );
	bwverify( ! (s!=pe) );
	bwverify(   (s!=pb) );
	bwverify(   (ps!=s) );
	bwverify( ! (pe!=s) );
	bwverify(   (pb!=s) );

	bwverify(   (s<=s) );
	bwverify( ! (s<=ss) );
	bwverify(   (s<=se) );
	bwverify(   (s<=sb) );
	bwverify( ! (s<=ps) );
	bwverify(   (s<=pe) );
	bwverify(   (s<=pb) );
	bwverify(   (ps<=s) );
	bwverify(   (pe<=s) );	//Error
	bwverify( ! (pb<=s) );

	bwverify(   (s>=s) );
	bwverify(   (s>=ss) );
	bwverify(   (s>=se) );
	bwverify( ! (s>=sb) );
	bwverify(   (s>=ps) );
	bwverify(   (s>=pe) );
	bwverify( ! (s>=pb) );
	bwverify( ! (ps>=s) );
	bwverify(   (pe>=s) );	//Error
	bwverify(   (pb>=s) );

	bwverify( ! (s<s) );
	bwverify( ! (s<ss) );
	bwverify( ! (s<se) );
	bwverify(   (s<sb) );
	bwverify( ! (s<ps) );
	bwverify( ! (s<pe) );
	bwverify(   (s<pb) );
	bwverify(   (ps<s) );
	bwverify( ! (pe<s) );	//Error
	bwverify( ! (pb<s) );

	bwverify( ! (s>s) );
	bwverify(   (s>ss) );
	bwverify( ! (s>se) );
	bwverify( ! (s>sb) );
	bwverify(   (s>ps) );
	bwverify( ! (s>pe) );
	bwverify( ! (s>pb) );
	bwverify( ! (ps>s) );
	bwverify( ! (pe>s) );	//Error
	bwverify(   (pb>s) );



	// Error checking
#ifdef NO_COMPILE
	wchar_t* bar=str8;		//This shouldn't compile
#endif

#ifdef NO_RUN
	wchar_t ch3 = str4[14];	//This should assert
#endif
}
