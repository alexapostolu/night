# Night

A (interpreted?) programming language. Still in development. I've never made one before, so I don't really know what I'm doing, but it should work out in the end.

The `source.night` file is where you write the code. But I don't like the extention `.night`. I'm looking for something shorter, but don't know what yet. I'm also working on a logo, but I'm really bad at designing. I'm thinking of something similar to `logo2.png`.

Also, this language is very similar to the C family since I'm not that creative, and I'm just starting out. Definitly going to change it down the road.

---

**Special Features (updated as progress continues):**

- no implicit conversions (similar to Rust)

---

**Language features**

You can check the `source.night` file for what the language currently supports. It's really basic stuff. Variable initialization, a `print` thingy, and simple expressions.

Edit: I just added a new feature, if statements!

**In progress**

A complete rework of the language and code. Also I want to add functions. And if I do, then in theory, I could use my language, to create my language (well, pre-defined functions at least)...

---

**How it works**

1. Take in a line of input

2. Separate line based of semicolons. Merge the second line with the first if it has to

That basically allows this:

```cpp
int a = 2 +

3;
```

To be perfectly valid code.

3. Send it off to the lexer to be tokenized

The lexer will ignore whitespace, expect for keywords:

```cpp
int a=2+3;
```

Notice how there must be a space after the `int` keyword.

4. Send it off to the parser

5. Evalutate the expression down to a single token

```cpp
if (true & true)
if (true)

int a = 2 + 3;
int a = 5;

print "Hello " + "World!";
print "Hello World!";
```

6. Do stuff idk
