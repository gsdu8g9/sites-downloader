#include <string>

#if __cplusplus >= 201103L
#include <mutex>
#endif

#pragma once

class Trie
{
protected:
#if __cplusplus >= 201103L
	std::mutex once_operation;
#endif
	struct node;
	node* root;
public:
	Trie();
	~Trie();

	bool search(const std::string&) const;

	/*
	* Returns false if all are OK or
	* returns true if element already exists.
	*/
	bool insert(const std::string&);
	void erase(const std::string&);
};


class IgnoreTrie : public Trie
{
public:
	IgnoreTrie(){}
	~IgnoreTrie(){}
	bool is_ignored(const std::string&) const;
};

template<class T>
class _Trie
{
protected:
#if __cplusplus >= 201103L
	std::mutex once_operation;
#endif
	struct node
	{
		bool is_pattern;
		unsigned char key;
		node* son[256];
		T* value;

		node(const unsigned char new_key=0, bool new_is_pattern=false): is_pattern(new_is_pattern), key(new_key)
		{
			for(int i=0; i<256; ++i)
				son[i]=NULL;
		}

		~node()
		{
			if(this->value!=NULL)
				delete this->value;
			for(int i=0; i<256; ++i)
				if(this->son[i]!=NULL) delete this->son[i];
		}
	};
	node* root;
public:
	class iterator : public std::iterator<std::input_iterator_tag, T>
	{
		node* p;
		iterator(node* x): p(x){}
	public:
		iterator(const iterator& _it): p(_it.p){}
		bool operator==(const iterator& _it) {return this->p==_it.p;}
		bool operator!=(const iterator& _it) {return this->p!=_it.p;}
		T& operator*() {return *this->p->value;}
	};

	_Trie(): root(new node){}

	~_Trie()
	{delete this->root;}

#if __cplusplus >= 201103L
	void lock(){this->once_operation.lock();}
	void unlock(){this->once_operation.unlock();}
#endif

	iterator end()
	{return iterator(NULL);}

	iterator search(const std::string&) const;

	/*
	* second is true if all is OK or
	* false if element already exists.
	*/
	std::pair<iterator, bool> insert(const std::string&);
	void erase(const std::string&);
};