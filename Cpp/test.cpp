#include <cstdlib>
#include <string>
#include <iostream>
#include <vector>
using namespace std;

string strs[4];
string* GetStrs() { return strs; }

int main()
{
	GetStrs()[0] = "123";
	GetStrs()[1] = "345";
	cout << GetStrs()[1] << endl;

	vector<vector<int>[5]> vec;
	vec.push_back((vector<int>[5])(new vector<int>[5]));
	vec[0][0].push_back(1);
	count << count[0][0] <<endl;
	return 1;
}
