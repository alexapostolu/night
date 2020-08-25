# Night

An interpreted programming language. It combines the simplicity of Python, with the type concepts of the C family.

It's still in early development, and I have a lot left to do. I'm also working on a [website](https://night-web.dynamicsquid.repl.co/) which you can definitly check out! There's even a place to code in there! I'm also working on a logo, you can check that out in the `logo.png` and `logo3.png` files.

Here's a sample of my current language:

```night
print("Hello World!\n");

// this is a comment

bit boolean = true; // supports '! || && == != < > <= >= ( )' all of those
syb character = 'c'; // doesn't support any expressions yet
int integer = 10; // supports '+ - * / % ( )' all of those
dec float = 3.14; // same with 'int', but without 'mod'
str string = "squid"; // suports string concatenation with 'syb', 'int', 'dec', and 'str'

int answer = 10 + 5;
answer += 2 + 3;

int legs = 10;
bit smart = true;

if (legs == 10 && smart)
{
    print("Hi squid\n");
}
else if (legs == 8 && smart)
{
    print("Hi octopus\n");
}
else if (legs == 2 && !smart)
{
    print("Hi human\n");
}
else
{
    print("Not sure who you are\n");
}

int add(int a, int b)
{
    print("Adding to numbers:\n");
    return a + b;
}

int number = add(2, 3) + 4;
print(number + "\n");
```

## How to Use

1. Open cmd, bash, or powershell
2. Type:

```bash
%PathToNightFolder%/src/Night.exe %filepath%.night
```

e.g.

```bash
/mnt/c/tools/Night/src/Night.exe /mnt/c/codeproject/test.night
```

### Making Usage Easier (Windows Only)

Type 'path' to search bar and this will show up:
!['Edit the system envirovment variable' Button](https://github.com/AntimatterReactor/Night/tree/master/instructionimage/1.png "Title")

Click the best match, and this will show up:
!['System Properties' Window](https://github.com/AntimatterReactor/Night/tree/master/instructionimage/2.png "Title")

Click 'Envirovement Variables':
![Button with 'Envirovement Variables' as the Label](https://github.com/AntimatterReactor/Night/tree/master/instructionimage/3.png "Title")

This will show up:
!['Envirovement Variables' Window](https://github.com/AntimatterReactor/Night/tree/master/instructionimage/4.png "Title")

Search and select 'Path' and click 'Edit':
!['Path' as System Variable and 'Edit...' Button](https://github.com/AntimatterReactor/Night/tree/master/instructionimage/5.png "Title")

Select and go to the end of 'Variable Value' and type ";%NightFilePath%/src":
!['Path Variable' Window](https://github.com/AntimatterReactor/Night/tree/master/instructionimage/6.png "Title")
!['D:\VS+VSCodeField\VSCode\C-variant\Night\src'](https://github.com/AntimatterReactor/Night/tree/master/instructionimage/7.png "Title")

## Future Plans

There's still a lot to be done though. Here are the things I'm working on for `v2-beta` (in order):

- `v2.2-beta` loops
- `v2.3-beta` more predefined functions and small libraries
  :  right now, the only predefined function is `print()`
- `v2.4-beta` user input
- `v2.5-beta` arrays
- `v2.6-beta` "include" thingy like in C++
- `v2.7-beta` function definitions and overloading

### Progress on Updates

I just got done with `v2.1-beta`, which is a release about more support for expressions. Now, I'm working on `v2.2-beta`, which are loops!

I'm also planning to have `v3` be the first non-beta version, and I'm really excited about that.
