#include "owca.h"

using namespace owca;

// from Tutorial 1
std::string load_file_as_string(const char *file_name);
void printfunction(const std::string &txt);

// file_name is a name of source file, for diagnosting purpouses only
// returns true if successful
// this time we compile all files into the same namespace
// namespace object has a VM object inside, so i only need
// to pass a reference to namespace
bool run_file_2(owca_namespace &nspace, const char *file_name)
{
	std::string source_code = load_file_as_string(file_name);
	if (source_code.empty()) return false;
	
	// get a reference to virtual machine from namespace object
	owca_vm &vm = nspace.vm();
	owca_message_list ml;   // list of compilation errors and warnings

	// compile source
	// this will run all statements and expressions in global scope!
	// thus this can result in Y exception
	// all created names (functions, classes, variables and so on)
	// are put into namespace nspace
	vm.compile(nspace, ml, owca_source_file_Text(source_code.c_str()));
		
	// Y doesnt run the garbage collector itself for now, so run it once a while
	vm.run_gc();
		
	if (ml.has_errors() || ml.has_warnings()) {
		for (owca_message_list::T it = ml.begin(); it != ml.end(); ++it) {
			printf("%5d:      %s\n",it->line(),it->text().c_str());
		}
		printf("\n");
	}
	return !ml.has_errors();
}

bool run_files(std::list<std::string> files, std::string function_name)
{
	try {
		// virtual machine object itself
		owca_vm vm;

		// construct new namespace object
		// the same virtual machine can have many namespaces
		// but a namespace can be used only with the vm
		// that created it
		owca_global gnspace = vm.create_namespace("test");

		// initialize C++ helper object
		owca_namespace nspace = gnspace.namespace_get();

		// set function handling Y' $print function
		vm.set_print_function(printfunction);

		while (!files.empty()) {
			if (!run_file_2(nspace, files.front().c_str())) {
				// compilation failed so we exit
				return false;
			}
			files.pop_front();
		}

		// get a function with function_name name
		// y_namespace object has operator [] overloaded, but it will throw an C++ exception
		// if member is not found
		auto function = nspace[function_name];

		// function C++ object contains an reference to Y object named function_name
		// so lets call it.
		// it might not be a function (and not a callable object), in which case 
		// Y will throw Y exception
		// or the function might throw its own exception, in all cases
		// those will be Y objects

		auto function_return_value = function.call();

		// Y doesnt run the garbage collector itself for now, so run it once a while
		vm.run_gc();

		printf("function returned value %s\n", function_return_value.str().c_str());
		return true;
	}
	catch (const std::exception &e) {
		printf("exception %s\n",e.what());
		return false;
	}
}
