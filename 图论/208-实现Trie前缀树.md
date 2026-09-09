# 208 实现 Trie (前缀树)

- 难度：中等
- 标签：**设计**、**字典树**、哈希表、字符串
- 力扣链接：https://leetcode.cn/problems/implement-trie-prefix-tree/?envType=study-plan-v2&envId=top-100-liked

> 说明：本题参考[力扣官方题解](https://leetcode.cn/problems/implement-trie-prefix-tree/solutions/717046/shi-xian-trie-qian-zhui-shu-by-leetcode-ti500/)「实现 Trie (前缀树)」（slug：`shi-xian-trie-qian-zhui-shu-by-leetcode-ti500`）。正文主解是 **「Trie 节点 + 子节点数组」的 26 叉树实现**（官方方法一）：每个节点开 26 个子指针（对应 a–z）加一个 `isEnd` 标记，`insert`/`search`/`startsWith` 都从根开始逐字符往下走。时间 O(L)，空间 O(总字符数×26)。这是 Trie 的标准实现写法。

## 题目描述

**Trie**（发音类似 "try"）或者说 **前缀树** 是一种树形数据结构，用于高效地存储和检索字符串数据集中的键。这一数据结构有相当多的应用情景，例如自动补全和拼写检查。

请你实现 Trie 类：

- `Trie()` 初始化前缀树对象。
- `void insert(String word)` 向前缀树中插入字符串 `word`。
- `boolean search(String word)` 如果字符串 `word` 在前缀树中，返回 `true`（即，在检索之前已经插入）；否则，返回 `false`。
- `boolean startsWith(String prefix)` 如果之前已经插入的字符串 `word` 的前缀之一为 `prefix`，返回 `true`；否则，返回 `false`。

**示例：**

```
输入
["Trie", "insert", "search", "search", "startsWith", "insert", "search"]
[[], ["apple"], ["apple"], ["app"], ["app"], ["app"], ["app"]]
输出
[null, null, true, false, true, null, true]

解释
Trie trie = new Trie();
trie.insert("apple");   // 向字典树中插入 "apple"
trie.search("apple");   // 返回 True
trie.search("app");     // 返回 False
trie.startsWith("app"); // 返回 True
trie.insert("app");     // 向字典树中插入 "app"
trie.search("app");     // 返回 True
```

**提示：**

- `1 <= word.length, prefix.length <= 2000`
- `word` 和 `prefix` 仅由小写英文字母组成
- `insert`、`search` 和 `startsWith` 调用次数总计不超过 `3 * 10^4` 次

> 读题要点：要实现的是一个 **`Trie` 类**（不是 `Solution` 类），有 4 个接口：构造函数 `Trie()`、`void insert(word)`、`bool search(word)`、`bool startsWith(prefix)`。两个关键约束让实现变简单：(1) 所有字符**只含小写字母** `a–z`，正好 26 个，所以每个节点开 26 个子指针就够用；(2) `search` 要的是「完整单词是否插入过」，而 `startsWith` 要的是「是否有单词以这个前缀开头」——这两个语义不一样，是本题的核心区分点（见思路）。

## 思路

> 本题主要参考力扣官方题解「方法一：字典树」。

### Trie 是什么：把公共前缀「共享存储」的树

Trie（又叫**前缀树**、**字典树**）本质是一棵**把字符串按字符逐层展开**的多叉树：根节点不存字符，从根往下的每条路径代表一个字符串。

为什么要这样组织？想象你要存 `"apple"`、`"app"`、`"apply"` 三个单词。它们**共享前缀 `"appl"`**。如果用数组/哈希表各存一份，`"appl"` 这 4 个字符会被重复存 3 遍。Trie 的做法是让它们**共用一条 `a→p→p→l` 的路径**，只在分叉处分家：

```
            (root)
              |
              a
              |
              p
              |
              p   ← "app" 在这里结束（这个节点的 isEnd = true）
             / \
            l   (其它分支)
            |
            e   ← "apple" 在这里结束（isEnd = true）
            |
            ...   ← "apply" 继续往下走 y...
```

好处有二：

1. **公共前缀只存一份，省空间**（尤其适合大量有共同前缀的单词，比如英文词典）。
2. **查询是「前缀匹配」的利器**：要判断「某个前缀是否存在」，只要从根开始顺着字符一条路走到底，O(L) 搞定，不用像哈希表那样遍历所有 key 去比开头。

### 节点结构：26 个子指针 + isEnd 标记

每个节点长这样：

```cpp
struct / class TrieNode {
    Trie* children[26];   // 26 个子指针，children[i] 指向下一个字符；为 nullptr 表示这条分支不存在
    bool isEnd;           // 标记「是否有单词在这个节点结束」
};
```

几个要点：

- **为什么是 26 个子指针？** 因为题目保证字符只来自 `a–z`。用 `ch - 'a'` 把字符映射成 `0–25` 的下标：`'a'→0`、`'b'→1`、…、`'z'→25`。所以 `children[ch-'a']` 就是「当前字符对应的那个子节点」。这种「固定字符集 → 数组下标」的映射，省去了哈希表查桶的开销。
- **`isEnd` 是干嘛的？** 它标记「**有没有一个完整单词正好结束在这个节点**」。注意：一个节点可以有子节点（说明后面还能继续走），同时 `isEnd=true`（说明有一个单词正好在这里结束）。比如上面例子里 `"app"` 结束的那个 `p` 节点，它**既是 `app` 的结尾，又是 `apple` 路径上的中间节点**——`isEnd` 只是「打了个标记」，不影响它继续有孩子。

官方代码用 `vector<Trie*> children(26)`（开 26 个指针的动态数组），等价于裸数组 `Trie* children[26]`，好处是可以用初始化列表 `children(26)` 一行把 26 个指针都置 `nullptr`（`vector<Trie*>` 默认初始化元素为 `nullptr`），省去手写循环。

### 为什么 search 要看 isEnd，startsWith 不要？

这是本题**最容易混**的地方，也是面试常考点。

- **`search(word)` 问的是「word 这个完整单词有没有被插入过」**。光顺着 `word` 走到底、路径存在还不够——比如插入了 `"apple"` 后 `search("app")`，路径 `a→p→p` 是存在的（因为 `"apple"` 经过它），但我们**从来没把 `"app"` 当作一个完整单词插进去**，所以答案应该是 `false`。怎么区分？**看走到底那个节点的 `isEnd`**：`true` 表示「确实有一个单词在这里收尾」，才算 `search` 命中。
- **`startsWith(prefix)` 问的是「有没有任何单词以 prefix 开头」**。只要顺着 `prefix` 能一路走完、中间没有断路（没有遇到 `nullptr`），就说明至少有一个插入的单词以它开头（具体是哪个我们不关心），返回 `true`。**最后那个节点的 `isEnd` 无所谓**——即使它不是任何单词的结尾，只要它下面还连着别的节点，就说明有单词以 prefix 开头。

一句话：**`search` = 走到底 + `isEnd` 为真；`startsWith` = 走到底不踩空就行**。

### insert / search / startsWith 各自怎么逐字符走

三者的「走法」高度相似，都是从根开始、对每个字符 `ch` 算 `idx = ch - 'a'`，然后看 `children[idx]`：

- **`insert(word)`**（创建为主）：`node` 从根开始，对每个字符 `ch`：
  - `children[ch-'a']` 为空 → `new Trie()` 建个新节点挂上去；
  - 不为空 → 直接复用；
  - 然后 `node = node->children[ch-'a']` 往下走一层。
  - 走完最后一个字符，把当前 `node->isEnd = true`。

- **`search(word)` / `startsWith(prefix)`**（只读为主）：官方抽了个公共辅助 `searchPrefix`：`node` 从根开始，对每个字符 `ch`：
  - `children[ch-'a']` 为空 → 这条前缀根本不存在，立刻返回 `nullptr`；
  - 不为空 → `node = node->children[ch-'a']` 往下走。
  - 走完全部字符，返回最后的 `node`（可能对应一个真实节点，也可能之前中途返回了 `nullptr`）。
  - 然后 `search` 在外面判 `node != nullptr && node->isEnd`，`startsWith` 只判 `node != nullptr`。

可以看到三者共用「逐字符往下走」的骨架，差别只在「碰到空怎么办」（insert 建、查询返空）和「走到底看什么」（search 看 isEnd、startsWith 不看）。官方把这个骨架抽成 `searchPrefix`，`search` 和 `startsWith` 各加一行判断，非常干净。

### 手动推导：先插 "apple" 再插 "app"

`isEnd=true` 的节点用 `✓` 标记，下面用「根→a→p→p→l→e」的链式表示（省略其它空分支）：

```
初始：root（空树，root.isEnd=false）
       所有 children[0..25] 都是 nullptr

insert("apple")：node=root
  ch='a'(idx=0)  children[0]=nullptr  → new 节点 A，挂到 children[0]，node=A
  ch='p'(idx=15) A->children[15]=nullptr → new 节点 P1，node=P1
  ch='p'(idx=15) P1->children[15]=nullptr → new 节点 P2，node=P2
  ch='l'(idx=11) P2->children[11]=nullptr → new 节点 L，node=L
  ch='e'(idx=4)  L->children[4]=nullptr  → new 节点 E，node=E
  循环结束 → E->isEnd=true ✓
  树：root → a → p → p → l → e✓   （一条直链）

search("apple")：沿 a→p→p→l→e 走，每步 children 都在，最后到 E
  → node=E != nullptr 且 E->isEnd=true → 返回 true ✓

search("app")：沿 a→p→p 走，每步都在，最后到 P2
  → node=P2 != nullptr 但 P2->isEnd=false（还没插 "app"）
  → 返回 false ✓   ★这就是 search 必须看 isEnd 的原因★

startsWith("app")：同样走到 P2，node != nullptr
  → 不管 isEnd，直接返回 true ✓

insert("app")：沿 a→p→p 走，节点 A/P1/P2 都已存在，直接复用（不 new）
  循环结束（"app" 只有 3 个字符）→ P2->isEnd=true ✓
  树：root → a → p → p✓ → l → e✓
                 └─ P2 现在既是 "app" 的结尾，又是通往 "apple" 的中间节点

search("app")：走到 P2，P2->isEnd 现在=true → 返回 true ✓
```

注意第二次 `insert("app")` 时，`a→p→p` 这段路径**没有新建任何节点**（全部复用 `"apple"` 已经建好的），只是在 `P2` 上多打了一个 `isEnd=true` 的标记。这就是 Trie「公共前缀共享存储」的直观体现：插 `"app"` 几乎是零成本（只改一个 bool）。

## 代码

```cpp
// ============ ① Trie 节点结构（本题的「类即节点」写法）============
// 官方实现把「Trie 树」和「Trie 节点」合并成一个 class：每个 Trie 对象本身就是一个节点。
//   - 自己存自己的 children 和 isEnd；
//   - 根节点就是 new Trie() 出来的那个对象；
//   - 子节点是 children[i] 指向的另一个 Trie 对象。
// 这种「节点即树」的写法省去了单独定义 TrieNode 结构体，是 Trie 题最常见的简洁写法。
class Trie {
private:
    // children：子节点指针数组，长度 26，对应 a–z 这 26 个小写字母。
    //   - vector<Trie*>：元素类型是 Trie*（指向 Trie 对象的指针）。
    //   - children[i] 指向「下一个字符是 ('a'+i) 」的那个子节点；
    //     为 nullptr 表示这条分支还不存在（没有单词往这个方向走）。
    //   - 用 vector 而不是裸数组 Trie* children[26]：vector<Trie*> 默认把 26 个元素初始化为 nullptr，
    //     构造函数里 children(26) 一行就把所有子指针清零，省去手写 for 循环置空。
    //   - 代价：每个节点固定占 26 个指针的空间（26×8=208 字节），不管用没用到。这是「数组版 Trie」
    //     换 O(1) 下标寻址的代价；字符集很大时（如 Unicode）应改用 unordered_map<char, Trie*>。
    vector<Trie*> children;

    // isEnd：标记「是否有完整单词正好在这个节点结束」。
    //   - insert 走到最后一个字符时把它置 true；
    //   - search 要靠它区分「路径存在」和「单词存在」；startsWith 不看它。
    bool isEnd;

    // ============ ④ 查询前缀的公共骨架（insert 不用它，search/startsWith 用）============
    // searchPrefix：从根开始逐字符往下走 prefix，返回走完后的节点指针；
    //               若中途某字符对应的子节点不存在，立刻返回 nullptr。
    //   - 参数 prefix：要查找的前缀（或完整单词，对它来说都是「一串字符」）。
    //   - 返回值：Trie* —— 走到底的节点指针；中途断路则 nullptr。
    //   - 抽出这个辅助函数是因为 search 和 startsWith 的「走法」完全一样，
    //     只是最后判断条件不同，复用避免重复代码。
    Trie* searchPrefix(string prefix) {
        // this 指向当前 Trie 对象（即根节点）。把它赋给游标 node，用 node 往下走。
        //   - 注意：node 是 Trie*（指针），用 -> 访问成员；改 node 只会动游标，不动树本身。
        Trie* node = this;
        // 范围 for：逐个字符遍历 prefix，ch 是 char 类型。
        for (char ch : prefix) {
            ch -= 'a';   // ★字符转下标★：'a'→0, 'b'→1, ..., 'z'→25。
                         //   - char 本质是小整数，'a' 的 ASCII 是 97，ch-=97 后 ch 变成 0..25 的下标。
                         //   - 前提：题目保证 ch 是小写字母；若有大写/其它字符这一步就错了。
            // node->children[ch]：取当前节点第 ch 号子指针。
            //   - == nullptr：这条分支不存在 → prefix 这个前缀在树里走不通，直接返空。
            if (node->children[ch] == nullptr) {
                return nullptr;
            }
            node = node->children[ch];   // 沿着子指针往下走一层
        }
        return node;   // 走完 prefix 所有字符，返回最后停在那个节点（可能是某个单词的结尾，也可能只是中间节点）
    }

public:
    // ============ 构造函数：初始化一个空节点（也用来初始化整棵树的根）============
    // 成员初始化列表 children(26), isEnd(false)：
    //   - children(26)：构造一个长度 26 的 vector，元素默认 nullptr（vector<Trie*> 的默认值就是空指针）。
    //   - isEnd(false)：新节点没有任何单词结束，标记置 false。
    //   - 成员初始化列表在进入函数体前就赋好值，比在函数体里赋值更高效。
    Trie() : children(26), isEnd(false) {}

    // ============ ② insert：往树里插入一个单词，O(L) ============
    void insert(string word) {
        Trie* node = this;            // 游标从根开始
        for (char ch : word) {
            ch -= 'a';                // 字符 → 0..25 下标（同上）
            // 与查询的区别：子节点不存在时「创建」而不是「返回失败」。
            if (node->children[ch] == nullptr) {
                // new Trie()：在堆上动态构造一个 Trie 对象，返回它的指针。
                //   - new 出来的对象不会自动释放（不像栈对象），本题不 delete 也不会错（力扣跑完即回收），
                //     但工程里是内存泄漏，见「疑惑/卡点」Q4。
                node->children[ch] = new Trie();
            }
            node = node->children[ch];   // 无论新建还是复用，都往下走一层
        }
        // 走完 word 的所有字符，当前 node 就是 word 最后一个字符对应的节点。
        // 打上 isEnd=true：标记「有一个完整单词在这里结束」。
        //   - 如果这个单词之前插过，这里只是把 true 再设成 true，无副作用（幂等）。
        node->isEnd = true;
    }

    // ============ ③ search：查完整单词是否存在，O(L) ============
    bool search(string word) {
        // 复用 searchPrefix 走完 word 的所有字符。
        Trie* node = this->searchPrefix(word);
        // 两个条件必须同时满足才算「单词存在」：
        //   - node != nullptr：路径没断（每个字符都能往下走）；
        //   - node->isEnd：最后那个节点确实是某个单词的结尾。
        // 缺一不可：只有路径存在但 isEnd=false，说明只是某个更长单词的前缀，本单词没插过（如插 "apple" 后 search "app"）。
        return node != nullptr && node->isEnd;
    }

    // ============ ④ startsWith：查是否有单词以 prefix 开头，O(L) ============
    bool startsWith(string prefix) {
        // 只要 searchPrefix 没有中途断路（返回非空），就说明至少有一个插入的单词以 prefix 开头。
        //   - 注意：这里【不看 isEnd】。prefix 对应的节点不一定是任何单词的结尾，
        //     只要它在树里存在，就说明有更长的单词经过了它，即「有单词以 prefix 开头」。
        return this->searchPrefix(prefix) != nullptr;
    }
};
```

## 复杂度

- **时间**：`Trie()` 初始化 O(1)；`insert` / `search` / `startsWith` 都是 **O(L)**，其中 L 是本次操作的字符串长度（`word.length` 或 `prefix.length`）。
  - 三者都是「从根开始，对每个字符做一次 O(1) 的下标寻址 + 指针移动」，所以总步数 = 字符数 L。没有循环嵌套，也没有哈希表扩容。
  - 这比把单词存进 `vector<string>` 后线性查找（O(n·L)）或排序后二分（O(L·log n)）都快，尤其当单词很多时优势明显。
- **空间**：**O(|T|·Σ)**，其中 |T| 是**所有插入字符串的长度之和**（即总共插进去的字符数），Σ 是字符集大小（本题 Σ=26）。
  - 最坏情况下所有单词没有任何公共前缀，每个字符都对应一个独立的新节点，节点总数 = |T|；每个节点固定开 26 个指针，所以是 |T|×26。
  - 实际中公共前缀越多越省（如本题插 `"apple"` 再插 `"app"` 几乎不增空间），所以「数组版 Trie」在英文词典这种高前缀重合场景下空间效率很好。

## 疑惑 / 卡点

> 这块记录做题时和 AI 的问答，方便以后回顾。随做题提问持续更新。

### Q1：为什么 search 要看 isEnd，startsWith 不要？

这是本题的**核心区分点**。

- `search(word)` 问的是「`word` 作为一个**完整单词**有没有被插过」。光顺着 `word` 的字符走完、路径不断还不够——比如插了 `"apple"` 后 `search("app")`，`a→p→p` 这条路确实存在（`"apple"` 经过它），但我们从没把 `"app"` 当作完整单词插进去，所以 `search("app")` 应该是 `false`。区分的办法就是看走到底那个节点的 `isEnd`：`true` 才表示「确实有个单词在此收尾」。
- `startsWith(prefix)` 问的是「**有没有任何单词**以 `prefix` 开头」。只要 `prefix` 这条路能走通（中途不踩 `nullptr`），就说明树里至少有一个更长的单词经过它，答案就是 `true`。最后那个节点是不是某个单词的结尾**无所谓**——比如插了 `"apple"` 后 `startsWith("app")` 是 `true`，虽然 `"app"` 自己没被插过，但 `"apple"` 以它开头。

所以代码里：`search` 是 `node != nullptr && node->isEnd`，`startsWith` 只是 `node != nullptr`。

### Q2：children 为什么用数组（`vector<Trie*>` 或 `Trie*[26]`）不用 `map`/`unordered_map`？

两者都能做 Trie 的子节点容器，各有取舍：

| 方案 | 寻址方式 | 单次查询 | 空间（每节点） | 适用场景 |
|---|---|---|---|---|
| 数组 `Trie* children[26]` / `vector<Trie*>` | `children[ch-'a']` 直接下标 | **O(1)**，常数极小 | 固定 26 个指针（208 字节），用不用都占 | 字符集**小且固定**（如 a–z、数字） |
| 哈希表 `unordered_map<char, Trie*>` | `children[ch]` 哈希查桶 | O(1) 均摊，常数大（哈希函数 + 可能冲突） | 只存实际存在的子节点，**省空间** | 字符集**大或稀疏**（如 Unicode、任意字符） |

本题字符集只有 26 个小写字母，**小且固定**，用数组最快（下标寻址比哈希快得多），代价是每节点固定占 26 个指针。官方就是这么写的。如果题目改成「字符可以是任意 ASCII」或「Unicode」，数组就开不下（256 或更多），这时候应该换成 `unordered_map<char, Trie*>`，用空间换灵活性。

### Q3：`isEnd` 到底是什么，为什么需要它？

`isEnd` 是一个 bool 标记，回答「**有没有一个完整单词正好在这个节点结束**」。它存在的根本原因是：**Trie 的节点不是「字符」而是「字符之间的位置」**——根节点对应「还没读任何字符」，根的 `a` 子节点对应「读了 `'a'`」，依此类推。一个单词 `word` 在 Trie 里对应「从根开始走完 `word` 所有字符后停下来的那个节点」，所以「`word` 被插过」就等价于「那个停下来的节点被打了 `isEnd=true`」。

为什么不直接「看这个节点有没有子节点」来判断单词结束？因为一个节点**可以同时是单词结尾、又有子节点**——比如插了 `"app"` 和 `"apple"`，中间那个 `p` 节点既是 `"app"` 的结尾（`isEnd=true`），又是通往 `"apple"` 的必经之路（有子节点 `l`）。所以「有没有子节点」和「是不是单词结尾」是两件独立的事，必须用单独的 `isEnd` 标记来区分。

### Q4：`new Trie()` 建出来的节点不 delete，会不会内存泄漏？

**会**，力扣环境看不出来（程序跑完整个进程结束，操作系统回收所有内存），但工程上是真泄漏。

本题代码里 `insert` 会 `new Trie()` 建节点，却没有任何地方 `delete`。力扣评测机一次性跑完所有调用就退出，所以能 AC；但在真实程序里，如果一个 Trie 对象生命周期很长、又频繁增删节点，不释放就会越占越多。

正规做法是给 Trie 加个析构函数，递归释放所有子节点：

```cpp
~Trie() {
    for (Trie* child : children) {
        if (child) delete child;   // 递归 delete，每个子节点析构时又会 delete 它自己的孩子
    }
}
```

这样当根节点被 `delete`（或离开作用域自动析构）时，整棵树自上而下递归释放，不会泄漏。本题是设计题主考 Trie 结构，力扣不测内存，所以官方代码省略了析构函数；自己写工程代码时记得补上。

## 复盘

### 可迁移套路（最重要）

- **Trie 是「前缀匹配 / 字符串集合检索」的专用数据结构**。一旦题目出现这些特征，就该条件反射地想到 Trie：
  - **前缀匹配**：判断「有没有单词以某串开头」（本题 `startsWith`）、自动补全（输入 `appl` 列出 `apple`/`apply`/`application`...）。
  - **字符串集合高效查找**：大量字符串里查某个串在不在，且字符串有公共前缀（比哈希表省空间、比排序数组查询快）。
  - **词频统计 / 字典序遍历**：Trie 节点上挂个 `count` 就能统计每个单词出现次数；DFS 整棵 Trie 就能按字典序输出所有单词。
  - **拼写检查、IP 路由（最长前缀匹配）、敏感词过滤、DFS 里的剪枝（如 212 单词搜索 II）**——都是 Trie 的经典应用场景。

- **「逐字符往下走」的骨架可复用**。`insert`/`search`/`startsWith` 三者共用「`node = root` → 对每个字符 `ch`：算 `idx = ch-'a'` → 看 `children[idx]` → 往下走」这套循环。变化只在两处：(1) 碰到空怎么办（insert 建、查询返空）；(2) 走到底看什么（search 看 isEnd、startsWith 不看）。把这套骨架背熟，遇到 211 添加与搜索单词、212 单词搜索 II、648 单词替换、676 实现一个魔法字典等 Trie 进阶题，只要在节点上「加字段、改判断」就能扩展。

- **「数组版 vs 哈希版」的选型**：字符集小而固定（≤几十）用数组，寻址最快；字符集大或稀疏用 `unordered_map`，省空间。这是写 Trie 时第一个要做的决定。

### 关键记忆点

1. **节点 = `children[26]` + `isEnd`**；根节点不存字符，从根往下的路径代表字符串。
2. **`ch - 'a'` 把字符转 0–25 下标**——前提是字符集就是 a–z。
3. **`search` 看 isEnd，`startsWith` 不看**——本题最核心的区分。
4. **公共前缀共享存储**——插 `"app"` 再插 `"apple"` 几乎零成本，这是 Trie 省空间的根本原因。
5. **`vector<Trie*> children(26)` 默认全 nullptr**，省掉手写置空循环；工程代码记得补析构函数递归 `delete`。
