/*
 * A read-only interface to /proc/pid/smaps
 *
 * Released as part of lru-cpp-cache:  http://code.google.com/p/lru-cache-cpp/
 *
 * Reference: http://linux.die.net/man/5/proc
 *
 * Licensed under the GNU LGPL: http://www.gnu.org/copyleft/lesser.html
 *
 * Pierre-Luc Brunelle, 2011
 * pierre-luc.brunelle@polytml.ca
 *
 */

#include <memory>
#include <string>
#include <vector>


namespace plb {

struct smaps_entry
{
	smaps_entry()
		: size(0), rss(0),
		  shared_clean(0), shared_dirty(0),
		  private_clean(0), private_dirty(0)
	{
	}
	
	smaps_entry & operator+=(const smaps_entry & other);
	
	// in kilobytes
	int size;
	int rss;
	int shared_clean;
	int shared_dirty;
	int private_clean;
	int private_dirty;
};

std::ostream & operator<<(std::ostream & os, const smaps_entry & obj);


struct smaps_header
{
	smaps_header()
		: address_start(0), address_end(0), offset(0),
		  device_major(0), device_minor(0), inode(0),
		  pathname(),
		  perm_read(false), perm_write(false), perm_execute(false),
		  perm_shared(false), perm_copy_on_write(false)
	{
	}
	
	unsigned long address_start;
	unsigned long address_end;
	unsigned long offset;
	int device_major;
	int device_minor;
	int inode;
	std::string pathname;
	bool perm_read;
	bool perm_write;
	bool perm_execute;
	bool perm_shared;
	bool perm_copy_on_write;
};

std::ostream & operator<<(std::ostream & os, const smaps_header & obj);


typedef std::vector<std::pair<smaps_header, smaps_entry> > smaps_vector;

struct smaps : public smaps_vector
{
	const_iterator find(const std::string pathname) const;
	
	smaps_entry total() const;
	
	static const std::string STACK;
	static const std::string HEAP;
};

std::ostream & operator<<(std::ostream & os, const smaps & obj);


// for current process
// the return value contains a NULL pointer if we could not read the smaps of the current process
std::auto_ptr<smaps> read_smaps();

// for some other process
// the return value contains a NULL pointer if we could not read the smaps of pid
std::auto_ptr<smaps> read_smaps(int pid);

}  // namespace plb
