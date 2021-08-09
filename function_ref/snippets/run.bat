docker run -it --rm -v "%cd%:/usr/src/project" -w /usr/src/project source-highlight source-highlight -s cpp -f html --doc snippet-function-pointer-simple.cc snippet-function-pointer-complex.cc snippet-function-pointer-struct.cc snippet-function_ref-private.cc snippet-function_ref-constructor.cc snippet-function_ref-constructors.cc snippet-__closure.cc snippet-delegate.cc
pause
