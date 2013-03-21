/* bwassert.h -- assert

Copyright (c) 1996-2013, Brian Bray

*/

namespace bw {

class BAssert {
public:
	static void doassert( const char* msg,
	                      const char* fileName, int lineNum,
	                      bool isFatal=true );
};

}	//Namespace bw

#ifdef BWASSERTDISCARD

#define bwassert( X )
#define bwverify( X ) {if(!(X)) {bw::BAssert::doassert( #X, __FILE__, __LINE__ , true );} }

#else

#define bwassert( X ) {if(!(X)) {bw::BAssert::doassert( #X, __FILE__, __LINE__ );} }
#define bwverify( X ) {if(!(X)) {bw::BAssert::doassert( #X, __FILE__, __LINE__ , false );} }

#endif

