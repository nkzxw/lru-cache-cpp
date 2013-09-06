//-------------------------------------------------------------
// Compares our implementation to a Map + List implementation:
// 1. CPU time
// 2. Memory usage
// 3. Correctness: are the two caches equal?
//    Use this in conjunction to the unit tests (lru_tests.cpp)
//-------------------------------------------------------------

#include <cstdlib>
#include <iostream>
#include <boost/timer.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>
#include "../lru.hpp"
#include "lru_cache.h"
#include "smaps.hpp"

using namespace std;


// show the amount of heap size used
void show_memory_usage()
{
	std::auto_ptr<plb::smaps> smaps_ptr = plb::read_smaps();
	if (smaps_ptr.get()) {
		/*
		plb::smaps::const_iterator heap_it = smaps_ptr->find(plb::smaps::HEAP);
		if (heap_it != smaps_ptr->end()) {
			cerr << "heap usage: " << heap_it->second.size << "/" << heap_it->second.rss << " kb" << endl; 
		}
		*/
		plb::smaps_entry total = smaps_ptr->total();
		//cerr << "total usage: " << total << endl;
		cerr << "total/rss usage: " << total.size << "/" << total.rss << " kb" << endl; 
	}
}


enum TestCase {
	TEST_CASE_INSERT = 0,
	TEST_CASE_INSERT_READ
};


ostream & operator<<(ostream & os, const TestCase & tc)
{
	return os << (tc == TEST_CASE_INSERT ? "TEST_CASE_INSERT" :
		          tc == TEST_CASE_INSERT_READ ? "TEST_CASE_INSERT_READ" :
		          "TEST_CASE_UNKNOWN");
}


struct TestParams
{
	TestParams(int cache_size,
			   int num_keys,
			   int insertions,
			   bool report_memory = true,
			   bool report_cpu = true,
			   bool show_header = true) :
		cache_size(cache_size),
		num_keys(num_keys),
		insertions(insertions),
		report_memory(report_memory),
		report_cpu(report_cpu),
		show_header(show_header)
	{
	}
	
	string name() const
	{
		return "c" + boost::lexical_cast<string>(cache_size) + "_" +
		       "k" + boost::lexical_cast<string>(num_keys) + "_" +
			   "i" + boost::lexical_cast<string>(insertions);
	}
	
	int cache_size;
    int num_keys;
	int insertions;
	bool report_memory;
	bool report_cpu;
	bool show_header;
};


template<class K, class V>
struct TestDriver
{
	TestDriver(const TestParams & params) : params(params)
	{
	}
	
	virtual ~TestDriver()
	{
	}
	
	void do_test(TestCase tc)
	{
		if (params.show_header) {
			cerr << "-------------------------------------" << endl;
			cerr << params.name() << endl;
		}
		
		std::auto_ptr<boost::timer> t;
		if (params.report_cpu) {
			t.reset(new boost::timer());
		}
		
		create_cache();
		init_rand();
		
		switch (tc) {
			case TEST_CASE_INSERT:
				test_insert();
				break;
			case TEST_CASE_INSERT_READ:
				test_insert_read();
				break;
			default:
				break;
		}
		
		if (params.report_cpu) {
			double elapsed = t->elapsed();
			cerr << "elapsed: " << elapsed << endl;
			cerr << "rate: " << rate(elapsed) << endl;
		}
		
		if (params.report_memory) {
			show_memory_usage();
		}
	}
	
	void test_insert()
	{
		for (int i = 0;  i < params.insertions;  ++i)
			do_insert(get_key(), get_value());
	}
	
	int test_insert_read()
	{
		int ret = 0;
	
		for (int i = 0;  i < params.insertions;  ++i)
			ret += do_fetch_or_insert(get_key());
	
		return ret;
	}
	
	void init_rand()
	{
		srand(171);
	}
	
	K get_key()
	{
		return rand() % params.num_keys;
	}
	
	V get_value()
	{
		return rand();
	}
	
	double rate(double elapsed) const
	{
		return (elapsed > 0.0 ? params.insertions / elapsed : 0.0);
	}

	virtual void create_cache() = 0;
	
	virtual void do_insert(const K & key, const V & value) = 0;
	
	virtual V do_fetch_or_insert(const K & key) = 0;
	
	TestParams params;
};


template<class K, class V>
struct TestDriverPLB : public TestDriver<K, V>
{
	TestDriverPLB(const TestParams & params) : TestDriver<K, V>(params)
	{
	}
	
	virtual void create_cache()
	{
		cache.reset(new plb::LRUCacheH4<K, V>(TestDriver<K, V>::params.cache_size));
	}
	
	virtual void do_insert(const K & key, const V & value)
	{
		(*cache)[key] = value;
	}
	
	virtual V do_fetch_or_insert(const K & key)
	{
		typename plb::LRUCacheH4<K, V>::const_iterator it = cache->find(key);
		if (it != cache->end()) {
			return it.value();
		}
		else {
			V value = TestDriver<K, V>::get_value();
			(*cache)[key] = value;
			return value;
		}
	}

	std::auto_ptr<plb::LRUCacheH4<K, V> > cache;
};


template<class K, class V>
struct TestDriverPA : public TestDriver<K, V>
{
	TestDriverPA(const TestParams & params) : TestDriver<K, V>(params)
	{
	}
	
	virtual void create_cache()
	{
		cache.reset(new LRUCache<K, V>(TestDriver<K, V>::params.cache_size));
	}
	
	virtual void do_insert(const K & key, const V & value)
	{
		cache->insert(key, value);
	}
	
	virtual V do_fetch_or_insert(const K & key)
	{
		if (cache->exists(key)) {
			return cache->fetch(key);
		}
		else {
			V value = TestDriver<K, V>::get_value();
			cache->insert(key, value);
			return value;
		}
	}
	
	std::auto_ptr<LRUCache<K, V> > cache;
};


template<class K, class V>
struct TestComparator
{
	TestComparator(const TestParams & params)
		: params(params)
	{
	}
	
	void do_test(TestCase tc)
	{
		if (params.show_header) {
			cerr << "-------------------------------------" << endl;
			cerr << params.name() << endl;
		}
		
		TestParams sub = params;
		sub.show_header = false;
		sub.report_memory = false;
		sub.report_cpu = false;
		
		TestDriverPLB<K, V> plb(sub);
		plb.do_test(tc);
		
		TestDriverPA<K, V> pa(sub);
		pa.do_test(tc);
		
		bool ret = check(*plb.cache, *pa.cache);
		cerr << "ret=" << ret << endl;
	}
	
	bool check(const plb::LRUCacheH4<K, V> & plb_cache,
			   /*const*/ LRUCache<K, V> & pa_cache) const
	{
		const int plb_size = plb_cache.size();
		const int pa_size = pa_cache.size();
		
		if (plb_size != pa_size)
		{
			cerr << params.name() << ": sizes: " << plb_size << " vs " << pa_size << endl;
			return false;
		}
		
		vector<int> pa_keys = pa_cache.get_all_keys();
		int i = 0;
		for (plb::LRUCacheH4<int, int>::const_iterator it = plb_cache.mru_begin();  it != plb_cache.end();  ++it, ++i)
		{
			const K & plb_key = it.key();
			const K & pa_key = pa_keys[i];
			if (plb_key != pa_key) {
				cerr << params.name() << ": key " << i << ": " << plb_key << " vs " << pa_key << endl;
				return false;
			}
			
			const V & plb_value = it.value();
			const V pa_value = pa_cache.fetch(pa_key, false /* touch_data */);
			if (plb_value != pa_value) {
				cerr << params.name() << ": value " << i << ": " << plb_value << " vs " << pa_value << endl;
				return false;
			}
		}
		
		return true;
	}

	TestParams params;
};


enum Action {
	RUN_PLB = 0,
	RUN_PA = 1,
	CORRECTNESS = 2
};


ostream & operator<<(ostream & os, const Action & a)
{
	return os << (a == RUN_PLB ? "RUN_PLB" :
		          a == RUN_PA ? "RUN_PA" :
		          a == CORRECTNESS ? "CORRECTNESS" :
		          "ACTION_UNKNOWN");
}


int main(int argc, char ** argv)
{
	TestCase tc = TEST_CASE_INSERT;
	Action action = CORRECTNESS;
	
	for (int i = 1;  i < argc;  ++i) {
		string a = argv[i];
		if (a == "RUN_PLB") action = RUN_PLB;
		else if (a == "RUN_PA") action = RUN_PA;
		else if (a == "CORRECTNESS") action = CORRECTNESS;
		else if (a == "TEST_CASE_INSERT") tc = TEST_CASE_INSERT;
		else if (a == "TEST_CASE_INSERT_READ") tc = TEST_CASE_INSERT_READ;
		else cerr << "Unrecognized option: " << a << endl;
	}
	
	cerr << "tc: " << tc << endl;
	cerr << "action: " << action << endl;
	
	vector<TestParams> tests
		= boost::assign::list_of<TestParams>
			(TestParams(20, 1000, 10))
			(TestParams(1000, 500, 10000000))
			(TestParams(10, 5000, 1000000))
			(TestParams(1000, 5000, 1000000))
			(TestParams(1000, 1000000, 10000000))
			(TestParams(1000000, 1000, 10000000))
			(TestParams(1000000, 10000000, 10000000))
			(TestParams(2000000, 20000000, 20000000))
			(TestParams(3000000, 30000000, 30000000));
			(TestParams(5000000, 50000000, 50000000));
	
	if (action == RUN_PLB) {
		// cpu time + memory usage of PLB cache
		show_memory_usage();
		for (int i = 0;  i < tests.size();  ++i) {
			TestDriverPLB<int, int> driver(tests[i]);
			driver.do_test(tc);
		}
	}
	
	else if (action == RUN_PA) {
		// cpu time + memory usage of PA cache
		show_memory_usage();
		for (int i = 0;  i < tests.size();  ++i) {
			TestDriverPA<int, int> driver(tests[i]);
			driver.do_test(tc);
		}
	}
	
	else if (action == CORRECTNESS) {
		// make sure all caches give the same sequence
		for (int i = 0;  i < tests.size();  ++i) {
			TestComparator<int, int> driver(tests[i]);
			driver.do_test(tc);
		}
	}
	
	return 0;
}
