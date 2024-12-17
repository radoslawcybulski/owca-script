#include "owca.h"

using namespace owca;

std::string load_file_as_string(const char *file_name);
void printfunction(const std::string &txt);

owca_global cpp_function(owca_namespace& ns, owca_int  v1, owca_int v2)
{
	return v1+v2;
}

bool run_file_3(const char *file_name)
{
	std::string source_code = load_file_as_string(file_name);
	if (source_code.empty()) return false;
	
	try {
		owca_vm vm;            // virtual machine object itself
		owca_message_list ml;   // list of compilation errors and warnings
		owca_global gnspace;   // this will hold Y namespace - result of compilation
		owca_namespace nspace; // C++ helper when accessing Y namespace

		// construct new namespace object
		// the same virtual machine can have many namespaces
		// but a namespace can be used only with the vm
		// that created it
		gnspace = vm.create_namespace("test");
		
		// initialize C++ helper object
		nspace = gnspace.namespace_get();

		nspace["cpp_function_as_y_function"].set(cpp_function,"param_1","param_2");

		// set function handling Y' $print function
		vm.set_print_function(printfunction);
		
		// compile source
		// this will run all statements and expressions in global scope!
		// thus this can result in Y exception
		// all created names (functions, classes, variables and so on)
		// are put into namespace nspace
		 vm.compile(nspace, ml, owca_source_file_Text(source_code.c_str()));
		
		// Y doesnt run the garbage collector itself for now, so run it once a while
		vm.run_gc();

		if (ml.has_errors() || ml.has_warnings()) {
			for(owca_message_list::T it = ml.begin();it != ml.end();++it) {
				printf("%5d:      %s\n",it->line(),it->text().c_str());
			}
			return false;
		}
		
		
		auto function = nspace["run"];
		auto function_return_value = function.call(function_return_value);
		
		// Y doesnt run the garbage collector itself for now, so run it once a while
		vm.run_gc();

		printf("function returned value %s\n",function_return_value.str().c_str());
		return function_return_value.int_is() && function_return_value.int_get() == 0;
	}
	catch (const std::exception &e) {
		printf("exception %s\n",e.what());
		return false;
	}
}
