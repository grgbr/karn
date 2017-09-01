source common.gdb
source slist.gdb
source dlist.gdb
source array.gdb

define pslistutent
	printf "%u\n", ($arg0).value
end

document pslistutent
	Print slist unit testing entry
end

define lslistutent
	print_formatted_slist_containers $arg0 ut_slist_entry node pslistutent
end

document lslistutent
	Print slist unit testing entry list passed in argument
end

set follow-fork-mode child
set detach-on-fork off
