/* context.h -- Interface for the Context class

Copyright (C) 1997-2013, Brian Bray

*/

namespace bw {

class Context {
public:
	Context();
	virtual ~Context();
	void addClip( int x, int y, unsigned int width, unsigned int height );
	void invalidate();
	bool needRedraw() {
		return m_isDirty;
	}
	void reset();

private:
	bool	m_isDirty;
};

}	//namespace bw
