#include <string>
#include <cstdint>
#include <vector>
#include <set>
#include "Windows.h"

#include "vanity/vanity.hpp"


class Client
{
private:
	uint32_t pid = 0;
	uint32_t vanitysearch_pid = 0;
	std::vector<uint32_t> subprocesses = {};
	
	uint32_t FindPID(const std::string& process_name);
	std::vector<uint32_t> FindSubProcesses(const uint32_t& parent_pid); // returns { 0 } if fails/no subprocesses found
	std::string FindProcessName(const uint32_t& pid);

public:
	Vanity* vanitysearch_process = nullptr;

	void UpdateSubProcesses();

	// get
	uint32_t GetPID() const { return this->pid; }
	std::vector<uint32_t> GetSubProcesses() const { return this->subprocesses; }
	uint32_t GetVanityPID() const { return this->vanitysearch_pid; }

	Client(const std::string& process_name)
	{
		this->pid = FindPID(process_name); // Find PID of process
		if (this->pid > 0) // if PID was found successfully then identify subprocesses
		{
			this->subprocesses = FindSubProcesses(this->pid);
			UpdateSubProcesses(); // populate vanitysearch_pid

			vanitysearch_process = new Vanity(this->vanitysearch_pid);
		}
		
		
	}

	~Client() { delete this->vanitysearch_process;  }
};