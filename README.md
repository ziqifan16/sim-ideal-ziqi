Copyright: Alireza Haghdoost
University of Minnesota

sim-ideal
=========
Ideal multi-level cache simulator. This simulator has been implemented for the performance mesurement and design study of cache replacement policies.

Implemented Replacement policies
=========
LRU
Page-MIN
Block-MIN
OWBP


Input Trace Format
=========
Microsoft Research trace format, published in SNIA IOTTA trace repository 



Compilation Macros
=========
DEBUG : enable debug mode
VERB : enable verbous output in log.txt
REQSIZE : expand Trace request based on the request size (memTrace is bigger in this case)
HITS : collect histogram for PIRD (page level stack distance) and BIRD (block level stack distance)


Development Branches
=========
master: active development branch
rmBoost: remoove boost library and bimap
futureDistinctSet: uses distinct items in the future for future distance metadata value in OWBP algorithm


Limimtations: 
=========
- only 4 caching layers hardcoded 
- all cache layer in the hierachy need to maitain the same page size

TODO:
=========
- bound block min to future sliding window
- block metadata update rate and update time ?
- a request with reqSize > 1 will always have future distance block 1 , Do we need to fix it ? 

Assumptions:
- a request in the trace access only one block. For example if the request want to access 8 pages, all pages blong to the same block

========
Block i Coldness value at time t = number of valid pages in the cache associated with block i 
	that will not receive any hit in future window. 
block i coldness initial value: At the time of block i insertion, the coldness value could either 0 or 1. 
	if page p accessed in the future window, the coldness value is 0, otherwise it would be 1

there might be a chance to see two block with the same coldness value and distance, for this reason I used multiset for maxHeap. 
However, maxHeapAtoms should maitain a range not only a operator. 
		
if( no future page hit)
		
	block miss , page miss

		coldness = 1;

	block hit, page hit
		hit on previously cold page 
			coldness (nochange)
			no update on coldness (My hypothesis is that this case will not happen any more). this case means that a page is initially cold and after some times
				convert to hot page. However my argument is that if the page is cold, it will not translate to hot page because the containing block will 
				kicked off the write buffer in advance. Therefore, there would be a block miss for the second reference that we assume it can translate cold 
				page to hot page. 
				This condition is very critical if my hyphotesis turns out to be true, It means that the future window size is accurate. 
				if a page does not show up in the future window, it determined as cold and the supporting block will kicked out. 
				any way, I need to validate this hypothesis. 
				The ultimate goal is to be able to detect if current cold page was originaly cold
		
		hit on previously Hot page
			-- coldness
		
	block hit, page miss
		++ codlness
		
example:
	valid_page	cold_page	hot_page	distance
	20		8		12		100
	25		6		19		210
	15		5		10		107