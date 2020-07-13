# Night

An (interpreted?) programming language. Still in development. I've never made one before, so I don't really know what I'm doing, but it should work out in the end.

The `source.night` file is where you write the code. But I don't like the extention `.night`. I'm also working on a logo, but I'm really bad at designing. I'm also thinking of making a website once it's done.

Also, this version isn't stable at all. This upcoming weekend however, I'll release a stable version :)

---

**Language features**

- variable declaration, initialization, and assignment
- expressions
- print thingy
- if statements

**In progress**

I'm working on refactoring my entire code and fixing a bunch of things. Don't really know what I'm doing. I'm also working on functions, and once I get those done, then in theory, I could use my language, to create my language (well, pre-defined functions at least)...

Oh, also, major update in the upcomming weekend! You'll see if statements fully finished, comparison operator, and better error messages!

---

**Example**

This is what the stable version will (hopefully) look like this weekend :)

```
print "Hello World!";

// this is a comment

bit boolean = true; // supports `! && || == !=` all of those
syb symbol = 'c';
int integer = 2 + 3; // supports '+ - / * % ( )' all of those
dec decimal = 3.14;
str string = "dynamic " + "squid"; // supports string concatenation

if (string == "dynamic squid")
{
    print "hello " + string;
}
else if (string == "octopus") // maybe I'll get this done in the next release
{
    print "no";
}
else
{
    print "hi";
}

// and possible functions??
func swim()
{
    // hopefully :)
}
```
