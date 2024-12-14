#include "owca.h"

using namespace owca;

void printfunction(const std::string &txt)
{
	// just print it to stdout
	printf("%s\n",txt.c_str());
}

// loads file from file, returns std::string containing it
std::string load_file_as_string(const char *file_name)
{	
	FILE *fl = fopen(file_name,"rb");
	std::string source_code;	
	if (fl) {
		std::list<std::string> text;
		unsigned int totalsize = 0;
		
		// read whole file into list of string blocks
		// keep total length as we go
		for(;;) {
			char buf[2048];
			buf[0] = 0;
			if (!fgets(buf,2048,fl)) break;
			text.push_back(buf);
			totalsize += text.back().size();
		}
		fclose(fl);
		
		// allocate string of correct size
		source_code.reserve(totalsize+1);
		
		// copy loaded string blocks into allocated string
		while(!text.empty()) {
			source_code += text.front();
			text.pop_front();
		}
	}
	
	return source_code;
}

// file_name is a name of source file, for diagnosting purpouses only
// returns true if successful
bool run_file_1(const char *file_name)
{
	std::string source_code = load_file_as_string(file_name);
	if (source_code.empty()) return false;
	
	try {
		owca::owca_vm vm;            // virtual machine object itself
		owca::owca_message_list ml;   // list of compilation errors and warnings
		owca::owca_global res;       // placeholder for owca values
		owca::owca_global gnspace;   // this will hold owca namespace - result of compilation
		owca::owca_namespace nspace; // C++ helper when accessing owca namespace

		// construct new namespace object
		// the same virtual machine can have many namespaces
		// but a namespace can be used only with the vm
		// that created it
		gnspace = vm.create_namespace("test"); 
		
		// initialize C++ helper object
		nspace = gnspace.namespace_get();
		
		// set function handling owca' $print function
		vm.set_print_function(printfunction);
		
		// compile source
		// this will run all statements and expressions in global scope!
		// thus this can result in owca exception
		// all created names (functions, classes, variables and so on)
		// are put into namespace nspace
		// vm.compile function returns object, which tells You how the code compilation
		// (or execution) went
		owca_function_return_value r = vm.compile(res, nspace, ml, owca::owca_source_file_Text(source_code.c_str()));
		
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
			for (owca::owca_message_list::T it = ml.begin(); it != ml.end(); ++it) {
				printf("%5d:      %s\n",it->line(),it->text().c_str());
			}
		}
		return !ml.has_errors() && r.type() != owca_function_return_value::EXCEPTION;
	}
	catch (owca_exception e) {
		printf("%s exception\n",e.message().c_str());
		return false;
	}
}

