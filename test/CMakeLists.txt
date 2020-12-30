# mandatory: SOURCE, EXPECTED
# optional: INPUT, defaults to /dev/null
# remarks: all paths should be relative
function(add_wcomp_test)
  set(flags "")
  set(singleValues NAME SOURCE INPUT EXPECTED)
  set(multiValues "")
  cmake_parse_arguments(add_wcomp_test
    "${flags}" "${singleValues}" "${multiValues}" ${ARGN})

  # Prepend the current dir to the params.
  set(add_wcomp_test_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${add_wcomp_test_SOURCE})
  set(add_wcomp_test_EXPECTED ${CMAKE_CURRENT_SOURCE_DIR}/${add_wcomp_test_EXPECTED})

  # Default INPUT to /dev/null.
  if(NOT DEFINED add_wcomp_test_INPUT)
    set(add_wcomp_test_INPUT /dev/null)
  else()
    set(add_wcomp_test_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/${add_wcomp_test_INPUT})
  endif()

  set(tmp "/tmp/result-${add_wcomp_test_NAME}")

  add_test(
    NAME test_${add_wcomp_test_NAME}_compile
    COMMAND sh -c "\
        $<TARGET_FILE:wcomp> -c ${add_wcomp_test_SOURCE} > ${tmp}.asm                       \
        && nasm -felf ${tmp}.asm -o ${tmp}.o                                                \
        && ${CMAKE_C_COMPILER} -m32 ${tmp}.o ${CMAKE_CURRENT_SOURCE_DIR}/io.c -o ${tmp}.out \
        && ${tmp}.out < ${add_wcomp_test_INPUT} > ${tmp}.output                             \
        && diff ${tmp}.output ${add_wcomp_test_EXPECTED} 1>&2"
    COMMAND_EXPAND_LISTS
  )
  add_test(
    NAME test_${add_wcomp_test_NAME}_interpret
    COMMAND sh -c "\
        $<TARGET_FILE:wcomp> -i ${add_wcomp_test_SOURCE} < ${add_wcomp_test_INPUT} > ${tmp}.output \
        && diff ${tmp}.output ${add_wcomp_test_EXPECTED} 1>&2"
    COMMAND_EXPAND_LISTS
  )
  # TODO: Add dependency to build the wcomp before the test.
endfunction()


add_wcomp_test(NAME     arithmetic
               SOURCE   test_arithmetic.ok
               EXPECTED test_arithmetic.out)
add_wcomp_test(NAME     assignment
               SOURCE   test_assignment.ok
               EXPECTED test_assignment.out)
add_wcomp_test(NAME     branching
               SOURCE   test_branching.ok
               EXPECTED test_branching.out)
add_wcomp_test(NAME     divisor
               SOURCE   test_divisor.ok
               EXPECTED test_divisor.out
               INPUT    test_divisor.in)
add_wcomp_test(NAME     logic
               SOURCE   test_logic.ok
               EXPECTED test_logic.out)
add_wcomp_test(NAME     looping
               SOURCE   test_looping.ok
               EXPECTED test_looping.out
               INPUT    test_looping.in)
add_wcomp_test(NAME     read
               SOURCE   test_read.ok
               EXPECTED test_read.out
               INPUT    test_read.in)
add_wcomp_test(NAME     write_boolean
               SOURCE   test_write_boolean.ok
               EXPECTED test_write_boolean.out)
add_wcomp_test(NAME     write_natural
               SOURCE   test_write_natural.ok
               EXPECTED test_write_natural.out)
