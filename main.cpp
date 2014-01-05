#include "functions.hpp"
#include "aho.hpp"
#include "trie.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <exception>
#include <cstdlib>
#include <sys/stat.h> // chmod()
#include <cstring>

#define LOG(x) std::cerr << #x << ": " << x << endl;

template<typename type>
class MutexQueue
{
private:
	std::mutex once_operation;
	std::queue<type> que;

public:
	MutexQueue(){}
	~MutexQueue(){}

	const std::queue<type>& queue() const
	{return this->que;}

	typename std::queue<type>::value_type& front()
	{return this->que.front();}

	const typename std::queue<type>::value_type& front() const
	{return this->que.front();}

	typename std::queue<type>::value_type& back()
	{return this->que.back();}

	const typename std::queue<type>::value_type& back() const
	{return this->que.back();}

	bool empty() const
	{return this->que.empty();}

	typename std::queue<type>::size_type size() const
	{return this->que.size();}

	void push(const type& str)
	{
		this->once_operation.lock();
		this->que.push(str);
		this->once_operation.unlock();
	}

	void pop()
	{
		this->once_operation.lock();
		this->que.pop();
		this->once_operation.unlock();
	}

	type extract()
	{
		this->once_operation.lock();
		if(this->que.empty())
		{
			this->once_operation.unlock();
			throw true;
		}
		type out=this->que.front();
		this->que.pop();
		this->once_operation.unlock();
		return out;
	}
};

using namespace std;

const unsigned THREADS=10;

mutex global_lock;

MutexQueue<std::string> download_queue;
MutexQueue<unsigned> free_threads;

vector<thread> threads;

Trie sites_base, wrong_sites_base;
IgnoreTrie ignored_sites;

void download(int thread_id)
{
download_function_begin:
	try
	{
		string site=download_queue.extract();
	}
	catch(...)
	{
		free_threads.push(thread_id);
		// We have one more free thread
		if(free_threads.size()==THREADS) global_lock.unlock();
		return;
	}
	/////////////
	// Searching for free threads of downloading
	while(!free_threads.empty() && !download_queue.empty())
	{
		try
		{
			int id=free_threads.extract();
			if(threads[id].joinable()) goto download_function_begin;
			threads[id]=thread(download, id);
		}
		catch(...)
		{goto download_function_begin;}
	}
	goto download_function_begin;
}

class temporary_directory
{
	char* _M_name;
public:
	explicit temporary_directory(const char* new_name): _M_name(new char[strlen(new_name)+2])
	{
		unsigned size=strlen(new_name);
		memcpy(this->_M_name, new_name, size);
		this->_M_name[size]=this->_M_name[size+1]='\0';
		if(NULL==mkdtemp(this->_M_name))
		{
			struct exception : std::exception
			{
				const char* what() const _GLIBCXX_USE_NOEXCEPT {return "Cannot create tmp directory\n";}
			};
			throw exception();
		}
		this->_M_name[size]='/';
		chmod(this->_M_name, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	}

	~temporary_directory()
	{
		remove_r(this->_M_name);
		delete[] this->_M_name;
	}

	const char* name() const
	{return this->_M_name;}

	operator const char*() const
	{return this->_M_name;}
} tmp_dir("sites-downloader.XXXXXX");

inline int system(const string& str)
{return system(str.c_str());}

int main(int argc, char **argv)
{
	if(argc<2)
	{
		cout << "Usage: sites-downloader <site> [options]\nOptions:\n    -iPAGE_URL      used to ignore urls with prefix PAGE_URL\n    -wWRONGS_FILE   used to repair/continue download page, WRONGS_FILE is file to which the program prints the error logs, you can use only one this option" << endl;
		exit(1);
	}
	global_lock.lock();
	threads.resize(THREADS);
	for(int i=1; i<THREADS; ++i)
		free_threads.push(i);
	// Extract server name
	string server, root_dir;
	download_queue.push(server=argv[1]);
	if(0==server.compare(0, 7, "http://"))
		server.erase(0, 7);
	else if(0==server.compare(0, 8, "https://"))
		server.erase(0, 8);
	int eraser=-1;
	while(++eraser<server.size() && server[eraser]!='/');
	server.erase(eraser);
	LOG(server);
	system("pwd > "+string(tmp_dir.name())+"/pwd");
	root_dir=GetFileContents(string(tmp_dir.name())+"/pwd");
	root_dir.back()='/';
	LOG(root_dir);
	// Run downloading
	threads[0]=thread(download, 0);
	global_lock.lock();
	for(int i=0; i<THREADS; ++i)
		if(threads[i].joinable())
			threads[i].join();
return 0;
}