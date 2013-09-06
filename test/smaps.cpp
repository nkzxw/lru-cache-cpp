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


#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <cstdio>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include "smaps.hpp"


namespace plb {

smaps_entry & smaps_entry::operator+=(const smaps_entry & other)
{
	this->size           += other.size;
	this->rss            += other.rss;
	this->shared_clean   += other.shared_clean;
	this->shared_dirty   += other.shared_dirty;
	this->private_clean  += other.private_clean;
	this->private_dirty  += other.private_dirty;
}

std::ostream & operator<<(std::ostream & os, const smaps_entry & obj)
{
	return os << "size/rss/shared/private: "
	          << obj.size << "/" << obj.rss << "/"
			  << obj.shared_clean << ":" << obj.shared_dirty << "/"
			  << obj.private_clean << ":" << obj.private_dirty;
}


std::ostream & operator<<(std::ostream & os, const smaps_header & obj)
{
	return os << "addr/dev/inode/perm/name: "
	          << obj.address_start << ":" << obj.address_end << ":" << obj.offset << "/"
			  << obj.inode << ":" << obj.device_major << ":" << obj.device_minor << "/"
			  << (obj.perm_read ? "r" : "")
			  << (obj.perm_write ? "w" : "")
			  << (obj.perm_execute ? "x" : "")
			  << (obj.perm_shared ? "s" : "")
			  << (obj.perm_copy_on_write ? "p" : "") << "/"
			  << obj.pathname;
}


smaps::const_iterator smaps::find(const std::string pathname) const
{
	for (const_iterator it = begin();  it != end();  ++it)
		if (it->first.pathname == pathname)
			return it;
	return end();
}


smaps_entry smaps::total() const
{
	smaps_entry ret;
	for (const_iterator it = begin();  it != end();  ++it)
		ret += it->second;
	return ret;
}


const std::string smaps::STACK = "[stack]";
const std::string smaps::HEAP = "[heap]";
	

std::ostream & operator<<(std::ostream & os, const smaps & obj)
{
	for (smaps::const_iterator it = obj.begin();  it != obj.end();  ++it)
		os << "-------------------------------------------" << std::endl
		   << it->first << std::endl
		   << it->second << std::endl;
	return os;
}


template<class T>
T from_hex_string(const std::string & s)
{
    T x;
    std::stringstream ss;
    ss << std::hex << s;
    ss >> x;
	return x;
}

	
std::auto_ptr<smaps> parse_smaps(std::ifstream & fin)
{
	static const boost::regex header_re("(\\w+)-(\\w+) *([rwxsp-]+) (\\w+) (\\w{2}):(\\w{2}) (\\d+) +([^ ]+)");
	static const boost::regex entry_re("(\\w+): +(\\d+) kB");
	
	std::auto_ptr<smaps> ret(new smaps());
	
	std::string line;
	boost::smatch what;
	while (getline(fin, line))
	{
		//std::cerr << line << std::endl;
		if (regex_match(line, what, header_re))
		{
			ret->resize(ret->size() + 1);
			smaps_header & header       = ret->back().first;
			std::string perms           = what[3];
			header.address_start        = from_hex_string<unsigned long>(what[1]);
			header.address_end          = from_hex_string<unsigned long>(what[2]);
			header.offset               = from_hex_string<unsigned long>(what[4]);
			header.device_major         = from_hex_string<int>(what[5]);
			header.device_minor         = from_hex_string<int>(what[6]);
			header.inode                = boost::lexical_cast<int>(what[7]);
			header.pathname             = what[8];
			header.perm_read            = perms.find('r') != std::string::npos;
			header.perm_write           = perms.find('w') != std::string::npos;
			header.perm_execute         = perms.find('x') != std::string::npos;
			header.perm_shared          = perms.find('s') != std::string::npos;
			header.perm_copy_on_write   = perms.find('p') != std::string::npos;
		}
		else if (regex_match(line, what, entry_re))
		{
			if (ret->empty())
				throw "parse_smaps: saw entry before header";
			smaps_entry & entry         = ret->back().second;
			std::string entry_name      = what[1];
			int kbytes                  = boost::lexical_cast<int>(what[2]);
			int * target = 
				(entry_name == "Size")           ? &entry.size :
				(entry_name == "Rss")            ? &entry.rss :
				(entry_name == "Shared_Clean")   ? &entry.shared_clean : 
				(entry_name == "Shared_Dirty")   ? &entry.shared_dirty : 
				(entry_name == "Private_Clean")  ? &entry.private_clean : 
				(entry_name == "Private_Dirty")  ? &entry.private_dirty : 
				NULL;
			if (target)
				*target = kbytes;
			else {
				//std::cerr << "parse_smaps: unrecognized entry: " << line << std::endl;
			}
		}
		else
		{
			//std::cerr << "parse_smaps: unrecognized line: " << line << std::endl;
		}
	}
	
	return ret;
}


std::auto_ptr<smaps> read_smaps()
{
	return read_smaps(getpid());
}


std::auto_ptr<smaps> read_smaps(int pid)
{
	std::string smaps_file("/proc/" + boost::lexical_cast<std::string>(pid) + "/smaps");
	std::ifstream fsmaps(smaps_file.c_str());
	return parse_smaps(fsmaps);
}


}  // namespace plb
