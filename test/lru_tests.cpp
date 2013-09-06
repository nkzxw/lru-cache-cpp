//-------------------------------------------------------------
// Test cases
//-------------------------------------------------------------

#include <iostream>
#include <memory>
#include <vector>
#include <boost/assign/list_of.hpp>
#include "lru.hpp"

using namespace plb;

// See test functions below on how to use this struct
template<class K, class V>
struct LRUCacheH4TestCase {
	typedef LRUCacheH4<K, V> Cache;
	typedef std::pair<K, V> Pair;
	typedef std::vector<Pair> Vector;
	
	Cache _cache;
	Vector _expected;  // from MRU to LRU
	
	LRUCacheH4TestCase(int size) : _cache(size)
	{
	}
	
	Vector vector_mru_to_lru(const Cache & c) const
	{
		Vector ret;
		for (typename Cache::const_iterator it = c.mru_begin();  it != c.end();  ++it)
			ret.push_back(Pair(it.key(), it.value()));
		return ret;
	}

	bool test() const
	{
		Vector vc = vector_mru_to_lru(_cache);
		bool ret = _expected == vc;
		std::cerr << "test: " << ret << std::endl;
		return ret;
	}
};

typedef LRUCacheH4TestCase<int, int> LRUCacheH4TestCaseII;
typedef std::pair<int, int> PairII;

std::auto_ptr<LRUCacheH4TestCaseII> T1()
{
	// cache empty
	std::auto_ptr<LRUCacheH4TestCaseII> tc(new LRUCacheH4TestCaseII(3));
	return tc;
}

std::auto_ptr<LRUCacheH4TestCaseII> T2()
{
	// cache almost empty: 1 item
	std::auto_ptr<LRUCacheH4TestCaseII> tc(new LRUCacheH4TestCaseII(3));
	tc->_cache.insert(1, 101);
	tc->_expected = boost::assign::list_of<PairII>(PairII(1, 101));
	return tc;
}

std::auto_ptr<LRUCacheH4TestCaseII> T3()
{
	// cache almost empty: 1 item
	std::auto_ptr<LRUCacheH4TestCaseII> tc(new LRUCacheH4TestCaseII(3));
	tc->_cache.insert(1, 101);
	tc->_cache.insert(2, 102);
	tc->_expected = boost::assign::list_of<PairII>(PairII(2, 102))(PairII(1, 101));
	return tc;
}

std::auto_ptr<LRUCacheH4TestCaseII> T4()
{
	// cache full
	std::auto_ptr<LRUCacheH4TestCaseII> tc(new LRUCacheH4TestCaseII(3));
	tc->_cache.insert(1, 101);
	tc->_cache.insert(2, 102);
	tc->_cache.insert(3, 103);
	tc->_expected = boost::assign::list_of<PairII>(PairII(3, 103))(PairII(2, 102))(PairII(1, 101));
	return tc;
}

std::auto_ptr<LRUCacheH4TestCaseII> T5()
{
	// 1 item, updating lru
	std::auto_ptr<LRUCacheH4TestCaseII> tc(new LRUCacheH4TestCaseII(3));
	tc->_cache.insert(1, 101);
	tc->_cache.insert(1, 1001);
	tc->_expected = boost::assign::list_of<PairII>(PairII(1, 1001));
	return tc;
}

std::auto_ptr<LRUCacheH4TestCaseII> T6()
{
	// some items, updating lru
	std::auto_ptr<LRUCacheH4TestCaseII> tc(new LRUCacheH4TestCaseII(3));
	tc->_cache.insert(1, 101);
	tc->_cache.insert(2, 102);
	tc->_cache.insert(1, 1001);
	tc->_expected = boost::assign::list_of<PairII>(PairII(1, 1001))(PairII(2, 102));
	return tc;
}

std::auto_ptr<LRUCacheH4TestCaseII> T7()
{
	// cache full, updating lru
	std::auto_ptr<LRUCacheH4TestCaseII> tc(new LRUCacheH4TestCaseII(3));
	tc->_cache.insert(1, 101);
	tc->_cache.insert(2, 102);
	tc->_cache.insert(3, 103);
	tc->_cache.insert(1, 1001);
	tc->_expected = boost::assign::list_of<PairII>(PairII(1, 1001))(PairII(3, 103))(PairII(2, 102));
	return tc;
}

std::auto_ptr<LRUCacheH4TestCaseII> T8()
{
	// some items, updating middle
	std::auto_ptr<LRUCacheH4TestCaseII> tc(new LRUCacheH4TestCaseII(4));
	tc->_cache.insert(1, 101);
	tc->_cache.insert(2, 102);
	tc->_cache.insert(3, 103);
	tc->_cache.insert(2, 1002);
	tc->_expected = boost::assign::list_of<PairII>(PairII(2, 1002))(PairII(3, 103))(PairII(1, 101));
	return tc;
}

std::auto_ptr<LRUCacheH4TestCaseII> T9()
{
	// cache full, updating middle
	std::auto_ptr<LRUCacheH4TestCaseII> tc(new LRUCacheH4TestCaseII(3));
	tc->_cache.insert(1, 101);
	tc->_cache.insert(2, 102);
	tc->_cache.insert(3, 103);
	tc->_cache.insert(2, 1002);
	tc->_expected = boost::assign::list_of<PairII>(PairII(2, 1002))(PairII(3, 103))(PairII(1, 101));
	return tc;
}

std::auto_ptr<LRUCacheH4TestCaseII> T10()
{
	// some items, updating mru
	std::auto_ptr<LRUCacheH4TestCaseII> tc(new LRUCacheH4TestCaseII(3));
	tc->_cache.insert(1, 101);
	tc->_cache.insert(2, 102);
	tc->_cache.insert(2, 1002);
	tc->_expected = boost::assign::list_of<PairII>(PairII(2, 1002))(PairII(1, 101));
	return tc;
}

std::auto_ptr<LRUCacheH4TestCaseII> T11()
{
	// cache full, updating mru
	std::auto_ptr<LRUCacheH4TestCaseII> tc(new LRUCacheH4TestCaseII(3));
	tc->_cache.insert(1, 101);
	tc->_cache.insert(2, 102);
	tc->_cache.insert(3, 103);
	tc->_cache.insert(3, 1003);
	tc->_expected = boost::assign::list_of<PairII>(PairII(3, 1003))(PairII(2, 102))(PairII(1, 101));
	return tc;
}

std::auto_ptr<LRUCacheH4TestCaseII> T12()
{
	// cache full, inserting new
	std::auto_ptr<LRUCacheH4TestCaseII> tc(new LRUCacheH4TestCaseII(3));
	tc->_cache.insert(1, 101);
	tc->_cache.insert(2, 102);
	tc->_cache.insert(3, 103);
	tc->_cache.insert(4, 104);
	tc->_expected = boost::assign::list_of<PairII>(PairII(4, 104))(PairII(3, 103))(PairII(2, 102));
	return tc;
}

std::auto_ptr<LRUCacheH4TestCaseII> T13()
{
	// cache full, updating lru, inserting new
	std::auto_ptr<LRUCacheH4TestCaseII> tc(new LRUCacheH4TestCaseII(3));
	tc->_cache.insert(1, 101);
	tc->_cache.insert(2, 102);
	tc->_cache.insert(3, 103);
	tc->_cache.insert(1, 1001);
	tc->_cache.insert(4, 104);
	tc->_expected = boost::assign::list_of<PairII>(PairII(4, 104))(PairII(1, 1001))(PairII(3, 103));
	return tc;
}

std::auto_ptr<LRUCacheH4TestCaseII> T14()
{
	// cache full, updating lru, updating lru, inserting new
	std::auto_ptr<LRUCacheH4TestCaseII> tc(new LRUCacheH4TestCaseII(3));
	tc->_cache.insert(1, 101);
	tc->_cache.insert(2, 102);
	tc->_cache.insert(3, 103);
	tc->_cache.insert(1, 1001);
	tc->_cache.insert(2, 1002);
	tc->_cache.insert(4, 104);
	tc->_expected = boost::assign::list_of<PairII>(PairII(4, 104))(PairII(2, 1002))(PairII(1, 1001));
	return tc;
}

std::auto_ptr<LRUCacheH4TestCaseII> T15()
{
	// cache full, updating lru, updating mru, inserting new
	std::auto_ptr<LRUCacheH4TestCaseII> tc(new LRUCacheH4TestCaseII(3));
	tc->_cache.insert(1, 101);
	tc->_cache.insert(2, 102);
	tc->_cache.insert(3, 103);
	tc->_cache.insert(1, 1001);
	tc->_cache.insert(1, 10001);
	tc->_cache.insert(4, 104);
	tc->_expected = boost::assign::list_of<PairII>(PairII(4, 104))(PairII(1, 10001))(PairII(3, 103));
	return tc;
}

int main()
{
	// TODO: large-scale tests, memory, CPU, complexity
	T1()->test();
	T2()->test();
	T3()->test();
	T4()->test();
	T5()->test();
	T6()->test();
	T7()->test();
	T8()->test();
	T9()->test();
	T10()->test();
	T11()->test();
	T12()->test();
	T13()->test();
	T14()->test();
	T15()->test();
	
	return 0;
}