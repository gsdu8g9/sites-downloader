#include <string>
#include <vector>
#include <cstring>

#pragma once

// aho.cpp
class aho
{
	class class_trie
	{
	public:
		struct node
		{
			int E[256], fail, long_sh_pat, pattern_id; // fail pointer, max shorter pattern, pattern id
			bool is_pattern; // is pattern end in this vertex
			unsigned char character; // this node character
			node(unsigned char letter=0): fail(), long_sh_pat(), pattern_id(), is_pattern(false), character(letter)
			{
				for(int i=0; i<256; ++i)
					E[i]=0;
			}
			~node(){}
		};

		std::vector<node> graph;

		class_trie(): graph(1) // add root
		{
			graph.front().fail=graph.front().long_sh_pat=0; // max shorter pattern isn't exist
		}

		void swap(class_trie& _t)
		{
			graph.swap(_t.graph);
		}

		int add_word(const std::string& word, int id);
		void add_fails(); // and the longest shorter patterns, based on BFS algorithm
	} trie;

	std::vector<std::vector<unsigned>* > fin; // finding patterns

public:
	aho(): trie(), fin(){}
	~aho(){}
	std::vector<std::vector<unsigned>* >::size_type size()
	{return fin.size();}

	std::vector<unsigned>& operator[](std::vector<std::vector<unsigned>* >::size_type n)
	{return *fin[n];}

	const std::vector<unsigned>& operator[](std::vector<std::vector<unsigned>* >::size_type n) const
	{return *fin[n];}

	void swap(aho& _a)
	{
		trie.swap(_a.trie);
		fin.swap(_a.fin);
	}

	void find(const std::vector<std::string>& patterns, const std::string& text);
};

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

class special_aho
{
	class class_trie
	{
	public:
		struct node
		{
			int E[256], fail, long_sh_pat, pattern_id; // fail pointer, max shorter pattern, pattern id
			bool is_pattern; // is pattern end in this vertex
			// unsigned char color; // highlight color
			unsigned char character; // this node character
			node(unsigned char letter=0): fail(), long_sh_pat(), pattern_id(), is_pattern(false), character(letter)
			{
				for(int i=0; i<256; ++i)
					E[i]=0;
			}
			~node(){}
		};

		std::vector<node> graph;

		class_trie(): graph(1) // add root
		{
			graph.front().fail=graph.front().long_sh_pat=0; // max shorter pattern isn't exist
		}

		void swap(class_trie& _t)
		{
			graph.swap(_t.graph);
		}

		int add_word(const std::string& word, int id);
		void add_fails(); // and the longest shorter patterns, based on BFS algorithm
	} trie;

	std::vector<std::pair<std::string, const_string> > patterns;
	std::vector<int> fin; // finding patterns

public:
	special_aho(): trie(), patterns(), fin(){}
	~special_aho(){}

	std::vector<int>::size_type size()
	{return fin.size();}

	int& operator[](std::vector<int>::size_type n)
	{return fin[n];}

	const int& operator[](std::vector<int>::size_type n) const
	{return fin[n];}

	void swap(special_aho& _a)
	{
		trie.swap(_a.trie);
		fin.swap(_a.fin);
	}

	const std::pair<std::string, const_string>& pattern(std::vector<std::pair<std::string, const_string> >::size_type n) const
	{return patterns[n];}

	void set_patterns(const std::vector<std::pair<std::string, const_string> >& new_patterns);

	void find(const std::string& text);
};