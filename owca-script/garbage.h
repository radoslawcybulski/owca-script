#ifndef RC_OWCA_SCRIPT_GARBAGE_H
#define RC_OWCA_SCRIPT_GARBAGE_H

namespace OwcaScript {
	class GenerationGC {
		unsigned int value;

	public:
		explicit GenerationGC(unsigned int value) : value(value) {}

		bool operator == (GenerationGC other) const { return value == other.value; }
		bool operator != (GenerationGC other) const { return !(*this == other); }
	};
}

#endif
