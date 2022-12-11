Adapted from a problem described [here](https://artofproblemsolving.com/wiki/index.php/1999_USAMO_Problems/Problem_5).

There are two players that alternate placing either an ´S´ or an ´O´ in one of 16 spaces.

```____S___________```

```____S_O_________```

The player that places the character that completes an ´SOS´ wins.

```___SOS_O________```

The game ends in a draw if there is no space left.

```OOOSOOSOOSSOOOOS```

# ./spell_sos_release

Returns a line of moves that may occur under
[Minimax](https://en.wikipedia.org/wiki/Minimax) conditions.
The evaluation is exhaustive, proving that there always is a strategy for
the second player to win.
Running it may take minutes.
