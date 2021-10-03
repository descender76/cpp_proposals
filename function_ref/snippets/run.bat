REM docker run -it --rm -v "%cd%:/usr/src/project" -w /usr/src/project source-highlight source-highlight -s cpp -f html --doc snippet-function-pointer-simple.cc snippet-function-pointer-complex.cc snippet-function-pointer-struct.cc snippet-function_ref-private.cc snippet-function_ref-constructor.cc snippet-function_ref-constructors.cc snippet-__closure.cc snippet-delegate.cc
docker run -it --rm -v "%cd%:/usr/src/project" -w /usr/src/project source-highlight source-highlight -s cpp -f html --no-doc --output-dir=site/snippets snippet-function-pointer-simple.cc snippet-function-pointer-complex.cc snippet-function-pointer-struct.cc snippet-function_ref-private-spec.cc snippet-function_ref-private-impl.cc snippet-function_ref-constructor.cc snippet-function_ref-constructors.cc snippet-__closure.cc snippet-delegate.cc snippet-cpp-cleanly.cc snippet-cpp-lambda-capture.cc snippet-cpp-unsafe-constructor.cc snippet-function-with-type-erasure.cc snippet-function-with-type-erasure-usage.cc snippet-function-without-type-erasure.cc snippet-function-without-type-erasure-usage.cc snippet-member-function-with-type-erasure.cc snippet-member-function-with-type-erasure-usage.cc snippet-member-function-without-type-erasure.cc snippet-member-function-without-type-erasure-usage.cc snippet-cpp-unsafe-constructor-usage.cc snippet-cpp-unsafe-constructor-usage-alt.cc snippet-cpp-unsafe-constructor-usage-declarations.cc snippet-functor-lambda.cc snippet-functor-lambda-usage.cc snippet-function_ref-prime.cc snippet-cpp-function-pointer-cast.cc snippet-cpp-safe-constructor.cc snippet-cpp-member-function-pointer-to-function-pointer-cast.cc snippet-function_ref-constructors-te.cc snippet-function_ref-constructors-te-alt.cc snippet-function_ref-spec.cc snippet-function_ref-spec-rev.cc snippet-function_ref-type-erased-member-function-usage.cc snippet-function_ref-type-erased-member-function-constructor.cc snippet-function_ref-type-erased-member-function-tag.cc make_function_ref-usage.cc make_function_ref-spec.cc mfpi2fp.cc
pause
