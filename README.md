# Expression Parser

This is the expression parser for your coding challenge! It's coded to the spec, additionally
it allows multicharacter variables and negative numbers. It uses an older version of Sooty,
my compiler front-end, because, well, it works. New Sooty(tm) has teething issues. You can
execute an expression on the command-line if you want. Use the `a` flag:

```
sooty -ra "4 + 5 * dragon - (12 * knight)"
```

Otherwise the usage is as you wanted.

# Notes

It uses Boost.ProgramOptions to parse the command line. This means it has to link to the
library. I've included the Multithreaded Debug library as part of the repository, and also
the headers for Boost (minus a few of the big ones, Phoenix, Wave, etc). If you want to build
it for Release, you'll have to build Boost `1_50_0` and place the libraries in
`/dependencies/boost/lib`.

# Complexity

Old Sooty is a recursive descent parser, but it's not a predictive parser. Predictive parsers
run in linear time by using a lookahead. A Recursive Descent parser can take up to exponential
time. I've dodged that bullet for such a small grammar by using Sooty's parsing stacks to avoid
backtracking almost all the time. The downside is that specific ill-formed inputs crash it
spectacularly. Like so: `4 + 5 +`

Lexing is linear, we only double-check dashes to determine negative numbers from subtraction.
Parsing is, as mentioned, close to linear as possible for a Recursive Descent parser. I
actually think that for such a small grammar as this it is wholly linear. Those stacks.

If you have any problems getting it to run or whatever, contact me! :D

`- Jonathan`

## Edited 31/07/2012 : 18:30

No idea if you've seen or cloned the repository yet - hopefully this ninja edit gets in before
you do - I fixed a bunch of bugs. Most notably operators are now correctly left-associative, and
not right-associative like they were. This means `4 - 3 - 2` correctly parses to
`- (- 4 3) 2` instead of `- 4 (- 3 2)`. Some of the reducing has also been corrected to take
into account non-commutative operators (`-` and `/` - although I still leave division alone
for the most part).