/* Cptr tests

Copyright (C) 1999-2013 Brian Bray

*/

#include <bw/bwassert.h>
#include <bw/countable.h>
#include <bw/string.h>
#include <map>

using std::map;

class Testme;
typedef cptr<Testme> TestmeRef;

class Testme : public Countable {
public:
	Testme() {
		++myCount;
	}
	~Testme() {
		--myCount;
	}
	static int count() {
		return myCount;
	}
	int mycount() {
		return m_cRef;
	}
	TestmeRef ref() {
		AddRef();
		return this;
	}

private:
	static int myCount;
};

int Testme::myCount = 0;

int main(int, char**)
{
	using bw::String;

	// This section is the unit test for cptrs
	bwverify(Testme::count()==0);

	TestmeRef zero = 0;
	bwverify(Testme::count()==0);

	TestmeRef one = new Testme;
	bwverify(Testme::count()==1);
	bwverify(one->mycount()==1);

	TestmeRef two = one->ref();
	bwverify(Testme::count()==1);
	bwverify(one->mycount()==2);
	bwverify(one==two);

	TestmeRef three = new Testme;
	bwverify(Testme::count()==2);
	bwverify(one->mycount()==2);
	bwverify(three->mycount()==1);
	bwverify(one==two);

	TestmeRef four = three;
	bwverify(Testme::count()==2);
	bwverify(one->mycount()==2);
	bwverify(three->mycount()==2);
	bwverify(one==two);

	one = (Testme*)0;
	bwverify(Testme::count()==2);
	bwverify(two->mycount()==1);
	bwverify(three->mycount()==2);
	bwverify(one==zero);

	{
		map<String,TestmeRef> tmap;
		tmap["foo"] = two->ref();
		bwverify(Testme::count()==2);
		bwverify(two->mycount()==2);
		bwverify(three->mycount()==2);
		bwverify(one==zero);

		tmap["bar"] = three;
		bwverify(Testme::count()==2);
		bwverify(two->mycount()==2);
		bwverify(three->mycount()==3);
		bwverify(one==zero);

	}

	bwverify(Testme::count()==2);
	bwverify(two->mycount()==1);
	bwverify(three->mycount()==2);
	bwverify(one==zero);

	//bwverify(false);
	//bwassert(false);

}

