# Night

A (interpreted?) programming language. Still in development. I've never made one before, so I don't really know what I'm doing, but it should work out in the end.

The `source.night` file is where you write the code. But I don't like the extention `.night`. I'm looking for something shorter, but don't know what yet. I'm also working on a logo, but I'm really bad at designing. I'm thinking of something similar to `logo2.png`.

Also, this language is very similar to the C family since I'm not that creative, and I'm just starting out. Definitly going to change it down the road.

---

**Special Features (updated as progress continues):**

- no implicit conversions (similar to Rust)
- and that's it :)

---

**Language features**

Right now, this language supports:

```.cpp
int answer = 5 + 3;
bool correct = true;
char letter = 'c';

int integer = true; // error! also I haven't implemented comments yet
```

And umm... that's it :)

I'm working on getting my math parser to link with the rest of my code so `answer` will be evalutated to `8`. I will also have a bool parser that will evaluate boolean expressions like `(true || false) && true`.

Hopefully in a couple days, I'll have my first working language done (albeit not that good or versitile)! It'll have comments, and maybe a `print` thingy as well.

**Edit:**

Okay, so I did code and now the math parser works great! But the bool parser kinda works. It was harder than I thought. It works for simple things line `true && true`, but the problem is when you have brackets and use NOT like this: `!(true || !false)`. Umm... but it's good enought for now.

So next up, the `print` thingy and comments and fixing some minor bugs. Hopefully I can get that done by Sunday.
