# LLM-generated labeling of students submissions

## OLLAMA models under consideration:
1. [gpt-oss:120b](https://ollama.com/library/gpt-oss:120b) -- *thinking*, OpenAI o4-mini
2. [deepseek-r1:70b](https://ollama.com/library/deepseek-r1:70b) -- *thinking*
3. [qwen3-coder:30](https://ollama.com/library/qwen3-coder:30b) -- *coder*

## Prompt
``` lua
You are a strict competitive-programming code judge.

INPUT FIELDS
- problem_name: string
- problem_statement: string (full text)
- submission: string (filename)
- code: string (full source; keep newlines)
- verdict: "OK" or "PT"

OUTPUT (return ONLY this single JSON object; no extra text):
{"problem":"<problem_name>","filename":"<submission>","verdict":"<verdict>","error_lines":[<ints>],"reason":["<one sentence per error line>"]}
- Exactly these keys, no extras, no trailing commas.

RULES
1) If verdict == "OK": error_lines = [] and reason = [].
2) If verdict == "PT":
   - Read problem_statement + code and list the 1-based line numbers in code that are the minimal root-cause of the wrong behavior.
   - Count every line in code (including blank lines) starting at 1.
   - error_lines: integers only; unique; sorted ascending.
   - For each error line, provide a brief, single-sentence explanation in reason at the same index.
   - Prefer the line that introduces the bug (wrong condition/formula/index/type/loop bound/I-O/missing step like sort).
   - If one logical bug spans multiple lines, include each culpable line (and one reason sentence per line).

CONSTRAINTS
- Base judgments ONLY on the provided problem_statement and code. Do not invent constraints or use external knowledge.

INTERNAL REASONING (do NOT output)
1) Extract problem requirements/constraints and sketch a correct high-level solution.
2) Map expected steps to code (I/O, preprocessing, core algo, corner cases).
3) Compare vs. reference; identify contradictions or missing steps; pick root-cause lines (not downstream effects).
4) Validate output: keys match schema; if verdict=OK → [], []; else non-empty ints sorted and reason length matches.

EDGE-CASE HINTS
- Off-by-one / bounds → mark the condition/loop header.
- Wrong formula/comparator/return → mark the exact line(s).
- Missing required step → mark the nearest place it must occur (e.g., before two-pointer).
- I/O/format/parse → mark read/print line(s).
- Overflow/type → mark the declaration or arithmetic using the wrong type.

EXAMPLE
{"problem":"p02718","filename":"s123456789.cpp","verdict":"PT","error_lines":[4,7],"reason":["incorrect loop bound causes off-by-one","division by zero when n==0"]}
```

## Results
[OpenAI](outputs_openai.json), [Qwen](outputs_qwen.json), [DeepSeek](outputs_deepseek.json) (cut)

### [s112877031.cpp](examples/s112877031.cpp)
**Problem**: p02696  

**OpenAI**:
- Line 12: declares A, B, N as double instead of integer types, leading to precision errors and incorrect arithmetic
- Line 18: loops x from 0 to b instead of up to N, so it evaluates the expression over the wrong range

**Qwen**:
```
[warn] could not parse JSON for s112877031.cpp; raw output:
{"problem":"p02696","filename":"s112877031.cpp","verdict":"PT","error_lines":[10,11,12,13,14],"reason":["Loop only checks up to B instead of N, missing larger values that could yield higher result","Variable tmp is computed as x/b but used in floor(a*tmp) which may cause precision issues with large B","The calculation floor(a * tmp) - a * floor(tmp) is incorrect for finding maximum because it doesn't consider the actual expression from problem","Using double for calculations can lead to precision errors when dealing with large numbers like 10^12","The loop bound x <= b is insufficient; it should go up to N to check all possible values"]}

Wait, let me reconsider this more carefully. Looking at the problem again and the code:

1) The problem asks for maximum of floor(A * x / B) - A * floor(x / B) for x from 0 to N
2) The code uses double precision which can cause issues with large numbers
3) The loop only goes up to B, not N
4) The mathematical approach is fundamentally flawed

Let me reanalyze:

{"problem":"p02696","filename":"s112877031.cpp","verdict":"PT","error_lines":[10,11,12,13,14],"reason":["Loop only checks up to B instead of N, missing the actual range of x values to consider","Using floating point arithmetic (double) introduces precision errors for large integers in the calculation","The mathematical expression being computed doesn't match the problem's required formula","The loop bound is incorrect - it should iterate up to N not B","The approach using floating point arithmetic is fundamentally flawed for this integer problem"]}

Actually, let me be more precise about what's wrong:

{"problem":"p02696","filename":"s112877031.cpp","verdict":"PT","error_lines":[10,11,12,13,14],"reason":["Loop only checks up to B instead of N, which is incorrect for the problem's constraint","The use of floating point arithmetic introduces precision errors when dealing with large integers","The mathematical approach computes an incorrect expression compared to what's required by the problem statement","The loop bound should be up to N not B to cover all possible values of x","The algorithmic approach using floating point is fundamentally unsuitable for this integer optimization problem"]}

Let me reconsider once more - I need to focus on the actual root causes:

{"problem":"p02696","filename":"s112877031.cpp","verdict":"PT","error_lines":[10,11,12,13,14],"reason":["Loop only iterates up to B instead of N, missing the correct range of x values","The use of double type introduces floating point precision errors for large integer computations","The mathematical expression computed doesn't correctly implement the problem's formula","The loop bound is wrong - it should go up to N not B to find maximum over all valid x","The entire approach using floating point arithmetic is incorrect for this integer optimization problem"]}

{"problem":"p02696","filename":"s112877031.cpp","verdict":"PT","error_lines":[10,11,12,13,14],"reason":["Loop only checks up to B instead of N, which is incorrect for the problem's constraint","The use of floating point arithmetic introduces precision errors when dealing with large integers","The mathematical approach computes an incorrect expression compared to what's required by the problem statement","The loop bound should be up to N not B to cover all possible values of x","The algorithmic approach using floating point is fundamentally unsuitable for this integer optimization problem"]}
```

**DeepSeek**:
- Line 4: Using floating-point variables for integer inputs causes precision errors.
- Line 9: Looping up to b is computationally infeasible for large values.

### [s180336410.cpp](examples/s180336410.cpp)
**Problem**: p02623  

**OpenAI**:
- Line 28: initializes prefix sum a[0] with A[0] instead of 0, causing off‑by‑one counting of books from stack A
- Line 42: sets j = M, which is outside the valid index range of array b (maximum index is M‑1)
- Line 43: accesses b[j] while j may be equal to M, leading to out‑of‑bounds array access and incorrect loop condition

**Qwen**:
- Line 25: Loop variable j is initialized to M instead of M-1, causing out-of-bounds access
- Line 26: The while loop condition checks b[j] which may be undefined when j becomes negative
- Line 27: The increment of ans is incorrect as it adds i+j+2 instead of the correct count of books read

**DeepSeek**:
- Line 25: out-of-bounds array access when initializing j to M instead of M-1

### [s191672447.cpp](examples/s191672447.cpp)
**Problem**: p02922  

**OpenAI**:
- Line 5: initializes socket count to a instead of the required starting 1 socket
- Line 6: initializes strip count to 1 instead of 0, causing an off‑by‑one error

**Qwen**:
- Line 5: The loop condition checks 'first < b' but the break statement inside the loop checks 'first >= b', creating a logical inconsistency
- Line 6: The increment logic is incorrect because it adds 'a-1' to 'first' but doesn't properly account for the fact that each strip increases the number of sockets by 'a-1' from the previous total

**DeepSeek**:
- Line 4: initial socket count should be 1, not a
- Line 6: loop condition incorrectly starts with first =a instead of 1

### [s295976724.cpp](examples/s295976724.cpp)
**Problem**: p02642  

**OpenAI**:
- Line 8: vector a is sized with uninitialized n before reading input, so the array has incorrect size
- Line 21: the condition a.at(j)%a.at(j)==0 is always true; it should compare a.at(i)%a.at(j)==0 to check divisibility

**Qwen**:
- Line 10: Loop runs from j=i-1 to n, causing out-of-bounds access when i=0
- Line 11: Loop condition j<n allows j to exceed array bounds
- Line 12: Checking divisibility by itself (a.at(j)%a.at(j)==0) is always true and incorrect logic
- Line 13: Break statement inside inner loop prevents proper checking of all elements
- Line 14: Incorrect logic for counting non-divisible elements
- Line 15: Wrong condition for checking duplicate elements

**DeepSeek**:
- Line 13: Incorrect divisibility check; should be a[i] % a[j] == 0 instead of a[j] % a[j] == 0




