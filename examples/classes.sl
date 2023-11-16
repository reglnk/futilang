

Foo =; {}
Foo ] "__init__"; Foo__init
Foo ] "__destroy__"; Foo__destroy
Foo ] "smth"; Foo__smth
template ~ Foo

return; call main

print ~
	n =;
	out ~ n
	out ~ "\n"
	return

Foo__init ~
	this =;
	call print; "Foo instance is created"
	return; this

Foo__destroy ~
	this =;
	call print; "Foo instance is destroyed"
	return

Foo__smth ~
	this =;
	call print; "smth is called";
	t =; this.v

	# check if v is existing
	if; t != "number"; t=; type~t;
		this ] "v"; 1
	else
		this ] "v"; 1 +; this.v
	end
	call print; this . v
	return

main ~
	obj =; new Foo
	obj:smth;
	obj:smth;
	obj:smth;
	return
