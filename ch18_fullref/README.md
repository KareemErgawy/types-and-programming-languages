# Typed Lamda Calculus (with varios extensions)

## Syntax

### Terms

```
t ::=
    x
    l x:T. t
    t t
    true
    false
    if t then t else t
    0
    succ t
    pred t
    iszero t
    {l_i=t_i} for i in 1..n
    t.l
    let x = t in t
    ref t
    !t
    t := t
    t; t
    fix t
    l
```

### Values

```
v ::=
    x
    l x:T. t
    true
    false
    {l_i=v_i} for i in 1..n
    nv
    l
    
nv ::=
    0
    succ nv
```

### Types

```
T ::=
    Bool
    Nat
    {l_i:T_i} for i in 1..n
    T -> T
    Ref T
    Source T
    Sink T
```

### Contexts

```
Γ ::=
    Φ
    Γ, x:T
```

## Typing Rules

TODO

## Evaluation Rules

TODO
