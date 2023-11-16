# header guard
if !; WRAP_SL
	out ~ "wrap.sl included 1st time\n"
	wrap = WRAP_wrap
	WRAP_SL=1
else
	out ~ "wrap.sl already included\n"
end
return;

include "myfunc.sl"
return;

WRAP_wrap ~
	return; call print

