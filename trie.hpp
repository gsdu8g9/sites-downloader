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
