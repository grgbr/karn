source slist.gdb

define ptok
	set $c = 0
	while $c < ($arg0).tok_length
		printf "%c", ($arg0).tok_data[$c]
		set $c = $c + 1
	end
	printf "(%u)\n", ($arg0).tok_rate
end

document ptok
	Print token passed in argument
end

define ptl
	print_formatted_slist_containers $arg0 mapred_token tok_node ptok
end

document ptl
	Print token slist passed in argument
end

define ptst
	printf "[%p]", $arg0
	echo $arg0:
	printf " count = %u\n", ($arg0)->tok_count
	ptl &($arg0)->tok_list
end

document ptst
	Print token store passed in argument
end

define plt
	set $tok = containerof($arg0, struct mapred_token, tok_node)
	printf "[%p]", $tok
	echo $arg0:
	ptok $tok
end

document plt
	Print token from slist node passed in argument
end

define nptst
	next
	echo \n
	ptst result
	ptst source
	printf "cmp: %d\n", cmp
	plt merge
	plt ref_tok
	plt res_tok
	plt src_tok
	echo \n
end

document nptst
	Exec instruction then print token store passed in argument
end

set follow-fork-mode child
set detach-on-fork off
b mapred_merge_token_store
b mapred_merge_token_list
#b mapred_merge_token_list
#b mapred_merge_token_list_segment
#b mapred_merge_token_store
#run
#pst result
#pst store
