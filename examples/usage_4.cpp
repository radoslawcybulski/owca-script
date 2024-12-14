#include "owca.h"

using namespace owca;

std::string load_file_as_string(const char *file_name);
void printfunction(const std::string &txt);

struct cpp_function_as_struct : public owca_user_function_base_object {
	unsigned int state;
	
	owca_function_return_value initialize(owca_global &retval) {
		// parameters object has some helper methods to ease checking for argument count and types
		// they also raise compatible exception types with owca virtual machine
		// here we check for one and only one argument
		if (!parameters.check_parameter_count(retval, 3, 3)) return owca_function_return_value::EXCEPTION;
		
		// state is my variable keeping memory, of what happened before
		state = 0;
		return run(retval,false);
	}
	owca_function_return_value run(owca_global &retval, bool is_exception)
	{
		printf("state %d\n",state);
		switch(state) {
		case 0: {
			// state 0 - first time
			owca_local c = parameters.parameter(0);
			
			// set state to 1, so when the function is resumed, its 
			// resumed in next state
			state = 1;
			return c.prepare_call(retval); }
		case 1: {
			// state 1
			owca_local c = parameters.parameter(1);
			
			// set state to 2
			state = 2;
			return c.prepare_call(retval); }
		case 2: {
			// state 2
			owca_local c = parameters.parameter(2);
			
			// set state to 2
			state = 3;
			return c.prepare_call(retval); }
		case 3:
			// we are done
			break;
		}
		return owca_function_return_value::NO_RETURN_VALUE;
	}
	
};

bool run_file_4(const char *file_name)
{
	std::string source_code = load_file_as_string(file_name);
	if (source_code.empty()) return false;
	
	try {
		owca_vm vm;            // virtual machine object itself
		owca_message_list ml;   // list of compilation errors and warnings
		owca_global res;       // placeholder for owca values
		owca_global gnspace;   // this will hold owca namespace - result of compilation
		owca_namespace nspace; // C++ helper when accessing owca namespace

		// construct new namespace object
		// the same virtual machine can have many namespaces
		// but a namespace can be used only with the vm
		// that created it
		gnspace = vm.create_namespace("test");
		
		// initialize C++ helper object
		nspace = gnspace.namespace_get();

		// slightly different syntax to use		
		nspace["cpp_function_as_y_function"].set<cpp_function_as_struct>();

		// set function handling owca' $print function
		vm.set_print_function(printfunction);
		
		// compile source
		// this will run all statements and expressions in global scope!
		// thus this can result in owca exception
		// all created names (functions, classes, variables and so on)
		// are put into namespace nspace
		// vm.compile function returns object, which tells You how the code compilation
		// (or execution) went
		owca_function_return_value r = vm.compile(res, nspace, ml, owca_source_file_Text(source_code.c_str()));
		
		// owca doesnt run the garbage collector itself for now, so run it once a while
		vm.run_gc();

		// only those values can be returned from vm.compile function
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
		
		// owca doesnt run the garbage collector itself for now, so run it once a while
		vm.run_gc();

		// only those values can be returned from call function
		switch(r.type()) {
		case owca_function_return_value::RETURN_VALUE:
			// function returned with a value, lets print it
			printf("function returned value %s\n",function_return_value.str().c_str());
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
		return false;
	}
}
