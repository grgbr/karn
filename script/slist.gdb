define print_slist
	printf "[%p]", &(($arg0).slist_head)
	echo $arg0:
	printf " slist_head = {slist_next = %p}, ", ($arg0).slist_head.slist_next,
	printf "slist_tail = %p\n", ($arg0).slist_tail
	set $node = ($arg0).slist_head.slist_next
	set $n = 0
	while $node != 0
		printf "\t[%p]%u: slist_next = %p\n", $node, $n, $node->slist_next
		set $node = ($node)->slist_next
		set $n = $n + 1
	end
	echo \n
end

document print_slist
	print_slist SLIST

	Print slist passed in argument
end

define print_slist_containers
	printf "[%p]", &(($arg0).slist_head)
	echo $arg0:\n
	set $node = ($arg0).slist_head.slist_next
	set $n = 0
	while $node != 0
		set $itm = containerof($node, struct $arg1, $arg2)
		printf "\t[%p]%u:", $itm, $n
		output *($itm)
		echo \n
		set $node = ($node)->slist_next
		set $n = $n + 1
	end
	echo \n
end

document print_slist_containers
	print_slist_container SLIST TYPE MEMBER

	Print containers linked into slist passed in argument in a raw formatted
	manner

	SLIST refers to the struct slist reference to print
	TYPE refers to the type of container the slist_node is embedded in
	     (without the struct keyword)
	MEMBER refers to the slist_node field embedded into container of type
	       TYPE
end

define print_formatted_slist_containers
	printf "[%p]", &(($arg0).slist_head)
	echo $arg0:\n
	set $node = ($arg0).slist_head.slist_next
	set $n = 0
	while $node != 0
		set $itm = containerof($node, struct $arg1, $arg2)
		printf "\t[%p]%u:", $itm, $n
		$arg3 *($itm)
		set $node = ($node)->slist_next
		set $n = $n + 1
	end
	echo \n
end

document print_formatted_slist_containers
	print_formatted_slist_container SLIST TYPE MEMBER FORMATTER

	Print containers linked into slist passed in argument according to the
	formatter passed as last argument

	SLIST refers to the struct slist reference to print
	TYPE refers to the type of container the slist_node is embedded in
	     (without the struct keyword)
	MEMBER refers to the slist_node field embedded into container of type
	       TYPE
	FORMATTER refers to a user defined gdb function used to format output
	for each container linked into SLIST
end
