---
name: learning-walkthrough
description: Guide users through code implementation step-by-step with explanations, maximizing learning. Use when the user wants to understand how to build something, asks for a walkthrough, wants to learn concepts behind code, or explicitly asks not to have code implemented directly but explained first.
---

# Learning Walkthrough

## Purpose

Walk through implementations step-by-step, explaining concepts before showing code. Present code suggestions in chat without directly editing files, allowing users to understand, modify, and implement at their own pace.

## When to Use

- User asks to "walk through" an implementation
- User wants to understand concepts behind code
- User explicitly asks not to have code written directly
- User says "explain step by step" or similar

## Core Principles

1. **Explain before showing**: Present the concept/problem before the solution
2. **One step at a time**: Break complex tasks into digestible chunks
3. **Code in chat, not in files**: Present code as markdown blocks for user to review
4. **Pause for understanding**: Summarize after each major concept
5. **Connect to fundamentals**: Link new concepts to familiar ones

## Walkthrough Structure

For each implementation step:

### 1. State the Goal
What are we trying to achieve?

### 2. Explain the Concept
- What principle or mechanism are we using?
- Why does this approach work?
- What alternatives exist?

### 3. Present the Code
Show code in markdown blocks (do NOT write to files):

```language
// Code suggestion here
```

### 4. Explain Key Lines
- What does this function/syscall do?
- What are the parameters?
- What are common pitfalls?

### 5. Transition
How does this connect to the next step?

## Example Fragment

```markdown
## Step 3: Reading User Input

### Goal
Read a line of input from the user without using `getline()`.

### Concept
At the lowest level, reading input means asking the OS to copy bytes from a file
descriptor into memory. The `read()` system call does exactly this.

Unlike `getline()`, `read()` doesn't know about "lines" - it just reads bytes.
We need to:
1. Read bytes into a buffer
2. Look for the newline ourselves
3. Handle input longer than our buffer

### Code

```c
ssize_t my_getline(char *buf, size_t capacity) {
    ssize_t total = 0;
    char c;
    
    while (total < capacity - 1) {
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n <= 0) break;        // EOF or error
        if (c == '\n') break;     // End of line
        buf[total++] = c;
    }
    buf[total] = '\0';
    return total;
}
```

### Key Points
- `STDIN_FILENO` is file descriptor 0 (standard input)
- `read()` returns bytes read, 0 for EOF, -1 for error
- We read one byte at a time for clarity (not efficient, but clear)
- Always null-terminate the buffer

### Next
Now that we can read input, we need to parse it into command and arguments...
```

## Handling User Interaction

- **Skip ahead**: Summarize skipped steps briefly
- **Clarification**: Dive deeper into that concept
- **Full code**: Present it but encourage reading explanations
- **Ready to implement**: User copies code from chat to files

## Anti-Patterns

- Don't write code directly to files
- Don't dump all code at once without explanation
- Don't assume knowledge - briefly explain "basic" concepts too
- Don't skip error handling explanations
- Don't rush - patience maximizes learning
