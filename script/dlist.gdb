define print_dlist
	printf "[%p]", ($arg0)
	echo $arg0:
	printf " dlist_prev = %p, dlist_next = %p\n", \
	       ($arg0).dlist_prev, ($arg0).dlist_next
	set $node = ($arg0).dlist_next
	set $n = 0
	while $node != ($arg0)
		printf "\t[%p]%u: dlist_prev = %p, dlist_next = %p\n", \
		       $node, $n, $node->dlist_prev, $node->dlist_next
		set $node = ($node)->dlist_next
		set $n = $n + 1
	end
	echo \n
end

document print_dlist
	print_dlist dlist

	Print dlist passed in argument
end

define print_dlist_containers
	printf "[%p]", ($arg0)
	echo $arg0:\n
	set $node = ($arg0).dlist_next
	set $n = 0
	while $node != ($arg0)
		set $itm = containerof($node, struct $arg1, $arg2)
		printf "\t[%p]%u:", $itm, $n
		output *($itm)
		echo \n
		set $node = ($node)->dlist_next
		set $n = $n + 1
	end
	echo \n
end

document print_dlist_containers
	print_dlist_container dlist TYPE MEMBER

	Print containers linked into dlist passed in argument in a raw formatted
	manner

	dlist refers to the struct dlist reference to print
	TYPE refers to the type of container the dlist_node is embedded in
	     (without the struct keyword)
	MEMBER refers to the dlist_node field embedded into container of type
	       TYPE
end

define print_formatted_dlist_containers
	printf "[%p]", ($arg0)
	echo $arg0:\n
	set $node = ($arg0).dlist_next
	set $n = 0
	while $node != ($arg0)
		set $itm = containerof($node, struct $arg1, $arg2)
		printf "\t[%p]%u:", $itm, $n
		$arg3 *($itm)
		set $node = ($node)->dlist_next
		set $n = $n + 1
	end
	echo \n
end

document print_formatted_dlist_containers
	print_formatted_dlist_container dlist TYPE MEMBER FORMATTER

	Print containers linked into dlist passed in argument according to the
	formatter passed as last argument

	dlist refers to the struct dlist reference to print
	TYPE refers to the type of container the dlist_node is embedded in
	     (without the struct keyword)
	MEMBER refers to the dlist_node field embedded into container of type
	       TYPE
	FORMATTER refers to a user defined gdb function used to format output
	for each container linked into dlist
end
