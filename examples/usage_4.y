function f1():
	$print('in f1() before pause')
	$debugbreak()
	$print('in f1() after pause')

function f2():
	$print('in f2() before pause')
	$debugbreak()
	$print('in f2() after pause')

function f3():
	$print('in f3() before pause')
	$debugbreak()
	$print('in f3() after pause')
	
function run():
	$print('started')
	r = cpp_function_as_y_function(f1,f2,f3)
	$print('done')
	return r
