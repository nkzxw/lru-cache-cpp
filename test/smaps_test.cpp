#include <iostream>
#include "smaps.hpp"
#include <boost/lexical_cast.hpp>

using namespace std;

int main()
{
	std::auto_ptr<plb::smaps> smaps_ptr = plb::read_smaps();
	if (smaps_ptr.get()) {
		cerr << *smaps_ptr << endl;
		plb::smaps::const_iterator heap_it = smaps_ptr->find(plb::smaps::HEAP);
		if (heap_it != smaps_ptr->end()) {
			cerr << "--- HEAP ---" << endl; 
			cerr << heap_it->first << endl;
			cerr << heap_it->second << endl;
		}
	}
	return 0;
}