source slist.gdb
source array.gdb

define pent
	printf "%u\n", ($arg0).value
end

document pent
	Print slist unit testing entry
end

define pentlist
	print_formatted_slist_containers $arg0 ut_slist_entry node pent
end

document pentlist
	Print slist unit testing entry list passed in argument
end

set follow-fork-mode child
set detach-on-fork off
