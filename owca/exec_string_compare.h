#ifndef _RC_Y_EXEC_STRING_COMPARE_H
#define _RC_Y_EXEC_STRING_COMPARE_H

namespace owca {
	namespace __owca__ {
		class owca_internal_string;

		class string_compare {
		public:
			bool operator () (const owca_internal_string *p1, const owca_internal_string *p2) const;
		};

	}
}

#endif
