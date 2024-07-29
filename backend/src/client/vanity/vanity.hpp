#include "Windows.h"
#include "Tlhelp32.h"
#include <cstdint>

class Vanity // For ttdclient VanitySearch subprocess
{
private:
	const uintptr_t update_time_offset = 0x415A2;

	uint32_t pid = 0;
	uint32_t search_thread_id = 0;

	uint32_t FindSearchThread(); // Find the main VanitySearch search thread that has search variables in its xmm registers
	double ReadXmm(const uint32_t& register_index); // Read Xmm9 register as a double
	
public:
	bool ChangeUpdateDelay(const uint32_t& ms); /* true if succeeds, false if fails
												  VanitySearch updates key rate, percent completed, and other variables every 500 ms by default */

	// get
	double GetKeyRate() { return this->ReadXmm(9); }
	double GetPercentCompleted() { return this->ReadXmm(7); }

	Vanity(const uint32_t& pid) : pid(pid)
	{
		this->search_thread_id = FindSearchThread();
	}
};

