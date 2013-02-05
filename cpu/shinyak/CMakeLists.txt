cmake_minimum_required(VERSION 2.8)

cpu_setup("shinyak")

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")

add_library(shinyak_lib
            ai.cc ai_manager.cc ctrl.cc field.cc field_evaluator.cc
            game.cc player_info.cc protocol.cc puyo_possibility.cc)

function(shinyak_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_libraries(${exe} shinyak_lib)
  cpu_target_link_common_libraries(${exe})
endfunction()

function(shinyak_add_test exe)
  shinyak_add_executable(${exe} ${exe}.cc)
  cpu_target_link_libraries(${exe} ${GTEST_BOTH_LIBRARIES})
  if(NOT ARGV1)
    cpu_add_test(${exe})
  endif()
endfunction()

shinyak_add_executable(shinyak_cpu main.cc)
shinyak_add_executable(interactive interactive.cc)
shinyak_add_executable(solver solver.cc)

if(GTEST_FOUND)
  include_directories(${GTEST_INCLUDE_DIRS})

  shinyak_add_test(field_bit_field_test)
  shinyak_add_test(field_column_bit_field_test)
  shinyak_add_test(field_evaluator_test)
  shinyak_add_test(field_test)
  shinyak_add_test(puyo_set_test)
  shinyak_add_test(puyo_test)
  # TODO(hamaji): Slow!
  shinyak_add_test(performance_test 1)
  shinyak_add_test(puyo_possibility_test 1)
endif()