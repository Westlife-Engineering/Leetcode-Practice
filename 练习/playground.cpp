// 本地草稿纸：对着左边笔记自己敲，写完 Ctrl+Alt+B 编译并运行。
// 换题时整份覆盖重写即可，不必新建文件或 VS 工程。
//
// 注意：
//   1. MSVC 没有 <bits/stdc++.h>，需要哪个容器就 #include 哪个头文件。
//   2. 贴回力扣时，只复制 class Solution { ... }; 不要带 main 和这些头文件。
//   3. 力扣环境已预置常用头文件；本地必须自己写 #include。

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <stack>
using namespace std;


vector<vector<int>> twoSumAll_hash(const vector<int>& nums, int target)
{
    unordered_map<int, vector<int>> hashtable;
    vector<vector<int>> pairs;

    for(int i=0; i<nums.size();i++)
    {
        auto it = hashtable.find(target - nums[i]);
        if(it != hashtable.end()){
            for(const int& j : it->second) pairs.push_back({j,i});
        }
        hashtable[nums[i]].push_back(i);
    }

    return pairs;
}

vector<int> longestConsecutiveSeq_hash(const vector<int> nums)
{
    unordered_set<int> num_set;
    for(int num:nums) num_set.insert(num);

    vector<int> best;

    for(const int num:num_set)
    {
        if(!num_set.count(num-1)){
            int current_num = num;
            int len = 1;

            while(num_set.count(current_num + 1)){
                current_num++;
                len++;
            }

            if(len > best.size()){
                best.resize(len);
                for(int i=0;i<len;i++) best[i] = num + i;
            }
        }
    }

    return best;
}

int subarraySum_hash(vector<int>& nums, int k)
{
    unordered_map<int, int> hashtable; //key为前缀和，value为前缀和出现次数
    hashtable[0] = 1;

    int cnt; // 和为k的子数组个数
    int pre = 0;

    for(int num:nums)
    {
        pre += num;
        if(hashtable.find(pre - k) != hashtable.end())
            cnt += hashtable[pre - k];
        hashtable[pre]++;
    }

    return cnt;
}

vector<vector<string>> groupAnagrams_hash(vector<string>& strs)
{
    unordered_map<string, vector<string>> hash;

    for(string str:strs)
    {
        string p = str;
        sort(p.begin(), p.end());
        hash[p].emplace_back(str);
    }

    vector<vector<string>> result;
    for(auto it = hash.begin(); it != hash.end(); it++){
        result.emplace_back(it->second);
    }

    return result;
}

bool bracket_match_stack(string strs)
{
    int len = strs.size();
    if(len % 2 != 0) return false;

    stack<char> stk;

    for(char c:strs)
    {
        if(c == '(' || c == '[' || c== '{')
            stk.push(c);
        else{
            if(stk.empty()) return false;

            char ch = stk.top();
            stk.pop();

            if(c == ')') {
                if(ch != '(') return false;
            }
            else if(c == ']'){
                if(ch != '[') return false;
            }
            else{
                if(ch != '{') return false;
            }
        }
    }

    return stk.empty();
}

int largestRectangleArea_stack(vector<int>& height)
{
    int len = height.size();
    vector<int> h(len+2);

    h[0] = 0; h[len+1] = 0;
    for(int i=0;i < len; i++)
        h[i+1] = height[i];

    stack<int> stk; stk.push(0);
    int max_area = 0;

    for(int i=1;i<len+1;i++)
    {
        while(h[i] < h[stk.top()]){
            int index = stk.top();
            stk.pop();
            int width = i - stk.top() - 1;
            max_area = max(max_area, width * h[index]);
        }

        stk.push(i);
    }

    return max_area;
}

vector<int> dailyTemperatures_stack(vector<int> temperatures)
{
    int len = temperatures.size();
    stack<int> stk;
    vector<int> result(len);

    for(int i=0; i<len; i++)
    {
        while(!stk.empty() && temperatures[i] > temperatures[stk.top()]){
            int day = stk.top();
            stk.pop();
            result[day] = i - day;
        }

        stk.push(i);
    }

    while(!stk.empty()){
        result[stk.top()] = 0;
        stk.pop();
    }

    return result;
}

string decodeString_stack(string strs)
{
    stack<int> numstk;
    stack<string> strstk;

    int num = 0;
    string str; //当前层正在拼的字符串

    for(char c:strs)
    {
        if(c-'0' >= 0 && c-'0' <= 9) //是数字
            num = num*10 + c-'0';

        else if(c == '['){
            numstk.push(num);
            num = 0;
            strstk.push(str);
            str.clear();
        }

        else if(c == ']'){
            int repeat = numstk.top();
            numstk.pop();
            
            string prefix = strstk.top();
            strstk.pop();

            while(repeat > 0){
                prefix += str;
                repeat --;
            }

            str = prefix; //拼好的串交回当前层，后面字母/外层]都接着用它
        }

        else str += c;
    }

    return str;
}

bool canJump_greedy(vector<int> nums)
{
    int right = 0;
    for(int i=0;i<nums.size();i++)
    {
        if(i > right) return false;
        else {
            right = max(right, i + nums[i]);
            if(right >= nums.size()-1) return true;
        }
    }

    if(right < nums.size()-1) return false;
    return true;
}

int maxProfit(vector<int>& prices)
{
    int minPrice = 10000;
    int maxProfit = 0;

    for(int i=0;i<prices.size();i++)
    {
        maxProfit = max(maxProfit, prices[i] - minPrice);
        minPrice = min(minPrice, prices[i]);
    }
    return maxProfit;
}

int main() {
    // 用题目示例喂给 s.xxx(...)，再 cout 看结果。
    // 下面是「两数之和」的写法示范，换题后改成当前题：
    //
    // Solution s;
    // vector<int> nums{2, 7, 11, 15};
    // auto ans = s.twoSum(nums, 9);
    // for (int x : ans) cout << x << " ";
    // cout << "\n";

    int n; cin >> n;
    vector<int> nums(n);
    for(int i=0; i < n; i++) cin >> nums[i];

    vector<int> result = longestConsecutiveSeq_hash(nums);

    for(int i=0; i < result.size() - 1 ; i++)
        cout << result[i] << ' ';
    cout << result[result.size() - 1] << endl;
    return 0;
}
