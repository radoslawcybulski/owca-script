#ifndef _RC_Y_EXEC_CLASS_OBJECT_H
#define _RC_Y_EXEC_CLASS_OBJECT_H

#include "exec_base.h"
#include "operatorcodes.h"
#include "structinfo.h"

namespace owca {
	class gc_iteration;
	namespace __owca__ {
		template <class key, class value> class lineartree;
		class exec_class_object;
		class exec_object;
		class exec_variable;
		class internal_class;
		class virtual_machine;
		class owca_internal_string;
	}
}

namespace owca {
	namespace __owca__ {
		class exec_class_object : public object_base {
			friend class virtual_machine;
			void _fill_names(virtual_machine &vm, exec_object *master, std::list<exec_object *> &lkp, std::map<exec_object *,unsigned int> &classes, unsigned int &total_size);
		public:
			exec_class_object(virtual_machine &vm, unsigned int oversize) : storage_size(0),name(NULL),constructor(NULL),destructor(NULL),marker(NULL),vm_class_object_data_lookup_type(NULL) { }
			~exec_class_object() { }

			std::string _create(internal_class &, exec_object *master);
			std::string _create(virtual_machine &vm, const exec_variable *inheritance, unsigned int inheritancecount, exec_object *master);
			std::vector<exec_object*> inherited;
			std::vector<exec_object*> lookup_order;
			lineartree<exec_object*,unsigned int> offsetmap;
			unsigned int storage_size,total_storage_size;
			bool inheritable,constructable;
			void (*constructor)(void *, unsigned int oversize, virtual_machine &vm);
			void (*destructor)(void *, virtual_machine &vm);
			void (*marker)(const void *, const gc_iteration &gc);
			owca_internal_string *name;
			exec_variable *operators[E_COUNT];
			exec_variable *lookup(owca_internal_string *ident);
			structid_type vm_class_object_data_lookup_type;

			void _marker(const gc_iteration &gc) const;
			void _destroy(virtual_machine &vm);
		};
	}
}
#endif
