#include "functions.hpp"
#include <vector>

using namespace std;

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

string absolute_path(const string& path)
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

bool compare_with_end(const string& str, const string& end)
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