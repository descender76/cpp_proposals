<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P3266R0</td>
</tr>
<tr>
<td>Date</td>
<td>2024-05-05</td>
</tr>
<tr>
<td>Reply-to</td>
<td>

Jarrad J. Waterloo &lt;descender76 at gmail dot com&gt;

</td>
</tr>
<tr>
<td>Audience</td>
<td>Evolution Working Group (EWG)</td>
</tr>
</table>

# non referenceable types

<style>
.inline-link
{
    font-size: small;
    margin-top: -2.8em;
    margin-right: 4px;
    text-align: right;
    font-weight: bold;
}

code
{
    font-family: "Fira Code", monospace !important;
    font-size: 0.87em;
}

.sourceCode
{
    font-size: 0.95em;
}

a code
{
    color: #0645ad;
}
</style>

## Table of contents

- [non referenceable types](#non-referenceable-types)
  - [Abstract](#Abstract)
  - [Motivational Examples](#Motivational-Examples)
  - [Motivation](#Motivation)
  - [More Potential Examples](#More-Potential-Examples)

## Abstract

Consumers of pure reference types with shallow `const` semantics would benefit by disallowing taking a reference to the type.

## Motivational Examples

### given

```cpp
  template<class... S> class function_ref;    // not defined

  template<class R, class... ArgTypes>
  class function_ref<R(ArgTypes...) cv noexcept(noex)> referenceable(false)// disengage being able to reference i.e. copy only
  {
    // ...
  };
 
template <class T>
  class optional<T&> referenceable(false)// disengage being able to reference i.e. copy only
  {
    // ...
  };
```

### usage

```cpp

// don't allow taking a reference to a non referenceable type
// as they are meant to be only copied
function_ref<void ()>& f(function_ref<void ()>&);// ill formed
optional<int&>& f(optional<int&>&);// ill formed

void action(){}

int main()
{
  function_ref<void ()> fr1{ action };
  // don't allow taking a reference to a non referenceable type
  // as they are meant to be only copied
  function_ref<void ()>& fr2 = fr1;// ill formed

  int i = 42;
  optional<int&> oi1{ i };// const by default
  // don't allow taking a reference to a non referenceable type
  // as they are meant to be only copied
  optional<int&>& oi2 = oi1;// ill formed

  return 0;
}
```

## Motivation

Currently a programmer can not create a reference to a reference as references only support a single level of indirection. Further, taking the address of a reference does not refer to the address of the reference but rather what the referent that the reference refers to. Allowing programmers to create non referenceable types would mean allowing programmers to create reference types that are more consistent with references. This would remove incorrect usage of reference types and prevent that creation of references which are largely useless and only serve to provide additional avenues to dangling.

## More Potential Examples

```cpp
class a referenceable(false){}; // prevent taking a referece to said type
class b referenceable(true){}; // same as class b {};
```

In terms of this proposal, the current default of the language is `referenceable(true)`.
