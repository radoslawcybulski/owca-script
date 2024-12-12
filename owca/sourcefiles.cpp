#include "stdafx.h"
#include "base.h"
#include "sourcefile.h"
#include "virtualmachine.h"
#undef RCPRINTF
#include <stdio.h>

namespace owca {
	owca_source_file_Text::owca_source_file_Text(const char *txt_) : txt(txt_), end(txt_)
	{
		while (*end != 0) ++end;
	}
	int owca_source_file_Text::next_line(const char *&src) const
	{
		if (txt == end)
			return -1;

		src = txt;
		while (txt != end && *txt != '\n') ++txt;

		if (txt == end)
			return (int)(txt - src);
		
		++txt;
		return (int)(txt - src) - 1;
	}

	owca_source_file_Text_array::owca_source_file_Text_array(const char **txt_, unsigned int cnt_) : txt(txt_), cnt(cnt_), index(0)
	{
		if (cnt == std::numeric_limits<unsigned int>::max()) {
			cnt = 0;
			while (txt[cnt] != NULL) ++cnt;
		}
	}

	int owca_source_file_Text_array::next_line(const char *&src) const
	{
		if (index < cnt) {
			src = txt[index++];
			const char *s = src;
			while (*s) ++s;
			return (int)(s - src);
		}
		return -1;
	}
}
