// Main program to excercise string routines
//

#include <iostream>

#include <bw/bwassert.h>
#include <bw/string.h>

using namespace bw;

int main(int, char**)
{
	// Constructors
	String str1;
	String str2(42);
	String str3 = "StrinG 3 and 4";
	String str4 = str3;
	bwverify( str3==str4 );

	String str5 = "abc";
	String str6 = "def";
	String str7;
	String str8(14), str9(15), str10(16), str11(17);

	// Member functions
	int i = str3.length();
	bwverify( i==14 );
	str10.append( str11 );
	bwverify( str10.length()==0 );
	str10 += str11;
	bwverify( str10.length()==0 );
	str9.append( "654" );
	bwverify( str9=="654" );
	str9 += "321";
	bwverify( str9=="654321" );
	str9.append( '0' );
	bwverify( str9.length()==7 );
	bwverify( str9=="6543210" );
	(str10 += '0') += '1';
	bwverify( str10=="01" );
	str3.ensureCapacity( 1000 );
	for (i=0; i<10; i++) {
		str3.append( str4 );
		str8 += str9;
	}
	int max3=str3.capacity();
	bwverify( max3>=140 );
	int max4=str8.capacity();
	bwverify( max4>=70 );

	String s("abc");
	String ss("aaa");
	String se("abc");
	String sb("ccc");
	String sei("ABC");
	String snei("aBd");
	String slb("b");
	const char* ps="aaa";
	const char* pe="abc";
	const char* pb="ccc";
	const char* pei="AbC";
	const char* pnei="AbD";
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
	bwverify(   s.startsWith("ab") );

	bwverify(   (s.indexOf('b')==1) );
	bwverify(   (s.indexOf('a')==0) );
	bwverify(   (s.indexOf('A')==-1) );
	bwverify(   (s.indexOf('\0')==3) );

	bwverify(   (s.indexOf(slb)==1) );
	bwverify(   (s.indexOf(se)==0) );
	bwverify(   (s.indexOf(ss)==-1) );

	bwverify(   (s.lastIndexOf('b')==1) );
	bwverify(   (s.lastIndexOf('a')==0) );
	bwverify(   (s.lastIndexOf('A')==-1) );
	bwverify(   (s.lastIndexOf('\0')==3) );

	bwverify(   (s.lastIndexOf(slb)==1) );
	bwverify(   (s.lastIndexOf(se)==0) );
	bwverify(   (s.lastIndexOf(ss)==-1) );
	bwverify(   (s.lastIndexOf("ab")==0) );
	bwverify(   (s.lastIndexOf("c")==2) );

	str1 = str4.substring(7);
	str2 = str4.substring(0,7);

	str1.toUpperCase();
	str2.toLowerCase();

	// Operators
	str7 = str5;
	str7 = str7;
	str7 = (const char*)str7;
	str8 = str5 + str6;
	str9 = str5 + "abcdefghijklmnopqrstuvwxyz";
	str11 = "xyz" + str5;
	str4[3] = 'a';
	char ch = str4[7];
	bwverify( ch=='3' );
	char ch2 = str4[13];
	bwverify( ch2=='4' );
	const char* foo=str8;
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
	char* bar=str8;		//This shouldn't compile
#endif

#ifdef NO_RUN
	char ch3 = str4[14];	//This should assert
#endif
}
