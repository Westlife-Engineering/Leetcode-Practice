---
name: leetcode-note
description: >-
  根据力扣题目链接抓取原文与官方题解，按题目标签自动在对应中文题型目录下创建初稿 Markdown 笔记。
  Use when the user pastes a leetcode.cn / leetcode.com problem URL, asks to 新建笔记 / 写初稿 /
  按标签建文件, or mentions 官方题解 for a new problem in this LeetCode notes repo.
---

# 力扣笔记初稿（按标签落盘）

本仓库是个人刷题笔记库。用户给出**一道题的力扣链接**时，按本 skill 抓取题目与**官方题解**，在对应题型目录下生成初稿 Markdown。

先读仓库根目录 `AGENTS.md`，再执行本流程。落盘规则与 `AGENTS.md`「标签 → 目录（自动落盘）」一致；其余（STL 详注、不捏造题目、不改已有笔记等）仍遵守 `AGENTS.md`。

## 触发

- 用户贴出 `leetcode.cn/problems/...` 或 `leetcode.com/problems/...` 链接，并希望建笔记 / 写初稿
- 用户说「按标签创建」「参考官方题解写 markdown」等

## 工作流（必须按序）

### 1. 抓取题目（禁止凭记忆编造）

只用用户给的链接抓取，得到至少：

- 题号、中文题名、难度
- **标签**（Tags，如：滑动窗口、哈希表、字符串）
- 题目描述、示例、数据范围（提示）
- 力扣链接（保留用户原 URL，含 `envType` 等查询参数亦可）

**首选方案：curl 打力扣 GraphQL（已验证可用，无需浏览器/鉴权）**。从 URL `.../problems/<slug>/...` 取出 `<slug>`，调用：

```bash
curl -s -X POST 'https://leetcode.cn/graphql' \
  -H 'Content-Type: application/json' \
  --data '{"query":"query questionData($titleSlug: String!){ question(titleSlug:$titleSlug){ questionFrontendId title titleSlug difficulty translatedTitle translatedContent topicTags{ name translatedName } } }","variables":{"titleSlug":"<slug>"},"operationName":"questionData"}' \
  -o _q.json -w 'HTTP %{http_code}\n'
```

- 返回 JSON 里 `data.question` 含：`questionFrontendId`（题号）、`translatedTitle`（中文名）、`difficulty`（`EASY`/`MEDIUM`/`HARD`）、`translatedContent`（**HTML 格式**的题面，含示例与数据范围）、`topicTags[].translatedName`（中文标签）。
- 题面是 HTML，写入笔记前用脚本去标签转纯文本/Markdown（见第 6 节「常用脚本」）。
- 若返回 `errors`（通常是 slug 拼错或字段名变动）：先用第 6 节的 introspection 备用查询确认字段，**不要回退到训练记忆编题面**。
- GraphQL 备用：浏览器/CDP 抽 `description`；WebFetch 对该页通常拿不到（JS 渲染），仅作最后兜底。

### 2. 抓取官方题解（正文主解来源）

找 **官方** 解法（不是热门用户题解，除非官方不可得）。**首选 curl + GraphQL 两步走**（均已验证）：

**第一步：列题解文章，找官方那篇的 slug。**

```bash
curl -s -X POST 'https://leetcode.cn/graphql' \
  -H 'Content-Type: application/json' \
  --data '{"query":"query questionSolutionArticles($questionSlug:String!,$first:Int,$orderBy:SolutionArticleOrderBy){ questionSolutionArticles(questionSlug:$questionSlug first:$first orderBy:$orderBy){ edges{ node{ title slug summary isEditorsPick hitCount } } } }","variables":{"questionSlug":"<slug>","first":20,"orderBy":"DEFAULT"},"operationName":"questionSolutionArticles"}' \
  -o _sol.json -w 'HTTP %{http_code}\n'
```

识别官方文章：

- **官方题解的 `slug` 几乎都以 `by-leetcode-solution` 结尾**（如 `liang-shu-zhi-he-by-leetcode-solution`）——这是最稳的判据。
- 也可参考 `isEditorsPick`、标题含「官方题解」、`summary` 内容，但 slug 后缀最可靠。
- `orderBy` 只接受 `DEFAULT`（其它如 `MOST_POPULAR`/`HOT` 中 `HOT` 也能用，`DEFAULT` 即可，`BEST`/`MOST_RECENT` 会报错）。

**第二步：用官方文章 slug 取正文。**

```bash
curl -s -X POST 'https://leetcode.cn/graphql' \
  -H 'Content-Type: application/json' \
  --data '{"query":"query solutionDetailArticle($slug:String!,$orderBy:SolutionArticleOrderBy!){ solutionArticle(slug:$slug orderBy:$orderBy){ title summary content } }","variables":{"slug":"<官方文章slug>","orderBy":"DEFAULT"},"operationName":"solutionDetailArticle"}' \
  -o _official.json -w 'HTTP %{http_code}\n'
```

- `content` 是 **Markdown 字符串**（不是 HTML，与题面不同），含官方「方法一/方法二」、各语言代码块（找 `cpp [solN-Cpp]` 或 `\`\`\`C++`）、复杂度分析。
- 官方题解里多种语言代码挤在一个 `content` 字段里、且会有截断/粘连，需脚本按代码块标记切出 C++ 段（见第 6 节）。

**已踩过的 GraphQL 坑（别再踩）：**

| 错误 | 原因 | 正确写法 |
|---|---|---|
| `Cannot query field "studyPlanDetail"` | 字段不存在 | 题目清单用页面 HTML 内嵌 JSON，不走 GraphQL（见下） |
| `Cannot query field "questionSolutions"` | 字段名错 | 用 `questionSolutionArticles` |
| `Unknown type "SortingOptionEnum"` | 类型名错 | 用 `SolutionArticleOrderBy` |
| `Variable "orderBy" got invalid value "MOST_POPULAR"` | 枚举值错 | 用 `"DEFAULT"` |
| `Cannot query field "name" on type "UserNode"` | `UserNode` 禁内省、字段不可查 | 不查 `author`，靠 slug 后缀判官方 |
| `__type` / `__schema` 返回 null | 力扣**禁用 introspection** | 别内省，字段名见第 6 节备用表 |

备用：`https://leetcode.cn/articles/<english-slug>/` 阅读文章；题目页「题解」Tab 官方篇。仍找不到才考虑用户高质量题解，并在笔记开头注明出处。

记下：推荐主算法名称、核心思路、官方 C++ 代码（若无 C++ 则用官方伪代码 / 他语言思路改写成可提交的 C++17 `Solution`）。

多解法时：**正文只放一份主解**（与目录主题最对应的那份官方推荐解）；其余放「复盘」。

### 3. 由标签决定目录（自动落盘）

一题多标签时，只选**一个最贴切题型目录**，不重复存放。

**标签 → 目录名**（力扣中文标签映射到本仓库中文文件夹）：

| 力扣标签（含近似） | 目录 |
|---|---|
| 滑动窗口 | `滑动窗口/` |
| 双指针 | `双指针/` |
| 哈希表 | `哈希/` |
| 子串 | `子串/` |
| 数组（且无明显更高阶技巧） | `普通数组/` |
| 矩阵 | `矩阵/` |
| 链表 | `链表/` |
| 二叉树、二叉树遍历等 | `二叉树/` |
| 图、深度优先搜索、广度优先搜索（偏图） | `图论/` |
| 回溯 | `回溯/` |
| 二分查找 | `二分查找/` |
| 栈、单调栈 | `栈/` |
| 堆（优先队列） | `堆/` |
| 贪心 | `贪心/` |
| 动态规划 | `动态规划/` |
| 动态规划且明显多维/区间/状压等 | `多维动态规划/` |
| 位运算、数学等技巧向 | `技巧/` |

**多标签优先级**（从高到低，命中即选）：

1. 滑动窗口 / 双指针 / 二分查找 / 回溯 / 单调栈·栈 / 堆 / 并查集（并入图论或技巧时需判断）
2. 动态规划 / 多维动态规划 / 贪心
3. 链表 / 二叉树 / 图论 / 矩阵
4. 哈希表 → `哈希/`
5. 子串 / 字符串 / 数组 → `子串/` 或 `普通数组/`（字符串题且标签含「子串」用 `子串/`；否则看是否更像哈希/窗口）

示例：标签 = `哈希表、字符串、滑动窗口` → 目录 **`滑动窗口/`**（与本题目录主题一致）。

- 目录不存在：直接 `mkdir` 创建（中文名）。
- **仅当**标签无法映射、或两个同级技巧标签难分时，才向用户确认目录；否则**自动创建，不必再问文件名**。
- 若目标文件已存在：不要覆盖；告知路径，询问是否更新（默认遵守 `AGENTS.md`：不擅自改已有笔记）。

### 4. 文件名

```
<题号>-<题名简写>.md
```

例：`3-无重复字符的最长子串.md`、`49-字母异位词分组.md`。题名用题目中文名，去掉多余空白。

路径：`<目录>/<题号>-<题名简写>.md`。

### 5. 写入初稿 Markdown

文风对齐已有优质笔记（如 `双指针/42-接雨水.md`、`滑动窗口/3-无重复字符的最长子串.md`）：

- 元信息：难度、标签（主标签可加 `**加粗**`）、力扣链接
- 开头一行说明：参考了哪篇官方题解 / 文章；正文主解是什么；其他解法在「复盘」
- **题目描述**：原文结构（示例 + 数据范围），来自抓取
- **思路**：由浅入深（可含会超时的暴力），再讲官方主解；关键观察写清楚
- **代码**：一份 C++17 `Solution`，能直接贴进力扣；**凡 STL 必须详细行内注释**（签名、含义、踩坑、复杂度）
- **复杂度**：时间 / 空间，与官方分析一致并简要说明
- **疑惑 / 卡点**：用 Q&A 形式预填 2–4 个本题易错点 / STL 常见疑问（如 `find` vs `[]`、边界处理、容器选型），开头注明「随做题提问持续更新」。这部分是给主人复习用的，宁可多写也不要空着
- **复盘**：可迁移套路；可选「解法二」放官方其他解，勿在正文并列多份主代码

模板骨架：

```markdown
# <题号> <题名>

- 难度：
- 标签：
- 力扣链接：

> 说明：本题参考……正文主解是 **……**；其他见「复盘」。

## 题目描述
## 思路
## 代码
## 复杂度
## 疑惑 / 卡点
## 复盘
```

至少包含「题目描述 / 思路 / 代码」三块。

### 6. 收尾

向用户报告：

- 创建路径
- 选用的标签 → 目录理由（一句话）
- 主解对应哪篇官方题解
- 清理本次产生的临时文件（`_q.json` / `_sol.json` / `_official.json` / `_hot100.html` / `_parse_*.py` 等），只保留 `_hot100_list.json`（题号清单，长期有用）

## 常用脚本与字段速查（实测可用，直接复用）

> 力扣 GraphQL **禁用 introspection**（`__type`/`__schema` 查询返回 null），字段名必须靠记。下面是已验证可用的字段与脚本，下次直接套。

### A. 题面 HTML → 纯文本（第 1 步产出 `_q.json` 后用）

```python
import json, re, html as html_mod
d = json.load(open('_q.json', encoding='utf-8'))['data']['question']
print('id:', d['questionFrontendId'], '|', d['translatedTitle'], '|', d['difficulty'])
print('tags:', [t['translatedName'] for t in d['topicTags']])
c = d['translatedContent']
# <pre>...</pre> 通常是示例代码块，保留换行；其它标签去干净
c = re.sub(r'<pre[^>]*>', '\n```\n', c)
c = re.sub(r'</pre>', '\n```\n', c)
c = re.sub(r'<li[^>]*>', '\n- ', c)
c = re.sub(r'<p[^>]*>', '\n', c)
c = re.sub(r'<br\s*/?>', '\n', c)
c = re.sub(r'<[^>]+>', '', c)                 # 去剩余标签
c = html_mod.unescape(c)                       # &nbsp; &lt; &sup2; 等转回字符
c = re.sub(r'\n{3,}', '\n\n', c).strip()
print(c)
```

### B. 官方题解 Markdown → 切出 C++ 代码段（第 2 步产出 `_official.json` 后用）

官方 `content` 里多语言代码块挤在一起、且长代码会被截断粘连。下面按标记切 C++ 段：

```python
import json, re
a = json.load(open('_official.json', encoding='utf-8'))['data']['solutionArticle']
c = a['content']
# 官方代码块形如：```cpp [sol1-Cpp] ... ``` 或 ```C++ ... ```
# 找所有 cpp 代码块
blocks = re.findall(r'```(?:cpp|C\+\+)\s*(?:\[[^\]]*\])?\s*(.*?)```', c, re.S)
for i, b in enumerate(blocks):
    print(f'=== C++ block {i} ===')
    print(b.strip()[:800]); print()
# 同时提取「思路及算法」「复杂度分析」等段落（以 ### 或 #### 开头）
for m in re.finditer(r'(#{2,4}\s*[^\n]+)', c):
    print('SECTION:', m.group(1))
```

> ⚠️ 官方长 C++ 代码在 `content` 里**可能被截断**（中间缺一大段）。切出来后**务必人眼核对**是否完整、可编译；不完整时去题目页题解 Tab 取完整版，不要把残缺代码写进笔记。

### C. Hot 100 题号清单（批量推进时用）

学习计划页 `https://leetcode.cn/studyplan/top-100-liked/` 的题目清单**走 GraphQL 拿不到**（`studyPlanDetail` 字段不存在）。改从**页面 HTML 内嵌的 dehydrate JSON** 解析：

```bash
# 1. 下整页 HTML
curl -s 'https://leetcode.cn/studyplan/top-100-liked/' -H 'User-Agent: Mozilla/5.0' -o _hot100.html
```

```python
# 2. 解析 planSubGroups（字符串感知的括号匹配，不能简单 split）
import json
html = open('_hot100.html', encoding='utf-8').read()
i = html.find('planSubGroups')
j = html.find('[', i)
depth = 0; end = None; instr = False; esc = False
for k in range(j, len(html)):
    ch = html[k]
    if instr:
        if esc: esc = False
        elif ch == '\\': esc = True
        elif ch == '"': instr = False
        continue
    if ch == '"': instr = True; continue
    if ch == '[': depth += 1
    elif ch == ']':
        depth -= 1
        if depth == 0: end = k + 1; break
arr = json.loads(html[j:end])
out = []
for g in arr:
    for q in g['questions']:
        out.append({'id': q['questionFrontendId'], 'titleCn': q.get('translatedTitle') or q['title'],
                    'slug': q['titleSlug'], 'diff': q['difficulty'], 'group': g['name'],
                    'tags': [t.get('nameTranslated') or t['name'] for t in q.get('topicTags', [])]})
json.dump(out, open('_hot100_list.json', 'w', encoding='utf-8'), ensure_ascii=False, indent=1)
print('total:', len(out), '| groups:', sorted({q['group'] for q in out}))
```

`_hot100_list.json` 长期保留。每条含 `id / titleCn / slug / diff / group(力扣分组名，与仓库目录对齐) / tags`。算「还差哪些题」：扫描各目录下 `<题号>-*.md`，与清单 `id` 集合做差集即可。

### D. GraphQL 字段速查（introspection 被禁，字段靠记）

| 用途 | Query 字段 | 关键参数 | 返回要点 |
|---|---|---|---|
| 题面 | `question(titleSlug)` | `titleSlug: <slug>` | `questionFrontendId` / `translatedTitle` / `difficulty` / `translatedContent`(HTML) / `topicTags[].translatedName` |
| 题解列表 | `questionSolutionArticles(questionSlug, first, orderBy)` | `orderBy` 只用 `DEFAULT` | `edges[].node.{slug, title, summary, isEditorsPick, hitCount}`；**不查 author** |
| 题解正文 | `solutionArticle(slug, orderBy)` | `orderBy` 只用 `DEFAULT`（必填） | `title` / `summary` / `content`(Markdown) |
| ❌ 学习计划清单 | `studyPlanDetail` | — | **字段不存在**，改用上面 C 节 HTML 解析 |
| ❌ Schema 内省 | `__type` / `__schema` | — | **被禁**，返回 null |

判官方题解最稳判据：文章 `slug` 以 **`by-leetcode-solution`** 结尾。

## 硬性约束

- 不捏造题面；不捏造官方没写过的「官方结论」
- 不重构、不擅自修改已有 `.md`
- 不引入构建工具 / 不 `git init`
- 默认语言 C++；STL 注释宁详勿略
- 正文一份主解，其余进复盘

## 参考示例

已完成的初稿可当作文风样板：

- `滑动窗口/3-无重复字符的最长子串.md`（标签含滑动窗口 → 自动进该目录，主解官方滑动窗口）
- `双指针/42-接雨水.md`（多解法时主解对齐目录，其余进复盘）
- `哈希/49-字母异位词分组.md`（官方排序 + 哈希，STL 详注）
