#include "ParserStream.hpp"

namespace dcpu
{
char ParserStream::get()
{
	if(r_is.eof())
		return r_is.get();
	// Line count
	if(isEndOfLine(m_lastChar))
	{
#ifdef WINDOWS // CR+LF
		if(r_is.peek() == '\n')
		{
			++m_row;
			m_col = 0;
		}
#else // POSIX: CR or LF only
		++m_row;
		m_col = 0;
#endif
	}
	else
		++m_col;

	m_lastChar = r_is.get();
	return m_lastChar;
}

// Puts read characters into os, starting from startPos to endPos.
// Does nothing if pos >= tellg().
// if endPos > tellg(), it is changed to tellg().
void ParserStream::getReadData(
		std::ostream & os,
		std::streampos startPos,
		std::streampos endPos)
{
	if(startPos < 0)
		return;

	if(endPos >= 0) // -1 means "end of stream"
	{
		if(endPos > r_is.tellg())
			endPos = r_is.tellg();
		if(startPos >= endPos)
			return;
	}

	// Memorize original pos (will be -1 if eof is set)
	std::streampos originalPos = r_is.tellg();

	if(r_is.eof())
		r_is.clear(); // clear error flags
	// Go to start pos
	r_is.seekg(startPos, std::ios::beg);

	// Read until endPos or end of stream
	char c;
	while(!r_is.eof() && r_is.tellg() != endPos)
	{
		c = r_is.get();
		if(r_is.eof())
		{
			r_is.clear();
			break;
		}
		os.put(c);
	}

	// Go back to original pos
	if(originalPos >= 0)
		r_is.seekg(originalPos, std::ios::beg);
	else
		r_is.seekg(0, std::ios::end); // end of stream
}

} // namespace dcpu



