**Paterson's Worms Simulator**

[Paterson's Worms](https://en.wikipedia.org/wiki/Paterson%27s_worms) is a turmite-type cellular automaton with complex behavior. There are a few hundred different configurations, of which two finish in 57.5 trillion steps, and one is still not known if it is finite or infinite.

**HashLife**

Tomas Rokicki and Benjamin Chaffin used HashLife algorithm to solve all worms except one. There are many good descriptions of the algorithm in internet. In short, the space is represented as a recursive Quad-tree of squares, where squares with same contents are deduplicated. Simulation remembers, for each $2n \times 2n$ square, the evolution of its center $n \times n$ square in $n$ steps, which can be built up recursively from smaller to larger squares.
This algorithm runs the simulation of the worm "1042022" which finishes in 57.5 trillion moves, in about 25 hours on my computer.

**HashWorm**

Since turmites only evolve at one location (the "head" of a worm), it is better to do the other way around: in the quad-tree, at each level, there is only one square that contains the worm's head in its center sub-square. Similarly to HashLife, we can recursively calculate and memoize its evolution. In practice, the evolution of $n \times n$ square can continue until the worm runs out of this square, thus having much more than $n$ steps, ideally approaching $O(n^2)$ steps. I call this algorithm "HashWorm" because I like how it sounds.

The worm "1042022" is simulated in 3.5 hours using this algorithm.

Also, the algorithm simulates the worms "1252121" and "1525115" (which Wikipedia says "are believed to be infinite") with exponential speed, approaching $10^{100}$ steps in a few minutes. There you can see pattern of 18 shapes that repeat infinitely, every time on a $2^{18}$ larger scale.

**9-ary tree**

We can use the HashWorm algorithm on a 9-ary tree (instead of a quad-tree), where each rectangle has 3x3 children. This works better for most worms, because as it turns out, they have repetitive patterns with period divisible by three. Interestingly, the algorithm becomes slightly simpler, because there is one "center subsquare" at a lower level, as opposed to the quad-tree where the center sub-square is composed of four squares two levels below.

*Parent caching*

Additional optimization resulted in about 10% performance improvement in almost all cases: for every square we cache a bigger square one level above it in the tree, which had this smaller square as its center last time. Usually, there are many repetitive patterns, and the cache is hit often, allowing to avoid re-constructing squares from smaller squares.

Simulation of worm "1042022" runs in 35 minutes on my computer (with speeds up to 50 billion steps per second)!


**Last unsolved worm**

I've run the simulation of worm "1042015" using the last algorithm up to 1.7e22 steps, and it still does not finish nor show any regular behavior.
