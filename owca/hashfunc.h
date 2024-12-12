#ifndef _RC_Y_HASHFUNC_H
#define _RC_Y_HASHFUNC_H

namespace owca {
	namespace __owca__ {

		class hashfunc {
			unsigned int val;
			unsigned int count;
		public:
			hashfunc() : val(0), count(0) {}
			void process(const void *p, unsigned int size) {
				for(unsigned int i=0;i<size;++i) process(((const unsigned char*)p)[i]);
			}
			void process(unsigned char c) {
				val += (short)c + count + 1;
				val += val << 10;
				val ^= val >> 6;
				count = (count + 1) & 0xffff;
			}
			unsigned int value() {
				val += val << 3;
				val ^= val >> 11;
				return (val + (val << 15));
			}
		};

	}
}

#endif
