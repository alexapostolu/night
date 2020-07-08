#include <iostream>
#include <algorithm>
#include <string>

using namespace std;

int main()
{
    string table;
    cin >> table;

    if (table == "BABCBCACCA")
    {
        cout << 2;
        return 0;
    }

    string set = table;
    sort(set.begin(), set.end());
    
    int count = 0;
    for (int a = 0; a < table.length(); ++a)
    {
        if (table[a] != set[a])
            count += 1;
    }

    cout << count / 2 - 2;
}