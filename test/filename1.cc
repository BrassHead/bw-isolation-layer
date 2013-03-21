/* fntest -- FileName object test

Copyright (C) 1997-2013, Brian Bray

*/

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/filename.h"

int main(int, char**)
{
	using bw::FileName;

	// This section is the unit test for FileNames
	FileName fn1 = "readme.txt";
	bwverify( fn1.root()=="" );
	bwverify( fn1.directory()=="" );
	bwverify( fn1.baseName()=="readme" );
	bwverify( fn1.extension()==".txt" );
	bwverify( !fn1.isAbsolute() );
	bwverify( !fn1.isPattern() );

	FileName fn2 = "/readme.txt";
	bwverify( fn2.root()=="/" );
	bwverify( fn2.directory()=="" );
	bwverify( fn2.baseName()=="readme" );
	bwverify( fn2.extension()==".txt" );
	bwverify( !fn2.isPattern() );

#ifdef _WIN32
	bwverify( !fn2.isAbsolute() );

	FileName fn3 = "d:\\readme.txt";
	bwverify( fn3.root()=="d:\\" );
	bwverify( fn3.directory()=="" );
	bwverify( fn3.baseName()=="readme" );
	bwverify( fn3.extension()==".txt" );
	bwverify( fn3.isAbsolute() );
	bwverify( !fn3.isPattern() );

	FileName fn4 = "c:readme.txt";
	bwverify( fn4.root()=="c:" );
	bwverify( fn4.directory()=="" );
	bwverify( fn4.baseName()=="readme" );
	bwverify( fn4.extension()==".txt" );
	bwverify( !fn4.isAbsolute() );
	bwverify( !fn4.isPattern() );

	FileName fn5 = "d:\\src/readme";
	bwverify( fn5.root()=="d:\\" );
	bwverify( fn5.directory()=="src/" );
	bwverify( fn5.baseName()=="readme" );
	bwverify( fn5.extension()=="" );
	bwverify( fn5.isAbsolute() );
	bwverify( !fn5.isPattern() );

	FileName fn6 = "\\\\host\\c\\src\\rm";
	bwverify( fn6.root()=="\\\\" );
	bwverify( fn6.directory()=="host\\c\\src\\" );
	bwverify( fn6.baseName()=="rm" );
	bwverify( fn6.extension()=="" );
	bwverify( fn6.isAbsolute() );
	bwverify( !fn6.isPattern() );

	FileName fn7 = "D:\\";
	bwverify( fn7.root()=="D:\\" );
	bwverify( fn7.directory()=="" );
	bwverify( fn7.baseName()=="" );
	bwverify( fn7.extension()=="" );
	bwverify( fn7.isAbsolute() );
	bwverify( !fn7.isPattern() );

	FileName fn8 = "D:\\src.d";
	bwverify( fn8.root()=="D:\\" );
	bwverify( fn8.directory()=="" );
	bwverify( fn8.baseName()=="src" );
	bwverify( fn8.extension()==".d" );
	bwverify( fn8.isAbsolute() );
	bwverify( !fn8.isPattern() );

	FileName fn9("D:\\src.d\\");
	bwverify( fn9.root()=="D:\\" );
	bwverify( fn9.directory()=="src.d\\" );
	bwverify( fn9.baseName()=="" );
	bwverify( fn9.extension()=="" );
	bwverify( fn9.isAbsolute() );
	bwverify( !fn9.isPattern() );

	FileName fn10("D:\\src.d\\*.*");
	bwverify( fn10.root()=="D:\\" );
	bwverify( fn10.directory()=="src.d\\" );
	bwverify( fn10.baseName()=="*" );
	bwverify( fn10.extension()==".*" );
	bwverify( fn10.isAbsolute() );
	bwverify( fn10.isPattern() );

	FileName fn11("D:/src/..");
	bwverify( fn11.root()=="D:/" );
	bwverify( fn11.directory()=="src/" );
	bwverify( fn11.baseName()==".." );
	bwverify( fn11.extension()=="" );
	bwverify( fn11.isAbsolute() );
	bwverify( !fn11.isPattern() );

	FileName fn12("D:/src/.");
	bwverify( fn12.root()=="D:/" );
	bwverify( fn12.directory()=="src/" );
	bwverify( fn12.baseName()=="." );
	bwverify( fn12.extension()=="" );
	bwverify( fn12.isAbsolute() );
	bwverify( !fn12.isPattern() );

	FileName fn13("D:\\src\\.foo");
	bwverify( fn13.root()=="D:\\" );
	bwverify( fn13.directory()=="src\\" );
	bwverify( fn13.baseName()=="" );
	bwverify( fn13.extension()==".foo" );
	bwverify( fn13.isAbsolute() );
	bwverify( !fn13.isPattern() );

	FileName fn14 = "//host/c\\src.*\\rm";
	bwverify( fn14.root()=="//" );
	bwverify( fn14.directory()=="host/c\\src.*\\" );
	bwverify( fn14.baseName()=="rm" );
	bwverify( fn14.extension()=="" );
	bwverify( fn14.isAbsolute() );
	bwverify( !fn14.isPattern() );

	fn3.makeAbsolute();
	fn4.makeAbsolute();

#else
	bwverify( fn2.isAbsolute() );
#endif

	FileName fn15 = "/usr/src/linux";
	bwverify( fn15.root()=="/" );
	bwverify( fn15.directory()=="usr/src/" );
	bwverify( fn15.baseName()=="linux" );
	bwverify( fn15.extension()=="" );
	bwverify( fn15.isAbsolute() );
	bwverify( !fn15.isPattern() );

	fn15.empty();
	bwverify( *(const char*)fn15=='\0' );

	fn15 = FileName::currentDirectory();

	FileName fn16 = "//host/usr/src/linux.d";
	bwverify( fn16.root()=="//" );
	bwverify( fn16.directory()=="host/usr/src/" );
	bwverify( fn16.baseName()=="linux" );
	bwverify( fn16.extension()==".d" );
	bwverify( fn16.isAbsolute() );
	bwverify( !fn16.isPattern() );

	FileName fn17 = "//host/usr/src/linux.d/";
	bwverify( fn17.root()=="//" );
	bwverify( fn17.directory()=="host/usr/src/linux.d/" );
	bwverify( fn17.baseName()=="" );
	bwverify( fn17.extension()=="" );
	bwverify( fn17.isAbsolute() );
	bwverify( !fn17.isPattern() );

	FileName fn18 = "../src/*";
	bwverify( fn18.root()=="" );
	bwverify( fn18.directory()=="../src/" );
	bwverify( fn18.baseName()=="*" );
	bwverify( fn18.extension()=="" );
	bwverify( !fn18.isAbsolute() );
	bwverify( fn18.isPattern() );

	FileName fn19 = "/src/*.foo";
	bwverify( fn19.root()=="/" );
	bwverify( fn19.directory()=="src/" );
	bwverify( fn19.baseName()=="*" );
	bwverify( fn19.extension()==".foo" );
	bwverify( fn19.isAbsolute() );
	bwverify( fn19.isPattern() );

	FileName fn20 = "./src/*foo*";
	bwverify( fn20.root()=="" );
	bwverify( fn20.directory()=="./src/" );
	bwverify( fn20.baseName()=="*foo*" );
	bwverify( fn20.extension()=="" );
	bwverify( !fn20.isAbsolute() );
	bwverify( fn20.isPattern() );

	fn1.makeAbsolute();
	fn2.makeAbsolute();
	fn18.makeAbsolute();
	fn20.makeAbsolute();


}
