#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <queue>
#include <stack>

using namespace std;

namespace aho
{
	namespace tree
	{
		struct node
		{
			int E[256], fail, long_sh_pat, pattern_id; // fail pointer, max shorter patter, pattern id
			bool is_pattern; // is pattern end in this vertex
			unsigned char character; // this node character
			node(unsigned char letter=0): is_pattern(false), character(letter)
			{
				for(int i=0; i<256; ++i)
					E[i]=0;
			}
			~node(){}
		};

		vector<node> graph;

		void init()
		{
			graph.resize(1); // add root
			graph[0].fail=graph[0].long_sh_pat=0; // max shorter pattern isn't exist
		}

		void add_word(const string& word, int id)
		{
			int ver=0; // actual node (vertex)
			for(int s=word.size(), i=0; i<s; ++i)
			{
				if(graph[ver].E[word[i]]!=0) ver=graph[ver].E[word[i]]; // actual view node = next node
				else
				{
					ver=graph[ver].E[word[i]]=graph.size(); // add id of new node
					graph.push_back(node(word[i])); // add new node
				}
			}
			graph[ver].is_pattern=true;
			graph[ver].pattern_id=id;
		}

		void add_fails() // and the longest shorter patterns, based on BFS algorithm
		{
			queue<int> V;
			// add root childrens
			for(int i=0; i<256; ++i)
			{
				if(graph[0].E[i]!=0) // if children exists
				{
					graph[graph[0].E[i]].fail=graph[graph[0].E[i]].long_sh_pat=0;
					V.push(graph[0].E[i]);
				}
			}
			while(!V.empty())
			{
				int actual=V.front(); // id of actual view node
				for(int i=0; i<256; ++i) // i is character of view node
				{
					if(graph[actual].E[i]!=0) // if children exists
					{
						actual=graph[actual].fail; // we have view node parent's fial edge
						while(actual>0 && graph[actual].E[i]==0) // while we don't have node with children of actual character (i)
							actual=graph[actual].fail;
						actual=graph[graph[V.front()].E[i]].fail=graph[actual].E[i]; // the longest sufix, if 0 then longest sufix = root
						// add the longest shorter pattern
						if(graph[actual].is_pattern) // if the fail node is pattern then is long_sh_pat
							graph[graph[V.front()].E[i]].long_sh_pat=actual;
						else // long_sh_pat is the fail node's long_sh_pat
							graph[graph[V.front()].E[i]].long_sh_pat=graph[actual].long_sh_pat;
						actual=V.front();
						V.push(graph[actual].E[i]); // add this children to queue
					}
				}
				V.pop(); // remove visited node
			}
		}
	}

	vector<int> fin; // finding patterns

	void find(const string& text)
	{
		if(text.size()==0) return;
		fin.resize(text.size());
		int act=0, pat; // actual node - root
		for(int s=text.size(), i=0; i<s; ++i)
		{
			while(act>0 && tree::graph[act].E[static_cast<unsigned char>(text[i])]==0)
				act=tree::graph[act].fail; // while we can't add text[i] to path, go to fail node
			if(tree::graph[act].E[static_cast<unsigned char>(text[i])]!=0) // if we can add text[i] to path
				act=tree::graph[act].E[static_cast<unsigned char>(text[i])];
			if(tree::graph[act].is_pattern) // if actual node is pattern, then add it to fin
				fin[i]=tree::graph[act].pattern_id;
			else
			{
				pat=tree::graph[act].long_sh_pat; // go to the pattern node
				fin[i]=tree::graph[pat].pattern_id;
			}
			if(fin[i]==4 && fin[i-1]==5) fin[i-1]=0;
			/*while(pat>0) // finding patterns
			{
				fin[tree::graph[pat].pattern_id].push_back(i); // add pat node to fin
				pat=tree::graph[pat].long_sh_pat; // go to the next pattern
			}*/
		}
	}

	void find(const vector<string>& patterns, const string& text)
	{
		vector<tree::node>().swap(tree::graph); // clear tree::graph
		//vector<vector<int> >(patterns.size()).swap(fin); // clear fin and set number of patterns
		tree::init(); // initialize tree
		for(int i=patterns.size()-1; i>=0; --i) // add patterns to tree
			tree::add_word(patterns[i], i);
		tree::add_fails(); // add fails edges
		find(text);
	}
}

namespace var_base
{
	struct ver
	{
		int t[256];
		bool is, ignore, file;
		ver();
		~ver(){}
	};
	extern vector<ver> _v;
	extern queue<int> _free;
	extern bool is_there;
	void add_var(const string&, char=0);
	bool remove_var(string);
	bool find_var(const string&, bool=false);
	string memory_dump(int=0);
}

namespace var_base
{
	ver::ver(): is(false), ignore(false), file(false)
	{
		for(short int i=0; i<256; ++i)
			t[i]=0;
	}
	vector<ver> _v(1);
	queue<int> _free;
	bool is_there;

	void add_var(const string& s, char type)
	{
		ver x;
		int k=0, sl=s.size();
		for(int i=0; i<sl; ++i)
		{
			if(_v[k].t[static_cast<int>(s[i])]==0)
			{
				if(_free.empty())
				{
					_v[k].t[static_cast<int>(s[i])]=_v.size();
					k=_v.size();
					_v.push_back(x);
				}
				else
				{
					_v[k].t[static_cast<int>(s[i])]=_free.front();
					k=_free.front();
					_v[k]=x;
					_free.pop();
				}
			}
			else k=_v[k].t[static_cast<int>(s[i])];
		}
		if(type==1) _v[k].ignore=true;
		else if(type==2) _v[k].file=true;
		else _v[k].is=true;
	}

	bool remove_var(const string& s)
	{
		int k=0, sl=s.size();
		stack<int> grt;
		grt.push(0);
		for(int i=0; i<sl; ++i)
		{
			if(_v[k].t[static_cast<int>(s[i])]==0) return false;
			else
			{
				k=_v[k].t[static_cast<int>(s[i])];
				grt.push(k);
			}
		}
		if(!_v[k].is) return false;
		_v[k].is=false;
		while(grt.size()>1)
		{
			--sl;
			for(short int i=0; i<256; ++i)
				if(_v[k].t[i]!=0) return true;
			grt.pop();
			_free.push(k);
			k=grt.top();
			_v[k].t[static_cast<int>(s[sl])]=0;
		}
	return true;
	}

	bool find_var(const string& s, bool file)
	{
		int k=0, sl=s.size();
		for(int i=0; i<sl; ++i)
		{
			if(_v[k].ignore) return true;
			if(_v[k].t[static_cast<int>(s[i])]==0) return false;
			else k=_v[k].t[static_cast<int>(s[i])];
		}
		if(!_v[k].ignore)
		{
			if(file) return _v[k].file;
			else return _v[k].is;
		}
	return true;
	}
	string memory_dump(int x)
	{
		string out;
		out+=_v[x].is ? "1":"0";
		out+=_v[x].ignore ? "1":"0";
		out+=_v[x].file ? "1":"0";
		for(int i=0; i<256; ++i)
		{
			if(_v[x].t[i]!=0)
			{
				out+=static_cast<unsigned char>(i);
				out+=memory_dump(_v[x].t[i]);
			}
		}
	return out+=";";
	}
	void make_from_memoty_dump(const string& str)
	{
		stack<int> V;
		V.push(0);
		_v[0].is=str[0]=='1';
		_v[0].ignore=str[1]=='1';
		_v[0].file=str[2]=='1';
		int i=3;
		char c;
		while(i<str.size())
		{
			c=str[i];
			if(c==';') V.pop();
			else
			{
				_v[V.top()].t[c]=_v.size();
				V.push(_v.size());
				_v.push_back(ver());
				++i;
				c=str[i];
				_v[V.top()].is=(c=='1');
				++i;
				c=str[i];
				_v[V.top()].ignore=(c=='1');
				++i;
				c=str[i];
				_v[V.top()].file=(c=='1');
			}
			++i;
		}
	}
}

bool is_slash(const string& str)
{
	for(string::const_iterator i=str.begin(); i!=str.end(); ++i)
		if(*i=='/') return true;
return false;
}

string get_path(const string& current, const string& target)
{
	string relative;
	int i=0, s=0;
	while(i<min(current.size(),target.size()) && current[i]==target[i])
		++i;
	--i;
	while(i>=0 && current[i]!='/')
		--i;
	for(int j=++i; j<current.size(); ++j)
		if(current[j]=='/') ++s;
	while(--s>=0)
		relative+="../";
	for(;i<target.size(); ++i)
		relative+=target[i];
return relative;
}

string absolute_path(string path)
{
	string out, act;
	vector<string> st;
	for(int i=0; i<path.size(); ++i)
	{
		act+=path[i];
		//cout << i << ": " << act << " " << endl;
		if(path[i]=='/')
		{
			if(act=="../") st.pop_back();
			else if(act!="./") st.push_back(act);
			act="";
		}
	}
	if(act!=".") st.push_back(act);
	for(int i=0; i<st.size(); ++i)
		out+=st[i];
return out;
}

void remove_hash(string& str)
{
	int i=-1;
	while(++i<str.size() && str[i]!='#');
	if(i<str.size() && str[i]=='#') str.erase(i);
}

string without_end_after_slash(const string& str)
{
	int i=str.size();
	while(--i>=0 && str[i]!='/');
	if(i>=0 && str[i]=='/') return string(str, 0, i+1);
return str;
}

bool is_wrong_name(const string& str)
{
	for(int i=str.size()-1; i>=0; --i)
		if(str[i]=='%' || str[i]==';') return true;
return false;
}

bool is_end(const string& str, const string& end)
{
	if(end.size()>str.size()) return false;
	for(int si=str.size()-1, ei=end.size()-1; ei>=0; --si,--ei)
		if(str[si]!=end[ei]) return false;
return true;
}

string to_shell(const string& str)
{
	string out;
	for(string::const_iterator i=str.begin(); i!=str.end(); ++i)
	{
		switch(*i)
		{
			case ' ': out+="\\ ";break;
			case '(': out+="\\(";break;
			case ')': out+="\\)";break;
			case '&': out+="\\&";break;
			case '>': out+="\\>";break;
			case '<': out+="\\<";break;
			case '*': out+="\\*";break;
			default: out+=*i;
		}
	}
return out;
}

int main(int argc, char **argv)
{
	/*string a, b;
	cin >> a >> b;
	cout << get_path(a,b) << endl;*/
	if(argc<2)
	{
		cout << "Usage: ... <page>\n";
		abort();
	}
	string site, adress, server, directory, site_prefix;
	queue<string> sites;
	bool continue_file=false;
	for(int i=1; i<argc; ++i)
	{
		if(argv[i][0]=='-' && argv[i][1]=='i')
		{
			string _s;
			for(int j=2; argv[i][j]!='\0'; ++j)
				_s+=argv[i][j];
			var_base::add_var(_s, 1);
			if(_s.size()>3 && _s[0]=='w' && _s[1]=='w' && _s[2]=='w' && _s[3]=='.') var_base::add_var(string(_s, 4), 1);
			else if(_s.size()>3) var_base::add_var("www."+_s, 1);
		}
		else if(argv[i][0]=='-' && argv[i][1]=='w')
		{
			if(continue_file)
			{
				cout << "You can use only one \"wrongs\" file!\n";
				abort();
			}
			continue_file=true;
			string _s;
			for(int j=2; argv[i][j]!='\0'; ++j)
				_s+=argv[i][j];
			fstream wr(_s.c_str(), ios::in);
			if(wr.good())
			{
				string kit;
				getline(wr, kit);
				var_base::make_from_memoty_dump(kit);
				getline(wr,server);
				while(wr.good())
				{
					getline(wr,kit);
					if(!kit.empty()/* && !var_base::find_var(kit)*/)
						{sites.push(kit);cout << "\t" << kit << endl;}
				}
			}
			else
			{
				cout << "error opening file: " << _s << "!" << endl;
				abort();
			}
			wr.close();
		}
		else site=argv[i];
	}
	fstream file;
	int ___empty;
	if(!site.empty())
	{
		if(string(site,0,min(7,static_cast<int>(site.size())))=="http://")
		{
			site.erase(0,7);
			site_prefix="http://";
		}
		else if(string(site,0,min(8,static_cast<int>(site.size())))=="https://")
		{
			site.erase(0,8);
			site_prefix="https://";
		}
		cout << site << " " << site_prefix << endl;
		for(int i=0; i<site.size() && site[i]!='/'; ++i)
			server+=site[i];

		___empty=system("pwd > lol.shell");
		file.open("lol.shell", ios_base::in);
		if(file.good())
		{
			getline(file,directory);
			file.close();
		}
		var_base::add_var(site);
		sites.push(site);
	}
	cout << "server: " << server << endl;
	//var_base::add_var(server);
	vector<string> patterns(1);
	patterns.push_back(server); // 1
	patterns.push_back("href="); // 2
	patterns.push_back("src="); // 3
	patterns.push_back("url(\""); // 4
	patterns.push_back("url("); // 5
	aho::find(patterns, "");

	vector<string> wrongs_sites(1, server);
	while(!sites.empty())
	{
		string actual_site=sites.front();
		while(actual_site.size()>0 && *--actual_site.end()=='#')
			actual_site.erase(actual_site.size()-1,1);
		cout << "\033[01;34msite: " << actual_site << "\033[00m" << endl;
		cout << "wget --no-check-certificate --connect-timeout=17 --tries=3 --no-use-server-timestamps --unlink -x -nv "+site_prefix+to_shell(actual_site)+" && ls -t -p -1 "+to_shell(without_end_after_slash(actual_site))+" > lol.shell" << endl;
		___empty=system(("rm -f lol.shell && wget --no-check-certificate --connect-timeout=23 --tries=3 --no-use-server-timestamps --unlink -x -nv "+site_prefix+to_shell(actual_site)+" && ls -t -p -1 "+to_shell(without_end_after_slash(actual_site))+" > lol.shell").c_str());
		file.open("lol.shell", ios_base::in);
		if(file.good())
		{
			string k="/", input, actual_dir;
			// get file path
			while(file.good() && *--k.end()=='/')
				getline(file,k);
			if(!is_slash(k)) k=(*--actual_site.end()=='/' ? actual_site:(actual_site==server ? server+"/":without_end_after_slash(actual_site)))+k;
			file.close();
			// add next address to base
			if(var_base::find_var(k,true))
			{
				sites.pop();
				continue;
			}
			var_base::add_var(k,2);
			var_base::add_var(k);
			cout << "\033[01;32mfile: " << k << "\033[00m" << endl;
			// parse file
			file.open(k.c_str(), ios_base::in);
			fstream out((k+"~").c_str(), ios_base::out);
			if(file.good() && out.good() && ___empty==0)
			{
				actual_dir=k;
				while(!actual_dir.empty() && *--actual_dir.end()!='/') actual_dir.erase(actual_dir.size()-1,1);
				if(is_end(k,".js") || is_end(k,".png") || is_end(k,".jpg") || is_end(k,".gif") || is_end(k,".ico"))
					while(file.good())
					{
						getline(file,input);
						/*aho::find(input);
						for(int i=0, il=input.size(); i<il; ++i)
						{
							//cout << aho::fin[i];
							out.put(input[i]);
							if(aho::fin[i]>1)
							{
								int oldfin=aho::fin[i];
								string g=server, primary;
								++i;
								while(input[i]==' ' && i<il) ++i;
								if(i<il && input[i]=='"')
								{
									out.put('"');
									++i;
								}
								if(oldfin==4 && i<il && (input[i]=='/' || input[i]=='.')) oldfin=3;
								if(i<il && input[i]!='/') g+='/';
								if(oldfin==5)
								{
									if(i<il && (input[i]=='/' || input[i]=='.'))
										oldfin=-1;
									while(i<il && input[i]!=')')
									{
										if(aho::fin[i]==1) g=server;
										else g+=input[i];
										primary+=input[i];
										++i;
									}
								}
								else
								{
									while(i<il && input[i]!='"')
									{
										if(aho::fin[i]==1) g=server;
										else g+=input[i];
										primary+=input[i];
										++i;
									}
								}
								if((oldfin==4 || oldfin==5)) cout << "\033[01;31m" << (g=actual_dir+primary) << "\033[00m" << endl;
								g=absolute_path(g);
								if(string(g,0,5+server.size())!=server+"/http" && !var_base::find_var(g))
								{
									cout << "\033[01;33m" << g << "\033[00m\n";
									sites.push(g);
									var_base::add_var(g);
								}
								if((oldfin==4 || oldfin==5))
								{
									out << primary << (oldfin==4 ? '"':')');
									continue;
								}
								//cout << actual_dir << " " << g << " " << get_path(actual_dir,g) << endl;
								out << get_path(actual_dir,g)+(*--g.end()=='/' ? "index.html":"");
								if(oldfin!=-1) out.put('"');
								else out.put(')');
							}
						}
						//out << input;
						//cout << endl;
						if(file.good()) out << endl;*/
						out << input;
						if(file.good()) out << endl;
					}
				else
					while(file.good())
					{
						getline(file,input);
						aho::find(input);
						for(int i=0, il=input.size(); i<il; ++i)
						{
							//cout << aho::fin[i];
							out.put(input[i]);
							if(aho::fin[i]>1)
							{
								bool is_server=false;
								int oldfin=aho::fin[i];
								string g, primary;
								++i;
								while(input[i]==' ' && i<il) ++i;
								if(i<il && input[i]=='"')
								{
									out.put('"');
									++i;
								}
								if(oldfin==4 && i<il && (input[i]=='/' || input[i]=='.')) oldfin=3;
								//if(i<il && input[i]!='/') g+='/';
								if(oldfin==5)
								{
									if(i<il && (input[i]=='/' || input[i]=='.'))
										oldfin=-1;
									while(i<il && input[i]!=')')
									{
										if(aho::fin[i]==1)
										{
											g=server;
											is_server=true;
										}
										else g+=input[i];
										primary+=input[i];
										++i;
									}
								}
								else
								{
									while(i<il && input[i]!='"')
									{
										if(aho::fin[i]==1)
										{
											g=server;
											is_server=true;
										}
										else g+=input[i];
										primary+=input[i];
										++i;
									}
								}
								bool is_this_server=true;
								// cout << "0 : " << is_server << " " << g << " " << primary << endl;
								if((oldfin==4 || oldfin==5)) g=actual_dir+primary;//cout << "\033[01;31m" << (g=actual_dir+primary) << "\033[00m" << endl;
								else if(!is_server)
								{
									if(g.size()>3 && string(g, 0, 4)=="http") is_this_server=false;
									else if(g[0]=='/') g=server+g;
									else g=actual_dir+g;
								}
								// cout << "1 : " << g << endl;
								g=absolute_path(g);
								// cout << "2 : " << g << endl;
								remove_hash(g);
								// cout << "3 : " << g << endl;
								//while(*--g.end()=='/') g.erase(g.size()-1,1);
								if(is_this_server && !is_wrong_name(g) && !var_base::find_var(g)/* && (*--g.end()!='/' || !var_base::find_var(string(g, 0, g.size()-1)))*/)
								{
									bool can_add=true;
									if(*--g.end()=='/' && var_base::find_var(string(g, 0, g.size()-1))) can_add=false;
									else if(var_base::find_var(g+"/")) can_add=false;
									if(can_add)
									{
										cout << "\033[01;33m" << g << "\033[00m\n";
										sites.push(g);
										var_base::add_var(g);
									}
								}
								if((oldfin==4 || oldfin==5))
								{
									out << primary << (oldfin==4 ? '"':')');
									continue;
								}
								// cout << actual_dir << " " << g << " " << get_path(actual_dir,g) << endl;
								string _path=get_path(actual_dir,g);
								if(is_this_server && !is_slash(_path)) _path="./"+_path;
								out << _path+(*--g.end()=='/' ? "index.html":"");
								if(oldfin!=-1) out.put('"');
								else out.put(')');
							}
						}
						//out << input;
						//cout << endl;
						if(file.good()) out << endl;
					}
				out.close();
				file.close();
			}
			else
			{
				cout << "\033[01;31mWarning!\033[00m" << endl;
				wrongs_sites.push_back(actual_site);
				if(out.good())
				{
					out.close();
					___empty=system(("rm -f "+to_shell(k)+"~").c_str());
				}
				if(file.good())
				{
					file.close();
					___empty=system(("rm -f "+to_shell(k)).c_str());
				}
			}
			// move to right file
			___empty=system(("mv -f "+to_shell(k)+"~ "+to_shell(k)).c_str());
		}
		else
		{
			cout << "\033[01;31mError!\033[00m" << endl;
			wrongs_sites.push_back(actual_site);
		}
		sites.pop();
	}
	___empty=system("rm -f lol.shell");
	/*while(!sites.empty())
	{
		cout << actual_site << endl;
		sites.pop();
	}*/
	fstream wrongs("Wrongs.log", ios_base::out);
	wrongs << var_base::memory_dump() << endl;
	cout << var_base::memory_dump() << endl;
	if(wrongs_sites.size()>1)
	{
		cout << "Wrong sites are in file: Wrongs.log\n";
		for(vector<string>::iterator i=wrongs_sites.begin(); i!=wrongs_sites.end(); ++i)
		{
			wrongs << *i;
			if(i+1!=wrongs_sites.end()) wrongs << endl;
			cout << *i << endl;
		}
	}
	else ___empty=system("rm -f Wrongs.log");
	wrongs.close();
	cout.flush();
	//cout << endl << directory << endl << site << endl << server << endl;
return 0;
}