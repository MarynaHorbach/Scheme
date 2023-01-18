#include "scheme_test.h"

 TEST_CASE_METHOD(SchemeTest, "Quote") {
    ExpectEq("(quote (1 2))", "(1 2)");

     ExpectEq("(quote (quote 1))", "(quote 1)");
     ExpectEq("'(1 2)", "(1 2)");
 }

TEST_CASE_METHOD(SchemeTest, "Be careful") {
    ExpectSyntaxError("' ");
//    ExpectSyntaxError("1 ( symbol ) + ");
    ExpectSyntaxError("())");
    ExpectSyntaxError(")");
    ExpectRuntimeError("(())");
    ExpectRuntimeError("(+ ())");
    ExpectRuntimeError("('() ())");
    ExpectEq("'(())", "(())");
}
