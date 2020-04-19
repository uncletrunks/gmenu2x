#ifndef _FONT_SPEC_H_
#define _FONT_SPEC_H_

#include <string>

struct FontSpec {
	std::string path;
	unsigned int size;
	bool operator==(const FontSpec &other) const {
		return other.path == path && other.size == size;
	}
	bool operator!=(const FontSpec &other) const { return !(*this == other); }
};

#endif  // _FONT_SPEC_H_
