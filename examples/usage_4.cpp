//#include "owca.h"
//
//using namespace owca;
//
//std::string load_file_as_string(const char *file_name);
//void printfunction(const std::string &txt);
//
//struct cpp_function_as_struct : public owca_user_function_base_object {
//	unsigned int state;
//	
//	owca_function_return_value initialize(owca_global &retval) {
//		// parameters object has some helper methods to ease checking for argument count and types
//		// they also raise compatible exception types with owca virtual machine
//		// here we check for one and only one argument
//		if (!parameters.check_parameter_count(retval, 3, 3)) return owca_function_return_value::EXCEPTION;
//		
//		// state is my variable keeping memory, of what happened before
//		state = 0;
//		return run(retval,false);
//	}
//	owca_function_return_value run(owca_global &retval, bool is_exception)
//	{
//		switch(state) {
//		case 0: {
//			// state 0 - first time
//			owca_local c = parameters.parameter(0);
//			
//			// set state to 1, so when the function is resumed, its 
//			// resumed in next state
//			state = 1;
//			return c.prepare_call(retval); }
//		case 1: {
//			// state 1
//			owca_local c = parameters.parameter(1);
//			
//			// set state to 2
//			state = 2;
//			return c.prepare_call(retval); }
//		case 2: {
//			// state 2
//			owca_local c = parameters.parameter(2);
//			
//			// set state to 2
//			state = 3;
//			return c.prepare_call(retval); }
//		case 3:
//			// we are done
//			break;
//		}
//		retval.null_set(true);
//		return owca_function_return_value::RETURN_VALUE;
//	}
//	
//};
//
//bool run_file_4(const char *file_name)
//{
//	std::string source_code = load_file_as_string(file_name);
//	if (source_code.empty()) return false;
//	
//	try {
//		owca_vm vm;            // virtual machine object itself
//		owca_message_list ml;   // list of compilation errors and warnings
//		owca_global gnspace;   // this will hold owca namespace - result of compilation
//		owca_namespace nspace; // C++ helper when accessing owca namespace
//
//		// construct new namespace object
//		// the same virtual machine can have many namespaces
//		// but a namespace can be used only with the vm
//		// that created it
//		gnspace = vm.create_namespace("test");
//		
//		// initialize C++ helper object
//		nspace = gnspace.namespace_get();
//
//		// slightly different syntax to use		
//		nspace["cpp_function_as_y_function"].set<cpp_function_as_struct>();
//
//		// set function handling owca' $print function
//		vm.set_print_function(printfunction);
//		
//		// compile source
//		// this will run all statements and expressions in global scope!
//		// thus this can result in owca exception
//		// all created names (functions, classes, variables and so on)
//		// are put into namespace nspace
//		vm.compile(nspace, ml, owca_source_file_Text(source_code.c_str()));
//		
//		// owca doesnt run the garbage collector itself for now, so run it once a while
//		vm.run_gc();
//
//		if (ml.has_errors() || ml.has_warnings()) {
//			for(owca_message_list::T it = ml.begin();it != ml.end();++it) {
//				printf("%5d:      %s\n",it->line(),it->text().c_str());
//			}
//			return false;
//		}
//
//		auto function = nspace["run"];
//		auto function_return_value = function.call(function_return_value);
//		
//		// owca doesnt run the garbage collector itself for now, so run it once a while
//		vm.run_gc();
//
//		printf("function returned value %s\n",function_return_value.str().c_str());
//		return function_return_value.int_is() && function_return_value.int_get() == 0;
//	}
//	catch (const std::exception &e) {
//		printf("exception %s\n",e.what());
//		return false;
//	}
//}
