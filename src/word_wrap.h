#ifndef _WORD_WRAP_H_
#define _WORD_WRAP_H_

#include <string>

#include "font.h"

std::string wordWrap(const Font &font, const std::string &text, int width);

#endif  // _WORD_WRAP_H_
