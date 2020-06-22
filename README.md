# Night

A (interpreted?) programming language. Still in development. Don't really know what I'm doing, but it should work out in the end.

Right now, all it supports is one line variable initialization like this:

```.cpp
int answer = 10;

or 

char letter = 'c';

or

bool lightSwitch = true;
```

The syntax is very similar to C++'s just because I want to focus on actually making a *working* language for now.

I'm working on expressions:

```.cpp
int answer = 5 * 4 + 3;

// variable storage:
(type)     (name)     (value)
int        answer     23
```
