#

add_test(NAME Usage COMMAND binlab-bindump)
set_tests_properties(Usage PROPERTIES PASS_REGULAR_EXPRESSION "Usage:.*<input> <output>")
