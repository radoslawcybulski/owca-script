#ifndef _RC_Y_EXEC_MAP_OBJECT_ITERATOR_H
#define _RC_Y_EXEC_MAP_OBJECT_ITERATOR_H

#include "hash_map.h"

namespace owca { namespace __owca__ {
	typedef hash_map_iterator exec_map_object_iterator;
} }

//namespace owca {
//	namespace __owca__ {
//		class exec_map_object;
//		class exec_map_object_iterator;
//	}
//}
//
//namespace owca { namespace __owca__ {
//	class exec_map_object_iterator {
//		unsigned int index;
//		friend class exec_map_object;
//	public:
//		exec_map_object_iterator() : index(-1) { }
//		explicit exec_map_object_iterator(unsigned int index_) : index(index_) { }
//
//		void clear() { index=-1; }
//	};
//} }

#endif
