#ifndef _RC_Y_COMPILE_VISIBLE_ITEMS_H
#define _RC_Y_COMPILE_VISIBLE_ITEMS_H

namespace owca {
	namespace __owca__ {

		class compile_visible_items {
			mutable  unsigned int builtinindex;
		protected:
			compile_visible_items() : builtinindex(0) { }
		public:
			DLLEXPORT virtual const char *builtinget() const;
			virtual const char *get(unsigned int &) const = 0;
		};
	}
}

#endif