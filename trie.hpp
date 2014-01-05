#include <string>
#include <mutex>

#pragma once

class Trie
{
protected:
	std::mutex once_operation;
	struct node;
	node* root;
public:
	Trie();
	~Trie();

	bool search(const std::string&) const;
	void insert(const std::string&);
	void erase(const std::string&);
};


class IgnoreTrie : public Trie
{
public:
	IgnoreTrie(){}
	~IgnoreTrie(){}
	bool is_ignore(const std::string&) const;
};
