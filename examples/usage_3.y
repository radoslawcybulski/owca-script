function run():
	a = cpp_function_as_y_function(10,4)
	if a != 14:
		return 1
	# a is an integer 14

	b = cpp_function_as_y_function(10,1.0)
	# b is an integer 11, since Y is allowed to convert yReal into yInt
	# when no value is lost
	if b != 11:
		return 2
		
	try:
		c = cpp_function_as_y_function(10,1.5)
		return 3
	except:
		pass
	# this will raise an exception (YEXCEPTION_INVALIDPARAMTYPE), as 1.5 is
	# not convertible to an integer without loosing part of the information

	try:
		d = cpp_function_as_y_function(10,'123')
		return 4
	except:
		pass
	# this will also raise an exception (YEXCEPTION_INVALIDPARAMTYPE)
	# Y doesnt try to convert strings to integers / reals
	return 0