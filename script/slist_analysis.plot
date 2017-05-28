#ins_min(n) = n
ins_avg(n) = (n+2) * (n-1) / 4
#ins_max(n) = (n+1) * (n-1) / 2
#
merge_avg(n) = n + (n * log(n) / log(2))
#
#insmerge_max(n,k) = (n * ins_max(k) / k) + (n * log(n / k) / (k * log(2)))
insmerge_avg(n,k) = (n * ins_avg(k) / k) + (n * log(n / k) / (k * log(2)))
insmerge_min(n,k) = (n * ins_min(k) / k) + (n * log(n / k) / (k * log(2)))
#
#set term svg enhanced background rgb 'white' size 1920 1080
##set terminal pngcairo dashed enhanced
##set output 'test.png'
#set output 'test.svg'
#
#set autoscale 
#set key default
#set key box
#
#set multiplot layout 3, 1 title "Hybrid insertion / Merge sort average comparison complexity"
#set tmargin 3
#
#
#set samples 1024*16
#set logscale xy 2
#set grid mxtics ytics xtics
#show grid
#show mxtics
#
#set title "Best case complexity"
#set key center right
#set key outside
#set yrange[0.03125:0.5]
#plot [0.1:1024*1024] \
#	for [k in "2 4 6 8 10 12 14 16 32 64"] \
#		(insmerge_min(x, k) / merge_avg(x)) title(k)
#
#
#set title "Average case complexity"
#set key center right
#set key outside
#set yrange[0.125:4]
#plot [0.1:1024*1024] \
#	for [k in "2 4 6 8 10"] \
#		(insmerge_avg(x, k) / merge_avg(x)) title(k)
#
#set title "Worst case complexity"
#set key center right
#set key outside
#set yrange[0.25:4]
#plot [0.1:1024*1024] \
#	for [k in "2 4 8 16 32 64"] \
#		(insmerge_max(x, k) / merge_avg(x)) title(k)
#


#plot [0:4096] \
#	for [n in "4 16 64 256 1024 4096 32768 262144 1048576"] \
#		(insmerge_avg(n, x) / merge_avg(n)) title(n)
#

set logscale xy 2
#set grid mxtics ytics xtics
#show grid
#show mxtics
set xrange[0.125:1024*1024]
set yrange[1:256*1024]
set zrange[0:]
splot insmerge_avg(x,y)

pause -1



