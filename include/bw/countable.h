/* countable.h -- smart pointer and Countable base class

Copyright (C) 1997-2013, Brian Bray

*/

/*
#include "bw/bwassert.h"
*/

/*: class cptr

	This template class acts like a pointer, but maintains a reference
	count to the pointed to item.  Assignment and construction using a regular
	pointer do not increase the reference count, but all other assigments
	and constructors do.

	Note that changing the reference count is considered a 'const' operation
	on the object.

    TODO: deprecated for C++11 builtin smart pointers
*/
template<class C>
// C is a countable class -- ie: it has AddRef and Release methods
class cptr {
public:
	cptr(const C* ptr=0)
		: m_ptr( (C*)ptr ) {}
	cptr(const cptr<C>& cp) {
		m_ptr=((cptr<C>&)cp).m_ptr;
		if (m_ptr) m_ptr->AddRef();
	}
	~cptr() {
		deallocate();
	}

	cptr<C>& operator=(const C* ptr) {
		deallocate();
		m_ptr=(C*)ptr;
		return *this;
	}
	cptr<C>& operator=(const cptr<C>& cp) {
		deallocate();
		m_ptr=((cptr<C>&)cp).m_ptr;
		if (m_ptr) m_ptr->AddRef();
		return *this;
	}

	C* operator->() const {
		bwassert(m_ptr);
		return m_ptr;
	}
	C& operator*() const {
		bwassert(m_ptr);
		return *m_ptr;
	}
	operator C*() const {
		return m_ptr;
	}

	bool operator==(const cptr<C>& cp) const {
		return m_ptr==((cptr<C>&)cp).m_ptr;
	}
	bool operator!=(const cptr<C>& cp) const {
		return m_ptr!=((cptr<C>&)cp).m_ptr;
	}

private:
	void deallocate() {
		if (m_ptr) m_ptr->Release();
		m_ptr=0;
	}

	C*	m_ptr;
};

/*: class Countable

	Base class for countable objects.  Defines AddRef() and Release().
*/
class Countable {
public:
	Countable() : m_cRef(1) {}
	virtual unsigned long AddRef() {
		return ++m_cRef;
	}
	virtual unsigned long Release() {
		if (--m_cRef) return m_cRef;
		delete this;
		return 0;
	}
protected:
	virtual ~Countable() {
		bwassert(m_cRef<=1);   // Note: 1 for throw in constructor
	}
	unsigned long m_cRef;
};

