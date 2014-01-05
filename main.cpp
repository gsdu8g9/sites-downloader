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
#include <csignal>
#include <fstream>

#define LOG(x) std::cerr << #x << ": " << x << endl;
#define INLINELOG(x) std::cerr << #x << ": " << x << flush;

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

	std::queue<type>& unsafe_queue()
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

	void lock()
	{this->once_operation.lock();}

	void unlock()
	{this->once_operation.unlock();}

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
			return type();
		}
		type out=this->que.front();
		this->que.pop();
		this->once_operation.unlock();
		return out;
	}
};

using namespace std;

unsigned THREADS=1;

mutex global_lock, loging;

MutexQueue<std::string> download_queue;
MutexQueue<unsigned> free_threads;

vector<thread> threads;

string server, root_dir;
Trie sites_base, wrong_sites_base;
IgnoreTrie ignored_sites;
special_aho model_parse;
/* 0 - "href="
*  1 - "src="
*  2 - "url("
*/
const_string link("");

inline int system(const string& str)
{return system(str.c_str());}

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

void download(int thread_id)
{
	string site;
download_function_begin:
	if(download_queue.empty())
	{
	download_exit:
		free_threads.push(thread_id);
		// We have one more free thread
		if(free_threads.size()==THREADS) global_lock.unlock();
		return;
	}
	site=download_queue.extract();
	if(site.empty()) goto download_exit;
	/////////////
	loging.lock();
	cout << "\033[01;34mDownloading: " << site << "\033[00m\n" << flush;
	loging.unlock();
	string tmp_file_name=string(tmp_dir.name())+"/download"+myto_string(thread_id);
	if(0==system("wget --no-check-certificate --connect-timeout=23 --tries=3 --no-use-server-timestamps --unlink -x -nv "+to_shell(site)+" 2> "+tmp_file_name))
	{
		string tmp_file=GetFileContents(tmp_file_name), downloaded_file_name, downloaded_file, new_content;
		for(int i=5, tfs=tmp_file.size(); i<tfs; ++i)
		{
			if(0==tmp_file.compare(i-5, 5, " -> \""))
			{
				--i;
				while(++i<tfs && tmp_file[i]!='"')
					downloaded_file_name+=tmp_file[i];
				break;
			}
		}
		if(downloaded_file_name.empty()) goto download_error;
		loging.lock();
		cout << "\033[01;34mSite: " << site << "\n\033[01;32mFile: " << downloaded_file_name << "\033[00m\n" << tmp_file << flush;
		loging.unlock();
		downloaded_file=GetFileContents(downloaded_file_name);
		special_aho parse=model_parse;
		parse.find(downloaded_file);
		for(int i=0, dfs=downloaded_file.size(); i<dfs; ++i)
		{
			if(parse[i]>=0)
			{
				int patt_id=parse[i];
				i+=parse.pattern(patt_id).first.size();
				while(i<dfs && (downloaded_file[i]==' ' || downloaded_file[i]=='\t')) ++i;
				char string_char=downloaded_file[i];
				if(patt_id==2 && string_char!='\'' && string_char!='"')
				{
					string_char=')';
					--i;
					new_content+=parse.pattern(patt_id).first;
				}
				else 
				{
					new_content+=parse.pattern(patt_id).first+string_char;
					if(string_char!='\'' && string_char!='"') continue;
				}
				/*INLINELOG(i);
				cout << " ; ";
				INLINELOG(patt_id);
				cout << " ; ";
				INLINELOG(string_char);
				cout << endl;*/
				string new_site, original_new_site;
				while(++i<dfs && downloaded_file[i]!=string_char)
					new_site+=downloaded_file[i];
				original_new_site=new_site;
				// LOG(new_site);
				if(0==new_site.compare(0, 7, "http://") || 0==new_site.compare(0, 8, "https://"))
				{
					eraseHTTPprefix(new_site);
					if(!(0==new_site.compare(0, server.size(), server) || 0==new_site.compare(0, 4+server.size(), "www."+server)))
					{
						new_content+=original_new_site+string_char;
						continue;
					}
				}
				// LOG(new_site);
				if(0==new_site.compare(0, server.size(), server))
					new_site.erase(0, server.size());
				if(0==new_site.compare(0, 4+server.size(), "www."+server))
					new_site.erase(0, 4+server.size());
				// LOG(new_site);
				if(new_site[0]!='/')
					new_site=without_end_after_slash(downloaded_file_name)+new_site;
				else
					new_site=server+new_site;
				absolute_path(new_site).swap(new_site);
				convert_from_HTML(new_site).swap(new_site);
				/*cout << "\033[01;34m";
				LOG(new_site);
				cout  << "\033[00m";*/
				if(is_good_name(new_site) && !ignored_sites.is_ignored(new_site) && !sites_base.insert(new_site))
				{
					loging.lock();
					cout << "\033[01;33m" << new_site << "\033[00m\n";
					loging.unlock();
					download_queue.push(new_site);
					new_content+=get_path(downloaded_file_name, (new_site.back()=='/' ? new_site+"index.html":new_site));
				}
				else if(!is_good_name(new_site))
					new_content+=original_new_site;
				else
					new_content+=get_path(downloaded_file_name, (new_site.back()=='/' ? new_site+"index.html":new_site));
				new_content+=string_char;
			}
			else
				new_content+=downloaded_file[i];
		}
		fstream file(downloaded_file_name.c_str(), ios::out);
		if(file.good())
		{
			// cerr << new_content << endl;
			file << new_content;
			file.close();
		}
		else cerr << "Error overwriting file: " << downloaded_file_name << endl;
	}
	else
	{
	download_error:
		loging.lock();
		cout << "\033[01;31mError: \033[00m" << site << "\033[00m\n" << GetFileContents(tmp_file_name) << flush;
		loging.unlock();
	}
	// Searching for free threads of downloading
	while(!download_queue.empty())
	{
		free_threads.lock();
		if(free_threads.empty())
		{
			free_threads.unlock();
			goto download_function_begin;
		}
		int id=free_threads.front();
		if(threads[id].joinable())
		{
			free_threads.unlock();
			goto download_function_begin;
		}
		free_threads.unsafe_queue().pop();
		free_threads.unlock();
		threads[id]=thread(download, id);
	}
	goto download_function_begin;
}

void control_exit(int ___nothing=0)
{
	download_queue.lock();
	free_threads.lock();
	for(int i=0; i<THREADS; ++i)
		if(threads[i].joinable())
			threads[i].detach();
	remove_r(tmp_dir);
	exit(1);
}

int main(int argc, char **argv)
{
	// signal control
	signal(SIGHUP, control_exit);
	signal(SIGINT, control_exit);
	signal(SIGQUIT, control_exit);
	signal(SIGILL, control_exit);
	signal(SIGTRAP, control_exit);
	signal(SIGABRT, control_exit);
	signal(SIGIOT, control_exit);
	signal(SIGBUS, control_exit);
	signal(SIGFPE, control_exit);
	signal(SIGKILL, control_exit); // We won't block SIGKILL
	signal(SIGUSR1, control_exit);
	signal(SIGSEGV, control_exit);
	signal(SIGUSR2, control_exit);
	signal(SIGPIPE, control_exit);
	signal(SIGALRM, control_exit);
	signal(SIGTERM, control_exit);
	signal(SIGSTKFLT, control_exit);
	signal(_NSIG, control_exit);
	if(argc<2)
	{
		cout << "Usage: sites-downloader <sites...> [options]\nSites have to belong to one server\nOptions:\n    -i PAGE_URL         Set ignore urls with prefix PAGE_URL\n    -w WRONGS_FILE      Repair/continue download page, WRONGS_FILE is file to which the program prints the error logs, you can use only one this option\n    -j [N], --jobs[=N]  Allow N jobs at once" << endl;
		exit(1);
	}
	for(int i=1; i<argc; ++i)
	{
		static bool have_wrongs_file=false;
		if(0==memcmp(argv[i], "-i", 2))
		{
			string site(argv[i]+2);
			if(site.empty())
			{
				if(++i<argc)
					site.assign(argv[i]);
				else
					continue;
			}
			cout << "Ignored prefix: " << site << endl;
			ignored_sites.insert(site);
		}
		else if(0==memcmp(argv[i], "-w", 2))
		{
			if(have_wrongs_file)
			{
				cerr << "Sites-downloader can use only one \"wrongs\" file!\n";
				exit(1);
			}
			have_wrongs_file=true;
			// string _s;
			// for(int j=2; argv[i][j]!='\0'; ++j)
			// 	_s+=argv[i][j];
			// fstream wr(_s.c_str(), ios::in);
			// if(wr.good())
			// {
			// 	string kit;
			// 	getline(wr, kit);
			// 	var_base::make_from_memoty_dump(kit);
			// 	getline(wr,server);
			// 	while(wr.good())
			// 	{
			// 		getline(wr,kit);
			// 		if(!kit.empty()/* && !var_base::find_var(kit)*/)
			// 			{sites.push(kit);cout << "\t" << kit << endl;}
			// 	}
			// }
			// else
			// {
			// 	cout << "error opening file: " << _s << "!" << endl;
			// 	abort();
			// }
			// wr.close();
		}
		else if(0==memcmp(argv[i], "-j", 2))
		{
			string number(argv[i]+2);
			if(number.empty() && ++i<argc)
				number.assign(argv[i]);
			if(is_number(number))
				THREADS=to_int(number);
			else
			{
				cerr << number << ": isn't a number" << endl;
				return 1;
			}
		}
		else if(0==memcmp(argv[i], "--jobs", 6))
		{
			string number;
			if(argv[i][6]=='=')
			{
				number.assign(argv[i]+7);
			check_number:
				if(is_number(number))
					THREADS=to_int(number);
				else
				{
					cerr << number << ": isn't a number" << endl;
					return 1;
				}
			}
			else if(++i<argc)
			{
				number.assign(argv[i]);
				goto check_number;
			}
			else
			{
				cerr << "Wrong --jobs option use" << endl;
				return 1;
			}
		}
		else download_queue.push(argv[i]);
	}
	cerr << THREADS << endl;
	global_lock.lock();
	threads.resize(THREADS);
	for(int i=1; i<THREADS; ++i)
		free_threads.push(i);
	// Extract server name
	if(download_queue.empty())
	{
		cerr << "You have to define at least one site" << endl;
		exit(1);
	}
	server=download_queue.front();
	eraseHTTPprefix(server);
	int eraser=-1;
	while(++eraser<server.size() && server[eraser]!='/');
	server.erase(eraser);
	LOG(server);
	system("pwd > "+string(tmp_dir.name())+"/pwd");
	root_dir=GetFileContents(string(tmp_dir.name())+"/pwd");
	root_dir.back()='/';
	LOG(root_dir);
	// Initialize model_parse
	vector<pair<string, const_string> > parse_patterns{make_pair("href=", link), make_pair("src=", link), make_pair("url(", link)};
	model_parse.set_patterns(parse_patterns);
	// Run downloading
	threads[0]=thread(download, 0);
	global_lock.lock();
	for(int i=0; i<THREADS; ++i)
		if(threads[i].joinable())
			threads[i].join();
return 0;
}