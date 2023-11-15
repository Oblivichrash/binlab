#

add_test(NAME Usage COMMAND "binlab-dumpbin")
set_tests_properties(Usage PROPERTIES PASS_REGULAR_EXPRESSION "Usage:.*<input> <output>")
