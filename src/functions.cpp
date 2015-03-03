#include "functions.hpp"
#include <vector>
#include <fstream>
#include <dirent.h>
#include <cstring>

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
	while(static_cast<unsigned>(i)<min(current.size(),target.size()) && current[i]==target[i])
		++i;
	--i;
	while(i>=0 && current[i]!='/')
		--i;
	for(unsigned j=static_cast<unsigned>(++i); j<current.size(); ++j)
		if(current[j]=='/') ++s;
	while(--s>=0)
		relative+="../";
	for(;static_cast<unsigned>(i)<target.size(); ++i)
		relative+=target[i];
return relative;
}

string absolute_path(const string& path)
{
	string out, act;
	vector<string> st;
	for(unsigned i=0; i<path.size(); ++i)
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
	for(unsigned i=0; i<st.size(); ++i)
		out+=st[i];
return out;
}

void remove_hash(string& str)
{
	unsigned i=-1;
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

bool is_good_name(const string& str)
{
	for(int i=str.size()-1; i>=0; --i)
		if((str[i]=='\'' || str[i]=='"' || str[i]==';') && (i==0 || str[i-1]!='\\'))
			return false;
return true;
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
			case ';': out+="\\;";break;
			default: out+=*i;
		}
	}
return out;
}

void remove_r(const char* path)
{
	DIR* directory;
	dirent* current_file;
	string tmp_dir_path=path;
	if(*tmp_dir_path.rbegin()!='/') tmp_dir_path+='/';
	if((directory=opendir(path)))
		while((current_file=readdir(directory)))
			if(strcmp(current_file->d_name, ".") && strcmp(current_file->d_name, ".."))
				remove_r((tmp_dir_path+current_file->d_name).c_str());
	remove(path);
}

string myto_string(long long int a)
{
	string w;
	while(a>0)
	{
		w=static_cast<char>(a%10+'0')+w;
		a/=10;
	}
	if(w.empty()) w="0";
return w;
}

deque<int> kmp(const string& text, const string& pattern)
{
	deque<int> out;
	int *P=new int[pattern.size()], k=0, pl=pattern.size();
	P[0]=0;
	for(int i=1; i<pl; ++i)
	{
		while(k>0 && pattern[k]!=pattern[i])
			k=P[k-1];
		if(pattern[k]==pattern[i]) ++k;
		P[i]=k;
	}
	k=0;
	for(int tl=text.size(), i=0; i<tl; ++i)
	{
		while(k>0 && pattern[k]!=text[i])
			k=P[k-1];
		if(pattern[k]==text[i]) ++k;
		if(k==pl)
		{
			out.push_back(i);
			k=P[k-1];
		}
	}
	delete[] P;
return out;
}

string GetFileContents(const string& file_name)
{
	FILE* file = fopen(file_name.c_str(), "r");
	if (file == NULL)
		return "";
	// Determine file size
	if (fseek(file, 0, SEEK_END) == -1)
		return "";
	long size = ftell(file);
	if (size == -1L)
		return "";
	try {
		char* content = new char[size];
		rewind(file);
		fread(content, sizeof(char), size, file);
		fclose(file);

		string out(content, content+size);
		delete[] content;
		return out;
	} catch (...) {
		return "";
	}
}

void eraseHTTPprefix(std::string& str)
{
	if(0==str.compare(0, 7, "http://"))
		str.erase(0, 7);
	else if(0==str.compare(0, 8, "https://"))
		str.erase(0, 8);
}

std::string convert_from_HTML(const std::string& str)
{
	string out;
	for(int i=0, slen=str.size(); i<slen; ++i)
	{
		if(0==str.compare(i, 6, "&quot;"))
		{
			i+=5;
			out+='"';
		}
		else if(0==str.compare(i, 5, "&amp;"))
		{
			i+=4;
			out+='&';
		}
		else if(0==str.compare(i, 4, "&lt;"))
		{
			i+=3;
			out+='<';
		}
		else if(0==str.compare(i, 4, "&gt;"))
		{
			i+=3;
			out+='>';
		}
		else
			out+=str[i];
	}
return out;
}

bool is_number(const string& str)
{
	if(str.empty()) return false;
	for(unsigned i=0; i<str.size(); ++i)
		if(!(str[i]>='0' && str[i]<='9')) return false;
return true;
}

int to_int(const string& str)
{
	int out=0;
	for(unsigned i=0; i<str.size(); ++i)
	{
		out*=10;
		out+=str[i]-'0';
	}
return out;
}