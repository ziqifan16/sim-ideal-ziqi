//
// C++ Interface: lru_ziqi
//
// Description:
//
//
// Author: ziqi fan, UMN
//
// Copyright: See COPYING file that comes with this distribution
//
///ziqi: dont forget to change this
#ifndef LRU_ZIQI_H
#define LRU_ZIQI_H

#include <map>
#include <list>
#include "cassert"
#include "iostream"
#include "global.h"
#include "baseCache.h"

using namespace std;

///ziqi: time gap between two sync in seconds
const int syncTime = 30;
// Class providing fixed-size (by number of records)
// LRU-replacement cache of a function with signature
// V f(K)
template <typename K, typename V> 
class ZiqiLRUCache : public TestCache<K,V>
{
public:
// Key access history, most recent at back
	typedef list<K> key_tracker_type;
// Key to value and key history iterator
	typedef map
	< K, pair<V, typename key_tracker_type::iterator> > 	key_to_value_type;
	
///ziqi: o_file for dirty pages
	std::ofstream o_file;
// Constuctor specifies the cached function and
// the maximum number of records to be stored.
	ZiqiLRUCache(
	        V(*f)(const K& , V),
	        size_t c,
	        unsigned levelMinus
	) : _fn(f) , _capacity(c),levelMinusMinus(levelMinus)  {
		///ARH: Commented for single level cache implementation
//         assert ( _capacity!=0 );
		o_file.open("dirtyPage.out");
	}
	// Obtain value of the cached function for k
	
	/*
	///ziqi
	void syncDirtyPage() {
		
		if(o_file.is_open()){
		      o_file<<"*****************30s********************"<<endl;
		      typename key_to_value_type::iterator it;
		      for(it = _dirty_page_map.begin(); it!=_dirty_page_map.end(); it++){
			    //std::cout<<"it->first"<<it->first<<endl;
			    o_file<<it->first<<"!@#$"<<it->second.first.getReq().issueTime<<endl;
		      }
		      for(it = _dirty_page_map.begin(); it!=_dirty_page_map.end(); it++){
			    _dirty_page_map.erase(it);
			    _dirty_page_tracker.pop_front();
		      }
		}
	}
	*/
	
	
	uint32_t access(const K& k  , V& value, uint32_t status) {
		assert(_capacity != 0);
		PRINTV(logfile << "Access key: " << k << endl;);
		

		///ziqi: if request is write, mark the page status as DIRTY
		if(status & WRITE) {
		      status|= DIRTY;
		      value.updateFlags(status);
		      //const V v1 = _fn(k, value);
		      //insertDirtyPage(k, v1);
		}
		/*
///FIXME if multiple pages' issueTime are the same, syncDirtyPage would run multiple times, but only one is needed
		if(0 == ((value.getReq().issueTime - 1172163600) % syncTime)) {
		      syncDirtyPage();
		}
		*/
		
// Attempt to find existing record
		const typename key_to_value_type::iterator it	= _key_to_value.find(k);
		//const typename key_to_value_type::iterator itNew	= _key_to_value.find(k);
		
		if(it == _key_to_value.end()) {
// We don’t have it:
			PRINTV(logfile << "Miss on key: " << k << endl;);
// Evaluate function and create new record
			const V v = _fn(k, value);

			///ziqi: inserts new elements on read and write miss
			status |=  insert(k, v);
			PRINTV(logfile << "Insert done on key: " << k << endl;);

			return (status | PAGEMISS);
		} else {
			PRINTV(logfile << "Hit on key: " << k << endl;);
// We do have it. Before returning value,
// update access record by moving accessed
// key to back of list.
			
			 _key_to_value.erase(it);
			 _key_tracker.remove(k);
			 
			 assert(_key_to_value.size() < _capacity);
			 
			 const V v = _fn(k, value);
			
			// Record k as most-recently-used key
			typename key_tracker_type::iterator itNew
			= _key_tracker.insert(_key_tracker.end(), k);
	// Create the key-value entry,
	// linked to the usage record.
			_key_to_value.insert(make_pair(k, make_pair(v, itNew))); 
			
			return (status | PAGEHIT | BLKHIT);
		}
		
	} //end operator access


	unsigned long long int get_min_key() {
		return (_key_to_value.begin())->first;
	}

	unsigned long long int get_max_key() {
// 			std::map< K, std::pair<V,typename key_tracker_type::iterator> >::iterator it;
		return (_key_to_value.rbegin())->first;
	}
	
	
	void remove(const K& k) {
		PRINTV(logfile << "Removing key " << k << endl;);
// Assert method is never called when cache is empty
		assert(!_key_tracker.empty());
// Identify  key
		const typename key_to_value_type::iterator it
		= _key_to_value.find(k);
		assert(it != _key_to_value.end());
		PRINTV(logfile << "Remove value " << endl;);
// Erase both elements to completely purge record
		_key_to_value.erase(it);
		_key_tracker.remove(k);
	}
private:

// Record a fresh key-value pair in the cache
	int insert(const K& k, const V& v) {
		PRINTV(logfile << "insert key " << k  << endl;);
		int localStatus = 0;
// Method is only called on cache misses
		assert(_key_to_value.find(k) == _key_to_value.end());

// Make space if necessary
		if(_key_to_value.size() == _capacity) {
			PRINTV(logfile << "Cache is Full " << _key_to_value.size() << " sectors" << endl;);
			evict();
			localStatus = EVICT;
		}

// Record k as most-recently-used key
		typename key_tracker_type::iterator it
		= _key_tracker.insert(_key_tracker.end(), k);
// Create the key-value entry,
// linked to the usage record.
		_key_to_value.insert(make_pair(k, make_pair(v, it)));
// No need to check return,
// given previous assert.
// 			add_sram_entry(k,false);
		return localStatus;
	}
	
	// Record a fresh key-value pair in the cache
	int insertDirtyPage(const K& k, const V& v) {
		PRINTV(logfile << "dirty page insert key " << k  << endl;);
		
// Record k as most-recently-used key
		typename key_tracker_type::iterator it
		= _dirty_page_tracker.insert(_dirty_page_tracker.end(), k);
// Create the key-value entry,
// linked to the usage record.
		_dirty_page_map.insert(make_pair(k, make_pair(v, it)));
// No need to check return,
// given previous assert.
// 			add_sram_entry(k,false);
		return 1;
	}
	
// Purge the least-recently-used element in the cache
	void evict() {
// Assert method is never called when cache is empty
		assert(!_key_tracker.empty());
// Identify least recently used key
		const typename key_to_value_type::iterator it
		= _key_to_value.find(_key_tracker.front());
		if(it == _key_to_value.end())
		  printf("debug");
		
		assert(it != _key_to_value.end());
		
///ziqi: denote sequential dirty page length
		int seqLength = 0;
///ziqi: above threshold count as one sequential write
		int threshold = 4;
		
///ziqi: itSeq is used to check whether their are other sequential dirty page in the map
		const typename key_to_value_type::iterator itSeq;
		
///ziqi: if the key is not dirty, evict it
		//if(!((it->second.first.getReq().flags) & DIRTY)) {
		  PRINTV(logfile << "evicting victim non-dirty key " << (*it).first <<  endl;);
		  //cout<<it->second.first.getReq().flags<<endl;
  // Erase both elements to completely purge record
		  _key_to_value.erase(it);
		  
		  _key_tracker.pop_front();
		
		/*
		}
///ziqi: if the key is dirty, check its sequential length
		else{
		    for(itSeq = _key_to_value.begin(); itSeq!=_key_to_value.end(); itSeq++){
			if((itSeq->second.first.getReq().flags) & DIRTY)
			{
			  if((it->second.first.getFsblkno()+seqLength) == itSeq->second.first.getFsblkno())
			  {
			    seqLength++;
			    itSeq = _key_to_value.begin();
			    continue;
			  }
			}
		    }
		    
		    if(seqLength > threshold)
		    {
		      
		    }
		}
		*/
		
		
	}
// The function to be cached
	V(*_fn)(const K& , V);
// Maximum number of key-value pairs to be retained
	const size_t _capacity;

// Key access history
	key_tracker_type _key_tracker;
// Key-to-value lookup
	key_to_value_type _key_to_value;
///ziqi: dirty page tracker
	key_tracker_type _dirty_page_tracker;
///ziqi: dirty page map
	key_to_value_type _dirty_page_map;
	unsigned levelMinusMinus;
};

#endif //end lru_stl
