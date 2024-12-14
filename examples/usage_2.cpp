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
	
	try {
		// get a reference to virtual machine from namespace object
		owca_vm &vm = nspace.vm();
		owca_message_list ml;   // list of compilation errors and warnings
		owca_global res;       // placeholder for Y values

		// compile source
		// this will run all statements and expressions in global scope!
		// thus this can result in Y exception
		// all created names (functions, classes, variables and so on)
		// are put into namespace nspace
		owca_function_return_value r = vm.compile(res, nspace, ml, owca_source_file_Text(source_code.c_str()));
		
		// Y doesnt run the garbage collector itself for now, so run it once a while
		vm.run_gc();
		
		// vm.compile function returns object, which tells You how the code compilation
		// (or execution) went

		// only those values can be returned from vm.compile function
		switch(r.type()) {
		case owca_function_return_value::RETURN_VALUE:
			// vm.compile will never returns this
			// as global scope cant return value
			break;
		case owca_function_return_value::NO_RETURN_VALUE:
			// vm.compile finished without an exception and everything went well
			// nothing to do
			break;
		case owca_function_return_value::EXCEPTION:
			// an exception was raised, when executing some code in global scope
			// res object will bear an actual exception object, which 
			// You can inspect for stack trace, exception code and message and so on
			break;
		}

		if (ml.has_errors() || ml.has_warnings()) {
			for (owca_message_list::T it = ml.begin(); it != ml.end(); ++it) {
				printf("%5d:      %s\n",it->line(),it->text().c_str());
			}
			printf("\n");
		}
		return !ml.has_errors() && r.type() != owca_function_return_value::EXCEPTION;
	}
	catch (owca_exception e) {
		printf("%s exception\n",e.message().c_str());
		return false;
	}
}

bool run_files(std::list<std::string> files, std::string function_name)
{
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
	
	while(!files.empty()) {
		if (!run_file_2(nspace,files.front().c_str())) {
			// compilation failed so we exit
			return false;
		}
		files.pop_front();
	}
	
	// get a function with function_name name
	// y_namespace object has operator [] overloaded, but it will throw an C++ exception
	// if member is not found
	owca_global function;
	try {
		function = nspace[function_name];
	}
	catch (owca_exception e) {
		// member is missing, so exiting
		printf("%s exception\n",e.message().c_str());		
		return false;
	}
	
	// function C++ object contains an reference to Y object named function_name
	// so lets call it.
	// it might not be a function (and not a callable object), in which case 
	// Y will throw Y exception
	// or the function might throw its own exception, in all cases
	// those will be Y objects
	
	owca_global function_return_value;
	owca_function_return_value r = function.call(function_return_value);
	
	// Y doesnt run the garbage collector itself for now, so run it once a while
	vm.run_gc();

	// only those values can be returned from call function
	switch(r.type()) {
	case owca_function_return_value::RETURN_VALUE:
		// function returned with a value, lets print it
		printf("function returned value %s\n",function_return_value.str().c_str());
		break;
	case owca_function_return_value::NO_RETURN_VALUE:
		// function returned without value
		// nothing to do
		break;
	case owca_function_return_value::EXCEPTION:
		// an exception was raised
		break;
	}
	
	printf("%d %d %d\n", __LINE__, r.type(), owca_function_return_value::EXCEPTION);
	return r.type() != owca_function_return_value::EXCEPTION;
}
