# header guard
if !; MYFUNC_SL
	out ~ "myfunc.sl included 1st time\n"
	print = MYFUNC_print
	MYFUNC_SL=1
else
	out ~ "myfunc.sl already included\n"
end
return;

MYFUNC_print ~
	n =;
	out ~ n
	out ~ "\n"
	return

