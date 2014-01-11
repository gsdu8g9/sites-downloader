#include "trie.hpp"
#include <cstdlib>

#ifdef OLD
#include <vector>
#include <queue>

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
	bool remove_var(const string&);
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
			if(_v[k].t[static_cast<unsigned char>(s[i])]==0)
			{
				if(_free.empty())
				{
					_v[k].t[static_cast<unsigned char>(s[i])]=_v.size();
					k=_v.size();
					_v.push_back(x);
				}
				else
				{
					_v[k].t[static_cast<unsigned char>(s[i])]=_free.front();
					k=_free.front();
					_v[k]=x;
					_free.pop();
				}
			}
			else k=_v[k].t[static_cast<unsigned char>(s[i])];
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
			if(_v[k].t[static_cast<unsigned char>(s[i])]==0) return false;
			else
			{
				k=_v[k].t[static_cast<unsigned char>(s[i])];
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
			_v[k].t[static_cast<unsigned char>(s[sl])]=0;
		}
	return true;
	}

	bool find_var(const string& s, bool file)
	{
		int k=0, sl=s.size();
		for(int i=0; i<sl; ++i)
		{
			if(_v[k].ignore) return true;
			if(_v[k].t[static_cast<unsigned char>(s[i])]==0) return false;
			else k=_v[k].t[static_cast<unsigned char>(s[i])];
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
#endif

using namespace std;

template<typename T>
inline T abs(T x)
{return (x<0 ? -x:x);}

unsigned rd()
{return abs(rand());}

#include <iostream>

struct lol{};

int main()
{
	cout << __cplusplus << endl;
	CompressedTrie<lol> ttt;
	// string k;
	// k+=char(-47);
	// k+=char(1);
	// k+=char(1);
	// k+=char(11);
	// ttt.insert(k);
	ttt.insert("my name is troll");
	cout << (ttt.end()!=ttt.find("my name is troll")) << ": " << ttt.get_name(ttt.find("my name is troll")) << endl;
	// return 0;
	srand(182431774);
#ifdef CPRST
	CompressedTrie<lol> my_trie;
#elif !defined OLD
	Trie<lol> my_trie;
#endif
	for(int i=0; i<20000; ++i)
	{
		string tmp;
		for(int j=rd()%400; j>=0; --j)
			tmp+=static_cast<char>(rd()%256);
		switch(rd()%3)
		{
			case 0:
				#ifdef OLD
					var_base::add_var(tmp);
				#else
					my_trie.insert(tmp);
				#endif
				break;
			case 1:
				#ifdef OLD
					var_base::find_var(tmp);
				#else
					my_trie.find(tmp);
				#endif
				break;
			default:
				#ifdef OLD
					var_base::remove_var(tmp);
				#else
					my_trie.erase(tmp);
				#endif
		}
	}
return 0;
}