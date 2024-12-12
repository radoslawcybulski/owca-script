#include "owca.h"

using namespace owca;

std::string load_file_as_string(const char *file_name);
void printfunction(const std::string &txt);

owca_function_return_value cpp_function(owca_global &retval, owca_namespace &ns, owca_int  v1, owca_int v2)
{
	retval.int_set(v1+v2);
	return owca_function_return_value::RETURN_VALUE;
}

bool run_file_3(const char *file_name)
{
	std::string source_code = load_file_as_string(file_name);
	if (source_code.empty()) return false;
	
	try {
		owca_vm vm;            // virtual machine object itself
		owca_message_list ml;   // list of compilation errors and warnings
		owca_global res;       // placeholder for Y values
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
		// vm.compile function returns object, which tells You how the code compilation
		// (or execution) went
		owca_function_return_value r = vm.compile(res, nspace, ml, owca_source_file_Text(source_code.c_str()));
		
		while (r.type() == owca_function_return_value::DEBUG_BREAK) {
			// $debugbreak function was called. it pauses the execution
			// and allows to inspect VM, check values and so on
			// ignore it for now
			r = vm.resume_execution();
		}
		
		// Y doesnt run the garbage collector itself for now, so run it once a while
		vm.run_gc();

		// only those values can be returned from vm.compile function
		// (and DEBUG_BREAK, which we just ignored)
		switch(r.type()) {
		case owca_function_return_value::RETURN_VALUE:
			// vm.compile will never returns this
			// as global scope cant return value
			break;
		case owca_function_return_value::NO_RETURN_VALUE:
			// vm.compile finished without an exception and everything went well
			// or compilation failed (in which case ml.has_errors() will be true)
			// nothing to do
			break;
		case owca_function_return_value::EXCEPTION:
			// an exception was raised, when executing some code in global scope
			// res object will bear an actual exception object, which 
			// You can inspect for stack trace, exception code and message and so on
			break;
		}

		if (ml.has_errors() || ml.has_warnings()) {
			for(owca_message_list::T it = ml.begin();it != ml.end();++it) {
				printf("%5d:      %s\n",it->line(),it->text().c_str());
			}
			return false;
		}
		
		
		owca_global function;
		function = nspace["run"];
	
		owca_global function_return_value;
		r = function.call(function_return_value);
		
		while (r.type() == owca_function_return_value::DEBUG_BREAK) {
			// $debugbreak function was called. it pauses the execution
			// and allows to inspect VM, check values and so on
			// ignore it for now
			r = vm.resume_execution();
		}
			
		// Y doesnt run the garbage collector itself for now, so run it once a while
		vm.run_gc();

		// only those values can be returned from call function
		// (and DEBUG_BREAK, which we just ignored)
		switch(r.type()) {
		case owca_function_return_value::RETURN_VALUE:
			// function returned with a value, lets print it
			return function_return_value.int_is() && function_return_value.int_get() == 0;
		case owca_function_return_value::NO_RETURN_VALUE:
			// function returned without value
			// nothing to do
			break;
		case owca_function_return_value::EXCEPTION:
			// an exception was raised
			break;
		}
		
		return false;
	}
	catch (owca_exception e) {
		printf("%s exception\n",e.message().c_str());
		// Y throws C++ exceptions only for C++ side mistakes
		// invalid parameters for example
		return false;
	}
}
