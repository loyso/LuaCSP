LuaCSP
======

Communicating Sequential Processes in Lua: an extension.

LuaCSP library is a framework that allows you to create pseudo-concurrent
programming language embeddable in your application.

[Read the comprehensive documentation (an article) here](http://htmlpreview.github.com/?https://github.com/loyso/LuaCSP/blob/master/doc/html/LuaCSP.html).

[See the introductional slides here](https://docs.google.com/presentation/d/e/2PACX-1vRlyPnFoHmeKGqGNUyObV1aGpJ8hCD8gVz7c8aIkX_AuvCXhSI3wYRphWC95YvWR7PkQp7OWu1pHGkV/pub?start=false&loop=false&delayms=3000).

Why?

- The world is parallel. Typical simulation and robotics projects deal with
many entities acting and interacting concurrently.

- Event-oriented systems (command+callback, finite state machines - FSMs) can be
implemented very efficiently but the resulting model logic is quite
fragmented, as the scheduling and the evaluation of conditions are
scattered throughout the event routines. 

- Callbacks and FSMs are way too low-level. FSMs are non-composable.

- Asyncronous "push" multi-threaded systems are hard to program and understand.
Whould be nice to orchestrate them at a higher level in synchronous way: back to
natural control flow - stack and sequence are very powerful abstractions.

We should resolve these complexity and maintainability issues.


How?

We introduce next level you model in: embeddable Lua-based programming language with
concurrent control-flow semantics.

So, LuaCSP:
- Uses the Occam programming language ideas by Tony Hoare (the author of Quicksort).
- Follows the principle of compositionality.
- Introduces restrictions and rules, how entities behave and interact.
- Extends Lua in standard way, includes it as a subset.

LuaCSP can be used under GNU General Public License (GPL), version 2.

http://www.gnu.org/licenses/old-licenses/gpl-2.0.html

Please, contact Alexey Baskakov if you want to use LuaCSP under any other license.

Copyright (c) 2012-2014 Alexey Baskakov.
Feel free to contact the author for additional features, consulting and support.
