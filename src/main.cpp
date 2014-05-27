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

#define eprint(...) fprintf(stderr, __VA_ARGS__)

#ifdef DEBUG
#define LOGS
#define D(...) __VA_ARGS__
#define LOGN(x) std::cerr << #x << ": " << x << endl;
#define LOG(x) std::cerr << #x << ": " << x << flush;
#else
#define D(...)
#define LOGN(x)
#define LOG(x)
#endif

class const_string
{
	char* _M_str;
public:
	explicit const_string(const char* _str): _M_str(new char[strlen(_str)+1])
	{
		memcpy(_M_str, _str, strlen(_str)+1);
	}
	const_string(const const_string& _cstr): _M_str(_cstr._M_str) {}
	const_string& operator=(const const_string& _cstr)
	{
		_M_str=_cstr._M_str;
	return *this;
	}
	~const_string(){}

	const char* str() const
	{return _M_str;}

	bool operator==(const const_string& _cstr)
	{return this==&_cstr;}

	bool operator!=(const const_string& _cstr)
	{return this!=&_cstr;}

	template<class ostream_type>
	friend ostream_type& operator<<(ostream_type& os, const const_string& _cstr)
	{return os << _cstr._M_str;}

	operator const char*() const
	{return _M_str;}
};

template<typename type>
class MutexQueue
{
private:
	std::mutex once_operation;
	std::queue<type> que;

public:
	MutexQueue(): once_operation(), que(){}
	~MutexQueue(){}

	const std::queue<type>& queue() const
	{return que;}

	std::queue<type>& unsafe_queue()
	{return que;}

	typename std::queue<type>::value_type& front()
	{return que.front();}

	const typename std::queue<type>::value_type& front() const
	{return que.front();}

	typename std::queue<type>::value_type& back()
	{return que.back();}

	const typename std::queue<type>::value_type& back() const
	{return que.back();}

	bool empty() const
	{return que.empty();}

	void lock()
	{once_operation.lock();}

	void unlock()
	{once_operation.unlock();}

	typename std::queue<type>::size_type size() const
	{return que.size();}

	void push(const type& str)
	{
		once_operation.lock();
		que.push(str);
		once_operation.unlock();
	}

	void pop()
	{
		once_operation.lock();
		que.pop();
		once_operation.unlock();
	}

	type extract()
	{
		once_operation.lock();
		if(que.empty())
		{
			once_operation.unlock();
			return type();
		}
		type out=que.front();
		que.pop();
		once_operation.unlock();
		return out;
	}
};

class IgnoreTrie : public CompressedTrie<std::nothrow_t>
{
public:
	IgnoreTrie(){}
	~IgnoreTrie(){}

	bool is_ignored(const std::string&) const;
};

bool IgnoreTrie::is_ignored(const std::string& name) const
{
	node* actual_node=root;
	node::son_type::iterator it;
	for(std::string::const_iterator i=name.begin(); i!=name.end(); ++i)
	{
		if(actual_node->son.end()==(it=actual_node->son.find(*i)))
			return false;
		actual_node=it->second;
		if(actual_node->is_pattern) return true;
	}
return false;
}

struct empty {};

class site_class
{
public:
	CompressedTrie<empty>::iterator file;
	site_class(): file(){}
	~site_class(){}
};

using namespace std;

unsigned THREADS=1;
bool show_links_origin=false;

mutex global_lock, loging;

MutexQueue<std::string> download_queue;
// first - file name, second - site adress
MutexQueue<std::pair<std::string, std::string> > parse_queue;
MutexQueue<unsigned> free_threads;

vector<thread> threads;

bool url_char[256]={};
string server, root_dir, download_command, download_command_no_extention;

CompressedTrie<empty> wrong_sites_base, extentions;
CompressedTrie<bool> files_base;
CompressedTrie<CompressedTrie<bool>::iterator> sites_base;
IgnoreTrie ignored_sites;

special_aho<const_string> model_parse;
/* 0 - "href="
*  1 - "src="
*  2 - "url("
*  3 - "HREF="
*  4 - "SRC="
*/
const_string link("");

inline int system(const string& str)
{return system(str.c_str());}

class temporary_directory
{
	char* _M_name;
	temporary_directory(const temporary_directory&): _M_name(){}
	temporary_directory& operator=(const temporary_directory&){return *this;}
public:
	explicit temporary_directory(const char* new_name): _M_name(new char[strlen(new_name)+2])
	{
		unsigned size=strlen(new_name);
		memcpy(_M_name, new_name, size);
		_M_name[size]=_M_name[size+1]='\0';
		if(NULL==mkdtemp(_M_name))
		{
			struct exception : std::exception
			{
				const char* what() const _GLIBCXX_USE_NOEXCEPT {return "Cannot create temporary directory\n";}
			};
			throw exception();
		}
		_M_name[size]='/';
		chmod(_M_name, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	}

	~temporary_directory()
	{
		remove_r(_M_name);
		delete[] _M_name;
	}

	const char* name() const
	{return _M_name;}

	operator const char*() const
	{return _M_name;}
} tmp_dir("sites-downloader.XXXXXX");

void download(int thread_id)
{
	LOGN("trololo");
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
	sites_base.insert(site);
	/////////////
	string & used_download_command = extentions.find(site.substr(site.size() > 3 ? site.size()-4 : 0)) == extentions.end() ? download_command : download_command_no_extention;
	/////////////
	loging.lock();
	cout << "\033[01;34mDownloading: " << site << "\033[00m\n" << flush;
	loging.unlock();
	string tmp_file_name=string(tmp_dir.name())+"download"+myto_string(thread_id);
#ifdef DEBUG
	cerr << used_download_command+to_shell(site)+" 2> "+tmp_file_name << endl;
#endif
	if(0==system(used_download_command+to_shell(site)+" 2> "+tmp_file_name))
	{
		string tmp_file=GetFileContents(tmp_file_name), downloaded_file_name, downloaded_file;
		/*char apostrophe_begin[]={'`', '\0'}, apostrophe_end[]={'\'', '\0'};
		// char apostrophe_begin[]={226, 128, 152}, apostrophe_end[]={226, 128, 153, '\0'};
		for(int i=3, tfs=tmp_file.size(); i<tfs; ++i)
		{
			if(0==tmp_file.compare(i-1, 1, apostrophe_begin))
			{
				--i;
				while(++i+1<tfs && tmp_file.compare(i, 1, apostrophe_end)!=0)
					downloaded_file_name+=tmp_file[i];
				break;
			}
		}*/
		{
			deque<int> kmp_results = kmp(tmp_file, server);
			if(kmp_results.empty())
				goto download_error;
			uint i = kmp_results.back();
			downloaded_file_name = server;
			D(for(uint j = i; j < tmp_file.size(); ++j)
				cout << tmp_file[j];)
			while(++i < tmp_file.size() && url_char[static_cast<unsigned char>(tmp_file[i])])
				downloaded_file_name += tmp_file[i];
			D(cout << "\n\033[01;32mExtracted: " << downloaded_file_name << "\033[00m\n" << flush;)
		}
		if(downloaded_file_name.empty() || !file_exist(downloaded_file_name))
			goto download_error;
		sites_base.lock();
		*sites_base.find(site)=files_base.insert(downloaded_file_name).first;
		sites_base.unlock();
		loging.lock();
		cout << "\033[01;34mSite: " << site << "\n\033[01;32mFile: " << downloaded_file_name << "\033[00m\n" << flush;
		LOG(tmp_file);
		loging.unlock();
		downloaded_file=GetFileContents(downloaded_file_name);
		special_aho<const_string> parse=model_parse;
		parse.find(downloaded_file);
		for(int i=0, dfs=downloaded_file.size(); i<dfs; ++i)
		{
			if(parse[i]>=0)
			{
				int patt_id=parse[i];
				i+=parse.pattern(patt_id).first.size();
				while(i<dfs && (downloaded_file[i]==' ' || downloaded_file[i]=='\t'))
					++i;
				char string_char=downloaded_file[i];
				if(patt_id==2 && string_char!='\'' && string_char!='"')
				{
					string_char=')';
					--i;
				}
				else if(string_char!='\'' && string_char!='"')
					continue;
				LOG(i);
				D(cerr << " ; ");
				LOG(patt_id);
				D(cerr << " ; ");
				LOG(string_char);
				D(cerr << " ");
				LOGN(dfs);
				string new_site, original_new_site;
				while(++i<dfs && downloaded_file[i]!=string_char)
					new_site+=downloaded_file[i];
				original_new_site=new_site;
				LOGN(new_site);
				if(new_site[0]=='?') new_site=site+new_site;
				if(0==new_site.compare(0, 7, "http://") || 0==new_site.compare(0, 8, "https://"))
				{
					eraseHTTPprefix(new_site);
					if(!(0==new_site.compare(0, server.size(), server) || 0==new_site.compare(0, 4+server.size(), "www."+server)))
						continue;
				}
				LOGN(new_site);
				if(0==new_site.compare(0, server.size(), server))
					new_site.erase(0, server.size());
				if(0==new_site.compare(0, 4+server.size(), "www."+server))
					new_site.erase(0, 4+server.size());
				LOGN(new_site);
				if(new_site[0]!='/')
					new_site=without_end_after_slash(downloaded_file_name)+new_site;
				else
					new_site=server+new_site;
				absolute_path(new_site).swap(new_site);
				convert_from_HTML(new_site).swap(new_site);
				D(cerr << "\033[01;34m");
				LOGN(new_site);
				D(cerr << "\033[00m");
				if(0!=original_new_site.compare(0, 11, "javascript:") && is_good_name(new_site) && !ignored_sites.is_ignored(new_site) && sites_base.insert(new_site).second)
				{
					loging.lock();
					cout << "\033[01;33m" << new_site << "\033[00m";
					if(show_links_origin)
						cout << " <- " << downloaded_file_name;
					cout << endl;
					loging.unlock();
					download_queue.push(new_site);
				}
			}
		}
		parse_queue.push(make_pair(site, downloaded_file_name));
	}
	else
	{
	download_error:
		loging.lock();
		cout << "\033[01;31mError: \033[00m" << site << "\033[00m\n" << flush;
	#ifdef LOGS
		cout << GetFileContents(tmp_file_name) << flush;
	#endif
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

void parse()
{
	while(!parse_queue.empty())
	{
		string site, downloaded_file_name, downloaded_file, new_content;
		parse_queue.lock();
		site=parse_queue.front().first;
		downloaded_file_name=parse_queue.front().second;
		parse_queue.unsafe_queue().pop();
		parse_queue.unlock();
		if(downloaded_file_name.empty() || *files_base.find(downloaded_file_name)) continue;
		*files_base.find(downloaded_file_name)=true;
		cout << "\033[01;36mParsing: " << downloaded_file_name << "\033[00m\n" << flush;
		downloaded_file=GetFileContents(downloaded_file_name);
		special_aho<const_string> parse=model_parse;
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
				string new_site, original_new_site;
				while(++i<dfs && downloaded_file[i]!=string_char)
					new_site+=downloaded_file[i];
				original_new_site=new_site;
				if(new_site[0]=='?') new_site=site+new_site;
				D(cerr << endl);
				LOGN(new_site);
				if(0==new_site.compare(0, 7, "http://") || 0==new_site.compare(0, 8, "https://"))
				{
					eraseHTTPprefix(new_site);
					if(!(0==new_site.compare(0, server.size(), server) || 0==new_site.compare(0, 4+server.size(), "www."+server)))
					{
						new_content+=original_new_site+string_char;
						continue;
					}
				}
				LOGN(new_site);
				if(0==new_site.compare(0, server.size(), server))
					new_site.erase(0, server.size());
				if(0==new_site.compare(0, 4+server.size(), "www."+server))
					new_site.erase(0, 4+server.size());
				if(new_site[0]!='/')
					new_site=without_end_after_slash(downloaded_file_name)+new_site;
				else
					new_site=server+new_site;
				LOGN(new_site);
				absolute_path(new_site).swap(new_site);
				convert_from_HTML(new_site).swap(new_site);
				LOGN(new_site);
				CompressedTrie<CompressedTrie<bool>::iterator>::iterator it=sites_base.find(new_site);
				if(it!=sites_base.end() && *it!=files_base.end())
				{
				#ifdef DEBUG
					cout << "\033[01;33m" << new_site << "\033[00m\n";
				#endif
					string site_address=files_base.get_name(*it);
					// convert '?' to '%3f'
					{
						string tmp_str;
						for(string::iterator j=site_address.begin(); j!=site_address.end(); ++j)
						{
							if(*j=='?')
								tmp_str+="%3f";
							else
								tmp_str+=*j;
						}
						site_address.swap(tmp_str);
					}
					// add hash
					bool is_hash=false;
					for(string::iterator j=new_site.begin(); j!=new_site.end(); ++j)
					{
						if(is_hash)
							site_address+=*j;
						else if(*j=='#')
						{
							is_hash=true;
							site_address+=*j;
						}
					}
					new_content+=get_path(downloaded_file_name, site_address);
				}
				else if(0==original_new_site.compare(0, 11, "javascript:")) // we are sure it isn't in sites_base
					new_content+=original_new_site;
				else
				{
					eraseHTTPprefix(original_new_site);
					if(0==original_new_site.compare(0, server.size(), server))
						original_new_site.erase(0, server.size());
					if(0==original_new_site.compare(0, 4+server.size(), "www."+server))
						original_new_site.erase(0, 4+server.size());
					new_content+="http://"+server+(original_new_site[0]=='/' ? "":"/")+original_new_site;
				}
				new_content+=string_char;
			}
			else
				new_content+=downloaded_file[i];
		}
		fstream file(downloaded_file_name.c_str(), ios::out);
		if(file.good())
		{
			file << new_content;
			file.close();
		}
		else cerr << "Error overwriting file: " << downloaded_file_name << endl;
	}
}

void control_exit(int=0)
{
	download_queue.lock();
	free_threads.lock();
	for(unsigned i=0; i<THREADS; ++i)
		if(threads[i].joinable())
			threads[i].detach();
	parse();
	remove_r(tmp_dir);
	exit(1);
}

int main(int argc, char const **argv)
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
		cout << "Usage: sd [options]... site... \nSites have to belong to one server\nOptions:\n    --enable-links-origin   Enables showing links origin (file)\n    --disable-links-origin  Disables showing links origin (file)\n    -i PAGE_URL             Set ignore urls with prefix PAGE_URL\n    -j [N], --jobs[=N]      Allow N jobs at once" << endl;
		exit(1);
	}
	for(int i=1; i<argc; ++i)
	{
		static bool have_wrongs_file=false;
		if(0==memcmp(argv[i], "--enable-links-origin", 21))
			show_links_origin=true;
		else if(0==memcmp(argv[i], "--disable-links-origin", 22))
			show_links_origin=false;
		else if(0==memcmp(argv[i], "-i", 2))
		{
			string site(argv[i]+2);
			if(site.empty())
			{
				if(++i<argc)
					site.assign(argv[i]);
				else
					continue;
			}
			if(0==site.compare(0, 4, "www."))
			site.erase(0, 4);
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
	// setup url_char
	for(unsigned char i = 'A'; i <= 'Z'; ++i)
		url_char[i] = true;
	for(unsigned char i = 'a'; i <= 'z'; ++i)
		url_char[i] = true;
	for(unsigned char i = '0'; i <= '9'; ++i)
		url_char[i] = true;
	url_char[static_cast<unsigned char>('/')] = url_char[static_cast<unsigned char>('_')] = url_char[static_cast<unsigned char>('-')] = url_char[static_cast<unsigned char>('%')] = url_char[static_cast<unsigned char>('?')] = url_char[static_cast<unsigned char>('#')] = url_char[static_cast<unsigned char>('.')] = url_char[static_cast<unsigned char>('[')] = url_char[static_cast<unsigned char>(']')] = url_char[static_cast<unsigned char>('=')] = url_char[static_cast<unsigned char>('<')] = url_char[static_cast<unsigned char>('>')] = url_char[static_cast<unsigned char>('&')] = url_char[static_cast<unsigned char>(':')] = url_char[static_cast<unsigned char>('(')] = url_char[static_cast<unsigned char>(')')] = url_char[static_cast<unsigned char>('~')] = url_char[static_cast<unsigned char>(' ')] = url_char[static_cast<unsigned char>('@')] = true;
	// start work
	cerr << THREADS << endl;
	global_lock.lock();
	threads.resize(THREADS);
	for(unsigned i=1; i<THREADS; ++i)
		free_threads.push(i);
	// Extract server name
	if(download_queue.empty())
	{
		cerr << "You have to define at least one site" << endl;
		exit(1);
	}
	server=download_queue.front();
	eraseHTTPprefix(server);
	unsigned eraser=-1;
	while(++eraser<server.size() && server[eraser]!='/');
	server.erase(eraser);
	if(0==server.compare(0, 4, "www."))
	server.erase(0, 4);
	download_command="wget --trust-server-names --no-check-certificate  --connect-timeout=23 --tries=3 --max-redirect=4 -x -nH --adjust-extension --directory-prefix="+to_shell(server)+" -nc ";
	download_command_no_extention="wget --trust-server-names --no-check-certificate  --connect-timeout=23 --tries=3 --max-redirect=4 -x -nH --directory-prefix="+to_shell(server)+" -nc ";
	LOGN(server);
	system("pwd > "+string(tmp_dir.name())+"/pwd");
	root_dir=GetFileContents(string(tmp_dir.name())+"/pwd");
	root_dir.back()='/';
	LOGN(root_dir);
	// Initialize model_parse
	vector<pair<string, const_string> > parse_patterns{make_pair("href=", link), make_pair("src=", link), make_pair("url(", link), make_pair("HREF=", link), make_pair("SRC=", link)};
	model_parse.set_patterns(parse_patterns);
	// Add no-extention-download extentions
	char const * t[]={".ttf", ".woff", ".otf"};
	for(unsigned i = 0, s = sizeof(t) / sizeof(char const *); i < s; ++i)
		extentions.insert(t[i]);
	// Run downloading
	threads[0]=thread(download, 0);
	global_lock.lock();
	for(unsigned i=0; i<THREADS; ++i)
		if(threads[i].joinable())
			threads[i].join();
	// Run parsing
	parse();
return 0;
}