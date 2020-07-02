# Night

A (interpreted?) programming language. Still in development. I've never made one before, so I don't really know what I'm doing, but it should work out in the end.

The `source.night` file is where you write the code. But I don't like the extention `.night`. I'm looking for something shorter, but don't know what yet. I'm also working on a logo, but I'm really bad at designing. I'm thinking of something similar to `logo2.png`.

Also, this language is very similar to the C family since I'm not that creative, and I'm just starting out. Definitly going to change it down the road.

---

**Special Features (updated as progress continues):**

- no implicit conversions (similar to Rust)

---

**Language features**

Right now, this language supports:

```.cpp
print "Hello World!";

// this is a comment

int answer = (5 + 3) * 2; // supports simple expressions
bool correct = (true || false) && true; // supports boolean expressions
char letter = 'c';
str name = "dynamic " + "squid"; // supports string concatenation

int integer = true; // error!

print "The answer is: "; // an auto newline will be added
print answer;
```

And that's it!

**In progress**

Right now, I'm working on if statements, and I got it so that everything inside the if statement is grouped together, so that's good. But that had me rework the entire line extraction thingy. Now all I have to do is the condition.
