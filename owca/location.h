#ifndef _RC_Y_LOCATION_H
#define _RC_Y_LOCATION_H

namespace owca {
	class owca_location;
}

namespace owca {
	class owca_location {
		const char *filename_;
		unsigned int line_;
	public:
		owca_location(unsigned int line__, const char *filename__) : line_(line__),filename_(filename__) { }
		owca_location(): line_(0),filename_("") { }

		unsigned int line() const { return line_; }
		const char *filename() const { return filename_; }

		bool operator < (const owca_location &l) const {
			return line()<l.line();
		}
		bool operator > (const owca_location &l) const { return l < *this; }
	};
}
#endif
