REM docker run -it --rm -v "%cd%:/usr/src/project" -w /usr/src/project source-highlight source-highlight -s cpp -f html --doc snippet-function-pointer-simple.cc snippet-function-pointer-complex.cc snippet-function-pointer-struct.cc snippet-function_ref-private.cc snippet-function_ref-constructor.cc snippet-function_ref-constructors.cc snippet-__closure.cc snippet-delegate.cc
docker run -it --rm -v "%cd%:/usr/src/project" -w /usr/src/project source-highlight source-highlight -s cpp -f html --no-doc --output-dir=site/snippets snippet-function-pointer-simple.cc snippet-function-pointer-complex.cc snippet-function-pointer-struct.cc snippet-function_ref-private-spec.cc snippet-function_ref-private-impl.cc snippet-function_ref-constructor.cc snippet-function_ref-constructors.cc snippet-__closure.cc snippet-delegate.cc snippet-cpp-cleanly.cc snippet-cpp-lambda-capture.cc snippet-cpp-unsafe-constructor.cc snippet-function-with-type-erasure.cc snippet-function-without-type-erasure.cc snippet-member-function-with-type-erasure.cc snippet-member-function-without-type-erasure.cc snippet-cpp-unsafe-constructor-usage.cc
pause
