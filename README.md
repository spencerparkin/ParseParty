# ParseParty

This is an attempt to develop a general-purpose formal language parser.

Loosely speaking, a language is defined as the set of all strings (or utterances) reachable by a given grammar and an initial non-termal.  A grammar is a set of reproduction rules.  A parser is what takes a given string and grammar, and not only attempts to determine if that string is a member of the associated language, but also how the string is a member of the language; specifically, which sequence of grammar rules must be applied, and in what way, to generate the given string.  If more than one such sequence exists, then the string (and therefore language too) is said to be ambiguous.  This makes sense, because it means there is more than one syntax tree for the parsed string.  Deriving meaning from a string requires an analysis of its syntax tree, and so if there's more than one such tree, the meaning is unclear.

To keep things simple, my initial focus will be on context-free grammars.  These are grammers where every production rule contains a single non-terminal character on its left-hand side.  Any combination of terminal and non-termal characters may appears on the right-hand side of a rule.  I'll assume a given set of production rules is redundancy-free.  A production rule is considered redundant if it can be obtained using one or more of the other rules.
